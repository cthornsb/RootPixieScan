/** \file TimingInformation.cpp
 *\brief Structures and methods for high res timing
 *
 *Contains common data structures and methods needed for high 
 *resolution timing analysis
 *
 *\author S. V. Paulauskas 
 *\date 09 May 2011
 */
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#include "RawEvent.hpp"
#include "TimingInformation.hpp"
#include "Trace.hpp"

using namespace std;

map<string, double> TimingInformation::constantsMap;
TimingInformation::TimingCalMap TimingInformation::calibrationMap;

//********** Data (Default)**********
TimingInformation::TimingData::TimingData(void) : trace(emptyTrace)
{
	aveBaseline = numeric_limits<double>::quiet_NaN();
	discrimination = numeric_limits<double>::quiet_NaN();
	highResTime = numeric_limits<double>::quiet_NaN();
	maxpos = numeric_limits<double>::quiet_NaN();
	maxval = numeric_limits<double>::quiet_NaN();
	phase = numeric_limits<double>::quiet_NaN();
	snr = numeric_limits<double>::quiet_NaN();
	stdDevBaseline = numeric_limits<double>::quiet_NaN();
	tqdc = numeric_limits<double>::quiet_NaN();
	walk = numeric_limits<double>::quiet_NaN();
	walkCorTime = numeric_limits<double>::quiet_NaN();
	
	numAboveThresh = -1;
}

//********** Data **********
TimingInformation::TimingData::TimingData(ChanEvent *chan) : trace(chan->GetTrace())
{
	//put all the times as ns
	// 11/18/2012 KM: after removal of Trace member, trace is
	// refered here directly. Should not change anything but allows to 
	// remove global variable emptyTrace
	const Trace& trace = chan->GetTrace();
	aveBaseline = trace.GetValue("baseline");
	discrimination = trace.GetValue("discrim");
	highResTime = chan->GetHighResTime()*1e+9;  
	maxpos = trace.GetValue("maxpos");
	maxval = trace.GetValue("maxval");
	numAboveThresh = trace.GetValue("numAboveThresh");
	phase = trace.GetValue("phase")*(pixie::adcClockInSeconds*1e+9);
	stdDevBaseline = trace.GetValue("sigmaBaseline");
	tqdc = trace.GetValue("tqdc")/qdcCompression;
	walk = trace.GetValue("walk");
		
	//Calculate some useful quantities.
	//snr = pow(maxval/stdDevBaseline,2); 
	snr = 20*log10(maxval/stdDevBaseline); 
	walkCorTime   = highResTime - walk;

	//validate data and set a flag saying it's ok
	// clean up condition at some point
	if((maxval == maxval) && (phase == phase) && (tqdc == tqdc) && (highResTime == highResTime) && (stdDevBaseline == stdDevBaseline)){ dataValid = true; }
	else{ dataValid = false; }
}

//********** BarData **********
TimingInformation::BarData::BarData(const TimingData &Right, const TimingData &Left, const TimingCal &cal, const string &type) 
{
	//Clear the maps that hold this information
	timeOfFlight.clear();
	energy.clear();
	corTimeOfFlight.clear();
	
	//Set the values for the useful bar stuff. 
	lMaxVal = Left.maxval;
	rMaxVal = Right.maxval;
	lqdc = Left.tqdc;
	rqdc = Right.tqdc;
	lTime = Left.highResTime;
	rTime = Right.highResTime;
	qdc = sqrt(Right.tqdc*Left.tqdc);
	qdcPos = (lqdc - rqdc) / qdc;
	timeAve = (Right.highResTime + Left.highResTime)*0.5; //in ns
	timeDiff = (Left.highResTime-Right.highResTime) + cal.lrtOffset; //in ns
	walkCorTimeDiff = (Left.walkCorTime-Right.walkCorTime) + cal.lrtOffset;
	walkCorTimeAve = (Left.walkCorTime+Right.walkCorTime)*0.5; //in ns

	//Calculate some useful quantities for the bar analysis.
	event = BarEventCheck(timeDiff, type);
	flightPath = CalcFlightPath(timeDiff, cal, type); //in cm
	theta = acos(cal.z0/flightPath);
}

//********** VMLData **********
TimingInformation::vmlData::vmlData(const BarData bar, double corTOF, double enrgy, double tLow, double tHigh) 
{
	//Set the values for the useful bar stuff. 
	tof = corTOF;
	lqdc = bar.lqdc;
	rqdc = bar.rqdc;
	tsLow = tLow;
	tsHigh = tHigh;
	lMaxVal = bar.lMaxVal;
	rMaxVal = bar.rMaxVal;
	qdc = bar.qdc;
	energy = enrgy;
}

//********** BarEventCheck **********
bool TimingInformation::BarData::BarEventCheck(const double &timeDiff, const string &type)
{
	if(type == "small") {
		double lengthSmallTime = TimingInformation::GetConstant("lengthSmallTime");
		return(fabs(timeDiff) < lengthSmallTime+20);
	} 
	else if(type == "big") {
		double lengthBigTime = TimingInformation::GetConstant("lengthBigTime");
		return(fabs(timeDiff) < lengthBigTime+20);
	} 
	return(false);
}


