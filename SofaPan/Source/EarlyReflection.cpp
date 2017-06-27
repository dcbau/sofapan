/*
  ==============================================================================

    EarlyReflection.cpp
    Created: 27 Jun 2017 12:18:49am
    Author:  David Bau

  ==============================================================================
*/

#include "EarlyReflection.h"

EarlyReflection::EarlyReflection(fftwf_complex* _hrtf, int lengthOfHRIR, int sampleRate)
:  firLength(lengthOfHRIR)
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
    
    
    //Init Delayline
    _sampleRate = sampleRate;
    delayLineLength = (int)(_sampleRate * maxDelayTimeMs * 0.001) + 1;
    delayLine = (float*)malloc(sizeof(float) * delayLineLength);
    
    prepareToPlay();
}

EarlyReflection::~EarlyReflection(){
    fftwf_free(inputBuffer);
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
    free(delayLine);
    
    
}

void EarlyReflection::prepareToPlay(){
    
    fifoIndex = 0;
    
    for(int i = 0; i < firLength; i++){
        lastInputBuffer[i] = 0.0;
        outputBuffer_L[i] = 0.0;
        outputBuffer_R[i] = 0.0;
    }
    
    for(int i = 0; i < delayLineLength; i++)
        delayLine[i] = 0.0;
    
    writeIndex = 0;
}

void EarlyReflection::process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, int delayInMs, float gain){
    
    float offset = delayInMs * _sampleRate * 0.001;
    int offset_int = truncf(offset);
    float offset_frac = offset - (float)offset_int;
    
    int readIndex1, readIndex2;
    
    for(int sample = 0; sample < numSamples; sample++){
        
        
        readIndex1 = writeIndex - offset;
        readIndex2 = readIndex1 - 1;
        while(readIndex1 < 0)
            readIndex1 += delayLineLength;
        while(readIndex2 < 0)
            readIndex2 += delayLineLength;
        

        inputBuffer[fifoIndex] = delayLine[readIndex1] * offset_frac + delayLine[readIndex2]* (1.0 - offset_frac);
        
        delayLine[writeIndex++] = inBuffer[sample];
        if (writeIndex >= delayLineLength)
            writeIndex -= delayLineLength;
        
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
