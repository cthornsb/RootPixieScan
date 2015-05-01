/** \file LiquidProcessor.cpp
 *
 * implementation for scintillator processor
 */
#include <vector>
#include <sstream>

#include <cmath>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "LiquidProcessor.hpp"
#include "TimingInformation.hpp"
#include "Trace.hpp"

using namespace std;
using namespace dammIds::scint::liquid;

namespace dammIds {
    namespace scint {
        namespace liquid {
            const int DD_TQDCLIQUID       = 0;
            const int DD_MAXLIQUID        = 1;
            const int DD_DISCRIM          = 2;
            const int DD_TOFLIQUID        = 3;
            const int DD_TRCLIQUID        = 4;
            const int DD_TQDCVSDISCRIM    = 5;
            const int DD_TOFVSDISCRIM     = 7;
            const int DD_NEVSDISCRIM      = 9;
            const int DD_TQDCVSLIQTOF     = 11;
            const int DD_TQDCVSENERGY     = 13;
        }
    }
}

//***** LiquidProcessor *******
LiquidProcessor::LiquidProcessor() : EventProcessor(OFFSET, RANGE, "Liquid")
{
    associatedTypes.insert("scint");
    save_waveforms = false;
}

LiquidProcessor::LiquidProcessor(bool save_waveforms_) : EventProcessor(OFFSET, RANGE, "Liquid")
{
    associatedTypes.insert("scint");
    save_waveforms = save_waveforms_;
}

//******* Declare Plots *******
bool LiquidProcessor::InitDamm(){
#ifdef USE_HHIRF
    std::cout << " LiquidProcessor: Initializing the damm output\n";
    if(use_damm){
        std::cout << " LiquidProcessor: Warning! Damm output already initialized\n";
        return false;
    }
    
    //To handle Liquid Scintillators
    DeclareHistogram2D(DD_TQDCLIQUID, SC, S3, "Liquid vs. Trace QDC");
    DeclareHistogram2D(DD_MAXLIQUID, SC, S3, "Liquid vs. Maximum");
    DeclareHistogram2D(DD_DISCRIM, SA, S3, "N-Gamma Discrimination");
    DeclareHistogram2D(DD_TOFLIQUID, SE, S3,"Liquid vs. TOF");
    DeclareHistogram2D(DD_TRCLIQUID, S7, S7, "LIQUID TRACES");
    
    for(unsigned int i=0; i < 2; i++) { 
    	DeclareHistogram2D(DD_TQDCVSDISCRIM+i, SA, SE,"Trace QDC vs. NG Discrim");
    	DeclareHistogram2D(DD_TOFVSDISCRIM+i, SA, SA, "TOF vs. Discrim");
    	DeclareHistogram2D(DD_NEVSDISCRIM+i, SA, SE, "Energy vs. Discrim");
    	DeclareHistogram2D(DD_TQDCVSLIQTOF+i, SC, SE, "Trace QDC vs. Liquid TOF");
    	DeclareHistogram2D(DD_TQDCVSENERGY+i, SD, SE, "Trace QDC vs. Energy");
    }
    
    use_damm = true;
    return true;
#else
	return false;
#endif
}

// Initialize for root output
bool LiquidProcessor::InitRoot(TTree* top_tree){
    if(!top_tree){
        use_root = false;
        return false;
    }
	
    // Create the branch
    local_branch = top_tree->Branch("Liquid", &structure);
    if(save_waveforms){
        std::cout << " LiquidProcessor: Dumping raw waveforms to root file\n";
    	local_branch = top_tree->Branch("LiquidWave", &waveform); 
    }

    use_root = true;
    return true;
}