//********** CalcFlightPath **********
double TimingInformation::BarData::CalcFlightPath(double &timeDiff, const TimingCal& cal, const string &type)
{
	if(type == "small") {
		double speedOfLightSmall = TimingInformation::GetConstant("speedOfLightSmall");
		return(sqrt(cal.z0*cal.z0 + pow(speedOfLightSmall*0.5*timeDiff+cal.xOffset,2)));
		//return(sqrt(pow(speedOfLightSmall*0.5*timeDiff+cal.xOffset,2)));
	} 
	else if(type == "big") {
		double speedOfLightBig = 
		TimingInformation::GetConstant("speedOfLightBig");
		return(sqrt(cal.z0*cal.z0 + pow(speedOfLightBig*0.5*timeDiff+cal.xOffset,2)));
	}
 	return(numeric_limits<double>::quiet_NaN());
}

//********** CalculateEnergy **********
double TimingInformation::CalcEnergy(const double &corTOF, const double &z0)
{
	double speedOfLight = TimingInformation::GetConstant("speedOfLight");
	double neutronMass = TimingInformation::GetConstant("neutronMass");
	return((0.5*neutronMass*pow((z0/corTOF)/speedOfLight, 2))*1000);
}

 
//********** GetConstant **********
double TimingInformation::GetConstant(const string &name)
{
	map<string, double>::iterator itTemp = constantsMap.find(name);
	if(itTemp == constantsMap.end()) {
		cout << endl << endl << "Cannot Locate " << name << " in the Timing Constants Map!!";
		cout << endl << "Please check timingConstants.txt" << endl << endl;
		exit(EXIT_FAILURE); 
	} 
	else {
		double value = (*constantsMap.find(name)).second;
		return(value);
	}
	return (numeric_limits<double>::quiet_NaN());
}


//********** GetTimingCalParameter **********
TimingInformation::TimingCal TimingInformation::GetTimingCal(const IdentKey &identity)
{
	map<IdentKey, TimingCal>::iterator itTemp = 
	calibrationMap.find(identity);
	if(itTemp == calibrationMap.end()) {
		cout << endl << endl << "Cannot locate detector named " << identity.second << " at location " << identity.first;
		cout << "in the Timing Calibration!!" << endl << "Please check timingCal.txt" << endl << endl;
		exit(EXIT_FAILURE); 
	} 
	else {
		TimingCal value = (*calibrationMap.find(identity)).second;
		return(value);
	}
}


//********** ReadTimingConstants **********
void TimingInformation::ReadTimingConstants(void)
{
	ifstream readConstants("./setup/timingConstants.txt");
	if (!readConstants) {
		cout << endl << "Cannot open file 'timingConstants.txt'" << "-- This is Fatal! Exiting..." << endl << endl;
		exit(EXIT_FAILURE);
	} 
	else{
		while(readConstants) {
			double value = 0.0;
			string name = "";
			if (isdigit(readConstants.peek())) {
				readConstants >> value  >> name;			
				constantsMap.insert(make_pair(name, value));
			} 
			else{ readConstants.ignore(1000, '\n'); }
		} // end while (!readConstants) loop 
	}
	readConstants.close();

	double lengthSmallTime = (*constantsMap.find("lengthSmallPhysical")).second / (*constantsMap.find("speedOfLightSmall")).second;
	double lengthBigTime = (*constantsMap.find("lengthBigPhysical")).second / (*constantsMap.find("speedOfLightBig")).second;

	constantsMap.insert(make_pair("lengthBigTime", lengthBigTime));
	constantsMap.insert(make_pair("lengthSmallTime", lengthSmallTime));
} //void TimingInformation::ReadTimingConstants


//********** ReadTimingCalibration **********
void TimingInformation::ReadTimingCalibration(void)
{
	TimingCal timingcal;
	ifstream timingCalFile("./setup/timingCal.txt");
	if (!timingCalFile) {
		cout << endl << "Cannot open file 'timingCal.txt'" << "-- This is Fatal! Exiting..." << endl << endl;
		exit(EXIT_FAILURE);
	} 
	else {
		while(timingCalFile) {
			if (isdigit(timingCalFile.peek())) {
			unsigned int location = -1;
			string type = "";
	
			timingCalFile >> location >> type >> timingcal.z0; //position stuff
			timingCalFile >> timingcal.xOffset >> timingcal.zOffset;
			timingCalFile >> timingcal.lrtOffset; //time stuff
			timingCalFile >> timingcal.tofOffset0 >> timingcal.tofOffset1;
		
			//minimum distance to the bar
			timingcal.z0 += timingcal.zOffset; 
			IdentKey calKey(location, type);
			calibrationMap.insert(make_pair(calKey, timingcal));
			} 
			else{ timingCalFile.ignore(1000, '\n'); }
		} // end while (!timingCalFile) loop 
	}
	timingCalFile.close();
}
