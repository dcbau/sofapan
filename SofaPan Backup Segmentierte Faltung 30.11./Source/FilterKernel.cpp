/*
  ==============================================================================

    FilterKernel.cpp
    Created: 13 Oct 2017 1:55:40pm
    Author:  David Bau

  ==============================================================================
*/

#include "FilterKernel.h"

FilterKernel::FilterKernel(){
    
    lastfftInputBuffer = NULL;
    outBuffer = NULL;
    fftInputBuffer = weightingCurve = fftOutputBuffer_L = fftOutputBuffer_R = NULL;
    complexBuffer = src = NULL;
    forward = inverse_L = inverse_R = NULL;
    firLength = 0;
    FDL = NULL;
}

FilterKernel::~FilterKernel(){
    releaseResources();
}


int FilterKernel::init(int _firPartialLength){
    
    releaseResources();
    
    firLength = _firPartialLength;
    
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
    FDL = fftwf_alloc_complex(complexLength * numFilterParts);
    
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
        
    
        
    return 0;
    
}


void FilterKernel::releaseResources(){
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
    if(FDL != NULL) fftwf_free(FDL);
    
    lastfftInputBuffer = NULL;
    outBuffer = NULL;
    fftInputBuffer = weightingCurve = fftOutputBuffer_L = fftOutputBuffer_R = NULL;
    complexBuffer = src = FDL = NULL;
    forward = inverse_L = inverse_R = NULL;
    
}



void FilterKernel::prepareToPlay(fftwf_complex* initialHRTF, int blockSize, int numBlocks){
    
    numFilterParts = numBlocks;
    
    if(firLength != blockSize){
        firLength = blockSize;
        init(firLength);
    }
        
    for(int i = 0; i < firLength; i++){
        lastfftInputBuffer[i] = 0.0;
        outBuffer[i] = 0.0;
        outBuffer[i+firLength] = 0.0;
    }
    
    for(int i = 0; i < complexLength * numFilterParts; i++){
        FDL[i][RE] = 0.0;
        FDL[i][IM] = 0.0;
    }
    
    previousFilter_l = initialHRTF;
    previousFilter_r = initialHRTF;
    

    
    for(int i = 0; i < numFilterParts; i++){
        filterArray_l[i] = initialHRTF;
        filterArray_r[i] = initialHRTF;
    }
    
}


