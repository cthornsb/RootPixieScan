#ifndef ROOTDATASTRUCTURE_H
#define ROOTDATASTRUCTURE_H

struct BetaDataStructure{
    double energy;
    unsigned int multiplicity;
    bool valid;
};

struct LogicDataStructure{
    double tdiff;
    unsigned int location;
    bool is_start, valid;
};

struct RuntimeDataStructure{
    double energy;
    bool valid;
};

struct LiquidDataStructure{
    double TOF, S, L;
    double liquid_tqdc, start_tqdc;
    unsigned int location;
    bool valid;
};

struct VandleDataStructure{
    double tof, lqdc, rqdc, tsLow, tsHigh;
    double lMaxVal, rMaxVal, qdc, energy;
    unsigned int multiplicity, location; 
    bool valid;
};

#endif
