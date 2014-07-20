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

#include "DammPlotIds.hpp"
#include "Trace.hpp"
#include "TraceAnalyzer.hpp"

using std::cout;
using std::endl;
using std::string;

/**
 * Set default filter parameters
 */

using namespace dammIds::trace;

void TraceAnalyzer::_initialize(){
    userTime = 0.0; systemTime = 0.0;
    name = "Trace";
    
    // start at -1 so that when incremented on first trace analysis,
    //   row 0 is respectively filled in the trace spectrum of inheritees 
    numTracesAnalyzed = -1;    
    clocksPerSecond = sysconf(_SC_CLK_TCK);
}

TraceAnalyzer::TraceAnalyzer() : histo(OFFSET, RANGE) 
{
    _initialize();
}

TraceAnalyzer::TraceAnalyzer(std::string name_) : histo(OFFSET, RANGE) 
{
    _initialize();
    name = name_;
}

TraceAnalyzer::TraceAnalyzer(int offset, int range) : histo(offset, range) 
{
    _initialize();
}

TraceAnalyzer::TraceAnalyzer(int offset, int range, std::string name_) : histo(offset, range) 
{
    _initialize();
    name = name_;
}

/** Output time processing traces */
TraceAnalyzer::~TraceAnalyzer() 
{
    cout << " " << name << "Analyzer : User Time = " << userTime << ", System Time = " << systemTime << endl;
}

/**
 * Initialize the trace analysis class.  Set the row numbers
 * for spectra 850 to zero
 */
bool TraceAnalyzer::Init(void)
{   
    return true;
}

/** declare the damm plots */
bool TraceAnalyzer::InitDamm()
{
    return false;
}

bool TraceAnalyzer::CheckInit()
{
    return false;
}

/**
 * Function to quickly analyze a trace online.
 */
void TraceAnalyzer::Analyze(Trace &trace, const string &detType, const string &detSubtype)
{
    times(&tmsBegin); // begin timing process
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

/**
 * Finish analysis updating the analyzer timing information
 */
void TraceAnalyzer::EndAnalyze(void)
{    
    tms tmsEnd;
    times(&tmsEnd);

    userTime += (tmsEnd.tms_utime - tmsBegin.tms_utime) / clocksPerSecond;
    systemTime += (tmsEnd.tms_stime - tmsBegin.tms_stime) / clocksPerSecond;

    // reset the beginning time so multiple calls of EndAnalyze from
    // derived classes work properly
    times(&tmsBegin);
}
