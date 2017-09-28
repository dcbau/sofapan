///*
//  ==============================================================================
//
//    ImageSource.h
//    Created: 18 Aug 2017 9:40:07pm
//    Author:  David Bau
//
//  ==============================================================================
//*/


#pragma once

#include "SOFAData.h"
#include "fftw3.h"
#include "ParameterStruct.h"
#include "Delayline.h"

#define RE 0
#define IM 1


class ImageSource{
    
public:
    ImageSource();
    ~ImageSource();
    
    void process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, Point<float> refSource);
    void prepareToPlay(int _sampleRate);
    
    int initWithSofaData(SOFAData* sD, int _sampleRate, Line<float> _mirrorWall, int _order);
    
private:
    SOFAData* sofaData;
    
    int firLength;
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
    float* weightingCurve;
    
    int fifoIndex;
    
    fftwf_complex* previousHRTF;
    float previousAzimuth;
    float previousElevation;
    
    
    void releaseResources();
    
    Delayline delay;
    //Delayline delay;
    int sampleRate;
    float ITDdelayL_ms;
    float ITDdelayR_ms;
    
    const float speedOfSound = 343.2;
    const float meterToMs = 1000.0 / speedOfSound;


    //onepole lp
    float z_1, z_2;
    const float cutoff = 5000;
    float b0, b1, a1;
    
    Line<float> mirrorWall;
    int order = 0;
};



