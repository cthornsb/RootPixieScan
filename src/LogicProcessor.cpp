/** \file LogicProcessor.cpp
 * \brief handling of logic events, derived from MtcProcessor.cpp
 *
 * Start subtype corresponds to leading edge
 * Stop subtype corresponds to trailing edge
 */

#include <iostream>
#include <string>
#include <vector>

#include "Globals.hpp"
#include "RawEvent.hpp"
#include "LogicProcessor.hpp"

using namespace std;
using namespace dammIds::logic;

namespace dammIds {
    namespace logic {
        const int D_COUNTER_START = 0;
        const int D_COUNTER_STOP  = 5;
        const int D_TDIFF_STARTX  = 10;
        const int D_TDIFF_STOPX   = 20;
        const int D_TDIFF_SUMX    = 30;
        const int D_TDIFF_LENGTHX = 50;
        const int DD_RUNTIME_LOGIC = 80;
    }
} // logic namespace

LogicProcessor::LogicProcessor(void) : EventProcessor(OFFSET, RANGE), lastStartTime(MAX_LOGIC, NAN), 
  lastStopTime(MAX_LOGIC, NAN), logicStatus(MAX_LOGIC), stopCount(MAX_LOGIC), startCount(MAX_LOGIC){
    name = "Logic";
    associatedTypes.insert("logic");
    plotSize = SA;
    for(int i = 0; i < MAX_LOGIC; i++){ logic_counts[i] = 0; }
}

LogicProcessor::LogicProcessor(bool save_waveforms_) : EventProcessor(OFFSET, RANGE), lastStartTime(MAX_LOGIC, NAN), 
  lastStopTime(MAX_LOGIC, NAN), logicStatus(MAX_LOGIC), stopCount(MAX_LOGIC), startCount(MAX_LOGIC){
    name = "Logic";
    associatedTypes.insert("logic"); 
    plotSize = SA;
    for(int i = 0; i < MAX_LOGIC; i++){ logic_counts[i] = 0; }
}

bool LogicProcessor::InitDamm(){
#ifdef USE_HHIRF
    std::cout << " LogicProcessor: Initializing the damm output\n";
    if(use_damm){
        std::cout << " LogicProcessor: Warning! Damm output already initialized\n";
        return false;
    }
    
    const int counterBins = S4;
    const int timeBins = SC;

    DeclareHistogram1D(D_COUNTER_START, counterBins, "logic start counter");
    DeclareHistogram1D(D_COUNTER_STOP, counterBins, "logic stop counter");
    for (int i=0; i < MAX_LOGIC; i++) {
        DeclareHistogram1D(D_TDIFF_STARTX + i, timeBins, "tdiff btwn logic starts, 10 us/bin");
        DeclareHistogram1D(D_TDIFF_STOPX + i, timeBins, "tdiff btwn logic stops, 10 us/bin");
        DeclareHistogram1D(D_TDIFF_SUMX + i, timeBins, "tdiff btwn both logic, 10 us/bin");
        DeclareHistogram1D(D_TDIFF_LENGTHX + i, timeBins, "logic high time, 10 us/bin");
    }

    DeclareHistogram2D(DD_RUNTIME_LOGIC, plotSize, plotSize, "runtime logic [1ms]");
    for (int i=1; i < MAX_LOGIC; i++) {
		DeclareHistogram2D(DD_RUNTIME_LOGIC+i, plotSize, plotSize, "runtime logic [1ms]");
    }
    
    use_damm = true;
    return true;
#else
	return true;
#endif
}

/**< Nothing to do to init Root output for logic, return true */
bool LogicProcessor::InitRoot(TTree *tree){
    use_root = true;
    return true;
}

/**< Returns true ONLY if there is data to fill to the root tree */
bool LogicProcessor::Process(RawEvent &event)
{
    if(!initDone){ return (didProcess = false); }

    // start the process timer
    StartProcess();

    //BasicProcessing(event); // Causes seg-faults (sometimes)
    bool output = TriggerProcessing(event);
    
    EndProcess(); // update processing time
    return output;
}

