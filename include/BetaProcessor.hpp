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
    BetaProcessor(bool);
    virtual bool InitDamm();
    virtual bool InitRoot(TTree*);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void Zero(){ 
    	if(!save_waveforms){ structure.Zero(); }
    	else{ waveform.Zero(); }
    }
    
    BetaStructure structure;
    BetaWaveform waveform;
    
  private:
	bool save_waveforms;
};

#endif // __BETAPROCESSOR_HPP_
