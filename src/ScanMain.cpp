
#include <iostream>
#include <string.h>
#include <thread>

#include "MapFile.hpp"
#include "NewPixieStd.hpp"
#include "hribf_buffers.h"
#include "poll2_socket.h"
#include "CTerminal.h"

#define SCAN_VERSION "1.1.01"

std::string prefix, extension;

bool debug_mode;
bool shm_mode;

bool kill_all = false;
bool scan_running = false;
bool run_ctrl_exit = false;

Server poll_server;

std::ifstream input_file;

DIR_buffer dirbuff;
HEAD_buffer headbuff;
DATA_buffer databuff;
EOF_buffer eofbuff;

std::string sys_message_head = "PixieLDF: ";

/* Print help dialogue for command line options. */
void help(){
	std::cout << "\n SYNTAX: ./poll2 [options]\n";
	std::cout << "  -a, --alarm=[e-mail] Call the alarm script with a given e-mail (or no argument)\n"; 
	std::cout << "  -f, --fast           Fast boot (false by default)\n";
	std::cout << "  -h, --hist <num>     Dump histogram data every num seconds\n";
	std::cout << "  -q, --quiet          Run quietly (false by default)\n";
	std::cout << "  -n, --no-wall-clock  Do not insert the wall clock in the data stream\n";
	std::cout << "  -r, --rates          Display module rates in quiet mode (false by defualt)\n";
	std::cout << "  -s, --stats <num>    Output statistics data every num seconds\n";
	std::cout << "  -t, --thresh <num>   Sets FIFO read threshold to num% full (50% by default)\n";
	std::cout << "  -z, --zero           Zero clocks on each START_ACQ (false by default)\n";
	std::cout << "  -d, --debug          Set debug mode to true (false by default)\n";
	std::cout << "  -m, --memory-share   Do not write data to shared memory (SHM) (true by default)\n\n";
}	
	
void start_run_control(){
	if(debug_mode){
		dirbuff.SetDebugMode();
		headbuff.SetDebugMode();
		databuff.SetDebugMode();
		eofbuff.SetDebugMode();
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
		char shm_data[40008]; // Array to store the temporary shm data (~40 kB)
		int dummy;
		int previous_chunk;
		int current_chunk;
		int total_chunks;
		unsigned int nTotalBytes;
		
		while(true){
			previous_chunk = 0;
			current_chunk = 0;
			total_chunks = -1;
			nTotalBytes = 0;
			
			while(current_chunk != total_chunks){
				if(kill_all == true){ 
					run_ctrl_exit = true;
					return;
				}

				std::cout << "[IDLE] Waiting for a spill...\n";// << std::flush;
				if(!poll_server.Select(dummy)){ continue; }
			
				nBytes = poll_server.RecvMessage(shm_data, 40008); // Read from the socket
				if(strcmp(shm_data, "$CLOSE_FILE") == 0 || strcmp(shm_data, "$OPEN_FILE") == 0 || strcmp(shm_data, "$KILL_SOCKET") == 0){ continue; } // Poll2 network flags
				else if(nBytes < 8){ continue; } // Did not read enough bytes

				if(debug_mode){ std::cout << "debug: Received " << nBytes << " bytes from the network\n"; }
				memcpy((char *)&current_chunk, &shm_data[0], 4);
				memcpy((char *)&total_chunks, &shm_data[4], 4);
			
				if(previous_chunk == -1 && current_chunk != 1){ // Started reading in the middle of a spill, ignore the rest of it
					if(debug_mode){ std::cout << "debug: Skipping chunk " << current_chunk << " of " << total_chunks << std::endl; }
					continue;
				}
				else if(previous_chunk != current_chunk - 1){ // We missed a spill chunk somewhere
					if(debug_mode){ std::cout << "debug: Found chunk " << current_chunk << " but expected chunk " << previous_chunk+1 << std::endl; }
					break;
				}

				previous_chunk = current_chunk;
			
				// Copy the shm spill chunk into the data array
				if(nTotalBytes + 2 + nBytes <= 1000000){ // This spill chunk will fit into the data buffer
					memcpy(&data[nTotalBytes], &shm_data[8], nBytes - 8);
					nTotalBytes += (nBytes - 8);				
				}
				else{ 
					if(debug_mode){ std::cout << "debug: Abnormally full spill buffer with " << nTotalBytes + 2 + nBytes << " bytes!\n"; }
					break; 
				}
			}
			
			int word1 = 2, word2 = 9999;
			memcpy(&data[nTotalBytes], (char *)&word1, 4);
			memcpy(&data[nTotalBytes+4], (char *)&word2, 4);
		
			if(debug_mode){ std::cout << "debug: Retrieved spill of " << nTotalBytes << " bytes (" << nTotalBytes/4 << " words)\n"; }
			ReadSpill(data, nTotalBytes/4 + 2);
		}
	}
	
	run_ctrl_exit = true;
}

void start_cmd_control(Terminal *terminal_){
	std::string cmd = "", arg;

	bool cmd_ready = true;
	
	while(true){
		cmd = terminal_->GetCommand();
		if(cmd == "CTRL_D"){ cmd = "quit"; }
		else if(cmd == "CTRL_C"){ continue; }		
		terminal_->flush();

		if(cmd_ready){
			if(cmd == "quit" || cmd == "exit"){
				if(scan_running){ std::cout << sys_message_head << "Warning! Cannot quit while scan is running\n"; }
				else{
					kill_all = true;
					while(!run_ctrl_exit){ sleep(1); }
					break;
				}
			}
			else{ std::cout << sys_message_head << "Unknown command '" << cmd << "'\n"; }
		}
	}		
}

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

	Terminal terminal;

	int arg_index = 1;
	
	debug_mode = false;
	shm_mode = false;
	
	int file_format = -1;
	while(arg_index < argc){
		if(argv[arg_index][0] != '\0' && argv[arg_index][0] != '-'){ // This must be a filename
			extension = GetExtension(argv[arg_index], prefix);
		}
	
		if(strcmp(argv[arg_index], "--debug") == 0){ 
			std::cout << " NOTE: Using DEBUG mode\n";
			debug_mode = true;
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
			file_format = 0;	Server poll_server;
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
		if(!poll_server.Init(5555, 1)){
			std::cout << " ERROR: Failed to open shm socket 5555!\n";
			return 1;
		}
		else{ std::cout << " NOTE: Listening on poll2 SHM port 5555\n"; }
	}

	terminal.Initialize(".pixieldf.cmd");
	terminal.SetPrompt("PIXIELDF $ ");
	terminal.AddStatusWindow();

	std::cout << "\n PIXIELDF v" << SCAN_VERSION << "\n"; 
	std::cout << " ==  ==  ==  ==  == \n\n"; 

	// Start the run control thread
	std::cout << "Starting data control thread\n";
	std::thread runctrl(start_run_control);
	
	// Start the command control thread. This needs to be the last thing we do to
	// initialize, so the user cannot enter commands before setup is complete
	std::cout << "Starting command thread\n";
	std::thread comctrl(start_cmd_control, &terminal);

	// Synchronize the threads and wait for completion
	comctrl.join();
	runctrl.join();
	
	terminal.Close();
	
	//Reprint the leader as the carriage was returned
	cout << "Running PixieLDF v" << SCAN_VERSION << "\n";

	input_file.close();	

	cleanup(); // Clean up detector driver
	
	// Close the shm socket if it's open
	poll_server.Close();

	return 0;
}
