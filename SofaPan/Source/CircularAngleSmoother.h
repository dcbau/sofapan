/*
  ==============================================================================

    CircularAngleSmoother.h
    Created: 30 Jan 2018 11:38:55am
    Author:  David Bau

  ==============================================================================
*/

/*
 This is a extension to the juce::LinearSmoothedValue object. It extends it with the capability to smooth between azimuth degree values in the transition area 359° to 0°.
 */
#pragma once

class CircularAngleSmoother{
public:
    CircularAngleSmoother(){}
    ~CircularAngleSmoother(){}

    void reset(double sampleRate, double rampLengthInSeconds){
        azimuthSmoothed.reset( sampleRate , rampLengthInSeconds);
        k = 0;
    }
    
    float getNextValue(){
        float value = fmod(azimuthSmoothed.getTargetValue(), 360.0);
        if(value < 0.0) value += 360.0;
        
        return value;
    }
    
    void setValue(float value){
        
        float target = fmod(azimuthSmoothed.getTargetValue(), 360.0);
        if(target < 0.0) target += 360.0;
        
        float valueDiff = value - target;
        
        if(valueDiff < - 180.0)     k++;
        if(valueDiff >   180.0)     k--;

        
        azimuthSmoothed.setValue(value + k * 360.0);
        
    }
    
    
private:
    
    LinearSmoothedValue<float> azimuthSmoothed;
    int k;
};
