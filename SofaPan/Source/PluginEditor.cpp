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
    : AudioProcessorEditor (&p), processor (p), panner2D_top(p, false), panner2D_rear(p, true)
{
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    LookAndFeel::setDefaultLookAndFeel(&juceDefaultLookAndFeel);
    
    
    setSize (900, 500);
    
    panner_az.setSliderStyle(Slider::Rotary);
    panner_az.setComponentID("1");
    panner_az.setLookAndFeel(&azSliderLookAndFeel);
    panner_az.setRange(0.0, 359.0, 0.1);
    panner_az.setTextValueSuffix(" deg");
    panner_az.setPopupDisplayEnabled(false, false, this);
    panner_az.setTextBoxStyle(Slider::TextBoxBelow, false, 70, 15);
    panner_az.setColour(Slider::textBoxBackgroundColourId, Colours::white);
    panner_az.setRotaryParameters(0, M_PI*2.0, false);
    panner_az.setDoubleClickReturnValue(true, 0.0);
    panner_az.addListener(this);
    addAndMakeVisible(&panner_az);

    
    panner_el.setSliderStyle(Slider::Rotary);
    panner_el.setComponentID("2");
    panner_el.setLookAndFeel(&elSliderLookAndFeel);
    panner_el.setRange(-90.0, 90.0, 1.0);
    panner_el.setTextValueSuffix(" deg");
    panner_el.setPopupDisplayEnabled(false, false, this);
    panner_el.setTextBoxStyle(Slider::TextBoxBelow, false, 70, 15);
    panner_el.setColour(Slider::textBoxBackgroundColourId, Colours::white);
    panner_el.setRotaryParameters(M_PI, M_PI*2, true);
    panner_el.setDoubleClickReturnValue(true, 0.0);
    panner_el.addListener(this);
    addAndMakeVisible(&panner_el);
    
    panner_dist.setSliderStyle(roomsimLayout ? Slider::LinearHorizontal : Slider::LinearVertical);
    panner_dist.setLookAndFeel(&sofaPanLookAndFeel);
    panner_dist.setRange(0.0, 1.0);
    panner_dist.setTextValueSuffix(" m");
    panner_dist.setPopupDisplayEnabled(false, false, this);
    panner_dist.setTextBoxStyle(Slider::TextBoxBelow, false, 70, 15);
    panner_dist.setDoubleClickReturnValue(true, 1.0);
    panner_dist.addListener(this);
    addAndMakeVisible(&panner_dist);
    
    loadSOFAButton.setButtonText ("Load HRTF");
    loadSOFAButton.setComponentID("3");
    loadSOFAButton.addListener(this);
    loadSOFAButton.setLookAndFeel(&juceDefaultLookAndFeel);
    loadSOFAButton.setColour (TextButton::buttonColourId, sofaPanLookAndFeel.mainCyan);
    loadSOFAButton.setColour (TextButton::textColourOffId, Colours::white);
    loadSOFAButton.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
    addAndMakeVisible(&loadSOFAButton);

#if ENABLE_TESTBUTTON
    testSwitchButton.setButtonText("Test Button");
    testSwitchButton.setColour(ToggleButton::textColourId, Colours::white);
    testSwitchButton.addListener(this);
    addAndMakeVisible(&testSwitchButton);
#endif
    
#if ENABLE_SEMISTATICS

    mirrorSourceReflectionsButton.setButtonText("Mirror Source");
    mirrorSourceReflectionsButton.setComponentID("mirrorSourceButton");
    mirrorSourceReflectionsButton.setLookAndFeel(&juceDefaultLookAndFeel);
    mirrorSourceReflectionsButton.setClickingTogglesState(true);
    mirrorSourceReflectionsButton.setToggleState(true, juce::dontSendNotification);
    mirrorSourceReflectionsButton.setColour(ToggleButton::textColourId, Colours::white);
    mirrorSourceReflectionsButton.addListener(this);
    mirrorSourceReflectionsButton.addMouseListener(this, true);
    mirrorSourceReflectionsButton.setColour (TextButton::buttonColourId, Colours::lightgrey);
    mirrorSourceReflectionsButton.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
    mirrorSourceReflectionsButton.setColour (TextButton::textColourOffId, Colours::grey);
    mirrorSourceReflectionsButton.setRadioGroupId(9174);
    mirrorSourceReflectionsButton.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
    addAndMakeVisible(&mirrorSourceReflectionsButton);
    
    semiStaticReflectionsButton.setButtonText("Semi Static");
    semiStaticReflectionsButton.setComponentID("semiStaticButton");
    semiStaticReflectionsButton.setLookAndFeel(&juceDefaultLookAndFeel);
    semiStaticReflectionsButton.setClickingTogglesState(true);
    semiStaticReflectionsButton.setToggleState(false, juce::dontSendNotification);
    semiStaticReflectionsButton.setColour(ToggleButton::textColourId, Colours::white);
    semiStaticReflectionsButton.addListener(this);
    semiStaticReflectionsButton.addMouseListener(this, true);
    semiStaticReflectionsButton.setColour (TextButton::buttonColourId, Colours::lightgrey);
    semiStaticReflectionsButton.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
    semiStaticReflectionsButton.setColour (TextButton::textColourOffId, Colours::grey);
    semiStaticReflectionsButton.setRadioGroupId(9174);
    semiStaticReflectionsButton.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
    addAndMakeVisible(&semiStaticReflectionsButton);
    
#endif
    
    useDistanceSimulationButton.setButtonText("Distance Simulation");
    useDistanceSimulationButton.setComponentID("distSimButton");
    useDistanceSimulationButton.setColour(ToggleButton::textColourId, Colours::white);
    useDistanceSimulationButton.addListener(this);
    useDistanceSimulationButton.addMouseListener(this, true);
    addAndMakeVisible(&useDistanceSimulationButton);
    useDistanceSimulationButton.setToggleState(true, NotificationType::sendNotification);
    
    useNearfieldSimulationButton.setButtonText("Nearfield Simulation");
    useNearfieldSimulationButton.setComponentID("nfSimButton");
    useNearfieldSimulationButton.setColour(ToggleButton::textColourId, Colours::white);
    useNearfieldSimulationButton.addListener(this);
    useNearfieldSimulationButton.addMouseListener(this, true);
    addAndMakeVisible(&useNearfieldSimulationButton);
    useNearfieldSimulationButton.setToggleState(true, NotificationType::sendNotification);
    
    ITDadjustButton.setButtonText("ITD Adjustment");
    ITDadjustButton.setColour(ToggleButton::textColourId, Colours::white);
    ITDadjustButton.addListener(this);
    addAndMakeVisible(&ITDadjustButton);
    
    
    headRadiusSlider.setSliderStyle(Slider::LinearHorizontal);
    headRadiusSlider.setLookAndFeel(&juceDefaultLookAndFeel);
    headRadiusSlider.setRange(14.0, 20.0);
    headRadiusSlider.setTextValueSuffix(" cm");
    headRadiusSlider.setPopupDisplayEnabled(true, true, this);
    headRadiusSlider.setTextBoxStyle(Slider::TextBoxLeft, false, 50, 15);
    headRadiusSlider.addListener(this);
    addAndMakeVisible(&headRadiusSlider);
    
    
    useGlobalSofaFileButton.setButtonText("Global SOFA File");
    useGlobalSofaFileButton.setComponentID("globalSofaButton");
    useGlobalSofaFileButton.setColour(ToggleButton::textColourId, Colours::white);
    useGlobalSofaFileButton.addListener(this);
    useGlobalSofaFileButton.addMouseListener(this, true);
    addAndMakeVisible(&useGlobalSofaFileButton);
    
    addAndMakeVisible(&plotHRTFView);
    addAndMakeVisible(&plotHRIRView);
    addAndMakeVisible(&panner2D_top);
    addAndMakeVisible(&panner2D_rear);
    
    if(!roomsimLayout){
        panner2D_top.setVisible(false);
        panner2D_rear.setVisible(false);
    }else{
        plotHRIRView.setVisible(false);
        plotHRTFView.setVisible(false);
    }
    

    showTooltipsButton.setButtonText("Show Mouseover Tooltips");
    addAndMakeVisible(&showTooltipsButton);
    
    
    useLayoutSimplePanningButton.setButtonText("Simple Panning");
    useLayoutSimplePanningButton.setClickingTogglesState(true);
    useLayoutSimplePanningButton.setRadioGroupId(1111);
    useLayoutSimplePanningButton.setColour (TextButton::buttonColourId, Colours::lightgrey);
    useLayoutSimplePanningButton.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
    useLayoutSimplePanningButton.setColour (TextButton::textColourOffId, Colours::grey);
    useLayoutSimplePanningButton.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
    useLayoutSimplePanningButton.setToggleState(true, dontSendNotification);
    useLayoutSimplePanningButton.setLookAndFeel(&juceDefaultLookAndFeel);
    useLayoutSimplePanningButton.addListener(this);
    addAndMakeVisible(&useLayoutSimplePanningButton);
    
    useLayoutRoomsimButton.setButtonText("Room Simulation");
    useLayoutRoomsimButton.setClickingTogglesState(true);
    useLayoutRoomsimButton.setRadioGroupId(1111);
    useLayoutRoomsimButton.setColour (TextButton::buttonColourId, Colours::lightgrey);
    useLayoutRoomsimButton.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
    useLayoutRoomsimButton.setColour (TextButton::textColourOffId, Colours::grey);
    useLayoutRoomsimButton.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
    useLayoutRoomsimButton.setToggleState(false, dontSendNotification);
    useLayoutRoomsimButton.setLookAndFeel(&juceDefaultLookAndFeel);
    useLayoutRoomsimButton.addListener(this);
    addAndMakeVisible(useLayoutRoomsimButton);
    
    showSOFAMetadataButton.setButtonText("Show More Information");
    showSOFAMetadataButton.addListener(this);
    showSOFAMetadataButton.setLookAndFeel(&juceDefaultLookAndFeel);
    showSOFAMetadataButton.setColour (TextButton::buttonColourId, sofaPanLookAndFeel.mainCyan);
    showSOFAMetadataButton.setColour (TextButton::textColourOffId, Colours::white);
    showSOFAMetadataButton.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
    addAndMakeVisible(&showSOFAMetadataButton);
    
    
#if REVSLIDERACTIVE
    reverbSlider1.setSliderStyle(Slider::LinearHorizontal);
    reverbSlider1.setLookAndFeel(&juceDefaultLookAndFeel);
    reverbSlider1.setRange(-48, -12);
    reverbSlider1.setTextValueSuffix(" dB");
    reverbSlider1.setPopupDisplayEnabled(true, true, this);
    reverbSlider1.setTextBoxStyle(Slider::NoTextBox, false, 30, 15);
    reverbSlider1.addListener(this);
    addAndMakeVisible(&reverbSlider1);
    
    reverbSlider2.setSliderStyle(Slider::LinearHorizontal);
    reverbSlider2.setLookAndFeel(&juceDefaultLookAndFeel);
    reverbSlider2.setRange(0.1, 2);
    reverbSlider2.setTextValueSuffix(" s");
    reverbSlider2.setPopupDisplayEnabled(true, true, this);
    reverbSlider2.setTextBoxStyle(Slider::NoTextBox, false, 30, 15);
    reverbSlider2.addListener(this);
    addAndMakeVisible(&reverbSlider2);
    
    reverbLabel1.setText("Reverb Amount", NotificationType::dontSendNotification);
    reverbLabel2.setText("Reverb Decay", NotificationType::dontSendNotification);
    if(!ENABLE_SEMISTATICS){
        addAndMakeVisible(&reverbLabel1);
        addAndMakeVisible(&reverbLabel2);
    }

    
#endif
  
    addAndMakeVisible(&metadataView);
    
    metadataView.setVisible(false);
    
    counter = 0;
    startTimer(50);
    lastAzimuthValue = 0.0;
    lastElevationValue = 0.0;
    lastDistanceValue = 0.0;

    
    //When the Editor is reloaded, the sofaMetadata is automtically omitted and has to be reloaded
    processor.updateSofaMetadataFlag = true;
    
    popUpInfo = new BubbleMessageComponent();
    popUpInfo->setAlwaysOnTop (true);
    popUpInfo->addToDesktop(0);
    
    if(!connect(9001))
       std::cout << "ERROR CONNECT";
    
    addListener (this, "/sofapan/rotaryknob");
    addListener (this, "/sofapan/connectHandshake");
 
    
    
    String buildDateAndTimeString = String("Build Date: ") + String(__DATE__) + String(" ") + String(__TIME__);
    
    String buildVersionString = String("  Vers.") + String(JucePlugin_Version);

    buildDateAndTimeLabel.setText(buildDateAndTimeString, NotificationType::dontSendNotification);
    buildDateAndTimeLabel.setFont(Font(8));
    addAndMakeVisible(buildDateAndTimeLabel);
    buildVersionLabel.setText(buildVersionString, NotificationType::dontSendNotification);
    buildVersionLabel.setFont(Font(8));
    addAndMakeVisible(buildVersionLabel);
    
    rearrange();
}

