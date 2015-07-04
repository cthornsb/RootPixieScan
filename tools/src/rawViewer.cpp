// rawViewer.cpp
// C. Thornsberry
// June 4th, 2015
// Load raw pixie data branches and view their contents
// SYNTAX: ./rawViewer [filename] [mod] [chan]

#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TBranch.h"
#include "TSystem.h"
#include "TApplication.h"

#include <sstream>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#define NUM_PIXIE_MOD 12
#define NUM_CHAN_PER_MOD 16

// For compilation
int main(int argc, char* argv[]){
	if(argc < 4){
		std::cout << " Error: Invalid number of arguments to " << argv[0] << ". Expected 3, received " << argc-1 << ".\n";
		std::cout << "  SYNTAX: " << argv[0] << " [filename] [mod] [chan]\n";
		return 1;
	}
	
	int mod = atoi(argv[2]);
	int chan = atoi(argv[3]);
	
	if(!(mod >= 0 && mod < NUM_PIXIE_MOD)){ 
		std::cout << " Error: Invalid module id = " << mod << " (max is " << NUM_PIXIE_MOD-1 << ")!\n";
		return 1; 
	}
	if(!(chan >= 0 && chan < NUM_CHAN_PER_MOD)){ 
		std::cout << " Error: Invalid channel id = " << chan << " (max is " << NUM_CHAN_PER_MOD-1 << ")!\n";
		return 1; 
	}
	
	std::cout << " Searching for module = " << mod << ", channel = " << chan << std::endl;
	std::stringstream stream1; stream1 << "[" << NUM_PIXIE_MOD << "][" << NUM_CHAN_PER_MOD << "]";
	std::stringstream stream2; stream2 << "[" << mod << "][" << chan << "]";
	std::string energy_branch_name = "raw_energy" + stream1.str(); std::string energy_title = "raw_energy" + stream2.str();
	std::string time_branch_name = "raw_time" + stream1.str(); std::string time_title = "raw_time" + stream2.str();

	// Variables for root graphics
	char* dummy[0]; 
	TApplication* rootapp = new TApplication("rootapp",0,dummy);
	gSystem->Load("libTree");
	
	// Branch variables
	std::vector<double> energy[NUM_PIXIE_MOD][NUM_CHAN_PER_MOD];
	std::vector<double> time[NUM_PIXIE_MOD][NUM_CHAN_PER_MOD];
		
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
	
	TBranch *b_energy, *b_time;
	tree->SetBranchAddress(energy_branch_name.c_str(), &energy, &b_energy);
	tree->SetBranchAddress(time_branch_name.c_str(), &time, &b_time);
	
	if(!b_energy){
		std::cout << " Failed to load the input branch '" << energy_branch_name << "'\n";
		file->Close();
		return 1;
	}
	if(!b_time){
		std::cout << " Failed to load the input branch '" << time_branch_name << "'\n";
		file->Close();
		return 1;
	}

	TCanvas *can = new TCanvas("can", "canvas");
	can->Divide(2,1);
	can->cd(1);

	std::vector<double>::iterator energy_iter, time_iter;
	TH1D *h1 = new TH1D("h1", energy_title.c_str(), 1000, 0, 10000);
	TH1D *h2 = new TH1D("h2", time_title.c_str(), 1000, 0, 10000);

	std::cout << " Processing " << tree->GetEntries() << " entries\n";
	for(unsigned int i = 0; i < tree->GetEntries(); i++){
		tree->GetEntry(i);
		if(i % 100000 == 0 && i != 0){ std::cout << " Entry no. " << i << std::endl; }
		for(energy_iter = energy[mod][chan].begin(), time_iter = time[mod][chan].begin(); energy_iter != energy[mod][chan].end() && time_iter != time[mod][chan].end(); energy_iter++, time_iter++){
			h1->Fill(*energy_iter);
			h2->Fill(*time_iter);
		}
	}

	h1->Draw();
	can->cd(2);
	h2->Draw();
	can->WaitPrimitive();

	can->Close();
	file->Close();
	rootapp->Delete();
	
	return 0;
}
