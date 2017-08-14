/*
  ==============================================================================

    DelayingAllpass.h
    Created: 9 Aug 2017 12:19:17pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once


#include "Delayline.h"

class DelayingAllpass{
public:
    DelayingAllpass(){}
    ~DelayingAllpass(){}
    
    void processBlock(const float* inBuffer, float* outBuffer, int numSamples, float delayInMs, float feedbackGain){
        
        if(delayLength != delayInMs){
            delayLength = delayInMs;
            delay.setDelayLength(delayLength);
        }
        
        for(int i = 0; i < numSamples; i++){
            
            float inSample = inBuffer[i];
            float delayLineOutput = delay.pullSample();
            float xDL = inSample + delayLineOutput * feedbackGain;
            delay.pushSample(xDL);
            if(addOutputMode)
                outBuffer[i] += delayLineOutput + (-feedbackGain * xDL);
            else
                outBuffer[i] = delayLineOutput + (-feedbackGain * xDL);

        }
        
    }
    
    void processBlock(const float* inBuffer, float* outBuffer, int numSamples){
        
        for(int i = 0; i < numSamples; i++){
            
            float inSample = inBuffer[i];
            float delayLineOutput = delay.pullSample();
            float xDL = inSample + delayLineOutput * fbGain;
            delay.pushSample(xDL);
            if(addOutputMode)
                outBuffer[i] += delayLineOutput + (-fbGain * xDL);
            else
                outBuffer[i] = delayLineOutput + (-fbGain * xDL);
        }
        
    }
    /**
     Setup Allpass Values. This is mandatory if the processBlock is called without delay/gain parameters
     
     @param delayLengthInMs  The time the signal will be delayed, in milliseconds
     @param feedbackGain Gain value in range of -1 to +1 for stability
     */
    void setDelayAndFeedback(float delayLengthInMs, float feedbackGain){
        delayLength = delayLengthInMs;
        fbGain = feedbackGain;
//        if(fbGain > 1) fbGain = 0.99;
//        if(fbGain < -1) fbGain = -0.99;

        
        delay.setDelayLength(delayLength);
    }
    
    void prepareToPlay(int _sampleRate){
        
        sampleRate = _sampleRate;
        delay.specifyMaxDelayLength(100);
        delay.prepareToPlay(sampleRate);
        delayLength = 0.0;
        fbGain = 0.0;
    }
    
    /** By default, the buffer passed as outBuffer to process function gets overwritten. By activating this option, the outBuffer remains and the produced samples are added onto them (summing audio). This is helpful for parallel effect chains */
    void setAddOutput(bool activate){
        addOutputMode = activate;
    }
    
    
    
private:
    Delayline delay;
    float delayLength;
    int sampleRate;
    float fbGain;
    bool addOutputMode = false;
    
};
