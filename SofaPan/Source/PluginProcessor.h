/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "fftw3.h"
#include "SOFAData.h"
//#include "SofaPathSharedUpdater.h"
#include "Reverberator.h"
#include "SoundSource.h"
#include "ParameterStruct.h"
#include "MirrorReflections.h"
#include "BiquadCascade.h"

#if ENABLE_SEMESTATICS
    #include "SemistaticReflections.h"
#endif

//Interface für SOFA File:
extern "C" {
#include <netcdf.h>
}

#ifdef _WIN64
#define SOFA_DEFAULT_PATH "C:\\Program Files\\Steinberg\\VstPlugins\\SOFAFiles\\mit_kemar_normal_pinna.sofa"
#elif _WIN32
#define SOFA_DEFAULT_PATH "C:\\Program Files (x86)\\Steinberg\\VstPlugins\\SOFAFiles\\mit_kemar_normal_pinna.sofa"
#elif __APPLE__
#define SOFA_DEFAULT_PATH "/Library/Audio/Plug-Ins/SOFAFiles/mit_kemar_normal_pinna.sofa"
#endif

#define ROOMRADIUS 5.0
#define HEADHEIGHT 1.7


//class FilterEngine;
class EarlyReflection;

class SofaPanAudioProcessor  : public AudioProcessor//, public InterprocessConnection
{
public:
    //==============================================================================
    SofaPanAudioProcessor();
    ~SofaPanAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void setSOFAFilePath(String sofaString);
    
    String pathToSOFAFile = String(SOFA_DEFAULT_PATH);
    sofaMetadataStruct metadata_sofafile;
    void initData(String sofaFile);
    bool updateSofaMetadataFlag;
    
    fftwf_complex* getCurrentHRTF();
    float* getCurrentHRIR();
    ITDStruct getCurrentITD();
    float* getCurrentMagSpectrum();
    float* getCurrentPhaseSpectrum();

    int getLengthOfHRIR(){return (HRTFs == NULL ?  0 :  HRTFs->getLengthOfHRIR());}
    
    int getSampleRate();

	//void setUsingGlobalSofaFile(bool useGlobal);
	//bool getUsingGlobalSofaFile();
	
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SofaPanAudioProcessor)

    parameterStruct params;
    
    //Zähler für periodische print-ausgaben
    int counter;
    
    float sampleRate_f;
    
    ScopedPointer<SOFAData> HRTFs;
    SoundSource directSource;
    
    //std::vector<EarlyReflection*> earlyReflections;
#if ENABLE_SEMESTATICS
    SemistaticReflections semistaticRefl;
#endif
    
    MirrorReflections mirrorRefl;
    

    Reverberator reverb;
    
    //SofaPathSharedUpdater* updater;
    
    //void connectionMade() override{}
    //void connectionLost() override{}
    //void messageReceived (const MemoryBlock &message) override;
    
    //bool usingGlobalSofaFile = false;
    
    int estimatedBlockSize;

    AudioSampleBuffer reflectionInBuffer, reflectionOutBuffer, reverbInBuffer, reverbOutBuffer;

    const float roomSize = 2 * ROOMRADIUS;


};



#endif  // PLUGINPROCESSOR_H_INCLUDED
