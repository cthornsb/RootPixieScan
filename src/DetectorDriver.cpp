/** \file DetectorDriver.cpp
 * \brief event processing
 *
 * In this file are the details for experimental processing of a raw event
 * created by ScanList() in PixieStd.cpp.  Event processing includes things
 * which do not change from experiment to experiment (such as energy
 * calibration and raw parameter plotting) and things that do (differences
 * between MTC and RMS experiment, for example).
 *
 * \author S. Liddick
 * \date 02 July 2007
 *
 * <STRONG>Modified:</strong> <br>
 * SNL - 7-12-07 : 
 *    Add root analysis. If the ROOT program has been
 *    detected on the computer system the and the
 *    makefile has the useroot flag declared, ROOT
 *    analysis will be included. <br>
 * DTM - Oct. '09 : 
 *    Significant structural/cosmetic changes. Event processing is
 *    now primarily handled by individual event processors which
 *    handle their own DetectorDrivers
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <sstream>

#include "PathHolder.hpp"
#include "Exceptions.hpp"
#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "MapFile.hpp"
#include "RandomPool.hpp"
#include "RawEvent.hpp"
#include "TimingInformation.hpp"
#include "TreeCorrelator.hpp"

#include "DammPlotIds.hpp"

#include "BetaProcessor.hpp"
#include "DssdProcessor.hpp"
#include "GeProcessor.hpp"
#include "ImplantSsdProcessor.hpp"
#include "IonChamberProcessor.hpp"
#include "LiquidProcessor.hpp"
#include "LogicProcessor.hpp"
#include "McpProcessor.hpp"
#include "MtcProcessor.hpp"
#include "NeutronProcessor.hpp"
#include "PositionProcessor.hpp"
#include "PulserProcessor.hpp"
#include "SsdProcessor.hpp"
#include "TraceFilterer.hpp"
#include "VandleProcessor.hpp"
#include "ValidProcessor.hpp"

#include "CfdAnalyzer.hpp"
#include "DoubleTraceAnalyzer.hpp"
#include "FittingAnalyzer.hpp"
#include "TauAnalyzer.hpp"
#include "TraceAnalyzer.hpp"
#include "TraceExtractor.hpp"
#include "WaveformAnalyzer.hpp"
#include "GetArguments.hpp"

using namespace std;
using namespace dammIds::raw;

/*!
  detector driver constructor

  Creates instances of all event processors
*/

using namespace dammIds::raw;

DetectorDriver* DetectorDriver::instance = NULL;

/** Instance is created upon first call */
DetectorDriver* DetectorDriver::get() {
    if (!instance) {
        instance = new DetectorDriver();
    }
    return instance;
}

DetectorDriver::DetectorDriver() : histo(OFFSET, RANGE) 
{
    is_init = false;
    root_fname = "";
    num_files = 0;
    
    // Get the arguments passed to fortran (max length of 128 characters per argument)
    // I had no idea we could do this, woooooo!!!! CRT
    char arg[128];
    for(int i = 0; i <= GetNumberArguments(); i++){
        GetArgument(i, arg, 128);
        AppendArgument(arg, 128);
    }
    
    std::cout << "DetectorDriver: Loading Analyzers\n";
    if(!HasArgument("pfit-off") || HasArgument("dcfd-on")){
        vecAnalyzer.push_back(new WaveformAnalyzer());
        std::cout << "DetectorDriver: Using WaveformAnalyzer\n";
        if(!HasArgument("pfit-off")){ 
            vecAnalyzer.push_back(new FittingAnalyzer()); 
            std::cout << "DetectorDriver: FittingAnalyzer is active\n";
        }
        if(HasArgument("dcfd-on")){ 
            vecAnalyzer.push_back(new CfdAnalyzer()); 
            std::cout << "DetectorDriver: CfdAnalyzer is active\n";
        }
    }
    
    // Writing of raw waveforms is disabled by default
    bool save_wave = false;
    if(HasArgument("waveform")){ save_wave = true; }

    cout << "DetectorDriver: Loading Processors\n";
    vecProcess.push_back(new BetaProcessor(save_wave)); // For beam scintillator
    vecProcess.push_back(new LiquidProcessor(save_wave)); // For UofM liquid array
    vecProcess.push_back(new LogicProcessor()); // For scalers
    vecProcess.push_back(new VandleProcessor()); // For Vandle bar array

    // ROOT output is ON by default!
    if(!HasArgument("root-off")){ 
    	use_root = true; 
    	root_fname = arguments[1];
    	std::cout << "DetectorDriver: Using ROOT output\n";
    	OpenNewFile();
    }
    else{ use_root = false; }
    
    // DAMM output is OFF by default!
    if(HasArgument("damm-on")){ 
    	use_damm = true; 
    	std::cout << "DetectorDriver: Using DAMM output\n";
    }
    else{ use_damm = false; }

    if(!use_root && !use_damm){ std::cout << "DetectorDriver: Warning! Neither output method is turned on\n"; }

    num_events = 0;
    num_fills = 0;
}

