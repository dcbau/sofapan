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
#include "BiquadCascade.h"
#include "math.h"

class vector3D{
public:
    
    float x, y, z;
    vector3D(){
        x = 0; y = 0; z = 0;
    }
    vector3D(float _x, float _y, float _z){
        x = _x; y = _y; z = _z;
    }
    
    ~vector3D(){}
};

class wall3D{
public:
    vector3D normVec;
    vector3D footVec;
    
    wall3D(){}
    wall3D(vector3D _normVec, vector3D _footVec): normVec(_normVec), footVec(_footVec) {}
    ~wall3D(){}
};


class MirrorReflections{
public:
    
    MirrorReflections(){
        
        wallDampingFilter.addFilter(bq_type_highshelf, 1000, 0.707, -3);
        wallDampingFilter.addFilter(bq_type_highshelf, 100, 0.707, -1);
        wallDampingFilter.addFilter(bq_type_peak, 1000, 2, -2);
        wallDampingFilter.addFilter(bq_type_peak, 3500, 2, 2);
        //wallDampingFilter.addFilter(bq_type_lowshelf, 10000, 0.707, -2); //to add an overall loss of energy

        wallDispersionFilter.addFilter(bq_type_allpass, 200, 1, 0);
        wallDispersionFilter.addFilter(bq_type_allpass, 200, 1, 0);
        //wallDispersionFilter.addFilter(bq_type_allpass, 200, 1, 0);
        //wallDispersionFilter.addFilter(bq_type_allpass, 200, 1, 0);
//        wallDispersionFilter.addFilter(bq_type_allpass, 500, 2, 0);
//        wallDispersionFilter.addFilter(bq_type_allpass, 500, 2, 0);
//        wallDispersionFilter.addFilter(bq_type_allpass, 500, 2, 0);
//        wallDispersionFilter.addFilter(bq_type_allpass, 500, 2, 0);
        wallDispersionFilter.addFilter(bq_type_highpass, 200, 0.707, 0);

        
    }
    ~MirrorReflections(){}
    
    int initWithSofaData(SOFAData* sD, float roomSize, int sampleRate){
        
        roomRadius = roomSize / 2.0;
        const float roomHeight = 3.0;
        const float receiverHeight = 1.7;

        //roomRadius *= 3.0; //TEST

//        wall[0]= Line<float>(-roomRadius, roomRadius, roomRadius, roomRadius); //Front
//        wall[1]= Line<float>(roomRadius, roomRadius, roomRadius, -roomRadius); //Right
//        wall[2]= Line<float>(roomRadius, -roomRadius, -roomRadius, -roomRadius); //Back
//        wall[3]= Line<float>(-roomRadius, -roomRadius, -roomRadius, roomRadius); //Left

        int status = 0;
        
        for(int i = 0; i < numReflections; i++){
            status += reflections[i].initWithSofaData(sD, sampleRate, i + 10);
        }
        

        wallDampFilter.init(5000.0, sampleRate);

    
        walls.resize(0);
        
        walls.push_back(wall3D(vector3D(-1.0, 0.0, 0.0), vector3D(roomRadius, 0.0, 0.0))); //front wall
        walls.push_back(wall3D(vector3D(0.0, -1.0, 0.0), vector3D(0.0, roomRadius, 0.0))); //right wall
        walls.push_back(wall3D(vector3D(1.0, 0.0, 0.0), vector3D(-roomRadius, 0.0, 0.0))); //back wall
        walls.push_back(wall3D(vector3D(0.0, 1.0, 0.0), vector3D(0.0, -roomRadius, 0.0))); //left wall
        
        numUsedReflections = 4;
        
//        //only use floor and ceiling reflections if there are suitable hrtfs present
//        if(sD->getMetadata().minElevation < -85.0){
//            walls.push_back(wall3D(vector3D(0.0, 0.0, 1.0), vector3D(0.0, 0.0, -receiverHeight))); //floor
//            numUsedReflections++;
//        }
//        if(sD->getMetadata().maxElevation > 85.0){
//            walls.push_back(wall3D(vector3D(0.0, 0.0, -1.0), vector3D(0.0, 0.0, roomHeight - receiverHeight))); //ceiling
//            numUsedReflections++;
//        }
        

        
        printf("\nINIT with %d reflections", numUsedReflections);
        
        //numUsedReflections = 4;
        
//        soundSourceData data;
//        data.azimuth = 90.0;
//        data.elevation = 0.0;
//        data.distance = 2.0;
//        data.ITDAdjust = false;
//        data.nfSimulation = false;
//        data.overwriteOutputBuffer = false;
//
//        soundSourceData mirrorSource = makeMSPosition3D(testWall, data);

        
        
        
        return status;
    }
    
    void prepareToPlay(int sampleRate){
        for(int i = 0; i < numReflections; i++){
            reflections[i].prepareToPlay();
        }
        


        wallDampFilter.prepareToPlay();
        wallDampingFilter.prepareToPlay(sampleRate);
        wallDispersionFilter.prepareToPlay(sampleRate);
    }
    