/**< Returns true ONLY if there is data to fill to the root tree */
bool LogicProcessor::BasicProcessing(RawEvent &event) {
    const double logicPlotResolution = 10e-6 / pixie::clockInSeconds;    
    static const vector<ChanEvent*> &events = sumMap["logic"]->GetList(); // This crashes the program sometimes
    
    for (vector<ChanEvent*>::const_iterator it = events.begin(); it != events.end(); it++) {
		ChanEvent *chan = *it;
		    
		string subtype   = chan->GetChanID().GetSubtype();
		unsigned int loc = chan->GetChanID().GetLocation();
		double time = chan->GetTime();
		double timediff;

		if(subtype == "start") {
#ifdef USE_HHIRF
			if (!isnan(lastStartTime.at(loc)) && use_damm) {
			    timediff = time - lastStartTime.at(loc);
				plot(D_TDIFF_STARTX + loc, timediff / logicPlotResolution);
				plot(D_TDIFF_SUMX + loc,   timediff / logicPlotResolution);
			}
#endif

			//? bounds checking
			lastStartTime.at(loc) = time;
			logicStatus.at(loc) = true;

			startCount.at(loc)++;
#ifdef USE_HHIRF
			if(use_damm){ plot(D_COUNTER_START, loc); }
#endif
		} else if (subtype == "stop") {
#ifdef USE_HHIRF
	  	    if (!isnan(lastStopTime.at(loc)) && use_damm) {
			timediff = time - lastStopTime.at(loc);
			plot(D_TDIFF_STOPX + loc, timediff / logicPlotResolution);
			plot(D_TDIFF_SUMX + loc,  timediff / logicPlotResolution);
				if (!isnan(lastStartTime.at(loc))) {
		  		    double moveTime = time - lastStartTime.at(loc);    
					plot(D_TDIFF_LENGTHX + loc, moveTime / logicPlotResolution);
				}
			}
#endif
			//? bounds checking
			lastStopTime.at(loc) = time;
			logicStatus.at(loc) = false;

			stopCount.at(loc)++;
#ifdef USE_HHIRF
			if(use_damm){ plot(D_COUNTER_STOP, loc); }
#endif
		}
    }
    
    return false; // Fix later, this method is un-used anyway at the moment
}

/**< Returns true ONLY if there is data to fill to the root tree */
bool LogicProcessor::TriggerProcessing(RawEvent &event) {
    bool output = false;
    const double logicPlotResolution = 1e-3 / pixie::clockInSeconds;
    const long maxBin = plotSize * plotSize;
    
    static DetectorSummary *stopsSummary = event.GetSummary("logic:stop");
    static DetectorSummary *triggersSummary = event.GetSummary("logic:trigger");
    static DetectorSummary *scalersSummary = event.GetSummary("logic:scaler");

    static const vector<ChanEvent*> &stops = stopsSummary->GetList();
    static const vector<ChanEvent*> &triggers = triggersSummary->GetList();
    static const vector<ChanEvent*> &scalers = scalersSummary->GetList();
    static int firstTimeBin = -1;
    
    for (vector<ChanEvent*>::const_iterator it = stops.begin(); it != stops.end(); it++) {
    	if((*it)->GetChanID().GetLocation() < MAX_LOGIC){ logic_counts[(*it)->GetChanID().GetLocation()]++; }
		ChanEvent *chan = *it;
		    
		unsigned int loc = chan->GetChanID().GetLocation();
		int timeBin = int(chan->GetTime() / logicPlotResolution);
		int startTimeBin = 0;
		    
		if (!isnan(lastStartTime.at(loc))) {
			startTimeBin = int(lastStartTime.at(loc) / logicPlotResolution);
			if (firstTimeBin == -1) {
				firstTimeBin = startTimeBin;
			}
		} else if (firstTimeBin == -1) {
			firstTimeBin = startTimeBin;
		}
		startTimeBin = max(0, startTimeBin - firstTimeBin);
		timeBin -= firstTimeBin;

#ifdef USE_HHIRF		    
		if(use_damm){ 
			for (int bin=startTimeBin; bin < timeBin; bin++) {
				int row = bin / plotSize;
				int col = bin % plotSize;
				plot(DD_RUNTIME_LOGIC, col, row, loc + 1); // add one since first logic location might be 0
				plot(DD_RUNTIME_LOGIC + loc, col, row, 1);
			}
		}
#endif
    }
    
    for (vector<ChanEvent*>::const_iterator it = triggers.begin(); it != triggers.end(); it++) {
    	if((*it)->GetChanID().GetLocation() < MAX_LOGIC){ logic_counts[(*it)->GetChanID().GetLocation()]++; }
        int timeBin = int((*it)->GetTime() / logicPlotResolution);
        timeBin -= firstTimeBin;
        if (timeBin >= maxBin || timeBin < 0)
            continue;

#ifdef USE_HHIRF        
        if(use_damm){
            int row = timeBin / plotSize;
            int col = timeBin % plotSize;
        
            plot(DD_RUNTIME_LOGIC, col, row, 20);
            for (int i=1; i < MAX_LOGIC; i++) {
                plot(DD_RUNTIME_LOGIC + i, col, row, 5);
            }
        }
#endif
    }

    for (vector<ChanEvent*>::const_iterator it = scalers.begin(); it != scalers.end(); it++) {
    	if((*it)->GetChanID().GetLocation() < MAX_LOGIC){ logic_counts[(*it)->GetChanID().GetLocation()]++; }
    }
    
    return output;
}

float LogicProcessor::Status(unsigned int total_events)
{
	float time_taken = 0.0;
	if (initDone) {
		// output the time usage and the number of valid events
		time_taken = ((float)total_time)/CLOCKS_PER_SEC;
		cout << " " << name << "Processor: Used " << time_taken << " seconds of CPU time\n";
		for(int i = 0; i < MAX_LOGIC; i++){
			if(logic_counts[i] > 0){ cout << " " << name << "Processor: Location " << i << " received " << logic_counts[i] << " counts\n"; }
		}
	}
	return time_taken;
}