SofaPanAudioProcessorEditor::~SofaPanAudioProcessorEditor()
{
    
}


void SofaPanAudioProcessorEditor::paint (Graphics& g)
{
    

    g.drawImage(backgroundImage, getLocalBounds().toFloat());
    
    // sofa metadata short info
    g.setColour(Colours::white);
    g.setFont(Font(11));
    g.drawFittedText(sofaMetadataID, 10, 60, 100, 100, Justification::topLeft, 3);
    g.drawFittedText(sofaMetadataValue, 110, 60, 300, 100, Justification::topLeft, 3);
    
}

void SofaPanAudioProcessorEditor::resized()
{
   
    rearrange();
}

// This timer periodically checks whether any of the filter's parameters have changed...
void SofaPanAudioProcessorEditor::timerCallback() {

    ITDadjustButton.setToggleState((bool)getParameterValue("ITDadjust"), NotificationType::dontSendNotification);
#if ENABLE_TESTBUTTON
    testSwitchButton.setToggleState((bool)getParameterValue("test"), NotificationType::dontSendNotification);
#endif
    
#if ENABLE_SEMISTATICS
    mirrorSourceReflectionsButton.setToggleState((bool)getParameterValue("mirrorSource"), NotificationType::dontSendNotification);
    semiStaticReflectionsButton.setToggleState(!(bool)getParameterValue("mirrorSource"), NotificationType::dontSendNotification);
#endif
    
    useGlobalSofaFileButton.setToggleState(processor.getUsingGlobalSofaFile(), NotificationType::dontSendNotification);
    
    bool distanceSimActive = (bool)getParameterValue("dist_sim");
    bool nearfieldSimActive = (bool)getParameterValue("nearfield_sim");
    useDistanceSimulationButton.setToggleState(distanceSimActive, NotificationType::dontSendNotification);
    useNearfieldSimulationButton.setToggleState(nearfieldSimActive, NotificationType::dontSendNotification);
    
    headRadiusSlider.setValue((float)getParameterValue("individualHeadDiameter") * 6.0 + 14.0);
    headRadiusSlider.setVisible(ITDadjustButton.getToggleState());
    
#if REVSLIDERACTIVE
    reverbSlider1.setValue((float)getParameterValue("reverbParam1") * 36.0 - 48);
    reverbSlider2.setValue((float)getParameterValue("reverbParam2") * 1.9 + 0.1);
#endif
    
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
        //printf("\n\n 1. Trigger Repaint! \n");
        if(!roomsimLayout){
//            int complexLength = processor.getComplexLength();
//            int firLength = complexLength - 1;
            int firLength = processor.getLengthOfHRIR();
            int complexLength = firLength + 1;
            int sampleRate = processor.getSampleRate();
            plotHRIRView.drawHRIR(processor.getCurrentHRIR(), firLength, sampleRate, processor.getCurrentITD());
            plotHRTFView.drawHRTF(processor.getCurrentMagSpectrum(), processor.getCurrentPhaseSpectrum(), complexLength, sampleRate);
        }
        panner2D_top.setDistanceAndAngle(distanceValue, azimuthValue, elevationValue);
        panner2D_rear.setDistanceAndAngle(distanceValue, azimuthValue, elevationValue);


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
        String earDistanceNote = String(processor.metadata_sofafile.headRadius * 200);
        earDistanceNote.append(" cm", 3);
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
            panner2D_rear.setHasElevation(true);

        }else{
            panner_el.setRotaryParameters((270. + eleMin) * deg2rad , (270. + eleMax) * deg2rad, true);
            elevationRange_Note = "none";
            panner_el.setValue(0.0);
            panner_el.setEnabled(false);
            panner2D_rear.setHasElevation(false);
        }
        float distMin = processor.metadata_sofafile.minDistance;
        float distMax = processor.metadata_sofafile.maxDistance;
        
        //distance-simulation overrides measured distance data
        if(distanceSimActive){
            distMin = simulationDistanceMin;
            distMax = simulationDistanceMax;
        }
        
        String distanceRange_Note;
        String distanceMin = String(distMin);
        String distanceMax = String(distMax);
        if(distMin != distMax){
            if(distanceSimActive)
                distanceRange_Note = (distanceMin + "m to " + distanceMax + "m (sim)" );
            else
                distanceRange_Note = (distanceMin + "m to " + distanceMax + "m" );
            panner_dist.setEnabled(true);
            panner_dist.setRange(distMin, distMax);
        }else{
            distanceRange_Note = (distanceMin + "m ");
            panner_dist.setValue(distMax);
            panner_dist.setRange(distMin, distMax);
            panner_dist.setEnabled(false);
        }
    
        
        sofaMetadataValue = String(listenerShortName_Note + "\n" +
                                   earDistanceNote + "\n" +
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
    
    // if the nearfield simulation is active, possible distance information of the loaded sofafile will be omitted and the simulation will become active. The simulation will use one single distance (most likely 1m) of the file for its simulation.
    if(distanceSimActive){
        panner_dist.setEnabled(true);
        panner_dist.setRange(simulationDistanceMin, simulationDistanceMax);
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
    if(slider == &headRadiusSlider){
        setParameterValue("individualHeadDiameter", (headRadiusSlider.getValue() - 14.0) / 6.0);
    }
    if(slider == &reverbSlider1){
        setParameterValue("reverbParam1", (reverbSlider1.getValue() + 48.0) / 36.0);
    }
    if(slider == &reverbSlider2){
        setParameterValue("reverbParam2", (reverbSlider2.getValue() - 0.1) / 1.9);
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
            

        }
    }
    
    if (button == &ITDadjustButton)
        setParameterValue("ITDadjust", ITDadjustButton.getToggleState());
    
#if ENABLE_TESTBUTTON
    if (button == &testSwitchButton)
        setParameterValue("test", testSwitchButton.getToggleState());
#endif
    
#if ENABLE_SEMISTATICS

    if (button == &mirrorSourceReflectionsButton || button == &semiStaticReflectionsButton)
        setParameterValue("mirrorSource", mirrorSourceReflectionsButton.getToggleState());
#endif
    
    if(button == &showSOFAMetadataButton)
        metadataView.setVisible(true);
    
    if(button == &useGlobalSofaFileButton)
        processor.setUsingGlobalSofaFile(useGlobalSofaFileButton.getToggleState());
    
    if(button == &useDistanceSimulationButton){
        setParameterValue("dist_sim", useDistanceSimulationButton.getToggleState());
        if(!useDistanceSimulationButton.getToggleState())
            setParameterValue("nearfield_sim", false);
        processor.updateSofaMetadataFlag = true;
    }
    
    if(button == &useNearfieldSimulationButton){
        setParameterValue("nearfield_sim", useNearfieldSimulationButton.getToggleState());
        if(useNearfieldSimulationButton.getToggleState())
            setParameterValue("dist_sim", true);
        processor.updateSofaMetadataFlag = true;
    }
    
    if(button == &useLayoutRoomsimButton){
        roomsimLayout = 1;
        rearrange();

        plotHRTFView.setVisible(false);
        plotHRIRView.setVisible(false);
        panner2D_top.setVisible(true);
        panner2D_rear.setVisible(true);
        panner_dist.setSliderStyle(Slider::LinearHorizontal);
        repaint();
        setParameterValue("dist_sim", true);

    }
    if(button == & useLayoutSimplePanningButton){
        roomsimLayout = 0;
        rearrange();
        plotHRTFView.setVisible(true);
        plotHRIRView.setVisible(true);
        panner2D_top.setVisible(false);
        panner2D_rear.setVisible(false);
        panner_dist.setSliderStyle(Slider::LinearVertical);
        repaint();
        setParameterValue("dist_sim", false);
        setParameterValue("nearfield_sim", false);

    }
    
    
    

}

