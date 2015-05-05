/** \file IonChamberProcessor.cpp
 *
 * implementation for ion chamber processor
 */
#include <vector>
#include <sstream>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "ChanEvent.hpp"
#include "IonChamberProcessor.hpp"

using namespace std;
using namespace dammIds::ionChamber;

IonChamberProcessor::IonChamberProcessor() : EventProcessor(OFFSET, RANGE) {
    name = "IonChamber";
    associatedTypes.insert("ion"); 
    save_waveforms = false;
}

IonChamberProcessor::IonChamberProcessor(bool save_waveforms_) : EventProcessor(OFFSET, RANGE) {
    name = "IonChamber";
    associatedTypes.insert("ion"); 
    save_waveforms = save_waveforms_;
}

bool IonChamberProcessor::InitDamm(void) {
#ifdef USE_HHIRF
    std::cout << " IonChamberProcessor: Initializing the damm output\n";
    if(use_damm){
        std::cout << " IonChamberProcessor: Warning! Damm output already initialized\n";
        return false;
    }
    
    DeclareHistogram2D(D_DE_E_ION, SC, SC, "IonChamber dE vs. E");
    DeclareHistogram1D(D_MULT_ION, S4, "IonChamber multiplicity");
    DeclareHistogram1D(D_SUM_ION, SE, "IonChamber sum");
    
    use_damm = true;
    return true;
#else
	return false;
#endif
}

// Initialize for root output
bool IonChamberProcessor::InitRoot(TTree* top_tree){
    if(!top_tree){
        use_root = false;
        return false;
    }

    // Create the branch
    local_branch = top_tree->Branch("IonChamber", &structure);
    if(save_waveforms){
    	std::cout << " IonChamberProcessor: Writing of raw waveforms is disabled!\n";
    	//std::cout << " IonChamberProcessor: Dumping raw waveforms to root file\n";
    	//local_branch = top_tree->Branch("IonChamberWave", &waveform);
    }

    use_root = true;
    return true;
}

// Returns true ONLY if there is data to fill to the root tree
bool IonChamberProcessor::Process(RawEvent &event){
    if(!initDone){ return (didProcess = false); }
    bool output = false;

	// Start the process timer
	StartProcess();

    static const vector<ChanEvent*> &ionDelta = event.GetSummary("ion:dE")->GetList();
    static const vector<ChanEvent*> &ionEnergy = event.GetSummary("ion:E")->GetList();

    unsigned int multiplicity = 0;
    vector<ChanEvent*>::const_iterator iter1, iter2;
    for(iter1 = ionDelta.begin(), iter2 = ionEnergy.begin(); iter1 != ionDelta.end() && iter2 != ionEnergy.end(); iter1++, iter2++) {
        double dE = (*iter1)->GetEnergy();
        double E = (*iter2)->GetEnergy();
#ifdef USE_HHIRF
        if(use_damm){ 
        	plot(D_DE_E_ION, E, dE); 
        	plot(D_SUM_ION, E+dE);
        }
#endif
        if(use_root){ 
            structure.Append(dE, E);
            //if(save_waveforms){ waveform.Append(trigger.trace); }
            if(!output){ output = true; }
            count++;
        }
        multiplicity++;
    }
#ifdef USE_HHIRF
    if(use_damm){ plot(D_MULT_ION, multiplicity); }
#endif
    
    EndProcess();
    return output;
}
