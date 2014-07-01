/** \file LogicProcessor.hpp
 * \brief Class to handle logic signals 
 * derived originally from MTC processor
 */

#ifndef __LOGICPROCESSOR_HPP_
#define __LOGICPROCESSOR_HPP_

#include <vector>

#include "EventProcessor.hpp"

struct LogicDataStructure{
    double tdiff;
    unsigned int location;
    bool is_start;
};

struct RuntimeDataStructure{
    double energy;
};

class LogicProcessor : public EventProcessor {
private:
    void BasicProcessing(RawEvent &event);
    void TriggerProcessing(RawEvent &event);
    int plotSize;
protected:
    std::vector<double> lastStartTime; //< time of last leading edge
    std::vector<double> lastStopTime;  //< time of last trailing edge
    std::vector<bool>   logicStatus;   //< current level of the logic signal

    std::vector<unsigned long> stopCount;  //< number of stops received
    std::vector<unsigned long> startCount; //< number of starts received
 public:
    LogicProcessor();
    virtual void DeclarePlots(void);
    virtual bool Process(RawEvent &event);
    virtual bool LogicStatus(size_t loc) const {
      return logicStatus.at(loc);
    }
    unsigned long StopCount(size_t loc) const {
	return stopCount.at(loc);
    }
    unsigned long StartCount(size_t loc) const {
	return startCount.at(loc);
    }
    double TimeOff(size_t loc, double t) const {
	return (!LogicStatus(loc) ? (t-lastStopTime.at(loc)) : 0.);
    }
    double TimeOn(size_t loc, double t) const {
	return (LogicStatus(loc) ? (t-lastStartTime.at(loc)) : 0.);
    }
    virtual bool InitRoot();
    virtual bool WriteRoot(TFile*);
    bool PackRoot(double);
    //bool PackRoot(double, unsigned int, bool);
    bool InitDamm();
    bool PackDamm();
    
    RuntimeDataStructure structure;
    //LogicDataStructure structure;
};

#endif // __LOGICPROCESSOR_HPP_
