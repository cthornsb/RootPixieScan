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
#include "PulseAnalysis.h"

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
}

//******* Declare Plots *******
bool LiquidProcessor::InitDamm()
{
    std::cout << " LiquidProcessor: Initializing the damm output\n";
    if(use_damm){
        std::cout << " LiquidProcessor: Warning! Damm output already initialized\n";
        return false;
    }
    
    //To handle Liquid Scintillators
    //DeclareHistogram2D(DD_TQDCLIQUID, SC, S3, "Liquid vs. Trace QDC");
    //DeclareHistogram2D(DD_MAXLIQUID, SC, S3, "Liquid vs. Maximum");
    //DeclareHistogram2D(DD_DISCRIM, SA, S3, "N-Gamma Discrimination");
    DeclareHistogram2D(DD_TOFLIQUID, SE, S3,"Liquid vs. TOF");
    /*DeclareHistogram2D(DD_TRCLIQUID, S7, S7, "LIQUID TRACES");
    
    for(unsigned int i=0; i < 2; i++) { 
    	DeclareHistogram2D(DD_TQDCVSDISCRIM+i, SA, SE,"Trace QDC vs. NG Discrim");
    	DeclareHistogram2D(DD_TOFVSDISCRIM+i, SA, SA, "TOF vs. Discrim");
    	DeclareHistogram2D(DD_NEVSDISCRIM+i, SA, SE, "Energy vs. Discrim");
    	DeclareHistogram2D(DD_TQDCVSLIQTOF+i, SC, SE, "Trace QDC vs. Liquid TOF");
    	DeclareHistogram2D(DD_TQDCVSENERGY+i, SD, SE, "Trace QDC vs. Energy");
    }*/
    
    use_damm = true;
    return true;
}

// Initialize for root output
bool LiquidProcessor::InitRoot(){
    std::cout << " LiquidProcessor: Initializing the root output\n";
    if(use_root){
        std::cout << " LiquidProcessor: Warning! Root output already initialized\n";
        return false;
    }
	
    // Create the branch
    local_tree = new TTree(name.c_str(),name.c_str());
    local_branch = local_tree->Branch("Liquid", &structure, "TOF/D:S/D:L/D:liquid_tqdc/D:start_tqdc/D:location/i:valid/O");

    use_root = true;
    return true;
}

//******** Pre-Process ********
bool LiquidProcessor::PreProcess(RawEvent &event){
    if (!EventProcessor::PreProcess(event))
        return false;
    return true;
}

