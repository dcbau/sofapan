/*
  ==============================================================================

    PlotHRIRComponent.h
    Created: 4 May 2017 11:19:35pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "GUI/sofaPanLookAndFeel.h"


//==============================================================================
/*
*/
class PlotHRIRComponent    : public Component, public Button::Listener
{
public:
    PlotHRIRComponent()
    {
        LookAndFeel::setDefaultLookAndFeel(&defaultLookAndFeel);
        IR_Left.resize(0);
        IR_Right.resize(0);
        IR_Left_Zoom.resize(0);
        IR_Right_Zoom.resize(0);
        zoomXFactor = 1.0;
        zoomYFactor = 1.0;
        
        
        zoomXButton1.setButtonText("1x");
        zoomXButton1.setClickingTogglesState(true);
        zoomXButton1.setRadioGroupId(1111);
        zoomXButton1.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomXButton1.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
        zoomXButton1.setColour (TextButton::textColourOffId, Colours::black);
        zoomXButton1.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomXButton1.setToggleState(true, dontSendNotification);
        zoomXButton1.addListener(this);
        addAndMakeVisible(zoomXButton1);
        
        zoomXButton2.setButtonText("2x");
        zoomXButton2.setClickingTogglesState(true);
        zoomXButton2.setRadioGroupId(1111);
        zoomXButton2.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomXButton2.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
        zoomXButton2.setColour (TextButton::textColourOffId, Colours::black);
        zoomXButton2.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomXButton2.setToggleState(false, dontSendNotification);
        zoomXButton2.addListener(this);
        addAndMakeVisible(zoomXButton2);
        
        zoomXButton3.setButtonText("4x");
        zoomXButton3.setClickingTogglesState(true);
        zoomXButton3.setRadioGroupId(1111);
        zoomXButton3.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomXButton3.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
        zoomXButton3.setColour (TextButton::textColourOffId, Colours::black);
        zoomXButton3.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomXButton3.setToggleState(false, dontSendNotification);
        zoomXButton3.addListener(this);
        addAndMakeVisible(zoomXButton3);
        
        zoomYButton1.setButtonText("1x");
        zoomYButton1.setClickingTogglesState(true);
        zoomYButton1.setRadioGroupId(2222);
        zoomYButton1.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomYButton1.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
        zoomYButton1.setColour (TextButton::textColourOffId, Colours::black);
        zoomYButton1.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomYButton1.setToggleState(true, dontSendNotification);
        zoomYButton1.addListener(this);
        addAndMakeVisible(zoomYButton1);
        
        zoomYButton2.setButtonText("1.3x");
        zoomYButton2.setClickingTogglesState(true);
        zoomYButton2.setRadioGroupId(2222);
        zoomYButton2.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomYButton2.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
        zoomYButton2.setColour (TextButton::textColourOffId, Colours::black);
        zoomYButton2.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomYButton2.setToggleState(false, dontSendNotification);
        zoomYButton2.addListener(this);
        addAndMakeVisible(zoomYButton2);
        
        zoomYButton3.setButtonText("1.8x");
        zoomYButton3.setClickingTogglesState(true);
        zoomYButton3.setRadioGroupId(2222);
        zoomYButton3.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomYButton3.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
        zoomYButton3.setColour (TextButton::textColourOffId, Colours::black);
        zoomYButton3.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomYButton3.setToggleState(false, dontSendNotification);
        zoomYButton3.addListener(this);
        addAndMakeVisible(zoomYButton3);
        
        zoomYButton4.setButtonText("2.5x");
        zoomYButton4.setClickingTogglesState(true);
        zoomYButton4.setRadioGroupId(2222);
        zoomYButton4.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomYButton4.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
        zoomYButton4.setColour (TextButton::textColourOffId, Colours::black);
        zoomYButton4.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomYButton4.setToggleState(false, dontSendNotification);
        zoomYButton4.addListener(this);
        addAndMakeVisible(zoomYButton4);
        
        zoomYButton5.setButtonText("3.5x");
        zoomYButton5.setClickingTogglesState(true);
        zoomYButton5.setRadioGroupId(2222);
        zoomYButton5.setColour (TextButton::buttonColourId, Colours::lightgrey);
        zoomYButton5.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
        zoomYButton5.setColour (TextButton::textColourOffId, Colours::black);
        zoomYButton5.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        zoomYButton5.setToggleState(false, dontSendNotification);
        zoomYButton5.addListener(this);
        addAndMakeVisible(zoomYButton5);
    
        
    }

