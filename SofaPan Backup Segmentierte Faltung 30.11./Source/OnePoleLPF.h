/*
  ==============================================================================

    OnePoleLPF.h
    Created: 9 Aug 2017 11:49:08pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "math.h"

class StupidOnePoleLPF{
public:
    StupidOnePoleLPF(){}
    ~StupidOnePoleLPF(){}
    
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






class OnePoleLPF{
public:
    OnePoleLPF(){}
    ~OnePoleLPF(){}
    
    void processBlock(float* IOBuffer, int numSamples){
        float v1;
        for(int i = 0; i < numSamples; i++){
            v1 =  IOBuffer[i] - (a1 * z_1);
            IOBuffer[i] = b0 * v1 + b1 * z_1;
            z_1 = v1;
        }
    }
    
    void processBlockStereo(float* IOBuffer_L, float* IOBuffer_R, int numSamples){
        float v1;
        for(int i = 0; i < numSamples; i++){
            v1 =  IOBuffer_L[i] - (a1 * z_1);
            IOBuffer_L[i] = b0 * v1 + b1 * z_1;
            z_1 = v1;
            
            v1 =  IOBuffer_R[i] - (a1 * z_2);
            IOBuffer_R[i] = b0 * v1 + b1 * z_2;
            z_2 = v1;
        }
    }
    
    void init(float cutoffFrequency, int sampleRate){
        float K = tanf(M_PI * cutoffFrequency/(float)sampleRate);
        b0 = b1 = K / (K + 1);
        a1 = (K - 1) / (K + 1);
        z_1 = z_2 = 0.0;
    }
    
    void prepareToPlay(){
        z_1 = z_2 = 0.0;
    }
    
    
private:
    
    bool addOutputMode = false;
    
    float z_1, z_2;
    float b0, b1, a1 = 0.0;
    
    
};
