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
    fftInputBuffer = weightingCurve = fftOutputBuffer_L = fftOutputBuffer_R = NULL;
    complexBuffer = src = NULL;
    forward = inverse_L = inverse_R = NULL;
    firLength = 0;
}

FilterKernel::~FilterKernel(){
    releaseResources();
}


int FilterKernel::init(int _firLength){
    
    printf("\n Kernel Init");
    
    if(firLength != _firLength){
        releaseResources();
        
        firLength = _firLength;
        
        //Initialize Variables
        fftLength = firLength * 2;
        complexLength = fftLength / 2 + 1;
        fftSampleScale = 1.0 / (float)fftLength;
        
        //Allocate Memory
        lastfftInputBuffer = fftwf_alloc_real(firLength);
        
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
           fftInputBuffer == NULL ||
           complexBuffer == NULL ||
           src == NULL ||
           fftOutputBuffer_L == NULL ||
           fftOutputBuffer_R == NULL ||
           weightingCurve == NULL){
            
            ErrorHandling::reportError("Filter Convolution Module", ERRMALLOC, true);
            return 1;
        }
        
        // Create the weighting function for the filter exchange crossfade
        
        int fadeTime_ms = 10;
        //Because FilterKernel does not know about the samplerate, an approximation is sufficient
        int sampleRateApprx = 44100;
        int fadeTime_samples = fadeTime_ms * 0.001 * sampleRateApprx;
        
        if(fadeTime_samples > firLength) fadeTime_samples = firLength;
        
        for(int i = 0; i < firLength; i++){
            if(i < fadeTime_samples)
            {
                //Make theta running from 0 to pi/2
                float theta = M_PI * 0.5 * (float)i / (float)fadeTime_samples;
                //Create Squared Cosine fade (running from 1 to 0)
                weightingCurve[i] = cosf(theta)*cosf(theta);
            }
            else
            {
                weightingCurve[i] = 0.0;
            }
        }
        
    }
    
    return 0;
    
}


void FilterKernel::releaseResources(){
    if(lastfftInputBuffer!= NULL) fftwf_free(lastfftInputBuffer);
    if(fftInputBuffer!= NULL) fftwf_free(fftInputBuffer);
    if(complexBuffer!= NULL) fftwf_free(complexBuffer);
    if(src!= NULL) fftwf_free(src);
    if(fftOutputBuffer_L!= NULL) fftwf_free(fftOutputBuffer_L);
    if(fftOutputBuffer_R!= NULL) fftwf_free(fftOutputBuffer_R);
    if(forward!= NULL) fftwf_destroy_plan(forward);
    if(inverse_L!= NULL) fftwf_destroy_plan(inverse_L);
    if(inverse_R!= NULL) fftwf_destroy_plan(inverse_R);
    if(weightingCurve!= NULL) free(weightingCurve);
    
    lastfftInputBuffer = NULL;
    fftInputBuffer = weightingCurve = fftOutputBuffer_L = fftOutputBuffer_R = NULL;
    complexBuffer = src = NULL;
    forward = inverse_L = inverse_R = NULL;
    
}


//Gets called once prior to all audio processin
void FilterKernel::prepareToPlay(fftwf_complex* initHRTF){
    
    for(int i = 0; i < firLength; i++){
        lastfftInputBuffer[i] = 0.0;
    }
    
    previousFilter_l = initHRTF;
    previousFilter_r = initHRTF;
    
}


void FilterKernel::processBlock(float* inputSamples, float* outBufferL, float* outBufferR, fftwf_complex* filter_l, fftwf_complex* filter_r){
    
    //Prepare the FFT input (Overlap-Save style): First half are the last N samples, second half are the current N samples
    for(int i = 0; i < firLength; i++){
        fftInputBuffer[i] = lastfftInputBuffer[i];
        fftInputBuffer[i+firLength] = inputSamples[i];
        lastfftInputBuffer[i] = inputSamples[i];
    }
    
    //Do FFT
    fftwf_execute(forward);
    
    memcpy(src, complexBuffer, sizeof(fftwf_complex) * complexLength);
    

    
    // LEFT Filtering by complex multiplication with left HRTF
    for ( int k=0; k<complexLength; k++ )
    {
        /*  Complex Multiplication: Yr = Xr * Hr - Xi * Hi   |  Yi = Xr * Hi + Xi * Hr */
        complexBuffer[k][RE] = (src[k][RE] * filter_l[k][RE] - src[k][IM] * filter_l[k][IM]) * fftSampleScale;
        complexBuffer[k][IM] = (src[k][RE] * filter_l[k][IM] + src[k][IM] * filter_l[k][RE]) * fftSampleScale;
    }
    //Do iFFT for left channel
    fftwf_execute(inverse_L);
    
    
    
   // RIGHT Filtering by complex multiplication with right HRTF
    for ( int k=0; k<complexLength; k++ )
    {
        /*  Complex Multiplication: Yr = Xr * Hr - Xi * Hi   |  Yi = Xr * Hi + Xi * Hr */
        complexBuffer[k][RE] = (src[k][RE] * filter_r[k][RE] - src[k][IM] * filter_r[k][IM]) * fftSampleScale;
        complexBuffer[k][IM] = (src[k][RE] * filter_r[k][IM] + src[k][IM] * filter_r[k][RE]) * fftSampleScale;
    }
    //Do iFFT for right channel
    fftwf_execute(inverse_R);
    
    
    //Store result and apply crossfade weighting (Fade-
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
    
    //Store result and apply crossfade weighting
    for(int i = 0; i < firLength; i++){
        outBufferL[i] += fftOutputBuffer_L[i + firLength] * weightingCurve[i];
        outBufferR[i] += fftOutputBuffer_R[i + firLength] * weightingCurve[i];
    }
    
    previousFilter_l = filter_l;
    previousFilter_r = filter_r;
    
    
}

void FilterKernel::processBlockMono(float* inputSamples, float* outBuffer,  fftwf_complex* filter){
    
    for(int i = 0; i < firLength; i++){
        fftInputBuffer[i] = lastfftInputBuffer[i];
        fftInputBuffer[i+firLength] = inputSamples[i];
        lastfftInputBuffer[i] = inputSamples[i];
    }
    
    fftwf_execute(forward);
    
    memcpy(src, complexBuffer, sizeof(fftwf_complex) * complexLength);
    
    for ( int k=0; k<complexLength; k++ ) {
        complexBuffer[k][RE] = (src[k][RE] * filter[k][RE] - src[k][IM] * filter[k][IM]) * fftSampleScale;
        complexBuffer[k][IM] = (src[k][RE] * filter[k][IM] + src[k][IM] * filter[k][RE]) * fftSampleScale;
    }
    fftwf_execute(inverse_L);

    for(int i = 0; i < firLength; i++){
        outBuffer[i] = fftOutputBuffer_L[i + firLength] * (1.0 - weightingCurve[i]);
    }
    
    
    
    for ( int k=0; k<complexLength; k++ ) {
        complexBuffer[k][RE] = (src[k][RE] * previousFilter_l[k][RE] - src[k][IM] * previousFilter_l[k][IM]) * fftSampleScale;
        complexBuffer[k][IM] = (src[k][RE] * previousFilter_l[k][IM] + src[k][IM] * previousFilter_l[k][RE]) * fftSampleScale;
    }
    fftwf_execute(inverse_L);
    
    for(int i = 0; i < firLength; i++){
        outBuffer[i] += fftOutputBuffer_L[i + firLength] * weightingCurve[i];
    }
    
    previousFilter_l = filter;
    
    
}

