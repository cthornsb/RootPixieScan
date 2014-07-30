#ifndef ROOTDATAStructure_H
#define ROOTDATAStructure_H

#include "TObject.h"

#include <vector>

///////////////////////////////////////////////////////////
// BetaProcessor
///////////////////////////////////////////////////////////
class BetaStructure : public TObject {
  public:
    std::vector<double> beta_energy;
    unsigned int beta_mult;
    bool beta_valid;

    BetaStructure();

    // Fill the root variables with processed data
    void Append(double);

    // Zero the data structure
    void Zero();
    
    ClassDefNV(BetaStructure, 1); // Beta
};

///////////////////////////////////////////////////////////
// LogicProcessor:Logic
///////////////////////////////////////////////////////////
/*class LogicStructure : public TObject {
  public:
    double logic_tdiff;
    unsigned int logic_loc;
    bool logic_start, logic_valid;

    LogicStructure();

    // Zero the data structure    
    void Zero();
    
    // Fill the root variables with processed data
    void Pack(double, unsigned int, bool);
    
    ClassDefNV(LogicStructure, 1); // Logic
};*/

///////////////////////////////////////////////////////////
// LogicProcessor:Runtime
///////////////////////////////////////////////////////////
class RuntimeStructure : public TObject {
  public:
    std::vector<double> rtime_energy;
    unsigned int rtime_mult;
    bool rtime_valid;

    RuntimeStructure();

    // Fill the root variables with processed data
    void Append(double);

    // Zero the data structure    
    void Zero();
    
    ClassDefNV(RuntimeStructure, 1); // Runtime
};

///////////////////////////////////////////////////////////
// LiquidProcessor
///////////////////////////////////////////////////////////
/*struct LiquidData{
    double liquid_TOF, liquid_S, L, liquid_tqdc, start_tqdc;
    int liquid_loc;
    
    LiquidData(){ Zero(); }
    LiquidData(unsigned int, double, double, double, double, double);

    // Zero the data structure
    void Zero();
};*/

class LiquidStructure : public TObject {
  public:
    //std::vector<LiquidData*> liquid_data;
    std::vector<double> liquid_TOF, liquid_S, liquid_L;
    std::vector<double> liquid_tqdc, start_tqdc;
    std::vector<int> liquid_loc;
    unsigned int liquid_mult;
    bool liquid_valid;

    LiquidStructure();

    // Fill the root variables with processed data
    //void Append(LiquidData*);
    void Append(unsigned int, double, double, double, double, double);
    
    // Zero the data structure    
    void Zero();
        
    ClassDefNV(LiquidStructure, 1); // Liquid
};

class LiquidWaveform : public TObject {
    public:
	int liquid_wave[124]; // Integer array for waveform
	bool liquid_valid;
	
	// Fill the root variable with raw waveform data
	void Append(std::vector<int>&);
	
	// Zero the waveform
	void Zero();
	
	ClassDefNV(LiquidWaveform, 1); // LiquidWaveform
};

///////////////////////////////////////////////////////////
// VandleProcessor
///////////////////////////////////////////////////////////
/*struct VandleData{
    double vandle_TOF, vandle_lqdc, vandle_rqdc, vandle_tsLow, vandle_tsHigh;
    double vandle_lMaxVal, vandle_rMaxVal, vandle_qdc, vandle_energy;
    int vandle_loc;
    
    VandleData(){ Zero(); }
    VandleData(unsigned int, double, double, double, double, double, double, double, double, double);

    // Zero the data structure
    void Zero();
};*/

class VandleStructure : public TObject {
  public:
    //std::vector<VandleData*> vandle_data;
    std::vector<double> vandle_TOF, vandle_lqdc, vandle_rqdc, vandle_tsLow, vandle_tsHigh;
    std::vector<double> vandle_lMaxVal, vandle_rMaxVal, vandle_qdc, vandle_energy;
    std::vector<int> vandle_loc;
    unsigned int vandle_mult; 
    bool vandle_valid;

    VandleStructure();
    
    // Add an entry to the data vector
    // Calling this method will mark the event as valid
    //void Append(VandleData*);
    void Append(unsigned int, double, double, double, double, double, double, double, double, double);
    
    // Zero the data structure
    void Zero();
    
    ClassDefNV(VandleStructure, 1); // Vandle
};

#endif
