#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TMath.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TBranch.h"
#include "TSystem.h"
#include "TApplication.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"

#include <sstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

unsigned int N_PEAKS = 2;
double PIXIE_TIME_RES = 4.0; // In ns

struct peak{
	unsigned int left, max, cfd;
	double value;
	
	peak(){
		left = 0; max = 0; cfd = 0; value = 0.0;
	}
	
	peak(unsigned int left_, unsigned int max_, unsigned int cfd_, double value_){
		left = left_; max = max_; cfd = cfd_; value = value_;
	}
};

// 1D function to use for pulse fitting
// x[0] = time t in ns
// parameters (3 * N_PEAKS)
//  par[0] = alpha (normalization of pulse 1)
//  par[1] = phi (phase of pulse 1 in ns)
//  par[2] = beta (decay parameter of the 1st pulse exponential in ns)
//  par[3] = gamma (width of the inverted square gaussian of the 1st pulse in ns^4)
//  ...
double fit_function(double *x, double *par){
	double arg = x[0] - par[1];
	if(arg >= 0.0){ return par[0]*TMath::Exp(-arg/par[2])*(1 - TMath::Exp(-arg*arg*arg*arg/par[3])); }
	return 0.0;
}

double variance(double *data, unsigned int start, unsigned int stop){
	double mean = 0.0;
	double output = 0.0;
	for(unsigned int i = start; i < stop; i++){
		mean += data[i];
	}
	mean = mean/(stop-start);
	for(unsigned int i = start; i < stop; i++){
		output += (data[i] - mean)*(data[i] - mean);
	}
	return output/(stop-start);
}

double chi_2(double *data, unsigned int start, unsigned int stop, double *pars){
	double var = variance(data, start, stop);
	double output = 0.0, temp;
	double x_value[1] = {0.0};
	for(unsigned int i = start; i < stop; i++){
		x_value[0] = i;
		temp = (data[i] - fit_function(x_value, pars));
		output += temp*temp/var;
	}
	return output;
}

