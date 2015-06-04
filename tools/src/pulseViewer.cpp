// PulseViewer.cpp
// C. Thornsberry
// Aug. 25th, 2014
// Load detector pulses and view them in sequence
// SYNTAX: ./pulseViewer [filename] [branchname] <skip>

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TBranch.h"
#include "TGraph.h"
#include "TSystem.h"
#include "TApplication.h"

#include <sstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

double PIXIE_TIME_RES = 4.0; // In ns

// For compilation
int main(int argc, char* argv[]){
	if(argc < 4){
		std::cout << " Error! Invalid number of arguments. Expected 3, received " << argc-1 << "\n";
		std::cout << "  SYNTAX: " << argv[0] << " [filename] [branchname] <skip>\n";
		return 1;
	}
	
	// Variables for root graphics
	char* dummy[0]; 
	TApplication* rootapp = new TApplication("rootapp",0,dummy);
	gSystem->Load("libTree");
	
	unsigned int skip = 0;
	if(argc <= 4){
		skip = atoi(argv[3]);
		if(skip < 0){ 
			std::cout << " Warning: Invalid number of entries to skip (" << skip << ")\n";
			skip = 0; 
		}
		std::cout << " Skipping " << skip << " pulses\n";
	}

	// Branch variables
	std::vector<int> wave;
	std::vector<double> energy;
	unsigned int mult;
	unsigned int wave_size = 0;
		
	TFile *file = new TFile(argv[1], "READ");
	if(!file->IsOpen()){
		std::cout << " Failed to load the input file '" << argv[1] << "'\n";
		return 1;
	}
	TTree *tree = (TTree*)file->Get("Pixie16");
	if(!tree){
		std::cout << " Failed to load the input tree 'Pixie16'\n";
		file->Close();
		return 1;
	}
	tree->SetMakeClass(1);
	
	std::stringstream branch_name;
	branch_name << argv[2];
	TBranch *b_wave, *b_mult;
	tree->SetBranchAddress((branch_name.str()+"_wave").c_str(), &wave, &b_wave);
	tree->SetBranchAddress((branch_name.str()+"_mult").c_str(), &mult, &b_mult);
	
	if(!b_wave){
		std::cout << " Failed to load the input branch '" << branch_name.str() << "_wave'\n";
		file->Close();
		return 1;
	}
	if(!b_mult){
		std::cout << " Failed to load the input branch '" << branch_name.str() << "_mult'\n";
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

	TCanvas *can = new TCanvas("can", "canvas");
	can->cd();

	// Variables for TGraph
	double *x = new double[wave_size];
	const double *x_val = new double[wave_size];
	const double *y_val = new double[wave_size];
	
	TGraph *graph = new TGraph(wave_size, x_val, y_val);
	for(unsigned int i = 0; i < wave_size; i++){
		x[i] = i*PIXIE_TIME_RES;
	}

	unsigned int count = 0, index = 0;
	std::cout << " Processing " << tree->GetEntries() << " entries\n";
	for(unsigned int i = 0; i < tree->GetEntries(); i++){
		tree->GetEntry(i);
		if(i % 100000 == 0 && i != 0){ std::cout << " Entry no. " << i << std::endl; }
		if(mult > 0 && count >= skip){
			count = 0;
			index = 0;
			std::cout << "  " << i << std::endl;
			for(std::vector<int>::iterator iter = wave.begin(); iter != wave.end(); iter++){
				if(index >= wave_size){ break; } // Reached maximum graph container size
				graph->SetPoint(index, x[index], (*iter));
				index++;
			}
			graph->Draw();
			can->Update();
			sleep(1);
		}
		else{ count += mult; }
	}

	can->Close();
	file->Close();
	graph->Delete();
	
	rootapp->Delete();
	
	return 0;
}

// For CINT
int Viewer(int argc, char* argv[]){ return main(argc, argv); }
