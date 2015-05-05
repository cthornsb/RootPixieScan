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
    
    void Append(double, double); /**< Fill the root variables with processed data */
    
    void Zero(); /**< Zero the data structure */
    
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
	
	void Append(std::vector<int>&); /**< Fill the root variable with raw waveform data */
	
	void Zero(); /**< Zero the waveform */
	
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

    void Append(unsigned int, double, double, double); /**< Fill the root variables with processed data */
       
    void Zero(); /**< Zero the data structure */ 
        
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
    
    // Add an entry to the data vector
    // Calling this method will mark the event as valid
    void Append(unsigned int, double, double, double, double, double, double, double, double,
    			double, double, double, double, double, double, double, double, double);
    
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

///////////////////////////////////////////////////////////
// IonChamberProcessor
///////////////////////////////////////////////////////////
class IonChamberStructure : public TObject {
  public:
    std::vector<double> ion_dE, ion_E, ion_sum;
    unsigned int ion_mult; 

    IonChamberStructure();
    
    // Add an entry to the data vector
    // Calling this method will mark the event as valid
    void Append(double, double);
    
    // Zero the data structure
    void Zero();
    
    ClassDefNV(IonChamberStructure, 1); // Vandle
};

#endif
