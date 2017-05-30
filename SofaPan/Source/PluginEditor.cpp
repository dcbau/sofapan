/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
SofaPanAudioProcessorEditor::SofaPanAudioProcessorEditor (SofaPanAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 400);
    
    LookAndFeel::setDefaultLookAndFeel(&sofaPanLookAndFeel);
    
    
    setSize (900, 500);
    
    panner_az.setSliderStyle(Slider::Rotary);
    panner_az.setLookAndFeel(&azSliderLookAndFeel);
    panner_az.setRange(0.0, 359.0, 1.0);
    panner_az.setTextValueSuffix(" deg");
    panner_az.setPopupDisplayEnabled(false, this);
    panner_az.setTextBoxStyle(Slider::TextBoxBelow, false, 70, 15);
    panner_az.setColour(Slider::textBoxBackgroundColourId, Colours::white);
    panner_az.setRotaryParameters(0, M_PI*2.0, false);
    panner_az.addListener(this);
    addAndMakeVisible(&panner_az);

    
    panner_el.setSliderStyle(Slider::Rotary);
    panner_el.setLookAndFeel(&elSliderLookAndFeel);
    panner_el.setRange(-90.0, 90.0, 1.0);
    panner_el.setTextValueSuffix(" deg");
    panner_el.setPopupDisplayEnabled(false, this);
    panner_el.setTextBoxStyle(Slider::TextBoxBelow, false, 70, 15);
    panner_el.setColour(Slider::textBoxBackgroundColourId, Colours::white);
    panner_el.setRotaryParameters(M_PI, M_PI*2, true);
    panner_el.addListener(this);
    addAndMakeVisible(&panner_el);
    
    panner_dist.setSliderStyle(Slider::LinearVertical);
    panner_dist.setRange(0.0, 1.0);
    panner_dist.setTextValueSuffix(" m");
    panner_dist.setPopupDisplayEnabled(false, this);
    panner_dist.setTextBoxStyle(Slider::TextBoxBelow, false, 70, 15);
    panner_dist.addListener(this);
    addAndMakeVisible(&panner_dist);
    
    loadSOFAButton.setButtonText ("Load HRTF");
    loadSOFAButton.addListener(this);
    addAndMakeVisible(&loadSOFAButton);
    
    bypassButton.setButtonText("Bypass");
    bypassButton.setColour(ToggleButton::textColourId, Colours::white);
    bypassButton.addListener(this);
    addAndMakeVisible(&bypassButton);
    
    testSwitchButton.setButtonText("Test Switch");
    testSwitchButton.setColour(ToggleButton::textColourId, Colours::white);
    testSwitchButton.addListener(this);
    addAndMakeVisible(&testSwitchButton);
 
    headTopImage = ImageCache::getFromMemory(headTopPicto, headTopPicto_Size);
    headSideImage = ImageCache::getFromMemory(headSidePicto, headSidePicto_Size);
    speakerImage = ImageCache::getFromMemory(speaker, speaker_Size);
    
    addAndMakeVisible(&plotHRTFView);
    addAndMakeVisible(&plotHRIRView);
    
    showSOFAMetadataButton.setButtonText("Show More Information");
    showSOFAMetadataButton.addListener(this);
    addAndMakeVisible(&showSOFAMetadataButton);
    addAndMakeVisible(&metadataView);
    metadataView.setVisible(false);
    
    counter = 0;
    startTimer(50);
    lastAzimuthValue = 0.0;
    lastElevationValue = 0.0;
    lastDistanceValue = 0.0;

}

SofaPanAudioProcessorEditor::~SofaPanAudioProcessorEditor()
{
}


void SofaPanAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::black);
    
    g.setFont (15.0f);
    g.setColour(Colours::white);
    g.setOpacity(1.0f);

    const Image logo = ImageFileFormat::loadFrom(hsd_logo, hsd_logo_size);
    g.drawImageAt(logo, 0, getHeight()-40, true);
    
    // images for azimuth slider
    Rectangle<int> rect = panner_az.getBounds();
    int newWidth = rect.getWidth() - panner_az.getTextBoxHeight(); //always quadratic
    rect.setSize(newWidth, newWidth);
    const int pngPixelAdjust = 2;
    
    // head image (top view)
    g.drawImage(headTopImage,
                rect.getCentreX()-rect.getWidth()/4 + 10,
                rect.getCentreY()-rect.getHeight()/4 + 10 - pngPixelAdjust,
                rect.getWidth()/2 - 20,
                rect.getWidth()/2 - 20,
                0, 0, 200, 200);

    // rotating speaker image
    const int speakerSize = 40;
    const int spaceBetweenSpeakerAndSlider = 6;
    float sliderPosition = (float)panner_az.getValue() / 360.0;
    AffineTransform t = AffineTransform().translated(rect.getCentreX()- speakerSize/2 ,
                 rect.getY() - speakerSize - spaceBetweenSpeakerAndSlider). rotated((float)(M_PI*2.0*sliderPosition), (float)rect.getCentreX(), (float)rect.getCentreY());
    g.drawImageTransformed(speakerImage.rescaled(speakerSize, speakerSize),
                           t,
                           false);

    // images for elevation slider
    rect = panner_el.getBounds();
    newWidth = rect.getWidth() - panner_el.getTextBoxHeight();
    rect.setSize(newWidth, newWidth);
    
    // head image (side)
    g.drawImage(headSideImage,
                rect.getCentreX()-rect.getWidth()/4 + 10,
                rect.getCentreY()-rect.getHeight()/4 + 10,
                rect.getWidth()/2 - 20,
                rect.getWidth()/2 - 20,
                0, 0, 200, 200);
    
    // rotating speaker image
    sliderPosition = panner_el.getValue() / 360 + 0.75;
    t = AffineTransform().translated(rect.getCentreX()- speakerSize/2 ,
                                    rect.getY() - speakerSize - spaceBetweenSpeakerAndSlider). rotated((float)(M_PI*2.0*sliderPosition), (float)rect.getCentreX(), (float)rect.getCentreY());
    g.drawImageTransformed(speakerImage.rescaled(speakerSize, speakerSize),
                           t,
                           false);
    
    // labels for panning sliders
    g.setFont(Font(20.f));
    g.drawText("Azimuth", panner_az.getBounds().withHeight(40).translated(0, -40), juce::Justification::topLeft);
    g.drawText("Elevation", panner_el.getBounds().withHeight(40).translated(0, -40), juce::Justification::topLeft);
    g.drawText("Distance", panner_dist.getBounds().withHeight(40).translated(0, -40), juce::Justification::topLeft);
    
    // sofa metadata short info
    g.setFont(Font(11));
    g.drawFittedText(sofaMetadataID, 10, 130, 100, 100, Justification::topLeft, 3);
    g.drawFittedText(sofaMetadataValue, 110, 130, 300, 100, Justification::topLeft, 3);
}

void SofaPanAudioProcessorEditor::resized()
{
    const int panner_size = 200;
    panner_az.setBounds(250, 300.0 * 0.50 - panner_size/2, panner_size, panner_size);
    panner_el.setBounds(550, 300.0 * 0.50 - panner_size/2, panner_size, panner_size);
    panner_dist.setBounds(750, 300.0 * 0.50 - panner_size/2, 100, panner_size);
    
    loadSOFAButton.setBounds(-5., 10., 150., 30.);
    showSOFAMetadataButton.setBounds(-8, 230, 200, 30);
    
    metadataView.setBounds(getLocalBounds().reduced(20));
    
    bypassButton.setBounds(10., 50., 150., 30.);
    testSwitchButton.setBounds(10., 80., 150., 30.);
    
    plotHRTFView.setBounds(25.0, 300.0, 350.0, 130.0);
    plotHRIRView.setBounds(425.0, 300.0, 350.0, 130.0);
}

