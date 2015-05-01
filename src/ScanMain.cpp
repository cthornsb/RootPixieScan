
#include <iostream>
#include <string.h>

#include "NewPixieStd.h"
#include "hribf_buffers.h"

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
	
	// Start reading the file
	// Every poll2 ldf file starts with a DIR buffer followed by a HEAD buffer
	int num_buffers;
	dirbuff.Read(&input_file, num_buffers);
	headbuff.Read(&input_file);
	
	// Let's read out the file information from these buffers
	std::cout << " 'DIR ' buffer-\n";
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
	unsigned int nWords;
	while(databuff.Read(&input_file, data, nWords, full_spill)){ 
		if(full_spill){ 
			std::cout << " Retrieved spill of " << nWords << " words (" << nWords*4 << " bytes)\n"; 
			ReadSpill(data, nWords);
		}
		else{ std::cout << " Retrieved spill fragment of " << nWords << " words (" << nWords*4 << " bytes)\n"; }
	}
	
	if(eofbuff.Read(&input_file) && eofbuff.Read(&input_file)){
		std::cout << " Encountered double EOF buffer.\n";
	}
	else{
		std::cout << " Failed to reach end of file!\n";
	}

	input_file.close();	

	return 0;
}
