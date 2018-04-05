/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "math.h"

//==============================================================================
SofaPanAudioProcessor::SofaPanAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(params.azimuthParam = new AudioParameterFloat("azimuth", "Azimuth", 0.f, 1.f, 0.f));
    addParameter(params.bypassParam = new AudioParameterFloat("bypass", "Bypass", 0.f, 1.f, 0.f));
    addParameter(params.elevationParam = new AudioParameterFloat("elevation", "Elevation", 0.f, 1.f, 0.5f));
    addParameter(params.distanceParam = new AudioParameterFloat("distance", "Distance", 0.f, 1.f, 0.5f));
    addParameter(params.mirrorSourceParam = new AudioParameterBool("mirrorSource", "Mirror Source Model", true));
    addParameter(params.testSwitchParam = new AudioParameterBool("test", "Test Switch", false));
    addParameter(params.distanceSimulationParam = new AudioParameterBool("dist_sim", "Distance Simulation", false));
    addParameter(params.nearfieldSimulationParam = new AudioParameterBool("nearfield_sim", "Nearfield Simulation", false));
    addParameter(params.ITDAdjustParam = new AudioParameterBool("ITDadjust", "ITD Adjustment", false));
    
    HRTFs = new SOFAData();

    sampleRate_f = 0;
    
    updateSofaMetadataFlag = false;

    updater = &SofaPathSharedUpdater::instance();
    String connectionID = updater->createConnection();
    connectToPipe(connectionID, 10);
    estimatedBlockSize = 0;
    
}

SofaPanAudioProcessor::~SofaPanAudioProcessor()
{
    
    delete HRTFs;
    HRTFs = NULL;
    updater->removeConnection(getPipe()->getName());
    
}

//==============================================================================
const String SofaPanAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SofaPanAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SofaPanAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double SofaPanAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SofaPanAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SofaPanAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SofaPanAudioProcessor::setCurrentProgram (int index)
{
}

const String SofaPanAudioProcessor::getProgramName (int index)
{
    return String();
}

void SofaPanAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SofaPanAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    printf("\n prepare to play \n");
    counter = 0;
    
    

    if(sampleRate != sampleRate_f || estimatedBlockSize != samplesPerBlock){
        sampleRate_f = sampleRate;
        estimatedBlockSize = samplesPerBlock;
        if(usingGlobalSofaFile){
            String currentGlobalSofaFile = updater->requestCurrentFilePath();
            if(currentGlobalSofaFile.length() > 1)
                pathToSOFAFile = currentGlobalSofaFile;
        }
        initData(pathToSOFAFile);
    }
    

    reflectionInBuffer = AudioSampleBuffer(1, estimatedBlockSize);
    reverbInBuffer = AudioSampleBuffer(1, estimatedBlockSize);
    reverbOutBuffer = AudioSampleBuffer(2, estimatedBlockSize);
    
    reflectionInBuffer.clear();
    reverbInBuffer.clear();
    reverbOutBuffer.clear();

    reverb.prepareToPlay((int)sampleRate);
    directSource.prepareToPlay(HRTFs, (int)sampleRate_f, hrirSegmentSize);
    semistaticRefl.prepareToPlay();
    mirrorRefl.prepareToPlay();

    
}

void SofaPanAudioProcessor::initData(String sofaFile){
    
    printf("\n initalise Data \n ");
    
    pathToSOFAFile = sofaFile;
    
    suspendProcessing(true);
    
    int status = HRTFs->initSofaData(pathToSOFAFile.toUTF8(), (int)sampleRate_f, estimatedBlockSize);
    
    hrirSegmentSize = HRTFs->getLengthOfHRIRPart();

        
    metadata_sofafile = HRTFs->getMetadata();
    
    updateSofaMetadataFlag = true;
    
    
    directSource.prepareToPlay(HRTFs, (int)sampleRate_f, hrirSegmentSize);
    status = 0;
    //status += semistaticRefl.init(HRTFs, roomSize, (int)sampleRate_f);
    //status += mirrorRefl.initWithSofaData(HRTFs, roomSize, (int)sampleRate_f);
    
    //If an critical error occured during sofa loading, turn this plugin to a brick 
    if(!status) suspendProcessing(false);
    
    //printf("Processing Suspenden: %d", proce)
}

