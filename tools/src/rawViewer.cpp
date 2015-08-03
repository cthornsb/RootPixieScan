// rawViewer.cpp
// C. Thornsberry
// June 4th, 2015
// Load raw pixie data branches and view their contents
// SYNTAX: ./rawViewer [filename] [mod] [chan]

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TApplication.h"

#include <sstream>
#include <iostream>

// For compilation
int main(int argc, char* argv[]){
	if(argc < 2){
		std::cout << " Error: Invalid number of arguments to " << argv[0] << ". Expected 3, received " << argc-1 << ".\n";
		std::cout << "  SYNTAX: " << argv[0] << " [filename] <mod> <chan>\n";
		return 1;
	}

	// Variables for root graphics
	char* dummy[0]; 
	TApplication* rootapp = new TApplication("rootapp",0,dummy);
	gSystem->Load("libTree");
	
	int mod = -1, chan = -1;
	if(argc >= 3){ mod = atoi(argv[2]); }
	if(argc >= 4){ chan = atoi(argv[3]); }
	
	std::stringstream stream1, stream2;
	if(mod >= 0){ stream1 << "raw_mod == " << mod; }
	if(chan >= 0){ stream2 << "raw_chan == " << chan; }
	
	std::string selection = "";
	if(!stream1.str().empty()){ selection = stream1.str(); }
	if(!stream2.str().empty()){ 
		if(selection.empty()){
			selection = stream2.str(); 
		}
		else{
			selection += " && " + stream2.str();
		}
	}

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
	
	std::cout << " Searching for module = " << mod << ", channel = " << chan << std::endl;

	TCanvas *can = new TCanvas("can", "canvas");
	can->Divide(2,1);
	
	can->cd(1);
	std::cout << "tree->Draw(\"raw_energy\", \"" << selection << "\", \"\") = " << tree->Draw("raw_energy", selection.c_str(), "") << std::endl;
	can->cd(2);
	std::cout << "tree->Draw(\"raw_time\", \"" << selection << "\", \"\") = " << tree->Draw("raw_time", selection.c_str(), "") << std::endl;
	
	can->WaitPrimitive();

	can->Close();
	file->Close();
	rootapp->Delete();
	
	return 0;
}