    ~PlotHRIRComponent()
    {
    }

    void paint (Graphics& g) override
    {
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
        g.reduceClipRegion(plotBox.toNearestInt().withTrimmedTop(6).withTrimmedLeft(6).withTrimmedRight(3));
        const float oversampling = 1.5;

        if(repaintFlag){
            repaintFlag = false;
            if(IR_Left.size() == 0 || IR_Right.size()==0){
                return;
            }
            waveform_l.clear();
            waveform_r.clear();
            
            const int boxWidth = plotBox.getWidth();
            const int numSamples = IR_Left_Zoom.size();
            const float sampleStep = (float)numSamples / (float)boxWidth;
            float xPos, yPos;
            xPosOnsetR = xPosOnsetL = 0.0;
            waveform_l.startNewSubPath(plotBox.getX(), plotBox.getY() + plotBox.getHeight()*0.5*(zoomYFactor * IR_Left_Zoom[0] + 1));
            waveform_r.startNewSubPath(plotBox.getX(), plotBox.getY() + plotBox.getHeight()*0.5*(zoomYFactor * IR_Right_Zoom[0] + 1));
            for(int i = 1; i < boxWidth * oversampling; i++){
                int sample = (int)truncf(float(i)/oversampling * sampleStep);
                if(sample >= numSamples)  sample = numSamples-1;
                xPos = float(i)/oversampling + plotBox.getX();
                if(sample >= onsetIndex_L && xPosOnsetL == 0.0)
                    xPosOnsetL = xPos;
                if(sample >= onsetIndex_R && xPosOnsetR == 0.0)
                    xPosOnsetR = xPos;
                
                yPos = plotBox.getY() + plotBox.getHeight() * 0.5 * (1 - zoomYFactor * IR_Left_Zoom[sample]);
                waveform_l.lineTo(xPos, yPos);
                yPos = plotBox.getY() + plotBox.getHeight() * 0.5 * (1 - zoomYFactor * IR_Right_Zoom[sample]);
                waveform_r.lineTo(xPos, yPos);

            }
            
        }
        //Draw Onset Markers
        g.setColour(Colour(0x88aaaa00));
        g.drawVerticalLine(xPosOnsetL, plotBox.getY(), plotBox.getBottom());
        g.setColour(Colour(0x8800aaaa));
        g.drawVerticalLine(xPosOnsetR, plotBox.getY(), plotBox.getBottom());
        
        g.setColour(Colours::black);
        g.drawText(ITDdisplayText, plotBox.getRight() - 70, plotBox.getBottom() - 12, 70, 10, juce::Justification::left, true);
        
        
        //Left Channel
        g.setColour(Colour(0xffaaaa00));
        g.strokePath(waveform_l.createPathWithRoundedCorners(1.0), PathStrokeType(1.5));
        //Right Channel
        g.setColour(Colour(0xff00aaaa));
        g.strokePath(waveform_r.createPathWithRoundedCorners(1.0), PathStrokeType(1.5));
        
        
    }
    
