/** \file IonChamberProcessor.hpp
 *
 * Processor for trigger scintillator detectors
 */

#ifndef __IONCHAMBERPROCESSOR_HPP_
#define __IONCHAMBERPROCESSOR_HPP_

#include <vector>

#include "EventProcessor.hpp"
#include "Structures.h"

class IonChamberProcessor : public EventProcessor
{
  public:
    IonChamberProcessor();
    IonChamberProcessor(bool);
    virtual bool InitDamm();
    virtual bool InitRoot(TTree*);
    virtual bool Process(RawEvent &event);
    virtual void Zero(){ 
    	structure.Zero();
    	//if(save_waveforms){ waveform.Zero(); }
    }
    
    IonChamberStructure structure;
    //IonChamberWaveform waveform;
    
  private:
	bool save_waveforms;
};

#endif // __TRIGGERPROCESSOR_HPP_
