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
    filterOutBufferL = filterOutBufferR = inputBuffer  = NULL;
    interpolatedHRTF = NULL;
    interpolatedHRTF_R = NULL;
    firLength = 0;
    
    
    
}

SoundSource::~SoundSource(){
    releaseResources();
}

int SoundSource::initWithSofaData(SOFAData *sD, int _sampleRate, int _index){
    
    index = _index;
    
    
    sofaData = sD;
    sampleRate = _sampleRate;
    
    filter.init(sD->getLengthOfHRIR());
    
    if(firLength != sD->getLengthOfHRIR()){
        releaseResources();
        
        firLength = sD->getLengthOfHRIR();
        complexLength = firLength + 1;
        //Allocate Memory
        inputBuffer = fftwf_alloc_real(firLength);
        filterOutBufferR = fftwf_alloc_real(firLength);
        filterOutBufferL =fftwf_alloc_real(firLength);
        interpolatedHRTF = fftwf_alloc_complex(complexLength * 2);
        interpolatedHRTF_R = fftwf_alloc_complex(complexLength * 2);

        if(inputBuffer == NULL || filterOutBufferL == NULL || filterOutBufferR == NULL){
            ErrorHandling::reportError("Direct Source Module", ERRMALLOC, true);
            return 1;
        }
    }
    
    previousAzimuth = 0;
    previousElevation = 0;
    previousDistance = 1.0;
    previousITDAdjust = false;
    hrtf_l = sofaData->getHRTFforAngle(0.0, 0.0, 1.0, hrtf_type_pseudoMinPhase);
    hrtf_r = sofaData->getHRTFforAngle(0.0, 0.0, 1.0, hrtf_type_pseudoMinPhase);
    
    
    interpolationOrder = 2;
    
    if(sofaData->getMetadata().hasMultipleDistances)
        interpolationOrder++;
    
    if(sofaData->getMetadata().hasElevation)
        interpolationOrder++;
    
    printf("\n\n INterpolation Order: %d \n\n", interpolationOrder);
    
    distanceDelaySmoother.reset((double)sampleRate, 0.5);
    distanceGainSmoother.reset((double)sampleRate, 0.5);
    ITDDelaySmootherL.reset((double)sampleRate, 0.02);
    ITDDelaySmootherR.reset((double)sampleRate, 0.02);
    
    
    azimuthSmoothed.reset( ((float)sampleRate / (float)firLength) , 0.1);
    elevationSmoothed.reset( ((float)sampleRate / (float)firLength) , 0.1);

    
    return 0;
}


void SoundSource::releaseResources(){
    if(inputBuffer!= NULL) fftwf_free(inputBuffer);
    if(filterOutBufferL != NULL) free(filterOutBufferL);
    if(filterOutBufferR != NULL) free(filterOutBufferR);
    if(interpolatedHRTF != NULL) fftwf_free(interpolatedHRTF);
    if(interpolatedHRTF_R != NULL) fftwf_free(interpolatedHRTF_R);

    filterOutBufferL = filterOutBufferR = inputBuffer = NULL;
    interpolatedHRTF = NULL;
    interpolatedHRTF_R = NULL;
    
}



