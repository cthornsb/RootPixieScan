#ifndef ROOTDATAStructure_H
#define ROOTDATAStructure_H

#include "TObject.h"

#include <vector>

///////////////////////////////////////////////////////////
// TriggerProcessor
///////////////////////////////////////////////////////////
class TriggerStructure : public TObject {
  public:
    std::vector<double> trigger_energy;
    unsigned int trigger_mult;

    TriggerStructure();

    // Fill the root variables with processed data
    void Append(double);

    // Zero the data structure
    void Zero();
    
    ClassDefNV(TriggerStructure, 1); // Trigger
};

class TriggerWaveform : public TObject {
  public:
	std::vector<int> trigger_wave; // Integer vector for trigger pulses
	
	// Fill the root variable with raw waveform data
	void Append(std::vector<int>&);
	
	// Zero the waveform
	void Zero();
	
	ClassDefNV(TriggerWaveform, 1); // TriggerWaveform
};

///////////////////////////////////////////////////////////
// LogicProcessor:Runtime
///////////////////////////////////////////////////////////
class RuntimeStructure : public TObject {
  public:
    std::vector<double> rtime_energy;
    unsigned int rtime_mult;

    RuntimeStructure();

    // Fill the root variables with processed data
    void Append(double);

    // Zero the data structure    
    void Zero();
    
    ClassDefNV(RuntimeStructure, 1); // Runtime
};

class RuntimeWaveform : public TObject {
  public:
	std::vector<int> rtime_wave; // Integer vectors for left and right vandle pulses
	
	// Fill the root variable with raw waveform data
	void Append(std::vector<int>&);
	
	// Zero the waveform
	void Zero();
	
	ClassDefNV(RuntimeWaveform, 1); // RuntimeWaveform
};	

///////////////////////////////////////////////////////////
// LiquidProcessor
///////////////////////////////////////////////////////////
class LiquidStructure : public TObject {
  public:
    std::vector<double> liquid_TOF, liquid_tqdc, start_tqdc;
    std::vector<int> liquid_loc;
    unsigned int liquid_mult;

    LiquidStructure();

    // Fill the root variables with processed data
    //void Append(unsigned int, double, double, double, double, double);
    void Append(unsigned int, double, double, double);
    
    // Zero the data structure    
    void Zero();
        
    ClassDefNV(LiquidStructure, 1); // Liquid
};

class LiquidWaveform : public TObject {
  public:
	std::vector<int> liquid_wave; // Integer vector for liquid pulses
	
	// Fill the root variable with raw waveform data
	void Append(std::vector<int>&);
	
	// Zero the waveform
	void Zero();
	
	ClassDefNV(LiquidWaveform, 1); // LiquidWaveform
};

///////////////////////////////////////////////////////////
// VandleProcessor
///////////////////////////////////////////////////////////
class VandleStructure : public TObject {
  public:
    std::vector<double> vandle_TOF, vandle_lqdc, vandle_rqdc, vandle_tsLow, vandle_tsHigh;
    std::vector<double> vandle_lMaxVal, vandle_rMaxVal, vandle_qdc, vandle_energy;
    std::vector<int> vandle_loc;
    unsigned int vandle_mult; 

    VandleStructure();
    
    // Add an entry to the data vector
    // Calling this method will mark the event as valid
    void Append(unsigned int, double, double, double, double, double, double, double, double, double);
    
    // Zero the data structure
    void Zero();
    
    ClassDefNV(VandleStructure, 1); // Vandle
};

class VandleWaveform : public TObject {
  public:
	std::vector<int> left_wave, right_wave; // Integer vectors for left and right vandle pulses
	
	// Fill the root variable with raw waveform data
	void Append(std::vector<int>&, std::vector<int>&);
	
	// Zero the waveform
	void Zero();
	
	ClassDefNV(VandleWaveform, 1); // VandleWaveform
};

#endif