void FilterKernel::processBlock(const float* inputSamples, float* outBufferL, float* outBufferR, fftwf_complex* filter_l, fftwf_complex* filter_r){
    
    printf("\n numFilterParts: %d", numFilterParts);
    
    for(int i = 0; i < firLength; i++){
        fftInputBuffer[i] = lastfftInputBuffer[i];
        fftInputBuffer[i+firLength] = inputSamples[i];
        lastfftInputBuffer[i] = inputSamples[i];
    }
    
    fftwf_execute(forward);
    
    //Shift contents of FDL by one slot
    for(int i = numFilterParts - 1; i > 0; i--){
        memcpy(FDL + i * complexLength, FDL + (i - 1) * complexLength, sizeof(fftwf_complex) * complexLength);
    }
    //Store new transformed input in FDL slot 1
    memcpy(FDL, complexBuffer, sizeof(fftwf_complex) * complexLength);
    
    //Shift contents of FitlerDL by one slot
    for(int i = numFilterParts - 1; i > 0; i--){
        filterArray_l[i] = filterArray_l[i-1];//filterArray_l[i-1];
        filterArray_r[i] = filterArray_r[i-1];// filterArray_r[i-1];
    }
    //Store newest HRTF in slot 1
    filterArray_l[0] = filter_l;
    filterArray_r[0] = filter_r;
    
    
    int n;
    
    // Left Channel
    for(int k = 0; k < complexLength; k++){
        complexBuffer[k][RE] = 0.0;
        complexBuffer[k][IM] = 0.0;
    }
    for(int i = 0; i < numFilterParts; i++){
        for ( int k=0; k<complexLength; k++ ) {
            /*  Complex Multiplication: Y = X * H
             Yr = Xr * Hr - Xi * Hi   |  Yi = Xr * Hi + Xi * Hr */
            n = k + i * complexLength;
            complexBuffer[k][RE] += (FDL[n][RE] * filterArray_l[i][n][RE] - FDL[n][IM] * filterArray_l[i][n][IM]) * fftSampleScale;
            complexBuffer[k][IM] += (FDL[n][RE] * filterArray_l[i][n][IM] + FDL[n][IM] * filterArray_l[i][n][RE]) * fftSampleScale;
        }
    }

    fftwf_execute(inverse_L);
    
    // Right Channel
    for(int k = 0; k < complexLength; k++){
        complexBuffer[k][RE] = 0.0;
        complexBuffer[k][IM] = 0.0;
    }
    for(int i = 0; i < numFilterParts; i++){
        for ( int k=0; k<complexLength; k++ ) {
            n = k + i * complexLength;
            complexBuffer[k][RE] += (FDL[n][RE] * filterArray_r[i][n][RE] - FDL[n][IM] * filterArray_r[i][n][IM]) * fftSampleScale;
            complexBuffer[k][IM] += (FDL[n][RE] * filterArray_r[i][n][IM] + FDL[n][IM] * filterArray_r[i][n][RE]) * fftSampleScale;
        }
    }
    fftwf_execute(inverse_R);
    
    for(int i = 0; i < firLength; i++){
        outBufferL[i] = fftOutputBuffer_L[i + firLength];// * (1.0 - weightingCurve[i]);
        outBufferR[i] = fftOutputBuffer_R[i + firLength];// * (1.0 - weightingCurve[i]);
    }

    
    
//    /* The same convolution is done a second time with the HRTF that was used in the last run. Then both results are added together with a certain weighting, resulting in a crossfade between the last convolution result and the newer convolution result. This technique prevents audible clicks, when the HRTF is exchanged and the audio stream would all of a sudden use another filter.
//     */
//    // Left Channel
//    for(int k = 0; k < complexLength; k++){
//        complexBuffer[k][RE] = 0.0;
//        complexBuffer[k][IM] = 0.0;
//    }
//    for(int i = 0; i < numFilterParts; i++){
//        for ( int k=0; k<complexLength; k++ ) {
//            /*  Complex Multiplication: Y = X * H
//             Yr = Xr * Hr - Xi * Hi   |  Yi = Xr * Hi + Xi * Hr */
//            n = k + i * complexLength;
//            complexBuffer[k][RE] += (FDL[n][RE] * previousFilter_l[n][RE] - FDL[n][IM] * previousFilter_l[n][IM]) * fftSampleScale;
//            complexBuffer[k][IM] += (FDL[n][RE] * previousFilter_l[n][IM] + FDL[n][IM] * previousFilter_l[n][RE]) * fftSampleScale;
//        }
//    }
//
//    fftwf_execute(inverse_L);
//
//    // Right Channel
//    for(int k = 0; k < complexLength; k++){
//        complexBuffer[k][RE] = 0.0;
//        complexBuffer[k][IM] = 0.0;
//    }
//    for(int i = 0; i < numFilterParts; i++){
//        for ( int k=0; k<complexLength; k++ ) {
//            n = k + i * complexLength;
//            complexBuffer[k][RE] += (FDL[n][RE] * previousFilter_r[n][RE] - FDL[n][IM] * previousFilter_r[n][IM]) * fftSampleScale;
//            complexBuffer[k][IM] += (FDL[n][RE] * previousFilter_r[n][IM] + FDL[n][IM] * previousFilter_r[n][RE]) * fftSampleScale;
//        }
//    }
//    fftwf_execute(inverse_R);
//
//
//    for(int i = 0; i < firLength; i++){
//        outBufferL[i] += fftOutputBuffer_L[i + firLength] * weightingCurve[i];
//        outBufferR[i] += fftOutputBuffer_R[i + firLength] * weightingCurve[i];
//    }
//
//    previousFilter_l = filter_l;
//    previousFilter_r = filter_r;
    
    
}

