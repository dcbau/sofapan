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

class Reverberator{
public:
    Reverberator(){}
    ~Reverberator(){}
    
    /** Mono */
    void processBlockM(const float* inBuffer, float* outBuffer, int numSamples){
        

    }
    /** Mono Input, Stereo Output*/
    void processBlockMS(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, bool testSwitch){
        
//        if(testSwitch){
//            dampingFilter[0].setLPFGain(0.65);
//            dampingFilter[1].setLPFGain(0.5);
//            dampingFilter[2].setLPFGain(0.5);
//        }else{
//            dampingFilter[0].setLPFGain(0.65);
//            dampingFilter[1].setLPFGain(0.6);
//            dampingFilter[2].setLPFGain(0.6);
//        }
        
        
        //Predelay
        preDelay.processBlock(inBuffer, outBuffer_L, numSamples);
        
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
        
        //calc comb gain based on rt60
        for(int i = 0; i < 8; i++){
            combFBGain[i] = calculateCombGain(combDelay[i], rt60);
        }
        //setup combs
        for(int i = 0; i < 8; i++){
            comb[i].prepareToPlay(sampleRate);
            comb[i].setDelayAndFeedback(combDelay[i], combFBGain[i]);
            comb[i].setLPFGain(combLPFGain[i]);
            comb[i].setAddOutput(true);

        }
        
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
    const float rt60 = 0.7;
    CombFilter comb[8];
    const float combDelay[8] = {31.71, 37.11, 40.23, 44.14, 30.47, 33.98, 41.41, 42.58};
    float combFBGain[8];
    float combLPFGain[8] = {0.0, 0.0, 0., 0., 0.0, 0.0, 0., 0.};
    
    DelayingAllpass allpass[4];
    const float apFBGain[4] = {0.7, -0.7, -0.6, 0.6};
    const float apDelay[4] = {13.28, 23.13, 9.38, 11};
    
    Delayline preDelay;
    const float preDelayMs = 5.0;
    
    OnePoleLPF dampingFilter[3];
    const float dampingGain[3] = {0.65, 0.8, 0.8};
    
    float calculateCombGain(float delayTimeMs, float rt)
    {
        return powf(0.001, 0.001*delayTimeMs / rt); //schroeder
    }

    /* Able to handle buffer sizes up to 4096. No elegant solution, but works and is better than dynamically allocating memory in processBlock */
    float intermediateBuffer[4096];
    
};