void SofaPanAudioProcessor::releaseResources()
{
   
    printf("Playback Stop");
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SofaPanAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SofaPanAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    
    const int numberOfSamples = buffer.getNumSamples();
    
    if (params.bypassParam->get() == true) { return; }
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    //This should actually never happen, BUT it is possible
    if(estimatedBlockSize != numberOfSamples){
        estimatedBlockSize = numberOfSamples;
        reflectionInBuffer.setSize(reflectionInBuffer.getNumChannels(), estimatedBlockSize);
        reverbInBuffer.setSize(reverbInBuffer.getNumChannels(), estimatedBlockSize);
        reverbOutBuffer.setSize(reverbOutBuffer.getNumChannels(), estimatedBlockSize);
        reflectionInBuffer.clear();
        reverbInBuffer.clear();
        reverbOutBuffer.clear();
    }
    
    reflectionInBuffer.copyFrom(0, 0, buffer.getReadPointer(0), numberOfSamples);
    reverbInBuffer.copyFrom(0, 0, buffer.getReadPointer(0), numberOfSamples);

    
    float* outBufferL = buffer.getWritePointer (0);
    float* outBufferR = buffer.getWritePointer (1);
    const float* inBuffer = buffer.getReadPointer(0);
    
    const float* inBufferRefl = reflectionInBuffer.getReadPointer(0);
    const float* inBufferReverb = reverbInBuffer.getReadPointer(0);
    float* outBufferReverbL = reverbOutBuffer.getWritePointer(0);
    float* outBufferReverbR = reverbOutBuffer.getWritePointer(1);

    soundSourceData data;
    data.azimuth = params.azimuthParam->get() * 360.0;
    data.elevation = (params.elevationParam->get()-0.5) * 180.0;
    data.distance = params.distanceParam->get();
    data.ITDAdjust = params.ITDAdjustParam->get();
    data.nfSimulation = params.nearfieldSimulationParam->get();
    data.overwriteOutputBuffer = true;
    directSource.process(inBuffer, outBufferL, outBufferR, numberOfSamples, data);

    //buffer.clear();
    
//    float gain = 1.0;
//    if(params.distanceSimulationParam->get() || metadata_sofafile.hasMultipleDistances){
//        if(params.distanceParam->get() > 0.1)
//             gain = 1 / params.distanceParam->get();
//        buffer.applyGain(gain);
//    }
    

    
//    if(params.distanceSimulationParam->get()){
//
//        if(params.mirrorSourceParam->get()){
//            mirrorRefl.process(inBufferRefl, outBufferL, outBufferR, numberOfSamples, params);
//        }else{
//
//            semistaticRefl.process(inBufferRefl, outBufferL, outBufferR, numberOfSamples, params);
//        }
//
//
////        reverb.processBlockMS(inBufferReverb, outBufferReverbL, outBufferReverbR, numberOfSamples, params.testSwitchParam->get());
////        reverbOutBuffer.applyGain(0.05);
////
////        buffer.addFrom(0, 0, reverbOutBuffer, 0, 0, numberOfSamples);
////        buffer.addFrom(1, 0, reverbOutBuffer, 1, 0, numberOfSamples);
//
//
//    }
    
    buffer.applyGain(0.25);


}

//==============================================================================
bool SofaPanAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SofaPanAudioProcessor::createEditor()
{
    return new SofaPanAudioProcessorEditor (*this);
}

//==============================================================================
void SofaPanAudioProcessor::getStateInformation (MemoryBlock& destData)
{
//    ScopedPointer<XmlElement> xml (new XmlElement ("SofaPanSave"));
//    xml->setAttribute ("gain", (double) *gain);
//    copyXmlToBinary (*xml, destData);
}

void SofaPanAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SofaPanAudioProcessor();
}



