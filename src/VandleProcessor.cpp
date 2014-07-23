/** \file VandleProcessor.cpp
 *\brief Processes information for VANDLE
 *
 *Processes information from the VANDLE Bars, allows for 
 *beta-gamma-neutron correlations. The prototype for this 
 *code was written by M. Madurga.
 *
 *\author S. V. Paulauskas 
 *\date 26 July 2010  
 */
#include <fstream>
#include <iostream>

#include <cmath>
#include <TFile.h>
#include <TTree.h>

#include "DetectorDriver.hpp"
#include "DammPlotIds.hpp"
#include "GetArguments.hpp"
#include "RawEvent.hpp"
#include "VandleProcessor.hpp"

namespace dammIds {
    const unsigned int BIG_OFFSET      = 30;
    const unsigned int MISC_OFFSET     = 60;
    const unsigned int TVANDLE_OFFSET  = 70;
    const unsigned int DEBUGGING_OFFSET = 100;

    namespace vandle {
	//Plots for the general information about VANDLE
	const int DD_TQDCBARS         = 0;
	const int DD_MAXIMUMBARS      = 1;
	const int DD_TIMEDIFFBARS     = 2;
	const int DD_TOFBARS          = 3;
	const int DD_CORTOFBARS       = 5;
	const int D_TOF               = 6;
	const int DD_TQDCAVEVSTDIFF   = 7;
	
	//Plots related to the TOF
	const int DD_TOFVSTDIFF       = 8;
	const int DD_MAXRVSTOF        = 9;
	const int DD_MAXLVSTOF        = 10;
	const int DD_TQDCAVEVSTOF     = 11;
	
	//Plots related to the CorTOF
	const int DD_CORTOFVSTDIFF     = 12;
	const int DD_MAXRVSCORTOF      = 13;
	const int DD_MAXLVSCORTOF      = 14;
	const int DD_TQDCAVEVSCORTOF   = 15;
	const int DD_TQDCAVEVSENERGY   = 16;
	
	//Plots related to correlated times
	const int DD_CORRELATED_TOF      = 17;
	
	//Plots related to the Start detectors
	const int DD_MAXSTART0VSTOF   = 18;
	const int DD_MAXSTART1VSTOF   = 19;
	const int DD_MAXSTART0VSCORTOF = 20;
	const int DD_MAXSTART1VSCORTOF = 21;
	const int DD_TQDCAVEVSSTARTQDCSUM= 22;
	const int DD_TOFVSSTARTQDCSUM    = 23;
	
	//Plots related to the Ge detectors
	const int DD_GAMMAENERGYVSTOF = 24;
	const int DD_TQDCAVEVSTOF_VETO= 25;
	const int DD_TOFBARS_VETO  = 26;
		
	//Plots used for debugging
	const int D_PROBLEMS     = 0+MISC_OFFSET;
	const int DD_PROBLEMS    = 1+MISC_OFFSET;

	//CrossTalk Subroutine
	const int DD_TOFBARBVSBARA      = 2+MISC_OFFSET;
	const int DD_GATEDTQDCAVEVSTOF  = 3+MISC_OFFSET;
	const int D_CROSSTALK           = 4+MISC_OFFSET;

	const int DD_DEBUGGING0  = 0+DEBUGGING_OFFSET;
	const int DD_DEBUGGING1  = 1+DEBUGGING_OFFSET;
	const int DD_DEBUGGING2  = 2+DEBUGGING_OFFSET;
	const int DD_DEBUGGING3  = 3+DEBUGGING_OFFSET;

	const int DD_DEBUGGING4  = 4+DEBUGGING_OFFSET;
	const int DD_DEBUGGING5  = 5+DEBUGGING_OFFSET;
	const int DD_DEBUGGING6  = 6+DEBUGGING_OFFSET;
	const int DD_DEBUGGING7  = 7+DEBUGGING_OFFSET;
	const int DD_DEBUGGING8  = 8+DEBUGGING_OFFSET;
    }//namespace vandle

    namespace tvandle {
	const int D_TIMEDIFF         = 2  + TVANDLE_OFFSET;
	const int DD_PVSP            = 3  + TVANDLE_OFFSET; 
	const int DD_MAXRIGHTVSTDIFF = 4  + TVANDLE_OFFSET; 
	const int DD_MAXLEFTVSTDIFF  = 5  + TVANDLE_OFFSET; 
	const int DD_MAXLVSTDIFFGATE = 6  + TVANDLE_OFFSET; 
	const int DD_MAXLVSTDIFFAMP  = 7  + TVANDLE_OFFSET; 
	const int DD_MAXLCORGATE     = 8  + TVANDLE_OFFSET; 
	const int DD_QDCVSMAX        = 9  + TVANDLE_OFFSET; 
	const int DD_SNRANDSDEV      = 10 + TVANDLE_OFFSET; 
    }// namespace tvandle
}//namespace dammIds

