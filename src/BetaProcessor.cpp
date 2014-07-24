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
    std::cout << " BetaProcessor: Initializing root output\n";
    if(use_root){
        std::cout << " BetaProcessor: Warning! Root output already initialized\n";
        return false;
    }
	
    // Create the branch
    //local_branch = top_tree->Branch("Beta", &structure, "energy/D:multiplicity/i:valid/O");
    local_branch = top_tree->Branch("Beta", &structure);

    use_root = true;
    return true;
}

bool BetaProcessor::PreProcess(RawEvent &event){
    if (!EventProcessor::PreProcess(event))
        return false;

    static const vector<ChanEvent*> &scintBetaEvents =  event.GetSummary("scint:beta")->GetList();
    std::vector<double> energies;

    unsigned int multiplicity = 0;
    for (vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin(); it != scintBetaEvents.end(); it++) {
        double energy = (*it)->GetEnergy();
        if (energy > detectors::betaThreshold)
            ++multiplicity;
        if(use_damm){ plot(D_ENERGY_BETA, energy); }
        energies.push_back(energy);
    }
    if(use_damm){ plot(D_MULT_BETA, multiplicity); }
    if(use_root){ PackRoot(energies, multiplicity); }
    return true;
}

bool BetaProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;
        
    EndProcess();
    return true;
}

// Fill the root variables with processed data
void BetaProcessor::PackRoot(std::vector<double> &energy_, unsigned int multiplicity_){
	// Quick fix just to get this processor working,
	// energy should be a vector (Fix later!)
	for(unsigned int i = 0; i < energy_.size(); i++){
		structure.energy = energy_[i];
		structure.multiplicity = multiplicity_;
	}
	
	structure.valid = true;
	count++;
}
