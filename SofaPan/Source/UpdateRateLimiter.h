/*
  ==============================================================================

    UpdateRateLimiter.h
    Created: 17 Oct 2017 11:24:45am
    Author:  David Bau

  ==============================================================================
*/

#pragma once
class UpdateRateLimiter{
public:
    UpdateRateLimiter(){
        maxUpdateValue = 1.0;
        lastValue = 0.0;
    }
    UpdateRateLimiter(float _maxUpdateValue){
        maxUpdateValue = _maxUpdateValue;
        lastValue = 0.0;
    }
    
    ~UpdateRateLimiter(){}
    
    void setMaxUpdateValue(float value){
        maxUpdateValue = value;
    }
    
    float update(float newValue){
        float returnValue = 0;

        float valueDiff = newValue - lastValue;
        
        if( valueDiff > maxUpdateValue){
            returnValue = lastValue + maxUpdateValue;
        }else if(valueDiff < - maxUpdateValue){
            returnValue = lastValue - maxUpdateValue;
        }else{
            returnValue = newValue;
        }
        lastValue = returnValue;
        
        return returnValue;
        
    }
private:
    
    float maxUpdateValue;
    float lastValue;
};
