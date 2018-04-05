/*
  ==============================================================================

    SOFAData.h
    Created: 4 Apr 2017 11:47:12am
    Author:  David Bau

  ==============================================================================
*/


/* Some words about storage of HRIRs / HRTFs:
 
 Every HRTF/HRIR has its own object "Single_HRIR_Measurement", where both HRIR and HRTF are stored. All measurements are stored in the array of objects "loadedHRIRs".
 
 ::::::::::
 
 For every left/right pair of HRIR, there is only one float array of size hrirLength*2, containing the left HRIR in the first half and the right HRIR in the second half, eg.:
         [.....HRIRLeft.......|.....HRIRRight......]
          <---256------------> <----256----------->
          <-----------------512------------------->
 To access the i-th sample of left HRIR, one can simply call hrir[i]. To access the i-th sample of the right HRIR, one must call hrir[i + lengthOfHRIR]
 
 :::::::::
 
 Similar to this, the HRTFs are stored. Basically, the difference is that the array is not of type "float", but of type "fftwf_complex", which is basically an length-of-2 float array (float[2]). So the HRTF array is a 2D array. Accessing the values via hrtf[i] will return the pointer to the complex value. To access the actual real or imaginary value, one must call hrtf[i][0] or hrtf[i][1], respectively.
     Only the useful values of the transformation are stored in the HRTF, this means the HRTF has (hrirLength + 1) complex values. (see the documentation of fftw)
         [.....HRTFLeft........|.....HRTFRight.......]
          <---257-------------> <----257------------>
          <------------------514-------------------->
 
 ::::::::
 
 A dramatic difference is added, when the filter is partitionend. Considering an impulse response of length 1024 and a part length of 256 (4 parts). The array of the HRIR remains the same in size (4 parts a 256 samples = 1024), and does not have to be modified. The array of the HRTF has to be allocated differnetly, because the length of the unpartioned HRTF is 1025, but the partitioned HRTFs are of length 257 (4 * 257 = 1028).
 
         [..HRTFLeft 1...|..HRTFLeft 2.|..HRTFLeft 3..|..HRTFLeft 4..|..HRTFRight 1..|..HRTFRight 2..|..HRTFRight 3..|..HRTFRight 4..]
          <---257------> <----257-----> <----257-----> <----257-----> <----257----->  <----257----->   <----257----->  <----257----->
          <------------------1028-----------------------------------> <----------------------1028----------------------------------->
          <---------------------------------------------------------2056------------------------------------------------------------>
 
 To access the i-th real (or imaginary) value of the j-th part of the left HRTF, on must call hrtf[i + j * lengthOfHRTFPart][0 or 1]
 To access the i-th real (or imaginary) value of the j-th part of the right HRTF, on must call hrtf[i + (numParts + j) * lengthOfHRTFPart][0 or 1]
 
 NOTE that the decision to store the hrtf parts serially in one big array is not a logical decision because the spectra are not serially connected (in contrast to the HRIR, where the parts in serial result in the original HRIR). But it is easier to adapt the access routine to different partitions or nonpartitioned HRTFs. Maybe it is actually a quite bad decision, but works quite well for me. Shame on me.
 
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


#define ERR_MEM_ALLOC   1
#define ERR_READFILE    2
#define ERR_UNKNOWN     3
#define ERR_OPENFILE    4
#define ERR_NOTSUP      5



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
    float receiverPosition[2];
    float headRadius;
    
    int numberOfGlobalAttributes;
    //char** globalAttributeNames;
    //char** globalAttributeValues;
    Array<String> globalAttributeNames;
    Array<String> globalAttributeValues;
    
    
    
} sofaMetadataStruct;


typedef struct{
    int onsetL_samples, onsetR_samples;
    float ITD_ms;
} ITDStruct;


class Single_HRIR_Measurement {
public:
    Single_HRIR_Measurement(int lengthHRIR, int lengthHRTFPart, int numHRTFParts){
        HRIR = fftwf_alloc_real(lengthHRIR * 2);
        HRTF = fftwf_alloc_complex(lengthHRTFPart * numHRTFParts * 2);
        magSpectrum = (float*)malloc((lengthHRIR+1) * 2 * sizeof(float));
        phaseSpectrum = (float*)malloc((lengthHRIR+1) * 2 * sizeof(float));
    }
    ~Single_HRIR_Measurement(){
        fftwf_free(HRIR); 
        fftwf_free(HRTF);
        free(magSpectrum);
        free(phaseSpectrum);
        
    }
    void setValues(float azimuth, float elevation, float distance){
        Elevation = elevation; Azimuth = azimuth; Distance = distance;
    }
    float *getHRIR(){return  HRIR;}
    fftwf_complex* getHRTF(){return HRTF;}
    float *getMagSpectrum(){return magSpectrum;}
    float *getPhaseSpectrum(){return phaseSpectrum;}
    
    float Elevation;
    float Azimuth;
    float Distance;
    int index;
    
protected:
    float* HRIR;
    fftwf_complex* HRTF;
    float* magSpectrum; //used for plotting
    float* phaseSpectrum; //used for plotting
    
};

class Single_MinPhase_HRIR_Measurement: public Single_HRIR_Measurement {
public:
    Single_MinPhase_HRIR_Measurement(int lengthHRIR, int lengthHRTFPart, int numHRTFParts): Single_HRIR_Measurement(lengthHRIR, lengthHRTFPart, numHRTFParts){
    }
    ~Single_MinPhase_HRIR_Measurement(){
    }
    
    //float ITD_ms;
    ITDStruct ITD;
};





class SOFAData{
public:
    SOFAData();
    ~SOFAData();
    
    int initSofaData(const char* filePath, int sampleRate, int lengthOfFilterBlock);
    
    /** Will directly return the length of the new resampled HRTF */
    int setSampleRate(int newSampleRate);
    
    /**
     Get the closest HRTF for a given elevation & azimuth. The function will return a pointer to the TF-Values (complex numbers): the left TF first, followed directly by the right TF. The returned HRTF has a length of 2*(HRIRlength/2 + 1): [ L=N/2+1 | R=N/2+1 ].
     */
    fftwf_complex* getHRTFforAngle(float elevation, float azimuth, float radius);
    fftwf_complex* getMinPhaseHRTFforAngle(float elevation, float azimuth, float radius);
    ITDStruct getITDForAngle(float elevation, float azimuth, float radius);
    float* getHRIRForAngle(float elevation, float azimuth, float radius);
    float* getMagSpectrumForAngle(float elevation, float azimuth, float radius);
    float* getPhaseSpectrumForAngle(float elevation, float azimuth, float radius);

    sofaMetadataStruct getMetadata();
    
    /** Returns the length of the interpolated(!) Impulse Response in samples */
    int getLengthOfHRIR();
    int getLengthOfHRIRPart();
    
private:
    /** Number of Samples in the samplerate-converted HRIRs */
    int lengthOfHRIR;
    /** Has to be double the size of HRIR (due to linear convolution) */
    int lengthOfFFT;
    /** Number of Frequency-Bins FFTLength/2 + 1: will be the length of the samplerate-converted HRTFs */
    int lengthOfHRTF;
    
    
    fftwf_complex* theStoredHRTFs;
    int lengthOfHRTFStorage;
    int loadSofaFile(const char* filePath, int hostSampleRate);
    
    Single_HRIR_Measurement** loadedHRIRs;
    Single_MinPhase_HRIR_Measurement** loadedMinPhaseHRIRs;

    int searchHRTF(float elevation, float azimuth, float radius);
    void errorHandling(int status);
    String errorDesc;
    void createPassThrough_FIR(int _sampleRate);
    
    sofaMetadataStruct sofaMetadata;
    
    String getSOFAGlobalAttribute(const char* attribute_ID, int ncid);
    
    char* currentFilePath;
    int currentSampleRate;
    
    int lengthOfHRIRPart;
    int lengthOfHRTFPart;
    int numFilterParts;
    
};



#endif  // SOFAData_H_INCLUDED
