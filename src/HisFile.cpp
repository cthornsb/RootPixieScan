#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string.h>

#include "TH1I.h"
#include "TH2I.h"

#include "HisFile.h"

/// Strip trailing whitespace from a c-string
std::string rstrip(char *input_){
	unsigned int index = strlen(input_)-1;
	std::string output = "";

	// Skip all trailing whitespace
	while(input_[index--] == ' ' && index > 0){ }
	
	if(index == 0){ return output; }
	else{ index++; }
	
	for(unsigned int i = 0; i <= index; i++){
		output += input_[i];
	}
	
	return output;
}

/// Read an entry from the drr file
HisFile::drr_entry* HisFile::read_entry(){
	drr_entry *output = new drr_entry();

	// Read 128 bytes from the drr file
	drr.read((char*)&output->hisDim, 2);
	drr.read((char*)&output->halfWords, 2);
	drr.read((char*)&output->params, 8);
	drr.read((char*)&output->raw, 8);
	drr.read((char*)&output->scaled, 8);
	drr.read((char*)&output->minc, 8);
	drr.read((char*)&output->maxc, 8);
	drr.read((char*)&output->offset, 4);
	drr.read(output->xlabel, 12); output->xlabel[12] = '\0';
	drr.read(output->ylabel, 12); output->ylabel[12] = '\0';
	drr.read((char*)&output->calcon, 16);
	drr.read(output->title, 40); output->title[40] = '\0';
	
	return output;
}

/// Get a drr entry from the vector
void HisFile::get_entry(size_t id_){
	if(id_ < drr_entries.size()){ current_entry = drr_entries.at(id_); }
	else{ current_entry = NULL; }
}

/// Set the size of the histogram and allocate memory for data storage
void HisFile::set_hist_size(){
	if(hdata){ delete[] hdata; }

	hd_size = 1;
	for(int i = 0; i < current_entry->hisDim; ++i){
		hd_size *= current_entry->scaled[i]-1;
	}
	
	hd_size *= current_entry->halfWords * 2;
	hdata = new char[hd_size];
}

/// Delete all drr entries and clear the entries vector
void HisFile::clear_drr_entries(){
	for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
		delete (*iter);
	}
	drr_entries.clear();
	current_entry = NULL;
}

HisFile::HisFile(){
	hdata = NULL;
	hd_size = 0;
	err_flag = 0;
	hists_processed = 0;
	is_good = false;
	is_open = false;
}

HisFile::HisFile(const char *prefix_){
	hdata = NULL;
	hd_size = 0;
	err_flag = 0;
	hists_processed = 0;
	Load(prefix_);
}

HisFile::~HisFile(){
	drr.close();
	his.close();
	
	if(hdata){ delete[] hdata; }
	clear_drr_entries();
}

/// Get the error code for a member function call
int HisFile::GetError(bool verbose_/*=true*/){
	if(verbose_){
		std::cout << " HIS_FILE ERROR!\n";
		if(err_flag == 0){ std::cout << "  0: No error occurred.\n"; }
		else if(err_flag == 1){ std::cout << "  1: Failed to open the .drr file. Check that the path is correct.\n"; }
		else if(err_flag == 2){ std::cout << "  2: The .drr file had an incorrect format and could not be read.\n"; }
		else if(err_flag == 3){ std::cout << "  3: Failed to open the .his file. Check that the path is correct.\n"; }
		else if(err_flag == 4){ std::cout << "  4: Either the .drr file and/or the .his file are not opened or are not of the correct format.\n"; }
		else if(err_flag == 5){ std::cout << "  5: Cannot call GetNextHistogram because the last entry in the .drr file is already loaded.\n"; }
		else if(err_flag == -1){ std::cout << "  -1: current_entry is uninitialized. Use GetHistogram, GetNextHistogram, or GetHistogramByID.\n"; }
		else if(err_flag == -2){ std::cout << "  -2: Specified .his cell size is larger than that of an integer (4 bytes).\n"; }
		else if(err_flag == -3){ std::cout << "  -3: GetHistogram returned 0. e.g. the specified histogram does not exist.\n"; }
		else if(err_flag == -4){ std::cout << "  -4: The current histogram has the incorrect dimension for the called function.\n"; }
		else if(err_flag == -5){ std::cout << "  -5: .his cell size is 2 bytes but size of histogram data array is not evenly divisible by 2.\n"; }
		else if(err_flag == -6){ std::cout << "  -6: .his cell size is 4 bytes but size of histogram data array is not evenly divisible by 4.\n"; }
		else if(err_flag == -7){ std::cout << "  -7: Encountered an invalid .his cell size.\n"; }
		else{ std::cout << "  " << err_flag << ": An unknown error occurred!\n"; }
	}
	return err_flag;
}

