/*
  ==============================================================================

    Delayline.h
    Created: 4 Aug 2017 3:27:59pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#define maxDelayLength 5000

class Delayline{
    
public:
    Delayline(float _maxLengthMs, float _sampleRate)
    : sampleRate(_sampleRate), maxLengthMs(_maxLengthMs){
        if(maxLengthMs > maxDelayLength){
            printf("Delay Length too long, using %d instead", maxDelayLength);
        }
        lengthInSamples = maxLengthMs * sampleRate * 0.001;
        
        delayLine = (float*)malloc(lengthInSamples * sizeof(float));
    }
    
    ~Delayline(){}
    
    void processBlock(const float* inBuffer, float* outBuffer, int numSamples, float delayInMs){
        
        float offset = delayInMs * sampleRate * 0.001;
        int offset_int = truncf(offset);
        float offset_frac = offset - (float)offset_int;
        
        int readIndex1, readIndex2;
        
        for(int sample = 0; sample < numSamples; sample++){
            
            readIndex1 = writeIndex - offset;
            readIndex2 = readIndex1 - 1;
            while(readIndex1 < 0)
                readIndex1 += lengthInSamples;
            while(readIndex2 < 0)
                readIndex2 += lengthInSamples;
            
            float delayLineOutput = delayLine[readIndex1] * offset_frac + delayLine[readIndex2]* (1.0 - offset_frac);
            
            delayLine[writeIndex++] = inBuffer[sample];
            if (writeIndex >= lengthInSamples)
                writeIndex -= lengthInSamples;
            

            outBuffer[sample] = delayLineOutput;

            
        }
    }

    
    void prepareToPlay(){
        for(int i = 0; i < lengthInSamples; i++)
            delayLine[i] = 0.0;
        writeIndex = 0;
    }
    
private:
    
    int sampleRate;
    float maxLengthMs;
    int lengthInSamples;
    
    
    
    //delayline stuff
    float* delayLine;
    int writeIndex;
    
};

