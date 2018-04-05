/*
  ==============================================================================

    SoundSource.cpp
    Created: 17 Oct 2017 11:00:56am
    Author:  David Bau

  ==============================================================================
*/

#include "SoundSource.h"

SoundSource::SoundSource()

{
    filterOutBufferL = filterOutBufferR = inputBuffer = NULL;
    firLength = 0;
    filterBlockSize = 0;
    
    
}

SoundSource::~SoundSource(){
    releaseResources();
}

//int SoundSource::initWithSofaData(SOFAData *sD, int _sampleRate, int blockSize){
//    
//    sofaData = sD;
//    sampleRate = _sampleRate;
//    
//
//    
////    if(firLength != sD->getLengthOfHRIR()){
////        releaseResources();
////
////        firLength = sD->getLengthOfHRIR();
////        complexLength = firLength + 1;
////        //Allocate Memory
////        inputBuffer = fftwf_alloc_real(firLength);
////        filterOutBufferR = fftwf_alloc_real(firLength);
////        filterOutBufferL =fftwf_alloc_real(firLength);
////        if(inputBuffer == NULL || filterOutBufferL == NULL || filterOutBufferR == NULL){
////            ErrorHandling::reportError("Direct Source Module", ERRMALLOC, true);
////            return 1;
////        }
////    }
//    
//    distanceDelaySmoother.reset((double)sampleRate, 0.1);
//    distanceGainSmoother.reset((double)sampleRate, 0.1);
//    ITDDelaySmootherL.reset((double)sampleRate, 0.02);
//    ITDDelaySmootherR.reset((double)sampleRate, 0.02);
//
//    
//    return 0;
//}


void SoundSource::releaseResources(){
    if(inputBuffer!= NULL) fftwf_free(inputBuffer);
    if(filterOutBufferL != NULL) free(filterOutBufferL);
    if(filterOutBufferR != NULL) free(filterOutBufferR);
    
    
    filterOutBufferL = filterOutBufferR = inputBuffer = NULL;
}



void SoundSource::prepareToPlay(SOFAData *sD, int _sampleRate, int _filterBlockSize){
    
    sofaData = sD;
    sampleRate = _sampleRate;
    filterBlockSize = _filterBlockSize;
    lengthOfHRTF = filterBlockSize + 1;
    numFilterBlocks = sD->getLengthOfHRIR() / filterBlockSize;
    
    filter.prepareToPlay(sofaData->getHRTFforAngle(0.0, 0.0, 1.0), filterBlockSize, numFilterBlocks);
    
    fifoIndex = 0;
    
    for(int i = 0; i < firLength; i++){
        filterOutBufferL[i] = 0.0;
        filterOutBufferR[i] = 0.0;
    }
    
    
    distanceDelaySmoother.reset((double)sampleRate, 0.1);
    distanceGainSmoother.reset((double)sampleRate, 0.1);
    ITDDelaySmootherL.reset((double)sampleRate, 0.02);
    ITDDelaySmootherR.reset((double)sampleRate, 0.02);
    
    
    ITDdelayL.specifyMaxDelayLength(MAX_ITD + MAX_DELAY);
    ITDdelayL.prepareToPlay(sampleRate);
    ITDdelayR.specifyMaxDelayLength(MAX_ITD + MAX_DELAY);
    ITDdelayR.prepareToPlay(sampleRate);
    delayL_ms = 0.0;
    delayR_ms = 0.0;
        
}



