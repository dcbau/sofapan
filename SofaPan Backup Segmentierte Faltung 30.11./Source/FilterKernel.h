/*
  ==============================================================================

    FilterKernel.h
    Created: 13 Oct 2017 1:45:31pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "fftw3.h"
#include "string.h"
#include <stdlib.h>
#include "ErrorHandling.h"

#define RE 0
#define IM 1


class FilterKernel{
public:
    // Processes the input samples in the frequency domain by convolving them with the two filters. The return is a array of floats of size numInputSamples*2, where the left and right output are stored sequentially
    void processBlock(const float* inputSamples, float* outBufferL, float* outBufferR,  fftwf_complex* filter_l, fftwf_complex* filter_r);
    
    FilterKernel();
    ~FilterKernel();
    
    void prepareToPlay(fftwf_complex* initialHRTF, int blockSize, int numBlocks);
    
private:

    int init(int _firPartialLength);

    void releaseResources();
    
    int firLength;
    int numFilterParts;
    int fftLength;
    int complexLength;
    float fftSampleScale;
    
    fftwf_plan forward;
    fftwf_plan inverse_L;
    fftwf_plan inverse_R;
    
    float* fftInputBuffer;
    float* lastfftInputBuffer;
    fftwf_complex* complexBuffer;
    fftwf_complex* src;
    float* fftOutputBuffer_L;
    float* fftOutputBuffer_R;
    float* outBuffer;

    fftwf_complex* previousFilter_l;
    fftwf_complex* previousFilter_r;
    
    fftwf_complex* filterArray_l[16];
    fftwf_complex* filterArray_r[16];

    
    
    
    float* weightingCurve;
    
    fftwf_complex* FDL;
    
};