    void process(const float* inBuffer, float* outBuffer_L, float* outBuffer_R, int numSamples, parameterStruct params){
        
        Point<float> origin = Point<float>(0.0, 0.0);
        float azimuth = 2.0* M_PI * params.azimuthParam->get();
        float distance = params.distanceParam->get();
        Point<float> sourcePos = origin.getPointOnCircumference(distance, azimuth);
        sourcePos.y *= -1.0;
//        printf("\n SOURCE Azimuth: %.2f, Distance: %.2f", azimuth, distance);
//        printf("\n SOURCE X: %.2f, Y: %.2f", sourcePos.getX(), sourcePos.getY());
        
        soundSourceData data;
        data.azimuth = params.azimuthParam->get() * 360.0;
        data.elevation = (params.elevationParam->get()-0.5) * 180.0;
        data.distance = params.distanceParam->get();
        data.ITDAdjust = params.ITDAdjustParam->get();
        data.nfSimulation = params.nearfieldSimulationParam->get();
        data.overwriteOutputBuffer = false;
        data.customHeadRadius = params.individualHeadDiameter->get() / 200.0;

        soundSourceData* firstOrderReflections = new soundSourceData[numUsedReflections];

        //convert refPos to xyz coords point
        float el_rad = data.elevation * d2r;
        float az_rad = data.azimuth * d2r;
        
        sourcePosCartesian.x = cosf(az_rad) * cosf(el_rad) * data.distance;
        sourcePosCartesian.y = sinf(az_rad) * cosf(el_rad) * data.distance;
        sourcePosCartesian.z = data.distance * sinf(el_rad);
//
//        if(counter > 100)  printf("\n");
//
        counter++;

        for(int i = 0; i < numSamples; i++){
            outBuffer_L[i] = outBuffer_R[i] = 0.0;
        }
        for(int i = 0; i < numUsedReflections; i++){
            firstOrderReflections[i] = makeMSPosition3D(walls[i], data);
            reflections[i].process(inBuffer, outBuffer_L, outBuffer_R, numSamples, firstOrderReflections[i]);
            
//            if(counter > 50){
//                printf("\n%d: Az: %.3f, Dst: %.3f", i, firstOrderReflections[i].azimuth, firstOrderReflections[i].distance);
//            }
        }
        if(counter > 50){
            counter = 0;
        }
        wallDampingFilter.processBlockStereo(outBuffer_L, outBuffer_R, numSamples);
        wallDispersionFilter.processBlockStereo(outBuffer_L, outBuffer_R, numSamples);


        
//        else
//        {
//            for(int i = 0; i < 4; i++){
//                reflections[i].process(inBuffer, outBuffer_L, outBuffer_R, numSamples, makeMSPosition(wall[i], data));
//            }
//            wallDampingFilter.processBlockStereo(outBuffer_L, outBuffer_R, numSamples);
//
//        }

        //Apply frequency damping from wall reflection
       // if(params.testSwitchParam->get())
       // else
         //   wallDampFilter.processBlockStereo(outBuffer_L, outBuffer_R, numSamples);
        
        
    }

private:
    
    const float r2d = 180.0 / M_PI; // radians to degrees
    const float d2r = M_PI / 180.0; // degrees to radians
    
    vector3D origin;
    vector3D sourcePosCartesian;
    vector3D imageSourcePos, r;
    float d, lengthOfXYProjection, lengthOfImageSourceVector;
    
    //convention: 0° points towards positive x-axis, 90° points towards positive y-axis
    soundSourceData makeMSPosition3D(wall3D wall, soundSourceData refPos){
        

        
        //make vector between footPoint of wall and sourcePos
        r.x = sourcePosCartesian.x - wall.footVec.x;
        r.y = sourcePosCartesian.y - wall.footVec.y;
        r.z = sourcePosCartesian.z - wall.footVec.z;
        
        //make distance between wall and source (dot product r*n, vorländer p.200 11.33)
        vector3D n = vector3D(wall.normVec);
        d = r.x * n.x + r.y * n.y + r.z * n.z;
        
        //make imagesource position (Si = S - 2d * n, vorländer p.200 11.34)
        imageSourcePos.x = sourcePosCartesian.x - 2 * d * n.x;
        imageSourcePos.y =  sourcePosCartesian.y - 2 * d * n.y;
        imageSourcePos.z = sourcePosCartesian.z - 2 * d * n.z;
        
        
        //transform to spherical coordinates
        lengthOfXYProjection = sqrtf( powf(imageSourcePos.x, 2) + powf(imageSourcePos.y, 2) );
        lengthOfImageSourceVector = sqrtf( powf(lengthOfXYProjection, 2) + powf(imageSourcePos.z, 2) );
        
        float az_rad = atan2f(imageSourcePos.y , imageSourcePos.x); // arctan(y/x) = {-pi ... pi}
        if(az_rad < 0.0) az_rad = 2*M_PI + az_rad; // -> 0 ... 2pi
        
        float el_rad = asinf(imageSourcePos.z / lengthOfImageSourceVector); //arcsin(z / r) = {pi/2 ... -pi/2}
        
        //Make mirrorsource data
        refPos.azimuth = az_rad * r2d;
        refPos.distance = lengthOfImageSourceVector;
//        if(refPos.test){
//            refPos.distance += 2.0;
//            printf("TEST   ");
//        }
        refPos.elevation = el_rad * r2d;
        refPos.ITDAdjust = refPos.ITDAdjust;
        refPos.nfSimulation = false;
        refPos.overwriteOutputBuffer = false;
        
        return refPos;
    }

    const int numReflections = 4;
    int numUsedReflections;
    SoundSource reflections[4];
    float roomRadius;

    //Line<float> wall[6];

    OnePoleLPF wallDampFilter;
    
    BiquadCascade wallDampingFilter;
    BiquadCascade wallDispersionFilter;

    int counter = 0;
    
    std::vector<wall3D> walls;

};
