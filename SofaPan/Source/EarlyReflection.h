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
#include "FilterKernel.h"
#include "SofaData.h"
#include "ParameterStruct.h"
#include "ITDToolkit.h"


/**
 This is a filter for one reflection. It is basically a stripped down version of the DirectSource, that uses one constant HRTF that is set during init. To make it a working reflection, it has a delayline and you can specify a delay up to 100ms and a damping factor (for every process call).
 
 Another important difference is that the output of the reflection-process does NOT overwrite the buffer that it was given, instead it sums up the contents of the given output buffer and its own output. So multiple reflections can be used in a row with summed output.
 */
class EarlyReflection{
    
public:
    EarlyReflection();
    ~EarlyReflection();
    
    int initWithSofaData(SOFAData *sD, int _sampleRate, float angle_az, float angle_el);
    
    void process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, float delayInMs, float gain, parameterStruct params);
    
    void prepareToPlay();
    
private:
    void releaseResources();
    
    fftwf_complex *hrtf;
    fftwf_complex *hrtf_minPhase;
    
    SOFAData* sofaData;
    float azimuth;
    float elevation;
    int sampleRate;

    float headRadius;
    
    int firLength;
    int complexLength;

    float* inputBuffer;
    float* filterOutBufferL;
    float* filterOutBufferR;
    
    
    int fifoIndex;

    
    Delayline delayL;
    Delayline delayR;
    
    float delayL_ms;
    float delayR_ms;
    
    //onepole lp
    float z1;
    const float cutoff = 5000;
    float b0, b1, a1;
    
    FilterKernel filter;
    
};