//********** Process **********
// Returns true ONLY if there is data to fill to the root tree
bool LiquidProcessor::Process(RawEvent &event) {
    if(!initDone){ return (didProcess = false); }
    bool output = false;

    // start the process timer
    StartProcess();

    static const vector<ChanEvent*> &liquidEvents = event.GetSummary("scint:liquid")->GetList();
    static const vector<ChanEvent*> &triggerStartEvents = event.GetSummary("scint:trigger:start")->GetList();
    static const vector<ChanEvent*> &liquidStartEvents = event.GetSummary("scint:liquid:start")->GetList();
    
    vector<ChanEvent*> startEvents; startEvents.insert(startEvents.end(), triggerStartEvents.begin(), triggerStartEvents.end());
    startEvents.insert(startEvents.end(), liquidStartEvents.begin(), liquidStartEvents.end());
    
    static int counter = 0;
    for(vector<ChanEvent*>::const_iterator itLiquid = liquidEvents.begin(); itLiquid != liquidEvents.end(); itLiquid++) {
        unsigned int loc = (*itLiquid)->GetChanID().GetLocation();
        TimingInformation::TimingData liquid((*itLiquid));

        //Graph traces for the Liquid Scintillators
        if(liquid.discrimination == 0) {
#ifdef USE_HHIRF
            if(use_damm){
                for(Trace::const_iterator i = liquid.trace.begin(); i != liquid.trace.end(); i++){
                    plot(DD_TRCLIQUID, int(i-liquid.trace.begin()), counter, int(*i)-liquid.aveBaseline);
                }
            }
#endif
            counter++;
        }
           
        // Valid Liquid
		if(liquid.dataValid) {
			TimingInformation::TimingCal calibration = TimingInformation::GetTimingCal(make_pair(loc, "liquid"));

#ifdef USE_HHIRF	
	        double discrimNorm = liquid.discrimination/liquid.tqdc;	    
			double discRes = 1000;
			double discOffset = 100;
			if(use_damm){
				if(discrimNorm > 0){ plot(DD_DISCRIM, discrimNorm*discRes+discOffset, loc); }
				plot(DD_TQDCVSDISCRIM, discrimNorm*discRes+discOffset, liquid.tqdc);
				plot(DD_TQDCLIQUID, liquid.tqdc, loc);
				plot(DD_MAXLIQUID, liquid.maxval, loc);
			}
#endif
		        
			if((*itLiquid)->GetChanID().HasTag("start")){ continue; }
		            
			for(vector<ChanEvent*>::iterator itStart = startEvents.begin(); itStart != startEvents.end(); itStart++){            
				unsigned int startLoc = (*itStart)->GetChanID().GetLocation();
				TimingInformation::TimingData start((*itStart));

				if(start.dataValid) {
					double tofOffset;
					if(startLoc == 0){ tofOffset = calibration.tofOffset0; }
					else{ tofOffset = calibration.tofOffset1; }
				            
					double TOF = liquid.highResTime - start.highResTime - tofOffset; //in ns
				                                
					//Root stuff
					if(use_root){ 
						structure.Append(loc, TOF, liquid.tqdc, start.tqdc);
						if(save_waveforms){ waveform.Append(liquid.trace); }
						if(!output){ output = true; }
						count++;
					}

#ifdef USE_HHIRF				            
					//Damm stuff
					double nEnergy = timeInfo.CalcEnergy(TOF, calibration.r0);
					int histLoc = loc + startLoc;
					const int resMult = 2;
					const int resOffset = 2000;					
					if(use_damm){
						plot(DD_TOFLIQUID, TOF*resMult+resOffset, histLoc);
						plot(DD_TOFVSDISCRIM+histLoc, discrimNorm*discRes+discOffset, TOF*resMult+resOffset);
						plot(DD_NEVSDISCRIM+histLoc, discrimNorm*discRes+discOffset, nEnergy);
						plot(DD_TQDCVSLIQTOF+histLoc, TOF*resMult+resOffset, liquid.tqdc);
						plot(DD_TQDCVSENERGY+histLoc, nEnergy, liquid.tqdc);
					}
#endif
				}
		    } //Loop over starts
		} // Good Liquid Check
    }//end loop over liquid events
    
    EndProcess();
    return output;
}
