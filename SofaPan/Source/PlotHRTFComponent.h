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
#include "GUI/sofaPanLookAndFeel.h"
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
        
        unwrapPhaseButton.setButtonText("Unwrap Phase");
        unwrapPhaseButton.setClickingTogglesState(true);
        unwrapPhaseButton.setColour (TextButton::buttonColourId, Colours::lightgrey);
        unwrapPhaseButton.setColour (TextButton::buttonOnColourId, sofaPanLookAndFeel.mainCyan);
        unwrapPhaseButton.setColour (TextButton::textColourOffId, Colours::black);
        unwrapPhaseButton.setToggleState(false, dontSendNotification);
        unwrapPhaseButton.setLookAndFeel(&juceDefaultLookAndFeel);
        unwrapPhaseButton.addListener(this);
        addAndMakeVisible(unwrapPhaseButton);
        unwrapPhaseButton.setVisible(false);
    }

    ~PlotHRTFComponent()
    {
    }

    void paint (Graphics& g) override
    {

        
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
        g.reduceClipRegion(plotBox.toNearestInt().withTrimmedTop(6).withTrimmedLeft(6).withTrimmedRight(3));
        
        if(repaintFlag){

            repaintFlag = false;
            if(mag_l.size() == 0 || mag_r.size()==0)
                return;
            
            
            spectrogram_l.clear();
            spectrogram_r.clear();
            float xPos, yPos;
            const float dBmax = 6.0;//24.0;
            const float dBmin = -40.0; //-60.0;
            const float dBrange = dBmax - dBmin;
            const float frequencyStep = (float)sampleRate / (float)fftSize;

            
            spectrogram_l.startNewSubPath(plotBox.getX(), plotBox.getBottom());
            spectrogram_r.startNewSubPath(plotBox.getX(), plotBox.getBottom());

            if(togglePhaseView){
                spectrogram_l.lineTo(plotBox.getX(), plotBox.getY() + plotBox.getHeight() * 0.5 * (1  -  phase_l[0]));
                spectrogram_r.lineTo(plotBox.getX(), plotBox.getY() + plotBox.getHeight() * 0.5 * (1  -  phase_r[0]));
            }else{
                spectrogram_l.lineTo(plotBox.getX(), plotBox.getY() - plotBox.getHeight() * (Decibels::gainToDecibels(mag_l[0])-dBmax) / dBrange);
                spectrogram_r.lineTo(plotBox.getX(), plotBox.getY() - plotBox.getHeight() * (Decibels::gainToDecibels(mag_r[0])-dBmax) / dBrange);
            }

            
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

        if(togglePhaseView){
            g.setFont(10.f);
            g.setColour(Colours::black);
            String text1 = String("-180deg");
            String text2 = String("+180deg");
            if(toggleUnwrapPhase){
                int circles = maxUnwrappedPhase / 360;
                text2 = String(String(circles) +" * 360 deg");
            }
            g.drawText(text1, plotBox.getX() + 5, plotBox.getY() + 8, 70, 10, juce::Justification::left, true);
            g.drawText(text2, plotBox.getX() + 5, plotBox.getBottom()-10, 70, 10, juce::Justification::left, true);
        }
        
        
        
    }

    void resized() override
    {
        const float heightLowerPanel = 20;
        Rectangle<float> bounds = getLocalBounds().toFloat();
        plotBox = bounds.withTrimmedBottom(heightLowerPanel);//.withTrimmedLeft(20).withTrimmedTop(5).withTrimmedRight(5);

        magViewButton.setBounds(plotBox.getX()+10 , plotBox.getBottom()+2, 40., heightLowerPanel - 7.);
        phaseViewButton.setBounds(plotBox.getX()+50, plotBox.getBottom()+2, 40, heightLowerPanel - 7.);
        unwrapPhaseButton.setBounds(plotBox.getX()+100 , plotBox.getBottom()+2, 40., heightLowerPanel - 7.);
        
        
        //Draw Background Image
        backgroundImage = Image(Image::RGB, getLocalBounds().getWidth(), getLocalBounds().getHeight(), true);
        Graphics g (backgroundImage);
        g.fillAll (Colours::white);   // clear the background

        g.setFont(Font(9.f));
        float xPos;
        Line<float> gridLine;
        float dashes[2] = {3.0, 3.0};
        //draw grid X
        float gridLinesX[8] = {50, 100, 200, 500, 1000, 2000, 5000, 10000};
        for(int i = 0; i < 8; i++){
            xPos = log10f(gridLinesX[i]/20.0) * (plotBox.getWidth()/3) + plotBox.getX();
            gridLine = Line<float>(xPos , plotBox.getBottom() - 1 , xPos , plotBox.getY() + 1);
            g.drawDashedLine(gridLine, dashes, 2);
            g.drawText(String(gridLinesX[i]) , xPos-40 , plotBox.getBottom() + 5 , 80 , 20 , juce::Justification::centredTop);
        }
        //g.drawText("20", plotBox.getX()-40, plotBox.getBottom() + 5, 80, 20, juce::Justification::centredTop);
        //g.drawText("20k", plotBox.getRight()-40, plotBox.getBottom() + 5, 80, 20, juce::Justification::centredTop);
        
        g.setColour (Colours::grey);
        g.drawRect (plotBox, 1);
        //g.setColour (Colours::lightgrey);

        g.drawRect (bounds.reduced(2,2).translated(1, 1), 3);
        g.setColour (sofaPanLookAndFeel.mainCyan);
        g.drawRect (bounds, 3);
        g.setColour (Colours::grey);
        
        
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
        
        if(toggleUnwrapPhase)
            unwrapPhase();


        /* Tryout für Wandabsorptions TF */
//        float freqStep = (float)sampleRate / (float)size;
//        
//        float wood[7] = {0.18, 0.12, 0.1, 0.09, 0.08, 0.07, 0.07};
//        float roughConcrete[7] = {0.02, 0.03, 0.03, 0.03, 0.04, 0.07, 0.07};
//        float cottonCurtains[7] = {0.3, 0.45, 0.65, 0.56, 0.59, 0.71, 0.71};
//        
//        float* absorbtionCoeffs;
//        String material = String("wood");
//        if(material == "wood")
//            absorbtionCoeffs = cottonCurtains;
//            
//        
//        for(int i = 0; i< size; i++){
//            float currentFrequency = i * freqStep;
//            if(currentFrequency < 250)
//                mag_l[i] = 1-absorbtionCoeffs[0];
//            else if(currentFrequency < 500)
//                mag_l[i] =  1-absorbtionCoeffs[1];
//            else if(currentFrequency < 1000)
//                mag_l[i] = 1-absorbtionCoeffs[2];
//            else if(currentFrequency < 2000)
//                mag_l[i] = 1-absorbtionCoeffs[3];
//            else if(currentFrequency < 4000)
//                mag_l[i] = 1-absorbtionCoeffs[4];
//            else if(currentFrequency < 8000)
//                mag_l[i] = 1-absorbtionCoeffs[5];
//            else
//                mag_l[i] = 1-absorbtionCoeffs[6];
//            
//            phase_l[i] = 1.0;
//            
//        }
//        
        
        sampleRate = _sampleRate;
        repaintFlag = true;
        repaint();
    }

    void buttonClicked (Button* button) override{
        
        if(button == &magViewButton) togglePhaseView = false;
        if(button == &phaseViewButton) togglePhaseView = true;
        
        unwrapPhaseButton.setVisible(togglePhaseView);
        
        if(button == &unwrapPhaseButton) {
            toggleUnwrapPhase = unwrapPhaseButton.getToggleState();
            if(toggleUnwrapPhase)
                unwrapPhase();
            
        }

        repaintFlag = true;
        repaint();
            
    }
    
    void unwrapPhase(){
        
        __SIZE_TYPE__ size = phase_l.size();

        float phaseUnwrapped_l[size];
        float phaseUnwrapped_r[size];
        
        for(int i = 0; i< size; i++){
            phase_l[i] /= M_PI;
            phase_r[i] /= M_PI;
        }
        
        phaseUnwrapped_l[0] = phase_l[0];
        float lastPhaseValue = phaseUnwrapped_l[0];
        int multiply2Pi = 0;
        float maxPhaseValue = 0.0;
        
        for(int i = 1; i < size; i++){
            if(fabs(phase_l[i] + lastPhaseValue) < fabs(lastPhaseValue))
                multiply2Pi++;
            phaseUnwrapped_l[i] = phase_l[i] - 2.0 * M_PI * (float)multiply2Pi;
            if(fabs(phaseUnwrapped_l[i]) > maxPhaseValue)
                maxPhaseValue = fabs(phaseUnwrapped_l[i]);
            
            lastPhaseValue = phase_l[i];
        }
        
        phaseUnwrapped_r[0] = phase_r[0];
        lastPhaseValue = phase_r[0];
        multiply2Pi = 0;
        for(int i = 1; i < size; i++){
            if(fabs(phase_r[i] + lastPhaseValue) < fabs(lastPhaseValue))
                multiply2Pi++;
            phaseUnwrapped_r[i] = phase_r[i] - 2.0 * M_PI * (float)multiply2Pi;
            if(fabs(phaseUnwrapped_r[i]) > maxPhaseValue)
                maxPhaseValue = fabs(phaseUnwrapped_r[i]);
            
            lastPhaseValue = phase_r[i];
        }
        
        for(int i = 0; i< size; i++){
            phase_l[i] = (phaseUnwrapped_l[i] / maxPhaseValue);
            phase_r[i] = (phaseUnwrapped_r[i] / maxPhaseValue);
        }
        
        maxUnwrappedPhase = trunc(maxPhaseValue * 360.0);
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
    TextButton unwrapPhaseButton;
    bool togglePhaseView = false;
    bool toggleUnwrapPhase = false;
    int maxUnwrappedPhase;
    LookAndFeel_V4 juceDefaultLookAndFeel;
    SofaPanLookAndFeel sofaPanLookAndFeel;
    
    
};
