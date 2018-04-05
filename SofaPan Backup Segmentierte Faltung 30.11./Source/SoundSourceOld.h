/*
 ==============================================================================
 
 SoundSourceOld.h
 Created: 17 Oct 2017 11:00:56am
 Author:  David Bau
 
 ==============================================================================
 */

#pragma once

#include "SOFAData.h"
#include "fftw3.h"
#include "ParameterStruct.h"
#include "Delayline.h"
#include "ITDToolkit.h"
#include "NFSimToolkit.h"
#include "FilterKernel.h"
#include "UpdateRateLimiter.h"

#define MAX_DELAY 1000

typedef struct{
    float azimuth;
    float elevation;
    float distance;
    bool ITDAdjust;
    bool nfSimulation;
    bool overwriteOutputBuffer;
}soundSourceData;

class SoundSourceOld{
    
public:
    SoundSourceOld();
    ~SoundSourceOld();
    
    
    void process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, soundSourceData data);
    
    void prepareToPlay();
    
    int initWithSofaData(SOFAData* sD, int _sampleRate);
    
private:
    void processNearfield(soundSourceData data);
    void processFarfield(soundSourceData data);
    
    SOFAData* sofaData;
    int sampleRate;
    
    int firLength;
    int complexLength;
    
    float* inputBuffer;
    float* filterOutBufferL;
    float* filterOutBufferR;
    
    int fifoIndex;
    
    void releaseResources();
    
    Delayline ITDdelayL;
    Delayline ITDdelayR;
    
    float delayL_ms;
    float delayR_ms;
    
    FilterKernel filter;
    
    LinearSmoothedValue<float> distanceDelaySmoother;
    LinearSmoothedValue<float> distanceGainSmoother;
    LinearSmoothedValue<float> ITDDelaySmootherL;
    LinearSmoothedValue<float> ITDDelaySmootherR;
    
    int counter = 0;
    
    const float meterToMs = 1000.0 / 343.2;
    
    
};

