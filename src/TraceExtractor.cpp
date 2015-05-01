/** \file TraceExtractor.cpp
 *  \brief Extract traces for a specific type and subtype
 */

#include <string>

#ifdef USE_HHIRF	
#include "DammPlotIds.hpp"
#endif

#include "Trace.hpp"
#include "TraceExtractor.hpp"

using std::string;

#ifdef USE_HHIRF
using namespace dammIds::trace;
#endif

const int TraceExtractor::traceBins = SC;
const int TraceExtractor::numTraces = 99;

#ifdef USE_HHIRF
TraceExtractor::TraceExtractor(const std::string& aType, const std::string &aSubtype) : TraceAnalyzer(extractor::OFFSET, extractor::RANGE), type(aType), subtype(aSubtype)
#else
TraceExtractor::TraceExtractor(const std::string& aType, const std::string &aSubtype) : TraceAnalyzer(), type(aType), subtype(aSubtype)
#endif
{
    name = "Extractor";
}

TraceExtractor::~TraceExtractor()
{
    // do nothing
}

#ifdef USE_HHIRF
/** Declare the damm plots */
void TraceExtractor::DeclarePlots(void)
{
    using namespace dammIds::trace;
    for (int i=0; i < numTraces; i++)
	DeclareHistogram1D(extractor::D_TRACE + i, traceBins, "traces data");
}
#endif

/** Plot the damm spectra of the first few traces analyzed with (level >= 1) */
void TraceExtractor::Analyze(Trace &trace, const string &aType, const string &aSubtype)
{   
    using namespace dammIds::trace;

    if (type ==  aType && subtype == aSubtype && numTracesAnalyzed < numTraces) {	
	TraceAnalyzer::Analyze(trace, type, subtype);
#ifdef USE_HHIRF	
	trace.OffsetPlot(extractor::D_TRACE + numTracesAnalyzed, trace.DoBaseline(1,20) );
#endif
	EndAnalyze(trace);
    }
}
