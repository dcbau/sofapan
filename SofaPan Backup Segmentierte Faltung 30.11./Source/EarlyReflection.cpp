/*
  ==============================================================================

    EarlyReflection.cpp
    Created: 27 Jun 2017 12:18:49am
    Author:  David Bau

  ==============================================================================
*/

#include "EarlyReflection.h"








EarlyReflection::EarlyReflection()
{
    filterOutBufferL = filterOutBufferR = inputBuffer = NULL;
    hrtf =  hrtf_minPhase = NULL;
    firLength = 0;
}

EarlyReflection::~EarlyReflection(){
    releaseResources();
}

int EarlyReflection::initWithSofaData(SOFAData *sD, int _sampleRate, float angle_az, float angle_el){
    
    azimuth = angle_az;
    elevation = angle_el;
    
    sofaData = sD;
    sampleRate = _sampleRate;
    complexLength = firLength + 1;
    
    
    if(firLength != sD->getLengthOfHRIR()){
        releaseResources();
        
        firLength = sD->getLengthOfHRIR();
        complexLength = firLength + 1;
        
        //Allocate Memory
        hrtf = fftwf_alloc_complex(2*complexLength);
        hrtf_minPhase = fftwf_alloc_complex(2*complexLength);
        
        inputBuffer = fftwf_alloc_real(firLength);
        filterOutBufferR = fftwf_alloc_real(firLength);
        filterOutBufferL =fftwf_alloc_real(firLength);
        if(inputBuffer == NULL ||
           filterOutBufferL == NULL ||
           filterOutBufferR == NULL ||
           hrtf == NULL||
           hrtf_minPhase == NULL){
            ErrorHandling::reportError("Early Reflection Module", ERRMALLOC, true);
            return 1;
        }
    }
    
    fftwf_complex* _hrtf = sD->getHRTFforAngle(elevation, azimuth, 1.0);
    for(int i = 0; i < 2 * complexLength; i++){
        hrtf[i][0] = _hrtf[i][0];
        hrtf[i][1] = _hrtf[i][1];
    }
    _hrtf = sD->getMinPhaseHRTFforAngle(elevation, azimuth, 1.0);
    for(int i = 0; i < 2 * complexLength; i++){
        hrtf_minPhase[i][0] = _hrtf[i][0];
        hrtf_minPhase[i][1] = _hrtf[i][1];
    }
    //filter.init(firLength, 4);
    
    //LP coeffs
    float K = tanf(M_PI * cutoff/sampleRate);
    b0 = b1 = K / (K + 1);
    a1 = (K - 1) / (K + 1);
    
    return 0;
}


void EarlyReflection::releaseResources(){
    
    if(inputBuffer != NULL) fftwf_free(inputBuffer);
    if(filterOutBufferL != NULL) free(filterOutBufferL);
    if(filterOutBufferR != NULL) free(filterOutBufferR);
    if(hrtf != NULL)fftwf_free(hrtf);
    if(hrtf_minPhase != NULL)fftwf_free(hrtf_minPhase);
    
    filterOutBufferL = filterOutBufferR = inputBuffer = NULL;
    hrtf = hrtf_minPhase = NULL;
}



void EarlyReflection::prepareToPlay(){
    
    //filter.prepareToPlay(sofaData->getHRTFforAngle(0.0, 0.0, 1.0));
    
    fifoIndex = 0;
    z1 = 0.0;
    
    for(int i = 0; i < firLength; i++){
        filterOutBufferL[i] = 0.0;
        filterOutBufferR[i] = 0.0;
    }
    
    delayL.specifyMaxDelayLength(100);
    delayL.prepareToPlay(sampleRate);
    delayR.specifyMaxDelayLength(100);
    delayR.prepareToPlay(sampleRate);
    
    delayL_ms = 0.0;
    delayR_ms = 0.0;
    
}

void EarlyReflection::process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, float delayInMs, float gain, parameterStruct params){
    
//    
//    for(int sample = 0; sample < numSamples; sample++){
//        
//        //Lowpass Filter
//        float v1 =  inBuffer[sample] - (a1 * z1);
//        float lowpassFilterOutput = b0 * v1 + b1 * z1;
//        z1 = v1;
//        
//        //Collect Samples
//        inputBuffer[fifoIndex] = lowpassFilterOutput;
//        
//        //Output of already convoluted Samples
//        outBuffer_L[sample] += delayL.pullSample(delayL_ms) * gain;
//        delayL.pushSample(filterOutBufferL[fifoIndex]);
//        
//        outBuffer_R[sample] += delayR.pullSample(delayR_ms) * gain;
//        delayR.pushSample(filterOutBufferR[fifoIndex]);
//        
//        fifoIndex++;
//        
//        if(fifoIndex == firLength){
//            fifoIndex = 0;
//            
//            delayL_ms = delayR_ms = delayInMs;
//            if(params.ITDAdjustParam->get()){
//                filter.processBlock(inputBuffer, filterOutBufferL, filterOutBufferR, hrtf_minPhase, hrtf_minPhase + complexLength);
//                float ITD = ITDToolkit::woodworthSphericalITD(headRadius, azimuth, elevation);
//                if(ITD > 0.0)
//                    delayL_ms += ITD;
//                else
//                    delayR_ms += fabsf(ITD);
//            }else{
//                filter.processBlock(inputBuffer, filterOutBufferL, filterOutBufferR, hrtf, hrtf + complexLength);
//            }
//        }
//    }
}
