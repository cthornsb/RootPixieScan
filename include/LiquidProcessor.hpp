/** \file LiquidProcessor.hpp
 *
 * Processor for liquid scintillator detectors
 */

#ifndef __LIQUIDPROCESSOR_HPP_
#define __LIQUIDPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "RootDataStructures.h"

class LiquidProcessor : public EventProcessor{
 public:
    LiquidProcessor(); // no virtual c'tors
    virtual bool InitDamm();
    virtual bool InitRoot();
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void Zero();
    virtual bool FillRoot();
    virtual bool WriteRoot(TFile*);
    void PackRoot(unsigned int, double, double, double, double, double);
    
    LiquidDataStructure structure;
       
 private:
    TimingInformation timeInfo;
    unsigned int goodCount, badCount;
};

#endif // __LIQUIDPROCSSEOR_HPP_
