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

void BetaProcessor::DeclarePlots(void) {
    DeclareHistogram1D(D_MULT_BETA, S4, "Beta multiplicity");
    DeclareHistogram1D(D_ENERGY_BETA, SE, "Beta energy");
}

bool BetaProcessor::PreProcess(RawEvent &event){
    if (!EventProcessor::PreProcess(event))
        return false;

    static const vector<ChanEvent*> &scintBetaEvents =  event.GetSummary("scint:beta")->GetList();
    std::vector<double> energies;

    unsigned int multiplicity = 0;
    for (vector<ChanEvent*>::const_iterator it = scintBetaEvents.begin(); 
	 it != scintBetaEvents.end(); it++) {
        double energy = (*it)->GetEnergy();
        if (energy > detectors::betaThreshold)
            ++multiplicity;
        plot(D_ENERGY_BETA, energy);
        energies.push_back(energy);
    }
    plot(D_MULT_BETA, multiplicity);
    PackRoot(energies, multiplicity);
    return true;
}

bool BetaProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return false;
    EndProcess();
    return true;
}

// Initialize for root output
bool BetaProcessor::InitRoot(){
	std::cout << " BetaProcessor: Initializing\n";
	if(outputInit){
		std::cout << " BetaProcessor: Warning! Output already initialized\n";
		return false;
	}
	
	// Create the branch
	local_tree = new TTree(name.c_str(),name.c_str());
	local_branch = local_tree->Branch("Beta", &structure, "energy/D:multiplicity/i");
	outputInit = true;
	return true;
}

// Fill the root variables with processed data
bool BetaProcessor::PackRoot(std::vector<double> &energy_, unsigned int multiplicity_){
	if(!outputInit){ return false; }
	// Quick fix just to get this processor working,
	// energy should be a vector (Fix later!)
	for(unsigned int i = 0; i < energy_.size(); i++){
		structure.energy = energy_[i];
		structure.multiplicity = multiplicity_;
		local_tree->Fill();
	}
        return true;
}

// Write the local tree to file
// Should only be called once per execution
bool BetaProcessor::WriteRoot(TFile* masterFile){
	if(!masterFile || !local_tree){ return false; }
	masterFile->cd();
	local_tree->Write();
	std::cout << local_tree->GetEntries() << " entries\n";
	return true;
}

bool BetaProcessor::InitDamm(){
	return false;
}

bool BetaProcessor::PackDamm(){
	return false;
}
