/** \file ScintProcessor.hpp
 * \brief Processor for scintillator detectors
 */

#ifndef __SCINTPROCESSOR_HPP_
#define __SCINTPROCESSOR_HPP_

#include "EventProcessor.hpp"

struct ScintDataStructure{
    // Add some variables later
};

class ScintProcessor : public EventProcessor
{
public:
    ScintProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual bool InitRoot();
    virtual bool WriteRoot(TFile*);
    bool PackRoot();
    bool InitDamm();
    bool PackDamm();
    
    ScintDataStructure scint;
    
private:
   virtual void LiquidAnalysis(RawEvent &event);
   unsigned int counter;
};

#endif // __SCINTPROCSSEOR_HPP_
