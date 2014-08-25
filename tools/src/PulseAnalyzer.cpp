#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TF1.h"
#include "TCutG.h"
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
#include <time.h>
#include <cmath>

unsigned int N_PEAKS = 2;
double PIXIE_TIME_RES = 4.0; // In ns

struct peak{
	unsigned int left, max, min, cfd;
	double max_value, min_value;
	
	peak(){
		left = 0; max = 0; min = 0; cfd = 0; 
		max_value = -9999.0; min_value = 9999.0;
	}
	
	peak(unsigned int left_, unsigned int max_, unsigned int min_, unsigned int cfd_, double max_value_, double min_value_){
		left = left_; max = max_; min = min_; cfd = cfd_; 
		max_value = max_value_; min_value = min_value_;
	}
	
	std::string print(){
		std::stringstream output;
		output << "left = " << left << ", cfd = " << cfd;
		output << ", max_bin = " << max << ", max_value = " << max_value;
		output << ", min_bin = " << min << ", min_value = " << min_value; 
		return output.str();
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
	if(arg >= 0.0){ return par[0]*std::exp(-arg/par[2])*(1 - std::exp(-arg*arg*arg*arg/par[3])); }
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
	double *x1 = new double[pulse_size];
	double *x2 = new double[pulse_size];
	const double *x_val1 = new double[pulse_size];
	const double *y_val1 = new double[pulse_size];
	const double *x_val2 = new double[pulse_size];
	const double *y_val2 = new double[pulse_size];
	
	TGraph *graph1 = NULL;
	TGraph *graph2 = NULL;
		
	if(debug){
		graph1 = new TGraph(pulse_size, x_val1, y_val1);
		graph2 = new TGraph(pulse_size, x_val2, y_val2);
		for(unsigned int i = 0; i < pulse_size; i++){ x1[i] = i; x2[i] = i; }
		graph1->SetLineColor(2); graph1->SetLineWidth(2);
		graph2->SetLineColor(4); graph2->SetLineWidth(2);
		canvas->cd();
	}
	
	unsigned int start, stop;
	peak first_peak;
	peak second_peak;
	double s, l;
	if(num_pulses != 1){ return 0; }
	for(unsigned int pulse = 0; pulse < num_pulses; pulse++){
	//for(unsigned int pulse = 0; pulse < num_pulses; pulse++){
		start = pulse*pulse_size;
		stop = (pulse+1)*pulse_size;

		// Do baseline correction. Use the range to the left of the pulse.
		// The right side of the pulse may have other pulses
		double base_sum = 0.0;
		unsigned int base_max = start + (unsigned int)(0.1*pulse_size); // First 10% of range
		for(unsigned int i = start; i < base_max; i++){
			base_sum += darr[i];
		}
	
		double base_line = base_sum/(base_max-start);
		for(unsigned int i = start; i < stop; i++){
			darr[i] = darr[i] - base_line;
		}

		if(debug){
			for(unsigned int i = start; i < stop; i++){ 
				graph1->SetPoint(i-start, x1[i-start], darr[i]); 
			}
		}
	
		// Find the global maximum
		double maximum = -9999.0;
		unsigned int maximum_bin = 0;
		for(unsigned int i = start; i < stop; i++){
			if(darr[i] > maximum){ 
				maximum = darr[i];
				maximum_bin = i;
			}
		}

		// Peak-finder (there could be more than 1)
		unsigned int leading_edge, peak_bin;
		double back_sub_left = 0.0;
		double back_sub_right = 0.0;
		double slope;
		bool up_slope = false;
		bool down_slope = false;
		bool found_peak = false;
		unsigned short peak_count = 0;
		for(unsigned int i = start; i < stop-1; i++){
			// Subtract background (10% of maximum)
			back_sub_left = darr[i] - 0.1*maximum;
			back_sub_right = darr[i+1] - 0.1*maximum;
			if(back_sub_left < 0.0){ back_sub_left = 0.0; }
			if(back_sub_right < 0.0){ back_sub_right = 0.0; }
			
			// Calculate the slope
			slope = back_sub_right - back_sub_left;
			if(!up_slope && slope > 0.0){ // Moving to up_slope marks leading edge of pulse
				up_slope = true; 
				down_slope = false;
				leading_edge = i;
			}
			else if(up_slope && slope < 0.0){ // Moving from up_slope to down_slope marks a pulse peak
				down_slope = true; 
				up_slope = false;
				found_peak = true;
				peak_count++;
				peak_bin = i;
			}
			else if(down_slope && slope > 0.0){ // Moving from down_slope to non-negative slope marks trailing edge (reset peak-finder)
				up_slope = false;
				down_slope = false;
			}
			
			if(found_peak){
				found_peak = false;
				if(peak_count == 1){
					first_peak.left = leading_edge;
					first_peak.max = peak_bin;
					first_peak.max_value = darr[peak_bin];
				}
				else if(peak_count == 2){
					second_peak.left = leading_edge;
					second_peak.max = peak_bin;
					second_peak.max_value = darr[peak_bin];
					break; // That's enough!
				}
			}
		}

		if(peak_count == 0){ // Failed to find a peak
			if(debug){ std::cout << "No peaks found, skipping entry\n"; }
			delete[] darr;
			return 0;
		}
		if((method == 4 || method == 5) && peak_count > 1){ // Multiple peaks will throw off the fitting functions
			if(debug){ std::cout << "Multiple peaks, skipping entry\n"; }
			delete[] darr;
			return 0;
		}

		// Find the local minimum
		if(peak_count == 1){ // Single pulse, minimum must occur to the right of the pulse
			for(unsigned int i = first_peak.max; i < stop; i++){
				if(darr[i] < first_peak.min_value){
					first_peak.min_value = darr[i];
					first_peak.min = i;
				}
			}
		}
		else{ // Multiple pulses, minimum must occur between pulse peaks
			for(unsigned int i = first_peak.max; i < second_peak.max; i++){
				if(darr[i] < first_peak.min_value){
					first_peak.min_value = darr[i];
					first_peak.min = i;
				}
			}
		}

		// Find 80% of the peak maximum
		for(unsigned int i = first_peak.left; i <= first_peak.max; i++){
			if(darr[i] >= 0.8*first_peak.max_value){
				if(i > 0){ first_peak.cfd = i-1; }
				else{ first_peak.cfd = i; }
				break;
			}
		}
		if(first_peak.cfd == first_peak.max){ first_peak.cfd--; }

		// Print debug information
		if(debug){ 
			std::cout << "Global: baseline = " << base_line << ", maximum_bin = " << maximum_bin;
			std::cout << ", maximum = " << maximum << ", peak_count = " << peak_count << std::endl;
			std::cout << "Peak: " << first_peak.print() << std::endl;
		}
		
		// Do short and long integrals
		s = 0.0; l = 0.0;			
		if(method == 0){ // Slope inversion
			// To the right of the maximum, we expect a negative slope
			// Stop short integral when the slope goes non-negative
			bool calc_short = true;
			for(unsigned int i = first_peak.cfd; i < first_peak.min; i++){
				// Integrate using trapezoid rule
				if(calc_short){ // Waiting on positive slope to stop calculating short integral
					if(i < first_peak.max || darr[i+1]-darr[i] <= 0.0){ s += (darr[i] + darr[i+1])/2.0; } // Negative slope, still on side of pulse
					else{ calc_short = false; }
				}
				l += (darr[i] + darr[i+1])/2.0;
			}
		} // method == 0
		else if(method == 1){ // Fixed pulse height
			// Stop short integral when we reach 10% of the pulse height
			// Find the 10% of maximum point
			unsigned int ten_percent_bin = 0;
			for(unsigned int i = first_peak.max; i < stop; i++){
				if(darr[i] <= 0.1*first_peak.max_value){ 
					ten_percent_bin = i;
					break;
				}
			}
			
			if(debug){ std::cout << " method = 1, 10p_bin = " << ten_percent_bin << std::endl; }	
			
			for(unsigned int i = first_peak.cfd; i < first_peak.min; i++){
				// Integrate using trapezoid rule with arbitrary bin width of 1
				if(i < ten_percent_bin){ s += (darr[i] + darr[i+1])/2.0; } // Stop calculating short below 10% maximum
				l += (darr[i] + darr[i+1])/2.0;
			}
		} // method == 1
		else if(method == 2){ // Use fast fit guessing (S vs. L version) Can handle multiple peaks
			double step = (first_peak.min - first_peak.cfd)/100.0; // Integration step
			double x_value[1] = {0.0};

			// Set optimized starting values
			double parameters[4];
			parameters[0] = first_peak.max_value*9.211 + 150.484;			// Normalization of pulse
			parameters[1] = (first_peak.left % pulse_size)*1.087 - 2.359;	// Phase (leading edge of pulse) (ns)
			parameters[2] = 1.7750575;										// Decay constant of exponential (ns)
			parameters[3] = 115.64125;										// Width of inverted square guassian (ns^4)

			if(debug){ 
				std::cout << " method = 2, par[0] = " << parameters[0] << ", par[1] = " << parameters[1] << ", par[2] = ";
				std::cout << parameters[2] << ", par[3] = " << parameters[3] << std::endl;
				
				TF1 *func = new TF1("func",fit_function,0.0,pulse_size,4);
				func->SetLineColor(4); func->SetLineWidth(2);
				func->SetParameters(parameters);

				canvas->Clear();
				graph1->Draw();
				func->Draw("SAME");
				canvas->Update();
				canvas->WaitPrimitive();
				
				func->Delete();			
			}	

			// Find the RMS of the "fit" function
			double temp;
			for(unsigned int j = 0; j <= 100; j++){
				x_value[0] = first_peak.cfd + j*step;
				temp = fit_function(x_value, parameters);
				s += temp*temp;
			}
			s = std::sqrt(s / 100.0);
			
			// Find the RMS of the pulse
			for(unsigned int j = first_peak.cfd; j <= first_peak.min; j++){
				l += darr[j]*darr[j];
			}
			l = std::sqrt(l / (first_peak.min - first_peak.cfd));
			
			// I fixed this problem, mostly (I think)
			if(debug && s == 0){ 
				for(unsigned int j = 0; j < 4; j++){ std::cout << parameters[j] << std::endl; }
				std::cout << "----" << std::endl;
				std::cout << first_peak.print() << std::endl << std::endl;
			}
		} // method == 2
		else if(method == 3){ // Search for decay-side slope variation
			double darr_prime[pulse_size];
			for(unsigned int i = start; i < stop-1; i++){
				darr_prime[i-start] = darr[i+1] - darr[i];
			}
			darr_prime[pulse_size-1] = 0.0;

			// Find the most negative slope, this will be the characteristic slope of the pulse
			double minimum_slope = 9999.0;
			unsigned int minimum_slope_bin = 0, sstop = 0;
			for(unsigned int i = start; i < stop; i++){
				if(darr_prime[i] < minimum_slope){
					minimum_slope = darr_prime[i-start];
					minimum_slope_bin = i-start;
				}
			}

			// Find the bin in which to stop short integration
			for(unsigned int i = minimum_slope_bin; i < stop; i++){
				if(darr_prime[i] >= 0.5*minimum_slope){
					sstop = i;
					break;
				}
			}

			if(debug){
				std::cout << " method = 3, slope_bin = " << minimum_slope_bin << ", min_slope = " << minimum_slope;
				std::cout << ", sstop = " << sstop << ", lstop = " << first_peak.min << std::endl;			
				for(unsigned int i = start; i < stop; i++){ // 1st derivative
					graph2->SetPoint(i-start, x2[i-start], darr_prime[i-start]);
				}
				canvas->Clear();
				graph2->Draw();
				graph1->Draw("SAME");
				canvas->Update();
				canvas->WaitPrimitive();
			}	
			
			// Integrate using trapezoid rule
			for(unsigned int i = first_peak.cfd; i < first_peak.min; i++){
				if(i < sstop){ s += (darr[i] + darr[i+1])/2.0; }
				l += (darr[i] + darr[i+1])/2.0;
			}
			
			if(debug && s == 0){ 
				std::cout << "peak_count = " << peak_count << std::endl;
				std::cout << "slope_bin = " << minimum_slope_bin << ", min_slope = " << minimum_slope;
				std::cout << ", sstop = " << sstop << ", lstop = " << first_peak.min << std::endl;
				std::cout << first_peak.print() << std::endl << std::endl;
			}
		} // method == 3
		else if(method == 4){ // Use fast fit guessing (chi^2 version)
			// Set optimized starting values
			double parameters[4];
			parameters[0] = first_peak.max_value*9.211 + 150.484;			// Normalization of pulse
			parameters[1] = (first_peak.left % pulse_size)*1.087 - 2.359;	// Phase (leading edge of pulse) (ns)
			parameters[2] = 1.7750575;										// Decay constant of exponential (ns)
			parameters[3] = 115.64125;										// Width of inverted square guassian (ns^4)

			TF1 *func = new TF1("func",fit_function,0.0,pulse_size,4);
			func->SetParameters(parameters);
			if(debug){ 
				func->SetLineColor(4); func->SetLineWidth(2);
				std::cout << " method = 4, par[0] = " << parameters[0] << ", par[1] = " << parameters[1] << ", par[2] = ";
				std::cout << parameters[2] << ", par[3] = " << parameters[3] << std::endl;
				canvas->Clear();
				graph1->Draw();
				func->Draw("SAME"); 
				canvas->Update();
				canvas->WaitPrimitive();
			}

			func->FixParameter(0, parameters[0]);
			func->FixParameter(1, parameters[1]);
			func->FixParameter(2, parameters[2]);
			func->FixParameter(3, parameters[3]);

			TFitResultPtr func_ptr = graph1->Fit(func,"S Q");
			s = func_ptr->Chi2();
			func->Delete();
		} // method == 4
		else if(method == 5){ // Use root fitting (slow)
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
			parameters[0] = first_peak.max_value*9.211 + 150.484;			// Normalization of pulse
			parameters[1] = (first_peak.left % pulse_size)*1.087 - 2.359;	// Phase (leading edge of pulse) (ns)
			parameters[2] = 1.7750575;										// Decay constant of exponential (ns)
			parameters[3] = 115.64125;										// Width of inverted square guassian (ns^4)

			TF1 *func = new TF1("func",fit_function,0.0,pulse_size,4);
			func->SetParameters(parameters);
			if(debug){ 
				func->SetLineColor(4); func->SetLineWidth(2);
				std::cout << " method = 5, par[0] = " << parameters[0] << ", par[1] = " << parameters[1] << ", par[2] = ";
				std::cout << parameters[2] << ", par[3] = " << parameters[3] << std::endl;
				graph1->Draw();
				func->Draw("SAME");
				canvas->Update();
				canvas->WaitPrimitive(); 
			}

			TFitResultPtr func_ptr = graph->Fit(func,"S Q");
			s = func_ptr->Chi2();
			func->Delete();
		} // method == 5
		if(debug){ std::cout << " s = " << s << ", l = " << l << std::endl; }
		s_int.push_back(s);
		l_int.push_back(l);
	} // Over num_pulses
	
	if(debug){ graph1->Delete(); graph2->Delete(); }
	delete[] darr, x1, x2, x_val1, y_val1, x_val2, y_val2;
	return num_pulses;
}

// For compilation
int main(int argc, char* argv[]){
	if(argc < 5){
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
	if(method > 5){
		std::cout << " Encountered undefined method (" << method << "), aborting\n";
		return 1;
	}
	
	bool debug = false;
	bool use_cut = false;
	if(argc > 5){
		if(strcmp(argv[5], "debug") == 0){ 
			debug = true; 
			std::cout << " DEBUGGING...\n";
		}
		else if(strcmp(argv[5], "cut") == 0){ use_cut = true; }
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
	tree->SetBranchAddress((branch_name.str()+"_mult").c_str(), &mult, &b_mult);
	
	if(!b_wave){
		std::cout << " Failed to load the input branch '" << branch_name.str() << "_wave'\n";
		file->Close();
		return 1;
	}
	if(!b_energy){
		std::cout << " Failed to load the input branch '" << branch_name.str() << "_energy'\n";
		file->Close();
		return 1;
	}
	if(!b_mult){
		std::cout << " Failed to load the input branch '" << branch_name.str() << "_mult'\n";
		file->Close();
		return 1;
	}

	double short_value, long_value, energy_value;
	TFile *out_file = NULL, *cut_file = NULL;
	TTree *out_tree = NULL, *cut_tree = NULL;
	TCutG *cut = NULL;
	if(!debug){
		if(use_cut){
			std::cout << " Loading the cut\n";
			cut_file = new TFile("cuts.root", "READ");
			if(!cut_file->IsZombie()){ 
				cut = (TCutG*)cut_file->Get("cut"); 
				if(!cut){
					std::cout << " Failed to load the cut 'cut'\n";
					use_cut = false; 
					cut_file->Close();
				}
			}
			else{ 
				std::cout << " Failed to open the cut file 'cuts.root'\n";
				use_cut = false; 
				cut_file->Close();
			}
		}

		std::cout << " Opening output file 'analyzer2.root'\n";
		out_file = new TFile("analyzer.root", "RECREATE");
		
		// Standard tree
		out_tree = new TTree(argv[2], "Pulse analysis tree");
		out_tree->Branch((branch_name.str() + "_short").c_str(), &short_value);
		out_tree->Branch((branch_name.str() + "_long").c_str(), &long_value);
		out_tree->Branch((branch_name.str() + "_energy").c_str(), &energy_value);
		
		// Cut tree
		if(use_cut){
			cut_tree = new TTree("gated", "Pulse analysis tree (gated)");
			cut_tree->Branch("cut_short", &short_value);
			cut_tree->Branch("cut_long", &long_value);
			cut_tree->Branch("cut_energy", &energy_value);
		}
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
	if(method == 0 || method == 1 || method == 3){
		hist = new TH2D("hist", "Long vs. Short", 300, 0, 3000, 300, 0, 3000);
		hist->GetXaxis()->SetTitle("Short (arb. units)");
		hist->GetYaxis()->SetTitle("Long (arb. units)");
	}
	else if(method == 2){
		hist = new TH2D("hist", "Fit RMS vs. Real RMS", 250, 0, 500, 250, 0, 500);
		hist->GetXaxis()->SetTitle("Fit RMS (arb. units)");
		hist->GetYaxis()->SetTitle("Real RMS (arb. units)");
	}
	else if(method == 4){
		hist = new TH2D("hist", "Chi^2 vs. Energy", 200, 0, 2000.0, 200, 0, 5000);
		hist->GetXaxis()->SetTitle("Chi^2 (arb. units)");
		hist->GetYaxis()->SetTitle("Energy (arb. units)");
	}
	else if(method == 5){
		hist = new TH2D("hist", "Chi^2 vs. Energy", 200, 0, 1000.0, 200, 0, 5000);
		hist->GetXaxis()->SetTitle("Chi^2 (arb. units)");
		hist->GetYaxis()->SetTitle("Energy (arb. units)");
	}
	hist->SetStats(false);

	std::vector<double> short_integral, long_integral;
	std::vector<double>::iterator iter1, iter2, iter3;
	unsigned int count = 0;
	
	long time_holder1;
	double time_holder2;
	clock_t cpu_time;
	time_t real_time;
	
	unsigned int num_entries = tree->GetEntries();
	if(debug){ num_entries = 100; }
	
	std::cout << " Processing " << num_entries << " entries\n";
	for(unsigned int i = 0; i < num_entries; i++){
		tree->GetEntry(i);
		if(i % 100000 == 0){ 
			time_holder1 = (long)(clock()-cpu_time);
			time_holder2 = difftime(time(NULL), real_time);
			cpu_time = clock();
			time(&real_time);
			if(i != 0){ 
				if(time_holder2 < 1){ time_holder2 = 1; } // Prevent zero time remaining
				std::cout << " Entry no. " << i << ", CPU = " << ((float)time_holder1)/CLOCKS_PER_SEC << " s, REAL = " << time_holder2 << " s, eta = ";
				std::cout << ((float)(num_entries-i)/100000)*time_holder2 << " s\n";
			}
		}
		count += integrate(wave, wave_size, short_integral, long_integral, method, debug, can);
		for(iter1 = short_integral.begin(), iter2 = long_integral.begin(), iter3 = energy.begin(); 
		  iter1 != short_integral.end() && iter2 != long_integral.end() && iter3 != energy.end(); iter1++, iter2++, iter3++){
			short_value = (*iter1);
			long_value = (*iter2);
			energy_value = (*iter3);
			if(!debug){
				out_tree->Fill();
				if(use_cut){
					if(cut->IsInside(short_value, long_value)){
						cut_tree->Fill(); 
					}
				}
				else{
					if(method < 4){ hist->Fill(short_value, long_value); }
					else{ hist->Fill(short_value, energy_value); }
				}
			}
			else{
				if(method < 4){ hist->Fill(short_value, long_value); }
				else{ hist->Fill(short_value, energy_value); }
			}
		}
	}
	
	std::cout << " Found " << count << " pulses in " << num_entries << " tree entries\n";
	if(!debug){ 
		std::cout << " Wrote " << out_tree->GetEntries() << " entries to the output tree\n";
		if(use_cut){ std::cout << " Wrote " << cut_tree->GetEntries() << " entries to the gated tree\n"; }
	}

	hist->Draw("COLZ");
	can->Update();
	can->WaitPrimitive();

	if(!debug){
		out_file->cd();
		out_tree->Write();
		if(use_cut){ 
			cut_tree->Write(); 
			cut_file->Close();
		}
		hist->Write();
		out_file->Close();
	}
	
	can->Close();
	file->Close();
		
	return 0;
}

// For CINT
//int PulseAnalyzer(int argc, char* argv[]){ return main(argc, argv); }
