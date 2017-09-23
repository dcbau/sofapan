/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

#include "SofaMetadataView.h"
#include "PlotHRTFComponent.h"
#include "PlotHRIRComponent.h"
#include "RoomPannerComponent.h"

#include "sofaPanLookAndFeel.h"
#include "azimuthSliderLookAndFeel.h"
#include "elevationSliderLookAndFeel.h"
#include "LogoHexData.h"
#include "HeadTopHexData.h"
#include "HeadSideHexData.h"
#include "SpeakerHexData.h"

//#define roomsimLayout 1

//==============================================================================
/**
*/
class SofaPanAudioProcessorEditor  : public AudioProcessorEditor, public Slider::Listener, public Button::Listener, private Timer
{
public:
    SofaPanAudioProcessorEditor (SofaPanAudioProcessor&);
    ~SofaPanAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    void sliderValueChanged(Slider* slider) override;
    void buttonClicked (Button* button) override;
    
    void mouseEnter (const MouseEvent &e) override;
    void mouseExit (const MouseEvent &e) override;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SofaPanAudioProcessor& processor;
    
    Slider panner_az;
    Slider panner_el;
    Slider panner_dist;
    TextButton loadSOFAButton;
    TextButton showSOFAMetadataButton;
    ToggleButton bypassButton;
    ToggleButton testSwitchButton;
    ToggleButton mirrorSourceButton;
    ToggleButton useGlobalSofaFileButton;
    ToggleButton useDistanceSimulationButton;
    ToggleButton useNearfieldSimulationButton;
    ToggleButton showTooltipsButton;
    TextButton useLayoutSimplePanningButton;
    TextButton useLayoutRoomsimButton;
    
    const String sofaMetadataID = String("Listener Short Name: \nMeasurements: \nSamples per IR: \nSOFA Convention: \nData Type: \nElevation: \nDistance:");
    
    String sofaMetadataValue;
    
    SofaPanLookAndFeel sofaPanLookAndFeel;
    ElevationSliderLookAndFeel elSliderLookAndFeel;
    AzimuthSliderLookAndFeel azSliderLookAndFeel;
    LookAndFeel_V4 juceDefaultLookAndFeel;
    
    AudioProcessorParameter* getParameter (const String& paramId);
    void setParameterValue (const String& paramId, float value);
    float getParameterValue (const String& paramId);
    
    void sliderDragStarted(Slider* slider) override;
    void sliderDragEnded(Slider* slider) override;

    SofaMetadataView metadataView;
    PlotHRTFComponent plotHRTFView;
    PlotHRIRComponent plotHRIRView;
    RoomPannerComponent panner2D_top;
    RoomPannerComponent panner2D_rear;
    int counter;
    
    float lastElevationValue;
    float lastAzimuthValue;
    float lastDistanceValue;
    
    const float deg2rad = 2.0 * M_PI / 360.0;
    
    const Image logoHSD = ImageFileFormat::loadFrom(hsd_logo, hsd_logo_size);
    const Image headTopImage = ImageCache::getFromMemory(headTopPicto, headTopPicto_Size);
    const Image speakerImage = ImageCache::getFromMemory(speaker, speaker_Size);
    const Image headSideImage = ImageCache::getFromMemory(headSidePicto, headSidePicto_Size);
    
    Image backgroundImage;
    
    const float simulationDistanceMin = 0.2;
    const float simulationDistanceMax = 5.0;
    
    ScopedPointer<BubbleMessageComponent> popUpInfo;
    
    bool roomsimLayout = 0;
    void rearrange();
    
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SofaPanAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
