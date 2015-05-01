/** \file TraceAnalyzer.cpp
 * \brief defines the Trace class.
 *
 * Implements a quick online trapezoidal filtering mechanism
 * for the identification of double pulses
 *
 * \author S. Liddick 
 * \date 7-2-07
 * <strong>Modified : </strong> SNL - 2-4-08 - Add plotting spectra
 */

#include <iostream>
#include <string>

#include <unistd.h>

#ifdef USE_HHIRF
#include "DammPlotIds.hpp"
#endif

#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

using std::cout;
using std::endl;
using std::string;

/**
 * Set default filter parameters
 */

#ifdef USE_HHIRF
using namespace dammIds::trace;
#endif

void TraceAnalyzer::_initialize(){
    name = "Trace";
    initDone = true;
	use_root = false; 
	use_damm = false; 
    
    // start at -1 so that when incremented on first trace analysis,
    //   row 0 is respectively filled in the trace spectrum of inheritees 
    numTracesAnalyzed = -1; 
	total_time = 0;
	start_time = clock();
}

#ifdef USE_HHIRF
TraceAnalyzer::TraceAnalyzer() : histo(OFFSET, RANGE){
    _initialize();
}

TraceAnalyzer::TraceAnalyzer(std::string name_) : histo(OFFSET, RANGE){
    _initialize();
    name = name_;
}

TraceAnalyzer::TraceAnalyzer(int offset, int range) : histo(offset, range){
    _initialize();
}

TraceAnalyzer::TraceAnalyzer(int offset, int range, std::string name_) : histo(offset, range){
    _initialize();
    name = name_;
}
#else
TraceAnalyzer::TraceAnalyzer(){
    _initialize();
}

TraceAnalyzer::TraceAnalyzer(std::string name_){
    _initialize();
    name = name_;
}

TraceAnalyzer::TraceAnalyzer(int offset, int range){
    _initialize();
}

TraceAnalyzer::TraceAnalyzer(int offset, int range, std::string name_){
    _initialize();
    name = name_;
}
#endif

/** Output time processing traces */
TraceAnalyzer::~TraceAnalyzer() 
{
}

float TraceAnalyzer::Status()
{
	float time_taken = 0.0;
	if(initDone){
		// output the time usage and the number of valid events
		time_taken = ((float)total_time)/CLOCKS_PER_SEC;
		cout << " " << name << "Analyzer: Used " << time_taken << " seconds of CPU time\n";
	}
	return time_taken;
}

/**
 * Initialize the trace analysis class.  Set the row numbers
 * for spectra 850 to zero
 */
bool TraceAnalyzer::Init(void)
{   
	initDone = true;
    return true;
}

bool TraceAnalyzer::CheckInit(){
	if(!initDone){
		std::cout << " " << name << "Analyzer: Warning! Processor has not been initialized\n";
		return false;
	}
	if(!use_root && !use_damm){
		std::cout << " " << name << "Analyzer: Warning! Both output types are marked inactive\n";
		return false;
	}
	return true;
}

/** declare the damm plots */
bool TraceAnalyzer::InitDamm()
{
    return false;
}

/**
 * Function to quickly analyze a trace online.
 */
void TraceAnalyzer::Analyze(Trace &trace, const string &detType, const string &detSubtype)
{
    StartAnalyze(); // begin timing process
    numTracesAnalyzed++;
    EndAnalyze(trace);
    return;
}

/**
 * End the analysis and record the analyzer level in the trace
 */
void TraceAnalyzer::EndAnalyze(Trace &trace)
{
    trace.SetValue("analyzedLevel", level);
    EndAnalyze();
}