/// Return the date formatted as mmm dd, yyyy HH:MM
std::string HisFile::GetDate(){
	err_flag = 0; // Reset the error flag
	if(!is_open){ 
		err_flag = 4;
		return ""; 
	}

	static std::string months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Nov", "Dec"};
	
	std::stringstream stream;
	if(date[2] > 0 && date[2] < 12){ stream << months[date[2]]; }
	else{ stream << date[2]; }
	if(date[3] < 10){ stream << " 0" << date[3]; }
	else{ stream << " " << date[3]; }
	stream << ", " << date[1] << " ";
	if(date[4] < 10){ stream << "0" << date[4] << ":"; }
	else{ stream << date[4] << ":"; }
	if(date[5] < 10){ stream << "0" << date[5]; }
	else{ stream << date[5]; }
	
	return stream.str();
}

/// Get the number of bins in the x-axis
short HisFile::GetXbins(){ 
	err_flag = 0; // Reset the error flag
	if(!current_entry){ return err_flag = -1; }
	return current_entry->scaled[0]; 
}

/// Get the number of bins in the y-axis
short HisFile::GetYbins(){ 
	err_flag = 0; // Reset the error flag
	if(!current_entry){ return err_flag = -1; }
	return current_entry->scaled[1]; 
}

/*/// Get the number of bins in the z-axis
short HisFile::GetZbins(){ 
	if(!current_entry){ return -1; }
	return current_entry->scaled[2]; 
}

/// Get the number of bins in the a-axis
short HisFile::GetAbins(){ 
	if(!current_entry){ return -1; }
	return current_entry->scaled[3]; 
}*/

/// Get the range of a 1d histogram
bool HisFile::Get1dRange(short &xmin, short &xmax){
	err_flag = 0; // Reset the error flag
	if(!current_entry){ 
		err_flag = -1;
		return false; 
	}
	xmin = current_entry->minc[0];
	xmax = current_entry->maxc[0];
	return true;
}

/// Get the range of a 2d histogram
bool HisFile::Get2dRange(short &xmin, short &xmax, short &ymin, short &ymax){
	err_flag = 0; // Reset the error flag
	if(!current_entry){ 
		err_flag = -1;
		return false; 
	}
	xmin = current_entry->minc[0];
	xmax = current_entry->maxc[0];
	ymin = current_entry->minc[1];
	ymax = current_entry->maxc[1];
	return true;
}

/*/// Get the range of a 3d histogram
bool HisFile::Get3dRange(short &xmin, short &xmax, short &ymin, short &ymax, short &zmin, short &zmax){
	if(!current_entry){ return false; }
	xmin = current_entry->minc[0];
	xmax = current_entry->maxc[0];
	ymin = current_entry->minc[1];
	ymax = current_entry->maxc[1];
	zmin = current_entry->minc[2];
	zmax = current_entry->maxc[2];
}

/// Get the range of a 4d histogram
bool HisFile::Get4dRange(short &xmin, short &xmax, short &ymin, short &ymax, short &zmin, short &zmax, short &amin, short &amax){
	if(!current_entry){ return false; }
	xmin = current_entry->minc[0];
	xmax = current_entry->maxc[0];
	ymin = current_entry->minc[1];
	ymax = current_entry->maxc[1];
	zmin = current_entry->minc[2];
	zmax = current_entry->maxc[2];
	amin = current_entry->minc[3];
	amax = current_entry->maxc[3];
}*/

