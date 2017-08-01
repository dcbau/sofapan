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
    : AudioProcessorEditor (&p), processor (p), panner2D(p, false), panner2D_elev(p, true)
{
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    LookAndFeel::setDefaultLookAndFeel(&sofaPanLookAndFeel);
    
    
    setSize (900, 500);
    
    panner_az.setSliderStyle(Slider::Rotary);
    panner_az.setComponentID("1");
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
    panner_el.setComponentID("2");
    panner_el.setLookAndFeel(&elSliderLookAndFeel);
    panner_el.setRange(-90.0, 90.0, 1.0);
    panner_el.setTextValueSuffix(" deg");
    panner_el.setPopupDisplayEnabled(false, this);
    panner_el.setTextBoxStyle(Slider::TextBoxBelow, false, 70, 15);
    panner_el.setColour(Slider::textBoxBackgroundColourId, Colours::white);
    panner_el.setRotaryParameters(M_PI, M_PI*2, true);
    panner_el.addListener(this);
    addAndMakeVisible(&panner_el);
    
    panner_dist.setSliderStyle(roomsimLayout ? Slider::LinearHorizontal : Slider::LinearVertical);
    panner_dist.setRange(0.0, 1.0);
    panner_dist.setTextValueSuffix(" m");
    panner_dist.setPopupDisplayEnabled(false, this);
    panner_dist.setTextBoxStyle(Slider::TextBoxBelow, false, 70, 15);
    panner_dist.addListener(this);
    addAndMakeVisible(&panner_dist);
    
    loadSOFAButton.setButtonText ("Load HRTF");
    loadSOFAButton.setComponentID("3");
    loadSOFAButton.addListener(this);
    addAndMakeVisible(&loadSOFAButton);
    
    bypassButton.setButtonText("Bypass");
    bypassButton.setColour(ToggleButton::textColourId, Colours::white);
    bypassButton.addListener(this);
    addAndMakeVisible(&bypassButton);
    
    testSwitchButton.setButtonText("Test Switch");
    testSwitchButton.setColour(ToggleButton::textColourId, Colours::white);
    testSwitchButton.addListener(this);
    //addAndMakeVisible(&testSwitchButton);
    
    useDistanceSimulationButton.setButtonText("Nearfield Simulation");
    useDistanceSimulationButton.setComponentID("nfSimButton");
    useDistanceSimulationButton.setColour(ToggleButton::textColourId, Colours::white);
    useDistanceSimulationButton.addListener(this);
    useDistanceSimulationButton.addMouseListener(this, true);
    addAndMakeVisible(&useDistanceSimulationButton);
    useDistanceSimulationButton.setToggleState(true, NotificationType::sendNotification);
    
    useGlobalSofaFileButton.setButtonText("Global SOFA File");
    useGlobalSofaFileButton.setComponentID("globalSofaButton");
    useGlobalSofaFileButton.setColour(ToggleButton::textColourId, Colours::white);
    useGlobalSofaFileButton.addListener(this);
    useGlobalSofaFileButton.addMouseListener(this, true);
    addAndMakeVisible(&useGlobalSofaFileButton);
    
    if(!roomsimLayout){
        addAndMakeVisible(&plotHRTFView);
        addAndMakeVisible(&plotHRIRView);
    }else{
        addAndMakeVisible(&panner2D);
        addAndMakeVisible(&panner2D_elev);
    }
    
    showSOFAMetadataButton.setButtonText("Show More Information");
    showSOFAMetadataButton.addListener(this);
    addAndMakeVisible(&showSOFAMetadataButton);
    addAndMakeVisible(&metadataView);
    metadataView.setVisible(false);
    
    showTooltipsButton.setButtonText("Show Mouseover Tooltips");
    addAndMakeVisible(&showTooltipsButton);
    
    
    useLayoutSOFAPlayerButton.setButtonText("SOFA Player");
    useLayoutSOFAPlayerButton.setClickingTogglesState(true);
    useLayoutSOFAPlayerButton.setRadioGroupId(1111);
    useLayoutSOFAPlayerButton.setColour (TextButton::buttonColourId, Colours::lightgrey);
    useLayoutSOFAPlayerButton.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
    useLayoutSOFAPlayerButton.setColour (TextButton::textColourOffId, Colours::grey);
    useLayoutSOFAPlayerButton.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
    useLayoutSOFAPlayerButton.setToggleState(true, dontSendNotification);
    useLayoutSOFAPlayerButton.setLookAndFeel(&juceDefaultLookAndFeel);
    useLayoutSOFAPlayerButton.addListener(this);
    addAndMakeVisible(&useLayoutSOFAPlayerButton);
    
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
    
    
 
    rearrange();
}

