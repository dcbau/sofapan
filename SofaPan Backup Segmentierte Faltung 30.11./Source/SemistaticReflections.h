/*
  ==============================================================================

    SemistaticReflections.h
    Created: 16 Oct 2017 10:17:55am
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "SOFAData.h"
#include "EarlyReflection.h"

class SemistaticReflections {
public:
    SemistaticReflections(){};
    ~SemistaticReflections(){}
    
    int init(SOFAData* _sD, float roomSize_m, int sampleRate){
        sofaData = _sD;
        
        int status = 0;
        
        for(int i=0; i < numReflections; i++)
            status += reflections[i].initWithSofaData(sofaData, sampleRate, angles[i], 0.0);
        
        
        roomRadius = roomSize_m / 2.0;
        
        return status;
    }
    
    void prepareToPlay(){
        for(int i=0; i < numReflections; i++)
            reflections[i].prepareToPlay();
    }
    
    
    void process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, parameterStruct params){
        
//        for(int i = 0; i < numSamples; i++){
//            outBuffer_L[i] = 0.0;
//            outBuffer_R[i] = 0.0;
//        }
        
        float alpha_s = sinf(params.azimuthParam->get() * 2.0 * M_PI); //Running from 0 -> 1 -> 0 -> -1 -> 0 for a full circle
        float alpha_c = cosf(params.azimuthParam->get() * 2.0 * M_PI); //Running from 1 -> 0 -> -1 -> 0 -> 1 for a full circle
        float delay[4], damp[4];
        
        float distance = 1.0;
        if(params.distanceSimulationParam || sofaData->getMetadata().hasMultipleDistances){
            distance = params.distanceParam->get();
        }
        //clip distance, to avoid negative delay values. More a dirty solution, because in the edges of a 10x10m room, where the distance is larger than 5m, there is no change in the reflections anymore. This might be neglible, because the reflections are only an approximation anyway
        if(distance > roomRadius){
            distance = roomRadius;
        }
        
        delay[0] = meterToMs * ((2.0 * roomRadius - distance) - alpha_s * distance);
        delay[1] = meterToMs * ((2.0 * roomRadius - distance) + alpha_s * distance);
        delay[2] = meterToMs * ((2.0 * roomRadius - distance) - alpha_c * distance);
        delay[3] = meterToMs * ((2.0 * roomRadius - distance) + alpha_c * distance);
        
        damp[0] = 1.0/(2*roomRadius-alpha_s * distance);
        damp[1] = 1.0/(2*roomRadius+alpha_s * distance);
        damp[2] = 1.0/(2*roomRadius-alpha_c * distance);
        damp[3] = 1.0/(2*roomRadius+alpha_c * distance);
        
        for(int i=0; i < 4; i++){
            reflections[i].process(inBuffer, outBuffer_L, outBuffer_R, numSamples, delay[i], damp[i], params);
        }
    }

private:
    const int numReflections = 4;
    EarlyReflection reflections[4];
    
    SOFAData* sofaData;
    float roomRadius;
    
    const float speedOfSound = 343.2;
    const float meterToMs = 1000.0 / speedOfSound;
    
    const float angles[4] = {90, 270, 0, 180}; //right, left, front, back

    
};
