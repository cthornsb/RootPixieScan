/** \file ChanEvent.hpp
 * \brief A Class to define what a channel event is
 */
#ifndef __CHANEVENT_HPP
#define __CHANEVENT_HPP

#include <vector>

///Included from PixieSuite2 
#include "PixieEvent.hpp"

#include "DetectorLibrary.hpp"
#include "pixie16app_defs.h"
#include "Globals.hpp"
#include "Trace.hpp"

/*! \brief A channel event
 *
 * All data is grouped together into channels.  For each pixie16 channel that
 * fires the energy, time (both trigger time and event time), and trace (if
 * applicable) are obtained.  Additional information includes the channels
 * identifier, calibrated energies, trace analysis information.
 * Note that this currently stores raw values internally through pixie word types
 *   but returns data values through native C types. This is potentially non-portable.
 */
class ChanEvent{
  public:
    /** Default constructor */
    ChanEvent(){ 
    	event = new PixieEvent();
    	trace = new Trace(); 
    }
    
    ChanEvent(PixieEvent *event_){ 
    	event = event_; // We will take ownership of the PixieEvent. No need to copy the variables.
    	trace = new Trace(event->adcTrace); // Copy the trace from the PixieEvent (messy, but needed for the Trace class).
    }
    
    ~ChanEvent(){ 
    	delete trace;
    	delete event;
    }

	/////////////////////////////////////////////////////////////////
	// Gets/Sets for implementation specific channel variables.
	/////////////////////////////////////////////////////////////////

    /** Set the calibrated energy
     * \param [in] a : the calibrated energy */
    void SetCalEnergy(double a) {calEnergy = a;}

    /** Set the Walk corrected time
     * \param [in] a : the walk corrected time */
    void SetCorrectedTime(double a) {correctedTime = a;}

    /** Set the high resolution time (Filter time + phase )
     * \param [in] a : the high resolution time */
    void SetHighResTime(double a) {hires_time =a;}

	double GetCalEnergy() const { return calEnergy; } /**< \return the calibrated energy */

    double GetCorrectedTime() const { return correctedTime; } /**< \return the corrected time */
    
    double GetHighResTime() const { return hires_time; } /**< \return the high-resolution time */

    const Trace& GetTrace() const { return *trace; } /**< \return a reference to the trace */
    
    Trace& GetTrace() { return *trace; } /** \return a reference which can alter the trace */

    //! \return The identifier in the map for the channel event
    const Identifier& GetChanID() const;
    
    /** \return the channel id defined as pixie module # * 16 + channel number */
    int GetID() const;

	/////////////////////////////////////////////////////////////////
	// Gets/Sets for raw channel variables from PixieEvent class.
	/////////////////////////////////////////////////////////////////

    /** Set the raw energy in case we do not want to extract it from the trace
     * ourselves
     * \param [in] a : the energy to set */
    void SetEnergy(double a) {event->energy = a;}
    
    /** Set the time
     * \param [in] a : the time to set */
    void SetTime(double a) {event->time = a;}
    
    bool GetCfdSourceBit() const { return(event->cfdTrigSource); } /**< \return the cfdTrigSource flag */
    
    bool CfdForceTrig() const { return(event->cfdForceTrig); } /**< \return the cfdForceTrig flag */
    
    double GetEnergy() const { return event->energy; } /**< \return the raw energy */
    
    double GetTime() const { return event->time; } /**< \return the raw time */
    
    double GetEventTime() const { return event->eventTime; } /**< \return the event time */
    
    unsigned long GetTrigTime() const { return event->trigTime; } /**< \return the channel trigger time */
    
    unsigned long GetEventTimeLo() const { return event->eventTimeLo; } /**< \return the lower 32 bits of event time */
    
    unsigned long GetEventTimeHi() const { return event->eventTimeHi; } /**< \return the upper 32 bits of event time */
    
    bool IsPileup() const { return event->pileupBit; } //!< \return true if channel is pileup */
    
    bool IsSaturated() const { return event->saturatedBit; } /**< \return whether the trace is saturated */

    /** \return The Onboard QDC value at i
     * \param [in] i : the QDC number to obtain, possible values [0,7] */
    unsigned long GetQdcValue(int i) const;

    /** Channel event zeroing
     *
     * All numerical values are set to -1, and the trace,
     * and traceinfo vectors are cleared and the channel
     * identifier is zeroed using its identifier::zeroid method. */
    void ZeroVar();

	PixieEvent *event;
private:
    double calEnergy; /**< Calibrated channel energy. */
    double correctedTime; /**< Energy-walk corrected time. */
	double hires_time; /**< timing resolution less than 1 adc size */

    Trace *trace; /**< Channel trace if present */

    void ZeroNums(void); /**< Zero members which do not have constructors associated with them */

    /** Make the front end responsible for reading the data able to set the
     * channel data directly from ReadBuffDataA - REVISION A */
    friend int ReadBuffDataA(pixie::word_t *, unsigned long *, std::vector<ChanEvent *> &);
    
    /** Make the front end responsible for reading the data able to set the
     * channel data directly from ReadBuffDataA - REVISION D */
    friend int ReadBuffDataD(pixie::word_t *, unsigned long *, std::vector<ChanEvent *> &);
    
    /** Make the front end responsible for reading the data able to set the
     * channel data directly from ReadBuffDataA - REVISION F */
    friend int ReadBuffDataF(pixie::word_t *, unsigned long *, std::vector<ChanEvent *> &);
};

/** Sort by increasing corrected time
 * \param [in] a : the left hand side for comparison
 * \param [in] b : the right hand side for comparison
 * \return True if LHS is less the RHS */
bool CompareCorrectedTime(const ChanEvent *a, const ChanEvent *b);

/** Sort by increasing raw time
 * \param [in] a : the left hand side for comparison
 * \param [in] b : the right hand side for comparison
 * \return True if LHS is less the RHS*/
bool CompareTime(const ChanEvent *a, const ChanEvent *b);

#endif