// This timer periodically checks whether any of the filter's parameters have changed...
void SofaPanAudioProcessorEditor::timerCallback() {
    
    bypassButton.setToggleState((bool)getParameterValue("bypass"), NotificationType::dontSendNotification);
    testSwitchButton.setToggleState((bool)getParameterValue("test"), NotificationType::dontSendNotification);
    
    // update panning sliders if needed
    float azimuthValue = getParameterValue("azimuth") * 360.;
    if(azimuthValue != lastAzimuthValue)
        panner_az.setValue(azimuthValue, NotificationType::dontSendNotification);
    
    float elevationValue = (getParameterValue("elevation")-0.5) * 180.;
    if(elevationValue != lastElevationValue)
        panner_el.setValue(elevationValue, NotificationType::dontSendNotification);
    
    float distanceValue = (getParameterValue("distance"));
    if(distanceValue != lastDistanceValue)
        panner_dist.setValue(distanceValue, NotificationType::dontSendNotification);
    
    // update hrtf/hrir plots
    if(lastElevationValue!=elevationValue || lastAzimuthValue!=azimuthValue || lastDistanceValue!=distanceValue || processor.updateSofaMetadataFlag){
        fftwf_complex* hrtf = processor.getCurrentHRTF();
        float* hrir = processor.getCurrentHRIR();
        int complexLength = processor.getComplexLength();
        int firLength = complexLength - 1;
        int sampleRate = processor.getSampleRate();
        plotHRTFView.drawHRTF(hrtf, complexLength, sampleRate);
        plotHRIRView.drawHRIR(hrir, firLength, processor.getSampleRate());
    }
   
    lastAzimuthValue = azimuthValue;
    lastElevationValue = elevationValue;
    lastDistanceValue = distanceValue;
    
    // update metadata when new sofa file is loaded
    if(processor.updateSofaMetadataFlag){
        
        metadataView.setMetadata(processor.metadata_sofafile.globalAttributeNames, processor.metadata_sofafile.globalAttributeValues);
        
        String numMeasurement_Note = static_cast <String> (processor.metadata_sofafile.numMeasurements);
        String numSamples_Note = static_cast <String> (processor.metadata_sofafile.numSamples);
        String sofaConvections_Note = String(processor.metadata_sofafile.SOFAConventions);
        String dataType_Note = String(processor.metadata_sofafile.dataType);
        String listenerShortName_Note = String(processor.metadata_sofafile.listenerShortName);
        float eleMin = processor.metadata_sofafile.minElevation;
        float eleMax = processor.metadata_sofafile.maxElevation;
        String elevationRange_Note;
        if (eleMax-eleMin != 0.0){
            String elevationMin = static_cast <String> (processor.metadata_sofafile.minElevation);
            String elevationMax = static_cast <String> (processor.metadata_sofafile.maxElevation);
            elevationRange_Note = (elevationMin + "° to " + elevationMax + "°" );
            panner_el.setEnabled(true);
            panner_el.setRange(eleMin, eleMax);
            panner_el.setRotaryParameters((270. + eleMin) * deg2rad , (270. + eleMax) * deg2rad, true);
        }else{
            elevationRange_Note = "none";
            panner_el.setValue(0.0);
            panner_el.setEnabled(false);
        }
        float distMin = processor.metadata_sofafile.minDistance;
        float distMax = processor.metadata_sofafile.maxDistance;
        String distanceRange_Note;
        if(distMax - distMin != 0.0){
            String distanceMin = static_cast<String>(processor.metadata_sofafile.minDistance);
            String distanceMax = static_cast<String>(processor.metadata_sofafile.maxDistance);
            distanceRange_Note = (distanceMin + "m to " + distanceMax + "m" );
            panner_dist.setEnabled(true);
            panner_dist.setRange(distMin, distMax);
        }else{
            distanceRange_Note = "none";
            panner_dist.setValue(0.0);
            panner_dist.setEnabled(false);
        }
        
        sofaMetadataValue = String(listenerShortName_Note + "\n" +
                                   numMeasurement_Note + "\n" +
                                   numSamples_Note + "\n" +
                                   sofaConvections_Note + "\n" +
                                   dataType_Note + "\n" +
                                   elevationRange_Note + "\n" +
                                   distanceRange_Note
                                   );

        processor.updateSofaMetadataFlag = false;
        repaint();
    }
    
    
    
}

void SofaPanAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if(slider == &panner_az){
        float panNormValue = panner_az.getValue() / 360.0;
        setParameterValue("azimuth", panNormValue);
        repaint();
    }
    if(slider == &panner_el){
        float elevationNormValue = (panner_el.getValue() / 180.0) + 0.5; //map -90/90 -> 0/1
        setParameterValue("elevation", elevationNormValue);
        repaint();
    }
    if(slider == &panner_dist){
        setParameterValue("distance", panner_dist.getValue());
        repaint();
    }
}

void SofaPanAudioProcessorEditor::sliderDragStarted(Slider* slider)
{
    if (slider == &panner_az) {
        if (AudioProcessorParameter* param = getParameter("azimuth"))
            param->beginChangeGesture();
    }
    if (slider == &panner_el) {
        if (AudioProcessorParameter* param = getParameter("elevation"))
            param->beginChangeGesture();
    }
    if (slider == &panner_dist) {
        if (AudioProcessorParameter* param = getParameter("distance"))
            param->beginChangeGesture();
    }
}

void SofaPanAudioProcessorEditor::sliderDragEnded(Slider* slider)
{
    if (slider == &panner_az) {
        if (AudioProcessorParameter* param = getParameter("azimuth"))
            param->endChangeGesture();
    }
    if (slider == &panner_el) {
        if (AudioProcessorParameter* param = getParameter("elevation"))
            param->endChangeGesture();
    }
    if (slider == &panner_dist) {
        if (AudioProcessorParameter* param = getParameter("distance"))
            param->endChangeGesture();
    }
}

AudioProcessorParameter* SofaPanAudioProcessorEditor::getParameter (const String& paramId)
{
    const OwnedArray<AudioProcessorParameter>& params = processor.getParameters();
    
    for (int i = 0; i < params.size(); ++i){
        if (AudioProcessorParameterWithID* param = dynamic_cast<AudioProcessorParameterWithID*> (params[i])){
            if (param->paramID == paramId)
                return param;
        }
    }
    return nullptr;
}

//==============================================================================
float SofaPanAudioProcessorEditor::getParameterValue (const String& paramId)
{
    if (AudioProcessorParameter* param = getParameter (paramId))
        return param->getValue();
    
    return 0.0f;
}

void SofaPanAudioProcessorEditor::setParameterValue (const String& paramId, float value)
{
    if (AudioProcessorParameter* param = getParameter (paramId))
        param->setValueNotifyingHost(value);
}

void SofaPanAudioProcessorEditor::buttonClicked(Button *button)
{
    
    if (button == &loadSOFAButton) {
        FileChooser fc ("Choose HRTF!",
                        File::getCurrentWorkingDirectory(),
                        "*.sofa",
                        true);
        
        if (fc.browseForFileToOpen()){
            String chosen = fc.getResult().getFullPathName();
            processor.setSOFAFilePath(chosen);
            processor.initData(chosen);
        }
    }
    
    if (button == &bypassButton)
        setParameterValue("bypass", bypassButton.getToggleState());
    
    if (button == &testSwitchButton)
        setParameterValue("test", testSwitchButton.getToggleState());
    
    if(button == &showSOFAMetadataButton)
        metadataView.setVisible(true);
}


