/** \file BetaProcessor.hpp
 *
 * Processor for beta scintillator detectors
 */

#ifndef __BETAPROCESSOR_HPP_
#define __BETAPROCESSOR_HPP_

#include <vector>

#include "EventProcessor.hpp"
#include "Structures.h"

class BetaProcessor : public EventProcessor
{
public:
    BetaProcessor();
    virtual bool InitDamm();
    virtual bool InitRoot(TTree*);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void Zero(){ structure.Zero(); }
    void PackRoot(std::vector<double>&, unsigned int);
    
    BetaDataStructure structure;
};

#endif // __BETAPROCESSOR_HPP_
