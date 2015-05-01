/** \file SsdProcessor.hpp
 * \brief Header file for SSD analysis
 */

#ifndef __SSD_PROCESSOR_HPp_
#define __SSD_PROCESSOR_HPP_

#include "EventProcessor.hpp"

class RawEvent;

/**
 * \brief Handles detectors of type ssd
 */
class SsdProcessor : public EventProcessor 
{
  public:
    SsdProcessor(); // no virtual c'tors
#ifdef USE_HHIRF
    virtual void DeclarePlots(void);
#endif
    virtual bool Process(RawEvent &event);
};

#endif // __SSD_PROCESSOR_HPP_
