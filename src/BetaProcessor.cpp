/** \file BetaProcessor.cpp
 *
 * implementation for beta scintillator processor
 */
#include <vector>
#include <sstream>

#include "DammPlotIds.hpp"
#include "RawEvent.hpp"
#include "ChanEvent.hpp"
#include "BetaProcessor.hpp"

using namespace std;
using namespace dammIds::scint::beta;

namespace dammIds {
    namespace scint {
        namespace beta {
            const int D_MULT_BETA = 0;
            const int D_ENERGY_BETA = 1;
        }
    }
} 

BetaProcessor::BetaProcessor() : EventProcessor(OFFSET, RANGE) {
    name = "Beta";
    associatedTypes.insert("scint"); 
}

bool BetaProcessor::InitDamm(void) {
    std::cout << " BetaProcessor: Initializing the damm output\n";
    if(use_damm){
        std::cout << " BetaProcessor: Warning! Damm output already initialized\n";
        return false;
    }
    
    DeclareHistogram1D(D_MULT_BETA, S4, "Beta multiplicity");
    DeclareHistogram1D(D_ENERGY_BETA, SE, "Beta energy");
    
    use_damm = true;
    return true;
}

// Initialize for root output
bool BetaProcessor::InitRoot(TTree* top_tree){
    if(!top_tree){
        use_root = false;
        return false;
    }
	
    // Create the branch
    local_branch = top_tree->Branch("Beta", &structure);

    use_root = true;
    return true;
}

// Returns true ONLY if there is data to fill to the root tree
bool BetaProcessor::PreProcess(RawEvent &event){
    if(!initDone){ return (didProcess = false); }
    bool output = false;

    static const vector<ChanEvent*> &scintBetaEvents =  event.GetSummary("scint:beta")->GetList();

    unsigned int multiplicity = 0;
    for (vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin(); it != scintBetaEvents.end(); it++) {
        double energy = (*it)->GetEnergy();
        if (energy > detectors::betaThreshold){ ++multiplicity; }
        if(use_damm){ plot(D_ENERGY_BETA, energy); }
        if(use_root){ 
            structure.Append(energy); 
            if(!output){ output = true; }
            count++;
        }
    }
    if(use_damm){ plot(D_MULT_BETA, multiplicity); }
    return output;
}

// Returns true ONLY if there is data to fill to the root tree
bool BetaProcessor::Process(RawEvent &event)
{
    if(!initDone){ return (didProcess = false); }

    // start the process timer
    times(&tmsBegin);
        
    EndProcess();
    return false;
}