/*!
  detector driver deconstructor

  frees memory for all event processors
 */
DetectorDriver::~DetectorDriver()
{
    this->Delete();
}

bool DetectorDriver::Delete()
{
    if(!is_init){
        std::cout << "DetectorDriver: Warning! Not initialized\n";
        return false;
    }
    
    // Write root tree to file
    if(use_root){
        std::cout << "DetectorDriver: Writing TTree to file with " << masterTree->GetEntries() << " entries...";
        masterFile->cd();
        masterTree->Write();
        
        unsigned int filesize = masterFile->GetSize();
        masterFile->Close();
        delete masterFile;
        std::cout << " done\n";
        std::cout << "DetectorDriver: Wrote " << filesize << " bytes to file\n";
    }    
    
    std::cout << "DetectorDriver: Cleaning up\n";
    std::cout << "DetectorDriver: Found " << num_events << " total events\n";
    std::cout << "DetectorDriver: Wrote " << num_fills << " (" << 100.0*num_fills/num_events << "%) events to file\n";

    // Iterate over processors and delete them
    if(vecProcess.size() > 0){ 
        std::cout << "DetectorDriver: Killing processors\n";
        for (vector<EventProcessor *>::iterator it = vecProcess.begin(); it != vecProcess.end(); it++) {
            (*it)->Status(num_fills);
            delete *it;
        }
    }
    vecProcess.clear();

    // Iterate over analyzers and delete them
    if(vecAnalyzer.size() > 0){ 
        std::cout << "DetectorDriver: Killing analyzers\n";
        for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin(); it != vecAnalyzer.end(); it++) {
            delete *it;
        }
    }
    vecAnalyzer.clear();

    return true;
}

/*!
  Called from PixieStd.cpp during initialization.
  The calibration file cal.txt is read using the function ReadCal() and 
  checked to make sure that all channels have a calibration.
*/
bool DetectorDriver::Init(RawEvent& rawev)
{
    if(is_init){
        std::cout << "DetectorDriver: Warning! Already initialized\n";
        return false;
    }
    std::cout << "DetectorDriver: Initializing\n";
    
    // initialize the trace analysis routine
    for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin(); it != vecAnalyzer.end(); it++) {
	(*it)->Init();
    	if(use_damm){ (*it)->InitDamm(); }
	(*it)->SetLevel(20); //! Plot traces
    }

    // initialize processors in the event processing vector
    for (vector<EventProcessor *>::iterator it = vecProcess.begin(); it != vecProcess.end(); it++) {
    	(*it)->Init(rawev); // Initialize EventProcessor	
    	if(use_damm){ (*it)->InitDamm(); }
    }

    /*
      Read in the calibration parameters from the file cal.txt
    */
    ReadCal();

    TimingInformation readFiles;
    readFiles.ReadTimingConstants();
    readFiles.ReadTimingCalibration();

    rawev.GetCorrelator().Init(rawev);
    is_init = true;
    return true;
}

bool DetectorDriver::HasArgument(std::string input){
    for(std::vector<std::string>::iterator iter = arguments.begin(); iter != arguments.end(); iter++){
        if(input == *iter){ return true; }
    }
    return false;
}