/// Get the size of the .his file
std::streampos HisFile::GetHisFilesize(){
	err_flag = 0; // Reset the error flag
	if(!is_open){ 
		err_flag = 4;
		return 0; 
	}
	std::streampos initial_pos = his.tellg();
	his.seekg(0, std::ios::end);
	std::streampos output = his.tellg();
	his.seekg(initial_pos, std::ios::beg);
	return output;
}

/// Get the ID of the histogram
int HisFile::GetHisID(){
	err_flag = 0; // Reset the error flag
	if(!current_entry){ return err_flag = -1; }
	return current_entry->hisID; 
}

/// Get the dimension of the histogram
short HisFile::GetDimension(){ 
	err_flag = 0; // Reset the error flag
	if(!current_entry){ return err_flag = -1; }
	return current_entry->hisDim; 
}

/// Return the size of the histogram cells (in bytes)
size_t HisFile::GetCellSize(){
	err_flag = 0; // Reset the error flag 
	if(!current_entry){ 
		err_flag = -1;
		return 0; 
	}
	return current_entry->halfWords * 2;
}

/// Get a pointer to a root TH1I
TH1I* HisFile::GetTH1(int hist_/*=-1*/){
	err_flag = 0; // Reset the error flag
	if(hist_ == -1 && !current_entry){ 
		err_flag = -1; 
		return NULL; 
	}
	
	size_t cell_size = current_entry->halfWords*2;
	
	// Check that the cell size is not too large
	if(cell_size > 4){ 
		err_flag = -2; 
		return NULL; 
	}
	
	// Get the histogram from the file
	if(hist_ != -1 && GetHistogram(hist_) == 0){			
		err_flag = -3; 
		return NULL; 
	}
	
	// Check that this histogram has the correct dimension
	if(current_entry->hisDim != 1){			
		err_flag = -4; 
		return NULL; 
	}
	
	bool use_int;
	short *sdata = NULL;
	int *idata = NULL;
	if(cell_size == 2){ // Cell size is a short int
		sdata = (short*)hdata;
		use_int = false;
		if(hd_size % 2 != 0){ 
			err_flag = -5; 
			return NULL; 
		}
	}
	else if(cell_size == 4){ // Cell size is a standard int
		idata = (int*)hdata;
		use_int = true;
		if(hd_size % 4 != 0){ 
			err_flag = -6; 
			return NULL; 
		}
	}
	else{ // Unrecognized cell size
		err_flag = -7; 
		return NULL; 
	}
	
	std::stringstream stream;
	stream << "d" << current_entry->hisID;

	TH1I *hist = new TH1I(stream.str().c_str(), rstrip(current_entry->title).c_str(), 
						  current_entry->scaled[0]-1, (double)current_entry->minc[0], (double)current_entry->maxc[0]);

	// Fill the histogram bins
	for(short x = 0; x < current_entry->scaled[0]-1; x++){
		if(use_int){ hist->SetBinContent(x+1, idata[x]); }
		else{ hist->SetBinContent(x+1, sdata[x]); }
	}
	hist->ResetStats(); // Update the histogram statistics to include new bin content

	return hist;
}

