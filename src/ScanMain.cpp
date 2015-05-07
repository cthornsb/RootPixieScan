
#include <iostream>
#include <string.h>

#include "MapFile.hpp"
#include "NewPixieStd.hpp"
#include "hribf_buffers.h"
#include "poll2_socket.h"

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
	if(argc < 2 || (argc >= 2 && strcmp(argv[1], "help") == 0)){
		if(argc < 2){ std::cout << " Invalid number of arguments to " << argv[0] << std::endl; }
		std::cout << "  SYNTAX: " << argv[0] << " <filename> <options>\n\n";
		std::cout << "  Available options:\n";
		std::cout << "   --debug      - Enable readout debug mode\n";
		std::cout << "   --shm        - Enable shared memory readout\n";
		std::cout << "   --force-ldf  - Force use of ldf readout\n";
		std::cout << "   --force-pld  - Force use of pld readout\n";
		std::cout << "   --force-root - Force use of root readout\n\n";
		return 1;
	}

	DIR_buffer dirbuff;
	HEAD_buffer headbuff;
	DATA_buffer databuff;
	EOF_buffer eofbuff;

	std::string prefix, extension;

	int arg_index = 1;
	bool debug_mode = false;
	bool shm_mode = false;
	Server poll_server;
	
	int file_format = -1;
	while(arg_index < argc){
		if(argv[arg_index][0] != '\0' && argv[arg_index][0] != '-'){ // This must be a filename
			extension = GetExtension(argv[arg_index], prefix);
		}
	
		if(strcmp(argv[arg_index], "--debug") == 0){ 
			std::cout << " NOTE: Using DEBUG mode\n";
			debug_mode = true;
			
			dirbuff.SetDebugMode();
			headbuff.SetDebugMode();
			databuff.SetDebugMode();
			eofbuff.SetDebugMode();
		}
		else if(strcmp(argv[arg_index], "--shm") == 0){ 
			std::cout << " NOTE: Using SHM mode\n";
			shm_mode = true;
		}
		else if(strcmp(argv[arg_index], "--force-ldf") == 0){ 
			std::cout << " NOTE: Forcing ldf file readout.\n";
			file_format = 0;
		}
		else if(strcmp(argv[arg_index], "--force-pld") == 0){ 
			std::cout << " NOTE: Forcing pld file readout.\n";
			file_format = 1;
		}
		else if(strcmp(argv[arg_index], "--force-root") == 0){ 
			std::cout << " NOTE: Forcing root file readout.\n";
			file_format = 2;
		}
		else{ std::cout << " WARNING: Unrecognized option '" << argv[arg_index] << "'\n"; }
		arg_index++;
	}

	if(!shm_mode && file_format == -1){
		if(extension == "ldf"){ // List data format file
			std::cout << " NOTE: Using readout for ldf (list data format) file.\n";
			file_format = 0;
		}
		else if(extension == "pld"){ // Pixie list data file format
			std::cout << " NOTE: Using readout for pld (pixie list data) file.\n";
			file_format = 1;
		}
		else if(extension == "root"){ // Pixie list data file format
			std::cout << " NOTE: Using readout for root file format.\n";
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
	}
	
	std::ifstream input_file;
	if(!shm_mode){
		input_file.open(argv[1], std::ios::binary);
		if(!input_file.is_open() || !input_file.good()){
			std::cout << " ERROR: Failed to open input file '" << argv[1] << "'! Check that the path is correct.\n";
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
	}
	else{
		if(!poll_server.Init(5555)){
			std::cout << " ERROR: Failed to open shm socket 5555!\n";
			return 1;
		}
		else{ std::cout << " NOTE: Listening on poll2 SHM port 5555\n"; }
	}
	
	// Load the map file
	MapFile theMapFile;

	// Now we're ready to read the first data buffer
	char data[1000000];
	bool full_spill;
	unsigned int nBytes;
	if(!shm_mode){
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
	}
	else{
		std::cout << std::endl;
		while(true){
			std::cout << "\r[IDLE] Waiting for a spill..." << std::flush;
			nBytes = poll_server.RecvMessage(data, 1000000); // Read from the socket
			
			if(strcmp(data, "$KILL_SOCKET") == 0){
				std::cout << "  Received KILL_SOCKET flag...\n\n";
				break;
			}
			
			if(debug_mode){ std::cout << " Retrieved spill of " << nBytes << " bytes (" << nBytes/4 << " words)\n"; }
			ReadSpill(data, nBytes/4);
		}
	}

	input_file.close();	

	cleanup(); // Clean up detector driver
	
	// Close the shm socket if it's open
	poll_server.Close();

	return 0;
}