void SofaPanAudioProcessor::setSOFAFilePath(String sofaString)
{
    pathToSOFAFile = sofaString;
    if(estimatedBlockSize != 0) //to avoid inits before prepareToPlay was called
        initData(pathToSOFAFile);
    
    if(usingGlobalSofaFile){
        MemoryBlock message;
        const char* messageText = pathToSOFAFile.toRawUTF8();
        size_t messageSize = pathToSOFAFile.getNumBytesAsUTF8();
        message.append(messageText, messageSize);
        
        sendMessage(message);
    }
    
    
}

fftwf_complex* SofaPanAudioProcessor::getCurrentHRTF()
{
    if(HRTFs == NULL)
        return NULL;
    
    float azimuth = params.azimuthParam->get() * 360.0;
    float elevation = (params.elevationParam->get()-0.5) * 180.0;
    float distance = 1;
    if(!(bool)params.distanceSimulationParam->get())
        distance = params.distanceParam->get();
    
    return HRTFs->getHRTFforAngle(elevation, azimuth, distance);
}

float* SofaPanAudioProcessor::getCurrentHRIR()
{

    if(HRTFs == NULL)
        return NULL;
    
    float azimuth = params.azimuthParam->get() * 360.0;
    float elevation = (params.elevationParam->get()-0.5) * 180.0;
    
    float distance = 1;
    if(!(bool)params.distanceSimulationParam->get())
        distance = params.distanceParam->get();
    
    return HRTFs->getHRIRForAngle(elevation, azimuth, distance);
}

ITDStruct SofaPanAudioProcessor::getCurrentITD()
{
    ITDStruct nullStruct;
    nullStruct.onsetL_samples = 0;
    nullStruct.onsetR_samples = 0;
    nullStruct.ITD_ms = 0;
    if(HRTFs == NULL)
        return nullStruct;
    
    float azimuth = params.azimuthParam->get() * 360.0;
    float elevation = (params.elevationParam->get()-0.5) * 180.0;
    
    float distance = 1;
    if(!(bool)params.distanceSimulationParam->get())
        distance = params.distanceParam->get();
    
    return HRTFs->getITDForAngle(elevation, azimuth, distance);
}

float* SofaPanAudioProcessor::getCurrentMagSpectrum()
{
    
    if(HRTFs == NULL)
        return NULL;
    
    float azimuth = params.azimuthParam->get() * 360.0;
    float elevation = (params.elevationParam->get()-0.5) * 180.0;
    
    float distance = 1;
    if(!(bool)params.distanceSimulationParam->get())
        distance = params.distanceParam->get();
    
    return HRTFs->getMagSpectrumForAngle(elevation, azimuth, distance);
}

float* SofaPanAudioProcessor::getCurrentPhaseSpectrum()
{
    
    if(HRTFs == NULL)
        return NULL;
    
    float azimuth = params.azimuthParam->get() * 360.0;
    float elevation = (params.elevationParam->get()-0.5) * 180.0;
    
    float distance = 1;
    if(!(bool)params.distanceSimulationParam->get())
        distance = params.distanceParam->get();
    
    return HRTFs->getPhaseSpectrumForAngle(elevation, azimuth, distance);
}



int SofaPanAudioProcessor::getSampleRate()
{
    return (int)sampleRate_f;
}


void SofaPanAudioProcessor::messageReceived (const MemoryBlock &message){
    
    if(usingGlobalSofaFile){
        String newFilePath = message.toString();
        //printf("\n%s: Set New File Path: %s", getPipe()->getName().toRawUTF8(), newFilePath.toRawUTF8());
        if(estimatedBlockSize != 0) //to avoid inits before prepareToPlay was called
            initData(newFilePath);
    }
    
}

void SofaPanAudioProcessor::setUsingGlobalSofaFile(bool useGlobal){
    if(useGlobal){
        String path = updater->requestCurrentFilePath();
        if(path.length() > 1 && path!=pathToSOFAFile){
            pathToSOFAFile = path;
            if(estimatedBlockSize != 0) //to avoid inits before prepareToPlay was called
                initData(pathToSOFAFile);
        }
        usingGlobalSofaFile = true;
    }else{
        usingGlobalSofaFile = false;
    }
}

bool SofaPanAudioProcessor::getUsingGlobalSofaFile(){
    return usingGlobalSofaFile;
}

