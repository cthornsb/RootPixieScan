/** \file TriggerProcessor.hpp
 *
 * Processor for trigger scintillator detectors
 */

#ifndef __TRIGGERPROCESSOR_HPP_
#define __TRIGGERPROCESSOR_HPP_

#include <vector>

#include "EventProcessor.hpp"
#include "Structures.h"

class TriggerProcessor : public EventProcessor
{
  public:
    TriggerProcessor();
    TriggerProcessor(bool);
    virtual bool InitDamm();
    virtual bool InitRoot(TTree*);
    virtual bool PreProcess(RawEvent &event);
    virtual void Zero(){ 
    	structure.Zero();
    	if(save_waveforms){ waveform.Zero(); }
    }
    
    TriggerStructure structure;
    TriggerWaveform waveform;
    
  private:
	bool save_waveforms;
};

#endif // __TRIGGERPROCESSOR_HPP_
