/*
  ==============================================================================

    UnwrapPhase.h
    Created: 17 Dec 2019 12:54:01pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once
#include <vector>
#include "math.h"

using namespace std;


class PhaseWrapping{
public:
    static vector<float> unwrap(const vector<float> input, bool normalize){
        
        vector<float> phaseUnwrapped(input);
        
        /* Procedure from "Gdeisat & Lilley - One-Dimensional Phase Unwrapping Problem", 2011
         https://www.researchgate.net/publication/265151826_One-Dimensional_Phase_Unwrapping_Problem
         */
        
        // walk through all phase values from left to right
        for(auto iUnwrapped = phaseUnwrapped.begin() + 1;
            iUnwrapped != phaseUnwrapped.end();
            ++iUnwrapped)
        {
            //check for phase jump
            float delta = *iUnwrapped - *(iUnwrapped-1);
            
            if(delta > M_PI)
            {
                //if phase jump from high to low, decrease every righthand sample by 2pi
                std::for_each(iUnwrapped, phaseUnwrapped.end(), [](float &n){n -= 2 * M_PI;});
            }
            if(delta < - M_PI)
            {
                //if phase jump from low to high, increase every righthand sample by 2pi
                std::for_each(iUnwrapped, phaseUnwrapped.end(), [](float &n){n += 2 * M_PI;});
            }
                              
        }
        
        if(normalize)
        {
            float max = 0.f;
            for(float i : phaseUnwrapped){
                if(fabsf(i) > max)
                    max = fabsf(i);
            }
            std::for_each(phaseUnwrapped.begin(), phaseUnwrapped.end(), [=](float &n){n /= max;});
        }
        
        return phaseUnwrapped;
    }
    
    
    
    static void unwrap(float* input, float* output, int numSamples, bool normalize){
        
        vector<float> inputVec(input, input + numSamples);
        vector<float> outputVec(unwrap(inputVec, normalize));
        
        for(int i = 0; i < numSamples; i++)
            output[i] = outputVec[i];
        
        return;
    }
    
    static vector<float> wrap(const vector<float> input){
        
        vector<float> phaseWrapped(input);
        
        // walk through all phase values from left to right
        for(auto it = phaseWrapped.begin();
            it != phaseWrapped.end();
            ++it)
        {
            if(*it > M_PI){
                //if phase jump from high to low, decrease every righthand sample by 2pi
                std::for_each(it, phaseWrapped.end(), [](float &n){n -= 2 * M_PI;});
            }
            if(*it < - M_PI){
                //if phase jump from low to high, increase every righthand sample by 2pi
                std::for_each(it, phaseWrapped.end(), [](float &n){n += 2 * M_PI;});
            }
                            
        }
        
        
        return phaseWrapped;
    }
    
    static void wrap(float* input, float* output, int numSamples){
        
        vector<float> inputVec(input, input + numSamples);
        vector<float> outputVec(wrap(inputVec));
        
        for(int i = 0; i < numSamples; i++)
            output[i] = outputVec[i];
        
        return;
    }
    
private:
    PhaseWrapping(){};
};