std::string DetectorDriver::AppendArgument(char* input, unsigned int size){
    std::string arg = "";
    for(unsigned int i = 0; i < size; i++){
    	if(input[i] != ' '){ arg += input[i]; }
    	else{ break; }
    }
    arguments.push_back(arg);
    return arg;
}

bool DetectorDriver::OpenNewFile(){
    if(!use_root){
        std::cout << "DetectorDriver: Warning! Cannot open Root file, Root usage is not enabled\n";
        std::cout << "DetectorDriver: Did you pass the root-off flag at the command line?\n";
        return false;
    }
    
    // Get the new file name
    std::string current_fname = "";
    if(num_files == 0){ current_fname = root_fname + ".root"; } // root_fname.root
    else{
        if(num_files < 10){ current_fname = root_fname + "_0" + to_str(num_files) + ".root"; } // root_fname_01.root
        else{ current_fname = root_fname + "_" + to_str(num_files) + ".root"; } // root_fname_10.root
        
        // Since num_files > 0, there is already a file open. It needs to be closed.
        std::cout << "DetectorDriver: Writing TTree to file with " << masterTree->GetEntries() << " entries...";
        masterFile->cd();
        masterTree->Write();
        
        unsigned int filesize = masterFile->GetSize();        
        masterFile->Close();
        delete masterFile; // Also deleting masterTree will cause a segfault!
        std::cout << " done\n";
        std::cout << "DetectorDriver: Wrote " << filesize << " bytes to file\n";
    }
    
    // Open the new file and create the tree
    std::cout << "DetectorDriver: Opening file '" << current_fname << "'\n";
    masterFile = new TFile(current_fname.c_str(), "RECREATE"); // Will overwrite the file!
    masterTree = new TTree("Pixie16","Pixie analysis tree");

    if(num_files == 0){
        // Add analyzer branches to root tree
        /*for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin(); it != vecAnalyzer.end(); it++) {
            std::cout << " " << (*it)->GetName() << "Analyzer: Initializing root output\n";
            if(!(*it)->InitRoot(masterTree)){ std::cout << " " << (*it)->GetName() << "Analyzer: Warning! Failed to add branch\n"; }
        }*/

        // Add processor branches to root tree
        for (vector<EventProcessor *>::iterator it = vecProcess.begin(); it != vecProcess.end(); it++) {
            std::cout << " " << (*it)->GetName() << "Processor: Initializing root output\n";
            if(!(*it)->InitRoot(masterTree)){ std::cout << " " << (*it)->GetName() << "Processor: Warning! Failed to add branch\n"; }
        }
    }
    else{
        // Add analyzer branches to root tree
        /*for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin(); it != vecAnalyzer.end(); it++) {
            (*it)->InitRoot(masterTree)
        }*/

        // Add processor branches to root tree
        for (vector<EventProcessor *>::iterator it = vecProcess.begin(); it != vecProcess.end(); it++) {
            (*it)->InitRoot(masterTree);
        }
    }    
    
    num_files++;
    if(!masterFile){ return false; }
    return true;
}

