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

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

unsigned int N_PEAKS = 2;
double PIXIE_TIME_RES = 4.0; // In ns

struct pair{
	unsigned int bin;
	double value;
	
	pair(unsigned int bin_, double value_){
		bin = bin_;
		value = value_;
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
unsigned int integrate(std::vector<int> &arr, unsigned int pulse_size, double &s, double &l, unsigned short method=0){
	unsigned int size = arr.size();
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
	
	unsigned int start, stop;
	std::vector<pair> peaks;
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
	
		// Find the maximum
		double maximum = -9999.0;
		unsigned int max_bin = 0;
		for(unsigned int i = start; i < stop; i++){
			if(darr[i] > maximum){ 
				maximum = darr[i];
				max_bin = i;
			}
		}

		// Find peaks (there could be more than 1 pulse)
		unsigned int num_peaks = 0, leading_edge = 0;
		double back_sub_left = 0.0;
		double back_sub_right = 0.0;
		double slope;
		bool up_slope = false;
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
					peaks.push_back(pair(leading_edge, darr[i]));
				}
			}
			else{ // Not on a positive slope
				if(slope > 0.0){ // Found a pulse
					up_slope = true; 
					leading_edge = i;
				}
			}
		}

		// Find 80% of maximum
		unsigned int cfd_bin = 0;
		for(unsigned int i = start; i < stop; i++){
			if(darr[i] > 0.8*maximum){
				if(i > 0){ cfd_bin = i-1; }
				else{ cfd_bin = i; }
				break;
			}
		}

		// Do short and long integrals
		s = 0.0; l = 0.0;
		if(method == 0){ // Slope inversion
			// To the right of the maximum, we expect a negative slope
			// Stop short integral when the slope goes non-negative
			bool calc_short = true;
			for(unsigned int i = cfd_bin; i < stop-1; i++){
				// Integrate using trapezoid rule
				if(i < max_bin){
					s += PIXIE_TIME_RES*(darr[i] + darr[i+1])/2.0;
					l += PIXIE_TIME_RES*(darr[i] + darr[i+1])/2.0;
				}
				else{
					if(calc_short){ // Waiting on positive slope to stop calculating short integral
						if(darr[i+1]-darr[i] <= 0.0){ // Negative slope, still on side of pulse
							s += PIXIE_TIME_RES*(darr[i] + darr[i+1])/2.0;
							l += PIXIE_TIME_RES*(darr[i] + darr[i+1])/2.0;
						}
						else{ 
							l += PIXIE_TIME_RES*(darr[i] + darr[i+1])/2.0;
							calc_short = false; 
						}
					}
					else{ // Encountered positive slope, only calculating long integral
						if(darr[i] <= 0.0){ break; } // Bottom of pulse, stop calculating long integral
						l += PIXIE_TIME_RES*(darr[i] + darr[i+1])/2.0;
					}
				}
			}
		}
		else if(method == 1){ // Fixed pulse height
			// Stop short integral when we reach 10% of the pulse height
			// Find the 10% of maximum point
			unsigned int ten_percent_bin = 0;
			for(unsigned int i = max_bin; i < stop; i++){
				if(darr[i] <= 0.1*maximum){ 
					ten_percent_bin = i;
					break;
				}
			}
			for(unsigned int i = cfd_bin; i < stop-1; i++){
				// Integrate using trapezoid rule with arbitrary bin width of 1
				if(i < ten_percent_bin){ s += PIXIE_TIME_RES*(darr[i] + darr[i+1])/2.0; } // Stop calculating short below 10% maximum
				if(darr[i] <= 0.0){ break; } // Bottom of pulse, stop calculating long integral
				l += PIXIE_TIME_RES*(darr[i] + darr[i+1])/2.0;
			}
		}
		else if(method == 2){ // Use fast fit guessing
			if(peaks.size() > 1){
				delete[] darr;
				return 0;
			}
			
			// Set optimized starting values
			double parameters[4];
			parameters[0] = peaks[0].value*9.211 + 150.484;	// Normalization of pulse
			parameters[1] = peaks[0].bin*1.087 - 2.359;		// Phase (leading edge of pulse) (ns)
			parameters[2] = 1.7750575;						// Decay constant of exponential (ns)
			parameters[3] = 115.64125; 						// Width of inverted square guassian (ns^4)

			s = chi_2(darr, start, stop, parameters);
		}
		else if(method == 3){ // Use root fitting (slow)
			if(peaks.size() > 1){
				delete[] darr;
				return 0;
			}
			
			// Set optimized starting values
			double parameters[4];
			parameters[0] = peaks[0].value*9.211 + 150.484;	// Normalization of pulse
			parameters[1] = peaks[0].bin*1.087 - 2.359;		// Phase (leading edge of pulse) (ns)
			parameters[2] = 1.7750575;						// Decay constant of exponential (ns)
			parameters[3] = 115.64125; 						// Width of inverted square guassian (ns^4)

			TF1 *func = new TF1("func",fit_function,0.0,pulse_size,4);
			func->SetParameters(parameters);

			TFitResultPtr func_ptr = graph->Fit(func,"S Q");
			s = func_ptr->Chi2();
			func->Delete();
		}
	}
	
	delete[] darr;
	return peaks.size();
}