    void resized() override
    {
        //printf("HRIR Resized");
        Rectangle<float> bounds = getLocalBounds().toFloat();
        plotBox = bounds.withTrimmedBottom(20);//.withTrimmedLeft(20).withTrimmedTop(5).withTrimmedRight(5);
        
        Rectangle<float> buttonXGroupArea = Rectangle<float>(plotBox.getX() + 10,
                                                        plotBox.getBottom()+2,
                                                        plotBox.getWidth() / 3,
                                                        bounds.getBottom() - plotBox.getBottom() - 7);
        Rectangle<float> buttonYGroupArea = buttonXGroupArea.withX(plotBox.getCentreX()).withWidth(plotBox.getWidth()/3);
        
        zoomXLabelBox = buttonXGroupArea.removeFromLeft(buttonXGroupArea.getWidth()/3.5);
        float buttonWidth = buttonXGroupArea.getWidth() / 3.0;
        Rectangle<int> button = buttonXGroupArea.withWidth(buttonWidth).toNearestInt();
        zoomXButton1.setBounds(button);
        button.translate(buttonWidth, 0);
        zoomXButton2.setBounds(button);
        button.translate(buttonWidth, 0);
        zoomXButton3.setBounds(button);
        
        
        zoomYLabelBox = buttonYGroupArea.removeFromLeft(buttonXGroupArea.getWidth()/3.5);
        buttonWidth = buttonYGroupArea.getWidth() / 5.0;
        button = buttonYGroupArea.withWidth(buttonWidth).toNearestInt();
        zoomYButton1.setBounds(button);
        button.translate(buttonWidth, 0);
        zoomYButton2.setBounds(button);
        button.translate(buttonWidth, 0);
        zoomYButton3.setBounds(button);
        button.translate(buttonWidth, 0);
        zoomYButton4.setBounds(button);
        button.translate(buttonWidth, 0);
        zoomYButton5.setBounds(button);
        
        
        //Create Background Image
        backgroundImage = Image (Image::RGB, bounds.getWidth(), bounds.getHeight(), true);
        Graphics g (backgroundImage);
        g.fillAll (Colours::white);   // clear the background
//        g.setColour (Colours::grey);
//        g.drawRect (bounds, 1);
//        g.setColour (Colours::black);
//        g.drawRect (plotBox, 1);
        g.setFont(Font(9.f));
        Line<float> gridLine;
        float dashes[2] = {3.0, 3.0};
        gridLine = Line<float>(plotBox.getX(), plotBox.getCentreY(), plotBox.getRight(), plotBox.getCentreY());
        g.drawDashedLine(gridLine, dashes, 2);
        g.drawFittedText("Zoom X:", zoomXLabelBox.toNearestInt(), Justification::centred, 1);
        g.drawFittedText("Zoom Y:", zoomYLabelBox.toNearestInt(), Justification::centred, 1);
        
        g.setColour (Colours::grey);
        g.drawRect (plotBox, 1);
        g.drawRect (bounds.reduced(2,2).translated(1, 1), 3);
        g.setColour (sofaPanLookAndFeel.mainCyan);
        g.drawRect (bounds, 3);
        g.setColour (Colours::grey);
        
        repaintFlag = true;
        repaint();

        
        

    }
    
    /** Size is the length of one IR. The float vector hrir should contain the left AND the right IR, making the vector of length size*2 */
    void drawHRIR(float* _HRIR, int size, int _sampleRate, ITDStruct ITD){
        
		std::vector<float> leHRIR;
		for (int i = 0; i< size*2; i++) {
			leHRIR.push_back(_HRIR[i]);
		}

        
        sampleRate = _sampleRate;
        IR_Left.clear();
        IR_Right.clear();
        IR_Left.resize(size, 0);
        IR_Right.resize(size, 0);

        for(int i = 0; i< size; i++){
            IR_Left[i] = 0.0;
            IR_Right[i] = 0.0;
        }

        for(int i = 0; i< size; i++){
            IR_Left[i] = _HRIR[i];
            IR_Right[i] = _HRIR[i + size];
        }
        
        

        
        
        //Remove zero-tail of long responses (minimum 128 samples remaining)
        if(size > 128){
            for(int i = 1; i < (size - 128); i++){
                if(IR_Left[size - i] == 0.0 && IR_Right[size - i] == 0.0){
                    IR_Left.pop_back();
                    IR_Right.pop_back();
                }else{
                    break;
                }
            }
        }
        
        //Insert ITD
        const int preSpace = 5;
        std::vector<float>::iterator it;
//        if(ITDsamples > 0){
//            onsetIndex_L = ITDsamples + preSpace;
//            onsetIndex_R = preSpace;
//        }else{
//            onsetIndex_L = preSpace;
//            onsetIndex_R = preSpace - ITDsamples;
//        }
      /*
		onsetIndex_L = ITD.onsetL_samples;
        onsetIndex_R = ITD.onsetR_samples;
        
        it = IR_Left.begin();
        IR_Left.insert ( it , onsetIndex_L, 0.0f);
        IR_Left.resize(size, 0);
        
        it = IR_Right.begin();
        IR_Right.insert(it, onsetIndex_R, 0.0f);
        IR_Right.resize(size, 0);
        
        ITDdisplayText = String(ITD.ITD_ms);
        ITDdisplayText = shortenFloatString(ITDdisplayText, 1);
        ITDdisplayText.append("ms ITD", 6);
       */

//        //RESIZE BOTH VECTORS
//        
//        //Detect Onset Threshold
//        float maxValue_L = 0.0;
//        float maxValue_R = 0.0;
//        
//        for(int i = 0; i< size; i++){
//            if (fabs(IR_Left[i]) > maxValue_L)
//                maxValue_L = fabs(IR_Left[i]);
//            if (fabs(IR_Right[i]) > maxValue_R)
//                maxValue_R = fabs(IR_Right[i]);
//        }
//        
//        printf("\nMaxValue_L: %f", maxValue_L);
//        maxValue_L *= 0.9;
//        maxValue_R *= 0.9;
//        onsetIndex_L = onsetIndex_R = 0;
//        for(int i = 0; i< size; i++){
//            if (fabs(IR_Left[i]) > maxValue_L && onsetIndex_L == 0)
//                onsetIndex_L = i;
//            if (fabs(IR_Right[i]) > maxValue_R && onsetIndex_R == 0)
//                onsetIndex_R = i;
//        }
        
//        printf("\nOnsetIndexL: %d \nOnsetIndexR: %d", onsetIndex_L, onsetIndex_R);

//        //Map values (apprx logarithmic via square root) for better display
//        float sign;
//        for(int i = 0; i< size; i++){
//            sign = IR_Left[i] < 0.0 ? -1.0 : 1.0;
//            IR_Left[i] = sqrtf(fabsf(IR_Left[i])) * sign;
//            sign = IR_Right[i] < 0.0 ? -1.0 : 1.0;
//            IR_Right[i] = sqrtf(fabsf(IR_Right[i])) * sign;
//        }
        
        
        IR_Left_Zoom = IR_Left;
        IR_Right_Zoom = IR_Right;
        
        if(zoomXFactor != 1.0){
            int newSize = IR_Left.size()/zoomXFactor;
            IR_Left_Zoom.resize(newSize);
            IR_Right_Zoom.resize(newSize);
            
        }
        
        repaintFlag = true;
        repaint();
        
        
        
    }
    
