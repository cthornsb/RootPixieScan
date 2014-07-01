/** \file BetaProcessor.hpp
 *
 * Processor for beta scintillator detectors
 */

#ifndef __BETAPROCESSOR_HPP_
#define __BETAPROCESSOR_HPP_

#include <vector>

#include "EventProcessor.hpp"

struct BetaDataStructure{
    double energy;
    unsigned int multiplicity;
};


class BetaProcessor : public EventProcessor
{
public:
    BetaProcessor();
    virtual void DeclarePlots(void);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual bool InitRoot();
    virtual bool WriteRoot(TFile*);
    bool PackRoot(std::vector<double>&, unsigned int);
    bool InitDamm();
    bool PackDamm();
    
    BetaDataStructure structure;
};

#endif // __BETAPROCESSOR_HPP_
