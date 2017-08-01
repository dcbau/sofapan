/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "FilterEngine.h"
#include "EarlyReflection.h"

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
    addParameter(params.testSwitchParam = new AudioParameterBool("test", "Test Switch", false));
    addParameter(params.distanceSimulationParam = new AudioParameterBool("dist_sim", "Distance Simulation", false));

    
    HRTFs = new SOFAData();
    Filter = NULL;

    earlyReflections.resize(4);
    for(int i = 0; i < earlyReflections.size(); i++)
        earlyReflections[i] = NULL;
    
    sampleRate_f = 0;
    
    updateSofaMetadataFlag = false;

    updater = &SofaPathSharedUpdater::instance();
    String connectionID = updater->createConnection();
    connectToPipe(connectionID, 10);
    
    
}

SofaPanAudioProcessor::~SofaPanAudioProcessor()
{
    
    delete HRTFs;
    if(Filter != NULL) delete Filter;
    for(int i=0; i < earlyReflections.size(); i++)
        if(earlyReflections[i] != NULL) delete earlyReflections[i];
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
    
    if(sampleRate != sampleRate_f){
        sampleRate_f = sampleRate;
        if(usingGlobalSofaFile){
            String currentGlobalSofaFile = updater->requestCurrentFilePath();
            if(currentGlobalSofaFile.length() > 1)
                pathToSOFAFile = currentGlobalSofaFile;
        }
        initData(pathToSOFAFile);
    }else{
        Filter->prepareToPlay();
        for(int i=0; i < earlyReflections.size(); i++)
            earlyReflections[i]->prepareToPlay();
    }

    
    
}

