#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string.h>
#include <time.h>

#include "TH1I.h"
#include "TH2I.h"

#include "HisFile.h"

///////////////////////////////////////////////////////////////////////////////
// Support Functions
///////////////////////////////////////////////////////////////////////////////

/// Create a DAMM 1D histogram (implemented for backwards compatibility)
void hd1d_(int dammId, int nHalfWords, int rawlen, int histlen, int min, int max, const char *title, unsigned int length){
	drr_entry *entry = new drr_entry(dammId, (short)nHalfWords, (short)rawlen, (short)histlen, (short)min, (short)max, title);
	output_his->push_back(entry);
}

/// Create a DAMM 2D histogram (implemented for backwards compatibility)
void hd2d_(int dammId, int nHalfWords, int rawXlen, int histXlen, int xMin, int xMax, int rawYlen, int histYlen, int yMin, int yMax, const char *title, unsigned int length){
	drr_entry *entry = new drr_entry(dammId, (short)nHalfWords, (short)rawXlen, (short)histXlen, (short)xMin, (short)xMax,
									 (short)rawYlen, (short)histYlen, (short)yMin, (short)yMax, title);
	output_his->push_back(entry);
}

/// Do banana gating using ban files (implemented for backwards compatibility)
bool bantesti_(const int &id, const double &x, const double &y){
	return false;
}

/// Increment histogram dammID at x and y (implemented for backwards compatibility)
void count1cc_(const int &dammID, const int &x, const int &y){
	output_his->Fill(dammID, x, y);
}

/// Unknown (implemented for backwards compatibility)
void set2cc_(const int &dammID, const int &x, const int &y, const int &z){
	output_his->Fill(dammID, x, y, z);
}

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
	comp[0] = raw_/scaled_; comp[1] = 0;  comp[2] = 0; comp[3] = 0;
	minc[0] = min_; minc[1] = 0; minc[2] = 0; minc[3] = 0;
	maxc[0] = max_; maxc[1] = 0; maxc[2] = 0; maxc[3] = 0;
	calcon[0] = 0; calcon[1] = 0; calcon[2] = 0; calcon[3] = 0;
	
	// Set label and titles
	set_char_array(xlabel, "            ", 13);
	set_char_array(ylabel, "            ", 13);
	set_char_array(title, std::string(title_), 41);

	offset = 0; // The file offset will be set later
	if(2*halfWords_ == 4){ use_int = true; }
	else if(2*halfWords == 2){ use_int = false; }
	else{ std::cout << "Invalid cell size (" << 2*halfWords << ")!\n"; }
}