/// Get a pointer to a root TH2I
TH2I* HisFile::GetTH2(int hist_/*=-1*/){
	err_flag = 0; // Reset the error flag
	if(hist_ == -1 && !current_entry){ 
		err_flag = -1; 
		return NULL; 
	}
	
	size_t cell_size = current_entry->halfWords*2;
	
	// Check that the cell size is not too large
	if(cell_size > 4){ 
		err_flag = -2; 
		return NULL; 
	}
	
	// Get the histogram from the file
	if(hist_ != -1 && GetHistogram(hist_) == 0){			
		err_flag = -3; 
		return NULL; 
	}
	
	// Check that this histogram has the correct dimension
	if(current_entry->hisDim != 2){			
		err_flag = -4; 
		return NULL; 
	}
	
	bool use_int;
	short *sdata = NULL;
	int *idata = NULL;
	if(cell_size == 2){ // Cell size is a short int
		sdata = (short*)hdata;
		use_int = false;
		if(hd_size % 2 != 0){ 
			err_flag = -5; 
			return NULL; 
		}
	}
	else if(cell_size == 4){ // Cell size is a standard int
		idata = (int*)hdata;
		use_int = true;
		if(hd_size % 4 != 0){ 
			err_flag = -6; 
			return NULL; 
		}
	}
	else{ // Unrecognized cell size
		err_flag = -7; 
		return NULL; 
	}
	
	std::stringstream stream;
	stream << "dd" << current_entry->hisID;

	TH2I *hist = new TH2I(stream.str().c_str(), rstrip(current_entry->title).c_str(), 
						  current_entry->scaled[0]-1, (double)current_entry->minc[0], (double)current_entry->maxc[0],
						  current_entry->scaled[1]-1, (double)current_entry->minc[1], (double)current_entry->maxc[1]);

	// Fill the histogram bins
	for(short x = 0; x < current_entry->scaled[0]-1; x++){
		for(short y = 0; y < current_entry->scaled[1]-1; y++){
			if(use_int){ hist->SetBinContent(hist->GetBin(y, x), idata[x * (current_entry->scaled[0]-1) + y]); }
			else{ hist->SetBinContent(hist->GetBin(y, x), sdata[x * (current_entry->scaled[0]-1) + y]); }
		}
	}
	hist->ResetStats(); // Update the histogram statistics to include new bin content

	return hist;
}

/// Load the specified histogram
size_t HisFile::GetHistogram(int hist_, bool no_copy_/*=false*/){
	err_flag = 0; // Reset the error flag
	if(!is_open){ 
		err_flag = 4;
		return 0; 
	}
	
	// Get the requested drr entry
	get_entry(hist_);
	if(!current_entry){ 
		err_flag = -1;
		return 0; 
	}
	
	if(!no_copy_){
		// Set up the histogram data array
		set_hist_size();
	
		// Seek to the start of this histogram
		his.seekg(current_entry->offset*2, std::ios::beg);

		// Read the histogram data
		his.read(hdata, hd_size);
	}
	
	return hd_size;
}

/// Load a specified histogram by ID
size_t HisFile::GetHistogramByID(int hist_id_, bool no_copy_/*=false*/){
	err_flag = 0; // Reset the error flag
	if(!is_open){ 
		err_flag = 4;
		return 0; 
	}

	int his_count = 0;
	for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
		if((*iter)->hisID == hist_id_){ return GetHistogram(his_count, no_copy_); }
		his_count++;
	}
	
	return 0;
}

size_t HisFile::GetNextHistogram(bool no_copy_/*=false*/){
	err_flag = 0; // Reset the error flag
	if(!is_open){
		err_flag = 4;
		return 0;
	} 
	else if(hists_processed >= nHis){ 
		err_flag = 5;
		return 0; 
	}
	
	return GetHistogram(hists_processed++, no_copy_);
}