/*!
  \brief controls event processing

  The ProcessEvent() function is called from ScanList() in PixieStd.cpp
  after an event has been constructed. This function is passed the mode
  the analysis is currently in (the options are either "scan" or
  "standaloneroot").  The function checks the thresholds for the individual
  channels in the event and calibrates their energies. 
  The raw and calibrated energies are plotted if the appropriate DAMM spectra
  have been created.  Then experiment specific processing is performed.  
  Currently, both RMS and MTC processing is available.  After all processing
  has occured, appropriate plotting routines are called.
*/
int DetectorDriver::ProcessEvent(const string &mode, RawEvent& rawev){   
    /*
      Begin the event processing looping over all the channels
      that fired in this particular event.
    */
    plot(dammIds::raw::D_NUMBER_OF_EVENTS, dammIds::GENERIC_CHANNEL);
    num_events++; // Count the number of raw events
    
    for (vector<ChanEvent*>::const_iterator it = rawev.GetEventList().begin(); it != rawev.GetEventList().end(); ++it) {
        string place = (*it)->GetChanID().GetPlaceName();
        if (place == "__-1") // empty channel
            continue;
        
        ThreshAndCal((*it), rawev); // check threshold and calibrate
        if(use_damm){ 
            PlotRaw((*it));
            PlotCal((*it));
        }

        double time = (*it)->GetTime();
        double energy = (*it)->GetCalEnergy();
        CorrEventData data(time, energy);
        TreeCorrelator::get()->place(place)->activate(data);
    } 

    // have each processor in the event processing vector handle the event
    /* First round is preprocessing, where process result must be guaranteed
     * to not to be dependent on results of other Processors. */
    // Zero the branch first and mark it as invalid. Processors with valid data should fill
    // their own branches by calling their PackRoot() routine internally.
    bool has_event = false;
    for (vector<EventProcessor*>::iterator iProc = vecProcess.begin(); iProc != vecProcess.end(); iProc++) {
        if(use_root){ (*iProc)->Zero(); } // Zero the structure in preparation for processing, mark entry as invalid (valid=false)
        if ( (*iProc)->HasEvent() ) { 
            if((*iProc)->PreProcess(rawev) && !has_event){ has_event = true; }
        }
    }
    /* In the second round the Process is called, which may depend on other
     * Processors. */
    for (vector<EventProcessor *>::iterator iProc = vecProcess.begin(); iProc != vecProcess.end(); iProc++) {
        if ( (*iProc)->HasEvent() ) { 
            if((*iProc)->Process(rawev) && !has_event){ has_event = true; }
        } 
    }
    
    // Fill all processor branches for each event (even if they are invalid)
    if(use_root && has_event){ 
        masterTree->Fill(); 
        num_fills++; // Count the number of tree fills
        
        // Limit root file size to roughly 2 GB
        if(masterFile->GetSize() >= 2147483648){ OpenNewFile(); }
    } 

    return 0;   
}

// declare plots for all the event processors
void DetectorDriver::DeclarePlots(MapFile& theMapFile)
{
    DetectorLibrary* modChan = DetectorLibrary::get();
    DetectorLibrary::size_type maxChan = (theMapFile ? modChan->size() : 192);
    
    if(use_damm){ // Declare plots for each channel
        for (vector<TraceAnalyzer *>::const_iterator it = vecAnalyzer.begin(); it != vecAnalyzer.end(); it++) { (*it)->InitDamm(); }
        for (vector<EventProcessor *>::const_iterator it = vecProcess.begin(); it != vecProcess.end(); it++) { (*it)->InitDamm(); }
        
        DeclareHistogram1D(D_HIT_SPECTRUM, S7, "channel hit spectrum");
        DeclareHistogram1D(D_SUBEVENT_GAP, SE, "time btwn chan-in event,10ns bin");
        DeclareHistogram1D(D_EVENT_LENGTH, SE, "time length of event, 10 ns bin");
        DeclareHistogram1D(D_EVENT_GAP, SE, "time between events, 10 ns bin");
        DeclareHistogram1D(D_EVENT_MULTIPLICITY, S7, "number of channels in event");
        DeclareHistogram1D(D_BUFFER_END_TIME, SE, "length of buffer, 1 ms bin");
        DeclareHistogram2D(DD_RUNTIME_SEC, SE, S6, "run time - s");
        DeclareHistogram2D(DD_DEAD_TIME_CUMUL, SE, S6, "dead time - cumul");
        DeclareHistogram2D(DD_BUFFER_START_TIME, SE, S6, "dead time - 0.1%");
        DeclareHistogram2D(DD_RUNTIME_MSEC, SE, S7, "run time - ms");
        DeclareHistogram1D(D_NUMBER_OF_EVENTS, S4, "event counter");
    }
    DeclareHistogram1D(D_HAS_TRACE, S7, "channels with traces");

    for (DetectorLibrary::size_type i = 0; i < maxChan; i++) {	 
        if (theMapFile && !modChan->HasValue(i)) {
            continue;
        }
        stringstream idstr; 
        
        if (theMapFile) {
            const Identifier &id = modChan->at(i);

            idstr << "M" << modChan->ModuleFromIndex(i) << " C" << modChan->ChannelFromIndex(i)
            << " - " << id.GetType() << ":" << id.GetSubtype() << " L" << id.GetLocation();
        } else {
            idstr << "id " << i;
        }
        
        if(use_damm){
            DeclareHistogram1D(D_RAW_ENERGY + i, SE, ("RawE " + idstr.str()).c_str() );
            DeclareHistogram1D(D_FILTER_ENERGY + i, SE, ("FilterE " + idstr.str()).c_str() );
            DeclareHistogram1D(D_SCALAR + i, SE, ("Scalar " + idstr.str()).c_str() );
#if !defined(REVD) && !defined(REVF)
            DeclareHistogram1D(D_TIME + i, SE, ("Time " + idstr.str()).c_str() ); 
#endif
            DeclareHistogram1D(D_CAL_ENERGY + i, SE, ("CalE " + idstr.str()).c_str() );
            DeclareHistogram1D(D_CAL_ENERGY_REJECT + i, SE, ("CalE NoSat " + idstr.str()).c_str() );
        }
    }
}

