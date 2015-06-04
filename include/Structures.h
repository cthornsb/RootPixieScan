/** \file Structures.h
 * \brief Data structures for root output
 * 
 * Special data types for Root output. Each individual processor which is
 * is used in the scan code should have its own Structure class. These classes
 * should contain simple C++ data types or vectors of simple C++ data types.
 * Vectors should be used for processors which are likely to have multiplicities
 * greater than one.
 *
 * \author C. Thornsbery
 * \date Sept. 25, 2014
 */

#ifndef ROOTDATAStructure_H
#define ROOTDATAStructure_H

#include "TObject.h"

#include <vector>

#define NUM_PIXIE_MOD 12
#define NUM_CHAN_PER_MOD 16

/** RawEventStructure
 * \brief Raw pixie16 channel data
 * 
 * Structure for raw pixie channels
 */
class RawEventStructure : public TObject {
  public:
  	std::vector<double> raw_energy[NUM_PIXIE_MOD][NUM_CHAN_PER_MOD]; /**< Raw pixie energy */
  	std::vector<double> raw_time[NUM_PIXIE_MOD][NUM_CHAN_PER_MOD]; /**< Raw pixie (low-res) time */
    unsigned int raw_mult; //! Multiplicity of the raw events
    unsigned int num_mod; //! Number of pixie modules

    RawEventStructure(unsigned int num_modules_=NUM_PIXIE_MOD);

	RawEventStructure(const RawEventStructure &other);

	~RawEventStructure();
    
    void Append(const int &mod_, const int &chan_, const double &time_, const double &energy_); /**< Fill the root variables with processed data */
    
    void Zero(); /**< Zero the data structure */
    
    void Set(RawEventStructure *other);
    
    ClassDefNV(RawEventStructure, 1); // RawEvent
};

/** TriggerStructure
 * \brief Trigger detector data structure
 * 
 * Structure for detectors of type "trigger"
 */
class TriggerStructure : public TObject {
  public:
	std::vector<double> trigger_time; /**< Raw pixie time */
    std::vector<double> trigger_energy; /**< Raw pixie energy */
    unsigned int trigger_mult; /**< Multiplicity of the trigger detector */

    TriggerStructure();

	TriggerStructure(const TriggerStructure &other);
    
    void Append(const double &time_, const double &energy_); /**< Fill the root variables with processed data */
    
    void Zero(); /**< Zero the data structure */
    
    void Set(TriggerStructure *other);
    
    ClassDefNV(TriggerStructure, 1); // Trigger
};

/** TriggerWaveform
 * \brief Trigger detector trace
 * 
 * Structure for detectors of type "trigger"
 */
class TriggerWaveform : public TObject {
  public:
	std::vector<int> trigger_wave; /**< Integer vector for trigger pulses */
	
	TriggerWaveform(){}
	
	TriggerWaveform(const TriggerWaveform &other){ trigger_wave = other.trigger_wave; }
	
	void Append(const std::vector<int> &pulse); /**< Fill the root variable with raw waveform data */
	
	void Zero(); /**< Zero the waveform */
	
	void Set(TriggerWaveform *other){ trigger_wave = other->trigger_wave; }
	
	ClassDefNV(TriggerWaveform, 1); // TriggerWaveform
};

/** LiquidStructure
 * \brief Liquid scintillator data structure
 * 
 * Structure for detectors of type "liquid"
 */
class LiquidStructure : public TObject {
  public:
    std::vector<double> liquid_TOF, liquid_tqdc, start_tqdc; /**< Double vectors for liquid scintillator detector variables */
    std::vector<int> liquid_loc; /**< Integer vector for liquid detector location */
    unsigned int liquid_mult; /**< Multiplicity of liquid detector event */

    LiquidStructure();

	LiquidStructure(const LiquidStructure &other);

    void Append(const unsigned int &location_, const double &TOF_, const double &ltqdc_, const double &stqdc_); /**< Fill the root variables with processed data */
       
    void Zero(); /**< Zero the data structure */ 
       
    void Set(LiquidStructure *other);
       
    ClassDefNV(LiquidStructure, 1); // Liquid
};

class LiquidWaveform : public TObject {
  public:
	std::vector<int> liquid_wave; // Integer vector for liquid pulses

	LiquidWaveform(){}

	LiquidWaveform(const LiquidWaveform &other){ liquid_wave = other.liquid_wave; }
	
	// Fill the root variable with raw waveform data
	void Append(const std::vector<int> &pulse);
	
	// Zero the waveform
	void Zero();
	
	void Set(LiquidWaveform *other){ liquid_wave = other->liquid_wave; }
	
	ClassDefNV(LiquidWaveform, 1); // LiquidWaveform
};

/** VandleStructure
 * \brief Vandle bar data structure
 * 
 * Structure for detectors of type "vandlesmall" or "vandlebig"
 */
class VandleStructure : public TObject {
  public:
    std::vector<double> vandle_TOF, vandle_lqdc, vandle_rqdc, vandle_tsLow, vandle_tsHigh;
    std::vector<double> vandle_lMaxVal, vandle_rMaxVal, vandle_qdc, vandle_energy;
    
    std::vector<double> vandle_recoilEnergy, vandle_recoilAngle, vandle_ejectAngle;
    std::vector<double> vandle_exciteEnergy, vandle_flightPath, vandle_xflightPath;
    std::vector<double> vandle_yflightPath, vandle_zflightPath;
    
    std::vector<int> vandle_loc;
    unsigned int vandle_mult; 

    VandleStructure();

	VandleStructure(const VandleStructure &other);
    
    // Add an entry to the data vector
    // Calling this method will mark the event as valid
	void Append(const unsigned int &location_, const double &tof_, const double &lqdc_, const double &rqdc_, const double &tsLow_, 
			     const double &tsHigh_, const double &lMaxVal_, const double &rMaxVal_, const double &qdc_, const double &energy_, const double &recoilE_,
			     const double &recoilAngle_, const double &ejectAngle_, const double &excitedE_, const double &flightPath_, const double &x_, const double &y_, const double &z_);
    
    // Zero the data structure
    void Zero();
    
    void Set(VandleStructure *other);
    
    ClassDefNV(VandleStructure, 1); // Vandle
};

class VandleWaveform : public TObject {
  public:
	std::vector<int> left_wave, right_wave; // Integer vectors for left and right vandle pulses
	
	VandleWaveform(){}
	
	VandleWaveform(const VandleWaveform &other);
	
	// Fill the root variable with raw waveform data
	void Append(const std::vector<int> &l_pulse, const std::vector<int> &r_pulse);
	
	// Zero the waveform
	void Zero();
	
	void Set(VandleWaveform *other);
	
	ClassDefNV(VandleWaveform, 1); // VandleWaveform
};

///////////////////////////////////////////////////////////
// IonChamberProcessor
///////////////////////////////////////////////////////////
class IonChamberStructure : public TObject {
  public:
    std::vector<double> ion_dE, ion_E, ion_sum;
    unsigned int ion_mult; 

    IonChamberStructure();

	IonChamberStructure(const IonChamberStructure &other);
    
    // Add an entry to the data vector
    // Calling this method will mark the event as valid
    void Append(const double &, const double &);
    
    // Zero the data structure
    void Zero();
    
    void Set(IonChamberStructure *other);
    
    ClassDefNV(IonChamberStructure, 1); // Vandle
};

#endif