bool HisFile::Load(const char* prefix_){
	err_flag = 0; // Reset the error flag
	if(drr.is_open()){ drr.close(); }
	
	// Clear the old drr entries
	clear_drr_entries();

	hists_processed = 0;
	
	std::string filename_prefix(prefix_);
	
	// Open the drr file
	drr.open((filename_prefix + ".drr").c_str(), std::ios::binary);
	
	// Check that this is a drr file
	is_open = drr.is_open() && drr.good();
	if(is_open){
		drr.read(initial, 12); initial[12] = '\0';
	
		if(strcmp(initial, "HHIRFDIR0001") == 0){ is_good = true; }
		else{ 
			err_flag = 2;
			is_good = false; 
			return false;
		}
	}
	else{ 
		err_flag = 1;
		return false; 
	}
	
	// Open the his file
	if(his.is_open()){ his.close(); }
	
	his.open((filename_prefix + ".his").c_str(), std::ios::binary);
	
	is_open = his.is_open() && his.good();
	if(!is_open){ 
		err_flag = 3;
		return false; 
	}

	// Read in the drr header
	drr.read((char*)&nHis, 4);
	drr.read((char*)&nHWords, 4);
	for(int i = 0; i < 6; i++){ drr.read((char*)&date[i], 4); }
	drr.seekg(44, std::ios::cur); // skip the trailing garabage
	drr.read(description, 40); description[40] = '\0';
	
	// Read in all drr entries
	for(int i = 0; i < nHis; i++){
		drr_entries.push_back(read_entry());
	}
	
	// Read in all his IDs
	int his_id;
	for(int i = 0; i < nHis; i++){
		drr.read((char*)&his_id, 4);
		drr_entries.at(i)->hisID = his_id;
	}

	return true;
}

void HisFile::PrintHeader(){
	err_flag = 0; // Reset the error flag
	if(!is_open){ 
		err_flag = 4;
		return; 
	}
	
	std::cout << "head: " << initial << std::endl;
	std::cout << "nHisto: " << nHis << std::endl;
	std::cout << "nHWords: " << nHWords << std::endl;
	std::cout << "label: " << description << std::endl;
	std::cout << "date: " << GetDate() << std::endl;
}

void HisFile::PrintEntry(){
	err_flag = 0; // Reset the error flag
	if(!current_entry){ 
		err_flag = -1;
		return; 
	}

	// hd1d(damm id, half-words per channel, param length, hist length, x-low, x-high, title)
	// hd2d(damm id, half-words per channel, x-param length, x-hist length, x-low, x-high, y-param length, y-hist length, y-low, y-high, title)
	
	std::cout << "hisID: " << current_entry->hisID << std::endl;
	std::cout << "dimension: " << current_entry->hisDim << std::endl;
	std::cout << "num hwords: " << current_entry->halfWords << std::endl;
	std::cout << "title: " << current_entry->title << std::endl;
	std::cout << "xlabel: " << current_entry->xlabel << std::endl;
	std::cout << "ylabel: " << current_entry->ylabel << std::endl;
	std::cout << "offset: " << current_entry->offset << std::endl;
	std::cout << "params: " << current_entry->params[0] << ", " << current_entry->params[1] << ", " << current_entry->params[2] << ", " << current_entry->params[3] << std::endl;
	std::cout << "raw: " << current_entry->raw[0] << ", " << current_entry->raw[1] << ", " << current_entry->raw[2] << ", " << current_entry->raw[3] << std::endl;
	std::cout << "scaled: " << current_entry->scaled[0] << ", " << current_entry->scaled[1] << ", " << current_entry->scaled[2] << ", " << current_entry->scaled[3] << std::endl;
	std::cout << "minc: " << current_entry->minc[0] << ", " << current_entry->minc[1] << ", " << current_entry->minc[2] << ", " << current_entry->minc[3] << std::endl;
	std::cout << "maxc: " << current_entry->maxc[0] << ", " << current_entry->maxc[1] << ", " << current_entry->maxc[2] << ", " << current_entry->maxc[3] << std::endl;
	std::cout << "cal: " << current_entry->calcon[0] << ", " << current_entry->calcon[1] << ", " << current_entry->calcon[2] << ", " << current_entry->calcon[3] << std::endl;
}