// sanity check for all our expectations
bool DetectorDriver::SanityCheck(void) const
{
    return true;
}

/*!
  \brief check threshold and calibrate each channel.

  Check the thresholds and calibrate the energy for each channel using the
  calibrations contained in the calibration vector filled during ReadCal()
*/

int DetectorDriver::ThreshAndCal(ChanEvent *chan, RawEvent& rawev)
{   
    // retrieve information about the channel
    Identifier chanId = chan->GetChanID();
    int id = chan->GetID();
    string type = chanId.GetType();
    string subtype = chanId.GetSubtype();
    bool hasStartTag = chanId.HasTag("start");
    Trace &trace = chan->GetTrace();

    RandomPool* randoms = RandomPool::get();

    double energy = 0.;

    if (type == "ignore" || type == "") {
        return 0;
    }
    /*
      If the channel has a trace get it, analyze it and set the energy.
    */
    if ( !trace.empty() ) {
        if(use_damm){ plot(D_HAS_TRACE, id); }

	for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin(); it != vecAnalyzer.end(); it++) {	
            (*it)->Analyze(trace, type, subtype);
	}

	if (trace.HasValue("filterEnergy") ) {     
	    if (trace.GetValue("filterEnergy") > 0) {
                energy = trace.GetValue("filterEnergy");
                if(use_damm){ plot(D_FILTER_ENERGY + id, energy); }
	    } else {
            energy = 2;
	    }
	}
	if (trace.HasValue("calcEnergy") ) {	    
	    energy = trace.GetValue("calcEnergy");
	    chan->SetEnergy(energy);
	} else if (!trace.HasValue("filterEnergy")) {
	    energy = chan->GetEnergy() + randoms->Get();
	    energy /= ChanEvent::pixieEnergyContraction;
	}
	if (trace.HasValue("phase") ) {
	    double phase = trace.GetValue("phase");
	    chan->SetHighResTime( phase * pixie::adcClockInSeconds + chan->GetTrigTime() * pixie::filterClockInSeconds);
	}
    } else {
	// otherwise, use the Pixie on-board calculated energy
	// add a random number to convert an integer value to a 
	//   uniformly distributed floating point

	energy = chan->GetEnergy() + randoms->Get();
	energy /= ChanEvent::pixieEnergyContraction;
    }
    /*
      Set the calibrated energy for this channel
    */
    chan->SetCalEnergy( cal[id].Calibrate(energy) );

    /*
      update the detector summary
    */
    rawev.GetSummary(type)->AddEvent(chan);
    DetectorSummary *summary;
    
    summary = rawev.GetSummary(type + ':' + subtype, false);
    if (summary != NULL)
	summary->AddEvent(chan);

    if(hasStartTag) {
	summary = rawev.GetSummary(type + ':' + subtype + ':' + "start", false);
	if (summary != NULL)
	    summary->AddEvent(chan);
    }
    
    return 1;
}

