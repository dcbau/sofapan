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
    filterOutBufferL = filterOutBufferR = inputBuffer  = nullptr;
    interpolatedHRTF = nullptr;
    interpolatedHRTF_R = nullptr;
    firLength = 0;
    
    
    
}

SoundSource::~SoundSource(){
    releaseResources();
}

int SoundSource::initWithSofaData(SOFAData *sD, int _sampleRate, int _index){
    
    //the index identifies the soundsource, because multiple soundsources are instantiated from the pluginprocessor
    index = _index;
    
    sofaData = sD;
    sampleRate = _sampleRate;
    
    filter.init(sD->getLengthOfHRIR());
    
    //reallocate the buffers that depend on the hrir, if the length of the HRIR (which influences the length of almost everything) changes
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

        if(inputBuffer == nullptr || filterOutBufferL == nullptr || filterOutBufferR == nullptr){
            ErrorHandling::reportError("Direct Source Module", ERRMALLOC, true);
            return 1;
        }
    }
    
    previousAzimuth = 0;
    previousElevation = 0;
    previousDistance = 1.0;
    previousITDAdjust = false;
    
    //Initialize with any hrtf, so no convolution is done with a empty memory
    hrtf_l = sofaData->getHRTFforAngle(0.0, 0.0, 1.0, hrtf_type_pseudoMinPhase);
    hrtf_r = sofaData->getHRTFforAngle(0.0, 0.0, 1.0, hrtf_type_pseudoMinPhase);
    
    //determine the interpolation ordner (minimum 2) acoording to the SOFA Data
    interpolationOrder = 2;
    if(sofaData->getMetadata().hasMultipleDistances)
        interpolationOrder++;
    if(sofaData->getMetadata().hasElevation)
        interpolationOrder++;
    
    //printf("\n\n Interpolation Order: %d \n\n", interpolationOrder);
    
    distanceDelaySmoother.reset((double)sampleRate, 0.5);
    distanceGainSmoother.reset((double)sampleRate, 0.5);
    ITDDelaySmootherL.reset((double)sampleRate, 0.02);
    ITDDelaySmootherR.reset((double)sampleRate, 0.02);
    
    
    azimuthSmoothed.reset( ((float)sampleRate / (float)firLength) , 0.1);
    elevationSmoothed.reset( ((float)sampleRate / (float)firLength) , 0.1);

    filter.prepareToPlay(sofaData->getHRTFforAngle(0.0, 0.0, 1.0, hrtf_type_pseudoMinPhase));
    
    return 0;
}


void SoundSource::releaseResources(){
    if(inputBuffer!= nullptr) fftwf_free(inputBuffer);
    if(filterOutBufferL != nullptr) fftwf_free(filterOutBufferL);
    if(filterOutBufferR != nullptr) fftwf_free(filterOutBufferR);
    if(interpolatedHRTF != nullptr) fftwf_free(interpolatedHRTF);
    if(interpolatedHRTF_R != nullptr) fftwf_free(interpolatedHRTF_R);

    filterOutBufferL = filterOutBufferR = inputBuffer = nullptr;
    interpolatedHRTF = nullptr;
    interpolatedHRTF_R = nullptr;
    
}


//Gets called once prior to all audio processin
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
    

    //tell the smoother objects the current values. if one of the values is needed, the smoother takes care that there are no abrupt jumps
    distanceDelaySmoother.setValue(data.distance * meterToMs);
    distanceGainSmoother.setValue(1.0 / data.distance);

    azimuthSmoothed.setValue(data.azimuth);
    elevationSmoothed.setValue(data.elevation);
    
    for(int sample = 0; sample < numSamples; sample++){
        
        //Gather samples for the convolution
        inputBuffer[fifoIndex] = inBuffer[sample];
        
        //Output of samples coming from the convolution
        //output of the delaylines left & right, delayed by the sum distance delay and the individual itd delay for left and right
        float smoothedDistanceDelay = distanceDelaySmoother.getNextValue();
        float delayLineOutputL = ITDdelayL.pullSample(ITDDelaySmootherL.getNextValue() + smoothedDistanceDelay);//smoothDelay.update(distanceDelay));
        ITDdelayL.pushSample(filterOutBufferL[fifoIndex]);
        float delayLineOutputR = ITDdelayR.pullSample(ITDDelaySmootherR.getNextValue() + smoothedDistanceDelay);//smoothDelay.update(distanceDelay));
        ITDdelayR.pushSample(filterOutBufferR[fifoIndex]);
        
        //apply the damping according to the distance from source to listener
        float g = distanceGainSmoother.getNextValue();
        if(data.overwriteOutputBuffer){
            outBuffer_L[sample] = delayLineOutputL * g;
            outBuffer_R[sample] = delayLineOutputR * g;
        }else{
            outBuffer_L[sample] += delayLineOutputL * g;
            outBuffer_R[sample] += delayLineOutputR * g;
        }
        
        fifoIndex++;
        
        //if buffer is full, trigger the convolution process
        if(fifoIndex == firLength){
            fifoIndex = 0;


            data.azimuth = azimuthSmoothed.getNextValue();
            data.elevation = elevationSmoothed.getNextValue();
            
            
            //update Distance Delay and ITD Delay
            delayL_ms = delayR_ms =0.0;
            
            float ITD = 0.0;
            if(data.ITDAdjust){
                ITD = ITDToolkit::woodworthSphericalITD(data.customHeadRadius, data.azimuth, data.elevation);
            }

            //delay left or right channel, depending on wether the ITD is positive or negative
            if(ITD > 0.0)
                delayL_ms = ITD;
            else
                delayR_ms = fabsf(ITD);
            
            //to avoid retriggers caused by small calculation errors
            if(fabsf(ITDDelaySmootherL.getTargetValue() - delayL_ms) > 0.00001)
                ITDDelaySmootherL.setValue(delayL_ms);
            if(fabsf(ITDDelaySmootherR.getTargetValue() - delayR_ms) > 0.00001)
                ITDDelaySmootherR.setValue(delayR_ms);

            
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
            //interpolation
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
    
    //Do the actual convolution!
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
            //interpolation
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
    
    
    //Do the actual convolution!
    filter.processBlock(inputBuffer, filterOutBufferL, filterOutBufferR, hrtf_l, hrtf_r + complexLength);
    
    
    float gainL = NFSimToolkit::getAdaptedIIDGain(data.distance, data.azimuth, data.elevation, 0);
    float gainR = NFSimToolkit::getAdaptedIIDGain(data.distance, data.azimuth, data.elevation, 1);
    for(int i = 0; i < firLength; i++){
        filterOutBufferL[i] *= gainL;
        filterOutBufferR[i] *= gainR;
    }
    
}


void SoundSource::interpolation(int leftOrRight){

    float mag, phase;

	float *w = new float[interpolationOrder];
    
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
    
    
    
}
