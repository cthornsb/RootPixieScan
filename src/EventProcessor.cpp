/*! \file EventProcessor.cpp
 * \brief Implementation of a generic event processor
 * \author David Miller
 * \date August 2009
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <unistd.h>
#include <sys/times.h>

#include "DetectorLibrary.hpp"
#include "EventProcessor.hpp"
#include "RawEvent.hpp"

using namespace std;

// Default initialization
void EventProcessor::_initialize(){
    userTime = 0.0; systemTime = 0.0;
    name = "Generic";
    didProcess = false; initDone = true;
    use_root = false; use_damm = false; 
    local_branch = NULL; count = 0;
    clocksPerSecond = sysconf(_SC_CLK_TCK);
}

EventProcessor::EventProcessor() : histo(0, 0)
{
    _initialize();
}

EventProcessor::EventProcessor(int offset, int range) : histo(offset, range) 
{
    _initialize();
}

EventProcessor::EventProcessor(std::string name_) : histo(0, 0) 
{
    _initialize();
    name = name_;
}

EventProcessor::EventProcessor(int offset, int range, std::string name_) : histo(offset, range) 
{
    _initialize();
    name = name_;
}

EventProcessor::~EventProcessor() 
{
}

void EventProcessor::Status(unsigned int total_events)
{
    if (initDone) {
	// output the time usage and the number of valid events
	cout << " " << name << "Processor: User Time = " << userTime << ", System Time = " << systemTime << endl;
	if(total_events > 0){ cout << " " << name << "Processor: " << count << " Valid Events (" << 1.0*count/total_events << "%)\n"; }
    }
}

/** Initialize Damm */
bool EventProcessor::InitDamm(){
    return false;
}

bool EventProcessor::InitRoot(TTree* top_tree){
    return false; 
}

/** See if the detectors of interest have any events */
bool EventProcessor::HasEvent(void) const
{
    for (map<string, const DetectorSummary*>::const_iterator it = sumMap.begin(); it != sumMap.end(); it++) {
	if (it->second->GetMult() > 0) {
	    return true;
	}
    }
    return false;
}

/** Initialize the processor if the detectors that require it are used in 
 * the analysis
 */
bool EventProcessor::Init(RawEvent& rawev) 
{    
    vector<string> intersect;   
    const set<string> &usedDets = DetectorLibrary::get()->GetUsedDetectors();
    set_intersection(associatedTypes.begin(), associatedTypes.end(), usedDets.begin(), usedDets.end(), back_inserter(intersect) );
    
    if (intersect.empty()) {
        return false;
    }

    // make the corresponding detector summary
    for (vector<string>::const_iterator it = intersect.begin(); it != intersect.end(); it++) {
        sumMap.insert( make_pair(*it, rawev.GetSummary(*it)) );
    }

    initDone = true;
    cout << " " << name << "Processor: Initialized, operating on " << intersect.size() << " detector type(s)." << endl;
	    
    return true;
}

bool EventProcessor::CheckInit(){
    if(!initDone){
        std::cout << " " << name << "Processor: Warning! Processor has not been initialized\n";
        return false;
    }
    if(!use_root && !use_damm){
    	std::cout << " " << name << "Processor: Warning! Both output types are marked inactive\n";
    	return false;
    }
    return true;
}

/** Process an event. In PreProcess correlator should filled (for
 * all derived classes) and basic analysis
 * is done. More sophisiticated analysis (which might also depend on other
 * processors) should be done in Process() function.
 * PreProcess will be first called for all Processors and only afterwards 
 * the Process function will be called.*/
bool EventProcessor::PreProcess(RawEvent &event)
{
    if (!initDone)
        return (didProcess = false);
    return (didProcess = true);
}

/** Process an event. PreProcess function should fill correlation tree and all processors
 * should have basic parameters calculated during PreProccessing.*/
bool EventProcessor::Process(RawEvent &event)
{
    if (!initDone)
        return (didProcess = false);

    // start the process timer
    times(&tmsBegin);

    EndProcess();
    return (didProcess = true);
}

/** Wrap up the processing and update the time spent by this processor */
void EventProcessor::EndProcess(void)
{
    tms tmsEnd;

    times(&tmsEnd);

    userTime += (tmsEnd.tms_utime - tmsBegin.tms_utime) / clocksPerSecond;
    systemTime += (tmsEnd.tms_stime - tmsBegin.tms_stime) / clocksPerSecond;

    // reset the beginning time so multiple calls of EndProcess from
    //   derived classes work properly
    times(&tmsBegin);
}

void EventProcessor::Zero(){
}
