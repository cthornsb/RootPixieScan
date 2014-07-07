/** \file IonChamberProcessor.hpp
 * \brief Processor for ion chamber
 */

#ifndef __IONCHAMBERPROCESSOR_HPP_
#define __IONCHAMBERPROCESSOR_HPP_

#include <deque>

#include "EventProcessor.hpp"

struct IonChamberDataStructure{
    // Add some variables later
};

class IonChamberProcessor : public EventProcessor 
{
 private:  
    static const size_t noDets = 6;
    static const size_t timesToKeep = 1000;
    static const double minTime;

    struct Data {
        double raw[noDets];
        double cal[noDets];
        int mult;

        void Clear(void);
    } data;

    double lastTime[noDets];
    std::deque<double> timeDiffs[noDets];
public:
    IonChamberProcessor(); // no virtual c'tors
    virtual bool InitDamm();
    virtual bool InitRoot();
    virtual bool Process(RawEvent &event);
    virtual bool FillRoot();
    virtual bool WriteRoot(TFile*);
    void PackRoot();
    
    IonChamberDataStructure structure;
};

#endif // __IONCHAMBERPROCSSEOR_HPP_