void SoundSource::process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, soundSourceData data){
    
    distanceDelaySmoother.setValue(data.distance * meterToMs);
    distanceGainSmoother.setValue(1.0 / data.distance);
    
    int filterBlocksPerProcessBlock;
    if(numSamples % filterBlockSize == 0)
        filterBlocksPerProcessBlock = numSamples / filterBlockSize;
    else
        printf("\nfucked up");
    
    printf("\nFilterBlocksPerProcessBlock = %d", filterBlocksPerProcessBlock);
    //update Distance Delay and ITD Delay
    delayL_ms = delayR_ms =0.0;
    if(data.ITDAdjust){
        float ITD = ITDToolkit::woodworthSphericalITD(sofaData->getMetadata().headRadius, data.azimuth, data.elevation);

        if(ITD > 0.0)   delayL_ms = ITD;
        else    delayR_ms = fabsf(ITD);
        
        if(fabsf(ITDDelaySmootherL.getTargetValue() - delayL_ms) > 0.00001) //to avoid retriggers caused by small calculation errors
            ITDDelaySmootherL.setValue(delayL_ms);
        if(fabsf(ITDDelaySmootherR.getTargetValue() - delayR_ms) > 0.00001)
            ITDDelaySmootherR.setValue(delayR_ms);
    }else{
        ITDDelaySmootherL.setValue(0.0);
        ITDDelaySmootherR.setValue(0.0);
    }
    
    for(int i = 0; i < filterBlocksPerProcessBlock; i++){
        if(data.nfSimulation && data.distance < 1.0)
            processNearfield(data, inBuffer + i * filterBlockSize, outBuffer_L + i * filterBlockSize, outBuffer_R + i * filterBlockSize);
        else
            processFarfield(data, inBuffer + i * filterBlockSize, outBuffer_L + i * filterBlockSize, outBuffer_R + i * filterBlockSize);
    }
    
    for(int sample = 0; sample < numSamples; sample++){
        
        //inputBuffer[fifoIndex] = inBuffer[sample];
        
        float smoothedDistanceDelay = distanceDelaySmoother.getNextValue();
        float delayLineOutputL = ITDdelayL.pullSample(ITDDelaySmootherL.getNextValue() + smoothedDistanceDelay);//smoothDelay.update(distanceDelay));
        ITDdelayL.pushSample(outBuffer_L[sample]);
        float delayLineOutputR = ITDdelayR.pullSample(ITDDelaySmootherR.getNextValue() + smoothedDistanceDelay);//smoothDelay.update(distanceDelay));
        ITDdelayR.pushSample(outBuffer_R[sample]);
        
        float g = distanceGainSmoother.getNextValue();
        if(data.overwriteOutputBuffer){
            outBuffer_L[sample] = delayLineOutputL * g;
            outBuffer_R[sample] = delayLineOutputR * g;
        }else{
            outBuffer_L[sample] += delayLineOutputL * g;
            outBuffer_R[sample] += delayLineOutputR * g;
        }
        
    }
}




void SoundSource::processFarfield(soundSourceData data, const float* inBuffer, float* outBuffer_L, float* outBuffer_R){
    
    fftwf_complex *hrtf;
    //update HRTF
    if(data.ITDAdjust)
        hrtf = sofaData->getMinPhaseHRTFforAngle(data.elevation, data.azimuth, data.distance);
    else
        hrtf = sofaData->getHRTFforAngle(data.elevation, data.azimuth, data.distance);
    
    filter.processBlock(inBuffer, outBuffer_L, outBuffer_R, hrtf, hrtf + lengthOfHRTF * numFilterBlocks);
}




void SoundSource::processNearfield(soundSourceData data, const float* inBuffer, float* outBuffer_L, float* outBuffer_R){
    
    float distance = data.distance;
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

    filter.processBlock(inBuffer, outBuffer_L, outBuffer_R, hrtf_l, hrtf_r + lengthOfHRTF * numFilterBlocks);

    float gainL = NFSimToolkit::getAdaptedIIDGain(data.distance, data.azimuth, data.elevation, 0);
    float gainR = NFSimToolkit::getAdaptedIIDGain(data.distance, data.azimuth, data.elevation, 1);
    for(int i = 0; i < firLength; i++){
        filterOutBufferL[i] *= gainL;
        filterOutBufferR[i] *= gainR;
    }
    
}
