#include "Structures.h"

///////////////////////////////////////////////////////////
// TriggerProcessor
///////////////////////////////////////////////////////////
TriggerStructure::TriggerStructure(){ trigger_mult = 0; }

void TriggerStructure::Append(const double &time_, const double &energy_){
	trigger_time.push_back(time_);
	trigger_energy.push_back(energy_);
	trigger_mult++;
}

void TriggerStructure::Zero(){
	if(trigger_mult == 0){ return ; } // Structure is already empty
	trigger_time.clear();
	trigger_energy.clear();
	trigger_mult = 0;
}

void TriggerWaveform::Append(const std::vector<int> &pulse){ // trigger_wave.size()/trigger_mult will give pulse size
	for(std::vector<int>::const_iterator iter = pulse.begin(); iter != pulse.end(); iter++){
		trigger_wave.push_back((*iter));
	}
}

void TriggerWaveform::Zero(){
	if(trigger_wave.size() > 0){ trigger_wave.clear(); }
}

///////////////////////////////////////////////////////////
// LiquidProcessor
///////////////////////////////////////////////////////////
LiquidStructure::LiquidStructure(){ liquid_mult = 0; }

void LiquidStructure::Append(const unsigned int &location_, const double &TOF_, const double &ltqdc_, const double &stqdc_){
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

void LiquidWaveform::Append(const std::vector<int> &pulse){ // liquid_wave.size()/liquid_mult will give pulse size
	for(std::vector<int>::const_iterator iter = pulse.begin(); iter != pulse.end(); iter++){
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

void VandleStructure::Append(const unsigned int &location_, const double &tof_, const double &lqdc_, const double &rqdc_, const double &tsLow_, 
			     const double &tsHigh_, const double &lMaxVal_, const double &rMaxVal_, const double &qdc_, const double &energy_, const double &recoilE_,
			     const double &recoilAngle_, const double &ejectAngle_, const double &excitedE_, const double &flightPath_, const double &x_, const double &y_, const double &z_){
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
	
    vandle_recoilEnergy.push_back(recoilE_);
    vandle_recoilAngle.push_back(recoilAngle_);
    vandle_ejectAngle.push_back(ejectAngle_);
    vandle_exciteEnergy.push_back(excitedE_);
    vandle_flightPath.push_back(flightPath_);
    vandle_xflightPath.push_back(x_);
    vandle_yflightPath.push_back(y_);
    vandle_zflightPath.push_back(z_);
	
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
	
    vandle_recoilEnergy.clear();
    vandle_recoilAngle.clear();
    vandle_ejectAngle.clear();
    vandle_exciteEnergy.clear();
    vandle_flightPath.clear();
    vandle_xflightPath.clear();
    vandle_yflightPath.clear();
    vandle_zflightPath.clear();
	
	vandle_mult = 0;
}

void VandleWaveform::Append(const std::vector<int> &l_pulse, const std::vector<int> &r_pulse){ // left(right)_wave.size()/vandle_mult will give pulse size
	for(std::vector<int>::const_iterator iter1 = l_pulse.begin(), iter2 = r_pulse.begin(); iter1 != l_pulse.end() && iter2 != r_pulse.end(); iter1++, iter2++){
		left_wave.push_back((*iter1));
		right_wave.push_back((*iter2));
	}
}

void VandleWaveform::Zero(){
	if(left_wave.size() > 0){ left_wave.clear(); }
	if(right_wave.size() > 0){ right_wave.clear(); }
}

///////////////////////////////////////////////////////////
// IonChamberProcessor
///////////////////////////////////////////////////////////
IonChamberStructure::IonChamberStructure(){ ion_mult = 0; }

void IonChamberStructure::Append(const double &delta_, const double &energy_){
	ion_dE.push_back(delta_);
	ion_E.push_back(energy_);
	ion_sum.push_back(delta_+energy_);
	ion_mult++;
}

void IonChamberStructure::Zero(){
	if(ion_mult == 0){ return ; } // Structure is already empty
	ion_dE.clear();
	ion_E.clear();
	ion_sum.clear();
	ion_mult = 0;
}
