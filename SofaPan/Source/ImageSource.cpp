///*
//  ==============================================================================
//
//    ImageSource.cpp
//    Created: 18 Aug 2017 9:40:07pm
//    Author:  David Bau
//
//  ==============================================================================
//*/


#include "ImageSource.h"

ImageSource::ImageSource()

{
    inputBuffer = lastInputBuffer = outputBuffer_L = outputBuffer_R = fftInputBuffer = weightingCurve = fftOutputBuffer_L = fftOutputBuffer_R = NULL;
    complexBuffer = src = NULL;
    forward = inverse_L = inverse_R = NULL;
    firLength = 0;
    
    
}

ImageSource::~ImageSource(){
    releaseResources();
}

int ImageSource::initWithSofaData(SOFAData *sD, int _sampleRate, Line<float> _mirrorWall, int _order){
    
    sampleRate = _sampleRate;
    mirrorWall = _mirrorWall;
    order = _order;
    
    sofaData = sD;
    
    if(firLength != sD->getLengthOfHRIR()){
        releaseResources();
        
        firLength = sD->getLengthOfHRIR();
        
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
        
        weightingCurve = (float*)malloc(firLength*sizeof(float));
        
        if(inputBuffer == NULL ||
           lastInputBuffer == NULL ||
           outputBuffer_L == NULL ||
           outputBuffer_R == NULL ||
           fftInputBuffer == NULL ||
           complexBuffer == NULL ||
           src == NULL ||
           fftOutputBuffer_L == NULL ||
           fftOutputBuffer_R == NULL ||
           weightingCurve == NULL){
            
            ErrorHandling::reportError("Early Reflection Module", ERRMALLOC, true);
            return 1;
        }
        
        for(int i = 0; i < firLength; i++){
            float theta = M_PI * 0.5 * (float)i / (float)firLength;
            weightingCurve[i] = cosf(theta)*cosf(theta);
        }
        
    }
    
    prepareToPlay(sampleRate);
    
    return 0;
    
}


void ImageSource::releaseResources(){
    if(inputBuffer!= NULL) fftwf_free(inputBuffer);
    if(lastInputBuffer!= NULL) fftwf_free(lastInputBuffer);
    if(outputBuffer_L!= NULL) fftwf_free(outputBuffer_L);
    if(outputBuffer_R!= NULL) fftwf_free(outputBuffer_R);
    if(fftInputBuffer!= NULL) fftwf_free(fftInputBuffer);
    if(complexBuffer!= NULL) fftwf_free(complexBuffer);
    if(src!= NULL) fftwf_free(src);
    if(fftOutputBuffer_L!= NULL) fftwf_free(fftOutputBuffer_L);
    if(fftOutputBuffer_R!= NULL) fftwf_free(fftOutputBuffer_R);
    if(forward!= NULL) fftwf_destroy_plan(forward);
    if(inverse_L!= NULL) fftwf_destroy_plan(inverse_L);
    if(inverse_R!= NULL) fftwf_destroy_plan(inverse_R);
    if(weightingCurve!= NULL) free(weightingCurve);
    
    inputBuffer = lastInputBuffer = outputBuffer_L = outputBuffer_R = fftInputBuffer = weightingCurve = fftOutputBuffer_L = fftOutputBuffer_R = NULL;
    complexBuffer = src = NULL;
    forward = inverse_L = inverse_R = NULL;
    
}



void ImageSource::prepareToPlay(int _sampleRate){
    
    sampleRate = _sampleRate;
    
    fifoIndex = 0;
    
    for(int i = 0; i < firLength; i++){
        lastInputBuffer[i] = 0.0;
        outputBuffer_L[i] = 0.0;
        outputBuffer_R[i] = 0.0;
    }
    
    delay.specifyMaxDelayLength(50);
    delay.prepareToPlay(sampleRate);
    
    previousHRTF = sofaData->getHRTFforAngle(0.0, 0.0, 1.0);
    
    previousAzimuth = 0.0;
    previousElevation = 0.0;
    
    //LP coeffs
    float K = tanf(M_PI * cutoff/sampleRate);
    b0 = b1 = K / (K + 1);
    a1 = (K - 1) / (K + 1);
    z_1 = z_2 = 0.0;
    
}

/** 
 @param The refSource is the position of the DirectSource or respectively the imageSource of lower order. The coordinate system uses the head position as origin (0|0), meter as units and is has inverted y-Coordinates in respect to the drawing canvas. (Moving upwards the display increases the y-value, like the standart cartesian system) */
