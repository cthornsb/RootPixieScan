/** \file VandleROOT.hpp
 * \brief Root functionality for Vandle Processor
 */

#ifndef __VANDLEROOT_HPP_
#define __VANDLEROOT_HPP_

class TTree;

#include "VandleProcessor.hpp"

class VandleROOT : public VandleProcessor
{
 public: 
    VandleROOT(TFile*); //--- new constructor
    bool AddBranch(TTree *tree);
    bool Process(RawEvent &event);
    void FillBranch();
    void vmlMapCpy(const VMLMap vmlMap1);
    const VMLMap vmlMap2;

 private:
    bool isSmall;
    bool isBig;

}; // class VandleROOT
#endif // __VANDLEROOT_HPP_
