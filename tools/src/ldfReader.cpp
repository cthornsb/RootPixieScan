/** \file ldfReader.cpp
  * 
  * \brief Unpacks run information from Pixie16 ldf files
  * 
  * \author Cory R. Thornsberry
  * 
  * \date April 30th, 2015
  * 
  * \version 1.0
*/

#include <iostream>
#include <fstream>
#include <string.h>

#include "hribf_buffers.h"

std::string GetExtension(const char *filename_, std::string &prefix){
	unsigned int count = 0;
	unsigned int last_index = 0;
	std::string output = "";
	prefix = "";
	
	// Find the final period in the filename
	while(filename_[count] != '\0'){
		if(filename_[count] == '.'){ last_index = count; }
		count++;
	}
	
	// Get the filename prefix and the extension
	for(unsigned int i = 0; i < count; i++){
		if(i < last_index){ prefix += filename_[i]; }
		else if(i > last_index){ output += filename_[i]; }
	}
	
	return output;
}

int main(int argc, char *argv[]){
	if(argc < 2){
		std::cout << " Error: Invalid number of arguments to " << argv[0] << ". Expected 1, received " << argc-1 << ".\n";
		std::cout << "  SYNTAX: " << argv[0] << " [filename]\n";
		return 1;
	}

	std::string prefix;
	std::string extension = GetExtension(argv[1], prefix);
	if(prefix == ""){
		std::cout << " ERROR: Input filename was not specified!\n";
		return 1;
	}

	int file_format;
	if(extension == "ldf"){ // List data format file
		file_format = 0;
	}
	else if(extension == "pld"){ // Pixie list data file format
		file_format = 1;
	}
	else if(extension == "root"){ // Pixie list data file format
		file_format = 2;
	}
	else{
		std::cout << " ERROR: Invalid file format '" << extension << "'\n";
		std::cout << "  The current valid data formats are:\n";
		std::cout << "   ldf - list data format (HRIBF)\n";
		std::cout << "   pld - pixie list data format\n";
		std::cout << "   root - root file containing raw pixie data\n";
		return 1;
	}
	
	PLD_header pldHead;
	PLD_data pldData;
	DIR_buffer dirbuff;
	HEAD_buffer headbuff;
	DATA_buffer databuff;
	EOF_buffer eofbuff;

	if(argc >= 3 && strcmp(argv[2], "debug") == 0){ 
		dirbuff.SetDebugMode();
		headbuff.SetDebugMode();
		databuff.SetDebugMode();
		eofbuff.SetDebugMode();
	}

	std::ifstream input_file(argv[1], std::ios::binary);
	if(!input_file.is_open() || !input_file.good()){
		std::cout << " Failed to open input file '" << argv[1] << "'! Check that the path is correct.\n";
		input_file.close();
		return 1;
	}
	
	if(file_format == 0){
		int num_buffers;
		dirbuff.Read(&input_file, num_buffers);
		headbuff.Read(&input_file);
		
		// Let's read out the file information from these buffers
		std::cout << "\n 'DIR ' buffer-\n";
		std::cout << "  Run number: " << dirbuff.GetRunNumber() << std::endl;
		std::cout << "  Number buffers: " << num_buffers << std::endl << std::endl;

		std::cout << " 'HEAD' buffer-\n";
		std::cout << "  Facility: " << headbuff.GetFacility() << std::endl;
		std::cout << "  Format: " << headbuff.GetFormat() << std::endl;
		std::cout << "  Type: " << headbuff.GetType() << std::endl;
		std::cout << "  Date: " << headbuff.GetDate() << std::endl;
		std::cout << "  Title: " << headbuff.GetRunTitle() << std::endl;
		std::cout << "  Run number: " << headbuff.GetRunNumber() << std::endl << std::endl;
	}
	else if(file_format == 1){
		pldHead.Read(&input_file);
		
		// Let's read out the file information from these buffers
		std::cout << " 'HEAD' buffer-\n";
		std::cout << "  Facility: " << pldHead.GetFacility() << std::endl;
		std::cout << "  Format: " << pldHead.GetFormat() << std::endl;
		std::cout << "  Start: " << pldHead.GetStartDate() << std::endl;
		std::cout << "  Stop: " << pldHead.GetEndDate() << std::endl; 
		std::cout << "  Title: " << pldHead.GetRunTitle() << std::endl;
		std::cout << "  Run number: " << pldHead.GetRunNumber() << std::endl;
		std::cout << "  Max spill: " << pldHead.GetMaxSpillSize() << " words\n";
		std::cout << "  ACQ Time: " << pldHead.GetRunTime() << " seconds\n\n";
	}
	else if(file_format == 2){
	}
	
	input_file.close();	

	return 0;
}
