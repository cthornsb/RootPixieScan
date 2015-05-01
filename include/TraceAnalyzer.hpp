/** \file TraceAnalyzer.hpp
 * \brief Header file for the TraceAnalyzer class
 * \author S. Liddick 
 * \date 02 July 2007
 */

#ifndef __TRACEANALYZER_HPP_
#define __TRACEANALYZER_HPP_

#include <string>
#include <time.h>

#ifdef USE_HHIRF	
#include "Plots.hpp"
#endif

class Trace;

/** \brief quick online trace analysis
 *
 *  Simple class which is the basis for all types of trace analysis 
 */

class TraceAnalyzer {
 private:
    // things associated with timing
	long total_time;
	clock_t start_time;
    
    void _initialize();

 protected:
    int level;                ///< the level of analysis to proceed with
    int numTracesAnalyzed;    ///< rownumber for DAMM spectrum 850
    std::string name;         ///< name of the analyzer
    bool use_root, use_damm;
    bool initDone;

#ifdef USE_HHIRF
    Plots histo;
    virtual void plot(int dammId, double val1, double val2 = -1, double val3 = -1, const char* name="h") {
        histo.Plot(dammId, val1, val2, val3, name);
    }
    virtual void DeclareHistogram1D(int dammId, int xSize, const char* title) {
        histo.DeclareHistogram1D(dammId, xSize, title);
    }
    virtual void DeclareHistogram2D(int dammId, int xSize, int ySize, const char* title) {
        histo.DeclareHistogram2D(dammId, xSize, ySize, title);
    }
#endif

 public:
    TraceAnalyzer();
    TraceAnalyzer(std::string);
    TraceAnalyzer(int, int);
    TraceAnalyzer(int, int, std::string);
    virtual ~TraceAnalyzer();
    
	virtual float Status();
    virtual bool Init(void);
    virtual bool CheckInit();
    virtual bool InitDamm();
    virtual void Analyze(Trace &trace, const std::string &type, const std::string &subtype);
    
    // Start the analysis timer
    void StartAnalyze(){ start_time = clock(); }
    
	// Finish analysis updating the analyzer timing information
    void EndAnalyze(Trace &trace);
    void EndAnalyze(void){ total_time += (clock() - start_time); }

    void SetLevel(int i) {level=i;}
    int GetLevel() {return level;}
    std::string GetName(){ return name; }
};

#endif // __TRACEANALYZER_HPP_