SofaPanAudioProcessorEditor::~SofaPanAudioProcessorEditor()
{
    
}


void SofaPanAudioProcessorEditor::paint (Graphics& g)
{
    

    g.drawImage(backgroundImage, getLocalBounds().toFloat());
    
    
    // rotating speaker image
//    const int speakerSize = 40;
//    const int spaceBetweenSpeakerAndSlider = 6;
//    float sliderPosition = (float)panner_az.getValue() / 360.0;
//    AffineTransform t = AffineTransform().translated(rect.getCentreX()- speakerSize/2 ,
//                 rect.getY() - speakerSize - spaceBetweenSpeakerAndSlider). rotated((float)(M_PI*2.0*sliderPosition), (float)rect.getCentreX(), (float)rect.getCentreY());
//    g.drawImageTransformed(speakerImage.rescaled(speakerSize, speakerSize),
//                           t,
//                           false);

    
    
    // rotating speaker image
//    sliderPosition = panner_el.getValue() / 360 + 0.75;
//    t = AffineTransform().translated(rect.getCentreX()- speakerSize/2 ,
//                                    rect.getY() - speakerSize - spaceBetweenSpeakerAndSlider). rotated((float)(M_PI*2.0*sliderPosition), (float)rect.getCentreX(), (float)rect.getCentreY());
//    g.drawImageTransformed(speakerImage.rescaled(speakerSize, speakerSize),
//                           t,
//                           false);
    
    // sofa metadata short info
    g.setColour(Colours::white);
    g.setFont(Font(11));
    g.drawFittedText(sofaMetadataID, 10, 130, 100, 100, Justification::topLeft, 3);
    g.drawFittedText(sofaMetadataValue, 110, 130, 300, 100, Justification::topLeft, 3);
    
}

void SofaPanAudioProcessorEditor::resized()
{
   
    rearrange();
}