// For testing
int main2(){
	double parameters[4];
	parameters[0] = 200*9.211 + 150.484;	// Normalization of pulse
	parameters[1] = 21*1.087 - 2.359;		// Phase (leading edge of pulse) (ns)
	parameters[2] = 1.7750575;						// Decay constant of exponential (ns)
	parameters[3] = 115.64125; 						// Width of inverted square guassian (ns^4)
	
	double data[62];
	double value[1];
	for(unsigned int i = 0; i < 62; i++){
		value[0] = (double)i;
		data[i] = fit_function(value, parameters);
	}
	
	std::cout << " chi^2: " << chi_2(data, 0, 62, parameters) << std::endl;
}

// For compilation
int main(int argc, char* argv[]){
	if(argc < 4){
		std::cout << " Missing required argument, aborting\n";
		std::cout << "  SYNTAX: Viewer {filename treename wave_branch wave_size}\n";
		return 1;
	}
	
	// Variables for root graphics
	char* dummy[0]; 
	TApplication* rootapp = new TApplication("rootapp",0,dummy);
	gSystem->Load("libTree");
	
	unsigned int wave_size = atol(argv[4]);
	std::cout << " Using wave array size of " << wave_size << std::endl;

	// Waveform array
	std::vector<int> wave;
	std::vector<double> energy;

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
	
	TBranch *b_wave, *b_energy;
	tree->SetBranchAddress(argv[3], &wave, &b_wave);
	tree->SetBranchAddress("trigger_energy", &energy, &b_energy);
	
	if(!b_wave){
		std::cout << " Failed to load the input branch '" << argv[3] << "'\n";
		file->Close();
		return 1;
	}

	// Check pulse size verses vector size
	for(unsigned int i = 0; i < tree->GetEntries(); i++){
		tree->GetEntry(i);
		if(wave.size() == 0){ continue; }
		else if(wave.size() % wave_size != 0){ 
			std::cout << " Warning! Pulse size (" << wave_size << ") does not correspond to the number of entries in vector (";
			std::cout << wave.size() << "), aborting!\n";
			file->Close();
			return 1;
		}
		else{ break; }
	}

	// Canvas
	TCanvas *can = new TCanvas("can", "canvas");
	can->cd();

	// Histogram
	TH2D *hist = new TH2D("hist", "Chi^2 vs. Energy", 200, 0, 100.0, 200, 0, 5000);
	//TH2D *hist = new TH2D("hist", "Long vs. Short", 500, 0, 1000, 500, 0, 5000);
	//TH1D *hist = new TH1D("hist", "Chi^2", 500, 0, 1000);
	//hist->GetXaxis()->SetTitle("Short (arb. units)");
	//hist->GetYaxis()->SetTitle("Long (arb. units)");
	hist->GetXaxis()->SetTitle("Chi^2 (arb. units)");
	hist->GetYaxis()->SetTitle("Energy (arb. units)");
	hist->SetStats(false);

	double short_integral, long_integral;
	unsigned int count = 0;
	std::cout << " Processing " << tree->GetEntries() << " entries\n";
	//for(unsigned int i = 0; i < tree->GetEntries(); i++){
	for(unsigned int i = 0; i < 100; i++){
		tree->GetEntry(i);
		if(i % 100000 == 0){ std::cout << " Entry no. " << i << std::endl; }
		count += integrate(wave, wave_size, short_integral, long_integral, 2);
		//std::cout << short_integral << std::endl;
		if(energy.size() > 0 && short_integral > 0.0){ hist->Fill(short_integral, energy.at(0)); }
		//hist->Fill(short_integral, long_integral);
		//hist->Fill(short_integral);
	}
	
	std::cout << " Found " << count << " pulses in " << tree->GetEntries() << " tree entries\n";
	
	hist->Draw("COLZ");
	can->Update();
	can->WaitPrimitive();

	can->Close();
	file->Close();
		
	return 0;
}

// For CINT
//int PulseAnalyzer(int argc, char* argv[]){ return main(argc, argv); }
