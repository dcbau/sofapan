/*
 ==============================================================================
 
 SOFAData.h
 Created: 4 Apr 2017 11:47:12am
 Author:  David Bau
 
 ==============================================================================
 */

#ifndef SOFAData_H_INCLUDED
#define SOFAData_H_INCLUDED

#include "fftw3.h"
extern "C" {
#include <netcdf.h>
}
#include "string.h"
#include <stdlib.h>
#include "math.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "ErrorHandling.h"
#include "ITDToolkit.h"
#include "MinPhaseGenerator.h"

#define ERR_MEM_ALLOC   1
#define ERR_READFILE    2
#define ERR_UNKNOWN     3
#define ERR_OPENFILE    4
#define ERR_NOTSUP      5

#define M_PI 3.14159265358979323846




enum {
    hrtf_type_original = 0,
    hrtf_type_pseudoMinPhase,
    hrtf_type_minPhase,
};

enum {
    hrir_type_original = 0,
    hrir_type_pseudoMinPhase,
    hrir_type_minPhase,
};


typedef struct{
    
    int sampleRate;
    int numMeasurements;
    int numMeasurements0ev;
    int numSamples;
    int numReceivers;
    String dataType;
    String SOFAConventions;
    String listenerShortName;
    float minElevation;
    float maxElevation;
    float minDistance;
    float maxDistance;
    bool hasMultipleDistances;
    bool hasElevation;
    float receiverPosition[2];
    float headRadius;
    int numDifferentDistances;
    
    int numberOfGlobalAttributes;
    //char** globalAttributeNames;
    //char** globalAttributeValues;
    Array<String> globalAttributeNames;
    Array<String> globalAttributeValues;
    
    
    
} sofaMetadataStruct;


typedef struct{
    int onsetL_samples, onsetR_samples;
    float ITD_ms;
    
    int onsetL_samples_2, onsetR_samples_2;
    float ITD_ms_2;

} ITDStruct;


class Single_HRIR_Measurement {
public:
    Single_HRIR_Measurement(int lengthHRIR, int lengthHRTF){
        HRIR = fftwf_alloc_real(lengthHRIR * 2);
        HRIRPseudoMinPhase = fftwf_alloc_real(lengthHRIR * 2);
        HRIRMinPhase = fftwf_alloc_real(lengthHRIR * 2);

        HRTF = fftwf_alloc_complex(lengthHRTF * 2);
        HRTFPseudoMinPhase = fftwf_alloc_complex(lengthHRTF * 2);
        HRTFMinPhase = fftwf_alloc_complex(lengthHRTF * 2);

        magSpectrum = (float*)malloc(lengthHRTF * 2 * sizeof(float));
        phaseSpectrum = (float*)malloc(lengthHRTF * 2 * sizeof(float));
        phaseSpectrumMinPhase = (float*)malloc(lengthHRTF * 2 * sizeof(float));

        
    }
    ~Single_HRIR_Measurement(){
        fftwf_free(HRIR); //not allocated when freed
        fftwf_free(HRIRPseudoMinPhase); //not allocated when freed
        fftwf_free(HRIRMinPhase); //not allocated when freed

        fftwf_free(HRTF);
        fftwf_free(HRTFPseudoMinPhase);
        fftwf_free(HRTFMinPhase);

        free(magSpectrum);
        free(phaseSpectrum);
        free(phaseSpectrumMinPhase);

        
    }
    void setValues(float azimuth, float elevation, float distance){
        Elevation = elevation; Azimuth = azimuth; Distance = distance;
        
        float d2r = M_PI / 180.0;
        
        x = distance * cosf(Elevation * d2r) * cosf(Azimuth * d2r);
        y = distance * cosf(Elevation * d2r) * sinf(Azimuth * d2r);
        z = distance * sinf(Elevation * d2r);
    }
    float *getHRIR(){return  HRIR;}
    float *getHRIRPseudoMinPhase(){return  HRIRPseudoMinPhase;}
    float *getHRIRMinPhase(){return  HRIRMinPhase;}

    fftwf_complex* getHRTF(){return HRTF;}
    fftwf_complex* getHRTFPseudoMinPhase(){return HRTFPseudoMinPhase;}
    fftwf_complex* getHRTFMinPhase(){return HRTFMinPhase;}

