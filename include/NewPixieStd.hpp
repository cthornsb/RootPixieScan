#ifndef NEW_PIXIE_STD_H
#define NEW_PIXIE_STD_H

#include <vector>

enum HistoPoints {BUFFER_START, BUFFER_END, EVENT_START = 10, EVENT_CONTINUE};

class ChanEvent;
class RawEvent;

// Function forward declarations
bool ScanList(std::vector<ChanEvent*> &eventList, RawEvent& rawev);

void RemoveList(std::vector<ChanEvent*> &eventList);

void HistoStats(unsigned int, double, double, HistoPoints);

void cleanup();

/**
 * \brief Extract channel information from the raw parameter array ibuf
 */
bool ReadSpill(unsigned int *data, unsigned int nWords, bool is_verbose=true);

bool Pixie16Error(int errornum);

#endif
