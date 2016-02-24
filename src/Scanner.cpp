#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <vector>

#include <cstring>
#include <ctime>

#include <unistd.h>
#include <sys/times.h>

#include "pixie16app_defs.h"

// PixieCore libraries
#include "Unpacker.hpp" 
#include "ScanMain.hpp"
#include "PixieEvent.hpp"

// Local files
#include "Scanner.hpp"
#include "RawEvent.hpp"
#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "TreeCorrelator.hpp"
#include "DammPlotIds.hpp"

using namespace std;
using namespace dammIds::raw;

// Contains event information, the information is filled in ScanList() and is
// referenced in DetectorDriver.cpp, particularly in ProcessEvent().
RawEvent rawev;

/// Process all events in the event list. Replaces hissub_sec/ScanList
void Scanner::ProcessRawEvent(){
    DetectorDriver* driver = DetectorDriver::get();
    DetectorLibrary* modChan = DetectorLibrary::get();
    PixieEvent *current_event = NULL;

    static clock_t clockBegin; // initialization time
    static struct tms tmsBegin;
    
    // local variable for the detectors used in a given event
    set<string> usedDetectors;
    stringstream ss;
    
    // Initialize the scan program before the first event
    if(counter == 0){
	// Retrieve the current time for use later to determine the total running time of the analysis.
	clockBegin = times(&tmsBegin);
	ss << "First buffer at " << clockBegin << " sys time";
	ss.str("");
	
	// After completion the descriptions of all channels are in the modChan
	// vector, the DetectorDriver and rawevent have been initialized with the
	// detectors that will be used in this analysis.
	modChan->PrintUsedDetectors(rawev);
	driver->Init(rawev);
	
	ss << "Init at " << times(&tmsBegin) << " sys time.";
	
    cout << "Using event width " << pixie::eventInSeconds * 1e6 << " us" << endl;
    cout << "                  " << pixie::eventWidth << " in pixie16 clock tics." << endl;
    } //if(counter == 0)

    //BEGIN SCANLIST PART
    
    /** Rejection regions defined here*/
    
    ///The initial event
    deque<PixieEvent*>::iterator iEvent = rawEvent.begin();

    // local variables for the times of the current event, previous
    // event and time difference between the two
    double diffTime = 0;
    //set last_t to the time of the first event
    double lastTime = (*iEvent)->time;
    double currTime = lastTime;
    unsigned int id = (*iEvent)->getID();

    /* KM
     * Save time of the beginning of the file,
     * this is needed for the rejection regions */
    bool IsFirstEvt = true;
    bool IsLastEvt = false;

    //loop over the list of channels that fired in this buffer
    while(!rawEvent.empty()) {
	///Check if this is the last event in the deque.
	if(rawEvent.size() == 1) 
	    IsLastEvt = true;
	
	current_event = rawEvent.front();
	rawEvent.pop_front(); // Remove this event from the raw event deque.

	// Check that this channel event exists.
	if(!current_event)
	    continue;

	///Completely ignore any channel that is set to be ignored
	if (id == 0xFFFFFFFF) {
            ss << "pattern 0 ignore";
            ss.str("");
            continue;
        }
	if ((*modChan)[id].GetType() == "ignore")
            continue;
	
	// Do something with the current event.
	ChanEvent *event = new ChanEvent(current_event);
	
	//calculate some of the parameters of interest
	id = event->GetID();
        /* retrieve the current event time and determine the time difference
        between the current and previous events.
        */
        currTime = event->GetTime();
        diffTime = currTime - lastTime;

	//Add the ChanEvent pointer to the rawev and used detectors.
	usedDetectors.insert((*modChan)[id].GetType());
	rawev.AddChan(event);

	if(IsFirstEvt) {
	    /* KM
	     * Save time of the beginning of the file,
	     * this is needed for the rejection regions */
	    IsFirstEvt = false;
	}else if(IsLastEvt) {
	    string mode;
	    
	    driver->ProcessEvent(rawev);
	    rawev.Zero(usedDetectors);
	}else
        
	//REJECTION REGIONS WOULD GO HERE

        /* if the time difference between the current and previous event is
        larger than the event width, finalize the current event, otherwise
        treat this as part of the current event
        */
        if ( diffTime > pixie::eventWidth ) {
            if(rawev.Size() > 0) {
            /* detector driver accesses rawevent externally in order to
            have access to proper detector_summaries
            */
                driver->ProcessEvent(rawev);
            }

            //after processing zero the rawevent variable
            rawev.Zero(usedDetectors);
            usedDetectors.clear();

            // Now clear all places in correlator (if resetable type)
            for (map<string, Place*>::iterator it =
                    TreeCorrelator::get()->places_.begin();
                it != TreeCorrelator::get()->places_.end(); ++it)
                if ((*it).second->resetable())
                    (*it).second->reset();
        }

	//DTIME STUFF GOES HERE

        // update the time of the last event
        lastTime = currTime;
    }//while(!rawEvent.empty())
    counter++;
}

Scanner::Scanner(){
    output_fname = "output";
}

Scanner::~Scanner(){
    Close(); // Close the Unpacker object.
}

/// Initialize the map file, the config file, the processor handler, and add all of the required processors.
bool Scanner::Initialize(std::string prefix_){
    if(init)
	return(false);
    counter = 0;
    
    return(init = true);
}

/// Return the syntax string for this program.
void Scanner::SyntaxStr(const char *name_, std::string prefix_){ 
    std::cout << prefix_ << "SYNTAX: " << std::string(name_) << " [input-fname] <options> <output-prefix>\n"; 
}       

/** 
 *	\param[in] prefix_ 
 */
void Scanner::CmdHelp(std::string prefix_){
    std::cout << prefix_ << "flush - Flush histogram entries to file.\n";
    std::cout << prefix_ << "zero  - Zero the output histogram (SLOW! Be patient...).\n";
}

/**
 * \param[in] args_
 * \param[out] filename
 */
bool Scanner::SetArgs(std::deque<std::string> &args_, std::string &filename){
    std::string current_arg;
    int count = 0;
    while(!args_.empty()){
	current_arg = args_.front();
	args_.pop_front();
	if(count == 0){ filename = current_arg; } // Set the input filename.
	else if(count == 1){ output_fname = current_arg; } // Set the output filename prefix.
	count++;
    }
    
    return true;
}

/** Search for an input command and perform the desired action.
 * 
 * \return True if the command is valid and false otherwise.
 */
bool Scanner::CommandControl(std::string cmd_, const std::vector<std::string> &args_){
    return false;
}

int main(int argc, char *argv[]){
    // Define a new unpacker object.
    Unpacker *scanner = (Unpacker*)(new Scanner());
    
    // Setup the ScanMain object and link it to the unpacker object.
    ScanMain scan_main(scanner);
    
    // Link the unpacker object back to the ScanMain object so we may
    // access its command line arguments and options.
    scanner->SetScanMain(&scan_main);
    
    // Set the output message prefix.
    scan_main.SetMessageHeader("pixie_ldf_c: ");
    
    // Run the main loop.
    return scan_main.Execute(argc, argv);
}