// Integrate a pulse and return the short and long integrals
unsigned int integrate(std::vector<int> &arr, unsigned int pulse_size, std::vector<double> &s_int, 
					   std::vector<double> &l_int, unsigned short method, bool debug=false, TCanvas *canvas=NULL){
	if(debug && !canvas){ debug = false; }
	unsigned int size = arr.size();
	s_int.clear(); l_int.clear();
	if(size == 0){ return 0; }
	if(size % pulse_size != 0){ return 0; }
	unsigned int num_pulses = size/pulse_size; // Expects size to be divisible by pulse_size	
	
	// Copy the pulse into a new array so the original pulse is unmodified
	double *darr = new double[size];
	unsigned int count = 0;
	for(std::vector<int>::iterator iter = arr.begin(); iter != arr.end(); iter++){
		if(count >= size){ break; }
		darr[count] = (double)(*iter);
		count++;
	}
	
	// Variables for TGraph
	double *x = new double[pulse_size];
	const double *x_val = new double[pulse_size];
	const double *y_val = new double[pulse_size];
	
	TGraph *graph = NULL;
		
	if(debug){
		graph = new TGraph(pulse_size, x_val, y_val);
		for(unsigned int i = 0; i < pulse_size; i++){ x[i] = i; }
	}
	
	unsigned int start, stop;
	peak first_peak;
	double s, l;
	for(unsigned int pulse = 0; pulse < num_pulses; pulse++){
		start = pulse*pulse_size;
		stop = (pulse+1)*pulse_size;

		// Do baseline correction (use the range to the left of the pulse)
		double base_sum = 0.0;
		unsigned int base_max = start + (unsigned int)(0.1*pulse_size); // First 10% of range
		for(unsigned int i = start; i < base_max; i++){
			base_sum += darr[i];
		}
	
		double base_line = base_sum/base_max;
		for(unsigned int i = start; i < stop; i++){
			darr[i] = darr[i] - base_line;
		}

		if(debug){
			for(unsigned int i = start; i < stop; i++){ graph->SetPoint(i, x[i], darr[i]); }
			canvas->cd();
			canvas->Clear();
			graph->Draw();
			canvas->Update();
			canvas->WaitPrimitive();
		}
	
		// Find the global maximum
		double maximum = -9999.0;
		unsigned int max_bin = 0;
		for(unsigned int i = start; i < stop; i++){
			if(darr[i] > maximum){ 
				maximum = darr[i];
				max_bin = i;
			}
		}

		// Find first peak (there could be more than 1)
		unsigned int leading_edge;
		double back_sub_left = 0.0;
		double back_sub_right = 0.0;
		double slope;
		bool up_slope = false;
		bool found_peak = false;
		bool multi_peak = false;
		for(unsigned int i = start; i < stop-1; i++){
			// Subtract background (10% of maximum)
			back_sub_left = darr[i] - 0.1*maximum;
			back_sub_right = darr[i+1] - 0.1*maximum;
			if(back_sub_left < 0.0){ back_sub_left = 0.0; }
			if(back_sub_right < 0.0){ back_sub_right = 0.0; }
			
			// Calculate the slope
			slope = back_sub_right - back_sub_left;
			if(up_slope){ // On a positive slope
				if(slope < 0.0){
					up_slope = false;
					first_peak.left = leading_edge;
					first_peak.max = i;
					first_peak.value = darr[i];
				}
			}
			else{ // Not on a positive slope
				if(slope > 0.0){ // Found a pulse
					if(!found_peak){ 
						up_slope = true;
						found_peak = true; 
						leading_edge = i;
					}
					else{ 
						multi_peak = true; 
						break;
					}
				}
			}
		}

		if((method == 3 || method == 4) && multi_peak){ // Multiple peaks will throw off the fitting functions
			if(debug){ std::cout << "Multiple peaks, skipping entry\n"; }
			delete[] darr;
			return 0;
		}

		// Find 80% of the peak maximum
		for(unsigned int i = start; i < stop; i++){
			if(darr[i] >= 0.8*first_peak.value){
				if(i > 0){ first_peak.cfd = i-1; }
				else{ first_peak.cfd = i; }
				break;
			}
		}

		// Do short and long integrals
		s = 0.0; l = 0.0;
		if(debug){ std::cout << "left = " << first_peak.left << ", cfd = " << first_peak.cfd << ", max = " << first_peak.max << ", value = " << first_peak.value << std::endl; }
			
		if(method == 0){ // Slope inversion
			// To the right of the maximum, we expect a negative slope
			// Stop short integral when the slope goes non-negative
			bool calc_short = true;
			for(unsigned int i = first_peak.cfd; i < stop-1; i++){
				// Integrate using trapezoid rule
				if(i < first_peak.max){
					s += (darr[i] + darr[i+1])/2.0;
					l += (darr[i] + darr[i+1])/2.0;
				}
				else{
					if(calc_short){ // Waiting on positive slope to stop calculating short integral
						if(darr[i+1]-darr[i] <= 0.0){ // Negative slope, still on side of pulse
							s += (darr[i] + darr[i+1])/2.0;
							l += (darr[i] + darr[i+1])/2.0;
						}
						else{ 
							l += (darr[i] + darr[i+1])/2.0;
							calc_short = false; 
						}
					}
					else{ // Encountered positive slope, only calculating long integral
						if(darr[i] <= 0.0){ break; } // Bottom of pulse, stop calculating long integral
						l += (darr[i] + darr[i+1])/2.0;
					}
				}
			}
		} // method == 0
		else if(method == 1){ // Fixed pulse height
			// Stop short integral when we reach 10% of the pulse height
			// Find the 10% of maximum point
			unsigned int ten_percent_bin = 0;
			for(unsigned int i = first_peak.max; i < stop; i++){
				if(darr[i] <= 0.1*maximum){ 
					ten_percent_bin = i;
					break;
				}
			}
			for(unsigned int i = first_peak.cfd; i < stop-1; i++){
				// Integrate using trapezoid rule with arbitrary bin width of 1
				if(i < ten_percent_bin){ s += (darr[i] + darr[i+1])/2.0; } // Stop calculating short below 10% maximum
				if(darr[i] <= 0.0){ break; } // Bottom of pulse, stop calculating long integral
				l += (darr[i] + darr[i+1])/2.0;
			}
		} // method == 1
		else if(method == 2){ // Use fast fit guessing (S vs. L version) Can handle multiple peaks
			double step = pulse_size/1000.0; // Integration step
			double x_value1[1] = {0.0};
			double x_value2[1] = {0.0};

			// Set optimized starting values
			double parameters[4];
			parameters[0] = first_peak.value*9.211 + 150.484;	// Normalization of pulse
			parameters[1] = first_peak.max*1.087 - 2.359;		// Phase (leading edge of pulse) (ns)
			parameters[2] = 1.7750575;						// Decay constant of exponential (ns)
			parameters[3] = 115.64125; 						// Width of inverted square guassian (ns^4)
		
			for(unsigned int j = 0; j < 999; j++){
				x_value1[0] = j*step; x_value2[0] = (j+1)*step;
				s += step*(fit_function(x_value1, parameters) + fit_function(x_value2, parameters))/2.0;
			}
		
			for(unsigned int j = start; j < stop-1; j++){
				l += (darr[j] + darr[j+1])/2.0;
			}
		} // method == 2
		else if(method == 3){ // Use fast fit guessing (chi^2 version)
			// Variables for TGraph
			unsigned int delta = stop - start;
			double *x = new double[delta];
			const double *x_val = new double[delta];
			const double *y_val = new double[delta];

			TGraph *graph = new TGraph(delta, x_val, y_val);
			for(unsigned int i = 0; i < delta; i++){
				x[i] = i;
			}

			unsigned int index = 0;
			for(unsigned int i = start; i < stop; i++){
				graph->SetPoint(i, x[i], darr[index]);
				index++;
			}

			// Set optimized starting values
			double parameters[4];
			parameters[0] = first_peak.value*9.211 + 150.484;	// Normalization of pulse
			parameters[1] = first_peak.left*1.087 - 2.359;		// Phase (leading edge of pulse) (ns)
			parameters[2] = 1.7750575;						// Decay constant of exponential (ns)
			parameters[3] = 115.64125; 						// Width of inverted square guassian (ns^4)

			TF1 *func = new TF1("func",fit_function,0.0,pulse_size,4);
			func->SetParameters(parameters);
			if(debug){ 
				func->Draw("SAME"); 
				canvas->Update();
				canvas->WaitPrimitive();
				std::cout << " parameters = {" << parameters[0] << ", " << parameters[1] << ", " << parameters[2] << ", " << parameters[3] << "}\n";
			}

			func->FixParameter(0, parameters[0]);
			func->FixParameter(1, parameters[1]);
			func->FixParameter(2, parameters[2]);
			func->FixParameter(3, parameters[3]);

			TFitResultPtr func_ptr = graph->Fit(func,"S Q");
			s = func_ptr->Chi2();
			func->Delete();
		} // method == 3
		else if(method == 4){ // Use root fitting (slow)
			// Variables for TGraph
			unsigned int delta = stop - start;
			double *x = new double[delta];
			const double *x_val = new double[delta];
			const double *y_val = new double[delta];

			TGraph *graph = new TGraph(delta, x_val, y_val);
			for(unsigned int i = 0; i < delta; i++){
				x[i] = i;
			}

			unsigned int index = 0;
			for(unsigned int i = start; i < stop; i++){
				graph->SetPoint(i, x[i], darr[index]);
				index++;
			}

			// Set optimized starting values
			double parameters[4];
			parameters[0] = first_peak.value*9.211 + 150.484;	// Normalization of pulse
			parameters[1] = first_peak.max*1.087 - 2.359;		// Phase (leading edge of pulse) (ns)
			parameters[2] = 1.7750575;						// Decay constant of exponential (ns)
			parameters[3] = 115.64125; 						// Width of inverted square guassian (ns^4)

			TF1 *func = new TF1("func",fit_function,0.0,pulse_size,4);
			func->SetParameters(parameters);
			if(debug){ 
				func->Draw("SAME");
				canvas->Update();
				canvas->WaitPrimitive(); 
				std::cout << " parameters = {" << parameters[0] << ", " << parameters[1] << ", " << parameters[2] << ", " << parameters[3] << "}\n";
			}

			TFitResultPtr func_ptr = graph->Fit(func,"S Q");
			s = func_ptr->Chi2();
			func->Delete();
		} // method == 4
		if(debug){ std::cout << " s = " << s << ", l = " << l << std::endl; }
		s_int.push_back(s);
		l_int.push_back(l);
	} // Over num_pulses
	
	if(debug){ graph->Delete(); }
	delete[] darr, x, x_val, y_val;
	return num_pulses;
}

