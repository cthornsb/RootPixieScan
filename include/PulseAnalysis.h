/*************************************************************************
 *
 *  Filename: PulseAnalysis.h 
 *
 *  Description: 
 *
 *	Author(s):
 *     Michael T. Febbraro
 *     
 *
 *  Creation Date: 11/25/2012
 *  Last modified: 9/17/2013 
 *
 * -----------------------------------------------------
 * 	Nuclear Reaction Group
 *  University of Michigan, Ann Arbor, MI, USA 
 *  (c) All Rights Reserved.
 *
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>

#include "Trace.hpp"

class PulseAnalysis {
    private:

    protected:
	double deltaT;

public:
	PulseAnalysis();
	~PulseAnalysis();
	void GetVersion();	
	bool PSD_Integration (Trace&, unsigned int, unsigned int, char, double&, double&);
	bool Baseline_restore (Trace&, unsigned int, char);
};

/** Constructor  */
PulseAnalysis::PulseAnalysis()
{
	deltaT = 1;
}

/** Destructor  */
PulseAnalysis::~PulseAnalysis()
{
}

/** ----------------------------------------------------  
*	Get version
*		-  Print the version and revision date
*          
*  Notes:  
*		Keep this information up to date.
*	----------------------------------------------------
*/
void PulseAnalysis::GetVersion()
{	
	cout << "Package: " << "Pulse analysis 1.0" << endl;
	cout << "Revision: " << "9/17/2013" << endl;
}

/** ----------------------------------------------------  
*	Determine PSD by charge integration method
*		- Calculates discrimination parameters by 
*		  integration of user defined regions of an
*		  input pulse.
*
*	Methods:
*			1 - trapezoidal rule
*			2 - composite Simpson's rule
*			3 - rectangular method 
*
*	Inputs:
*			pulse - input array
*			length - the length of pulse
*			start - start index of long integral
*			stop - stop index of both integrals
*			offset - start index of short integral
*			method - see above...
*	----------------------------------------------------
*/
bool PulseAnalysis::PSD_Integration (Trace &pulse, unsigned int left, unsigned int right, char method, double &paraL, double &paraS){
	// Disclaimer!!! I don't understand this code at all.  I'm trying
	// to match it to Mike's version while fixing some errors which
	// could cause unstable behavior.  I believe these errors are what
	// kept causing the crashes during the experimental run.  CRT
	if (pulse.size() == 0) {return false;}
	double integralS = 0.0, integralL = 0.0;
	
	// Find the pulse maximum...
	// In this version,
	// maximum == j in Mike's code
	// max_bin == l in Mike's code
	// cfd_bin == k in Mike's code
	int maximum = 0;
	unsigned int max_bin, cfd_bin;
	for(unsigned int i = 0; i < pulse.size(); i++){
		if(pulse[i] > maximum) { maximum = pulse[i]; max_bin = i; }
	}

	int temp;
	unsigned int start, stop;

	// Constant fraction discrimination (CFD) at 50% of amplitude
	temp = max_bin - left;
	if(temp > 0){ start = temp; }
	else{ start = 0; }
	for(unsigned int i = start; i < max_bin; i++){
		if(pulse[i] < (int)maximum*0.5){ cfd_bin = i; }
	}
	
	// Some out of range protection
	temp = cfd_bin - left;
	if(temp < 0){ start = temp; }
	else{ start = 0; }
	
	if(cfd_bin + right < pulse.size()){ stop = cfd_bin + right; }
	else{ stop = pulse.size(); }

	// Integrate the pulse
	if((cfd_bin - start) > 0 && (cfd_bin + start) < pulse.size()) {
		if (method == 1) { // Begin integration using trapezoidal rule (using arb. bin width of 1)
			for (unsigned int i = start; i < stop-1; i++) {
				integralL += 0.5*(pulse[i] + pulse[i+1]);
				if (i > cfd_bin) { integralS += 0.5*(pulse[i] + pulse[i+1]);}
			}
		}
		else if (method == 2) { // Begin integration using composite Simpson's rule
			double sumL = 0;
			double sumS = 0;
			integralL = pulse[cfd_bin - start];
			integralL = pulse[cfd_bin];
		
			for (unsigned int i = start; i < stop; i += 2) {
				sumL += pulse[i];
				if (i > cfd_bin) { sumS += pulse[i];}
			}
		
			integralL += 4*sumL;
			integralS += 4*sumS;
		
			sumL = 0;
			sumS = 0;
			for (unsigned int i = (cfd_bin - start + 2); i < (cfd_bin + stop) - 3; i+=2) {
				sumL += pulse[i];
				if (i > cfd_bin) { sumS += pulse[i];}
			}
		
			integralL += 2*sumL;
			integralS += 2*sumS;
		
			integralL += pulse[cfd_bin + stop];
			integralS += pulse[cfd_bin + stop];
		
			integralL = integralL/3;
			integralS = integralS/3;
		}
		else if (method == 3) { // Begin integration using rectangular method 
			// cfd_bin is the 50% cfd value
			// Iterate from 0.5CFD-start to 0.5CFD+stop
			for (unsigned int i = start; i < stop; i++) {
				integralL += pulse[i];				
				if (i > cfd_bin) { integralS += pulse[i];}
			}
		}
		else if(method == 4){ // CRT, for debugging
			if(start >= pulse.size()){ return false; }
			for(unsigned int index = start; index < stop; index++){
				// Using an arbitrary bin width of 1
				integralL += (pulse[index-1] + pulse[index])/2.0;
				if(index > cfd_bin){ integralS += (pulse[index-1] + pulse[index])/2.0; }
			}
		}
		else{ return false; }				
	} 
	else { return false; }

	if(integralS != 0.0 && integralL != 0.0){ 
		double ratio = integralS/integralL;
		if(ratio <= 1.0 && ratio >= 0.0){
			paraL = integralL;
			paraS = integralS;
		}
		else{
			paraL = -1.0;
			paraS = -1.0;
			return false;
		}
	}
	else{ 
		paraL = -1.0;
		paraS = -1.0;
		return false; 
	}
	return true;
}

