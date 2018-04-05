/*
 ==============================================================================
 
 SoundSourceOld.cpp
 Created: 17 Oct 2017 11:00:56am
 Author:  David Bau
 
 ==============================================================================
 */

#include "SoundSourceOld.h"

SoundSourceOld::SoundSourceOld()

{
    filterOutBufferL = filterOutBufferR = inputBuffer = NULL;
    firLength = 0;
    
    
}

SoundSourceOld::~SoundSourceOld(){
    releaseResources();
}

int SoundSourceOld::initWithSofaData(SOFAData *sD, int _sampleRate){
    
    sofaData = sD;
    sampleRate = _sampleRate;
    
    filter.init(sD->getLengthOfHRIR(), 1);
    
    if(firLength != sD->getLengthOfHRIR()){
        releaseResources();
        
        firLength = sD->getLengthOfHRIR();
        complexLength = firLength + 1;
        //Allocate Memory
        inputBuffer = fftwf_alloc_real(firLength);
        filterOutBufferR = fftwf_alloc_real(firLength);
        filterOutBufferL =fftwf_alloc_real(firLength);
        if(inputBuffer == NULL || filterOutBufferL == NULL || filterOutBufferR == NULL){
            ErrorHandling::reportError("Direct Source Module", ERRMALLOC, true);
            return 1;
        }
    }
    
    distanceDelaySmoother.reset((double)sampleRate, 0.1);
    distanceGainSmoother.reset((double)sampleRate, 0.1);
    ITDDelaySmootherL.reset((double)sampleRate, 0.02);
    ITDDelaySmootherR.reset((double)sampleRate, 0.02);
    
    
    return 0;
}


void SoundSourceOld::releaseResources(){
    if(inputBuffer!= NULL) fftwf_free(inputBuffer);
    if(filterOutBufferL != NULL) free(filterOutBufferL);
    if(filterOutBufferR != NULL) free(filterOutBufferR);
    
    
    filterOutBufferL = filterOutBufferR = inputBuffer = NULL;
}



void SoundSourceOld::prepareToPlay(){
    
    filter.prepareToPlay(sofaData->getHRTFforAngle(0.0, 0.0, 1.0));
    
    fifoIndex = 0;
    
    for(int i = 0; i < firLength; i++){
        filterOutBufferL[i] = 0.0;
        filterOutBufferR[i] = 0.0;
    }
    
    ITDdelayL.specifyMaxDelayLength(MAX_ITD + MAX_DELAY);
    ITDdelayL.prepareToPlay(sampleRate);
    ITDdelayR.specifyMaxDelayLength(MAX_ITD + MAX_DELAY);
    ITDdelayR.prepareToPlay(sampleRate);
    delayL_ms = 0.0;
    delayR_ms = 0.0;
    
}