// For compilation
int main(int argc, char* argv[]){
	if(argc < 4){
		std::cout << " Missing required argument, aborting\n";
		std::cout << "  SYNTAX: Viewer {filename treename branch method#}\n";
		return 1;
	}
	
	// Variables for root graphics
	char* dummy[0]; 
	TApplication* rootapp = new TApplication("rootapp",0,dummy);
	gSystem->Load("libTree");
	
	unsigned short method = atol(argv[4]);
	std::cout << " Using analysis method " << method << std::endl;
	if(!(method == 0 || method == 1 || method == 2 || method == 3)){
		std::cout << " Encountered undefined method (" << method << "), aborting\n";
		return 1;
	}

	// Branch variables
	std::vector<int> wave;
	std::vector<double> energy;
	unsigned int mult;
	unsigned int wave_size;

	TFile *file = new TFile(argv[1], "READ");
	if(file->IsZombie()){
		std::cout << " Failed to load the input file '" << argv[1] << "'\n";
		return 1;
	}
	TTree *tree = (TTree*)file->Get(argv[2]);
	if(!tree){
		std::cout << " Failed to load the input tree '" << argv[2] << "'\n";
		file->Close();
		return 1;
	}
	tree->SetMakeClass(1);
	
	std::stringstream branch_name;
	branch_name << argv[3];
	TBranch *b_wave, *b_mult, *b_energy;
	tree->SetBranchAddress((branch_name.str()+"_wave").c_str(), &wave, &b_wave);
	tree->SetBranchAddress((branch_name.str()+"_energy").c_str(), &energy, &b_energy);
	tree->SetBranchAddress((branch_name.str()+"_wave_mult").c_str(), &mult, &b_mult);
	
	if(!b_wave){
		std::cout << " Failed to load the input branch '" << branch_name.str() << "_wave'\n";
		file->Close();
		return 1;
	}
	if(!b_energy && (method == 3 || method == 4)){
		std::cout << " Failed to load the input branch '" << branch_name.str() << "_energy'\n";
		std::cout << "  Method " << method << " requires a branch named '" << branch_name.str() << "_energy' to plot Chi^2 vs. Energy\n";
		file->Close();
		return 1;
	}
	if(!b_mult){
		std::cout << " Failed to load the input branch '" << branch_name.str() << "_wave_mult'\n";
		file->Close();
		return 1;
	}

	// Get the pulse size
	for(unsigned int i = 0; i < tree->GetEntries(); i++){
		tree->GetEntry(i);
		if(mult == 0){ continue; }
		else{ 
			wave_size = wave.size()/mult;
			break; 
		}
	}
	std::cout << " Using wave size " << wave_size << std::endl;

	// Canvas
	TCanvas *can = new TCanvas("can", "canvas");
	can->cd();

	// Histogram
	TH2D *hist = NULL;
	if(method == 0 || method == 1 || method == 2){
		hist = new TH2D("hist", "Long vs. Short", 200, 0, 3000, 200, 0, 5000);
		hist->GetXaxis()->SetTitle("Short (arb. units)");
		hist->GetYaxis()->SetTitle("Long (arb. units)");
	}
	else if(method == 3){
		hist = new TH2D("hist", "Chi^2 vs. Energy", 200, 0, 2000.0, 200, 0, 5000);
		hist->GetXaxis()->SetTitle("Chi^2 (arb. units)");
		hist->GetYaxis()->SetTitle("Energy (arb. units)");
	}
	else if(method == 4){
		hist = new TH2D("hist", "Chi^2 vs. Energy", 200, 0, 1000.0, 200, 0, 5000);
		hist->GetXaxis()->SetTitle("Chi^2 (arb. units)");
		hist->GetYaxis()->SetTitle("Energy (arb. units)");
	}
	//hist->SetStats(false);

	std::vector<double> short_integral, long_integral;
	std::vector<double>::iterator iter1, iter2;
	unsigned int count = 0;
	std::cout << " Processing " << tree->GetEntries() << " entries\n";
	//for(unsigned int i = 0; i < tree->GetEntries(); i++){
	for(unsigned int i = 0; i < 100000; i++){
		tree->GetEntry(i);
		if(i % 100000 == 0 && i != 0){ std::cout << " Entry no. " << i << std::endl; }
		count += integrate(wave, wave_size, short_integral, long_integral, method, false, can);
		if(method == 0 || method == 1 || method == 2){ 
			for(iter1 = short_integral.begin(), iter2 = long_integral.begin(); iter1 != short_integral.end() && iter2 != long_integral.end(); iter1++, iter2++){
				hist->Fill((*iter1), (*iter2)); 
			}
		}
		if(method == 3 || method == 4){ 
			for(iter1 = short_integral.begin(), iter2 = energy.begin(); iter1 != short_integral.end() && iter2 != energy.end(); iter1++, iter2++){
				hist->Fill((*iter1), (*iter2)); 
			}
		}
	}
	
	std::cout << " Found " << count << " pulses in " << tree->GetEntries() << " tree entries\n";
	
	hist->Draw("COLZ");
	can->Update();
	can->WaitPrimitive();

	hist->Delete();
	can->Close();
	file->Close();
		
	return 0;
}

// For CINT
//int PulseAnalyzer(int argc, char* argv[]){ return main(argc, argv); }
