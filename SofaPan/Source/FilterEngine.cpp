/*
  ==============================================================================

    FilterEngine.cpp
    Created: 27 Apr 2017 11:14:15am
    Author:  David Bau

  ==============================================================================
*/
//
#include "FilterEngine.h"
#include "PluginProcessor.h"

FilterEngine::FilterEngine(SOFAData& sD)
    : sofaData(sD), firLength(sD.getLengthOfHRIR())
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
    
    weightingCurve = (float*)malloc(firLength*sizeof(float));
    for(int i = 0; i < firLength; i++){
        float theta = M_PI * 0.5 * (float)i / (float)firLength;
        weightingCurve[i] = cosf(theta)*cosf(theta);
    }
    
    prepareToPlay();
}

FilterEngine::~FilterEngine(){
    fftwf_free(inputBuffer);
    fftwf_free(lastInputBuffer);
    fftwf_free(outputBuffer_L);
    fftwf_free(outputBuffer_R);
    fftwf_free(fftInputBuffer);
    fftwf_free(complexBuffer);
    fftwf_free(src);
    fftwf_free(fftOutputBuffer_L);
    fftwf_free(fftOutputBuffer_R);
    fftwf_destroy_plan(forward);
    fftwf_destroy_plan(inverse_L);
    fftwf_destroy_plan(inverse_R);
    free(weightingCurve);

}

void FilterEngine::prepareToPlay(){
    
    fifoIndex = 0;
    
    for(int i = 0; i < firLength; i++){
        lastInputBuffer[i] = 0.0;
        outputBuffer_L[i] = 0.0;
        outputBuffer_R[i] = 0.0;
    }
    
    previousHRTF_l = sofaData.getHRTFforAngle(0.0, 0.0, 1.0);
    previousHRTF_r = sofaData.getHRTFforAngle(0.0, 0.0, 1.0);

    previousAzimuth = 0.0;
    previousElevation = 0.0;
    
}