void SoundSource::prepareToPlay(){
    
    filter.prepareToPlay(sofaData->getHRTFforAngle(0.0, 0.0, 1.0, hrtf_type_pseudoMinPhase));
    
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



void SoundSource::process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, soundSourceData data){
    

    
    distanceDelaySmoother.setValue(data.distance * meterToMs);
    distanceGainSmoother.setValue(1.0 / data.distance);

    azimuthSmoothed.setValue(data.azimuth);
    elevationSmoothed.setValue(data.elevation);
    
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
            

            
            data.azimuth = azimuthSmoothed.getNextValue();
            data.elevation = elevationSmoothed.getNextValue();
            
//            if(index == 11 && (data.azimuth < 45.0 || data.azimuth > 135.0))
//                printf("\n ERROR [%d]  :%.3f", index, data.azimuth);

//            if(index == 12 && (data.azimuth < 135.0 || data.azimuth > 225.0))
//                printf("\n ERROR [%d]  :%.3f", index, data.azimuth);
//
//            if(index == 13 && (data.azimuth < 225.0 || data.azimuth > 315.0))
//                printf("\n ERROR [%d]  :%.3f", index, data.azimuth);
            
            //printf("\nDistanceDelay: %.3f, Smoothed Delay: %.3f", distanceDelay, smoothedDelay);
            fifoIndex = 0;
            
            //update Distance Delay and ITD Delay
            delayL_ms = delayR_ms =0.0;
            
            float ITD = 0.0;
            if(data.ITDAdjust){
                //if(data.test)
                    ITD = ITDToolkit::woodworthSphericalITD(data.customHeadRadius, data.azimuth, data.elevation);
                //else
                  //  ITD = ITDToolkit::woodworthSphericalITD(sofaData->getMetadata().headRadius, data.azimuth, data.elevation);
            }
//            else
//                ITD = sofaData->getITDForAngle(data.elevation, data.azimuth, data.distance).ITD_ms;
//                //printf("\n%.3f°: Woodworth: %.3f, Measured: %.3f, Error: %.3f", data.azimuth, ITD, mITD, fabsf(ITD - mITD));
//                
                
            if(ITD > 0.0)
                delayL_ms = ITD;
            else
                delayR_ms = fabsf(ITD);
            
            if(fabsf(ITDDelaySmootherL.getTargetValue() - delayL_ms) > 0.00001) //to avoid retriggers caused by small calculation errors
                ITDDelaySmootherL.setValue(delayL_ms);
            if(fabsf(ITDDelaySmootherR.getTargetValue() - delayR_ms) > 0.00001)
                ITDDelaySmootherR.setValue(delayR_ms);
//            }else{
//                ITDDelaySmootherL.setValue(0.0);
//                ITDDelaySmootherR.setValue(0.0);
//            }
            
            if(data.nfSimulation && data.distance < 1.0)
                processNearfield(data);
            else
                processFarfield(data);
        }
    }
}




void SoundSource::processFarfield(soundSourceData data){
    
    //update HRTF
    if(previousITDAdjust != data.ITDAdjust
       || previousAzimuth != data.azimuth
       || previousElevation != data.elevation
       || previousDistance != data.distance)
    {
        
        if(data.ITDAdjust)
        {
            sofaData->getHRTFsForInterpolation(hrtfsForInterpolation_Mag, hrtfsForInterpolation_Phase, interpolationDistances, data.elevation, data.azimuth, data.distance, interpolationOrder);
            interpolation(L);
            hrtf_l = interpolatedHRTF;
        }else
        {
            hrtf_l = sofaData->getHRTFforAngle(data.elevation, data.azimuth, data.distance, hrtf_type_original);
        }
        
        hrtf_r = hrtf_l;

        previousAzimuth = data.azimuth;
        previousElevation = data.elevation;
        previousDistance = data.distance;
        previousITDAdjust = data.ITDAdjust;

    }
    
    
    filter.processBlock(inputBuffer, filterOutBufferL, filterOutBufferR, hrtf_l, hrtf_r + complexLength);
}




