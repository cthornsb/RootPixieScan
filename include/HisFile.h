#ifndef HISFILE_H
#define HISFILE_H

#include <fstream>
#include <vector>

class TH1I;
class TH2I;

/// Create a DAMM 1D histogram
void hd1d_(int dammId, int nHalfWords, int rawlen, int histlen, int min, int max, const char *title, unsigned int length);

/// Create a DAMM 2D histogram
void hd2d_(int dammId, int nHalfWords, int rawXlen, int histXlen, int xMin, int xMax, int rawYlen, int histYlen, int yMin, int yMax, const char *title, unsigned int length);

/// Do banana gating using ban files
bool bantesti_(const int &id, const double &x, const double &y);

/// Increment histogram dammID at x and y
void count1cc_(const int &dammID, const int &x, const int &y);

/// Unknown
void set2cc_(const int &dammID, const int &x, const int &y, const int &z);

/// drr entry information
struct drr_entry{
	int hisID; /// ID of the histogram
	short hisDim; /// Number of dimensions
	short halfWords; /// Number of half-words (2 bytes) per channel
	short params[4]; /// Parameter id numbers, for each dimension (up to 4)
	short raw[4]; /// Raw length
	short scaled[4]; /// Scaled length
	short comp[4]; /// The compression level of the histogram
	short minc[4]; /// Min channel number
	short maxc[4]; /// Max channel number
	int offset; /// Location in his file (in 2-bytes units)
	char xlabel[13]; /// X axis label
	char ylabel[13]; /// Y axis label
	float calcon[4]; /// Calibration for X axis
	char title[41]; /// Title
	bool use_int; /// True if the size of a cell is 4 bytes

	/// Default constructor
	drr_entry(){}
	
	/// Constructor for 1d histogram
	drr_entry(int hisID_, short halfWords_, short raw_, short scaled_, short min_, short max_, const char * title_);
	
	/// Constructor for 2d histogram
	drr_entry(int hisID_, short halfWords_, short Xraw_, short Xscaled_, short Xmin_, short Xmax_,
			  short Yraw_, short Yscaled_, short Ymin_, short Ymax_, const char * title_);
};

struct fill_queue{
	drr_entry *entry; /// .drr entry of the histogram to be filled
	int byte; /// Offset of bin (in bytes)
	int weight; /// Weight of fill
	
	fill_queue(drr_entry *entry_, int bin_, int w_){
		entry = entry_; byte = bin_ * entry->halfWords * 2; weight = w_;
	}
};

class HisFile{
  protected:
	bool is_good; /// True if a valid drr file is open
	bool is_open; /// True if both the drr and his files are open (also requires is_good == true)
	bool debug_mode; /// True if debug mode is set
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

	drr_entry *current_entry; /// Pointer to the current working drr entry
	std::vector<drr_entry *> drr_entries; /// Vector of pointers to all drr_entries in drr file

	/// Read an entry from the drr file
	drr_entry *read_entry();
	
	/// Set the size of the histogram and allocate memory for data storage
	void set_hist_size();
	
	/// Delete all drr entries and clear the entries vector
	void clear_drr_entries();

  public:
	HisFile();
	
	HisFile(const char *prefix_);
	
	~HisFile();

	/// Toggle debug mode on or off
	void SetDebugMode(bool input_=true){ debug_mode = input_; }

	/// Get the error code for a member function call
	int GetError(bool verbose_=true);
	
	/// Return true if a valid drr file is open
	bool IsGood(){ return is_good; }
	
	/// Return true if both the drr and his files are open (also requires is_good == true)
	bool IsOpen(){ return is_open; }

	/// Return a pointer to the current .drr file entry
	drr_entry *GetDrrEntry(){ return current_entry; }

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

	/// Get a drr entry from the vector
	void GetEntry(size_t id_);

	/// Load the specified histogram
	size_t GetHistogram(int hist_, bool no_copy_=false);
	
	/// Load a specified histogram by ID
	size_t GetHistogramByID(int hist_id_, bool no_copy_=false);
	
	/// Load the next histogram specified in the .drr file
	size_t GetNextHistogram(bool no_copy_=false);
	
	/// Load drr entries from the .drr file
	bool LoadDrr(const char* prefix_, bool open_his_=true);

	void PrintHeader();
	
	void PrintEntry();
};

class OutputHisFile : public HisFile{
  private:
	std::fstream ofile; /// The output .his file stream
	std::string fname; /// The output filename prefix
	bool writable; /// True if the output .his file is open and writable
	bool finalized; /// True if the .his and .drr files are locked
	bool existing_file; /// True if the .his file was a previously existing file
	unsigned int flush_wait; /// Number of fills to wait between flushes
	unsigned int flush_count; /// Number of fills since last flush
	std::vector<fill_queue*> fills_waiting; /// Vector containing list of histograms to be filled

	/// Flush histogram fills to file
	void flush();
	
  public:
	OutputHisFile();
  
	OutputHisFile(std::string fname_prefix);

	~OutputHisFile();

	/// Return true if the output .his file is open and writable and false otherwise
	bool IsWritable(){ return writable; }
	
	/// Set the number of fills to wait between file flushes
	void SetFlushWait(unsigned int wait_){ flush_wait = wait_; }
	
	/* Push back with another histogram entry. This command will also
	 * extend the length of the .his file (if possible). DO NOT delete
	 * the passed drr_entry after calling. OutputHisFile will handle cleanup.
	 * On success, returns the number of bytes the file was extended by and zero
	 * upon failure.
	 */
	size_t push_back(drr_entry *entry_);
	
	/* Lock the .his and .drr files from being modified. This prevents the user from
	 * adding any more histograms to the .drr entry list.
	 */
	bool Finalize(bool make_list_file_=false, const std::string &descrip_="RootPixieScan .drr file");
	
	/// Increment a histogram at (x, y) by weight_
	bool Fill(int hisID_, int x_, int y_, int weight_=1);
	
	/// Increment a histogram at bin (x, y) by weight_
	bool FillBin(int hisID_, int x_, int y_, int weight_=1);
	
	/// Open a new .his file
	bool Open(std::string fname_prefix);
	
	/// Close the histogram file and write the drr file
	void Close();
};

extern OutputHisFile *output_his; /// The global .his file handler

#endif
