/*
  ==============================================================================

    Reverberator.h
    Created: 9 Aug 2017 3:04:04pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "CombFilter.h"
#include "DelayingAllpass.h"
#include "Delayline.h"
#include "OnePoleLPF.h"
#include "ParameterStruct.h"

class Reverberator{
public:
    Reverberator(){}
    ~Reverberator(){}
    

    /** Mono Input, Stereo Output*/
    void processBlockMS(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, parameterStruct params){
        
        float reverbLevel = params.reverbParam1->get() * 36.0 - 48;
        float reverbTime = params.reverbParam2->get() * 1.9 + 0.1;
        //printf("\n Reverb Level: %f, Decay Time: %f", reverbLevel, rt60);

        if(rt60 != reverbTime){
            rt60 = reverbTime;
            configCombGains(rt60);
        }
        
        float revGain = Decibels::decibelsToGain(reverbLevel);
        
        for(int i = 0; i < numSamples; i++){
            outBuffer_L[i] = inBuffer[i] * revGain;
        }

        
        //Predelay
        preDelay.processBlock(outBuffer_L, outBuffer_L, numSamples);

        
        //Input Diffusion
        dampingFilter[0].processBlock(outBuffer_L, outBuffer_L, numSamples);
        allpass[0].processBlock(outBuffer_L, outBuffer_L, numSamples);
        allpass[1].processBlock(outBuffer_L, outBuffer_L, numSamples);
        
        //prepare for parallel processing
        for(int i = 0; i < numSamples; i++){
            intermediateBuffer[i] = outBuffer_L[i];
            outBuffer_L[i] = 0.0;
            outBuffer_R[i] = 0.0;
        }
        
        //Stereo Comb Section
        for(int i = 0; i < 4; i++){
            comb[i].processBlock(intermediateBuffer, outBuffer_L, numSamples);
            comb[i+4].processBlock(intermediateBuffer, outBuffer_R, numSamples);
        }
        
//        //Left Diffusion
        dampingFilter[1].processBlock(outBuffer_L, outBuffer_L, numSamples);
        allpass[2].processBlock(outBuffer_L, outBuffer_L, numSamples);
//        
//        //Right Diffusion
        dampingFilter[2].processBlock(outBuffer_R, outBuffer_R, numSamples);
        allpass[3].processBlock(outBuffer_R, outBuffer_R, numSamples);
        
        

        
        
    }
    void prepareToPlay(int _sampleRate){
        sampleRate = _sampleRate;
        

        //setup combs
        for(int i = 0; i < 8; i++){
            comb[i].prepareToPlay(sampleRate);
            comb[i].setAddOutput(true);
        }
        
        //calc comb gain based on rt60
        configCombGains(rt60);
        
        //setup allps
        for(int i = 0; i < 4; i++){
            allpass[i].prepareToPlay(sampleRate);
            allpass[i].setDelayAndFeedback(apDelay[i], apFBGain[i]);
        }
        
        //setup diffusion filters
        for(int i = 0; i < 3; i++){
            dampingFilter[i].prepareToPlay(sampleRate);
            dampingFilter[i].setLPFGain(dampingGain[i]);
        }
        
        //setup predelay
        preDelay.prepareToPlay(sampleRate);
        preDelay.specifyMaxDelayLength(preDelayMs + 10.0);
        preDelay.setDelayLength(preDelayMs);
    }
    void setReverbParams(){
        //UNUSED
    }
private:
    int sampleRate;
    float rt60 = 1.0; //inital 1s
    CombFilter comb[8];
    const float combDelay[8] = {31.71f, 37.11f, 40.23f, 44.14f, 30.47f, 33.98f, 41.41f, 42.58f};
//    const float combDelay[8] = {31.71, 37.11, 40.23, 44.14, 30.47, 33.98, 41.41, 42.58};

    float combFBGain[8];
    float combLPFGain[8] = {0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f};
    
    DelayingAllpass allpass[4];
    const float apFBGain[4] = {0.7f, -0.7f, -0.6f, 0.6f};
    const float apDelay[4] = {13.28f, 23.13f, 9.38f, 11.f};
    
    Delayline preDelay;
    const float preDelayMs = 5.0;
    
    StupidOnePoleLPF dampingFilter[3];
    //1: input Damping, 2+3: output damping
    const float dampingGain[3] = {0.55f, 0.75f, 0.8f};
    
//    const float revGain = 0.05;
    
    
    
    void configCombGains(float rt)
    {
        for(int i = 0; i < 8; i++)
        {
            combFBGain[i] = powf(0.001, 0.001*combDelay[i] / rt); //schroeder
            comb[i].setDelayAndFeedback(combDelay[i], combFBGain[i]);
            
            comb[i].setLPFGain(combLPFGain[i]);
        }
    }

    /* Able to handle buffer sizes up to 4096. No elegant solution, but works and is better than dynamically allocating memory in processBlock */
    float intermediateBuffer[4096];
    
};
