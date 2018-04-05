/*
  ==============================================================================

    MinPhaseGenerator.h
    Created: 19 Jan 2018 11:09:39am
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "fftw3.h"
#include "ITDToolkit.h"

class MinPhaseGenerator{
public:
    MinPhaseGenerator(){
        lengthOfHRTF = 0;
        realBuffer = NULL;
        complexBuffer = NULL;
        inverse = NULL;
        forward = NULL;
    }
    
    ~MinPhaseGenerator(){
        releaseResources();
    }

    /** Creates the PMP by detecting the onset (determinded by the threshold) and cutting away all prior samples
     
     @param inputSequence  Pointer to array of floats. Resulting Minimum Phase Version will be stored back into this
     @param sequenceLength Number of elements in the array
     @param onset_detection_threshold A value between 0 and 1
     @return  the onset position in samples
     */
    int makePseudoMinPhaseFilter(float* inputSequence, int sequenceLength, float onset_detection_threshold){
        
        int onsetIndex = ITDToolkit::detectOnset(inputSequence, sequenceLength, onset_detection_threshold, false);
        
        //Create the quasi minimumphase HRIR by shifting the onset position to the beginning
        for(int i = 0; i< sequenceLength; i++)
        {
            if ((i + onsetIndex) < sequenceLength)
                inputSequence[i] = inputSequence[i + onsetIndex];
            else
                inputSequence[i] = 0.0;
            
        }
        
        return onsetIndex;
        
    }

    /** Takes a sequence of numbers (e.g. an impulse response) and creates a minimum phase sequence with the same magnitude spectrum
     
     @param (Input) Pointer real-valued input sequence
     @param (Output) Pointer to preallocated array, where the minimum phase sequence is stored
     @param Length of the input sequence AND the output sequence
     */
    void makeMinPhaseFilter(float* filter_in, float* rfilter_out, int lengthOfFIR){
        
        
        //extend the length of the FFT to at least 8 times the input length to reduce aliasing
        const int oversampling = 8;
        int lengthOfFilter= lengthOfFIR * (oversampling/2) + 1;
        
        if(lengthOfFilter != lengthOfHRTF)
            init(lengthOfFilter);
        
        
        /* The following steps have been evaluated with matlab and deliver the same results as the rceps matlab function, which can be used to derive a minphase filter. For more information on that algorithm, see https://de.mathworks.com/help/signal/ref/rceps.html */
        
        //zeropadding
        for(int n = 0; n < lengthOfFIR; n++)
        {
            realBuffer[n] = filter_in[n];
            realBuffer[n + lengthOfFIR] = 0.0;
        }
        for(int n = lengthOfFIR; n < lengthOfFFT; n++)
        {
            realBuffer[n] = 0.0;
        }
        
        //transform
        fftwf_execute(forward);
        
        //take magnitude value and convert to log scale
        float mag;
        for(int k = 0; k < lengthOfHRTF; k++)
        {
            mag = sqrtf(complexBuffer[k][0] * complexBuffer[k][0] + complexBuffer[k][1] * complexBuffer[k][1]);
            complexBuffer[k][0] = logf( mag );
            complexBuffer[k][1] = 0.0;
        }
        
        //inverse transform to real cepstrum
        fftwf_execute(inverse);
        
        //apply windowing function
        for(int n = 0; n < lengthOfFFT; n++)
        {
            realBuffer[n] = realBuffer[n] / (float)lengthOfFFT;
            realBuffer[n] *= L[n];
        }
        
        //convert back to spectrum
        fftwf_execute(forward);
        
        //make complex exp to finally derive minphase filter:
        //e^z = e^x * (cos(y) + i sin(y))
        float re, im;
        for(int k = 0; k < lengthOfHRTF; k++)
        {
            re = expf(complexBuffer[k][0]) * cosf(complexBuffer[k][1]); // xr = e^x * cos(y)
            im = expf(complexBuffer[k][0]) * sinf(complexBuffer[k][1]); // xi = e^x * sin(y)
            complexBuffer[k][0] = re;
            complexBuffer[k][1] = im;
        }
        
        //recreate the minphase hrir (only use the samples up to FIR length, since the hrir was zero padded before transfromation, and the rest contains alias signals )
        
        fftwf_execute(inverse);
        
        for(int n = 0; n < lengthOfFIR; n++)
        {
            rfilter_out[n] = realBuffer[n] / (float)lengthOfFFT;
        }
            
        
        
        
        
    }
    
    /** The initalization routine can be called prior to the minphase transformation. But it will be called by the transformation itself if needed
     @param The length of the HRTF (Length of Impulse Response + 1)
     */
    void init(int _lengthOfHRTF){
        
        releaseResources();
        
        lengthOfHRTF = _lengthOfHRTF;
        lengthOfFFT = (lengthOfHRTF - 1) * 2;
        
        realBuffer = fftwf_alloc_real(lengthOfFFT);
        complexBuffer = fftwf_alloc_complex(lengthOfHRTF);
        L = (float*)malloc(lengthOfFFT * sizeof(double));
        
        forward = fftwf_plan_dft_r2c_1d(lengthOfFFT, realBuffer, complexBuffer, FFTW_ESTIMATE);
        inverse = fftwf_plan_dft_c2r_1d(lengthOfFFT, complexBuffer, realBuffer, FFTW_ESTIMATE);
        
        //Create windowing sequence
        int N2 = lengthOfFFT / 2;
        for(int n = 1; n < N2; n++){
            L[n] = 2.0;
            L[n + N2] = 0.0;
        }
        L[0] = L[N2] = 1.0;
    }
    
private:
    fftwf_plan forward, inverse;
    float* realBuffer;
    fftwf_complex* complexBuffer;
    int lengthOfHRTF;
    int lengthOfFFT;
    float* L;
    
    void releaseResources(){
        if(realBuffer != NULL) fftwf_free(realBuffer);
        if(complexBuffer != NULL) fftwf_free(complexBuffer);
        if(forward != NULL) fftwf_destroy_plan(forward);
        if(inverse != NULL) fftwf_destroy_plan(inverse);

    }
};
