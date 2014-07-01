/** \file LiquidProcessor.hpp
 *
 * Processor for liquid scintillator detectors
 */

#ifndef __LIQUIDPROCESSOR_HPP_
#define __LIQUIDPROCESSOR_HPP_

/*#include "EventProcessor.hpp"
#include "Trace.hpp"

#include "TFile.h"
#include "TTree.h"*/

#include "EventProcessor.hpp"

class TFile;

struct LiquidDataStructure{
    double TOF, S, L;
    double liquid_tqdc, start_tqdc;
    unsigned int location;
};

class LiquidProcessor : public EventProcessor{
 public:
    LiquidProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual bool InitRoot();
    virtual bool WriteRoot(TFile*);
    bool PackRoot(unsigned int, double, double, double, double, double);
    bool InitDamm();
    bool PackDamm();
    
    LiquidDataStructure structure;
       
 private:
    TimingInformation timeInfo;
    unsigned int goodCount, badCount;
};

#endif // __LIQUIDPROCSSEOR_HPP_
