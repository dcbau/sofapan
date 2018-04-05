/*
 ==============================================================================
 
 FilterKernelOld.cpp
 Created: 13 Oct 2017 1:55:40pm
 Author:  David Bau
 
 ==============================================================================
 */

#include "FilterKernelOld.h"

FilterKernelOld::FilterKernelOld(){
    
    lastfftInputBuffer = NULL;
    outBuffer = NULL;
    fftInputBuffer = weightingCurve = fftOutputBuffer_L = fftOutputBuffer_R = NULL;
    complexBuffer = src = NULL;
    forward = inverse_L = inverse_R = NULL;
    firLength = 0;
}

FilterKernelOld::~FilterKernelOld(){
    releaseResources();
}


int FilterKernelOld::init(int _firLength){
    
    
    if(firLength != _firLength){
        releaseResources();
        
        firLength = _firLength;
        
        //Initialize Variables
        fftLength = firLength * 2;
        complexLength = fftLength / 2 + 1;
        fftSampleScale = 1.0 / (float)fftLength;
        
        //Allocate Memory
        lastfftInputBuffer = fftwf_alloc_real(firLength);
        outBuffer = fftwf_alloc_real(fftLength);
        
        fftInputBuffer = fftwf_alloc_real(fftLength);
        complexBuffer = fftwf_alloc_complex(complexLength);
        src = fftwf_alloc_complex(complexLength);
        fftOutputBuffer_L = fftwf_alloc_real(fftLength);
        fftOutputBuffer_R = fftwf_alloc_real(fftLength);
        
        //Init FFTW Plans
        forward = fftwf_plan_dft_r2c_1d(fftLength, fftInputBuffer, complexBuffer, FFTW_ESTIMATE);
        inverse_L = fftwf_plan_dft_c2r_1d(fftLength, complexBuffer, fftOutputBuffer_L, FFTW_ESTIMATE);
        inverse_R = fftwf_plan_dft_c2r_1d(fftLength, complexBuffer, fftOutputBuffer_R, FFTW_ESTIMATE);
        
        weightingCurve = (float*)malloc(firLength*sizeof(float));
        
        if(lastfftInputBuffer == NULL ||
           outBuffer == NULL ||
           fftInputBuffer == NULL ||
           complexBuffer == NULL ||
           src == NULL ||
           fftOutputBuffer_L == NULL ||
           fftOutputBuffer_R == NULL ||
           weightingCurve == NULL){
            
            ErrorHandling::reportError("Filter Convolution Module", ERRMALLOC, true);
            return 1;
        }
        
        int fadeTime_ms = 10;
        int sampleRateApprx = 44100;
        int fadeTime_samples = fadeTime_ms * 0.001 * sampleRateApprx;
        fadeTime_samples = 10;
        //printf("\n fadeTime Samples: %d", fadeTime_samples);
        if(fadeTime_samples > firLength) fadeTime_samples = firLength;
        for(int i = 0; i < firLength; i++){
            float theta = M_PI * 0.5 * (float)i / (float)fadeTime_samples;
            if(i < fadeTime_samples)
                weightingCurve[i] = cosf(theta)*cosf(theta);
            else{
                weightingCurve[i] = 0.0;
            }
            //printf("\nWeighting Curve %d: %.3f", i, weightingCurve[i]);
        }
        
    }
    
    return 0;
    
}


void FilterKernelOld::releaseResources(){
    if(lastfftInputBuffer!= NULL) fftwf_free(lastfftInputBuffer);
    if(fftInputBuffer!= NULL) fftwf_free(fftInputBuffer);
    if(complexBuffer!= NULL) fftwf_free(complexBuffer);
    if(outBuffer != NULL) fftwf_free(outBuffer);
    if(src!= NULL) fftwf_free(src);
    if(fftOutputBuffer_L!= NULL) fftwf_free(fftOutputBuffer_L);
    if(fftOutputBuffer_R!= NULL) fftwf_free(fftOutputBuffer_R);
    if(forward!= NULL) fftwf_destroy_plan(forward);
    if(inverse_L!= NULL) fftwf_destroy_plan(inverse_L);
    if(inverse_R!= NULL) fftwf_destroy_plan(inverse_R);
    if(weightingCurve!= NULL) free(weightingCurve);
    
    lastfftInputBuffer = NULL;
    outBuffer = NULL;
    fftInputBuffer = weightingCurve = fftOutputBuffer_L = fftOutputBuffer_R = NULL;
    complexBuffer = src = NULL;
    forward = inverse_L = inverse_R = NULL;
    
}



