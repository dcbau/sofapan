/*
  ==============================================================================

    PlotHRTFComponent.h
    Created: 4 May 2017 10:24:17am
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "fftw3.h"
#include "sofaPanLookAndFeel.h"
//==============================================================================
/*
*/
class PlotHRTFComponent    : public Component, public Button::Listener
{
public:
    PlotHRTFComponent()
    {
        mag_l.resize(0);
        mag_r.resize(0);
        phase_l.resize(0);
        phase_r.resize(0);

        LookAndFeel::setDefaultLookAndFeel(&juceDefaultLookAndFeel);

        
        phaseViewButton.setButtonText("Phase");
        phaseViewButton.setClickingTogglesState(true);
        phaseViewButton.setRadioGroupId(1111);
        phaseViewButton.setColour (TextButton::buttonColourId, Colours::lightgrey);
        phaseViewButton.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
        phaseViewButton.setColour (TextButton::textColourOffId, Colours::black);
        phaseViewButton.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        phaseViewButton.setToggleState(false, dontSendNotification);
        phaseViewButton.setLookAndFeel(&juceDefaultLookAndFeel);
        phaseViewButton.addListener(this);
        addAndMakeVisible(phaseViewButton);
        
        magViewButton.setButtonText("Mag");
        magViewButton.setClickingTogglesState(true);
        magViewButton.setRadioGroupId(1111);
        magViewButton.setColour (TextButton::buttonColourId, Colours::lightgrey);
        magViewButton.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
        magViewButton.setColour (TextButton::textColourOffId, Colours::black);
        magViewButton.setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnLeft);
        magViewButton.setToggleState(true, dontSendNotification);
        magViewButton.setLookAndFeel(&juceDefaultLookAndFeel);
        magViewButton.addListener(this);
        addAndMakeVisible(magViewButton);
    }

    ~PlotHRTFComponent()
    {
    }

    void paint (Graphics& g) override
    {
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
        g.reduceClipRegion(plotBox.toNearestInt());
        
        if(repaintFlag){
            repaintFlag = false;
            if(mag_l.size() == 0 || mag_r.size()==0)
                return;
            spectrogram_l.clear();
            spectrogram_r.clear();
            float xPos, yPos;
            const float dBmax = 24.0;
            const float dBmin = -60.0;
            const float dBrange = dBmax - dBmin;
            const float frequencyStep = (float)sampleRate / (float)fftSize;

            
            //Left Channel
            spectrogram_l.startNewSubPath(plotBox.getX(), plotBox.getBottom());
            spectrogram_l.lineTo(plotBox.getX(), plotBox.getY() - plotBox.getHeight() * (Decibels::gainToDecibels(mag_l[0])-dBmax) / dBrange);
            //RightChannel
            spectrogram_r.startNewSubPath(plotBox.getX(), plotBox.getBottom());
            spectrogram_r.lineTo(plotBox.getX(), plotBox.getY() - plotBox.getHeight() * (Decibels::gainToDecibels(mag_r[0])-dBmax) / dBrange);
            
            float oldXPos = 0;
            for(int i = 1; i < mag_l.size(); i++){
                float freq = i * frequencyStep;
                if(freq > 20 && freq < 20000){
                    xPos = log10f(freq / 20.0) * (plotBox.getWidth()/3) + plotBox.getX();
                    if(xPos - oldXPos > 1.0){
                        if(togglePhaseView){
                            yPos = plotBox.getY() + plotBox.getHeight() * 0.5 * (1  -  phase_l[i]);
                            spectrogram_l.lineTo(xPos, yPos);
                            yPos = plotBox.getY() + plotBox.getHeight() * 0.5 * (1  -  phase_r[i]);
                            spectrogram_r.lineTo(xPos, yPos);
                        }else{
                            yPos = plotBox.getY() - plotBox.getHeight() * (Decibels::gainToDecibels(mag_l[i])-dBmax) / dBrange;
                            spectrogram_l.lineTo(xPos, yPos);
                            yPos = plotBox.getY() - plotBox.getHeight() * (Decibels::gainToDecibels(mag_r[i])-dBmax) / dBrange;
                            spectrogram_r.lineTo(xPos, yPos);
                        }
                        oldXPos = xPos;
                
                    }
                }
            }
    
            spectrogram_l.lineTo(plotBox.getRight(), yPos);
            spectrogram_l.lineTo(plotBox.getRight(), plotBox.getBottom());
            spectrogram_r.lineTo(plotBox.getRight(), yPos);
            spectrogram_r.lineTo(plotBox.getRight(), plotBox.getBottom());

        }

        g.setColour(Colour(0xffaaaa00));
        g.strokePath(spectrogram_l.createPathWithRoundedCorners(2.0), PathStrokeType(1.5));
        g.setColour(Colour(0xff00aaaa));
        g.strokePath(spectrogram_r.createPathWithRoundedCorners(2.0), PathStrokeType(1.5));

        
        
        
    }

    void resized() override
    {
        Rectangle<float> bounds = getLocalBounds().toFloat();
        plotBox = bounds.withTrimmedBottom(20).withTrimmedLeft(20).withTrimmedTop(5).withTrimmedRight(5);

        magViewButton.setBounds(plotBox.getX(), plotBox.getBottom()+2, 40, 16);
        phaseViewButton.setBounds(plotBox.getX()+40, plotBox.getBottom()+2, 40, 16);

        
        
        //Draw Background Image
        backgroundImage = Image(Image::RGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);
        Graphics g (backgroundImage);
        g.fillAll (Colours::white);   // clear the background
        g.setColour (Colours::grey);
        g.drawRect (bounds, 1);
        g.drawRect (plotBox, 1);
        g.setFont(Font(9.f));
        float xPos;
        Line<float> gridLine;
        float dashes[2] = {3.0, 3.0};
        //draw grid X
        float gridLinesX[8] = {50, 100, 200, 500, 1000, 2000, 5000, 10000};
        for(int i = 0; i < 8; i++){
            xPos = log10f(gridLinesX[i]/20.0) * (plotBox.getWidth()/3) + plotBox.getX();
            gridLine = Line<float>(xPos , plotBox.getBottom() , xPos , plotBox.getY());
            g.drawDashedLine(gridLine, dashes, 2);
            g.drawText(String(gridLinesX[i]) , xPos-40 , plotBox.getBottom() + 5 , 80 , 20 , juce::Justification::centredTop);
        }
        g.drawText("20", plotBox.getX()-40, plotBox.getBottom() + 5, 80, 20, juce::Justification::centredTop);
        g.drawText("20k", plotBox.getRight()-40, plotBox.getBottom() + 5, 80, 20, juce::Justification::centredTop);
        
        repaintFlag = true;

    }
    
    /** Size is the length of one TF. The complex vector hrtf should contain the left AND the right TF, making the vector of length size*2 */
    void drawHRTF(fftwf_complex* hrtf, int size, int _sampleRate){
        
        mag_l.resize(size);
        mag_r.resize(size);
        phase_l.resize(size);
        phase_r.resize(size);
    
        fftSize = (size - 1) * 2;
        float scale = 1.0;// 8.0 / fftSize;
        for(int i = 0; i< size; i++){
            mag_l[i] = scale * (sqrtf(hrtf[i][0] * hrtf[i][0] + hrtf[i][1] * hrtf[i][1]));
            mag_r[i] = scale * (sqrtf(hrtf[i+size][0] * hrtf[i+size][0] + hrtf[i+size][1] * hrtf[i+size][1]));
            phase_l[i] = atan2f(hrtf[i][1], hrtf[i][0]) / M_PI;
            phase_r[i] = atan2f(hrtf[i+size][1], hrtf[i+size][0]) / M_PI;
            
         }
        sampleRate = _sampleRate;
        repaintFlag = true;
    }

    void buttonClicked (Button* button) override{
        
        if(button == &magViewButton) togglePhaseView = false;
        if(button == &phaseViewButton) togglePhaseView = true;
        repaintFlag = true;
        repaint();
            
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlotHRTFComponent)
    
    std::vector<float> mag_l, mag_r;
    std::vector<float> phase_l, phase_r;
    int sampleRate;
    int fftSize;
    
    Path spectrogram_l, spectrogram_r;
    
    Rectangle<float> plotBox;
    Image backgroundImage;
    bool repaintFlag = false;
    
    TextButton phaseViewButton;
    TextButton magViewButton;
    bool togglePhaseView = false;
    LookAndFeel_V4 juceDefaultLookAndFeel;
    SofaPanLookAndFeel sofaPanLookAndFeel;
    
};