void SoundSourceOld::process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, soundSourceData data){
    
    distanceDelaySmoother.setValue(data.distance * meterToMs);
    distanceGainSmoother.setValue(1.0 / data.distance);
    for(int sample = 0; sample < numSamples; sample++){
        
        inputBuffer[fifoIndex] = inBuffer[sample];
        
        float smoothedDistanceDelay = distanceDelaySmoother.getNextValue();
        float delayLineOutputL = ITDdelayL.pullSample(ITDDelaySmootherL.getNextValue() + smoothedDistanceDelay);//smoothDelay.update(distanceDelay));
        ITDdelayL.pushSample(filterOutBufferL[fifoIndex]);
        float delayLineOutputR = ITDdelayR.pullSample(ITDDelaySmootherR.getNextValue() + smoothedDistanceDelay);//smoothDelay.update(distanceDelay));
        ITDdelayR.pushSample(filterOutBufferR[fifoIndex]);
        
        float g = distanceGainSmoother.getNextValue();
        if(data.overwriteOutputBuffer){
            outBuffer_L[sample] = delayLineOutputL * g;
            outBuffer_R[sample] = delayLineOutputR * g;
        }else{
            outBuffer_L[sample] += delayLineOutputL * g;
            outBuffer_R[sample] += delayLineOutputR * g;
        }
        
        fifoIndex++;
        
        if(fifoIndex == firLength){
            //printf("\nDistanceDelay: %.3f, Smoothed Delay: %.3f", distanceDelay, smoothedDelay);
            fifoIndex = 0;
            
            //update Distance Delay and ITD Delay
            delayL_ms = delayR_ms =0.0;
            if(data.ITDAdjust){
                float ITD = ITDToolkit::woodworthSphericalITD(sofaData->getMetadata().headRadius, data.azimuth, data.elevation);
                float mITD = sofaData->getITDForAngle(data.elevation, data.azimuth, data.distance).ITD_ms;
                //printf("\n%.3f°: Woodworth: %.3f, Measured: %.3f, Error: %.3f", data.azimuth, ITD, mITD, fabsf(ITD - mITD));
                
                
                if(ITD > 0.0)
                    delayL_ms = ITD;
                else
                    delayR_ms = fabsf(ITD);
                
                if(fabsf(ITDDelaySmootherL.getTargetValue() - delayL_ms) > 0.00001) //to avoid retriggers caused by small calculation errors
                    ITDDelaySmootherL.setValue(delayL_ms);
                if(fabsf(ITDDelaySmootherR.getTargetValue() - delayR_ms) > 0.00001)
                    ITDDelaySmootherR.setValue(delayR_ms);
            }else{
                ITDDelaySmootherL.setValue(0.0);
                ITDDelaySmootherR.setValue(0.0);
            }
            
            if(data.nfSimulation && data.distance < 1.0)
                processNearfield(data);
            else
                processFarfield(data);
        }
    }
}




void SoundSourceOld::processFarfield(soundSourceData data){
    
    fftwf_complex *hrtf;
    //update HRTF
    if(data.ITDAdjust)
        hrtf = sofaData->getMinPhaseHRTFforAngle(data.elevation, data.azimuth, data.distance);
    else
        hrtf = sofaData->getHRTFforAngle(data.elevation, data.azimuth, data.distance);
    
    filter.processBlock(inputBuffer, filterOutBufferL, filterOutBufferR, hrtf, hrtf + complexLength);
}




void SoundSourceOld::processNearfield(soundSourceData data){
    
    float distance = data.distance;
    float nfDistance = distance;
    float headradius = sofaData->getMetadata().headRadius;
    
    float azimuth_l = data.azimuth;
    float azimuth_r = data.azimuth;
    fftwf_complex *hrtf_l, *hrtf_r;
    
    //NearfieldSimulation: Acoustic parallax effect
    if(distance < 1.0 ){
        
        azimuth_l = NFSimToolkit::calculateNFAngleOffset(data.azimuth, distance, headradius);
        azimuth_r = NFSimToolkit::calculateNFAngleOffset(data.azimuth, distance, -headradius);
        
        //In case there are stored HRTFs with a smaller distance, this ensures that only the 1m-distance-hrtfs are used for the parallax-effect
        distance = 1.0;
        
    }
    
    //update HRTF
    if(data.ITDAdjust){
        hrtf_l = sofaData->getMinPhaseHRTFforAngle(data.elevation, azimuth_l, distance);
        hrtf_r = sofaData->getMinPhaseHRTFforAngle(data.elevation, azimuth_r, distance);
    }else{
        hrtf_l = sofaData->getHRTFforAngle(data.elevation, azimuth_l, distance);
        hrtf_r = sofaData->getHRTFforAngle(data.elevation, azimuth_r, distance);
    }
    
    filter.processBlock(inputBuffer, filterOutBufferL, filterOutBufferR, hrtf_l, hrtf_r + complexLength);
    
    float gainL = NFSimToolkit::getAdaptedIIDGain(data.distance, data.azimuth, data.elevation, 0);
    float gainR = NFSimToolkit::getAdaptedIIDGain(data.distance, data.azimuth, data.elevation, 1);
    for(int i = 0; i < firLength; i++){
        filterOutBufferL[i] *= gainL;
        filterOutBufferR[i] *= gainR;
    }
    
}

