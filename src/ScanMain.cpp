
#include <iostream>
#include <string.h>

#include "MapFile.hpp"
#include "NewPixieStd.hpp"
#include "hribf_buffers.h"

// Everything in this code is global, so putting this here is perfectly fine :-P
MapFile theMapFile;

int main(int argc, char *argv[]){
	if(argc < 2){
		std::cout << " Invalid number of arguments to " << argv[0] << std::endl;
		std::cout << "  SYNTAX: " << argv[0] << " [filename]\n";
		return 1;
	}
	
	DIR_buffer dirbuff;
	HEAD_buffer headbuff;
	DATA_buffer databuff;
	EOF_buffer eofbuff;

	bool debug_mode = false;
	if(argc >= 3 && strcmp(argv[2], "debug") == 0){ 
		debug_mode = true;
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
	
	// Start reading the file
	// Every poll2 ldf file starts with a DIR buffer followed by a HEAD buffer
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
	
	// Now we're ready to read the first data buffer
	char data[1000000];
	bool full_spill;
	unsigned int nBytes;
	while(databuff.Read(&input_file, data, nBytes, 1000000, full_spill)){ 
		if(full_spill){ 
			if(debug_mode){ std::cout << " Retrieved spill of " << nBytes << " bytes (" << nBytes/4 << " words)\n"; }
			ReadSpill(data, nBytes/4);
		}
		else if(debug_mode){ std::cout << " Retrieved spill fragment of " << nBytes << " bytes (" << nBytes/4 << " words)\n"; }
	}
	
	if(eofbuff.Read(&input_file) && eofbuff.Read(&input_file)){
		std::cout << " Encountered double EOF buffer.\n";
	}
	else{
		std::cout << " Failed to reach end of file!\n";
	}

	input_file.close();	

	cleanup(); // Clean up detector driver

	return 0;
}
