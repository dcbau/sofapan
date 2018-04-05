/*
  ==============================================================================

    SVF.h
    Created: 11 Jan 2018 9:18:23pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once


#include "math.h"

#define L 0
#define R 1

enum {
    svf_type_lowpass = 0,
    svf_type_highpass,
    svf_type_bandpass,
    svf_type_peak
};

typedef struct{
    int filterType;
    float cutoffFrequency;
    float Q;
    float peakGain;
} sfvFilterSpecs;

class SVF{
public:
    SVF(){}
    
    SVF(int filterType){
        type = filterType;
    }
    
    
    ~SVF(){}
    

    void processBlock(float* IOBuffer, int numSamples, float freq, float Q, float gain){
        
        /* sanity checking */
        if (freq < 20.0 ) freq = 20;
        if (freq > 20000.0) freq = 20000.0;
        
        //calculate the tuning parameter
        float f1 = 2*sinf(M_PI * freq / (float)sampleRate);
        
        if (Q < 0.5 ) Q = 0.5;
        if (Q > 50.0) Q = 50.0;
        
        Q = 1/Q;
        
        //filter stability
//        if(f1 < (2-Q))
        
        
        float vHP, vBP, vLP;
        for(int i = 0; i < numSamples; i++){
 
            /* calculate the first stage and therefore simultaneously the Highpass-output. The first stage is only the input plus the feedback path */
            vHP = IOBuffer[i] - z_2[L] - Q * z_1[L];
            
            /* calculate the second stage / bandpass-output */
            vBP = f1 * vHP   + z_1[L];
            
            /* calculate the last stage / lowpass-output */
            vLP = f1 * vBP   + z_2[L];
            
            switch(type){
                case svf_type_lowpass:
                    IOBuffer[i] = vLP; break;
                case svf_type_highpass:
                    IOBuffer[i] = vHP; break;
                case svf_type_bandpass:
                    IOBuffer[i] = vBP; break;
                case svf_type_peak:
                    IOBuffer[i] += vBP * gain; break;
                default:
                    IOBuffer[i] = 0.0; break;
            }
            
            /* store the z-elements */
            z_2[L] = vLP;
            z_1[L] = vBP;
        }
    }
    
    void processBlock(const float* inBuffer, float* outBuffer, int numSamples, float freq, float Q, float gain){
        
        /* sanity checking */
        if (freq < 20.0 ) freq = 20;
        if (freq > 20000.0) freq = 20000.0;
        
        //calculate the tuning parameter
        float f1 = 2*sinf(M_PI * freq / (float)sampleRate);
        
        if (Q < 0.5 ) Q = 0.5;
        if (Q > 50.0) Q = 50.0;
        
        Q = 1/Q;
        
        //filter stability
        //        if(f1 < (2-Q))
        
        
        float vHP, vBP, vLP;
        for(int i = 0; i < numSamples; i++){
            
            /* calculate the first stage and therefore simultaneously the Highpass-output. The first stage is only the input plus the feedback path */
            vHP = inBuffer[i] - z_2[L] - Q * z_1[L];
            
            /* calculate the second stage / bandpass-output */
            vBP = f1 * vHP   + z_1[L];
            
            /* calculate the last stage / lowpass-output */
            vLP = f1 * vBP   + z_2[L];
            
            switch(type){
                case svf_type_lowpass:
                    outBuffer[i] = vLP; break;
                case svf_type_highpass:
                    outBuffer[i] = vHP; break;
                case svf_type_bandpass:
                    outBuffer[i] = vBP; break;
                case svf_type_peak:
                    outBuffer[i] = inBuffer[i] + vBP * gain; break;
                default:
                    outBuffer[i] = 0.0; break;
            }
            
            /* store the z-elements */
            z_2[L] = vLP;
            z_1[L] = vBP;
        }
    }
    
//    void processBlockStereo(float* IOBuffer_L, float* IOBuffer_R, int numSamples){
//        float v1;
//        for(int i = 0; i < numSamples; i++){
//            v1 =  IOBuffer_L[i] - (a1 * z_1[L]) - (a2 * z_2[L]);
//            IOBuffer_L[i] = b0 * v1 + b1 * z_1[L] + b2 * z_2[L];
//            z_2[L] = z_1[L];
//            z_1[L] = v1;
//
//            v1 =  IOBuffer_R[i] - (a1 * z_1[R]) - (a2 * z_2[R]);
//            IOBuffer_R[i] = b0 * v1 + b1 * z_1[R] + b2 * z_2[R];
//            z_2[R] = z_1[R];
//            z_1[R] = v1;
//        }
//    }
//
    void init(int filterType){
        type = filterType;
    }
    
    void prepareToPlay(int _sampleRate){
        
        if(_sampleRate != sampleRate){
            sampleRate = _sampleRate;
        }
        
        z_1[L] = z_1[R] = z_2[L] = z_2[R] = 0.0;
    }
    

    
private:
    
    double z_1[2], z_2[2];
    int sampleRate = 0;
    int type = 0;
    

    
    
};
