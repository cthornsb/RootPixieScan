#define BETA_WAVE_SIZE 62
#define LIQUID_WAVE_SIZE 124

#include "Structures.h"

///////////////////////////////////////////////////////////
// BetaProcessor
///////////////////////////////////////////////////////////
BetaStructure::BetaStructure(){ beta_mult = 0; }

void BetaStructure::Append(double energy_){
	beta_energy.push_back(energy_);
	beta_mult++;
}

void BetaStructure::Zero(){
	if(beta_mult == 0){ return ; } // Structure is already empty
	beta_energy.clear();
	beta_mult = 0;
}

void BetaWaveform::Append(std::vector<int> &pulse){
	unsigned int count = 0;
	for(std::vector<int>::iterator iter = pulse.begin(); iter != pulse.end(); iter++){
		if(count >= BETA_WAVE_SIZE){ break; }
		beta_wave[count] = *iter;
		count++;
	}
	beta_valid = true;
}

void BetaWaveform::Zero(){
	for(unsigned short i = 0; i < BETA_WAVE_SIZE; i++){ beta_wave[i] = 0; }
	beta_valid = false;
}

///////////////////////////////////////////////////////////
// LogicProcessor:Logic
///////////////////////////////////////////////////////////
/*LogicStructure::LogicStructure(){ Zero(); }

void LogicStructure::Zero(){
	logic_tdiff = 0.0; logic_loc = 0; 
	logic_start = false; logic_valid = false;
}

void LogicStructure::Pack(double tdiff_, unsigned int location_, bool is_start_){
	logic_tdiff = tdiff_;
	logic_loc = location_;
	logic_start = is_start_;
}*/

///////////////////////////////////////////////////////////
// LogicProcessor:Runtime
///////////////////////////////////////////////////////////
RuntimeStructure::RuntimeStructure(){ rtime_mult = 0; }

void RuntimeStructure::Append(double energy_){
	rtime_energy.push_back(energy_);
	rtime_mult++;
}

void RuntimeStructure::Zero(){
	if(rtime_mult == 0){ return ; } // Structure is already empty
	rtime_energy.clear();
	rtime_mult = 0;
}

///////////////////////////////////////////////////////////
// LiquidProcessor
///////////////////////////////////////////////////////////
LiquidStructure::LiquidStructure(){ liquid_mult = 0; }

void LiquidStructure::Append(unsigned int location_, double TOF_, double S_, double L_, double ltqdc_, double stqdc_){
	liquid_loc.push_back(location_);
	liquid_TOF.push_back(TOF_);
	liquid_S.push_back(S_);
	liquid_L.push_back(L_);
	liquid_tqdc.push_back(ltqdc_);
	start_tqdc.push_back(stqdc_);
	liquid_mult++;
}

void LiquidStructure::Zero(){
	if(liquid_mult == 0){ return ; } // Structure is already empty
	liquid_loc.clear();
	liquid_TOF.clear();
	liquid_S.clear();
	liquid_L.clear();
	liquid_tqdc.clear();
	start_tqdc.clear();
	liquid_mult = 0;
}

void LiquidWaveform::Append(std::vector<int> &pulse){
	unsigned int count = 0;
	for(std::vector<int>::iterator iter = pulse.begin(); iter != pulse.end(); iter++){
		if(count >= LIQUID_WAVE_SIZE){ break; }
		liquid_wave[count] = *iter;
		count++;
	}
	liquid_valid = true;
}

void LiquidWaveform::Zero(){
	for(unsigned short i = 0; i < LIQUID_WAVE_SIZE; i++){ liquid_wave[i] = 0; }
	liquid_valid = false;
}

///////////////////////////////////////////////////////////
// VandleProcessor
///////////////////////////////////////////////////////////
VandleStructure::VandleStructure(){ vandle_mult = 0; }

void VandleStructure::Append(unsigned int location_, double tof_, double lqdc_, double rqdc_, double tsLow_, 
			     double tsHigh_, double lMaxVal_, double rMaxVal_, double qdc_, double energy_){
	vandle_loc.push_back(location_);
	vandle_TOF.push_back(tof_);
	vandle_lqdc.push_back(lqdc_);
	vandle_rqdc.push_back(rqdc_);
	vandle_tsLow.push_back(tsLow_);
	vandle_tsHigh.push_back(tsHigh_);
	vandle_lMaxVal.push_back(lMaxVal_);
	vandle_rMaxVal.push_back(rMaxVal_);
	vandle_qdc.push_back(qdc_);
	vandle_energy.push_back(energy_);
	vandle_mult++;
}

void VandleStructure::Zero(){
	if(vandle_mult == 0){ return ; } // Structure is already empty
	vandle_loc.clear();
	vandle_TOF.clear();
	vandle_lqdc.clear();
	vandle_rqdc.clear();
	vandle_tsLow.clear();
	vandle_tsHigh.clear();
	vandle_lMaxVal.clear();
	vandle_rMaxVal.clear();
	vandle_qdc.clear();
	vandle_energy.clear();
	vandle_mult = 0;
}
