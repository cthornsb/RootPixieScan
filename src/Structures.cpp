#include "Structures.h"

///////////////////////////////////////////////////////////
// RawEventStructure
///////////////////////////////////////////////////////////
RawEventStructure::RawEventStructure(){
	raw_mult = 0;
}

RawEventStructure::RawEventStructure(const RawEventStructure &other){
	raw_mult = other.raw_mult;
	raw_mod = other.raw_mod;
	raw_chan = other.raw_chan;
	raw_energy = other.raw_energy;
	raw_time = other.raw_time;
}

RawEventStructure::~RawEventStructure(){
}

void RawEventStructure::Append(const int &mod_, const int &chan_, const double &time_, const double &energy_){
	raw_mod.push_back(mod_);
	raw_chan.push_back(chan_);
	raw_energy.push_back(energy_);
	raw_time.push_back(time_);
	raw_mult++;
}

void RawEventStructure::Zero(){
	if(raw_mult == 0){ return; } // Structure is already empty
	raw_mod.clear();
	raw_chan.clear();
	raw_energy.clear();
	raw_time.clear();
	raw_mult = 0;
}

void RawEventStructure::Set(RawEventStructure *other){
	raw_mult = other->raw_mult;
	raw_mod = other->raw_mod;
	raw_chan = other->raw_chan;
	raw_energy = other->raw_energy;
	raw_time = other->raw_time;
}

///////////////////////////////////////////////////////////
// TriggerProcessor
///////////////////////////////////////////////////////////
TriggerStructure::TriggerStructure(){ trigger_mult = 0; }

TriggerStructure::TriggerStructure(const TriggerStructure &other){
	trigger_time = other.trigger_time;
	trigger_energy = other.trigger_energy;
	trigger_mult = other.trigger_mult;
}

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

void TriggerStructure::Set(TriggerStructure *other){
	trigger_time = other->trigger_time;
	trigger_energy = other->trigger_energy;
	trigger_mult = other->trigger_mult;
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

LiquidStructure::LiquidStructure(const LiquidStructure &other){
	liquid_TOF = other.liquid_TOF;
	liquid_tqdc = other.liquid_tqdc;
	start_tqdc = other.start_tqdc;
	liquid_loc = other.liquid_loc;
	liquid_mult = other.liquid_mult;
}

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

void LiquidStructure::Set(LiquidStructure *other){
	liquid_TOF = other->liquid_TOF;
	liquid_tqdc = other->liquid_tqdc;
	start_tqdc = other->start_tqdc;
	liquid_loc = other->liquid_loc;
	liquid_mult = other->liquid_mult;
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

VandleStructure::VandleStructure(const VandleStructure &other){
	vandle_TOF = other.vandle_TOF;
	vandle_lqdc = other.vandle_lqdc;
	vandle_rqdc = other.vandle_rqdc;
	vandle_tsLow = other.vandle_tsLow;
	vandle_tsHigh = other.vandle_tsHigh;
	vandle_lMaxVal = other.vandle_lMaxVal;
	vandle_rMaxVal = other.vandle_rMaxVal;
	vandle_qdc = other.vandle_qdc;
	vandle_energy = other.vandle_energy;
	vandle_recoilEnergy = other.vandle_recoilEnergy;
	vandle_recoilAngle = other.vandle_recoilAngle;
	vandle_ejectAngle = other.vandle_ejectAngle;
	vandle_exciteEnergy = other.vandle_exciteEnergy;
	vandle_flightPath = other.vandle_flightPath;
	vandle_xflightPath = other.vandle_xflightPath;
	vandle_yflightPath = other.vandle_yflightPath;
	vandle_zflightPath = other.vandle_zflightPath;
	vandle_loc = other.vandle_loc;
	vandle_mult = other.vandle_mult;
}

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

void VandleStructure::Append(const unsigned int &location_, const double &tof_, const double &lqdc_, const double &rqdc_, const double &tsLow_, const double &tsHigh_, const double &qdc_){
	vandle_loc.push_back(location_);
	vandle_TOF.push_back(tof_);
	vandle_lqdc.push_back(lqdc_);
	vandle_rqdc.push_back(rqdc_);
	vandle_tsLow.push_back(tsLow_);
	vandle_tsHigh.push_back(tsHigh_);
	vandle_qdc.push_back(qdc_);
	
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

void VandleStructure::Set(VandleStructure *other){
	vandle_TOF = other->vandle_TOF;
	vandle_lqdc = other->vandle_lqdc;
	vandle_rqdc = other->vandle_rqdc;
	vandle_tsLow = other->vandle_tsLow;
	vandle_tsHigh = other->vandle_tsHigh;
	vandle_lMaxVal = other->vandle_lMaxVal;
	vandle_rMaxVal = other->vandle_rMaxVal;
	vandle_qdc = other->vandle_qdc;
	vandle_energy = other->vandle_energy;
	vandle_recoilEnergy = other->vandle_recoilEnergy;
	vandle_recoilAngle = other->vandle_recoilAngle;
	vandle_ejectAngle = other->vandle_ejectAngle;
	vandle_exciteEnergy = other->vandle_exciteEnergy;
	vandle_flightPath = other->vandle_flightPath;
	vandle_xflightPath = other->vandle_xflightPath;
	vandle_yflightPath = other->vandle_yflightPath;
	vandle_zflightPath = other->vandle_zflightPath;
	vandle_loc = other->vandle_loc;
	vandle_mult = other->vandle_mult;
}

VandleWaveform::VandleWaveform(const VandleWaveform &other){
	left_wave = other.left_wave;
	right_wave = other.right_wave;
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

void VandleWaveform::Set(VandleWaveform *other){
	left_wave = other->left_wave;
	right_wave = other->right_wave;
}

///////////////////////////////////////////////////////////
// IonChamberProcessor
///////////////////////////////////////////////////////////
IonChamberStructure::IonChamberStructure(){ ion_mult = 0; }

IonChamberStructure::IonChamberStructure(const IonChamberStructure &other){
	ion_dE = other.ion_dE;
	ion_E = other.ion_E;
	ion_sum = other.ion_sum;
	ion_mult = other.ion_mult;
}

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

void IonChamberStructure::Set(IonChamberStructure *other){
	ion_dE = other->ion_dE;
	ion_E = other->ion_E;
	ion_sum = other->ion_sum;
	ion_mult = other->ion_mult;
}
