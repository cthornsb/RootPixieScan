/** \file BetaProcessor.hpp
 *
 * Processor for beta scintillator detectors
 */

#ifndef __BETAPROCESSOR_HPP_
#define __BETAPROCESSOR_HPP_

#include <vector>

#include "EventProcessor.hpp"
#include "RootDataStructures.h"

class BetaProcessor : public EventProcessor
{
public:
    BetaProcessor();
    virtual bool InitDamm();
    virtual bool InitRoot();
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void Zero();    
    virtual bool FillRoot();
    virtual bool WriteRoot(TFile*);
    void PackRoot(std::vector<double>&, unsigned int);
    
    BetaDataStructure structure;
};

#endif // __BETAPROCESSOR_HPP_
