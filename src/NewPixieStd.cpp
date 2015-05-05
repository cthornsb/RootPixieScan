/** \file NewPixieStd.cpp
 *
 * \brief pixie_std provides the interface between the HRIBF scan 
 * and the C++ analysis
 *
 * This provides the interface between the HRIBF scan and the C++ analysis
 * and as such is not a class in its own right.  In this file the data
 * received from scan is first reassembled into a pixie16 spill and then
 * channel objects are made.
 *
 * The main program.  Buffers are passed to hissub_() and channel information
 * is extracted in ReadBuffData(). All channels that fired are stored as a
 * vector of pointers which is sorted based on time and then events are built
 * with each event being sent to the detector driver for processing.
 *
 * \author S. Liddick 
 * \date 20 July 2007 
 *
 * <strong> Modified : </strong> <br>
 * S. Liddick - 2-5-08 - Added in diagnostic spectra including:
 *   runtime, channel time difference in an event, time difference between
 *   events, length of event, and length of buffer <br>
 * 
 * S. Liddick - 5-14-08 - At SP's request, error message and termination occur if a
 *   module number is encountered in the data stream that is not included in
 *   the map.txt file <br>
 *
 * David Miller - 5-5-10 - Significant changes throughout for conciseness, 
 *   optimization, and better error checking of incoming buffers
 *
 * Cory Thornsberry - 5-1-15
 */

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <cstring>
#include <ctime>

#include <unistd.h>
#include <sys/times.h>

#include "pixie16app_defs.h"

#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "DetectorSummary.hpp"
#include "ChanEvent.hpp"
#include "RawEvent.hpp"
#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "Plots.hpp"
#include "PlotsRegister.hpp"
#include "TreeCorrelator.hpp"

#include "NewPixieStd.hpp"

using namespace dammIds::raw;
using pixie::word_t;

#define VERBOSE

/**
 * Contains event information, the information is filled in ScanList() and is 
 * referenced in DetectorDriver.cpp, particularly in ProcessEvent()
 */
RawEvent rawev;

const string scanMode = "scan";

/** In a typical experiment, Pixie16 reads data from all modules when one module
 * has hit the maximum number of events which is programmed during experimental
 * setup.  This spill of data is then broken into smaller chunks for
 * transmitting across the network.  The hissub_ function takes the chunks
 * and reconstructs the spill.
 *
 * Summarizing the terminology:
 *  - Spill  - a readout of all Pixie16 modules
 *  - Buffer - the data from a specific Pixie16 module
 *  - Chunk  - the packet transferred from Pixie16 to the acquisition
*/

// THIS SHOULD NOT BE SET LARGER THAN 1,000,000
//  this defines the maximum amount of data that will be received in a spill
const unsigned int TOTALREAD = 1000000;

#if defined(REVD) || defined(REVF)
const unsigned int maxWords = EXTERNAL_FIFO_LENGTH; //Revision D
#else
const unsigned int maxWords = IO_BUFFER_LENGTH; // Revision A
#endif

// Clean up the c++ objects
void cleanup(){
	std::cout << "\nCleaning up..\n";
	DetectorDriver* driver = DetectorDriver::get();
	driver->Delete();
}

/**
 * If the new Pixie16 readout is used (default), this routine processes the
 * reconstructed buffer.  Specifically, it retrieves channel information
 * and places the channel information into a list of channels that triggered in
 * this spill.  The list of channels is sorted according to the event time
 * assigned to each channel by Pixie16 and the sorted list is passed to
 * ScanList() for raw event creation. 
 *
 * If the old pixie readout is used then this function is
 * redefined as hissub_.
 */
