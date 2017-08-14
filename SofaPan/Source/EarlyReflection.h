/*
  ==============================================================================

    EarlyReflection.h
    Created: 27 Jun 2017 12:18:49am
    Author:  David Bau

  ==============================================================================
*/

#pragma once


#include "fftw3.h"
#include "string.h"
#include <stdlib.h>
#include "math.h"
#include "Delayline.h"
#include "ErrorHandling.h"

#define RE 0
#define IM 1

/**
 This is a filter for one reflection. It is basically a stripped down version of the FilterEngine, that uses one constant HRTF that is set during init. To make it a working reflection, it has a delayline and you can specify a delay up to 100ms and a damping factor (for every process call).
 
 Another important difference is that the output of the reflection-process does NOT overwrite the buffer that it was given, instead it sums up the contents of the given output buffer and its own output. So multiple reflections can be used in a row with summed output.
 */
class EarlyReflection{
    
public:
    EarlyReflection(fftwf_complex* hrtf, int lengthOfHRIR, int _sampleRate);
    ~EarlyReflection();
    
    void process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, float delayInMs, float gain);
    
    void prepareToPlay();
    
private:
    fftwf_complex* hrtf_l;
    fftwf_complex* hrtf_r;
    int sampleRate;

    
    const int firLength;
    int fftLength;
    int complexLength;
    float fftSampleScale;
    
    fftwf_plan forward;
    fftwf_plan inverse_L;
    fftwf_plan inverse_R;
    
    float* fftInputBuffer;
    fftwf_complex* complexBuffer;
    fftwf_complex* src;
    float* fftOutputBuffer_L;
    float* fftOutputBuffer_R;
    
    float* inputBuffer;
    float* lastInputBuffer;
    float* outputBuffer_L;
    float* outputBuffer_R;
    
    
    int fifoIndex;

    Delayline delay;
    
    //onepole lp
    float z1;
    const float cutoff = 5000;
    float b0, b1, a1;
    
    
};

