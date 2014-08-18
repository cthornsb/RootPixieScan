/** \file WaveformAnalyzer.cpp 
 *\brief Preliminary waveoform analysis
 *
 *Does preliminary waveform analysis on traces. The parameters set here
 *will be used for the high resolution timing algorithms to do their thing. 
 *
 *\author S. V. Paulauskas 
 *\date 16 July 2009
*/
#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>

#include <cmath>

#include "WaveformAnalyzer.hpp"

using namespace std;
using namespace dammIds::trace::waveform;


//********** WaveformAnalyzer **********
WaveformAnalyzer::WaveformAnalyzer() : TraceAnalyzer(OFFSET, RANGE, "Waveform") 
{
}

//********** DeclarePlots **********
bool WaveformAnalyzer::InitDamm()
{
    std::cout << " WaveformAnalyzer: Initializing the damm output\n";
    if(use_damm){
        std::cout << " WaveformAnalyzer: Warning! Damm output already initialized\n";
        return false;
    }
    
    use_damm = true;
    return true;
}

//********** Analyze **********
void WaveformAnalyzer::Analyze(Trace &trace, const string &detType, const string &detSubtype)
{
	StartAnalyze();
    TraceAnalyzer::Analyze(trace, detType, detSubtype);
    
    if(detType == "vandleSmall" || detType == "vandleBig" || detType == "scint" || detType == "pulser" || detType == "tvandle") {
    	unsigned int maxPos;
    	if(detSubtype == "liquid"){ maxPos = trace.FindMaxInfo("traceDelayLiquid"); }
    	else{ maxPos = trace.FindMaxInfo("traceDelayVandle"); }

	if(trace.HasValue("saturation")) {
	    EndAnalyze();
	    return;
	}

	unsigned int waveformLow = GetConstant("waveformLow");
	unsigned int waveformHigh = GetConstant("waveformHigh");
	unsigned int startDiscrimination = GetConstant("startDiscrimination");
	double qdc = trace.DoQDC(maxPos-waveformLow, waveformHigh+waveformLow);

	trace.InsertValue("qdcToMax", qdc/trace.GetValue("maxval"));

	if(detSubtype == "liquid")
	    trace.DoDiscrimination(startDiscrimination, waveformHigh - startDiscrimination);
    } //if(detType
    EndAnalyze();
}
