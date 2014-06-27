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
	string version; 
	string revision;

	double deltaT, tempV;
	double PSD, integralS, integralL, sumL, sumS;	
	double linear[4], pulse_temp[2000], summing[2000], MovingAverage[2000];	
	double w,x,y,z;
	unsigned int i,j,k,l,m,n;	
	int return_code;
	bool positive, negative;

public:
	PulseAnalysis();
	~PulseAnalysis();
	void GetVersion();	
	int PSD_Integration (Trace&, unsigned int, unsigned int, int, int, double&, double&);
	int Baseline_restore (Trace&, unsigned int, unsigned int);
	int PSD_Zerocross (float*, unsigned int, unsigned int, int, float*);	
	int Parameters (float*, unsigned int, int, float*, float*, float*, float*, float*);	
	int Time_Pickoff (float*, unsigned int, int, unsigned int, unsigned int, int, float*);
	int Derivative(float*, unsigned int, unsigned int);
	int Integral(float*, unsigned int);
	int PeakFinder (float*, unsigned int, int, unsigned int, int, int*, int*);
	int OptimizePSD (float*, unsigned int, int, int, unsigned int, unsigned int, float*, float*);
	int Half_Integral(float*, unsigned int, float*);
	int Smooth(float*, unsigned int, unsigned int, int, float);
	int HPGe(float*, unsigned int, float*);
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
*	Optimize PSD by charge integration method
*		- Calculates discrimination parameters by 
*		  integration of user defined regions of an
*		  input pulse over a user defined range for evaluation.
*
*	Inputs:
*			pulse - input array
*			length - the length of pulse
*			start - start index of long integral
*			stop - stop index of both integrals
*			lower - lower index range of short integral
*			upper - upper index range of short integral
*			method - see above...
*	----------------------------------------------------
*/
int PulseAnalysis::OptimizePSD (float* pulse, unsigned int length, int start, int stop, unsigned int lower, unsigned int upper, float* paraL, float* paraS)
{
	if (length < sizeof(pulse)/sizeof(float)) {return -1;}
	if ((upper - lower) < 0) {return -1;}
	if ((upper - lower) < sizeof(paraS)/sizeof(float)) {return -1;}

	j = 0;
	for (i = 0; i < length; i++)
	{
		if(pulse[i] > j) { j = (int)pulse[i]; l= i;}
	}
	
	// Constant fraction discrimination (CFD) at 50% of amplitude
	j = l;
	for(i = j - 50; i < j; i++) { if(pulse[i] < (pulse[l])*0.5) {k = i;} }
	x = ((0.5*(pulse[l]) - pulse[k - 1])/((pulse[k + 1] - pulse[k - 1])/3))+ ((float)k - 1);
				
	if((k - start) > 0 && (k + start) < length) {
		// Initialization
		integralL = 0;
		for ( j = 0; j < upper - lower; j++){
			 paraS[j] = 0;
		}
		
		// Begin integration using trapezoidal rule 
		for (i = (k - start); i < (k + stop); i++) {
			integralL += 0.5*(pulse[i-1] + pulse[i]);
			if (i >= (k + lower)) { 
				for ( j = 0; j < upper - lower; j++)
				{
					if (i >= (k + lower + j)) {
						 paraS[j] += 0.5*(pulse[i-1] + pulse[i]);
					}
				}
			}

		}
	}
	
	if (integralL != 0) {
		*paraL = integralL;
		*paraS = integralS;
		return 0;
	} 
	else 
	{ return -1;}

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
*			4 - Harwell method
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
int PulseAnalysis::PSD_Integration (Trace &pulse, unsigned int start, unsigned int stop, int offset, int method, double &paraL, double &paraS)
{
	if (pulse.size() == 0) {return -1;}
	integralS = 0.0; integralL = 0.0;
	
	// Find the pulse maximum
	int maximum = 0;
	unsigned int max_bin, cfd_bin;
	for(i = 0; i < pulse.size(); i++){
		if(pulse[i] > maximum) { maximum = pulse[i]; max_bin = i; }
	}
	
	// Constant fraction discrimination (CFD) at 80% of amplitude
	for(i = max_bin; i < stop; i++){
		if(pulse[i] <= (int)maximum*0.8){ break; }
		cfd_bin = i;
	}
	
	if(method == 4){ // CRT	
		if(start >= pulse.size()){ return -1; }
		for(unsigned int index = start; index < stop; index++){
			// Using an arbitrary bin width of 1
			integralL += (pulse[index-1] + pulse[index])/2.0;
			if(index > cfd_bin){ integralS += (pulse[index-1] + pulse[index])/2.0; }
		}
	}	
	else{
		x = ((0.5*j - pulse[i - 1])/((pulse[i + 1] - pulse[i - 1])/3)) + (i - 1);
	
		if((k - start) > 0 && (k + start) < pulse.size()) {
			// Begin integration using trapezoidal rule 
			if (method == 1) {
				for (i = (k - start); i < (k + stop); i++) {
					integralL += 0.5*(pulse[i-1] + pulse[i]);
					if (i > (k + offset)) { integralS += 0.5*(pulse[i-1] + pulse[i]);}
				}
			}
			else if (method == 2) { // Begin integration using composite Simpson's rule
				sumL = 0;
				sumS = 0;
				integralL = pulse[k - start];
				integralL = pulse[k + offset];
			
				for (i = (k - start) + 1; i < (k + stop) - 2; i+=2) {
		
					sumL += pulse[i];
				
					if (i > (k + offset)) { sumS += pulse[i];}
				}
			
				integralL += 4*sumL;
				integralS += 4*sumS;
			
				sumL = 0;
				sumS = 0;
				for (i = (k - start + 2); i < (k + stop) - 3; i+=2) {
		
					sumL += pulse[i];
				
					if (i > (k + offset)) { sumS += pulse[i];}
				}
			
				integralL += 2*sumL;
				integralS += 2*sumS;
			
				integralL += pulse[k + stop];
				integralS += pulse[k + stop];
			
				integralL = integralL/3;
				integralS = integralS/3;
			}
			else if (method == 3) { // Begin integration using rectangular method 
				for (i = (k - start); i < (k + stop); i++) {
					integralL += pulse[i];				
					if (i > (k + offset)) { integralS += pulse[i];}
					//if (i > (k - start) && i <= (k + offset)) { integralS += pulse[i];}
				}
			
				// Adjust for error due to array indexing (i.e. rounding)
				if (x < k + offset) {
					//integralS =+ ((pulse[k + offset] - pulse[k + offset-1])*x*0.5) + ((x - (float)(k + offset))*pulse[k + offset-1]); 
				}
			}			
		} 
		else { return -1; }
	}
	
	/*if(paraL > 10000 || paraS > 10000){ 
		std::cout << paraL << "\t" << paraS << "\t" << std::endl;
		for(unsigned int index = 0; index < pulse.size(); index++){ std::cout << index << " " << pulse[index] << std::endl; } 
	}*/

	if(integralS != 0.0 && integralL != 0.0){ 
		double ratio = integralS/integralL;
		if(ratio <= 1.0 && ratio >= 0.0){
			paraL = integralL;
			paraS = integralS;
		}
		else{
			paraL = -1.0;
			paraS = -1.0;
			return -1;
		}
	}
	else{ 
		paraL = -1.0;
		paraS = -1.0;
		return -1; 
	}
	return 0;
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
int PulseAnalysis::Baseline_restore (Trace &pulse, unsigned int iterations, unsigned int method)
{
	if (pulse.size() == 0) {return -1;}
	if (method == 1)
	{
		// Michael Febbraro 3/2/2013
		cout << "Method Disabled!" << endl; return -1;
			
		/*for (i=0; i< pulse.size(); i++) {
			baseline[i] = pulse[i] + 0.3;
		}
		
		//TSpectrum::Background(baseline, pulse.size(), iterations,TSpectrum::kBackDecreasingWindow,TSpectrum::kBackOrder2,
		//		TSpectrum::kTRUE, TSpectrum::kBackSmoothing3,TSpectrum::kFALSE);

		for (i=0; i< pulse.size(); i++) {
			pulse[i] = pulse[i] - baseline[i];
		}*/
	}
	else if (method == 2)
	{
		x = 0.0;
		for (i = 0; i < iterations; i++) { x += pulse[i]; }
		x = x/iterations;
		
		for (i = 0; i < pulse.size(); i++) {
			pulse[i] -= x;
		}
	}
	else if (method == 3)
	{
		/*j = 0;
		for (i = 0; i < pulse.size(); i++)
		{
			if(pulse[i] > j) { j = (int)pulse[i]; k = i;}
		}*/
			
		j = 0;
		for(i = 0; i < pulse.size(); i++){
			if(pulse[i] > j) { j = pulse[i]; k = i; }
		}
	
		x = 0.0; y = 0.0;
		for (i = 0; i < iterations; i++){ x += pulse[i]; y += pulse[pulse.size() - i]; }
		x = x/iterations; y = y/iterations;
		
		for (i = 0; i < pulse.size(); i++) {
			if(i >= k) { pulse[i] -= y; }
			else { pulse[i] -= x; }
		}
	}
	else if(method == 4){
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
	
	return 0;
}

/** ----------------------------------------------------  
*	Determine time characteristics of the a pulse
*		- Fits timing regions using linear regression
*		  over a user defined range.
*
*	Inputs:
*			pulse - input array
*			length - the length of pulse
*			range - range of linear regression for timing
*	----------------------------------------------------
*/
int PulseAnalysis::Parameters (float* pulse, unsigned int length, int range, float* CFD, float* amplitude, float* risetime, float* falltime, float* width)
{
	if (length < sizeof(pulse)/sizeof(float)) {return -1;}
	
	j = 0;
	for (i = 0; i < length; i++)
	{
		if(pulse[i] > j) { j = (int)pulse[i]; k = i;}
	}
	
	*amplitude = pulse[k];
	
	// Constant fraction discrimination (CFD) at 50% of amplitude
	memset(linear, 0, sizeof(linear));
	
	j = k;
	for(i = j - 50; i < j; i++)
	{
		if(pulse[i] < (*amplitude)*0.5) {k = i;}
	}
	
	/*
	for(i = (point_t - (int)(range/2)); i <= (point_t - (int)(range/2) + range); i++) {
		linear[0] += i*pulse[i];
		linear[1] += i;
		linear[2] += pulse[i];
		linear[3] += i*i;
	}
	linear[0] = linear[0]/range;
	linear[1] = linear[1]/range;
	linear[2] = linear[2]/range;
	linear[3] = linear[3]/range;
	
	*CFD = ((0.5*(*amplitude) - pulse[(point_t - (int)(range/2))])/((linear[0] 
		- linear[1]*linear[2])/(linear[3] - linear[1]*linear[1])))+ (point_t - (int)(range/2));
		
	*/
	
	*CFD = ((0.5*(*amplitude) - pulse[(k - (int)(range/2))])/((pulse[k + (int)(range/2)] 
			- pulse[k - (int)(range/2)])/(float)range))+ ((float)k - ((float)range/2));
	
	// Risetime by 10-90% Method
	j = k;
	for(i = j - 50; i < j; i++)
	{
		if(pulse[i] < (*amplitude)*0.9) {k = i;}
	}
	
	memset(linear, 0, sizeof(linear));
	for(i = (k - (int)(range/2)); i <= (k - (int)(range/2) + range); i++) {
		linear[0] += i*pulse[i];
		linear[1] += i;
		linear[2] += pulse[i];
		linear[3] += i*i;
	}
	linear[0] = linear[0]/range;
	linear[1] = linear[1]/range;
	linear[2] = linear[2]/range;
	linear[3] = linear[3]/range;
	
	*risetime = ((0.1*(*amplitude) - pulse[(k - (int)(range/2))])/((linear[0] 
		- linear[1]*linear[2])/(linear[3] - linear[1]*linear[1]))) + (k - (int)(range/2));
		
	j = k;
	for(i = j - 50; i < j; i++)
	{
		if(pulse[i] < (*amplitude)*0.1) {k = i;}
	}
	
	memset(linear, 0, sizeof(linear));
	for(i = (k - (int)(range/2)); i <= (k - (int)(range/2) + range); i++) {
		linear[0] += i*pulse[i];
		linear[1] += i;
		linear[2] += pulse[i];
		linear[3] += i*i;
	}
	linear[0] = linear[0]/range;
	linear[1] = linear[1]/range;
	linear[2] = linear[2]/range;
	linear[3] = linear[3]/range;
	
	*risetime = *risetime - ((0.9*(*amplitude) - pulse[(k - (int)(range/2))])/((linear[0] 
		- linear[1]*linear[2])/(linear[3] - linear[1]*linear[1]))) + (k - (int)(range/2));

		
	// NOTE: Still need to do fall time and width...
	*falltime = 0;
	*width = 0;
	
	return 0;
}

/** ----------------------------------------------------  
*	Determine location and number of peaks in a waveform
*		- Determines the number of peaks by applying a
*		  above-threshold condition at a user defined 
*		  number of std. dev. of the baseline all using
*		  the first derv. of the input waveform.  
*
*	Inputs:
*			pulse - input array
*			length - the length of pulse
*			sigma - threshold in number of std. deviations
*			range - number of points to deterimine std. dev.
*	Outputs:
*			numPeaks - number of peaks found
*			locPeaks - array of array indexes of peak locations
*
*	----------------------------------------------------
*/
int PulseAnalysis::PeakFinder (float* pulse, unsigned int length, int sigma, unsigned int range, int method, int* numPeaks, int* value)
{
	if (length < sizeof(pulse)/sizeof(float)) {return -1;}
	
	//locPeaks = new int*[10];
	for(i = 0; i < 10; ++i) 
	{
		//locPeaks[i] = new int[3];
	}
	
	// Calculate first derivative and average of first derv.
	x = 0;
	for (i = 0; i < length - 1; i++)
	{
		pulse_temp[i] = pulse[i + 1] - pulse[i];
		x =+ pulse_temp[i];
	}
	
	// Determine standard deviation of baseline
	x = x/range;
	z = 0;
	for (i = 0; i < range; i++) { z =+ pow((pulse_temp[i] - x), 2); }
	z = sqrt(z/range);
	
	*numPeaks = 0;
	if(method == 1) { w = sigma*z; }
	else if(method == 2) { w = sigma; }
	
	j = 0; k = 0; x = 0; y = 0; m = 0; n = 0;
	positive = 0; negative = 0;
	for (i = 0; i < length - 5; i++)
	{	
		if( pulse_temp[i] <= w && pulse_temp[i] >= -1*w)
		{	
			if (positive == 1 && negative == 1)
			{
				positive = 0;
				negative = 0;
				//locPeaks[*numPeaks - 1][1] = j;
				//locPeaks[*numPeaks - 1][2] = k;
				//locPeaks[*numPeaks - 1][0] = n;
				*value = k;
				
				x = 0; y = 0;
			}
		}	
		else
		{
				
			if (positive == 0 && negative == 0 && i > n + 40) { m++; n = i;}
		
			// Positive inflection point
			if(pulse_temp[i] > 0) 
			{	
				positive = 1;
				if (pulse_temp[i] > x) {x = pulse_temp[i]; j = i;}
			}
			
			// Negative inflection point
			if(pulse_temp[i] < 0) 
			{
				negative = 1;
				if (pulse_temp[i] < y) {y = pulse_temp[i]; k = i;}
			}
			
		}		
		
	}
	
	*numPeaks = m;
	
	return 0;
}

/** ----------------------------------------------------  
*	Determine the nth derivative of a waveform
*
*	Inputs:
*			pulse - input array
*			length - the length of pulse
*			order - order of the dervative (i.e. 1 = first, 2 = second, ...)
*	----------------------------------------------------
*/
int PulseAnalysis::Derivative(float* pulse, unsigned int length, unsigned int order)
{
	if (length < sizeof(pulse)/sizeof(float)) {return -1;}
	
	for(j = 1; j <= order; j++)
	{	
		for (i = 0; i < length - j; i++)
		{
			pulse[i] = pulse[i + 1] - pulse[i];
		}
	}
	return 0;
}

/** ----------------------------------------------------  
*	Determine the nth derivative of a waveform
*
*	Inputs:
*			pulse - input array
*			length - the length of pulse
*			order - order of the dervative (i.e. 1 = first, 2 = second, ...)
*	----------------------------------------------------
*/
int PulseAnalysis::Integral(float* pulse, unsigned int length)
{
	if (length < sizeof(pulse)/sizeof(float)) {return -1;}
	for (i = 0; i < length - 1; i++)
	{
			pulse[i + 1] = pulse[i + 1] + pulse[i];
	}
	return 0;
}

/** ----------------------------------------------------  
*	Output a time pickoff from user defined condition
*
*	Inputs:
*			pulse - input array
*			length - the length of pulse
*			
*	----------------------------------------------------
*/
int PulseAnalysis::Time_Pickoff (float* pulse, unsigned int length, int range, unsigned int low, unsigned int high , int method, float* CFD)
{
	float max, time;
	unsigned int index;
	
	if (length < sizeof(pulse)/sizeof(float)) {return -1;}
	
	if (method == 1)
	{	
	max = 0;
	
	for (i = low; i <= high; i++)
	{
		if (pulse[i] > max)
		{
			max = pulse[i];
			index = i;
		}		
	}	

	// Constant fraction discrimination (CFD) at 50% of amplitude
	
	for(i = low; i < index; i++)
	{
		if(pulse[i] < (max*0.5)) {k = i;}
	}

	time = (((max*0.5) - pulse[k])/(pulse[k+1] - pulse[k])) + (float)k;
	
	if (time >= low && time <= high){*CFD = time;}
	else {time = -1;}
	
	}
	
	if (method == 2)
	{
	
	max = 0;
	
	// Find first value over 'range'
	for (i = low; i <= high; i++)
	{
		if (pulse[i] > range)
		{
			k = i; break;
		}		
	}	
	
	time = (((((float)(range)) - pulse[k-1]))/(pulse[k] - pulse[k-1])) + (float)k;
	
	if (time >= low && time <= high){*CFD = time;}
	else {time = -1;}
	
	
	}
	
	return 0;
}

int PulseAnalysis::PSD_Zerocross (float* pulse, unsigned int length, unsigned int integration, int differentiation, float* PSD)
{
	if (length < sizeof(pulse)/sizeof(float)) {return -1;}
	
	j = 0;
	for (i = 0; i < length; i++)
	{
		if(pulse[i] > j) { j = (int)pulse[i]; k = i;}
	}
	
	// CFD timing pickoff at 50% of amplitude
	memset(linear, 0, sizeof(linear));
	
	for(i = (k - (int)(3/2)); i <= (k - (int)(3/2) + 3); i++) {
		linear[0] += i*pulse[i];
		linear[1] += i;
		linear[2] += pulse[i];
		linear[3] += i*i;
	}
	linear[0] = linear[0]/3;
	linear[1] = linear[1]/3;
	linear[2] = linear[2]/3;
	linear[3] = linear[3]/3;
	
	*PSD = ((0.5*(pulse[k]) - pulse[(k - (int)(3/2))])/((linear[0] 
		- linear[1]*linear[2])/(linear[3] - linear[1]*linear[1])))+ (k - (int)(3/2));
		
	// Shaping amplifier 
	memset(pulse_temp, 0, sizeof(pulse_temp));
	for(i = (int)integration; i < (length - integration); i++) {
		for(j = 0; j < integration; j++) {
			if((j + i) >= 0 && (j + i) < length) {pulse_temp[i] = pulse_temp[i] + pulse[j + i];}
		}
	}
	
	//memset(pulse, 0, sizeof(pulse));
	//for (i = 0; i < (length - 1); i++) {pulse[i] = pulse_temp[i + 1] - pulse_temp[i];}
	
	
	//PSD = TMath::LocMin(length,pulse) - TMath::LocMax(length,pulse);
	
	
	memset(pulse_temp, 0, sizeof(pulse_temp));
	for (i = 0; i < (length - 1); i++) {pulse_temp[i] = pulse[i + 1] - pulse[i];}
	
	//for (i=0; i< length; i++) {
	//	pulse[i] = pulse_temp[i];
	//}
	return 0;
}

/** ----------------------------------------------------  
*	Determine the integral below half the pulse amplitude.  Used for
*   reconstruction of partial pulses.  
*
*	Inputs:
*			pulse - input array
*			length - the length of pulse
*			integral - integral below half the pulse amplitude
*	----------------------------------------------------
*/
int PulseAnalysis::Half_Integral(float* pulse, unsigned int length, float* integral)
{
	if (length < sizeof(pulse)/sizeof(float)) {return -1;}
	
	*integral = 0;
	j = 0;
	for (i = 0; i < length; i++)
	{
		if(pulse[i] > j) { j = (int)pulse[i]; k = i;}
	}

	for (i = 0; i < length ; i++)
	{
		if(pulse[i] < (0.5*pulse[k]))
		{
			*integral = pulse[i + 1] + pulse[i];
		}
		else
		{
			*integral = pulse[i + 1] +pulse[k];
		}
	}

	
	return 0;
}

/** ----------------------------------------------------  
*	Smoothing method for waveforms
* 
*	Methods:
*			1 - Moving Average (option = range of moving average)
*
*	Inputs:
*			pulse - input array
*			length - the length of pulse
*			iterations - number of iterations to apply smoothing method
*			method - see above
*			option - see above
*	----------------------------------------------------
*/
int PulseAnalysis::Smooth(float* pulse, unsigned int length, unsigned int iterations, int method, float option)
{
	if (length < sizeof(pulse)/sizeof(float)) {return -1;}
	
	for(k = 0; k < iterations; k++)
	{
	// Moving average
	if (method == 1)
	{
		for (i = 2; i < length - 3; i++)
		{
			MovingAverage[i] = 0;
			for (j = i - 2; j <= i + 2; j++) {MovingAverage[i] += pulse[j];}
		}
		for (i =1; i < length - 3; i++)
		{
			pulse[i] = (MovingAverage[i]/5) ;
		}
	}

	}
	return 0;
}

/** ----------------------------------------------------  
*	HPGe
*	- This method processes signals from HPGe detectors.
*
*	Inputs:
*			pulse - input array
*			length - the length of pulse
*			amplitude - amplitude of the pulse
*	----------------------------------------------------
*/
int PulseAnalysis::HPGe(float* pulse, unsigned int length, float* amplitude){
	if (length < sizeof(pulse)/sizeof(float)) {return -1;}

	return 0;
}