/*!
  Plot the raw energies of each channel into the damm spectrum number assigned
  to it in the map file with an offset as defined in DammPlotIds.hpp
*/
int DetectorDriver::PlotRaw(const ChanEvent *chan)
{
    int id = chan->GetID();
    float energy = chan->GetEnergy() / ChanEvent::pixieEnergyContraction;
    
    plot(D_RAW_ENERGY + id, energy);
    
    return 0;
}

/*!
  Plot the calibrated energies of each channel into the damm spectrum number
  assigned to it in the map file with an offset as defined in DammPlotIds.hpp
*/
int DetectorDriver::PlotCal(const ChanEvent *chan)
{
    int id = chan->GetID();
    // int dammid = chan->GetChanID().GetDammID();
    float calEnergy = chan->GetCalEnergy();
    
    plot(D_CAL_ENERGY + id, calEnergy);
    if (!chan->IsSaturated() && !chan->IsPileup())
        plot(D_CAL_ENERGY_REJECT + id, calEnergy);
    return 0;
}

vector<EventProcessor *> DetectorDriver::GetProcessors(const string& type) const
{
  vector<EventProcessor *> retVec;

  for (vector<EventProcessor *>::const_iterator it = vecProcess.begin(); it != vecProcess.end(); it++) {
    if ( (*it)->GetTypes().count(type) > 0 )
      retVec.push_back(*it);
  }

  return retVec;
}

/*!
  Read in the calibration for each channel according to the data in cal.txt
*/
void DetectorDriver::ReadCal()
{
    /*
      The file cal.txt contains the calibration for each channel.  The first
      five variables describe the detector's physical location (strip number,
      detector number, ...), the detector type, the detector subtype, the number
      of calibrations, and their polynomial order.  Using this information, the
      rest of a channel's calibration is read in as -- lower threshold for the 
      current calibration, followed by the polynomial constants in increasing
      polynomial order.  The lower thresholds and polynomial values are read in
      for each distinct calibration specified by the number of calibrations.
    */

    // lookup table for information from map.txt (from PixieStd.cpp)
    DetectorLibrary* modChan = DetectorLibrary::get();
    Identifier lookupID;

    /*
      Values used to read in the thresholds and polynomials from cal.txt
      The numbers can not be read directly into the vectors
    */
    float thresh;
    float val;

    /*
      The channels module number, channel number, detector location
      the number of calibrations, polynomial order, if the detector
      should be ignored, the detector type and subtype.
    */
    unsigned int detLocation;
    string detType, detSubtype;

    PathHolder* conf_path = new PathHolder();
    string calFilename = conf_path->GetFullPath("cal.txt");
    delete conf_path;

    ifstream calFile(calFilename.c_str());

    // make sure there is a generic calibration for each channel in the map
    cal.resize(modChan->size());

    if (!calFile) {
        throw IOException("Could not open file " + calFilename);
    } else {
      cout << "Reading in calibrations from " << calFilename << endl;
      while (calFile) {
            /*
              While the end of the calibration file has not been reached,
              increment the number of lines read and if the first input on a
	      line is a number, read in the first five parameters for a channel
            */
            if ( isdigit(calFile.peek()) ) {
                calFile >> detLocation >> detType >> detSubtype;
		lookupID.SetLocation(detLocation);
		lookupID.SetType(detType);
		lookupID.SetSubtype(detSubtype);
		// find the identifier in the map
		DetectorLibrary::iterator mapIt = find(modChan->begin(), modChan->end(), lookupID); 
		if (mapIt == modChan->end()) {
		    cout << "Can not match detector type " << detType << " and subtype " << detSubtype << " with location " << detLocation << " to a channel in the map.\n";
		    exit(EXIT_FAILURE);
		}
		size_t id = distance(modChan->begin(), mapIt);
		Calibration &detCal = cal.at(id);
		//? make this a member function of Calibration
		detCal.id = id;
		calFile	>> detCal.numCal >> detCal.polyOrder;
		detCal.thresh.clear();
		detCal.val.clear();
		detCal.detLocation = detLocation;
		detCal.detType = detType;
		detCal.detSubtype = detSubtype;
                /*
                  For the number of calibrations read in the
                  thresholds and the polynomial values
                */
                for (unsigned int i = 0; i < detCal.numCal; i++) {
                    calFile >> thresh;
                    detCal.thresh.push_back(thresh);

                    for(unsigned int j = 0; j < detCal.polyOrder+1; j++){
                        /*
                          For the calibration order, read in the polynomial 
                          constants in ascending order
                        */
                        calFile >> val;
			detCal.val.push_back(val);
                    } // finish looping on polynomial order
                } // finish looping on number of calibrations

                /*
                  Add the value of MAX_PAR from the Globals.hpp file
                  as the upper limit of all calibrations
                */
                detCal.thresh.push_back(MAX_PAR);
            } else {
                // this is a comment, skip line 
                calFile.ignore(1000,'\n');
            }            
        } // end while (!calFile) loop - end reading cal.txt file
    }
    calFile.close();

    // check to make sure every channel has a calibration, no default
    //   calibration is allowed
    DetectorLibrary::const_iterator mapIt = modChan->begin();
    vector<Calibration>::iterator calIt = cal.begin();
    for (;mapIt != modChan->end(); mapIt++, calIt++) {
	string type = mapIt->GetType();
	if (type == "ignore" || type == "") {
	    continue;
	};
	if (calIt->detType!= type) {
	    if (mapIt->HasTag("uncal")) {
		// set the remaining fields properly
		calIt->detType = type;
		calIt->detSubtype = mapIt->GetSubtype();
		calIt->detLocation = mapIt->GetLocation(); 
		continue;
	    }
	    cout << "Uncalibrated detector found for type " << type  << " at location " << mapIt->GetLocation() << ". No default calibration is given, please correct.\n";
	    exit(EXIT_FAILURE);
	}
    }
    /*
      Print the calibration values that have been read in
    */
    if (verbose::CALIBRATION_INIT) {
        cout << setw(4)  << "mod" << setw(4)  << "ch" << setw(4)  << "loc" << setw(10) << "type" << setw(8)  << "subtype"
        << setw(5)  << "cals" << setw(6)  << "order"  << setw(31) << "cal values: low-high thresh, coeffs" << endl;
    
        //? calibration print command?
        for(size_t a = 0; a < cal.size(); a++){
        cout << setw(4)  << int(a/16) << setw(4)  << (a % 16) << setw(4)  << cal[a].detLocation  << setw(10) << cal[a].detType
        << setw(8)  << cal[a].detSubtype  << setw(5)  << cal[a].numCal << setw(6)  << cal[a].polyOrder;      
            for(unsigned int b = 0; b < cal[a].numCal; b++){
                cout << setw(6) << cal[a].thresh[b];
                cout << " - " << setw(6) << cal[a].thresh[b+1];
                for(unsigned int c = 0; c < cal[a].polyOrder+1; c++){
                    cout << setw(7) << setprecision(5) << cal[a].val[b*(cal[a].polyOrder+1)+c];
                }
            }
            cout << endl;
        }
    }
}

