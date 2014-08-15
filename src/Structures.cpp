#include "Structures.h"

///////////////////////////////////////////////////////////
// TriggerProcessor
///////////////////////////////////////////////////////////
TriggerStructure::TriggerStructure(){ trigger_mult = 0; }

void TriggerStructure::Append(double energy_){
	trigger_energy.push_back(energy_);
	trigger_mult++;
}

void TriggerStructure::Zero(){
	if(trigger_mult == 0){ return ; } // Structure is already empty
	trigger_energy.clear();
	trigger_mult = 0;
}

void TriggerWaveform::Append(std::vector<int> &pulse){
	for(std::vector<int>::iterator iter = pulse.begin(); iter != pulse.end(); iter++){
		trigger_wave.push_back((*iter));
	}
	trigger_wave_mult++; // trigger_wave.size()/trigger_wave_mult will give pulse size
}

void TriggerWaveform::Zero(){
	if(trigger_wave_mult > 0){
		trigger_wave.clear();
		trigger_wave_mult = 0;
	}
}

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

//void LiquidStructure::Append(unsigned int location_, double TOF_, double S_, double L_, double ltqdc_, double stqdc_){
void LiquidStructure::Append(unsigned int location_, double TOF_, double ltqdc_, double stqdc_){
	liquid_loc.push_back(location_);
	liquid_TOF.push_back(TOF_);
	liquid_tqdc.push_back(ltqdc_);
	start_tqdc.push_back(stqdc_);
	liquid_mult++;
}

void LiquidStructure::Zero(){
	if(liquid_mult == 0){ return ; } // Structure is already empty
	liquid_loc.clear();
	liquid_TOF.clear();
	liquid_tqdc.clear();
	start_tqdc.clear();
	liquid_mult = 0;
}

void LiquidWaveform::Append(std::vector<int> &pulse){
	for(std::vector<int>::iterator iter = pulse.begin(); iter != pulse.end(); iter++){
		liquid_wave.push_back((*iter));
	}
	liquid_wave_mult++; // liquid_wave.size()/liquid_wave_mult will give pulse size
}

void LiquidWaveform::Zero(){
	if(liquid_wave_mult > 0){
		liquid_wave.clear();
		liquid_wave_mult = 0;
	}
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