void SoundSource::processNearfield(soundSourceData data){
    
    float distance = data.distance;
    float headradius = sofaData->getMetadata().headRadius;
    
    float azimuth_l = data.azimuth;
    float azimuth_r = data.azimuth;
    
    //NearfieldSimulation: Acoustic parallax effect

    azimuth_l = NFSimToolkit::calculateNFAngleOffset(data.azimuth, distance, headradius);
    azimuth_r = NFSimToolkit::calculateNFAngleOffset(data.azimuth, distance, -headradius);
        
    //In case there are stored HRTFs with a smaller distance, this ensures that only the 1m-distance-hrtfs are used for the parallax-effect
    distance = 1.0;
    
    //update HRTF

    if(previousITDAdjust != data.ITDAdjust
       || previousAzimuth != data.azimuth
       || previousElevation != data.elevation
       || previousDistance != data.distance)
    {

        if(data.ITDAdjust){
            sofaData->getHRTFsForInterpolation(hrtfsForInterpolation_Mag, hrtfsForInterpolation_Phase, interpolationDistances, data.elevation, azimuth_l, distance, interpolationOrder);
            interpolation(L);
            hrtf_l = interpolatedHRTF;
            
            sofaData->getHRTFsForInterpolation(hrtfsForInterpolation_Mag, hrtfsForInterpolation_Phase, interpolationDistances, data.elevation, azimuth_r, distance, interpolationOrder);
            interpolation(R);
            hrtf_r = interpolatedHRTF_R;
        }else{
            hrtf_l = sofaData->getHRTFforAngle(data.elevation, azimuth_l, data.distance, hrtf_type_original);
            hrtf_r = sofaData->getHRTFforAngle(data.elevation, azimuth_r
                                               , data.distance, hrtf_type_original);

        }

        
        previousAzimuth = data.azimuth;
        previousElevation = data.elevation;
        previousDistance = data.distance;
        previousITDAdjust = data.ITDAdjust;
    }
    
    filter.processBlock(inputBuffer, filterOutBufferL, filterOutBufferR, hrtf_l, hrtf_r + complexLength);
    
    float gainL = NFSimToolkit::getAdaptedIIDGain(data.distance, data.azimuth, data.elevation, 0);
    float gainR = NFSimToolkit::getAdaptedIIDGain(data.distance, data.azimuth, data.elevation, 1);
    for(int i = 0; i < firLength; i++){
        filterOutBufferL[i] *= gainL;
        filterOutBufferR[i] *= gainR;
    }
    
}


