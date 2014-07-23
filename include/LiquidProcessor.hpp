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
    LiquidProcessor(); // no virtual c'tors
    virtual bool InitDamm();
    virtual bool InitRoot(TTree*);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void Zero(){ structure.Zero(); }
    void PackRoot(unsigned int, double, double, double, double, double);
    
    LiquidDataStructure structure;
       
 private:
    TimingInformation timeInfo;
    unsigned int goodCount, badCount;
};

#endif // __LIQUIDPROCSSEOR_HPP_
