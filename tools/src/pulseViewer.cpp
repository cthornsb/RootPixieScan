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

void help(char * prog_name_){
	std::cout << "  SYNTAX: " << prog_name_ << " [filename] [branchname] <options>\n";
	std::cout << "   Available options:\n";
	std::cout << "    --skip <num>       | Skip a number of entries between displaying pulses.\n";
	std::cout << "    --fast-fwd <entry> | Skip a specified number of entries at the beginning of the tree.\n";
}

// For compilation
int main(int argc, char* argv[]){
	if(argc < 3){
		std::cout << " Error: Invalid number of arguments to " << argv[0] << ". Expected 2, received " << argc-1 << ".\n";
		help(argv[0]);
		return 1;
	}

	int skip = 0;
	int start_entry = 0;
	int index = 3;
	while(index < argc){
		if(strcmp(argv[index], "--skip") == 0){
			if(index + 1 >= argc){
				std::cout << " Error! Missing required argument to '--skip'!\n";
				help(argv[0]);
				return 1;
			}
			skip = atoi(argv[++index]);
			if(skip < 0){
				std::cout << " Warning: Invalid number of pulses to skip (" << skip << ")!\n";
				skip = 0;
			}
			std::cout << " Skipping " << skip << " pulses\n";
		}
		else if(strcmp(argv[index], "--fast-fwd") == 0){
			if(index + 1 >= argc){
				std::cout << " Error! Missing required argument to '--fast-fwd'!\n";
				help(argv[0]);
				return 1;
			}
			start_entry = atoi(argv[++index]);
			if(skip < 0){
				std::cout << " Warning: Invalid starting entry no. (" << start_entry << ")!\n";
				skip = 0;
			}
			std::cout << " Starting at entry no. " << start_entry << std::endl;
		}
		else{ 
			std::cout << " Error! Unrecognized option '" << argv[index] << "'!\n";
			help(argv[0]);
			return 1;
		}
		index++;
	}

	// Variables for root graphics
	char* dummy[0]; 
	TApplication* rootapp = new TApplication("rootapp",0,dummy);
	gSystem->Load("libTree");
	
	// Branch variables
	std::vector<int> wave;
	std::vector<double> energy;
	int mult;
	int wave_size = 0;
		
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
	for(int i = 0; i < tree->GetEntries(); i++){
		tree->GetEntry(i);
		if(mult == 0 || wave.size() == 0){ continue; }
		else{ 
			wave_size = wave.size()/mult;
			break; 
		}
	}
	if(wave_size == 0){
		std::cout << " Error: wave_size == 0! Aborting!\n";
		file->Close();
		return 1;
	}
	std::cout << " Using wave size " << wave_size << std::endl;

	TCanvas *can = new TCanvas("can", "canvas");
	can->cd();

	// Variables for TGraph
	double *x = new double[wave_size];
	const double *x_val = new double[wave_size];
	const double *y_val = new double[wave_size];
	
	TGraph *graph = new TGraph(wave_size, x_val, y_val);
	for(int i = 0; i < wave_size; i++){
		x[i] = i*PIXIE_TIME_RES;
	}

	if(start_entry >= tree->GetEntries()){ start_entry = tree->GetEntries()-1; }

	int count = 0;
	std::cout << " Processing " << tree->GetEntries() << " entries\n";
	for(int i = start_entry; i < tree->GetEntries(); i++){
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