    void setZoomX(float _zoomFactor){
        
        //printf("\n setZoomX\n");

        
        if(_zoomFactor != zoomXFactor){
            int newSize = IR_Left.size()/_zoomFactor;
            
            if(_zoomFactor < zoomXFactor){
                IR_Left_Zoom = IR_Left;
                IR_Right_Zoom = IR_Right;
            }
            
            IR_Left_Zoom.resize(newSize);
            IR_Right_Zoom.resize(newSize);
            
            zoomXFactor = _zoomFactor;
            
            repaintFlag = true;
            repaint();
        }
    }
    
    void setZoomY(float _zoomFactor){
        if(_zoomFactor != zoomYFactor){
            zoomYFactor = _zoomFactor;
            repaintFlag = true;
            repaint();
        }
    }
    
    void buttonClicked (Button* button) override{
        
        if(button == &zoomXButton1) setZoomX(1.0);
        if(button == &zoomXButton2) setZoomX(2.0);
        if(button == &zoomXButton3) setZoomX(4.0);
        if(button == &zoomYButton1) setZoomY(1.0);
        if(button == &zoomYButton2) setZoomY(1.3);
        if(button == &zoomYButton3) setZoomY(1.8);
        if(button == &zoomYButton4) setZoomY(2.5);
        if(button == &zoomYButton5) setZoomY(3.5);
    }
    
    String shortenFloatString(String _string, int decimalValues){
        if(_string.containsChar('.'))
            return _string.substring(0, _string.indexOfChar('.') + decimalValues + 1);
        else{
            _string.append(".00000000000", decimalValues + 1);
            return _string;
        }
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlotHRIRComponent)

    std::vector<float> IR_Left;
    std::vector<float> IR_Right;
    
    std::vector<float> IR_Left_Zoom;
    std::vector<float> IR_Right_Zoom;
    
    float zoomXFactor;
    float zoomYFactor;
    int sampleRate;
    
    TextButton zoomXButton1;
    TextButton zoomXButton2;
    TextButton zoomXButton3;
    
    TextButton zoomYButton1;
    TextButton zoomYButton2;
    TextButton zoomYButton3;
    TextButton zoomYButton4;
    TextButton zoomYButton5;
    
    Rectangle<float> plotBox;
    Rectangle<float> zoomXLabelBox;
    Rectangle<float> zoomYLabelBox;
    
    Image backgroundImage;
    Path waveform_l, waveform_r;
    float xPosOnsetR, xPosOnsetL;

    
    SofaPanLookAndFeel sofaPanLookAndFeel;
    LookAndFeel_V4 defaultLookAndFeel;
    
    bool repaintFlag = false;
    
    int onsetIndex_L, onsetIndex_R;
    String ITDdisplayText;
};