/// Constructor for 2d histogram
drr_entry::drr_entry(int hisID_, short halfWords_, short Xraw_, short Xscaled_, short Xmin_, short Xmax_,
					 short Yraw_, short Yscaled_, short Ymin_, short Ymax_, const char * title_){
	hisID = hisID_; hisDim = 2; halfWords = halfWords_;

	// Set range and scaling variables
	params[0] = 0; params[1] = 0; params[2] = 0; params[3] = 0; 
	raw[0] = Xraw_; raw[1] = Yraw_; raw[2] = 0; raw[3] = 0; 
	scaled[0] = Xscaled_; scaled[1] = Yscaled_; scaled[2] = 0; scaled[3] = 0;
	comp[0] = Xraw_/Xscaled_; comp[1] = Yraw_/Yscaled_;  comp[2] = 0; comp[3] = 0;
	minc[0] = Xmin_; minc[1] = Ymin_; minc[2] = 0; minc[3] = 0;
	maxc[0] = Xmax_; maxc[1] = Ymax_; maxc[2] = 0; maxc[3] = 0;
	calcon[0] = 0; calcon[1] = 0; calcon[2] = 0; calcon[3] = 0;
	
	// Set label and titles
	set_char_array(xlabel, "            ", 13);
	set_char_array(ylabel, "            ", 13);
	set_char_array(title, std::string(title_), 41);

	offset = 0; // The file offset will be set later
	if(2*halfWords_ == 4){ use_int = true; }
	else if(2*halfWords == 2){ use_int = false; }
	else{ std::cout << "Invalid cell size (" << 2*halfWords << ")!\n"; }
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

/// Delete all drr drr_entries and clear the drr_entries vector
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
	LoadDrr(prefix_);
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
	size_t num_bins = hd_size;
	if(cell_size == 2){ // Cell size is a short int
		num_bins = num_bins/2;
		use_int = false;
		if(hd_size % 2 != 0){ 
			err_flag = -5; 
			return NULL; 
		}
	}
	else if(cell_size == 4){ // Cell size is a standard int
		num_bins = num_bins/4;
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
						  num_bins, (double)current_entry->minc[0], (double)current_entry->maxc[0]);

	// Fill the histogram bins
	unsigned short sval;
	unsigned int ival;
	for(size_t x = 0; x < num_bins; x++){
		if(use_int){ 
			memcpy((char*)&ival, &hdata[4*x], 4);
			hist->SetBinContent(x+1, ival); 
		}
		else{ 
			memcpy((char*)&sval, &hdata[2*x], 2);
			hist->SetBinContent(x+1, sval); 
		}
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
	size_t num_bins = hd_size;
	if(cell_size == 2){ // Cell size is a short int
		num_bins = num_bins/2;
		use_int = false;
		if(hd_size % 2 != 0){ 
			err_flag = -5; 
			return NULL; 
		}
	}
	else if(cell_size == 4){ // Cell size is a standard int
		num_bins = num_bins/4;
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
	unsigned short sval;
	unsigned int ival;
	/*for(size_t x = 0; x < current_entry->scaled[0]-1; x++){
		for(short y = 0; y < current_entry->scaled[1]-1; y++){
			if(use_int){ 
				memcpy((char*)&ival, &hdata[y*current_entry->scaled[1]-1+x], 4);
				hist->SetBinContent(hist->GetBin(x, y), ival); }
			else{ 
				memcpy((char*)&sval, &hdata[y*current_entry->scaled[1]-1+x], 2);
				hist->SetBinContent(hist->GetBin(x, y), sval); 
			}
		}
	}*/
	for(size_t x = 0; x < num_bins; x++){
		if(use_int){ 
			memcpy((char*)&ival, &hdata[4*x], 4);
			hist->SetBinContent(x+1, ival); 
		}
		else{ 
			memcpy((char*)&sval, &hdata[2*x], 2);
			hist->SetBinContent(x+1, sval); 
		}
	}
	hist->ResetStats(); // Update the histogram statistics to include new bin content

	return hist;
}

/// Get a drr entry from the vector
void HisFile::GetEntry(size_t id_){
	if(id_ < drr_entries.size()){ current_entry = drr_entries.at(id_); }
	else{ current_entry = NULL; }
}

/// Load the specified histogram
size_t HisFile::GetHistogram(int hist_, bool no_copy_/*=false*/){
	err_flag = 0; // Reset the error flag
	if(!is_open){ 
		err_flag = 4;
		return 0; 
	}
	
	// Get the requested drr entry
	GetEntry(hist_);
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

bool HisFile::LoadDrr(const char* prefix_, bool open_his_/*=true*/){
	err_flag = 0; // Reset the error flag
	if(drr.is_open()){ drr.close(); }
	
	// Clear the old drr drr_entries
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
	if(open_his_){
		if(his.is_open()){ his.close(); }

		his.open((filename_prefix + ".his").c_str(), std::ios::binary);

		is_open = his.is_open() && his.good();
		if(!is_open){ 
			err_flag = 3;
			return false; 
		}
	}

	// Read in the drr header
	drr.read((char*)&nHis, 4);
	drr.read((char*)&nHWords, 4);
	for(int i = 0; i < 6; i++){ drr.read((char*)&date[i], 4); }
	drr.seekg(44, std::ios::cur); // skip the trailing garabage
	drr.read(description, 40); description[40] = '\0';

	// Read in all drr drr_entries
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
	std::cout << "comp: " << current_entry->comp[0] << ", " << current_entry->comp[1] << ", " << current_entry->comp[2] << ", " << current_entry->comp[3] << std::endl;
	std::cout << "minc: " << current_entry->minc[0] << ", " << current_entry->minc[1] << ", " << current_entry->minc[2] << ", " << current_entry->minc[3] << std::endl;
	std::cout << "maxc: " << current_entry->maxc[0] << ", " << current_entry->maxc[1] << ", " << current_entry->maxc[2] << ", " << current_entry->maxc[3] << std::endl;
	std::cout << "cal: " << current_entry->calcon[0] << ", " << current_entry->calcon[1] << ", " << current_entry->calcon[2] << ", " << current_entry->calcon[3] << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// class OutputHisFile
///////////////////////////////////////////////////////////////////////////////

void OutputHisFile::flush(){
	if(debug_mode){ std::cout << "debug: Flushing histogram entries to file.\n"; }

	if(writable){ // Do the filling
		std::streampos location;
		for(std::vector<fill_queue*>::iterator iter = fills_waiting.begin(); iter != fills_waiting.end(); iter++){
			current_entry = (*iter)->entry;
			
			// Seek to the specified bin
			ofile.seekg(current_entry->offset*2 + (*iter)->byte, std::ios::beg); // input offset
			
			unsigned short sval = 0;
			unsigned int ival = 0;
			
			// Overwrite the bin value
			if(current_entry->use_int){
				// Get the original value of the bin
				ofile.read((char*)&ival, 4);
				ival += (*iter)->weight;
				
				// Set the new value of the bin
				ofile.seekp(current_entry->offset*2 + (*iter)->byte, std::ios::beg); // output offset
				ofile.write((char*)&ival, 4);
			}
			else{
				// Get the original value of the bin
				ofile.read((char*)&sval, 2);
				sval += (short)(*iter)->weight;
				
				// Set the new value of the bin
				ofile.seekp(current_entry->offset*2 + (*iter)->byte, std::ios::beg); // output offset
				ofile.write((char*)&sval, 2);
			}
		}
	}
	
	// Delete the drr_entries in the fill_queue vector
	for(std::vector<fill_queue*>::iterator iter = fills_waiting.begin(); iter != fills_waiting.end(); iter++){
		delete (*iter);
	}
	fills_waiting.clear();
	
	flush_count = 0;
}

OutputHisFile::OutputHisFile(){
	writable = false;
	finalized = false;
	existing_file = false;
	debug_mode = true;
}

OutputHisFile::OutputHisFile(std::string fname_prefix){
	finalized = false;
	existing_file = false;
	debug_mode = true;
	Open(fname_prefix);
}

OutputHisFile::~OutputHisFile(){
	Close();
}

size_t OutputHisFile::push_back(drr_entry *entry_){
	if(!entry_){ 
		if(debug_mode){ std::cout << "debug: OutputHisFile::push_back was passed a NULL pointer!\n"; }
		return 0; 
	}
	else if(!writable || finalized){ 
		if(debug_mode){ std::cout << "debug: The .drr and .his files have already been finalized and are locked!\n"; }
		return 0; 
	}
	
	// Search for existing histogram with the same id
	for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
		if((*iter)->hisID == entry_->hisID){ // Found a match in the drr entry list
			if(debug_mode){ std::cout << "debug: His id = " << entry_->hisID << " is already in the drr entry list!\n"; }
			
			/*if(existing_file && !(*iter)->Compare(entry_)){ // The his id is in the list, but the drr entries do not match
				if(debug_mode){ std::cout << "debug: Input drr entry does not match existing drr entry!\n"; }
			}*/
			
			return false;
		}
	}

	hd_size = 1;
	for(int i = 0; i < entry_->hisDim; ++i){
		hd_size *= entry_->scaled[i]-1;
	}
	
	hd_size *= entry_->halfWords * 2;
	
	// Seek to the end of this histogram file
	ofile.seekp(0, std::ios::end);
	entry_->offset = (size_t)ofile.tellp()/2; // Set the file offset (in 2 byte words)
	drr_entries.push_back(entry_);

	// Extend the size of the histogram file
	size_t size = 1;
	for(size_t i = 0; i < (size_t)entry_->hisDim; ++i){
		size *= (size_t)(entry_->scaled[i]-1);
	}
	size *= (size_t)(entry_->halfWords * 2);
	
	char dummy = 0x0;

	if(debug_mode){	std::cout << "debug: Extending .his file by " << size << " bytes for his ID = " << entry_->hisID << " i.e. '" << entry_->title << "'\n"; }
	
	for(size_t i = 0; i < size; i++){ ofile.write(&dummy, 1); }
	
	return size;
}

bool OutputHisFile::Finalize(bool make_list_file_/*=false*/, const std::string &descrip_/*="RootPixieScan .drr file"*/){
	if(!writable || finalized){ 
		if(debug_mode){ std::cout << "debug: The .drr and .his files have already been finalized and are locked!\n"; }
		return false; 
	}

	bool retval = false;

	set_char_array(initial, "HHIRFDIR0001", 13);
	set_char_array(description, descrip_, 41);
	
	if(debug_mode){ std::cout << "debug: NHIS = " << drr_entries.size() << std::endl; }
	nHis = drr_entries.size(); 
	nHWords = (128 * (1 + drr_entries.size()) + drr_entries.size() * 4)/2;
	
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
		for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
			if(debug_mode){ std::cout << "debug: Writing .drr entry for his id = " << (*iter)->hisID << std::endl; }
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
		for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
			his_id = (*iter)->hisID;
			drr_file.write((char*)&his_id, 4);
		}
		
		finalized = true;
		retval = true;
	}
	
	drr_file.close();
	
	if(!retval){
		if(debug_mode){ std::cout << "debug: Failed to open the .drr file for writing!\n"; }
		return false;
	}
	return true;
}

bool OutputHisFile::Fill(int hisID_, int x_, int y_, int weight_/*=1*/){
	if(!writable){ return false; }

	// Search for the specified histogram in the .drr entry list
	for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
		if((*iter)->hisID == hisID_){
			double dx, dy;
			int x_comp, y_comp;
			int binx=0, biny=0;
			x_comp = (short)(x_ / (*iter)->comp[0]);
			
			// Check that x_ and y_ are within their respective axes
			if(x_comp < (int)(*iter)->minc[0] || x_comp > (int)(*iter)->maxc[0]){ continue; }
			dx = ((*iter)->maxc[0] - (*iter)->minc[0])/((*iter)->scaled[0]-1);
			binx = (x_/dx);
			
			if((*iter)->hisDim > 1){
				y_comp = (short)(y_ / (*iter)->comp[1]);
				if(y_comp < (int)(*iter)->minc[1] || y_comp > (int)(*iter)->maxc[1]){ continue; }
				else{
					dy = ((*iter)->maxc[0] - (*iter)->minc[0])/((*iter)->scaled[0]-1); 
					biny = (y_/dy);
				}
			}
		
			// Push this fill into the queue
			fill_queue *fill = new fill_queue((*iter), binx * ((*iter)->scaled[0]-1) + biny, weight_);
			fills_waiting.push_back(fill);
	
			if(++flush_count >= flush_wait){ flush(); }
			return true;
		}
	}
	
	//if(debug_mode){ std::cout << "debug: Failed to find his id = " << hisID_ << " in the drr entry list!\n"; }
	
	return false;
}

