#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string.h>
#include <time.h>

#include "TH1I.h"
#include "TH2I.h"

#include "HisFile.h"
#include "PlotsRegister.hpp"

///////////////////////////////////////////////////////////////////////////////
// Support Functions
///////////////////////////////////////////////////////////////////////////////

//#ifdef USE_DAMM_OUTPUT

OutputHisFile output_his;

/// Create a DAMM 1D histogram (implemented for backwards compatibility)
void hd1d_(int dammId, int nHalfWords, int rawlen, int histlen, int min, int max, const char *title, unsigned int length){
	drr_entry *entry = new drr_entry(dammId, (short)nHalfWords, (short)rawlen, (short)histlen, (short)min, (short)max, title);
	output_his.push_back(entry);
}

/// Create a DAMM 2D histogram (implemented for backwards compatibility)
void hd2d_(int dammId, int nHalfWords, int rawXlen, int histXlen, int xMin, int xMax, int rawYlen, int histYlen, int yMin, int yMax, const char *title, unsigned int length){
	drr_entry *entry = new drr_entry(dammId, (short)nHalfWords, (short)rawXlen, (short)histXlen, (short)xMin, (short)xMax,
									 (short)rawYlen, (short)histYlen, (short)yMin, (short)yMax, title);
	output_his.push_back(entry);
}

/// Do banana gating using ban files (implemented for backwards compatibility)
bool bantesti_(const int &id, const double &x, const double &y){
	return false;
}

/// Increment histogram dammID at x and y (implemented for backwards compatibility)
void count1cc_(const int &dammID, const int &x, const int &y){
}

/// Unknown (implemented for backwards compatibility)
void set2cc_(const int &dammID, const int &x, const int &y, const int &z){
	count1cc_(dammID, x, y);
}

//#endif

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

/// Copy a string into a character array
void set_char_array(char *output, const std::string &input_, size_t arr_size_){
	for(size_t index = 0; index < arr_size_; index++){
		if(index < input_.size()){ output[index] = input_[index]; }
		else{ output[index] = ' '; }
	}
	output[arr_size_] = '\0';
}

///////////////////////////////////////////////////////////////////////////////
// struct drr_entry
///////////////////////////////////////////////////////////////////////////////

/// Constructor for 1d histogram
drr_entry::drr_entry(int hisID_, short halfWords_, short raw_, short scaled_, short min_, short max_, const char * title_){
	hisID = hisID_; hisDim = 1; halfWords = halfWords_;

	// Set range and scaling variables
	params[0] = 0; params[1] = 0; params[2] = 0; params[3] = 0; 
	raw[0] = raw_; raw[1] = 0; raw[2] = 0; raw[3] = 0; 
	scaled[0] = scaled_; scaled[1] = 0; scaled[2] = 0; scaled[3] = 0;
	minc[0] = min_; minc[1] = 0; minc[2] = 0; minc[3] = 0;
	maxc[0] = max_; maxc[1] = 0; maxc[2] = 0; maxc[3] = 0;
	calcon[0] = 0; calcon[1] = 0; calcon[2] = 0; calcon[3] = 0;
	
	// Set label and titles
	set_char_array(xlabel, "            ", 13);
	set_char_array(ylabel, "            ", 13);
	set_char_array(title, std::string(title_), 41);

	offset = 0; // The file offset will be set later
}

/// Constructor for 2d histogram
drr_entry::drr_entry(int hisID_, short halfWords_, short Xraw_, short Xscaled_, short Xmin_, short Xmax_,
					 short Yraw_, short Yscaled_, short Ymin_, short Ymax_, const char * title_){
	hisID = hisID_; hisDim = 2; halfWords = halfWords_;

	// Set range and scaling variables
	params[0] = 0; params[1] = 0; params[2] = 0; params[3] = 0; 
	raw[0] = Xraw_; raw[1] = Yraw_; raw[2] = 0; raw[3] = 0; 
	scaled[0] = Xscaled_; scaled[1] = Yscaled_; scaled[2] = 0; scaled[3] = 0;
	minc[0] = Xmin_; minc[1] = Ymin_; minc[2] = 0; minc[3] = 0;
	maxc[0] = Xmax_; maxc[1] = Xmax_; maxc[2] = 0; maxc[3] = 0;
	calcon[0] = 0; calcon[1] = 0; calcon[2] = 0; calcon[3] = 0;
	
	// Set label and titles
	set_char_array(xlabel, "            ", 13);
	set_char_array(ylabel, "            ", 13);
	set_char_array(title, std::string(title_), 41);

	offset = 0; // The file offset will be set later
}

///////////////////////////////////////////////////////////////////////////////
// class HisFile
///////////////////////////////////////////////////////////////////////////////

