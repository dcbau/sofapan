/*
  ==============================================================================

    CombFilter.h
    Created: 9 Aug 2017 12:19:32pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "Delayline.h"

class CombFilter{
public:
    CombFilter(){}
    ~CombFilter(){}
    
    void processBlock(const float* inBuffer, float* outBuffer, int numSamples, float delayInMs, float feedbackGain)
    {
        if(delayLength != delayInMs){
            delayLength = delayInMs;
            delay.setDelayLength(delayLength);
        }
        
        for(int i = 0; i < numSamples; i++){
            float inSample = inBuffer[i];
            float delayLineOutput = delay.pullSample();
            float lpf = delayLineOutput + gLPF*f_z1;
            delay.pushSample(inSample + lpf * feedbackGain);
            if(addOutputMode)
                outBuffer[i] += delayLineOutput;
            else
                outBuffer[i] = delayLineOutput;
            f_z1 = lpf;
        }
        
    }
    
    void processBlock(const float* inBuffer, float* outBuffer, int numSamples)
    {
        for(int i = 0; i < numSamples; i++){
            
            float inSample = inBuffer[i];
            float delayLineOutput = delay.pullSample();
            float lpf = delayLineOutput + gLPF*f_z1;
            delay.pushSample(inSample + lpf * fbGain);
            if(addOutputMode)
                outBuffer[i] += delayLineOutput;
            else
                outBuffer[i] = delayLineOutput;
            f_z1 = lpf;
        }
        
    }
    
    /**
     Setup Comb Values. This is mandatory if the processBlock is called without delay/gain parameters

     @param delayLengthInMs  The time the signal will be delayed, in milliseconds
     @param feedbackGain Gain value in range of -1 to +1 for stability
     */
    void setDelayAndFeedback(float delayLengthInMs, float feedbackGain)
    {
        delayLength = delayLengthInMs;
        fbGain = feedbackGain;
        if(fbGain > 1) fbGain = 0.99;
        if(fbGain < -1) fbGain = -0.99;
                
        delay.setDelayLength(delayLength);
    }
    
    /** By default, the LPF Gain is zero. In this case, the LPF has no effect. By setting a gain value, it will become active */
    void setLPFGain(float filterGain)
    {
        gLPF = filterGain;
    }
    
    void prepareToPlay(int _sampleRate)
    {
        sampleRate = _sampleRate;
        delay.specifyMaxDelayLength(100);
        delay.prepareToPlay(sampleRate);
        f_z1 = 0.0;

    }
    
    /** By default, the buffer passed as outBuffer to processBlock function gets overwritten. By activating this option, the outBuffer remains and the produced samples are added onto them (summing audio). This is helpful for parallel effect chains */
    void setAddOutput(bool activate)
    {
        addOutputMode = activate;
    }
    
    
private:
    Delayline delay;
    float delayLength = 0.0;
    int sampleRate;
    float fbGain = 0.0;
    bool addOutputMode = false;
    
    float f_z1;
    float gLPF = 0.0;
    

};