bool OutputHisFile::FillBin(int hisID_, int x_, int y_, int weight_){
	if(!writable){ return false; }

	// Search for the specified histogram in the .drr entry list
	for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
		if((*iter)->hisID == hisID_){
			// Check that x_ and y_ are within their respective axes
			if(x_ < 0 || x_ >= (*iter)->scaled[0]-1){ break; }
			if((*iter)->hisDim > 1 && (y_ < 0 || y_ >= (*iter)->scaled[1]-1)){ break; }
		
			// Push this fill into the queue
			fill_queue *fill;
			if((*iter)->hisDim == 1){ fill = new fill_queue((*iter), x_, weight_); }
			else{ fill = new fill_queue((*iter), x_ * ((*iter)->scaled[0]-1) + x_, weight_); }
			fills_waiting.push_back(fill);
	
			if(++flush_count >= flush_wait){ flush(); }
			return true;
		}
	}
	
	//if(debug_mode){ std::cout << "debug: Failed to find his id = " << hisID_ << " in the drr entry list!\n"; }
	
	return false;
}
	
bool OutputHisFile::Open(std::string fname_prefix){
	if(writable){ 
		if(debug_mode){ std::cout << "debug: The .his file is already open!\n"; }
		return false; 
	}
	
	fname = fname_prefix;
	existing_file = false;
	
	//touch.close();
	ofile.open((fname+".his").c_str(), std::ios::out | std::ios::in | std::ios::trunc | std::ios::binary);
	flush_wait = 100000;
	flush_count = 0;
	return (writable = ofile.good());
}

void OutputHisFile::Close(){
	flush();

	if(!finalized){ Finalize(); }

	// Clear the .drr entries in the entries vector
	clear_drr_entries();
	
	writable = false;
	ofile.close();
}