using namespace std;
using namespace dammIds::vandle;

//*********** VandleProcessor **********
VandleProcessor::VandleProcessor(): EventProcessor(OFFSET, RANGE, "Vandle") //--- These values directly affect the plotted IDS
{
    associatedTypes.insert("scint"); 
    associatedTypes.insert("vandleSmall"); 
    associatedTypes.insert("vandleBig");
    associatedTypes.insert("tvandle");
}

VandleProcessor::VandleProcessor(const int VML_OFFSET, const int RANGE):
    EventProcessor(VML_OFFSET, RANGE)//--- These values directly affect the plotted IDS
{
}

VandleProcessor::VandleProcessor(const int RP_OFFSET, const int RANGE, int i): 
    EventProcessor(RP_OFFSET, RANGE)//--- These values directly affect the plotted IDS
{
}

//********** Damm stuff **********
bool VandleProcessor::InitDamm()
{
    std::cout << " VandleProcessor: Initializing the damm output\n";
    if(use_damm){
        std::cout << " VandleProcessor: Warning! Damm output already initialized\n";
        return false;
    }
    
    bool hasSmall   = true;
    bool hasBig     = false;
    //const unsigned int numSmallEnds = S7;
    const unsigned int numBigEnds   = S7;

    //Plots used for debugging
    DeclareHistogram1D(D_PROBLEMS, S5, "1D Debugging");
    DeclareHistogram2D(DD_PROBLEMS, S7, S7, "2D Debugging");
    
    if(hasSmall) {
       //Plots used for the general information about VANDLE
       // DeclareHistogram2D(DD_TQDCBARS, SD, numSmallEnds, "Det Loc vs Trace QDC");
       // DeclareHistogram2D(DD_MAXIMUMBARS, SC, S5, "Det Loc vs. Maximum");
       // DeclareHistogram2D(DD_TIMEDIFFBARS, S9, numSmallEnds, "Bars vs. Time Differences");
       // DeclareHistogram2D(DD_TOFBARS, SC, numSmallEnds, "Bar vs. Time of Flight");
       // DeclareHistogram2D(DD_CORTOFBARS, SC, numSmallEnds, "Bar vs  Cor Time of Flight");
       // DeclareHistogram2D(DD_TQDCAVEVSTDIFF, SC, SD, "<E> vs. Time Diff(0.5ns/bin)");

       //Plots related to the TOF
       // DeclareHistogram2D(DD_TOFVSTDIFF, S9, SC, "TOF vs. TDiff(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSTOF, SD, SC, "MaxR vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSTOF, SD, SC, "MaxL vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSTOF, SC, SD, "<E> vs. TOF(0.5ns/bin)");

       //Plots related to the corTOF
       // DeclareHistogram2D(DD_CORTOFVSTDIFF, S9, SC, "corTOF vs. Tdiff(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSCORTOF, SD, SC, "MaxR vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSCORTOF, SD, SC, "MaxL vs. CorTOF(0.5ns/bin)");    
       // DeclareHistogram2D(DD_TQDCAVEVSCORTOF, SC, SD, "<E> vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSENERGY, SC, SD, "TQDC vs Energy (kev/bin)");

       //Plots related to Correlated times
       // DeclareHistogram2D(DD_CORRELATED_TOF, SC, SC, "Correlated TOF");

       //Plots related to the Starts
       // DeclareHistogram2D(DD_MAXSTART0VSTOF, SD, SC, "Max Start0 vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART1VSTOF, SD, SC, "Max Start1 vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART0VSCORTOF, SD, SC, "Max Start0 vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART1VSCORTOF, SD, SC, "Max Start1 vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSSTARTQDCSUM, SC, SD, "<E> VANDLE vs. <E> BETA - SUMMED");
       // DeclareHistogram2D(DD_TOFVSSTARTQDCSUM, SC, SD, "TOF VANDLE vs. <E> BETA - SUMMED");
       
       //Plots related to the Ge detectors
       // DeclareHistogram2D(DD_GAMMAENERGYVSTOF, SC, S9, "GAMMA ENERGY vs. CorTOF VANDLE");
       // DeclareHistogram2D(DD_TQDCAVEVSTOF_VETO, SC, SD, "<E> VANDLE vs. CorTOF VANDLE - Gamma Veto");
       // DeclareHistogram2D(DD_TOFBARS_VETO, SC, S9, "Bar vs CorTOF - Gamma Veto"); 
    }//if (hasSmall)

    if(hasBig) {
       //Plots used for the general information about VANDLE
       DeclareHistogram2D(DD_TQDCBARS+dammIds::BIG_OFFSET, SD, numBigEnds, "Det Loc vs Trace QDC");
       DeclareHistogram2D(DD_MAXIMUMBARS+dammIds::BIG_OFFSET, SC, numBigEnds, "Det Loc vs. Maximum");
       DeclareHistogram2D(DD_TIMEDIFFBARS+dammIds::BIG_OFFSET, SA, numBigEnds, "Bars vs. Time Differences");
       DeclareHistogram2D(DD_TOFBARS+dammIds::BIG_OFFSET, SC, numBigEnds, "Bar vs. Time of Flight");
       DeclareHistogram2D(DD_CORTOFBARS+dammIds::BIG_OFFSET, SC, numBigEnds, "Bar vs  Cor Time of Flight");
       // DeclareHistogram2D(DD_TQDCAVEVSTDIFF+dammIds::BIG_OFFSET, SC, SD, "<E> vs. Time Diff(0.5ns/bin)");

       //Plots related to the TOF
       // DeclareHistogram2D(DD_TOFVSTDIFF+dammIds::BIG_OFFSET, S9, SC, "TDiff vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSTOF+dammIds::BIG_OFFSET, SD, SC, "MaxR vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSTOF+dammIds::BIG_OFFSET, SD, SC, "MaxL vs. TOF(0.5ns/bin)");
       DeclareHistogram2D(DD_TQDCAVEVSTOF+dammIds::BIG_OFFSET, SC, SD, "<E> vs. TOF(0.5ns/bin)");

       //Plots related to the corTOF
       // DeclareHistogram2D(DD_CORTOFVSTDIFF+dammIds::BIG_OFFSET, S9, SC, "TDiff vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXRVSCORTOF+dammIds::BIG_OFFSET, SD, SC, "MaxR vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXLVSCORTOF+dammIds::BIG_OFFSET, SD, SC, "MaxL vs. CorTOF(0.5ns/bin)");    
       DeclareHistogram2D(DD_TQDCAVEVSCORTOF+dammIds::BIG_OFFSET, SC, SD, "<E> vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSENERGY+dammIds::BIG_OFFSET, SC, SD, "TQDC vs Energy (kev/bin)");

       //Plots related to Correlated times
       // DeclareHistogram2D(DD_CORRELATED_TOF+dammIds::BIG_OFFSET, SC, SC, "Correlated TOF");

       //Plots related to the Starts
       // DeclareHistogram2D(DD_MAXSTART0VSTOF+dammIds::BIG_OFFSET, SD, SC, "Max Start0 vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART1VSTOF+dammIds::BIG_OFFSET, SD, SC, "Max Start1 vs. TOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART0VSCORTOF+dammIds::BIG_OFFSET, SD, SC, "Max Start0 vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_MAXSTART1VSCORTOF+dammIds::BIG_OFFSET, SD, SC, "Max Start1 vs. CorTOF(0.5ns/bin)");
       // DeclareHistogram2D(DD_TQDCAVEVSSTARTQDCSUM+dammIds::BIG_OFFSET, SC, SD, "<E> VANDLE vs. <E> BETA - SUMMED");
       // DeclareHistogram2D(DD_TOFVSSTARTQDCSUM+dammIds::BIG_OFFSET, SC, SD, "TOF VANDLE vs. <E> BETA - SUMMED");

       //Plots related to the Ge detectors
       // DeclareHistogram2D(DD_GAMMAENERGYVSTOF+dammIds::BIG_OFFSET, SC, S9, "GAMMA ENERGY vs. CorTOF VANDLE");
       // DeclareHistogram2D(DD_TQDCAVEVSTOF_VETO+dammIds::BIG_OFFSET, SC, SD, "<E> VANDLE vs. CorTOF VANDLE - Gamma Veto");
       // DeclareHistogram2D(DD_TOFBARS_VETO+dammIds::BIG_OFFSET, SC, S9, "Bar vs CorTOF - Gamma Veto"); 
    }//if (hasBig)

    //Histograms for the CrossTalk Subroutine
    // DeclareHistogram1D(D_CROSSTALK, SC, "CrossTalk Between Two Bars");
    // DeclareHistogram2D(DD_GATEDTQDCAVEVSTOF, SC, SD, "<E> vs. TOF0 (0.5ns/bin) - Gated");
    // DeclareHistogram2D(DD_TOFBARBVSBARA, SC, SC, "TOF Bar1 vs. Bar2");
    // DeclareHistogram2D(, S8, S8, "tdiffA vs. tdiffB");
    // DeclareHistogram1D(, SD, "Muons");
    // DeclareHistogram2D(, S8, S8, "tdiffA vs. tdiffB");
    
    use_damm = true;
    return true;
}// Declare Plots

// Initialize for root output
bool VandleProcessor::InitRoot(TTree* top_tree){
    std::cout << " VandleProcessor: Initializing root output\n";
    if(use_root){
        std::cout << " VandleProcessor: Warning! Root output already initialized\n";
        return false;
    }
	
    // Create the branch
    local_branch = top_tree->Branch("Vandle", &structure, "tof/D:lqdc/D:rqdc/D:tsLow/D:tsHigh/D:lMaxVal/D:rMaxVal/D:qdc/D:energy/D:multiplicity/i:location/i:valid/O");

    use_root = true;
    return true;
}

//********** Process **********
bool VandleProcessor::Process(RawEvent &event) 
{
    if (!EventProcessor::Process(event)) //start event processing
	return false;
    if(use_damm){ plot(D_PROBLEMS, 30); } //DEBUGGING
    if(RetrieveData(event)){
        AnalyzeData(event);
        //CrossTalk();

        EndProcess();
        return true;
    } 
    else{
        EndProcess();
        return (didProcess = false);
    }
}

//********** RetrieveData **********
bool VandleProcessor::RetrieveData(RawEvent &event) 
{    
    ClearMaps();

    static const vector<ChanEvent*> &smallEvents = event.GetSummary("vandleSmall")->GetList(); //--- GetList returns the eventList
    static const vector<ChanEvent*> &bigEvents = event.GetSummary("vandleBig")->GetList(); //--- the eventList is a list of events associated with this detector group
    static const vector<ChanEvent*> &betaStarts = event.GetSummary("scint:beta:start")->GetList(); //--- vectors are filled with events of the specified type
    static const vector<ChanEvent*> &liquidStarts = event.GetSummary("scint:liquid:start")->GetList();
    static const vector<ChanEvent*> &tvandleEvents = event.GetSummary("tvandle")->GetList();

    //Construct and fill the vector for the startEvents
    vector<ChanEvent*> startEvents;
    startEvents.insert(startEvents.end(), betaStarts.begin(), betaStarts.end());
    startEvents.insert(startEvents.end(), liquidStarts.begin(), liquidStarts.end());

    if(smallEvents.empty() && bigEvents.empty() && tvandleEvents.empty()) {
        if(use_damm){ plot(D_PROBLEMS, 27); } //DEBUGGING
	return(false);
    }
     
    FillMap(smallEvents, "small", smallMap);
    FillMap(bigEvents, "big", bigMap);
    FillMap(tvandleEvents, "tvandle", tvandleMap);
    FillMap(startEvents, "start", startMap);
    
    //Make the VandleBars - small/big will be in the same map
    //BuildBars(bigMap, "big", barMap); // Turned off! CRT
    BuildBars(smallMap, "small", barMap); // Turned on! CRT
    
    if(barMap.empty() && tvandleMap.empty()) {
	if(use_damm){ plot(D_PROBLEMS, 25); } //DEBUGGING
	return(false);
    }

    return(true);
} // bool VandleProcessor::RetrieveData

//********** AnalyzeData **********
void VandleProcessor::AnalyzeData(RawEvent& rawev)
{
    //Analyze the Teeny VANDLE data if there is any
    if(!tvandleMap.empty() && tvandleMap.size()%2 == 0) //--- there should be an even amount of events
	Tvandle();

    //Analyze the VANDLE bars if any are present.
    //--- returns BarMap iterator to begining key (and corresponding data), iterates to another key in barMap as long as its not the last one
    for (BarMap::iterator itBar = barMap.begin(); itBar !=  barMap.end(); itBar++) {
	if(!(*itBar).second.event) //--- second refers to the event boolean of BarData struct
	    continue;
	
	BarData bar = (*itBar).second; //--- bar is filled with values from the second part of barMap, the struct

	//Set some useful values.
	const int resMult = 2; //set resolution of histograms
	const int resOffset = 600; // offset of histograms
	unsigned int barLoc = (*itBar).first.first; //--- IdentKey, unsigned int
	unsigned int idOffset = -1;
	if((*itBar).first.second == "small")
	    idOffset = 0;
	else
	   idOffset = dammIds::BIG_OFFSET;
	//cout << "Vandle true " << barLoc << endl; //--- ERROR CHECK
	TimingCal calibration =
	    GetTimingCal((*itBar).first);
	
	double timeDiff = bar.timeDiff;

        if(use_damm){
	    plot(DD_DEBUGGING0, bar.qdcPos*resMult+resOffset, timeDiff*resMult+resOffset);
	    plot(DD_TIMEDIFFBARS+idOffset, timeDiff*resMult+resOffset, barLoc);
	    plot(DD_TQDCAVEVSTDIFF+idOffset,  timeDiff*resMult+resOffset, bar.qdc);
	}
	WalkBetaVandle(startMap, bar);

	//Loop over the starts in the event
	//--- each event has multiple starts, find the ONE start
	for(TimingDataMap::iterator itStart = startMap.begin(); itStart != startMap.end(); itStart++) {
	    if(!(*itStart).second.dataValid) //--- that ONE start may or may not be valid
		continue;

	    unsigned int startLoc = (*itStart).first.first;
	    unsigned int barPlusStartLoc = barLoc*2 + startLoc;
	    double tofOffset;
	    if(startLoc == 0)
		tofOffset = calibration.tofOffset0;
	    else 
		tofOffset = calibration.tofOffset1;
	    
	    //times are calculated in ns, energy in keV
	    double TOF = bar.walkCorTimeAve - (*itStart).second.walkCorTime + tofOffset; 
	    double corTOF = CorrectTOF(TOF, bar.flightPath, calibration.z0); 
	    double energy = CalcEnergy(corTOF, calibration.z0);

	    //--- have everything at this point, need to fill VMLMap here from barMap. add timestamp at this point 
       	    static const vector<ChanEvent*> & validEvents =  rawev.GetSummary("valid")->GetList();

   	    double timeLow, timeHigh;
	    for(vector<ChanEvent*>::const_iterator itValid = validEvents.begin();
		itValid != validEvents.end(); itValid++) { //--- is it output type?
		if ((*itValid)->GetChanID().GetTag("output")) {
			timeLow = (*itValid)->GetQdcValue(0);
			timeHigh = (*itValid)->GetQdcValue(1); 
		} 
            }
	    VMLMap::iterator itVML = vmlMap.insert(make_pair(barLoc, vmlData(bar, TOF, energy, timeLow, timeHigh))).first;

	    bar.timeOfFlight.insert(make_pair(startLoc, TOF));
	    bar.corTimeOfFlight.insert(make_pair(startLoc, corTOF));
	    bar.energy.insert(make_pair(startLoc, energy));
	    
	    if(use_damm){
	        if(corTOF >= 5) // cut out the gamma prompt
		    plot(DD_TQDCAVEVSENERGY+idOffset, (*itVML).second.energy, (*itVML).second.qdc);
	        plot(DD_TOFBARS+idOffset, TOF*resMult+resOffset, barPlusStartLoc);
                plot(DD_TOFVSTDIFF+idOffset, timeDiff*resMult+resOffset, TOF*resMult+resOffset);
	        plot(DD_MAXRVSTOF+idOffset, TOF*resMult+resOffset, bar.rMaxVal);
	        plot(DD_MAXLVSTOF+idOffset, TOF*resMult+resOffset, bar.lMaxVal);
	        plot(DD_TQDCAVEVSTOF+idOffset, TOF*resMult+resOffset, bar.qdc);

	        plot(DD_CORTOFBARS, corTOF*resMult+resOffset, barPlusStartLoc); 
	        plot(DD_CORTOFVSTDIFF+idOffset, timeDiff*resMult + resOffset, corTOF*resMult+resOffset);
	        plot(DD_MAXRVSCORTOF+idOffset, corTOF*resMult+resOffset, bar.rMaxVal);
	        plot(DD_MAXLVSCORTOF+idOffset, corTOF*resMult+resOffset, bar.lMaxVal);
	        plot(DD_TQDCAVEVSCORTOF+idOffset, corTOF*resMult+resOffset, bar.qdc);
	    
	        if(startLoc == 0) {
                    plot(DD_MAXSTART0VSTOF+idOffset, TOF*resMult+resOffset, (*itStart).second.maxval);
                    plot(DD_MAXSTART0VSCORTOF+idOffset, corTOF*resMult+resOffset, (*itStart).second.maxval);
                } else if (startLoc == 1) {
                    plot(DD_MAXSTART1VSCORTOF+idOffset, corTOF*resMult+resOffset, (*itStart).second.maxval);
                    plot(DD_MAXSTART1VSCORTOF+idOffset, corTOF*resMult+resOffset, (*itStart).second.maxval);
                }
            }

	    //Now we will do some Ge related stuff
	    static const DetectorSummary *geSummary = rawev.GetSummary("ge");
	    
	    if (geSummary && use_damm) {
	        if (geSummary->GetMult() > 0) {
		    const vector<ChanEvent *> &geList = geSummary->GetList();
		    for (vector<ChanEvent *>::const_iterator itGe = geList.begin(); itGe != geList.end(); itGe++) {
		        double calEnergy = (*itGe)->GetCalEnergy();
			plot(DD_GAMMAENERGYVSTOF+idOffset, TOF, calEnergy);
		    }   
		} else {
		    // vetoed stuff
		    plot(DD_TQDCAVEVSTOF_VETO+idOffset, TOF, bar.qdc);
		    plot(DD_TOFBARS_VETO+idOffset, TOF, barPlusStartLoc);
		}
	    } 
	} // for(TimingDataMap::iterator itStart
    } //(BarMap::iterator itBar

    if(use_root){
        unsigned int multiplicity = 1;
        for(VMLMap::const_iterator itTempA = vmlMap.begin(); itTempA != vmlMap.end(); itTempA++) {
            //---loops over vml map, creating root structure         
            PackRoot((*itTempA).first, &(*itTempA).second, multiplicity); //--- filled from barMap
            multiplicity++;
        } // over vmlMAP
    } // if use_root
} //void VandleProcessor::AnalyzeData


//********** BuildBars **********
void VandleProcessor::BuildBars(const TimingDataMap &endMap, const string &type, BarMap &barMap) 
{
    for(TimingDataMap::const_iterator itEndA = endMap.begin();
	itEndA != endMap.end();) {

	TimingDataMap::const_iterator itEndB = itEndA;
	itEndB++;
	
	if(use_damm){
	    if(itEndB == endMap.end()) {
	      plot(D_PROBLEMS, 0);  //--- is it the end?
	      break;
	    }
	    if((*itEndA).first.first != (*itEndB).first.first) {
	      itEndA = itEndB;
	      plot(D_PROBLEMS, 2); //--- are the 2 events at the same location?
	      continue;
	    }
	    if(!(*itEndA).second.dataValid || !(*itEndB).second.dataValid){
	      plot(D_PROBLEMS, 4);
	      itEndA = itEndB; //--- are they both valid events
	      continue;
	    }
	}
	
	IdentKey barKey((*itEndA).first.first, type); //--- makes first part of pair of barKey same as endMap
	TimingCal calibrations = GetTimingCal(barKey);
	
	if((*itEndA).second.dataValid && (*itEndB).second.dataValid) 
	    barMap.insert(make_pair(barKey, BarData((*itEndB).second, 
						    (*itEndA).second, 
						    calibrations, type)));
        else {
	    itEndA = itEndB;
	    continue;
	}
	itEndA = itEndB;
    } // for(itEndA
} //void VandleProcessor::BuildBars

//********** ClearMaps *********
void VandleProcessor::ClearMaps(void)
{
    barMap.clear();
    bigMap.clear();
    smallMap.clear();
    startMap.clear();
    tvandleMap.clear();
    vmlMap.clear();
}

//********** CrossTalk **********
void VandleProcessor::CrossTalk(void)
{
    //This whole routine is stupidly written, it needs cleaned up something fierce.
    if(barMap.size() < 2)
	return;

    for(BarMap::iterator itBarA = barMap.begin(); 
	itBarA != barMap.end(); itBarA++) {
	BarMap::iterator itTemp = itBarA;
	itTemp++;
	
	const double &timeAveA = (*itBarA).second.timeAve;
	const unsigned int &locA = (*itBarA).first.first;
	
	for(BarMap::iterator itBarB = itTemp; itBarB != barMap.end(); 
	    itBarB++) {
	    if((*itBarA).first.second != (*itBarB).first.second)
		continue;
	    if((*itBarA).first == (*itBarB).first)
		continue;
	    
	    const double &timeAveB = (*itBarB).second.timeAve;
	    const unsigned int &locB = (*itBarB).first.first;
	    
	    CrossTalkKey bars(locA, locB);
	    crossTalk.insert(make_pair(bars, timeAveB - timeAveA));
	}
    }

    //Information for the bar of interest.
    string barType = "small";
    IdentKey barA(0, barType);
    IdentKey barB(1, barType);

    CrossTalkKey barsOfInterest(barA.first, barB.first);
    
    CrossTalkMap::iterator itBars = 
	crossTalk.find(barsOfInterest);
    
    const int resMult = 2; //set resolution of histograms
    const int resOffset = 200; // set offset of histograms
    
    if(itBars != crossTalk.end() && use_damm)
	plot(D_CROSSTALK, (*itBars).second * resMult + resOffset);
    
    //Carbon Recoil Stuff
    BarMap::iterator itBarA = barMap.find(barA);
    BarMap::iterator itBarB = barMap.find(barB);
    
    if(itBarA == barMap.end() || itBarB == barMap.end())
	return;
    
//    TimeOfFlightMap::iterator itTofA = 
// 	(*itBarA).second.timeOfFlight.find(startLoc);
//     TimeOfFlightMap::iterator itTofB = 
// 	(*itBarB).second.timeOfFlight.find(startLoc);
    
//     if(itTofA == (*itBarA).second.timeOfFlight.end() ||
//        itTofB == (*itBarB).second.timeOfFlight.end())
// 	return;
    
//     double tofA = (*itTofA).second;
//     double tofB = (*itTofB).second;
    double tdiffA = (*itBarA).second.walkCorTimeDiff;
    double tdiffB = (*itBarB).second.walkCorTimeDiff;
    double qdcA = (*itBarA).second.qdc;
    double qdcB = (*itBarB).second.qdc;

    //bool onBar = (tdiffA + tdiffB <= 0.75 && tdiffA + tdiffB >= 0.25);
    bool muon = (qdcA > 7500 && qdcB > 7500);
    
    double muonTOF = (*itBarA).second.timeAve - (*itBarB).second.timeAve;

    if(use_damm){ 
        plot(3950, tdiffA*resMult+100, tdiffB*resMult+100);
    
        if(muon){
	    plot(3951, tdiffA*resMult+100, tdiffB*resMult+100);
	    plot(3952, muonTOF*resMult*10 + resOffset);
        }
    }
} //void VandleProcessor::CrossTalk


//********** FillMap **********
void VandleProcessor::FillMap(const vector<ChanEvent*> &eventList, const string type, TimingDataMap &eventMap) //--- maps filled by vectors with events stored in them
{
    unsigned int OFFSET = 0;
    if(type == "big")
	OFFSET = dammIds::BIG_OFFSET;
    else if(type == "tvandle")
	OFFSET = dammIds::TVANDLE_OFFSET;

    for(vector<ChanEvent*>::const_iterator it = eventList.begin(); it != eventList.end(); it++) { //--- there is an event at eventList.begin() but not at eventList.end()
	unsigned int location = (*it)->GetChanID().GetLocation(); //--- from the vector eventList
	string subType = (*it)->GetChanID().GetSubtype();
	IdentKey key(location, subType); //--- the key is the location and subtype of the eventList
	
	TimingDataMap::iterator itTemp = eventMap.insert(make_pair(key, TimingData(*it))).first; //--- inserts into map the key and value, which in turn are a pair, first refers to map::insert
	
	if(type == "start")
	    continue;
    	
    	if(use_damm){
	    if((*itTemp).second.dataValid && (*itTemp).first.second == "right") {
	        plot(DD_TQDCBARS + OFFSET, (*itTemp).second.tqdc, location*2); //--- 0 + 30/70 (seems to be 30)
	        plot(DD_MAXIMUMBARS + OFFSET, (*itTemp).second.maxval, location*2); //--- 1 + 30/70 (seems to be 30)
	    } 
	    else if((*itTemp).second.dataValid && (*itTemp).first.second == "left") {
	        plot(DD_TQDCBARS + OFFSET, (*itTemp).second.tqdc, location*2+1); //--- 0 + 30/70 (seems to be 30)
	        plot(DD_MAXIMUMBARS + OFFSET, (*itTemp).second.maxval, location*2+1); //--- 1 + 30/70 (seems to be 30)
	    }
	}
    }//for(vector<chanEvent
}

//********* Tvandle **********
void VandleProcessor::Tvandle(void) 
{
    //Needs cleaned heavily!!
    //Method is never called, I'm not going to clean it. CRT
    using namespace dammIds::tvandle;
    TimingData right = (*tvandleMap.find(make_pair(0,"right"))).second;
    TimingData left  = (*tvandleMap.find(make_pair(0,"left"))).second;

    char hisFileName[32];
    GetArgument(2, hisFileName, 32);
    string temp = hisFileName;
    temp = temp.substr(0, temp.find_first_of(" "));
    double offset = atof(temp.c_str());

    double timeDiff = left.highResTime - right.highResTime - offset;
    double corTimeDiff = left.walkCorTime - right.walkCorTime - offset;

    vector<int> trc = right.trace;
    vector<int> trc1 = left.trace;
    if(timeDiff < -5.418 && left.maxval < 90) {
	for(vector<int>::iterator it = trc.begin(); it != trc.end(); it++)
    	    plot(dammIds::vandle::DD_PROBLEMS, it-trc.begin(), counter, *it); //--- 61
    	for(vector<int>::iterator it = trc1.begin(); it != trc1.end(); it++)
    	    plot(dammIds::vandle::DD_PROBLEMS, it-trc1.begin(), counter+1, *it); //--- 61
    	counter+=2;
    }

    plot(DD_QDCVSMAX, right.maxval, right.tqdc); //--- 79, declared but cannot find
    
    if(right.dataValid && left.dataValid){
	double timeRes = 50; //20 ps/bin
	double timeOff = 500; 

	plot(D_TIMEDIFF, timeDiff*timeRes + timeOff); //--- 72, declared but cannot find
	plot(DD_PVSP, right.tqdc, left.tqdc); //--- 73, declared but not found (found ones at 72/73, but title mismatch)
	plot(DD_MAXRIGHTVSTDIFF, timeDiff*timeRes+timeOff, right.maxval); //--- 74
	plot(DD_MAXLEFTVSTDIFF, timeDiff*timeRes+timeOff, left.maxval); //--- 75, declared but not found (found ones at 74/75, but title mismatch)
	
	//Plot information about the SNR
	plot(DD_SNRANDSDEV, right.snr+50, 0); //--- 80, declared but not found (found one at 80, but title mismatch)
	plot(DD_SNRANDSDEV, right.stdDevBaseline*timeRes+timeOff, 1);
	plot(DD_SNRANDSDEV, left.snr+50, 2);
	plot(DD_SNRANDSDEV, left.stdDevBaseline*timeRes+timeOff, 3);
	
	//Plot information used to determine the impact of walk.
	double tempVal = fabs(right.maxval-left.maxval);
	if(tempVal <= 50)
	    plot(DD_MAXLVSTDIFFAMP, timeDiff*timeRes+timeOff, left.maxval); //--- 77, declared but not found (one found at 77 with title mismatch)
	plot(DD_MAXLCORGATE, corTimeDiff*timeRes+timeOff, left.maxval); //--- 78, declared but not found (one found at 78 with title mismatch)

	if(right.maxval > 3000) {
	    plot(DD_MAXLVSTDIFFGATE, timeDiff*timeRes+timeOff, left.maxval); //--- 76, same situation
	}
    } else // if(right.dataValid
	plot(D_PROBLEMS, 2);
}

void VandleProcessor::WalkBetaVandle(const TimingInformation::TimingDataMap &beta, const TimingInformation::BarData &bar) {
    double cutoff = 1500;
    for(TimingDataMap::const_iterator it = beta.begin(); it != beta.end(); it++) {
	plot(DD_DEBUGGING4, bar.lMaxVal, bar.rMaxVal); //--- 104, found at 3204, declared but not functioning
	if((*it).first.first == 0 && bar.rMaxVal > cutoff
	   && bar.lMaxVal > cutoff) {
		plot(DD_DEBUGGING5, (bar.walkCorTimeAve - (*it).second.highResTime)*2+500, (*it).second.maxval); //--- 105, found at 3205, declared but not functioning
		plot(DD_DEBUGGING6, (bar.walkCorTimeAve - (*it).second.walkCorTime)*2+500, (*it).second.maxval); //--- 106, found at 3206, declared but not functioning
	} else if((*it).first.first == 1 && bar.rMaxVal > cutoff
	   && bar.lMaxVal > cutoff) {
		plot(DD_DEBUGGING7, (bar.walkCorTimeAve - (*it).second.highResTime)*2+500, (*it).second.maxval);
		plot(DD_DEBUGGING8, (bar.walkCorTimeAve - (*it).second.walkCorTime)*2+500, (*it).second.maxval);
	}
    }
}

// Fill the root variables with processed data
void VandleProcessor::PackRoot(unsigned int location_, const vmlData* current_data, unsigned int multiplicity_){
	// Integers
	structure.location = location_;
	structure.multiplicity = multiplicity_;
	
	// Doubles
	structure.tof = current_data->tof; 
	structure.lqdc = current_data->lqdc;
	structure.rqdc = current_data->rqdc;
	structure.tsLow = current_data->tsLow;
	structure.tsHigh = current_data->tsHigh;
        structure.lMaxVal = current_data->lMaxVal;
        structure.rMaxVal = current_data->rMaxVal;
        structure.qdc = current_data->qdc;
        structure.energy = current_data->energy;
        
        // Bools
        structure.valid = true;
        count++;
}
