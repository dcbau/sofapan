/*
  ==============================================================================

    EarlyReflection.cpp
    Created: 27 Jun 2017 12:18:49am
    Author:  David Bau

  ==============================================================================
*/

#include "EarlyReflection.h"

EarlyReflection::EarlyReflection(fftwf_complex* _hrtf, int lengthOfHRIR, int _sampleRate)
:  sampleRate(_sampleRate), firLength(lengthOfHRIR)
{
    //Initialize Variables
    fftLength = firLength * 2;
    complexLength = fftLength / 2 + 1;
    fftSampleScale = 1.0 / (float)fftLength;
    
    //Allocate Memory
    inputBuffer = fftwf_alloc_real(firLength);
    lastInputBuffer = fftwf_alloc_real(firLength);
    outputBuffer_L = fftwf_alloc_real(firLength);
    outputBuffer_R = fftwf_alloc_real(firLength);
    
    fftInputBuffer = fftwf_alloc_real(fftLength);
    complexBuffer = fftwf_alloc_complex(complexLength);
    src = fftwf_alloc_complex(complexLength);
    fftOutputBuffer_L = fftwf_alloc_real(fftLength);
    fftOutputBuffer_R = fftwf_alloc_real(fftLength);

    if(inputBuffer == NULL ||
       lastInputBuffer == NULL ||
       outputBuffer_L == NULL ||
       outputBuffer_R == NULL ||
       fftInputBuffer == NULL ||
       complexBuffer == NULL ||
       src == NULL ||
       fftOutputBuffer_L == NULL ||
       fftOutputBuffer_R == NULL){
        ErrorHandling::reportError("Early Reflection Module", ERRMALLOC, true);
    }
    
    //Init FFTW Plans
    forward = fftwf_plan_dft_r2c_1d(fftLength, fftInputBuffer, complexBuffer, FFTW_ESTIMATE);
    inverse_L = fftwf_plan_dft_c2r_1d(fftLength, complexBuffer, fftOutputBuffer_L, FFTW_ESTIMATE);
    inverse_R = fftwf_plan_dft_c2r_1d(fftLength, complexBuffer, fftOutputBuffer_R, FFTW_ESTIMATE);
    
    
    hrtf_l = fftwf_alloc_complex(complexLength);
    hrtf_r = fftwf_alloc_complex(complexLength);

    for(int i = 0; i < complexLength; i++){
        hrtf_l[i][0] = hrtf_l[i][1] = hrtf_r[i][0] = hrtf_r[i][1] = 0.0;
    }
    for(int i = 0; i < complexLength; i++){
        hrtf_l[i][0] = _hrtf[i][0];
        hrtf_l[i][1] = _hrtf[i][1];
        hrtf_r[i][0] = _hrtf[i+complexLength][0];
        hrtf_r[i][1] = _hrtf[i+complexLength][1];
        
    }
    
    
    //LP coeffs
    float K = tanf(M_PI * cutoff/sampleRate);
    b0 = b1 = K / (K + 1);
    a1 = (K - 1) / (K + 1);
    z1 = 0.0;
    
    prepareToPlay();
}

EarlyReflection::~EarlyReflection(){
    if(inputBuffer != NULL) fftwf_free(inputBuffer);
    fftwf_free(lastInputBuffer);
    fftwf_free(outputBuffer_L);
    fftwf_free(outputBuffer_R);
    fftwf_free(fftInputBuffer);
    fftwf_free(complexBuffer);
    fftwf_free(src);
    fftwf_free(fftOutputBuffer_L);
    fftwf_free(fftOutputBuffer_R);
    fftwf_free(hrtf_l);
    fftwf_free(hrtf_r);
    fftwf_destroy_plan(forward);
    fftwf_destroy_plan(inverse_L);
    fftwf_destroy_plan(inverse_R);
    
    
}

void EarlyReflection::prepareToPlay(){
    
    fifoIndex = 0;
    
    for(int i = 0; i < firLength; i++){
        lastInputBuffer[i] = 0.0;
        outputBuffer_L[i] = 0.0;
        outputBuffer_R[i] = 0.0;
    }
    
    delay.specifyMaxDelayLength(50);
    delay.prepareToPlay(sampleRate);
    
}

void EarlyReflection::process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, float delayInMs, float gain){

    delay.setDelayLength(delayInMs);
    //delay->processBlock(inBuffer, outBuffer_L, numSamples);
    
    for(int sample = 0; sample < numSamples; sample++){

        float delayLineOutput = delay.pullSample();
        delay.pushSample(inBuffer[sample]);

        //Lowpass Filter 
        float v1 =  delayLineOutput - (a1 * z1);
        float lowpassFilterOutput = b0 * v1 + b1 * z1;
        z1 = v1;
        
        //Collect Samples
        inputBuffer[fifoIndex] = lowpassFilterOutput;
        
        //Output of already convoluted Samples
        outBuffer_L[sample] += outputBuffer_L[fifoIndex] * gain;
        outBuffer_R[sample] += outputBuffer_R[fifoIndex] * gain;
        
        
        fifoIndex++;
        
        if(fifoIndex == firLength){
            fifoIndex = 0;
            
            for(int i = 0; i < firLength; i++){
                fftInputBuffer[i] = lastInputBuffer[i];
                fftInputBuffer[i+firLength] = inputBuffer[i];
                
                lastInputBuffer[i] = inputBuffer[i];
            }
            
            
            fftwf_execute(forward);
            
            memcpy(src, complexBuffer, sizeof(fftwf_complex) * complexLength);
            
            // Left Channel
            for ( int k=0; k<complexLength; k++ ) {
                /*  Complex Multiplication: Y = X * H
                 Yr = Xr * Hr - Xi * Hi   |  Yi = Xr * Hi + Xi * Hr */
                complexBuffer[k][RE] = (src[k][RE] * hrtf_l[k][RE] - src[k][IM] * hrtf_l[k][IM]) * fftSampleScale;
                complexBuffer[k][IM] = (src[k][RE] * hrtf_l[k][IM] + src[k][IM] * hrtf_l[k][RE]) * fftSampleScale;
            }
            fftwf_execute(inverse_L);
            
            // Right Channel
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * hrtf_r[k][RE] - src[k][IM] * hrtf_r[k][IM]) * fftSampleScale;
                complexBuffer[k][IM] = (src[k][RE] * hrtf_r[k][IM] + src[k][IM] * hrtf_r[k][RE]) * fftSampleScale;
            }
            fftwf_execute(inverse_R);
            
            for(int i = 0; i < firLength; i++){
                outputBuffer_L[i] = fftOutputBuffer_L[i + firLength];
                outputBuffer_R[i] = fftOutputBuffer_R[i + firLength];
            }
            
            
            
            
        }
    }
    
    
}