void FilterEngine::process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, parameterStruct params){
    
    
    for(int sample = 0; sample < numSamples; sample++){
        
        inputBuffer[fifoIndex] = inBuffer[sample];

        outBuffer_L[sample] = outputBuffer_L[fifoIndex];
        outBuffer_R[sample] = outputBuffer_R[fifoIndex];


        fifoIndex++;
        
        if(fifoIndex == firLength){
            fifoIndex = 0;
            
            for(int i = 0; i < firLength; i++){
                fftInputBuffer[i] = lastInputBuffer[i];
                fftInputBuffer[i+firLength] = inputBuffer[i];
                
                lastInputBuffer[i] = inputBuffer[i];
            }
            
            float azimuth = params.azimuthParam->get() * 360.0;
            float elevation = (params.elevationParam->get()-0.5) * 180.0;
            float distance = params.distanceParam->get();
            
            //printf("AZIMUTH: %f  ##  ", azimuth);
            
            float alpha = params.azimuthParam->get() * 2 * M_PI;
            const float standardHeadradius = 0.18; //if no other headradius is specified
            
            
            /* The simulation of nearfield hrtfs is in an experimental state! It aims to model the acoustic parallax effect. The idea is that if a source approaches the head, the angle between the individual ears and the source will deviate from the orignal angle between the head centre and the source. With simple geometrical methods, the exact angle for every ear can be derivated (=> calculateNFAngleOffset). The used HRTF-pair will then consist of two different left and right HRTFs from different angles
                
                This idea was first discussed by Douglas S. Brungart in his article "AUDITORY PARALLAX EFFECTS IN THE HRTF FOR NEARBY SOURCES", published in 1999 in the Proceedings of the "IEEE Engineering in Medicine and Biology Society", Volume 20(3), pp.1101-1104.
             
                For further information, the dissertation of Fotis Georgiou constains a short introduction to the acoustic parallax effect on pages 15/16. It is available for free on researchgate.net and the title is "Relative Distance Perception Of Sound Sources In Critical Listening Environment Via Binaural Reproduction"
             */
            
            float azimuth_l = azimuth;
            float azimuth_r = azimuth;
            if(params.distanceSimulationParam->get() && distance < 1.0 && params.testSwitchParam->get() == true){
                float angleLeftEar = calculateNFAngleOffset(alpha, distance, standardHeadradius/2);
                float angleRightEar = calculateNFAngleOffset(alpha, distance, -standardHeadradius/2);
                
                azimuth_l = (angleLeftEar / (2*M_PI)) * 360;
                azimuth_r = (angleRightEar / (2*M_PI)) * 360;
                distance = 1.0;
            }
            
            fftwf_complex* hrtf_l = sofaData.getHRTFforAngle(elevation, azimuth_l, distance);
            fftwf_complex* hrtf_r = sofaData.getHRTFforAngle(elevation, azimuth_r, distance);
            
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
                complexBuffer[k][RE] = (src[k][RE] * hrtf_r[k+complexLength][RE] - src[k][IM] * hrtf_r[k+complexLength][IM]) * fftSampleScale;
                complexBuffer[k][IM] = (src[k][RE] * hrtf_r[k+complexLength][IM] + src[k][IM] * hrtf_r[k+complexLength][RE]) * fftSampleScale;
            }
            fftwf_execute(inverse_R);
            
            for(int i = 0; i < firLength; i++){
                outputBuffer_L[i] = fftOutputBuffer_L[i + firLength] * (1.0 - weightingCurve[i]);
                outputBuffer_R[i] = fftOutputBuffer_R[i + firLength] * (1.0 - weightingCurve[i]);
            }

            
            /* The same convolution is done a second time with the HRTF that was used in the last run. Then both results are added together with a certain weighting, resulting in a crossfade between the last convolution result and the newer convolution result. This technique prevents audible clicks, when the HRTF is exchanged and the audio stream would all of a sudden use another filter.
             */
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * previousHRTF_l[k][RE] - src[k][IM] * previousHRTF_l[k][IM]) * fftSampleScale;
                complexBuffer[k][IM] = (src[k][RE] * previousHRTF_l[k][IM] + src[k][IM] * previousHRTF_l[k][RE]) * fftSampleScale;
            }
            fftwf_execute(inverse_L);
            
            for ( int k=0; k<complexLength; k++ ) {
                complexBuffer[k][RE] = (src[k][RE] * previousHRTF_r[k+complexLength][RE] - src[k][IM] * previousHRTF_r[k+complexLength][IM]) * fftSampleScale;
                complexBuffer[k][IM] = (src[k][RE] * previousHRTF_r[k+complexLength][IM] + src[k][IM] * previousHRTF_r[k+complexLength][RE]) * fftSampleScale;
            }
        
            fftwf_execute(inverse_R);
            
            
            for(int i = 0; i < firLength; i++){
                outputBuffer_L[i] += fftOutputBuffer_L[i + firLength] * weightingCurve[i];
                outputBuffer_R[i] += fftOutputBuffer_R[i + firLength] * weightingCurve[i];
            }
            
            previousHRTF_l = hrtf_l;
            previousHRTF_r = hrtf_r;
            previousAzimuth = azimuth;
            previousElevation = elevation;
            
            
            
        }
    }
    
    
}

int FilterEngine::getComplexLength(){
    return complexLength;
}

/**
 angle = angle from headcenter to source in radians
 distance = distance from headcenter to source in m
 earPosition = distance from ear to headcenter in m, positive = left shift, negative = rightshift
 (e.g. 0.09 for left ear, -0.09 for right ear)
 return value = new angle in radians
 */
float FilterEngine::calculateNFAngleOffset(float angle, float r, float earPosition){
    
    float newAngle;
    
    if( angle < M_PI / 2.0)
        newAngle = atan( (sin(angle)*r + earPosition) / (cos(angle)*r) );
    else if(angle < M_PI)
        newAngle = M_PI - atan( (sin(M_PI-angle)*r + earPosition) / (cos(M_PI-angle)*r) );
    else if(angle < M_PI*3/2)
        newAngle = M_PI + atan( (sin(angle)*r + earPosition) / (cos(angle)*r) );
    else if(angle < M_PI * 2)
        newAngle = 2*M_PI - atan( (sin(M_PI-angle)*r + earPosition) / (cos(M_PI-angle)*r) );
    
    if(newAngle < 0)
        newAngle = newAngle + 2*M_PI;
    if(newAngle > 2*M_PI)
        newAngle = newAngle - 2*M_PI;
    
    return newAngle;
    
}
