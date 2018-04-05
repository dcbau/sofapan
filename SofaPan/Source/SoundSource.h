/*
 ==============================================================================
 
 SoundSource.h
 Created: 17 Oct 2017 11:00:56am
 Author:  David Bau
 
 ==============================================================================
 */

#pragma once

#include "SofaData.h"
#include "fftw3.h"
#include "ParameterStruct.h"
#include "Delayline.h"
#include "ITDToolkit.h"
#include "NFSimToolkit.h"
#include "FilterKernel.h"
//#include "UpdateRateLimiter.h"
#include "CircularAngleSmoother.h"


#define MAX_DELAY 1000
#define L 0
#define R 1


typedef struct{
    float azimuth;
    float elevation;
    float distance;
    bool ITDAdjust;
    bool nfSimulation;
    bool overwriteOutputBuffer;
    float customHeadRadius;
    bool test;
}soundSourceData;

class SoundSource{
    
public:
    SoundSource();
    ~SoundSource();
    
    
    void process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, soundSourceData data);
    
    void prepareToPlay();
    
    int initWithSofaData(SOFAData* sD, int _sampleRate, int _index);
    
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
    CircularAngleSmoother azimuthSmoothed;
    LinearSmoothedValue<float> elevationSmoothed;

    
    int counter = 0;
    
    const float meterToMs = 1000.0 / 343.2;
    
    fftwf_complex* hrtf_l;
    fftwf_complex* hrtf_r;
    
    void interpolation(int leftOrRight);

    float* hrtfsForInterpolation_Mag[4];
    float* hrtfsForInterpolation_Phase[4];
    
    fftwf_complex* interpolatedHRTF;
    fftwf_complex* interpolatedHRTF_R;
    float interpolationDistances[4];
    
    int interpolationOrder;
    
    float previousAzimuth;
    float previousElevation;
    float previousDistance;
    bool previousITDAdjust;

    int index;
    
    
};

