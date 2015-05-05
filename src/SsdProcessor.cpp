/*! \file SsdProcessor.cpp
 * \brief The SSD processor handles detectors of type ssd 
 */

#include <iostream>

#include "SsdProcessor.hpp"
#include "RawEvent.hpp"

#ifdef USE_HHIRF
#include "DammPlotIds.hpp"
using namespace dammIds::ssd;
#endif

using std::cout;
using std::endl;

#define NUM_DETECTORS 4
#define DD_POSITION__ENERGY_DETX 1 // for x detectors

#ifdef USE_HHIRF
SsdProcessor::SsdProcessor() : EventProcessor(OFFSET, RANGE)
#else
SsdProcessor::SsdProcessor()
#endif
{
    name = "ssd";
    associatedTypes.insert("ssd");
}

#ifdef USE_HHIRF
void SsdProcessor::DeclarePlots(void)
{
    
    const int energyBins    = SE; 
    const int positionBins  = S5;
    // const int timeBins     = S8;

    for (int i=0; i < NUM_DETECTORS; i++) {
        DeclareHistogram2D(DD_POSITION__ENERGY_DETX + i,
                energyBins, positionBins, "SSD Strip vs E");
    }
}
#endif

bool SsdProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
	return false;

    static bool firstTime = true;
    static const DetectorSummary *ssdSummary[NUM_DETECTORS];

    if (firstTime) {
        ssdSummary[0] = event.GetSummary("ssd:implant");
        ssdSummary[1] = event.GetSummary("ssd:box");
        ssdSummary[2] = event.GetSummary("ssd:digisum");
        ssdSummary[3] = event.GetSummary("ssd:ssd_4");
        firstTime = false;
    }
    
    for (int i = 0; i < NUM_DETECTORS; i++) {
        if (ssdSummary[i]->GetMult() == 0)
            continue;
#ifdef USE_HHIRF
        const ChanEvent *ch = ssdSummary[i]->GetMaxEvent();
        int position = ch->GetChanID().GetLocation();
        double energy   = ch->GetCalEnergy();
        plot(DD_POSITION__ENERGY_DETX + i, energy, position);
#endif
    }

    EndProcess(); // update the processing time
    return true;
}