// This timer periodically checks whether any of the filter's parameters have changed...
void SofaPanAudioProcessorEditor::timerCallback() {

    bypassButton.setToggleState((bool)getParameterValue("bypass"), NotificationType::dontSendNotification);
    testSwitchButton.setToggleState((bool)getParameterValue("test"), NotificationType::dontSendNotification);
    useGlobalSofaFileButton.setToggleState(processor.getUsingGlobalSofaFile(), NotificationType::dontSendNotification);

    bool distanceSimActive = (bool)getParameterValue("dist_sim");
    useDistanceSimulationButton.setToggleState(distanceSimActive, NotificationType::dontSendNotification);
    
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
            float* hrir = processor.getCurrentHRIR();
            fftwf_complex* hrtf = processor.getCurrentHRTF();
            int complexLength = processor.getComplexLength();
            int firLength = complexLength - 1;
            int sampleRate = processor.getSampleRate();
            plotHRIRView.drawHRIR(hrir, firLength, sampleRate);
            plotHRTFView.drawHRTF(hrtf, complexLength, sampleRate);
        }
        panner2D.setDistanceAndAngle(distanceValue, azimuthValue, elevationValue);
        panner2D_elev.setDistanceAndAngle(distanceValue, azimuthValue, elevationValue);


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
            panner_el.setRotaryParameters((270. + eleMin) * deg2rad , (270. + eleMax) * deg2rad, true);
            elevationRange_Note = "none";
            panner_el.setValue(0.0);
            panner_el.setEnabled(false);
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
    
    if (button == &bypassButton)
        setParameterValue("bypass", bypassButton.getToggleState());
    
    if (button == &testSwitchButton)
        setParameterValue("test", testSwitchButton.getToggleState());
    
    if(button == &showSOFAMetadataButton)
        metadataView.setVisible(true);
    
    if(button == &useGlobalSofaFileButton)
        processor.setUsingGlobalSofaFile(useGlobalSofaFileButton.getToggleState());
    
    if(button == &useDistanceSimulationButton){
        setParameterValue("dist_sim", useDistanceSimulationButton.getToggleState());
        processor.updateSofaMetadataFlag = true;
    }
    
    if(button == &useLayoutRoomsimButton){
        roomsimLayout = 1;
        rearrange();
        removeChildComponent(&plotHRIRView);
        removeChildComponent(&plotHRTFView);
        addAndMakeVisible(&panner2D);
        addAndMakeVisible(&panner2D_elev);
        panner_dist.setSliderStyle(Slider::LinearHorizontal);
        repaint();

    }
    if(button == & useLayoutSOFAPlayerButton){
        roomsimLayout = 0;
        rearrange();
        removeChildComponent(&panner2D);
        removeChildComponent(&panner2D_elev);
        addAndMakeVisible(&plotHRTFView);
        addAndMakeVisible(&plotHRIRView);
        panner_dist.setSliderStyle(Slider::LinearVertical);
        repaint();
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
        if(e.eventComponent->getComponentID() == "nfSimButton"){
            
            AttributedString text ("By activating this option, the effect of the source approaching the head will be simulated. \n The distance slider becomes active and can be adjusted from 1m to 20cm. \n If the loaded SOFA file has already multiple distance sets, those will be deactivated and the set with the distance nearest to 1m will be used for the simulation");
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

    
    loadSOFAButton.setBounds(-5., 10., 150., 30.);
    showSOFAMetadataButton.setBounds(-8, 230, 200, 30);
    showTooltipsButton.setBounds(100, getLocalBounds().getBottom() - 30, 100, 30);
    
    metadataView.setBounds(getLocalBounds().reduced(20));
    
    bypassButton.setBounds(10., 50., 150., 30.);
    //testSwitchButton.setBounds(10., 80., 150., 30.);
    useGlobalSofaFileButton.setBounds(135, 10., 80, 30);
    useDistanceSimulationButton.setBounds(10, showSOFAMetadataButton.getBottom() + 30, 100, 30);
    
    
    
    
    int panner_size;
    const int spacing = 30;

    const Rectangle<int> mainControlBox = Rectangle<int>(240, spacing, getWidth() - 260, getHeight() - spacing * 2);
    Rectangle<int> boxWithSpacing = mainControlBox.reduced(spacing, 10);

    Rectangle<int> azimuthLabel, elevationLabel, distanceLabel, topViewLabel, rearViewLabel, hrtfLabel, hrirLabel;
    
    if(roomsimLayout) {
        int panner2D_size = (boxWithSpacing.getWidth() - spacing) / 2.0;

        topViewLabel = boxWithSpacing.withSize(panner2D_size, 30);
        rearViewLabel = topViewLabel.translated(panner2D_size + spacing, 0);
        
        panner2D.setBounds(topViewLabel.getX(), topViewLabel.getBottom(), panner2D_size, panner2D_size);
        panner2D_elev.setBounds(rearViewLabel.getX(), rearViewLabel.getBottom(), panner2D_size, panner2D_size);
        
        panner_size = 80;
        panner_az.setBounds(panner2D.getX(), panner2D.getBottom() + spacing, panner_size, panner_size);
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
    useLayoutSOFAPlayerButton.setBounds(modeSelectionBox.withWidth(modeSelectionBoxWidth/2.0));
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