bool ReadSpill(char *ibuf, unsigned int nWords){
	const unsigned int maxVsn = 14; // No more than 14 pixie modules per crate
	unsigned int nWords_read = 0;
	
	static clock_t clockBegin; // Initialization time
	static struct tms tmsBegin;

	vector<ChanEvent*> eventList; // Vector to hold the events

	// Pointer to singleton DetectorLibrary class 
	DetectorLibrary* modChan = DetectorLibrary::get();
	
	// Pointer to singleton DetectorDriver class 
	DetectorDriver* driver = DetectorDriver::get();

	// Local version of ibuf pointer
	word_t *data = (word_t *)ibuf;

	int retval = 0; // return value from various functions
	
	unsigned long bufLen;
	
	// Various event counters 
	unsigned long numEvents = 0;
	static int counter = 0; // the number of times this function is called
	static int evCount;	 // the number of times data is passed to ScanList
	static unsigned int lastVsn; // the last vsn read from the data
	time_t theTime = 0;

	// Initialize the scan program before the first event 
	if (counter==0) {
		// Retrieve the current time for use later to determine the total
		// running time of the analysis.
		clockBegin = times(&tmsBegin);

		std::cout << "\nFirst buffer at " << clockBegin << " sys time" << std::endl;
		// After completion the descriptions of all channels are in the modChan
		// vector, the DetectorDriver and rawevent have been initialized with the
		// detectors that will be used in this analysis.
		std::cout << "Using event width " << pixie::eventInSeconds * 1e6 << " us (" << pixie::eventWidth << " in pixie16 clock tics).\n\n";

		modChan->PrintUsedDetectors(rawev);
		if (verbose::MAP_INIT)
			modChan->PrintMap();

		driver->Init(rawev);

		// Make a last check to see that everything is in order for the driver 
		// before processing data
		if ( !driver->SanityCheck() ) {
			std::cout << "Detector driver did not pass sanity check!" << std::endl;
			return false;
		}

		lastVsn=-1; // Set last vsn to -1 so we expect vsn 0 first 	

		std::cout << "Init done at " << times(&tmsBegin) << " sys time.\n\n";
	}
	counter++;
 
	word_t lenRec = U_DELIMITER;
	word_t vsn = U_DELIMITER;
	bool fullSpill=false; // True if spill had all vsn's

	// While the current location in the buffer has not gone beyond the end
	// of the buffer (ignoring the last three delimiters, continue reading
	while (nWords_read <= nWords) {
		// Retrieve the record length and the vsn number
		lenRec = data[nWords_read]; // Number of words in this record
		vsn = data[nWords_read+1]; // Module number
	
		// Check sanity of record length and vsn
		if(lenRec > maxWords || (vsn > maxVsn && vsn != 9999 && vsn != 1000)) { 
#ifdef VERBOSE
			std::cout << "SANITY CHECK FAILED: lenRec = " << lenRec << ", vsn = " << vsn << ", read " << nWords_read << " of " << nWords << "\n";
#endif
			return false;  
		}

		// If the record length is 6, this is an empty channel.
		// Skip this vsn and continue with the next
		//! Revision specific, so move to ReadBuffData
		if (lenRec==6) {
			nWords_read += lenRec;
			lastVsn=vsn;
			continue;
		}

		// If both the current vsn inspected is within an acceptable 
		// range, begin reading the buffer.
		if ( vsn < modChan->GetPhysicalModules() ) {
			if ( lastVsn != U_DELIMITER ) {
				// the modules should be read out cyclically
				if ( ((lastVsn+1) % modChan->GetPhysicalModules()) != vsn ) {
#ifdef VERBOSE
					std::cout << " MISSING BUFFER " << vsn << "/" << modChan->GetPhysicalModules();
					std::cout << " -- lastVsn = " << lastVsn << "  " << ", length = " << lenRec << std::endl;
#endif
					RemoveList(eventList);
					fullSpill=true;
				}
			}
			
			// Read the buffer.  After read, the vector eventList will 
			//contain pointers to all channels that fired in this buffer
			retval= ReadBuffData(&data[nWords_read], &bufLen, eventList);

			// If the return value is less than the error code, 
			//reading the buffer failed for some reason.  
			//Print error message and reset variables if necessary
			if ( retval <= readbuff::ERROR ) {
				std::cout << " READOUT PROBLEM " << retval << " in event " << counter << std::endl;
				if ( retval == readbuff::ERROR ) {
					std::cout << "  Remove list " << lastVsn << " " << vsn << std::endl;
					RemoveList(eventList); 							
				}
				return false;
			} else if ( retval > 0 ) {		
				// Increment the total number of events observed 
				numEvents += retval;
			}
			
			// Update the variables that are keeping track of what has been
			// analyzed and increment the location in the current buffer
			lastVsn = vsn;
			nWords_read += lenRec;
		} 
		else if (vsn == clockVsn) { // Buffer with vsn 1000 was inserted with the time for superheavy exp't
			memcpy(&theTime, &data[nWords_read+2], sizeof(time_t));
#ifdef VERBOSE
			struct tm * timeinfo;
			timeinfo = localtime (&theTime);
			std::cout << " Read wall clock time of " << asctime(timeinfo);
#endif
			nWords_read += lenRec;
			continue;
		}
		else if(vsn == 9999){
			// End spill vsn
			break;
		}
		else{
#ifdef VERBOSE
			// Bail out if we have lost our place,		
			// (bad vsn) and process events	 
			std::cout << "UNEXPECTED VSN " << vsn << std::endl;
#endif
			break;
		}
	} // while still have words

	if(nWords > TOTALREAD || nWords_read > TOTALREAD) {
		std::cout << "Values of nn - " << nWords << " nk - "<< nWords_read << " TOTALREAD - " << TOTALREAD << std::endl;
		Pixie16Error(2); 
		return false;
	}

	/* If the vsn is 9999 this is the end of a spill, signal this buffer
	for processing and determine if the buffer is split between spills.
	*/
	if ( vsn == 9999 || vsn == clockVsn ) {
		fullSpill = true;
		nWords_read += 2; // Skip it
		lastVsn=U_DELIMITER;
	}

#ifdef VERBOSE
	// Check the number of read words
	if(nWords_read != nWords){
		std::cout << "Received spill of " << nWords << " words, but only read " << nWords_read << " words\n";
	}
#endif

	/* if there are events to process, continue */
	if( numEvents>0 ) {
		if (fullSpill) { // if full spill process events
			// sort the vector of pointers eventlist according to time
			double lastTimestamp = (*(eventList.rbegin()))->GetTime();

			sort(eventList.begin(),eventList.end(),CompareTime);
			driver->CorrelateClock(lastTimestamp, theTime);

			/* once the vector of pointers eventlist is sorted based on time,
			begin the event processing in ScanList()
			*/
			ScanList(eventList, rawev);

			/* once the eventlist has been scanned, remove it from memory
			and reset the number of events to zero and update the event
			counter
			*/
			evCount++;
			
			/*
			every once in a while (when evcount is a multiple of 1000)
			print the time elapsed doing the analysis
			*/
			if((evCount % 1000 == 0 || evCount == 1) && theTime != 0) {
				std::cout << "\n Data read up to poll status time " << ctime(&theTime);
			}		
			RemoveList(eventList);
			numEvents=0;
		} // end fullSpill 
		else {
			std::cout << "Spill split between buffers" << std::endl;
			return false; //! this tosses out all events read into the vector so far
		}		
	}  // end numEvents > 0
	else if (retval != readbuff::STATS) {
		std::cout << "bad buffer, numEvents = " << numEvents << std::endl;
		return false;
	}

	return true;	  
}

