#include "Structures.h"

BetaDataStructure::BetaDataStructure(){ Zero(); }

void BetaDataStructure::Zero(){
	energy = 0.0; multiplicity = 0; 
	valid = false;
}

LogicDataStructure::LogicDataStructure(){ Zero(); }

void LogicDataStructure::Zero(){
	tdiff = 0.0; location = 0; 
	is_start = false; valid = false;
}

RuntimeDataStructure::RuntimeDataStructure(){ Zero(); }

void RuntimeDataStructure::Zero(){
	energy = 0.0; valid = false;
}

LiquidDataStructure::LiquidDataStructure(){ Zero(); }

void LiquidDataStructure::Zero(){
	TOF = 0.0; S = 0.0; L = 0.0;
	liquid_tqdc = 0.0; start_tqdc = 0.0;
	location = 0; valid = false;
}

VandleDataStructure::VandleDataStructure(){ Zero(); }

void VandleDataStructure::Zero(){
	tof = 0.0; lqdc = 0.0; rqdc = 0.0; tsLow = 0.0; tsHigh = 0.0;
	lMaxVal = 0.0; rMaxVal = 0.0; qdc = 0.0; energy = 0.0;
	multiplicity = 0; location = 0; valid = false;
}
