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

void TriggerWaveform::Append(std::vector<int> &pulse){ // trigger_wave.size()/trigger_mult will give pulse size
	for(std::vector<int>::iterator iter = pulse.begin(); iter != pulse.end(); iter++){
		trigger_wave.push_back((*iter));
	}
}

void TriggerWaveform::Zero(){
	if(trigger_wave.size() > 0){ trigger_wave.clear(); }
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

void RuntimeWaveform::Append(std::vector<int> &pulse){ // rtime_wave.size()/rtime_mult will give pulse size
	for(std::vector<int>::iterator iter = pulse.begin(); iter != pulse.end(); iter++){
		rtime_wave.push_back((*iter));
	}
}

void RuntimeWaveform::Zero(){
	if(rtime_wave.size() > 0){ rtime_wave.clear(); }
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

void LiquidWaveform::Append(std::vector<int> &pulse){ // liquid_wave.size()/liquid_mult will give pulse size
	for(std::vector<int>::iterator iter = pulse.begin(); iter != pulse.end(); iter++){
		liquid_wave.push_back((*iter));
	}
}

void LiquidWaveform::Zero(){
	if(liquid_wave.size() > 0){ liquid_wave.clear(); }
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

void VandleWaveform::Append(std::vector<int> &l_pulse, std::vector<int> &r_pulse){ // left(right)_wave.size()/vandle_mult will give pulse size
	for(std::vector<int>::iterator iter1 = l_pulse.begin(), iter2 = r_pulse.begin(); iter1 != l_pulse.end() && iter2 != r_pulse.end(); iter1++, iter2++){
		left_wave.push_back((*iter1));
		right_wave.push_back((*iter2));
	}
}

void VandleWaveform::Zero(){
	if(left_wave.size() > 0){ left_wave.clear(); }
	if(right_wave.size() > 0){ right_wave.clear(); }
}