void SofaPanAudioProcessorEditor::mouseEnter(const MouseEvent &e){
    
    if(showTooltipsButton.getToggleState()){
        if(e.eventComponent->getComponentID() == "globalSofaButton"){

            AttributedString text ("By activating this option, a single SOFA file will be used by every plugin instance running on this host that has this option activated. \n Deactivating this option in one ore more instances excludes them from this feature.");
            text.setJustification (Justification::centred);
            text.setColour(Colours::white);
            popUpInfo->showAt(e.eventComponent, text, 10000, true, false);
            
        }
        if(e.eventComponent->getComponentID() == "distSimButton"){
            
            AttributedString text ("By activating this option, it is possible to conrol the distance of the source even without a multiple-distance SOFA-File. The distance is modeled with various effects, like early reflections of a simulated shoebox room. \n The distance slider becomes active and can be adjusted from 0.2m to 5m. \n If the loaded SOFA file has already multiple distance sets, those will be deactivated and the set with the distance nearest to 1m will be used for the simulation");
            text.setJustification (Justification::centred);
            text.setColour(Colours::white);
            popUpInfo->showAt(e.eventComponent, text, 10000, true, false);
            
        }
        
        if(e.eventComponent->getComponentID() == "nfSimButton"){
            
            AttributedString text ("By activating this option, the effect of the source approaching the head will be simulated. \n The nearfield effects are occuring in the range 0.2m to 1m. \n Note that this option is only available in comination with the distance simulation");
            text.setJustification (Justification::centred);
            text.setColour(Colours::white);
            popUpInfo->showAt(e.eventComponent, text, 10000, true, false);
            
        }
        
        if(e.eventComponent->getComponentID() == "mirrorSourceButton"){
            
            AttributedString text ("When in RoomSim mode, the modeling of early reflections can be set to the 'Image Source' model.\nThis model creates discrete early reflections as binaural sources with a geometrically computed source position. \nWhen this option is not active, a semi-static appoximation of four early reflections is used, where a blending of damping and delay between reflections from 4 directions (fixed binaural sources) is used");
            text.setJustification (Justification::centred);
            text.setColour(Colours::white);
            popUpInfo->showAt(e.eventComponent, text, 10000, true, false);
            
        }
    }

}


