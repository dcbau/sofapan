/*
  ==============================================================================

    OnePoleLPF.h
    Created: 9 Aug 2017 11:49:08pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

class OnePoleLPF{
public:
    OnePoleLPF(){}
    ~OnePoleLPF(){}
    
    void processBlock(const float* inBuffer, float* outBuffer, int numSamples)
    {
        for(int i = 0; i < numSamples; i++){
            float lpf = (1.0-gLPF)*inBuffer[i] + gLPF*f_z1;
            if(addOutputMode)
                outBuffer[i] += lpf;
            else
                outBuffer[i] = lpf;
            f_z1 = lpf;
        }
    }
    
    float processSample(float inSample)
    {
        float lpf = (1-gLPF)*inSample + gLPF*f_z1;
        f_z1 = lpf;
        
        return lpf;
    }
    
    
    void setLPFGain(float filterGain)
    {
        gLPF = filterGain;
    }
    
    void prepareToPlay(int _sampleRate)
    {

        f_z1 = 0.0;
        
    }
    
    /** By default, the buffer passed as outBuffer to processBlock function gets overwritten. By activating this option, the outBuffer remains and the produced samples are added onto them (summing audio). This is helpful for parallel effect chains */
    void setAddOutput(bool activate)
    {
        addOutputMode = activate;
    }
    
    
private:
    
    bool addOutputMode = false;
    
    float f_z1;
    float gLPF = 0.0;
    
    
};