void SoundSource::interpolation(int leftOrRight){

    float w1, w2, w3, w4;
    float mag, phase;

    float w[interpolationOrder];
    
    float weightSum = 0.0;
    for(int k = 0; k < interpolationOrder; k++)
    {
        w[k] = interpolationDistances[k] <0.00001 ? 100000 : 1 / interpolationDistances[k];
        weightSum += w[k];
    }

    for(int k = 0; k < interpolationOrder; k++)
        w[k] /= weightSum;
    
    
    for(int i = 0; i < complexLength * 2; i++)
    {
        mag = 0;
        phase = 0;
        
        for(int k = 0; k < interpolationOrder; k++)
        {
            mag += hrtfsForInterpolation_Mag[k][i] * w[k];
            phase += hrtfsForInterpolation_Phase[k][i] * w[k];
        }
        
        
        if(leftOrRight == L)
        {
            interpolatedHRTF[i][0] = mag * cosf(phase);
            interpolatedHRTF[i][1] = mag * sinf(phase);
        }
        if(leftOrRight == R)
        {
            interpolatedHRTF_R[i][0] = mag * cosf(phase);
            interpolatedHRTF_R[i][1] = mag * sinf(phase);
        }
    }
    
    
//
//    switch(interpolationOrder){
//        case 2:
//
//            //if one distance is zero, no interpolation is done at all
//            if(interpolationDistances[0] == 0 ||interpolationDistances[1] == 0)
//            {
//                w1 = interpolationDistances[0] == 0 ? 1 : 0;
//                w2 = interpolationDistances[1] == 0 ? 1 : 0;
//            }
//            else
//            {
//                w1 = 1.0 / interpolationDistances[0];
//                w2 = 1.0 / interpolationDistances[1];
//            }
//
//            for(int i = 0; i < complexLength * 2; i++)
//            {
//                mag = (hrtfsForInterpolation_Mag[0][i] * w1 + hrtfsForInterpolation_Mag[1][i] * w2) / (w1 + w2);
//                phase = (hrtfsForInterpolation_Phase[0][i] * w1 + hrtfsForInterpolation_Phase[1][i] * w2) / (w1 + w2);
//
//                if(leftOrRight == L)
//                {
//                    interpolatedHRTF[i][0] = mag * cosf(phase);
//                    interpolatedHRTF[i][1] = mag * sinf(phase);
//                }
//                if(leftOrRight == R)
//                {
//                    interpolatedHRTF_R[i][0] = mag * cosf(phase);
//                    interpolatedHRTF_R[i][1] = mag * sinf(phase);
//                }
//            }
//            break;
//
//        case 3:
//
//            //if one distance is zero, no interpolation is done at all
//            if(interpolationDistances[0] == 0 ||interpolationDistances[1] == 0 ||interpolationDistances[2] == 0 )
//            {
//                w1 = interpolationDistances[0] == 0 ? 1 : 0;
//                w2 = interpolationDistances[1] == 0 ? 1 : 0;
//                w3 = interpolationDistances[2] == 0 ? 1 : 0;
//            }
//            else
//            {
//                w1 = 1.0 / interpolationDistances[0];
//                w2 = 1.0 / interpolationDistances[1];
//                w3 = 1.0 / interpolationDistances[2];
//            }
//
//            for(int i = 0; i < complexLength * 2; i++)
//            {
//                mag = (hrtfsForInterpolation_Mag[0][i] * w1 + hrtfsForInterpolation_Mag[1][i] * w2 + hrtfsForInterpolation_Mag[2][i] * w3) / (w1 + w2 + w3);
//                phase = (hrtfsForInterpolation_Phase[0][i] * w1 + hrtfsForInterpolation_Phase[1][i] * w2 + hrtfsForInterpolation_Phase[2][i] * w3) / (w1 + w2 + w3);
//
//                if(leftOrRight == L)
//                {
//                    interpolatedHRTF[i][0] = mag * cosf(phase);
//                    interpolatedHRTF[i][1] = mag * sinf(phase);
//                }
//                if(leftOrRight == R)
//                {
//                    interpolatedHRTF_R[i][0] = mag * cosf(phase);
//                    interpolatedHRTF_R[i][1] = mag * sinf(phase);
//                }
//            }
//
//            break;
//
//        case 4:
//
//            //if one distance is zero, no interpolation is done at all
//            if(interpolationDistances[0] == 0 ||interpolationDistances[1] == 0 ||interpolationDistances[2] == 0 ||interpolationDistances[3] == 0)
//            {
//                w1 = interpolationDistances[0] == 0 ? 1 : 0;
//                w2 = interpolationDistances[1] == 0 ? 1 : 0;
//                w3 = interpolationDistances[2] == 0 ? 1 : 0;
//                w4 = interpolationDistances[2] == 0 ? 1 : 0;
//            }
//            else
//            {
//                w1 = 1.0 / interpolationDistances[0];
//                w2 = 1.0 / interpolationDistances[1];
//                w3 = 1.0 / interpolationDistances[2];
//                w4 = 1.0 / interpolationDistances[3];
//            }
//
//            for(int i = 0; i < complexLength * 2; i++)
//            {
//                mag = (hrtfsForInterpolation_Mag[0][i] * w1 + hrtfsForInterpolation_Mag[1][i] * w2 + hrtfsForInterpolation_Mag[2][i] * w3 + hrtfsForInterpolation_Mag[3][i] * w4) / (w1 + w2 + w3 + w4);
//                phase = (hrtfsForInterpolation_Phase[0][i] * w1 + hrtfsForInterpolation_Phase[1][i] * w2 + hrtfsForInterpolation_Phase[2][i] * w3 + hrtfsForInterpolation_Phase[3][i] * w4) / (w1 + w2 + w3 + w4);
//
//                if(leftOrRight == L)
//                {
//                    interpolatedHRTF[i][0] = mag * cosf(phase);
//                    interpolatedHRTF[i][1] = mag * sinf(phase);
//                }
//                if(leftOrRight == R)
//                {
//                    interpolatedHRTF_R[i][0] = mag * cosf(phase);
//                    interpolatedHRTF_R[i][1] = mag * sinf(phase);
//                }
//            }
//
//            break;
//
//    }
//
    
    
}