/// Read an entry from the drr file
drr_entry* HisFile::read_entry(){
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
		else if(err_flag == -3){ std::cout << "  -3: GetHistogram returned 0. i.e. the specified histogram does not exist.\n"; }
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

///////////////////////////////////////////////////////////////////////////////
// class OutputHisFile
///////////////////////////////////////////////////////////////////////////////

void OutputHisFile::flush(){
	if(!writable){ return; }
}

OutputHisFile::OutputHisFile(){
	writable = false;
	debug_mode = false;
}

OutputHisFile::OutputHisFile(std::string fname_prefix){
	debug_mode = false;
	Open(fname_prefix);
}

OutputHisFile::~OutputHisFile(){
	Close();
}

size_t OutputHisFile::push_back(drr_entry *entry_){
	if(!entry_ || !writable){ return 0; }
	
	// Seek to the end of this histogram file
	ofile.seekp(0, std::ios::end);
	entry_->offset = (size_t)ofile.tellp();
	entries.push_back(entry_);

	// Extend the size of the histogram file
	size_t size = 1;
	for(size_t i = 0; i < (size_t)entry_->hisDim; ++i){
		size *= (size_t)(entry_->scaled[i]-1);
	}
	size *= (size_t)(entry_->halfWords * 2);
	
	char *dummy = new char[size];
	
	for(size_t i = 0; i < size; i++){ dummy[i] = 0x0; }
	
	if(debug_mode){	std::cout << " Extending .his file by " << size << " bytes for his ID = " << entry_->hisID << std::endl; }
	
	ofile.write(dummy, size);
	
	delete[] dummy;
	
	return size;
}

bool OutputHisFile::Open(std::string fname_prefix){
	if(writable){ return false; }
	fname = fname_prefix;
	ofile.open((fname+".his").c_str(), std::ios::binary);
	flush_wait = 10000;
	return (writable = ofile.good());
}

bool OutputHisFile::Close(bool make_list_file_/*=false*/, const std::string &descrip_/*="RootPixieScan .drr file"*/){
	bool retval = true;

	flush();

	set_char_array(initial, "HHIRFDIR0001", 13);
	set_char_array(description, descrip_, 41);
	
	nHis = entries.size(); 
	nHWords = (128 * (1 + entries.size()) + entries.size() * 4)/2;
	
	time_t rawtime;
	struct tm * timeinfo;
	
	time(&rawtime);
	timeinfo = localtime (&rawtime);

	date[0] = 0;
	date[1] = timeinfo->tm_year + 1900; // tm_year measures the year from 1900
	date[2] = timeinfo->tm_mon; // tm_mon ranges from 0 to 11
	date[3] = timeinfo->tm_mday;
	date[4] = timeinfo->tm_hour;
	date[5] = timeinfo->tm_min;

	// Write the .drr file
	std::ofstream drr_file((fname+".drr").c_str(), std::ios::binary);
	if(drr_file.good()){
		char dummy = 0x0;
		int his_id;
	
		// Write the 128 byte drr header
		drr_file.write(initial, 12);
		drr_file.write((char*)&nHis, 4);
		drr_file.write((char*)&nHWords, 4);
		for(int i = 0; i < 6; i++){ drr_file.write((char*)&date[i], 4); }
		for(int i = 0; i < 44; i++){ drr_file.write(&dummy, 1); } // add the trailing garbage
		drr_file.write(description, 40);

		// Write the drr entries
		for(std::vector<drr_entry*>::iterator iter = entries.begin(); iter != entries.end(); iter++){
			drr_file.write((char*)&(*iter)->hisDim, 2);
			drr_file.write((char*)&(*iter)->halfWords, 2);
			drr_file.write((char*)&(*iter)->params, 8);
			drr_file.write((char*)&(*iter)->raw, 8);
			drr_file.write((char*)&(*iter)->scaled, 8);
			drr_file.write((char*)&(*iter)->minc, 8);
			drr_file.write((char*)&(*iter)->maxc, 8);
			drr_file.write((char*)&(*iter)->offset, 4);
			drr_file.write((*iter)->xlabel, 12); (*iter)->xlabel[12] = '\0';
			drr_file.write((*iter)->ylabel, 12); (*iter)->ylabel[12] = '\0';
			drr_file.write((char*)&(*iter)->calcon, 16);
			drr_file.write((*iter)->title, 40); (*iter)->title[40] = '\0';
		}
		
		// Write the histogram IDs
		for(std::vector<drr_entry*>::iterator iter = entries.begin(); iter != entries.end(); iter++){
			his_id = (*iter)->hisID;
			drr_file.write((char*)&his_id, 4);
			delete (*iter);
		}
		
		entries.clear();
	}
	else{ retval = false; }
	
	writable = false;
	drr_file.close();
	ofile.close();
	
	return retval;
}
