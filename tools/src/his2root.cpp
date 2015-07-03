#include <iostream>

#include "TNamed.h"
#include "TFile.h"
#include "TH1I.h"
#include "TH2I.h"

#include "HisFile.h"

int main(int argc, char *argv[]){
	if(argc < 2){
		std::cout << " Error! Invalid number of arguments. Expected 1, received " << argc-1 << "\n";
		std::cout << "  SYNTAX: " << argv[0] << " [prefix] <options>\n";
		return 1;
	}

	bool verbose = true;
	int arg_index = 2;
	while(arg_index < argc){
		if(strcmp(argv[arg_index], "--quiet") == 0){ verbose = false; }
		else{ 
			std::cout << " Error: Encountered unrecognized option '" << argv[arg_index] << "'\n";
			return 1;
		}
		arg_index++;
	}

	std::string output_fname(argv[1]);
	output_fname += ".root";
	
	TFile *file = new TFile(output_fname.c_str(), "CREATE");
	if(!file->IsOpen()){
		std::cout << " Error: Failed to open output file '" << output_fname << "'\n";
		return 1;
	}
	
	file->cd();

	HisFile his_file(argv[1]);
	if(!his_file.IsGood()){
		his_file.GetError();
		file->Close();
		return 1;
	}
	
	if(verbose){
		his_file.PrintHeader();
		std::cout << std::endl;
	}
	
	TNamed *name = new TNamed("date", his_file.GetDate().c_str());
	name->Write();
	name->Delete();
	
	int count = 0;
	while(his_file.GetNextHistogram() > 0){
		if(verbose){ 
			his_file.PrintEntry(); 
			std::cout << std::endl;
		}
		if(his_file.GetDimension() == 1){
			TH1I *h1 = his_file.GetTH1();
			if(h1){
				h1->Write();
				h1->Delete();
			}
			count++;
		}
		else if(his_file.GetDimension() == 2){
			TH2I *h2 = his_file.GetTH2();
			if(h2){
				h2->Write();
				h2->Delete();
			}
			count++;
		}
		else{
			std::cout << " Warning! Unsupported histogram dimension (" << his_file.GetDimension() << ") for id = " << his_file.GetHisID() << std::endl;
		}
	}
	
	std::cout << " Done! Wrote " << count << " histograms to '" << output_fname << "'\n";
	std::streampos rsize = (std::streampos)file->GetSize();
	std::cout << "  Output file size is " << rsize << " bytes, reduction of " << 100*(1-(1.0*rsize)/his_file.GetHisFilesize()) << "%\n";
	
	file->Close();
	
	return 0;
}
