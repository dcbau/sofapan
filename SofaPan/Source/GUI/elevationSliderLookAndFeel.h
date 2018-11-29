/*
  ==============================================================================

    elevationSliderLookAndFeel.h
    Created: 14 Dec 2016 3:25:21pm
    Author:  David Bau

  ==============================================================================
*/

#ifndef ELEVATIONSLIDERLOOKANDFEEL_H_INCLUDED
#define ELEVATIONSLIDERLOOKANDFEEL_H_INCLUDED


#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
class ElevationSliderLookAndFeel : public LookAndFeel_V3
{
public:
    //==============================================================================
    ElevationSliderLookAndFeel()
    {
        setColour (ResizableWindow::backgroundColourId, windowBackgroundColour);
        setColour (TextButton::buttonOnColourId, brightButtonColour);
        setColour (TextButton::buttonColourId, disabledButtonColour);
    }
    
    
    void drawRotarySlider  	(Graphics & g, int x, int y, int width, int height,
                             float sliderPosProportional, float rotaryStartAngle,
                             float rotaryEndAngle, Slider& slider) override
    {
        
        
        //assure that the knob is always the size of the smallest edge
        if(width > height){
            width = height;
        }
        
        Rectangle<int> r = Rectangle<int>(x, y, width, width);
        
        
        const int borderlineThickness = 4;
        const int innerRadius = r.getWidth() * 0.5 - 10;
        const int innerThickness = innerRadius / 2;
        const float thumbSize = r.getWidth() * 0.2;
        const float thumbThickness = thumbSize * 0.5;
        const float markerSize = thumbSize * 0.7;
        const float markerThickness = borderlineThickness * 0.5;

        //Outer Circle (size = full rectangle, non-adaptive thickness)
        g.setColour(sliderInactivePart);
        const int t_2 = borderlineThickness * 0.5;
        
        Path outerCircle;
        outerCircle.addCentredArc(r.getCentreX(), r.getCentreY(),
                                  r.getWidth()/2.0 - t_2, r.getWidth()/2.0 - t_2, 0,
                                  0, M_PI * 2.0,
                                  true);
        g.strokePath(outerCircle, PathStrokeType(borderlineThickness));
        
        
        
        
        
        //Inner Circle (size = always 10px smaller than outer circle)
        g.setColour(Colour(knobBackgroundColour));
        Path innerCircle;
        innerCircle.addCentredArc(r.getCentreX(), r.getCentreY(),
                                  innerRadius - innerThickness/2, innerRadius - innerThickness/2, 0,
                                  rotaryStartAngle - 0.01, rotaryEndAngle + 0.01,
                                  true);
        g.strokePath(innerCircle, PathStrokeType(innerThickness ));
        
//        Rectangle<int> innerCircle = r.withSizeKeepingCentre(innerRadius*2, innerRadius*2);
//        g.setColour(Colour(knobBackgroundColour));
//        g.fillEllipse(innerCircle.toFloat());
        
        
        float angle;
        
        angle = sliderPosProportional * (rotaryEndAngle-rotaryStartAngle) + rotaryStartAngle;

        //Creating the Thumb
        Rectangle<int> thumb;
        thumb = r.withSizeKeepingCentre(thumbThickness, thumbSize - borderlineThickness).translated(0, -(r.getWidth() - thumbSize)/2.0);
        
        g.setColour(sliderActivePart);
        
        //rotate to knob position (knobPos*range + startAngle)
        AffineTransform rotate;
        rotate = AffineTransform::rotation(angle, r.getCentreX(), r.getCentreY());
        g.addTransform(rotate);
        g.setColour(sliderActivePart);
        g.fillRoundedRectangle(thumb.toFloat(), thumbThickness * 0.25);
        
        //inverse rotation
        rotate = AffineTransform::rotation(-angle, r.getCentreX(), r.getCentreY());
        g.addTransform(rotate);
        
        
    }
    
    //==============================================================================
    Font getTextButtonFont (TextButton& button, int buttonHeight) override
    {
        return LookAndFeel_V3::getTextButtonFont (button, buttonHeight).withHeight (buttonFontSize);
    }
    
    Font getLabelFont (Label& label) override
    {
        return LookAndFeel_V3::getLabelFont (label).withHeight (labelFontSize);
    }
    
    //==============================================================================
    const int labelFontSize = 12;
    const int buttonFontSize = 15;
    
    //=============================================================================
    const int knobActiveRadius = 12;
    const int knobInActiveRadius = 8;
    const int haloRadius = 18;
    
    //==============================================================================
    const Colour windowBackgroundColour = Colour (0xff262328);
    const Colour backgroundColour = Colour (0xff4d4d4d);
    const Colour brightButtonColour = Colour (0xff80cbc4);
    const Colour disabledButtonColour = Colour (0xffe4e4e4);
    const Colour sliderInactivePart = Colour (0xff545d62); //benutzt für rotary->außenring und slider->inaktiverpart
    const Colour sliderActivePart = Colour (0xff80cbc4);
    const Colour knobBackgroundColour = Colour(0xffcccccc);
};






#endif  // ELEVATIONSLIDERLOOKANDFEEL_H_INCLUDED
