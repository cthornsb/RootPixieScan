/** \file LiquidProcessor.hpp
 *
 * Processor for liquid scintillator detectors
 */

#ifndef __LIQUIDPROCESSOR_HPP_
#define __LIQUIDPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "Structures.h"

class LiquidProcessor : public EventProcessor{
 public:
    LiquidProcessor();
    LiquidProcessor(bool);
    virtual bool InitDamm();
    virtual bool InitRoot(TTree*);
    virtual bool Process(RawEvent &event);
    virtual void Zero(){ 
    	structure.Zero();
    	if(save_waveforms){ waveform.Zero(); }
    }
    
    LiquidStructure structure;
    LiquidWaveform waveform;
       
 private:
    TimingInformation timeInfo;
    unsigned int goodCount, badCount;
    bool save_waveforms;    
};

#endif // __LIQUIDPROCSSEOR_HPP_
