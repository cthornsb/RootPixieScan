/** \file EventProcessor.hpp
 * \brief Generic event processor
 */

#ifndef __EVENTPROCESSOR_HPP_
#define __EVENTPROCESSOR_HPP_

#include <map>
#include <set>
#include <string>
#include <time.h>

#ifdef USE_HHIRF
#include "Plots.hpp"
#endif

#include "TreeCorrelator.hpp"
#include "TimingInformation.hpp"

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"

// forward declarations
class DetectorSummary;
class RawEvent;

class EventProcessor : public TimingInformation{
 private:
    void _initialize();
    
 protected:
    // things associated with timing
	long total_time;
	clock_t start_time;

    // define the associated detector types and only initialize if present
    std::string name;
    std::set<std::string> associatedTypes; //--- set of type string
    bool didProcess, initDone;
    bool use_root, use_damm;
    TBranch *local_branch;
    unsigned int count;

    // map of associated detector summary
    std::map<std::string, const DetectorSummary *> sumMap;

#ifdef USE_HHIRF
    // Plots class for given Processor, takes care of declaration
    // and plotting within boundries allowed by PlotsRegistry
    Plots histo;

    virtual void plot(int dammId, double val1, double val2 = -1, double val3 = -1, const char* name="h") {
        histo.Plot(dammId, val1, val2, val3, name);
    }
    virtual void DeclareHistogram1D(int dammId, int xSize, const char* title) {
        histo.DeclareHistogram1D(dammId, xSize, title);
    }
    virtual void DeclareHistogram2D(int dammId, int xSize, int ySize, const char* title) {
        histo.DeclareHistogram2D(dammId, xSize, ySize, title);
    }
#endif

 public:
    EventProcessor();
    EventProcessor(int offset, int range);
    EventProcessor(std::string);
    EventProcessor(int offset, int range, std::string);
    virtual ~EventProcessor();

    virtual float Status(unsigned int);
    virtual bool InitDamm();
    virtual const std::set<std::string>& GetTypes(void) const {
      return associatedTypes; 
    }
    virtual bool DidProcess(void) const {
      return didProcess;
    }
    
    // Return true on success
    virtual bool HasEvent(void) const;
    virtual bool Init(RawEvent&);
    virtual bool CheckInit();
    virtual bool PreProcess(RawEvent &event);   
    virtual bool Process(RawEvent &event); 
    
    // Start the process timer
    void StartProcess(){ start_time = clock(); }
    
    // Update the amount of time taken by the processor
    void EndProcess(void){ total_time += (clock() - start_time); }
    
    std::string GetName(void) const {
      return name;
    }

    virtual void Zero();
    virtual bool InitRoot(TTree*);
        
    std::string GetName(){ return name; }
};

#endif // __EVENTPROCESSOR_HPP_
