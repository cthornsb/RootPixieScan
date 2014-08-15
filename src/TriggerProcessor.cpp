/** \file TriggerProcessor.cpp
 *
 * implementation for trigger scintillator processor
 */
#include <vector>
#include <sstream>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "ChanEvent.hpp"
#include "TriggerProcessor.hpp"

using namespace std;
using namespace dammIds::scint::trigger;

namespace dammIds {
    namespace scint {
        namespace trigger {
            const int D_MULT_TRIGGER = 0;
            const int D_ENERGY_TRIGGER = 1;
        }
    }
} 

TriggerProcessor::TriggerProcessor() : EventProcessor(OFFSET, RANGE) {
    name = "Trigger";
    associatedTypes.insert("scint"); 
    save_waveforms = false;
}

TriggerProcessor::TriggerProcessor(bool save_waveforms_) : EventProcessor(OFFSET, RANGE) {
    name = "Trigger";
    associatedTypes.insert("scint"); 
    save_waveforms = save_waveforms_;
}

void TriggerProcessor::Status(unsigned int total_events){
    if (initDone) {
		// output the time usage and the number of valid events
		cout << " TriggerProcessor: User Time = " << userTime << ", System Time = " << systemTime << endl;
		if(total_events > 0){ cout << " TriggerProcessor: " << count << " Valid Events (" << 100.0*count/total_events << "%)\n"; }
		if(count != total_events){ 
			cout << " TriggerProcessor: Warning! Number of trigger events does not match total!\n";
		}
    }
}

bool TriggerProcessor::InitDamm(void) {
    std::cout << " TriggerProcessor: Initializing the damm output\n";
    if(use_damm){
        std::cout << " TriggerProcessor: Warning! Damm output already initialized\n";
        return false;
    }
    
    DeclareHistogram1D(D_MULT_TRIGGER, S4, "Trigger multiplicity");
    DeclareHistogram1D(D_ENERGY_TRIGGER, SE, "Trigger energy");
    
    use_damm = true;
    return true;
}

// Initialize for root output
bool TriggerProcessor::InitRoot(TTree* top_tree){
    if(!top_tree){
        use_root = false;
        return false;
    }

    // Create the branch
    local_branch = top_tree->Branch("Trigger", &structure);
    if(save_waveforms){
    	std::cout << " TriggerProcessor: Dumping raw waveforms to root file\n";
    	local_branch = top_tree->Branch("TriggerWave", &waveform);
    }

    use_root = true;
    return true;
}

// Returns true ONLY if there is data to fill to the root tree
bool TriggerProcessor::PreProcess(RawEvent &event){
    if(!initDone){ return (didProcess = false); }
    bool output = false;

    static const vector<ChanEvent*> &scintTriggerEvents =  event.GetSummary("scint:trigger")->GetList();

    unsigned int multiplicity = 0;
    for (vector<ChanEvent*>::const_iterator it = scintTriggerEvents.begin(); it != scintTriggerEvents.end(); it++) {
    	TimingInformation::TimingData trigger((*it));
        double energy = (*it)->GetEnergy();
        if (energy > detectors::triggerThreshold){ ++multiplicity; }
        if(use_damm){ plot(D_ENERGY_TRIGGER, energy); }
        if(use_root){ 
            structure.Append(energy);
            if(save_waveforms){ waveform.Append(trigger.trace); }
            if(!output){ output = true; }
            count++;
        }
    }
    if(use_damm){ plot(D_MULT_TRIGGER, multiplicity); }
    return output;
}

// Returns true ONLY if there is data to fill to the root tree
bool TriggerProcessor::Process(RawEvent &event)
{
    if(!initDone){ return (didProcess = false); }

    // start the process timer
    times(&tmsBegin);
        
    EndProcess();
    return false;
}