//********** Process **********
bool LiquidProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    static const vector<ChanEvent*> &liquidEvents = event.GetSummary("scint:liquid")->GetList();
    static const vector<ChanEvent*> &betaStartEvents = event.GetSummary("scint:beta:start")->GetList();
    static const vector<ChanEvent*> &liquidStartEvents = event.GetSummary("scint:liquid:start")->GetList();
    
    vector<ChanEvent*> startEvents; startEvents.insert(startEvents.end(), betaStartEvents.begin(), betaStartEvents.end());
    startEvents.insert(startEvents.end(), liquidStartEvents.begin(), liquidStartEvents.end());
    
    static int counter = 0;
    PulseAnalysis *Analysis = new PulseAnalysis(); // For Mike F.
    double L, S;    

    for(vector<ChanEvent*>::const_iterator itLiquid = liquidEvents.begin(); itLiquid != liquidEvents.end(); itLiquid++) {
        unsigned int loc = (*itLiquid)->GetChanID().GetLocation();
        TimingInformation::TimingData liquid((*itLiquid));

        //Graph traces for the Liquid Scintillators
        if(liquid.discrimination == 0) {
            if(use_damm){
                for(Trace::const_iterator i = liquid.trace.begin(); i != liquid.trace.end(); i++)
                    plot(DD_TRCLIQUID, int(i-liquid.trace.begin()), counter, int(*i)-liquid.aveBaseline);
            }
            counter++;
        }
           
        // Valid Liquid
	if(liquid.dataValid) {
            double discrimNorm = liquid.discrimination/liquid.tqdc;	    
            double discRes = 1000;
            double discOffset = 100;
            
            TimingInformation::TimingCal calibration = TimingInformation::GetTimingCal(make_pair(loc, "liquid"));
            
            if(use_damm){
                if(discrimNorm > 0)
                    plot(DD_DISCRIM, discrimNorm*discRes+discOffset, loc);
                plot(DD_TQDCVSDISCRIM, discrimNorm*discRes+discOffset, liquid.tqdc);
                plot(DD_TQDCLIQUID, liquid.tqdc, loc);
                plot(DD_MAXLIQUID, liquid.maxval, loc);
            }
            
            if((*itLiquid)->GetChanID().HasTag("start"))
                continue;
                
            for(vector<ChanEvent*>::iterator itStart = startEvents.begin(); itStart != startEvents.end(); itStart++){            
                unsigned int startLoc = (*itStart)->GetChanID().GetLocation();
                TimingInformation::TimingData start((*itStart));
                int histLoc = loc + startLoc;
                const int resMult = 2;
                const int resOffset = 2000;
                if(start.dataValid) {
                    // This calls Mike Febbraro's code from "PulseAnalysis.h"
                    // Probably slow, we should only call when we have a valid start. CRT
                    Analysis->Baseline_restore(liquid.trace, 1, 3);
                    if(Analysis->PSD_Integration(liquid.trace, 10, 50, 1, L, S)){ goodCount++; }
                    else{ badCount++; }
                
                    double tofOffset;
                    if(startLoc == 0)
                        tofOffset = calibration.tofOffset0;
                    else
                        tofOffset = calibration.tofOffset1;
                    
                    double TOF = liquid.highResTime - start.highResTime - tofOffset; //in ns
                    double nEnergy = timeInfo.CalcEnergy(TOF, calibration.r0);
                    
                    // Root stuff
                    if(use_root){ PackRoot(loc, TOF, S, L, liquid.tqdc, start.tqdc); }
                    
                    // Damm stuff
                    if(use_damm){
                        plot(DD_TOFLIQUID, TOF*resMult+resOffset, histLoc);
                        plot(DD_TOFVSDISCRIM+histLoc, discrimNorm*discRes+discOffset, TOF*resMult+resOffset);
                        plot(DD_NEVSDISCRIM+histLoc, discrimNorm*discRes+discOffset, nEnergy);
                        plot(DD_TQDCVSLIQTOF+histLoc, TOF*resMult+resOffset, liquid.tqdc);
                        plot(DD_TQDCVSENERGY+histLoc, nEnergy, liquid.tqdc);
                    }
                }
            } //Loop over starts
        } // Good Liquid Check
    }//end loop over liquid events
    EndProcess();
    return true;
}

// "Zero" the root structure
void LiquidProcessor::Zero(){
	structure.TOF = 0.0; structure.S = 0.0; structure.L = 0.0;
	structure.liquid_tqdc = 0.0; structure.start_tqdc = 0.0; 
	structure.location = 0; structure.valid = false;
}

// Fill the root variables with processed data
void LiquidProcessor::PackRoot(unsigned int location_, double TOF_, double S_, double L_, double ltqdc_, double stqdc_){
	// Integers
	structure.location = location_;
	
	// Doubles
	structure.TOF = TOF_;
	structure.S = S_;
	structure.L = L_;
	structure.liquid_tqdc = ltqdc_;
	structure.start_tqdc = stqdc_;
	
	// Bools
	structure.valid = true;
}

// Fill the local tree with processed data (to be called from detector driver)
bool LiquidProcessor::FillRoot(){
	if(!use_root){ return false; }
	local_tree->Fill();
	this->Zero();
	return true;
}

// Write the local tree to file
// Should only be called once per execution
bool LiquidProcessor::WriteRoot(TFile* masterFile){
	if(!masterFile || !local_tree){ return false; }
	masterFile->cd();
	local_tree->Write();
	std::cout << local_tree->GetEntries() << " entries\n";
	std::cout << " DEBUG: goodCount = " << goodCount << " badCount = " << badCount << std::endl;
	return true;
}
