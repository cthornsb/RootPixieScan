#include "Structures.h"

///////////////////////////////////////////////////////////
// BetaProcessor
///////////////////////////////////////////////////////////
BetaStructure::BetaStructure(){ Zero(); }

void BetaStructure::Append(double energy_){
	beta_energy.push_back(energy_);
	beta_mult++;
	if(!beta_valid){ beta_valid = true; }
}

void BetaStructure::Zero(){
	beta_energy.clear();
	beta_mult = 0;
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
RuntimeStructure::RuntimeStructure(){ Zero(); }

void RuntimeStructure::Append(double energy_){
	rtime_energy.push_back(energy_);
	rtime_mult++;
	if(!rtime_valid){ rtime_valid = true; }
}

void RuntimeStructure::Zero(){
	rtime_energy.clear();
	rtime_mult = 0;
	rtime_valid = false;
}

///////////////////////////////////////////////////////////
// LiquidProcessor
///////////////////////////////////////////////////////////
/*LiquidData::LiquidData(unsigned int location_, double TOF_, double S_, double L_, double ltqdc_, double stqdc_){
	// Integers
	liquid_loc = location_;
	
	// Doubles
	liquid_TOF = TOF_;
	liquid_S = S_;
	liquid_L = L_;
	liquid_tqdc = ltqdc_;
	start_tqdc = stqdc_;
}

void LiquidData::Zero(){
	liquid_TOF = 0.0; liquid_S = 0.0; 
	liquid_L = 0.0; liquid_tqdc = 0.0; 
	start_tqdc = 0.0; liquid_loc = 0;
}*/

LiquidStructure::LiquidStructure(){ Zero(); }

void LiquidStructure::Append(unsigned int location_, double TOF_, double S_, double L_, double ltqdc_, double stqdc_){
	liquid_loc.push_back(location_);
	liquid_TOF.push_back(TOF_);
	liquid_S.push_back(S_);
	liquid_L.push_back(L_);
	liquid_tqdc.push_back(ltqdc_);
	start_tqdc.push_back(stqdc_);
	liquid_mult++;
	liquid_valid = true;
}

/*void LiquidStructure::Append(LiquidData* data_){
	liquid_data.push_back(data_);
	liquid_mult++;
	if(!liquid_valid){ liquid_valid = true; }
}*/

void LiquidStructure::Zero(){
	liquid_loc.clear();
	liquid_TOF.clear();
	liquid_S.clear();
	liquid_L.clear();
	liquid_tqdc.clear();
	start_tqdc.clear();
	liquid_mult = 0;
	liquid_valid = false;
}

/*void LiquidStructure::Zero(){
	for(std::vector<LiquidData*>::iterator iter = liquid_data.begin(); iter != liquid_data.end(); iter++){
		delete (*iter);
	}
	liquid_data.clear();
	liquid_mult = 0;
	liquid_valid = false;
}*/

void LiquidWaveform::Append(std::vector<int> &pulse){
	unsigned int count = 0;
	for(std::vector<int>::iterator iter = pulse.begin(); iter != pulse.end(); iter++){
		if(count >= 124){ break; }
		wave[count] = *iter;
		count++;
	}
}

void LiquidWaveform::Zero(){
	for(unsigned short i = 0; i < 124; i++){ wave[i] = 0; }
}

///////////////////////////////////////////////////////////
// VandleProcessor
///////////////////////////////////////////////////////////
/*VandleData::VandleData(unsigned int location_, double tof_, double lqdc_, double rqdc_, double tsLow_, 
		       double tsHigh_, double lMaxVal_, double rMaxVal_, double qdc_, double energy_){
	// Integers
	location = location_;
	
	// Doubles
	tof = tof_; 
	lqdc = lqdc_;
	rqdc = rqdc_;
	tsLow = tsLow_;
	tsHigh = tsHigh_;
        lMaxVal = lMaxVal_;
        rMaxVal = rMaxVal_;
        qdc = qdc_;
        energy = energy_;
}

void VandleData::Zero(){
	tof = 0.0; lqdc = 0.0; rqdc = 0.0; tsLow = 0.0; tsHigh = 0.0;
	lMaxVal = 0.0; rMaxVal = 0.0; qdc = 0.0; energy = 0.0; location = 0;
}*/

VandleStructure::VandleStructure(){ Zero(); }

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
	vandle_valid = true;
}

/*void VandleStructure::Append(VandleData* data_){
	data.push_back(data_);
	multiplicity++;
	if(!valid){ valid = true; }
}*/

void VandleStructure::Zero(){
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
	vandle_valid = false;
}

/*void VandleStructure::Zero(){
	for(std::vector<VandleData*>::iterator iter = data.begin(); iter != data.end(); iter++){
		delete (*iter);
	}
	data.clear();
	multiplicity = 0;
	valid = false;
}*/