void SofaPanAudioProcessor::initData(String sofaFile){
    
    printf("\n initalise Data \n ");
    
    pathToSOFAFile = sofaFile;
    
    suspendProcessing(true);
    
    HRTFs->initSofaData(pathToSOFAFile.toUTF8(), (int)sampleRate_f);
    metadata_sofafile = HRTFs->getMetadata();
    
    updateSofaMetadataFlag = true;
    
    if(Filter != NULL) delete Filter;
    Filter = new FilterEngine(*HRTFs);
    
    const float angles[4] = {90, 270, 0, 180}; //right, left, front, back
    for(int i=0; i < earlyReflections.size(); i++){
        if(earlyReflections[i] != NULL) delete earlyReflections[0];
        earlyReflections[i] = new EarlyReflection(HRTFs->getHRTFforAngle(0.0, angles[i], 1.0), HRTFs->getLengthOfHRIR(), (int)sampleRate_f);
    }
    
    suspendProcessing(false);
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
    
    
    AudioSampleBuffer reflectionInBuffer = AudioSampleBuffer(2, numberOfSamples);
    reflectionInBuffer.clear();
    reflectionInBuffer.copyFrom(0, 0, buffer.getReadPointer(0), numberOfSamples);
    
    float* outBufferL = buffer.getWritePointer (0);
    float* outBufferR = buffer.getWritePointer (1);
    const float* inBuffer = buffer.getReadPointer(0);
    
    const float* inBufferRefl = reflectionInBuffer.getReadPointer(0);
    
    
    Filter->process(inBuffer, outBufferL, outBufferR, numberOfSamples, params);

    //buffer.clear();
    float gain = 1.0;
    if(params.distanceSimulationParam || metadata_sofafile.hasMultipleDistances){
        if(params.distanceParam->get() > 0.1)
             gain = 1 / params.distanceParam->get();
        buffer.applyGain(gain);

    }
    
    //Calculating an approximation of the angle depentant reflection delays and damping-factors (the approximations were made for a square room of 3.5x3.5m and a source distance of 1m)
    
    if(params.distanceSimulationParam->get()){
        
        float alpha_s = sinf(params.azimuthParam->get() * 2.0 * M_PI); //Running from 0 -> 1 -> 0 -> -1 -> 0 for a full circle
        float alpha_c = cosf(params.azimuthParam->get() * 2.0 * M_PI); //Running from 1 -> 0 -> -1 -> 0 -> 1 for a full circle
        float delay[4], damp[4];
        
        float distance = 1.0;
        if(params.distanceSimulationParam || metadata_sofafile.hasMultipleDistances){
            distance = params.distanceParam->get();
        }
        const float roomRadius = 4; //6x6m room
        const float speedOfSound = 343.2;
        const float meterToMs = 1000.0 / speedOfSound;
        delay[0] = meterToMs * ((2.0 * roomRadius - distance) - alpha_s * distance);
        delay[1] = meterToMs * ((2.0 * roomRadius - distance) + alpha_s * distance);
        delay[1] = meterToMs * ((2.0 * roomRadius - distance) - alpha_c * distance);
        delay[1] = meterToMs * ((2.0 * roomRadius - distance) + alpha_c * distance);

        damp[0] = 1.0/(2*roomRadius-alpha_s * distance);
        damp[1] = 1.0/(2*roomRadius+alpha_s * distance);
        damp[2] = 1.0/(2*roomRadius-alpha_c * distance);
        damp[3] = 1.0/(2*roomRadius+alpha_c * distance);
        
        for(int i=0; i < 2; i++){
            earlyReflections[i]->process(inBufferRefl, outBufferL, outBufferR, numberOfSamples, delay[i], damp[i]);
        }

//    }else{
//    
//        //Approximation by interpolating between extremal values
//        delay[0] = 14.0 - (alpha_s * 3.5); //rightSide: 14 -> 11 -> 14 -> 17ms delay
//        delay[1] = 14.0 + (alpha_s * 3.5); //leftSide: 14 -> 17 -> 14 -> 11ms
//        delay[2] = 14.0 - (alpha_c * 3.5); //front: 11 -> 14 -> 17 -> 14ms
//        delay[3] = 14.0 + (alpha_c * 3.5); //back: 17 -> 14 -> 11 -> 14ms
////        printf("\n delay1 = %f, newDelay = %f", delay[0], newDelay);
//
//        damp[0] = 1.0/(5.0-alpha_s); // 1/5 -> 1/4 -> 1/5 -> 1/6
//        damp[1] = 1.0/(5.0+alpha_s); // 1/5 -> 1/6 -> 1/5 -> 1/4
//        damp[2] = 1.0/(5.0-alpha_c); // 1/4 -> 1/5 -> 1/6 -> 1/5
//        damp[3] = 1.0/(5.0+alpha_c); // 1/6 -> 1/5 -> 1/4 -> 1/5
//        
//    }

//        earlyReflections[0]->process(inBufferRefl, outBufferL, outBufferR, numberOfSamples, delay[0], damp[0]); //90°
//        earlyReflections[1]->process(inBufferRefl, outBufferL, outBufferR, numberOfSamples, delay[1], damp[1]); //-90°
//        earlyReflections[2]->process(inBufferRefl, outBufferL, outBufferR, numberOfSamples, delay[2], damp[2]); //90°
//        earlyReflections[3]->process(inBufferRefl, outBufferL, outBufferR, numberOfSamples, delay[3], damp[3]); //-90°
    }
    
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

int SofaPanAudioProcessor::getSampleRate()
{
    return (int)sampleRate_f;
}

int SofaPanAudioProcessor::getComplexLength()
{
    return Filter->getComplexLength();
}

void SofaPanAudioProcessor::messageReceived (const MemoryBlock &message){
    
    if(usingGlobalSofaFile){
        String newFilePath = message.toString();
        printf("\n%s: Set New File Path: %s", getPipe()->getName().toRawUTF8(), newFilePath.toRawUTF8());
    
        initData(newFilePath);
    }
    
}

void SofaPanAudioProcessor::setUsingGlobalSofaFile(bool useGlobal){
    if(useGlobal){
        String path = updater->requestCurrentFilePath();
        if(path.length() > 1 && path!=pathToSOFAFile){
            pathToSOFAFile = path;
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
