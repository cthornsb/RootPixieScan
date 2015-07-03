#ifndef HISFILE_H
#define HISFILE_H

#include <fstream>

class TH1I;
class TH2I;

class HisFile{
  private:
	bool is_good; /// True if a valid drr file is open
	bool is_open; /// True if both the drr and his files are open (also requires is_good == true)
	std::ifstream drr; /// The input .drr file
	std::ifstream his; /// The input .his file
	
	int hists_processed; /// The number of histograms which have been processed
	
	int err_flag; /// Integer value for storing error information
	
	char *hdata; /// Array for storing histogram data
	size_t hd_size; /// Size of histogram data array
	
	// drr header information
	char initial[13]; /// String HHIRFDIR0001
	int nHis; /// Number of histograms in file
	int nHWords; /// Number of half-words (2 bytes) in file
	int date[6]; /// Date 0 YY MM DD HR MN
	char description[41]; /// Field for text description 

	// drr entry information
	struct drr_entry{
		int hisID; /// ID of the histogram
		short hisDim; /// Number of dimensions
		short halfWords; /// Number of half-words (2 bytes) per channel
		short params[4]; /// Parameter id numbers, for each dimension (up to 4)
		short raw[4]; /// Raw length
		short scaled[4]; /// Scaled length
		short minc[4]; /// Min channel number
		short maxc[4]; /// Max channel number
		int offset; /// Location in his file (in 2-bytes units)
		char xlabel[13]; /// X axis label
		char ylabel[13]; /// Y axis label
		float calcon[4]; /// Calibration for X axis
		char title[41]; /// Title
	};
	
	drr_entry *current_entry; /// Pointer to the current working drr entry
	std::vector<drr_entry *> drr_entries; /// Vector of pointers to all drr_entries in drr file

	/// Read an entry from the drr file
	drr_entry *read_entry();
	
	/// Get a drr entry from the vector
	void get_entry(size_t id_);
	
	/// Set the size of the histogram and allocate memory for data storage
	void set_hist_size();
	
	/// Delete all drr entries and clear the entries vector
	void clear_drr_entries();

  public:
	HisFile();
	
	HisFile(const char *prefix_);
	
	~HisFile();

	/// Get the error code for a member function call
	int GetError(bool verbose_=true);
	
	/// Return true if a valid drr file is open
	bool IsGood(){ return is_good; }
	
	/// Return true if both the drr and his files are open (also requires is_good == true)
	bool IsOpen(){ return is_open; }

	/// Return the date formatted as mmm dd, yyyy HH:MM
	std::string GetDate();
	
	/// Get the number of bins in the x-axis
	short GetXbins();
	
	/// Get the number of bins in the y-axis
	short GetYbins();

	/*/// Get the number of bins in the z-axis
	short GetZbins();

	/// Get the number of bins in the a-axis
	short GetAbins();*/
	
	/// Get the range of a 1d histogram
	bool Get1dRange(short &xmin, short &xmax);
	
	/// Get the range of a 2d histogram
	bool Get2dRange(short &xmin, short &xmax, short &ymin, short &ymax);

	/*/// Get the range of a 3d histogram
	bool Get3dRange(short &xmin, short &xmax, short &ymin, short &ymax, short &zmin, short &zmax);

	/// Get the range of a 4d histogram
	bool Get4dRange(short &xmin, short &xmax, short &ymin, short &ymax, short &zmin, short &zmax, short &amin, short &amax);*/

	/// Get the size of the .his file
	std::streampos GetHisFilesize();

	/// Get the ID of the histogram
	int GetHisID();

	/// Get the dimension of the histogram
	short GetDimension();

	/// Return pointer to the histogram data array
	char *GetData(){ return hdata; }

	/// Return the size of the histogram data array (in bytes)
	size_t GetDataSize(){ return hd_size; }

	/// Return the size of the histogram cells (in bytes)
	size_t GetCellSize();

	/// Get a pointer to a root TH1I
	TH1I *GetTH1(int hist_=-1);
	
	/// Get a pointer to a root TH2I
	TH2I *GetTH2(int hist_=-1);

	/// Load the specified histogram
	size_t GetHistogram(int hist_, bool no_copy_=false);
	
	/// Load a specified histogram by ID
	size_t GetHistogramByID(int hist_id_, bool no_copy_=false);
	
	size_t GetNextHistogram(bool no_copy_=false);
	
	bool Load(const char* prefix_);

	void PrintHeader();
	
	void PrintEntry();
};

/*class OutputHisFile : public HisFile{
  private:
	std::ofstream ofile;
	
	bool can_write;
	
	unsigned int flush_wait;
	
	void flush();
	
  public:
	OutputHisFile
	
	bool CanWrite(){ return can_write; }
	
	void DeclareHistogram2D
	
	void Plot(const int &id_, const double &val_);
};*/

#endif