/** Remove events in list from memory when no longer needed */
void RemoveList(vector<ChanEvent*> &eventList){
	/*
	  using the iterator and starting from the beginning and going to 
	  the end of eventlist, delete the actual objects
	*/
	for(vector<ChanEvent*>::iterator it = eventList.begin();
	it != eventList.end(); it++) {
		delete *it;
	}
	
	// once the actual objects are deleted, clear the vector eventList
	eventList.clear();   
}

/** \brief event by event analysis
 * 
 * ScanList() operates on the time sorted list of all channels that triggered in
 * a given spill.  Starting from the begining of the list and continuing to the
 * end, an individual channel event time is compared with the previous channel
 * event time to determine if they occur within a time period defined by the
 * diff_t variable (time is in units of 10 ns).  Depending on the answer,
 * different actions are performed:
 *   - yes - the two channels are grouped together as belonging to the same event
 *   and the current channel is added to the list of channels for the rawevent 
 *   - no - the previous rawevent is sent for processing and once finished, the
 *   rawevent is zeroed and the current channel placed inside it.
 */

void ScanList(vector<ChanEvent*> &eventList, RawEvent& rawev) {
	unsigned long chanTime, eventTime;

	DetectorLibrary* modChan = DetectorLibrary::get();
	DetectorDriver* driver = DetectorDriver::get();

	// local variable for the detectors used in a given event
	set<string> usedDetectors;
	
	vector<ChanEvent*>::iterator iEvent = eventList.begin();

	// local variables for the times of the current event, previous
	// event and time difference between the two
	double diffTime = 0;
	
	//set last_t to the time of the first event
	double lastTime = (*iEvent)->GetTime();
	double currTime = lastTime;
	unsigned int id = (*iEvent)->GetID();

	HistoStats(id, diffTime, lastTime, BUFFER_START);

	//loop over the list of channels that fired in this buffer
	for(; iEvent != eventList.end(); iEvent++) { 
		id = (*iEvent)->GetID();
		if (id == U_DELIMITER) {
			std::cout << "pattern 0 ignore" << std::endl;
			continue;
		}
		if ((*modChan).at(id).GetType() == "ignore") {
			continue;
		}

		// this is a channel we're interested in
		chanTime  = (*iEvent)->GetTrigTime(); 
		eventTime = (*iEvent)->GetEventTimeLo();

		// retrieve the current event time and determine the time difference 
		// between the current and previous events. 
		currTime = (*iEvent)->GetTime();
		diffTime = currTime - lastTime;

		// if the time difference between the current and previous event is 
		//larger than the event width, finalize the current event, otherwise
		//treat this as part of the current event
		if ( diffTime > pixie::eventWidth ) {
			if(rawev.Size() > 0) {
			// detector driver accesses rawevent externally in order to
			//have access to proper detector_summaries
				driver->ProcessEvent(scanMode, rawev);
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

			HistoStats(id, diffTime, currTime, EVENT_START);
		} else HistoStats(id, diffTime, currTime, EVENT_CONTINUE);

		unsigned long dtimebin = 2000 + eventTime - chanTime;
		if (dtimebin < 0 || dtimebin > (unsigned)(SE)) {
			std::cout << "strange dtime for id " << id << ":" << dtimebin << std::endl;
		}
#ifdef USE_HHIRF
		driver->plot(D_TIME + id, dtimebin);
#endif
		usedDetectors.insert((*modChan).at(id).GetType());
		rawev.AddChan(*iEvent);

		// update the time of the last event
		lastTime = currTime; 
	} //end loop over event list

	//process the last event in the buffer
	if (rawev.Size() > 0) {
		string mode;
		HistoStats(id, diffTime, currTime, BUFFER_END);

		driver->ProcessEvent(scanMode, rawev);
		rawev.Zero(usedDetectors);
	}
}

/**
 * At various points in the processing of data in ScanList(), HistoStats() is
 * called to increment some low level pixie16 informational and diagnostic
 * spectra.  The list of spectra filled includes runtime in second and
 * milliseconds, the deadtime, time between events, and time width of an event.
 */
void HistoStats(unsigned int id, double diff, double clock, HistoPoints event){
#ifdef USE_HHIRF
	static const int specNoBins = SE;

	static double start, stop;
	static int count;
	static double firstTime = 0.;
	static double bufStart;

	double runTimeSecs   = (clock - firstTime) * pixie::clockInSeconds;
	int	rowNumSecs	= int(runTimeSecs / specNoBins);
	double remainNumSecs = runTimeSecs - rowNumSecs * specNoBins;

	double runTimeMsecs   = runTimeSecs * 1000;
	int	rowNumMsecs	= int(runTimeMsecs / specNoBins);
	double remainNumMsecs = runTimeMsecs - rowNumMsecs * specNoBins;

	static double bufEnd = 0, bufLength = 0;
	// static double deadTime = 0 // not used
	DetectorDriver* driver = DetectorDriver::get();

	if (firstTime > clock) {
		std::cout << "Backwards clock jump detected: prior start " << firstTime
			<< ", now " << clock << std::endl;
		// detect a backwards clock jump which occurs when some of the
		//   last buffers of a previous run sneak into the beginning of the 
		//   next run, elapsed time of last buffers is usually small but 
		//   just in case make some room for it
		double elapsed = stop - firstTime;
		// make an artificial 10 second gap by 
		//   resetting the first time accordingly
		firstTime = clock - 10 / pixie::clockInSeconds - elapsed;
		std::cout << elapsed*pixie::clockInSeconds << " prior seconds elapsed "
			<< ", resetting first time to " << firstTime << std::endl;	
	}

	switch (event) {
		case BUFFER_START:
			bufStart = clock;
			if(firstTime == 0.) {
				firstTime = clock;
			} else if (bufLength != 0.){
				//plot time between buffers as a function of time - dead time spectrum		
				// deadTime += (clock - bufEnd)*pixie::clockInSeconds;
				// plot(DD_DEAD_TIME_CUMUL,remainNumSecs,rownum,int(deadTime/runTimeSecs));				
				driver->plot(dammIds::raw::DD_BUFFER_START_TIME, remainNumSecs,rowNumSecs, (clock-bufEnd)/bufLength*1000.);		
			}
			break;
		case BUFFER_END:
			driver->plot(D_BUFFER_END_TIME, (stop - bufStart) * pixie::clockInSeconds * 1000);
			bufEnd = clock;
			bufLength = clock - bufStart;
		case EVENT_START:
			driver->plot(D_EVENT_LENGTH, stop - start);
			driver->plot(D_EVENT_GAP, diff);
			driver->plot(D_EVENT_MULTIPLICITY, count);
			
			start = stop = clock; // reset the counters	  
			count = 1;
			break;
		case EVENT_CONTINUE:
			count++;
			if(diff > 0.) {
				driver->plot(D_SUBEVENT_GAP, diff + 100);
			}
			stop = clock;
			break;
		default:
			std::cout << "Unexpected type " << event << " given to HistoStats" << std::endl;
	}

	//fill these spectra on all events, id plots and runtime.
	// Exclude event type 0/1 since it will also appear as an
	// event type 11
	if ( event != BUFFER_START && event != BUFFER_END ){	  
		driver->plot(DD_RUNTIME_SEC, remainNumSecs, rowNumSecs);
		driver->plot(DD_RUNTIME_MSEC, remainNumMsecs, rowNumMsecs);
		//fill scalar spectrum (per second) 
		driver->plot(D_HIT_SPECTRUM, id);
		driver->plot(D_SCALAR + id, runTimeSecs);
	}
#endif	
}

/** \brief pixie16 scan error handling.
 *
 * Print out an error message and terminate program depending on value
 * of errorNum. 
 */
bool Pixie16Error(int errorNum){
	//print error message depending on errornum
	switch (errorNum) {
		case 1:
		std::cout << std::endl;
		std::cout << " **************  SCAN ERROR  ****************** " << std::endl;
		std::cout << "There appears to be more modules in the data " << std::endl;
		std::cout << "stream than are present in the map.txt file. " << std::endl;
		std::cout << "Please verify that the map.txt file is correct " << std::endl;
		std::cout << "This is a fatal error, program terminating" << std::endl;
		return false;
		case 2:
		std::cout << std::endl;
		std::cout << "***************  SCAN ERROR  ******************* "<<std::endl;
		std::cout << "One of the variables named nn, nk, or mm" << std::endl;
		std::cout << "have exceeded the value of TOTALREAD. The value of" << std::endl;
		std::cout << "TOTALREAD MUST NEVER exceed 1000000 for correct " << std::endl;
		std::cout << "opertation of code between 32-bit and 64-bit architecture " << std::endl;
		std::cout << "Either these variables have not been zeroed correctly or" << std::endl;
		std::cout << "the poll program controlling pixie16 is trying to send too " << std::endl;
		std::cout << "much data at once" << std::endl;
		std::cout << "This is a fatal error, program terminating " << std::endl;
		return false;
	}
	return true;
}