void FilterKernelOld::prepareToPlay(fftwf_complex* initHRTF){
    
    for(int i = 0; i < firLength; i++){
        lastfftInputBuffer[i] = 0.0;
        outBuffer[i] = 0.0;
        outBuffer[i+firLength] = 0.0;
    }
    
    previousFilter_l = initHRTF;
    previousFilter_r = initHRTF;
    
}


void FilterKernelOld::processBlock(float* inputSamples, float* outBufferL, float* outBufferR, fftwf_complex* filter_l, fftwf_complex* filter_r){
    
    for(int i = 0; i < firLength; i++){
        fftInputBuffer[i] = lastfftInputBuffer[i];
        fftInputBuffer[i+firLength] = inputSamples[i];
        lastfftInputBuffer[i] = inputSamples[i];
    }
    
    fftwf_execute(forward);
    
    memcpy(src, complexBuffer, sizeof(fftwf_complex) * complexLength);
    
    // Left Channel
    for ( int k=0; k<complexLength; k++ ) {
        /*  Complex Multiplication: Y = X * H
         Yr = Xr * Hr - Xi * Hi   |  Yi = Xr * Hi + Xi * Hr */
        complexBuffer[k][RE] = (src[k][RE] * filter_l[k][RE] - src[k][IM] * filter_l[k][IM]) * fftSampleScale;
        complexBuffer[k][IM] = (src[k][RE] * filter_l[k][IM] + src[k][IM] * filter_l[k][RE]) * fftSampleScale;
    }
    fftwf_execute(inverse_L);
    
    // Right Channel
    for ( int k=0; k<complexLength; k++ ) {
        complexBuffer[k][RE] = (src[k][RE] * filter_r[k][RE] - src[k][IM] * filter_r[k][IM]) * fftSampleScale;
        complexBuffer[k][IM] = (src[k][RE] * filter_r[k][IM] + src[k][IM] * filter_r[k][RE]) * fftSampleScale;
    }
    fftwf_execute(inverse_R);
    
    for(int i = 0; i < firLength; i++){
        outBufferL[i] = fftOutputBuffer_L[i + firLength] * (1.0 - weightingCurve[i]);
        outBufferR[i] = fftOutputBuffer_R[i + firLength] * (1.0 - weightingCurve[i]);
    }
    
    
    /* The same convolution is done a second time with the HRTF that was used in the last run. Then both results are added together with a certain weighting, resulting in a crossfade between the last convolution result and the newer convolution result. This technique prevents audible clicks, when the HRTF is exchanged and the audio stream would all of a sudden use another filter.
     */
    for ( int k=0; k<complexLength; k++ ) {
        complexBuffer[k][RE] = (src[k][RE] * previousFilter_l[k][RE] - src[k][IM] * previousFilter_l[k][IM]) * fftSampleScale;
        complexBuffer[k][IM] = (src[k][RE] * previousFilter_l[k][IM] + src[k][IM] * previousFilter_l[k][RE]) * fftSampleScale;
    }
    fftwf_execute(inverse_L);
    
    for ( int k=0; k<complexLength; k++ ) {
        complexBuffer[k][RE] = (src[k][RE] * previousFilter_r[k][RE] - src[k][IM] * previousFilter_r[k][IM]) * fftSampleScale;
        complexBuffer[k][IM] = (src[k][RE] * previousFilter_r[k][IM] + src[k][IM] * previousFilter_r[k][RE]) * fftSampleScale;
    }
    
    fftwf_execute(inverse_R);
    
    
    for(int i = 0; i < firLength; i++){
        outBufferL[i] += fftOutputBuffer_L[i + firLength] * weightingCurve[i];
        outBufferR[i] += fftOutputBuffer_R[i + firLength] * weightingCurve[i];
    }
    
    previousFilter_l = filter_l;
    previousFilter_r = filter_r;
    
    
}