void SofaPanAudioProcessorEditor::mouseExit(const MouseEvent &e){
    popUpInfo->setVisible(false);
    
}

void SofaPanAudioProcessorEditor::rearrange(){
    
    buildVersionLabel.setBounds(getLocalBounds().getRight()-100, getLocalBounds().getBottom()-10, 100, 10);
    buildDateAndTimeLabel.setBounds(buildVersionLabel.getBounds().translated(-100, 0));
    
    metadataView.setBounds(getLocalBounds().reduced(20));

    loadSOFAButton.setBounds(10., 10., 130., 30.);
    useGlobalSofaFileButton.setBounds(loadSOFAButton.getBounds().getRight() + 10, 10., 80, 30);
    //bypassButton.setBounds(10., 50., 150., 30.);

    
    showSOFAMetadataButton.setBounds(10., 160, 160, 30);
    Rectangle<int> distanceSimControlBox = Rectangle<int>(10, showSOFAMetadataButton.getBottom() + 20,220, 90);
    useDistanceSimulationButton.setBounds(distanceSimControlBox.getX() + 10, distanceSimControlBox.getY() + 10, 100, 30);
    useNearfieldSimulationButton.setBounds(20., useDistanceSimulationButton.getBottom(), 100., 30.);
#if ENABLE_SEMISTATICS

    mirrorSourceReflectionsButton.setBounds(useDistanceSimulationButton.getRight()+10, useDistanceSimulationButton.getY() + 5, 45., 20.);
    semiStaticReflectionsButton.setBounds(mirrorSourceReflectionsButton.getBounds().withX(mirrorSourceReflectionsButton.getRight()));
#endif

#if REVSLIDERACTIVE
        Rectangle<int> spaceForRevSliders = distanceSimControlBox.withLeft(useNearfieldSimulationButton.getRight());
        reverbLabel1.setBounds(spaceForRevSliders.getX(), spaceForRevSliders.getY()+5, spaceForRevSliders.getWidth()*0.9, spaceForRevSliders.getHeight() / 8);
        reverbSlider1.setBounds(spaceForRevSliders.getX(), reverbLabel1.getBottom(), spaceForRevSliders.getWidth(), spaceForRevSliders.getHeight() / 3);
        reverbLabel2.setBounds(spaceForRevSliders.getX(), reverbSlider1.getBottom(), spaceForRevSliders.getWidth()*0.9, spaceForRevSliders.getHeight() / 8);
        reverbSlider2.setBounds(spaceForRevSliders.getX(), reverbLabel2.getBottom(), spaceForRevSliders.getWidth(), spaceForRevSliders.getHeight() / 3);
#if ENABLE_SEMISTATICS

        spaceForRevSliders = distanceSimControlBox.withLeft(useNearfieldSimulationButton.getRight()).withTop(mirrorSourceReflectionsButton.getBottom());
        reverbSlider1.setBounds(spaceForRevSliders.removeFromTop(spaceForRevSliders.getHeight()/2));
        reverbSlider2.setBounds(spaceForRevSliders);
#endif
    
#endif
    Rectangle<int> ITDAdjustControlBox = Rectangle<int>(distanceSimControlBox.withY(distanceSimControlBox.getBottom() + 10));
    ITDadjustButton.setBounds(ITDAdjustControlBox.getX() + 10, ITDAdjustControlBox.getY() + 10, 130., 30.);
    //ITDadjustButton.setBounds(ITDAdjustControlBox.removeFromTop(ITDAdjustControlBox.getHeight()/2).reduced(10));
    
    headRadiusSlider.setBounds(ITDAdjustControlBox.getX() + 20, ITDAdjustControlBox.getY() + 50, ITDAdjustControlBox.getWidth() - 30, 30);
    
#if ENABLE_TESTBUTTON
    testSwitchButton.setBounds(10., ITDAdjustControlBox.getBottom() + 10, 100., 30.);
#endif
    
    showTooltipsButton.setBounds(100, getLocalBounds().getBottom() - 30, 100, 30);

    
    
    int panner_size;
    const int spacing = 30;

    const Rectangle<int> mainControlBox = Rectangle<int>(240, spacing, getWidth() - 260, getHeight() - 70);
    Rectangle<int> boxWithSpacing = mainControlBox.reduced(spacing, 10);

    Rectangle<int> azimuthLabel, elevationLabel, distanceLabel, topViewLabel, rearViewLabel, hrtfLabel, hrirLabel;
    
    if(roomsimLayout) {
        int panner2D_size = (boxWithSpacing.getWidth() - spacing) / 2.0;

        topViewLabel = boxWithSpacing.withSize(panner2D_size, 30);
        rearViewLabel = topViewLabel.translated(panner2D_size + spacing, 0);
        
        panner2D_top.setBounds(topViewLabel.getX(), topViewLabel.getBottom(), panner2D_size, panner2D_size);
        panner2D_rear.setBounds(rearViewLabel.getX(), rearViewLabel.getBottom(), panner2D_size, panner2D_size);
        
        panner_size = 80;
        panner_az.setBounds(panner2D_top.getX(), panner2D_top.getBottom() + spacing, panner_size, panner_size);
        panner_el.setBounds(panner_az.getRight() + spacing, panner_az.getY(), panner_size, panner_size);
        panner_dist.setBounds(panner_el.getRight() + spacing, panner_az.getY(), panner_size * 3, panner_size);
        azimuthLabel.setBounds(panner_az.getX(), panner_az.getY() - 20, panner_az.getWidth(), 20);
        elevationLabel.setBounds(panner_el.getX(), panner_el.getY() - 20, panner_el.getWidth(), 20);
        distanceLabel.setBounds(panner_dist.getX(), panner_dist.getY() - 20, panner_dist.getWidth(), 20);

        
    }else{
        
        panner_size = (boxWithSpacing.getWidth() - spacing * 2) / 2.5;
        
        Rectangle<int>boxForTopLabels = boxWithSpacing.removeFromTop(30);
        Rectangle<int>boxForSliders = boxWithSpacing.removeFromTop(panner_size);
        
        azimuthLabel = boxForTopLabels.removeFromLeft(panner_size);
        elevationLabel = azimuthLabel.translated(panner_size + spacing, 0);
        distanceLabel = elevationLabel.translated(panner_size + spacing, 0).withWidth(panner_size / 2);

        panner_az.setBounds(boxForSliders.removeFromLeft(panner_size));
        panner_el.setBounds(panner_az.getBounds().translated(panner_size + spacing, 0));
        panner_dist.setBounds(panner_el.getBounds().translated(panner_size + spacing, 0).withWidth(panner_size/2));
        
        int plotWidth = (boxWithSpacing.getWidth() - spacing) / 2;
        boxWithSpacing.removeFromTop(spacing);
        Rectangle<int> boxForPlotLabels = boxWithSpacing.removeFromTop(spacing);
        hrirLabel = boxForPlotLabels.removeFromLeft(plotWidth);
        hrtfLabel = hrirLabel.translated(plotWidth + spacing, 0);
        
        plotHRIRView.setBounds(boxWithSpacing.removeFromLeft(plotWidth));
        plotHRTFView.setBounds(plotHRIRView.getBounds().translated(plotWidth + spacing, 0));
        
        
    }
    
    const float modeSelectionBoxWidth = getWidth() / 4.0;
    Rectangle<int> modeSelectionBox = Rectangle<int>(mainControlBox.getX(), mainControlBox.getY()-20, modeSelectionBoxWidth, 20);
    useLayoutSimplePanningButton.setBounds(modeSelectionBox.withWidth(modeSelectionBoxWidth/2.0));
    useLayoutRoomsimButton.setBounds(modeSelectionBox.withWidth(modeSelectionBoxWidth/2.0).translated(modeSelectionBoxWidth/2.0, 0));
    
    //Create Background Image
    backgroundImage = Image(Image::RGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);
    Graphics g(backgroundImage);
    
    g.fillAll (Colour(0xFF222222));
    g.setFont (15.0f);
    g.setColour(Colours::white);
    g.setOpacity(1.0f);
    g.drawImageAt(logoHSD, 0, getHeight()-40, true);
    
    g.setColour(Colour(0xFF000000));
    g.fillRect(mainControlBox);
    g.setColour(Colours::grey);
    g.drawRect(mainControlBox);
    
    g.setColour(Colour(0xFF2A2A2A));
    g.fillRect(distanceSimControlBox);
    g.fillRect(ITDAdjustControlBox);
    g.setColour(Colours::grey);
    g.drawRect(distanceSimControlBox);
    g.drawRect(ITDAdjustControlBox);

    
    g.setColour(Colours::white);
    
    if(!roomsimLayout){
        // azimuth slider
        Rectangle<int> rect = panner_az.getBounds();
        int newWidth = rect.getWidth() - panner_az.getTextBoxHeight(); //always quadratic
        rect.setSize(newWidth, newWidth);
        //const int pngPixelAdjust = 0;//2;
        g.drawImage(headTopImage,
                    rect.getCentreX()-rect.getWidth()/4 + 10 ,
                    rect.getCentreY()-rect.getHeight()/4 + 10,// - pngPixelAdjust,
                    rect.getWidth()/2 - 20,
                    rect.getWidth()/2 - 20,
                    0, 0, 200, 200);
        
        // elevation slider
        rect = panner_el.getBounds();
        newWidth = rect.getWidth() - panner_el.getTextBoxHeight();
        rect.setSize(newWidth, newWidth);
        g.drawImage(headSideImage,
                    rect.getCentreX()-rect.getWidth()/4 + 10,
                    rect.getCentreY()-rect.getHeight()/4 + 10,
                    rect.getWidth()/2 - 20,
                    rect.getWidth()/2 - 20,
                    0, 0, 200, 200);
        
        // labels for panning sliders
        g.setFont(Font(20.f));
        g.drawText("Azimuth", azimuthLabel, juce::Justification::topLeft);
        g.drawText("Elevation", elevationLabel, juce::Justification::topLeft);
        g.drawText("Distance", distanceLabel, juce::Justification::topLeft);
        g.setFont(Font(15.f));
        g.drawText("HRIR", hrirLabel, Justification::centredLeft);
        g.drawText("HRTF", hrtfLabel, Justification::centredLeft);
                   

    }else{
        g.setFont(Font(12.f));
        g.drawText("Azimuth", azimuthLabel, juce::Justification::topLeft);
        g.drawText("Elevation", elevationLabel, juce::Justification::topLeft);
        g.drawText("Distance", distanceLabel, juce::Justification::topLeft);
        g.setFont(Font(20.f));

        g.drawText("Top View", topViewLabel, Justification::topLeft);
        g.drawText("Rear View", rearViewLabel, Justification::topLeft);
        
    }
}

void SofaPanAudioProcessorEditor::oscMessageReceived(const OSCMessage& message){
    
    
//    if (message.size() == 1 && message[0].isFloat32())
//        rotaryKnob.setValue (jlimit (0.0f, 10.0f, message[0].getFloat32()));
//
//
    if(message.getAddressPattern().matches("/sofapan/connectHandshake") && message[0].isString()){
        if(message[0].getString().contains("SYN")){
            std::cout << "\n\nMessageReceived!" << message[0].getString().substring(4);

            oscSender.connect(message[0].getString().substring(4), 9001);
            oscSender.send("/sofapan/connectHandshake", String("ACK"));

        }
    }
    
}
