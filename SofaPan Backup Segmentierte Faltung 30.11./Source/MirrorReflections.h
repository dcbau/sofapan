/*
  ==============================================================================

    MirrorReflections.h
    Created: 16 Oct 2017 4:15:22pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "SoundSource.h"
#include "OnePoleLPF.h"

class MirrorReflections{
public:
    
    MirrorReflections(){}
    ~MirrorReflections(){}
    
    int initWithSofaData(SOFAData* sD, float roomSize, int sampleRate){
        
//        roomRadius = roomSize / 2.0;
//
//        //roomRadius *= 3.0; //TEST
//
//        wall[0]= Line<float>(-roomRadius, roomRadius, roomRadius, roomRadius); //Front
//        wall[1]= Line<float>(roomRadius, roomRadius, roomRadius, -roomRadius); //Right
//        wall[2]= Line<float>(roomRadius, -roomRadius, -roomRadius, -roomRadius); //Back
//        wall[3]= Line<float>(-roomRadius, -roomRadius, -roomRadius, roomRadius); //Left
//
//        int status = 0;
//        for(int i = 0; i < numReflections; i++){
//            status += reflections[i].initWithSofaData(sD, sampleRate);
//        }
//
//        wallDampFilter.init(3000.0, sampleRate);
//
//        return status;
    }
    
    void prepareToPlay(){
//        for(int i = 0; i < numReflections; i++){
//            reflections[i].prepareToPlay();
//        }
//
//        wallDampFilter.prepareToPlay();
    }
    
    void process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, parameterStruct params){
//                
//        Point<float> origin = Point<float>(0.0, 0.0);
//        float azimuth = 2.0* M_PI * params.azimuthParam->get();
//        float distance = params.distanceParam->get();
//        Point<float> sourcePos = origin.getPointOnCircumference(distance, azimuth);
//        sourcePos.y *= -1.0;
////        printf("\n SOURCE Azimuth: %.2f, Distance: %.2f", azimuth, distance);
////        printf("\n SOURCE X: %.2f, Y: %.2f", sourcePos.getX(), sourcePos.getY());
//        
//        soundSourceData data;
//        data.azimuth = params.azimuthParam->get() * 360.0;
//        data.elevation = (params.elevationParam->get()-0.5) * 180.0;
//        data.distance = params.distanceParam->get();
//        data.ITDAdjust = params.ITDAdjustParam->get();
//        data.nfSimulation = params.nearfieldSimulationParam->get();
//        for(int i = 0; i < numReflections; i++){
//            reflections[i].process(inBuffer, outBuffer_L, outBuffer_R, numSamples, makeMSPosition(wall[i], data));
//        }
//
//        //Apply frequency damping from wall reflection
//        //if(params.testSwitchParam->get())
//            wallDampFilter.processBlockStereo(outBuffer_L, outBuffer_R, numSamples);
    }

private:

    soundSourceData makeMSPosition(Line<float> wall, soundSourceData refPos){
        soundSourceData mirrorSourcePos;
        
        Point<float> origin = Point<float>(0.0, 0.0);
        Point<float> sourcePos = origin.getPointOnCircumference(refPos.distance, refPos.azimuth * M_PI / 180.0);
        sourcePos.y *= -1.0;
        
        Point<float> imageSourcePos;
        if(wall.isVertical()){
            Point<float> mirrorPoint = Point<float>(wall.getStartX(), sourcePos.getY());
            imageSourcePos = mirrorPoint.withX(2*mirrorPoint.getX() - sourcePos.getX());
        }
        if(wall.isHorizontal()){
            Point<float> mirrorPoint = Point<float>(sourcePos.getX(), wall.getStartY());
            imageSourcePos = mirrorPoint.withY(2*mirrorPoint.getY() - sourcePos.getY());
        }
        
        imageSourcePos.y *= -1.0;
        float azimuth = origin.getAngleToPoint(imageSourcePos);
        if (azimuth < 0)
            azimuth = 2.0*M_PI + azimuth;
        
        mirrorSourcePos.azimuth = 180.0 * azimuth / M_PI;
        mirrorSourcePos.distance = origin.getDistanceFrom(imageSourcePos);
        mirrorSourcePos.elevation = 0.0;
        mirrorSourcePos.ITDAdjust = refPos.ITDAdjust;
        mirrorSourcePos.nfSimulation = false;
        mirrorSourcePos.overwriteOutputBuffer = false;
        
        //printf("\nAzimuth: %.3f, Distance: %.3f", mirrorSourcePos.azimuth, mirrorSourcePos.distance);
        
        return mirrorSourcePos;
    }

    const int numReflections = 4;
    SoundSource reflections[4];
    float roomRadius;

    Line<float> wall[4];

    OnePoleLPF wallDampFilter;

};