    float *getMagSpectrum(){return magSpectrum;}
    float *getPhaseSpectrum(){return phaseSpectrum;}
    float *getPhaseSpectrumMinPhase(){return phaseSpectrumMinPhase;}

    
    float Elevation;
    float Azimuth;
    float Distance;
    float x, y, z;
    int index;
    
    ITDStruct ITD;


    
protected:
    float* HRIR;
    float* HRIRPseudoMinPhase;
    float* HRIRMinPhase;
    fftwf_complex* HRTF;
    fftwf_complex* HRTFPseudoMinPhase;
    fftwf_complex* HRTFMinPhase;
    float* magSpectrum; //used for plotting
    float* phaseSpectrum; //used for plotting
    float* phaseSpectrumMinPhase;
    
    
};

//class Single_MinPhase_HRIR_Measurement: public Single_HRIR_Measurement {
//public:
//    Single_MinPhase_HRIR_Measurement(int lengthHRIR, int lengthHRTF): Single_HRIR_Measurement(lengthHRIR, lengthHRTF){
//    }
//    ~Single_MinPhase_HRIR_Measurement(){
//    }
//
//    //float ITD_ms;
//    ITDStruct ITD;
//};





class SOFAData{
public:
    SOFAData();
    ~SOFAData();
    
    int initSofaData(const char* filePath, int sampleRate);
    
    /** Will directly return the length of the new resampled HRTF */
    int setSampleRate(int newSampleRate);
    
    /**
     Get the closest HRTF for a given elevation & azimuth. The function will return a pointer to the TF-Values (complex numbers): the left TF first, followed directly by the right TF. The returned HRTF has a length of 2*(HRIRlength/2 + 1): [ L=N/2+1 | R=N/2+1 ].
     */
    fftwf_complex* getHRTFforAngle(float elevation, float azimuth, float radius, int version);
    float* getHRIRforAngle(float elevation, float azimuth, float radius, int version);

    ITDStruct getITDForAngle(float elevation, float azimuth, float radius);
    
    void getHRTFsForInterpolation(float** resultsMag, float** resultsPhase, float* distances, float elevation, float azimuth, float radius, int numDesiredHRTFs);


    float* getInterpolatedMagSpectrumForAngle(float elevation, float azimuth, float radius);
    float* getPhaseSpectrumForAngle(float elevation, float azimuth, float radius, int version);

    
    sofaMetadataStruct getMetadata();
    
    /** Returns the length of the interpolated(!) Impulse Response in samples */
    int getLengthOfHRIR();
    
private:
    /** Number of Samples in the samplerate-converted HRIRs */
    int lengthOfHRIR;
    /** Has to be double the size of HRIR (due to linear convolution) */
    int lengthOfFFT;
    /** Number of Frequency-Bins FFTLength/2 + 1: will be the length of the samplerate-converted HRTFs */
    int lengthOfHRTF;
    
    int searchHRTF(float elevation, float azimuth, float radius);
    void searchClosestHRTFs(int* results, float* distances, int desiredClosestPoints, float elevation, float azimuth, float radius);
    float* interpolatedReturnMagSpectrum;
    float last_elevation, last_azimuth, last_radius;

    fftwf_complex* theStoredHRTFs;
    int lengthOfHRTFStorage;
    int loadSofaFile(const char* filePath, int hostSampleRate);
    
    Single_HRIR_Measurement** loadedHRIRs;
//    Single_MinPhase_HRIR_Measurement** loadedMinPhaseHRIRs;
//    Single_MinPhase_HRIR_Measurement** loadedMinPhaseHRIRsVERSION;

    Single_HRIR_Measurement* testMeasurement;
    
    void errorHandling(int status);
    String errorDesc;
    void createPassThrough_FIR(int _sampleRate);
    
    sofaMetadataStruct sofaMetadata;
    
    String getSOFAGlobalAttribute(const char* attribute_ID, int ncid);
    
    char* currentFilePath;
    int currentSampleRate;
    
    const float d2r = M_PI / 180.0;

    typedef int nearestPoints[10];
    nearestPoints lookupTable[360][180];
    
};



#endif  // SOFAData_H_INCLUDED

