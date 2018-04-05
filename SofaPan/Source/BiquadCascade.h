/*
  ==============================================================================

    BiquadCascade.h
    Created: 12 Dec 2017 4:04:13pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "Biquad.h"
class BiquadCascade{
public:

    BiquadCascade(){
        filterCascade.resize(0);
    }
    ~BiquadCascade(){}

    void processBlockStereo(float* IOBuffer_L, float* IOBuffer_R, int numSamples){

        for(int i = 0; i < filterCascade.size(); i++){
            filterCascade[i].processBlockStereo(IOBuffer_L, IOBuffer_R, numSamples);
        }
    }
    
    void addFilter(int type, float cutoffFrequency, float Q, float gain){
        filterCascade.push_back(Biquad(type, cutoffFrequency, Q, gain));
    }
    
    void addFilterCopyFromIndex(int index){
        filterCascade.push_back(Biquad(filterCascade[index].getFilterSpecs()));
    }
    
    uint64 getNumFilters(){
        return filterCascade.size();
    }
    
    filterSpecs getSpecsOfFilterAtIndex(int index){
        return filterCascade[index].getFilterSpecs();
    }
    
    void removeFilter(int index){
        filterCascade.erase(filterCascade.begin()+index);
    }
    
    void prepareToPlay(int sampleRate){
        for(int i = 0; i < filterCascade.size(); i++){
            filterCascade[i].prepareToPlay(sampleRate);
        }
        
    }
    
    
    
private:
    std::vector<Biquad> filterCascade;
    
    

    
};