void ImageSource::process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, Point<float> refSource){
    
    Point<float> imageSourcePos;
    if(mirrorWall.isVertical()){
        Point<float> mirrorPoint = Point<float>(mirrorWall.getStartX(), refSource.getY());
        imageSourcePos = mirrorPoint.withX(2*mirrorPoint.getX() - refSource.getX());
        
    }
    if(mirrorWall.isHorizontal()){
        Point<float> mirrorPoint = Point<float>(refSource.getX(), mirrorWall.getStartY());
        imageSourcePos = mirrorPoint.withY(2*mirrorPoint.getY() - refSource.getY());
    }
    
    Point<float> origin = Point<float>(0.0,0.0);
    imageSourcePos.y *= -1.0;
    float azimuth = origin.getAngleToPoint(imageSourcePos);
    if (azimuth < 0)
        azimuth = 2.0*M_PI + azimuth;
    azimuth = 180.0 * azimuth / M_PI;
    float distance = origin.getDistanceFrom(imageSourcePos);
    
    //Remove the distance from the source to the head, because the arrival of the direct sound is regarded as t=0 and all other delayed sounds must be delayed respectivly to that moment (and not the actual emitting moment). If the refSource is not the direct source, this is not applicable -> improve method or is the effect negligible for higher order?
    if(order == 1)
        distance -= origin.getDistanceFrom(refSource);
    
    float elevation = 0.0;
    
    
    
    float delayInMs = distance * meterToMs;
    delay.setDelayLength(delayInMs);
    
    float dampGain = 1.0 / distance;
    
    for(int sample = 0; sample < numSamples; sample++){
        
        float delayLineOutput = delay.pullSample();
        delay.pushSample(inBuffer[sample]);
        
        //Lowpass Filter
        float v1 =  delayLineOutput - (a1 * z_1);
        float lowpassFilterOutput = b0 * v1 + b1 * z_1;
        z_1 = v1;
        
        //If order = 2, repeat the filtering for the second wall
        if(order == 2){
            v1 = lowpassFilterOutput - (a1 * z_2);
            lowpassFilterOutput = b0 * v1 + b1 * z_2;
            z_2 = v1;
        }
        
        inputBuffer[fifoIndex] = lowpassFilterOutput * dampGain;
        
        outBuffer_L[sample] += outputBuffer_L[fifoIndex];
        outBuffer_R[sample] += outputBuffer_R[fifoIndex];
        
        
        fifoIndex++;
        
        if(fifoIndex == firLength){
            fifoIndex = 0;
            
            for(int i = 0; i < firLength; i++){
                fftInputBuffer[i] = lastInputBuffer[i];
                fftInputBuffer[i+firLength] = inputBuffer[i];
                
                lastInputBuffer[i] = inputBuffer[i];
            }

            
            fftwf_complex* hrtf = sofaData->getHRTFforAngle(elevation, azimuth, distance);
            
            fftwf_execute(forward);
            
            memcpy(src, complexBuffer, sizeof(fftwf_complex) * complexLength);
            
            // Left Channel
            for ( int k=0; k<complexLength; k++ ) {
                /*  Complex Multiplication: Y = X * H
                 Yr = Xr * Hr - Xi * Hi   |  Yi = Xr * Hi + Xi * Hr */
                complexBuffer[k][RE] = (src[k][RE] * hrtf[k][RE] - src[k][IM] * hrtf[k][IM]) * fftSampleScale;
                complexBuffer[k][IM] = (src[k][RE] * hrtf[k][IM] + src[k][IM] * hrtf[k][RE]) * fftSampleScale;
            }
            fftwf_execute(inverse_L);
            
            // Right Channel
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * hrtf[k+complexLength][RE] - src[k][IM] * hrtf[k+complexLength][IM]) * fftSampleScale;
                complexBuffer[k][IM] = (src[k][RE] * hrtf[k+complexLength][IM] + src[k][IM] * hrtf[k+complexLength][RE]) * fftSampleScale;
            }
            fftwf_execute(inverse_R);
            
            for(int i = 0; i < firLength; i++){
                outputBuffer_L[i] = fftOutputBuffer_L[i + firLength] * (1.0 - weightingCurve[i]);
                outputBuffer_R[i] = fftOutputBuffer_R[i + firLength] * (1.0 - weightingCurve[i]);
            }
            
            
            /* The same convolution is done a second time with the HRTF that was used in the last run. Then both results are added together with a certain weighting, resulting in a crossfade between the last convolution result and the newer convolution result. This technique prevents audible clicks, when the HRTF is exchanged and the audio stream would all of a sudden use another filter.
             */
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * previousHRTF[k][RE] - src[k][IM] * previousHRTF[k][IM]) * fftSampleScale;
                complexBuffer[k][IM] = (src[k][RE] * previousHRTF[k][IM] + src[k][IM] * previousHRTF[k][RE]) * fftSampleScale;
            }
            fftwf_execute(inverse_L);
            
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * previousHRTF[k+complexLength][RE] - src[k][IM] * previousHRTF[k+complexLength][IM]) * fftSampleScale;
                complexBuffer[k][IM] = (src[k][RE] * previousHRTF[k+complexLength][IM] + src[k][IM] * previousHRTF[k+complexLength][RE]) * fftSampleScale;
            }
            
            fftwf_execute(inverse_R);
            
            
            for(int i = 0; i < firLength; i++){
                outputBuffer_L[i] += fftOutputBuffer_L[i + firLength] * weightingCurve[i];
                outputBuffer_R[i] += fftOutputBuffer_R[i + firLength] * weightingCurve[i];
            }
            
            previousHRTF = hrtf;
            previousAzimuth = azimuth;
            previousElevation = elevation;
            
            
            
        }
    }
    
    
}
