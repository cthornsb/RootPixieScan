#ifndef ROOTDATAStructure_H
#define ROOTDATAStructure_H

#include "TObject.h"

class BetaDataStructure : public TObject {
  public:
    double energy;
    unsigned int multiplicity;
    bool valid;
    
    BetaDataStructure();
    
    void Zero();
    ClassDefNV(BetaDataStructure, 1); // BetaDataStructure
};

class LogicDataStructure : public TObject {
  public:
    double tdiff;
    unsigned int location;
    bool is_start, valid;

    LogicDataStructure();
    
    void Zero();
    ClassDefNV(LogicDataStructure, 1); // LogicDataStructure
};

class RuntimeDataStructure : public TObject {
  public:
    double energy;
    bool valid;

    RuntimeDataStructure();
    
    void Zero();
    ClassDefNV(RuntimeDataStructure, 1); // RuntimeDataStructure
};

class LiquidDataStructure : public TObject {
  public:
    double TOF, S, L;
    double liquid_tqdc, start_tqdc;
    unsigned int location;
    bool valid;

    LiquidDataStructure();
    
    void Zero();
    ClassDefNV(LiquidDataStructure, 1); // LiquidDataStructure
};

class VandleDataStructure : public TObject {
  public:
    double tof, lqdc, rqdc, tsLow, tsHigh;
    double lMaxVal, rMaxVal, qdc, energy;
    unsigned int multiplicity, location; 
    bool valid;

    VandleDataStructure();
    
    void Zero();
    ClassDefNV(VandleDataStructure, 1); // VandleDataStructure
};

#endif