/** ----------------------------------------------------  
*	Restores the baseline of a input pulse
*
*	Methods:
*			1 - SNIP fitting routine
*			2 - baseline averaging
*			 
*	Inputs:
*			pulse - input array
*			length - the length of pulse
*			iterations - number of iterations
*			method - see above...
*
*	----------------------------------------------------
*/
bool PulseAnalysis::Baseline_restore (Trace &pulse, unsigned int iterations, char method){
	double x, y;
	if (pulse.size() == 0) {return false;}
	if (method == 1){
		// Michael Febbraro 3/2/2013
		cout << "Method Disabled!" << endl; return false;
			
		/*for (i=0; i< pulse.size(); i++) {
			baseline[i] = pulse[i] + 0.3;
		}
		
		//TSpectrum::Background(baseline, pulse.size(), iterations,TSpectrum::kBackDecreasingWindow,TSpectrum::kBackOrder2,
		//		TSpectrum::kTRUE, TSpectrum::kBackSmoothing3,TSpectrum::kFALSE);

		for (i=0; i< pulse.size(); i++) {
			pulse[i] = pulse[i] - baseline[i];
		}*/
	}
	else if (method == 2){
		x = 0.0;
		for (unsigned int i = 0; i < iterations; i++) { x += pulse[i]; }
		x = x/iterations;
		
		for (unsigned int i = 0; i < pulse.size(); i++) {
			pulse[i] -= x;
		}
	}
	else if (method == 3){
		int maximum = 0;
		unsigned int max_bin;
		for(unsigned int i = 0; i < pulse.size(); i++){
			if(pulse[i] > maximum) { maximum = pulse[i]; max_bin = i; }
		}
	
		x = 0.0; y = 0.0;
		// This could possibly crash the program if iterations is greater than pulse.size()
		// I'm not sure how to fix it because I'm not sure what it does
		for (unsigned int i = 0; i < iterations; i++){ x += pulse[i]; y += pulse[pulse.size() - i - 1]; }
		x = x/iterations; y = y/iterations;
		
		for (unsigned int i = 0; i < pulse.size(); i++) {
			if(i >= max_bin) { pulse[i] -= y; }
			else { pulse[i] -= x; }
		}
	}
	else if(method == 4){ // CRT, for debugging
		// Use "iterations" as the number of bins to average for the baseline
		double baseline_L = 0.0;
		double baseline_R = 0.0;
		int baseline;
		for(unsigned int index = 0; index < iterations; index++){ baseline_L += (double)pulse[index]; }
		for(unsigned int index = pulse.size()-iterations; index < pulse.size(); index++){ baseline_R += (double)pulse[index]; }
		baseline_L = baseline_L/iterations;
		baseline_R = baseline_R/iterations;
		baseline = (int)(baseline_L + baseline_R)/2.0;
		for(unsigned int index = 0; index < pulse.size(); index++){ pulse[index] -= baseline; }
	}
	
	return true;
}
