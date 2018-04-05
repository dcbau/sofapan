/*
  ==============================================================================

    Biquad.h
    Created: 12 Dec 2017 4:03:50pm
    Author:  David Bau

  ==============================================================================


 
 Direct Form II Biquad
 
 */

#pragma once

#include "math.h"

#define L 0
#define R 1

enum {
    bq_type_lowpass = 0,
    bq_type_highpass,
    bq_type_bandpass,
    bq_type_notch,
    bq_type_peak,
    bq_type_lowshelf,
    bq_type_highshelf,
    bq_type_allpass
};

typedef struct{
    int filterType;
    float cutoffFrequency;
    float Q;
    float peakGain;
} filterSpecs;

class Biquad{
public:
    Biquad(){}
    
    Biquad(int filterType, float cutoffFrequency, float _Q, float gain){
        type = filterType;
        cutoff = cutoffFrequency;
        Q = _Q;
        peakGain = gain;
    }
    
    Biquad(filterSpecs specs){
        type = specs.filterType;
        cutoff = specs.cutoffFrequency;
        Q = specs.Q;
        peakGain = specs.peakGain;
    }
    
    ~Biquad(){}
    
    void processBlock(float* IOBuffer, int numSamples){
        float v1;
        for(int i = 0; i < numSamples; i++){
            v1 =  IOBuffer[i] - (a1 * z_1[L]) - (a2 * z_2[L]);
            IOBuffer[i] = b0 * v1 + b1 * z_1[L] + b2 * z_2[L];
            z_2[L] = z_1[L];
            z_1[L] = v1;
        }
    }
    
    void processBlockStereo(float* IOBuffer_L, float* IOBuffer_R, int numSamples){
        float v1;
        for(int i = 0; i < numSamples; i++){
            v1 =  IOBuffer_L[i] - (a1 * z_1[L]) - (a2 * z_2[L]);
            IOBuffer_L[i] = b0 * v1 + b1 * z_1[L] + b2 * z_2[L];
            z_2[L] = z_1[L];
            z_1[L] = v1;
            
            v1 =  IOBuffer_R[i] - (a1 * z_1[R]) - (a2 * z_2[R]);
            IOBuffer_R[i] = b0 * v1 + b1 * z_1[R] + b2 * z_2[R];
            z_2[R] = z_1[R];
            z_1[R] = v1;
        }
    }
    
    void init(int filterType, float cutoffFrequency, float _Q, float gain){
        type = filterType;
        cutoff = cutoffFrequency;
        Q = _Q;
        peakGain = gain;
    }
    
    void init(filterSpecs specs){
        type = specs.filterType;
        cutoff = specs.cutoffFrequency;
        Q = specs.Q;
        peakGain = specs.peakGain;
    }
    
    void prepareToPlay(int _sampleRate){

        if(_sampleRate != sampleRate){
            sampleRate = _sampleRate;
            calcBiquad();
        }

        z_1[L] = z_1[R] = z_2[L] = z_2[R] = 0.0;
    }
    
    filterSpecs getFilterSpecs(){
        
        filterSpecs specs;
        
        specs.filterType = type;
        specs.cutoffFrequency = cutoff;
        specs.Q = Q;
        specs.peakGain = peakGain;
        
        return specs;
        
    }
    
private:

    double z_1[2], z_2[2];
    double b0, b1, b2, a1, a2  = 0.0;
    int sampleRate = 0;
    int type;
    float cutoff;
    float Q;
    float peakGain;
    