/*!
  Construct calibration parameters using Zero() method
*/
Calibration::Calibration() : 
    id(-1), detType(""), detSubtype(""), detLocation(-1),
    numCal(1), polyOrder(1)
{
    thresh.push_back(0);
    thresh.push_back(MAX_PAR);
    // simple linear calibration
    val.push_back(0); // constant coeff
    val.push_back(1); // coeff linear in raw energy
}

double Calibration::Calibrate(double raw)
{
    /*
      Make sure we don't have any calibration values below the lowest
      calibration theshold or any calibrated energies above the 
      maximum threshold value set in cal.txt
    */
    if(raw < thresh[0]) {
        return 0;
    } 

    if(raw >= thresh[numCal]) {
	return thresh[numCal] - 1;
    }

    double calVal = 0;
    /*
      Begin threshold check and calibration, first
      loop over the number of calibrations
    */
    for(unsigned int a = 0; a < numCal; a++) {
        //check to see if energy falls in this calibration range
        if (raw >= thresh[a] && raw < thresh[a+1]) {
            //loop over the polynomial order
            for(unsigned int b = 0; b < polyOrder+1; b++) {
		calVal += pow(raw,(double)b) * val[a*(polyOrder+1) + b];
            }
	    break;
        }
    }

    return calVal;
}