    void calcBiquad() {
        double G;
        double V = pow(10, fabs(peakGain) / 20.0);
        double K = tan(M_PI * cutoff/(double)sampleRate);
        switch (type) {
            case bq_type_lowpass:
                G = 1 / (1 + K / Q + K * K);
                b0 = K * K * G;
                b1 = 2 * b0;
                b2 = b0;
                a1 = 2 * (K * K - 1) * G;
                a2 = (1 - K / Q + K * K) * G;
                break;
                
            case bq_type_highpass:
                G = 1 / (1 + K / Q + K * K);
                b0 = 1 * G;
                b1 = -2 * b0;
                b2 = b0;
                a1 = 2 * (K * K - 1) * G;
                a2 = (1 - K / Q + K * K) * G;
                break;
                
            case bq_type_bandpass:
                G = 1 / (1 + K / Q + K * K);
                b0 = K / Q * G;
                b1 = 0;
                b2 = -b0;
                a1 = 2 * (K * K - 1) * G;
                a2 = (1 - K / Q + K * K) * G;
                break;
                
            case bq_type_notch:
                G = 1 / (1 + K / Q + K * K);
                b0 = (1 + K * K) * G;
                b1 = 2 * (K * K - 1) * G;
                b2 = b0;
                a1 = b1;
                a2 = (1 - K / Q + K * K) * G;
                break;
                
            case bq_type_peak:
                if (peakGain >= 0) {    // boost
                    G = 1 / (1 + 1/Q * K + K * K);
                    b0 = (1 + V/Q * K + K * K) * G;
                    b1 = 2 * (K * K - 1) * G;
                    b2 = (1 - V/Q * K + K * K) * G;
                    a1 = b1;
                    a2 = (1 - 1/Q * K + K * K) * G;
                }
                else {    // cut
                    G = 1 / (1 + V/Q * K + K * K);
                    b0 = (1 + 1/Q * K + K * K) * G;
                    b1 = 2 * (K * K - 1) * G;
                    b2 = (1 - 1/Q * K + K * K) * G;
                    a1 = b1;
                    a2 = (1 - V/Q * K + K * K) * G;
                }
                break;
            case bq_type_lowshelf:
                if (peakGain >= 0) {    // boost
                    G = 1 / (1 + sqrt(2) * K + K * K);
                    b0 = (1 + sqrt(2*V) * K + V * K * K) * G;
                    b1 = 2 * (V * K * K - 1) * G;
                    b2 = (1 - sqrt(2*V) * K + V * K * K) * G;
                    a1 = 2 * (K * K - 1) * G;
                    a2 = (1 - sqrt(2) * K + K * K) * G;
                }
                else {    // cut
                    G = 1 / (1 + sqrt(2*V) * K + V * K * K);
                    b0 = (1 + sqrt(2) * K + K * K) * G;
                    b1 = 2 * (K * K - 1) * G;
                    b2 = (1 - sqrt(2) * K + K * K) * G;
                    a1 = 2 * (V * K * K - 1) * G;
                    a2 = (1 - sqrt(2*V) * K + V * K * K) * G;
                }
                break;
            case bq_type_highshelf:
                if (peakGain >= 0) {    // boost
                    G = 1 / (1 + sqrt(2) * K + K * K);
                    b0 = (V + sqrt(2*V) * K + K * K) * G;
                    b1 = 2 * (K * K - V) * G;
                    b2 = (V - sqrt(2*V) * K + K * K) * G;
                    a1 = 2 * (K * K - 1) * G;
                    a2 = (1 - sqrt(2) * K + K * K) * G;
                }
                else {    // cut
                    G = 1 / (V + sqrt(2*V) * K + K * K);
                    b0 = (1 + sqrt(2) * K + K * K) * G;
                    b1 = 2 * (K * K - 1) * G;
                    b2 = (1 - sqrt(2) * K + K * K) * G;
                    a1 = 2 * (K * K - V) * G;
                    a2 = (V - sqrt(2*V) * K + K * K) * G;
                }
                break;
                
            case bq_type_allpass:
                G = 1 / (Q + K  + Q * K * K);
                b0 = (Q * K * K - K + Q) * G;
                b1 = (2 * Q * (K * K - 1)) * G;
                b2 = 1;
                a1 = b1;
                a2 = b0;
                printf("\n%.20f %.20f %.20f 1 %.20f %.20f\n", b0, b1, b2, a1, a2);
                break;
                
        }
        return;
    }
    
    
};
