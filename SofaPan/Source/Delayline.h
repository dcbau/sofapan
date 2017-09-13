/*
  ==============================================================================

    Delayline.h
    Created: 4 Aug 2017 3:27:59pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once
#include  "ErrorHandling.h"
#define MAXDELAYLENGTH 500

class Delayline{
    
public:
    Delayline()
    {
        sampleRate = 0;
        delayLine = NULL;
        maxLengthMs = MAXDELAYLENGTH;
    }
    
    
    ~Delayline()
    {
        if(delayLine != NULL) free(delayLine);
    }
    
    
    void processBlock(const float* inBuffer, float* outBuffer, int numSamples, float delayInMs)
    {
        float offset = delayInMs * (float)sampleRate * 0.001;
        offset_int = truncf(offset);
        offset_frac = offset - (float)offset_int;
        
        for(int sample = 0; sample < numSamples; sample++){
            
            readIndex1 = writeIndex - offset_int;
            readIndex2 = readIndex1 - 1;
            while(readIndex1 < 0)   readIndex1 += lengthInSamples;
            while(readIndex2 < 0)   readIndex2 += lengthInSamples;
            
            float delayLineOutput = delayLine[readIndex1] * offset_frac + delayLine[readIndex2]* (1.0 - offset_frac);
            
            delayLine[writeIndex++] = inBuffer[sample];
            if (writeIndex >= lengthInSamples)  writeIndex -= lengthInSamples;
            
            outBuffer[sample] = delayLineOutput;
        }
    }
    
    
    /** Calling this method requires prior call to setDelayLength() */
    void processBlock(const float* inBuffer, float* outBuffer, int numSamples)
    {
        for(int sample = 0; sample < numSamples; sample++){
            
            readIndex1 = writeIndex - offset_int;
            readIndex2 = readIndex1 - 1;
            while(readIndex1 < 0)   readIndex1 += lengthInSamples;
            while(readIndex2 < 0)   readIndex2 += lengthInSamples;
            
            float delayLineOutput = delayLine[readIndex1] * offset_frac + delayLine[readIndex2]* (1.0 - offset_frac);
            
            delayLine[writeIndex++] = inBuffer[sample];
            if (writeIndex >= lengthInSamples)  writeIndex -= lengthInSamples;
            
            outBuffer[sample] = delayLineOutput;
        }
    }
    
    
    /** Pushes a single sample into delayline (FIFO). pullSample() can read out the delayline
     @param Input sample for delayline */
    void pushSample(float inSample)
    {
        delayLine[writeIndex++] = inSample;
        if (writeIndex >= lengthInSamples)  writeIndex -= lengthInSamples;
    }
    
    
    /** 
     Returns a single sample with relative delay to last pushed sample (with pushSample()). Multiple calls to pullSample() without calling pushSample will return the same sample
     @return Output sample of delayline
     - important: Calling this method requires prior call to setDelayLength()*/
    float pullSample()
    {
        readIndex1 = writeIndex - offset_int;
        readIndex2 = readIndex1 - 1;
        while(readIndex1 < 0)   readIndex1 += lengthInSamples;
        while(readIndex2 < 0)   readIndex2 += lengthInSamples;
        
        return delayLine[readIndex1] * offset_frac + delayLine[readIndex2]* (1.0 - offset_frac);
    }
    
    
    /** Returns a single sample with relative delay to last pushed sample (with pushSample())
     @return Output sample of delayline */
    float pullSample(float delayInMs)
    {
        float offset = delayInMs * (float)sampleRate * 0.001;
        offset_int = truncf(offset);
        offset_frac = offset - (float)offset_int;
        
        readIndex1 = writeIndex - offset_int;
        readIndex2 = readIndex1 - 1;
        while(readIndex1 < 0)   readIndex1 += lengthInSamples;
        while(readIndex2 < 0)   readIndex2 += lengthInSamples;
        
        return delayLine[readIndex1] * offset_frac + delayLine[readIndex2]* (1.0 - offset_frac);
    }
    
    
    /** Method used to set the delay length, e.g. for static or infrequently modulated delay length. Calling this method is necessary for calling a process-block or sample-pulling method without specifying the delay length */
    void setDelayLength(float delayInMs)
    {
        if(delayInMs > maxLengthMs){
            maxLengthMs = delayInMs + 10.0;
            forceReallocationFlag = true;
            prepareToPlay(sampleRate);
        }
        float offset = delayInMs * (float)sampleRate * 0.001;
        offset_int = truncf(offset);
        offset_frac = offset - (float)offset_int;
        
    }
    
    
    void prepareToPlay(int _sampleRate)
    {
        if(sampleRate != _sampleRate || forceReallocationFlag){
            sampleRate = _sampleRate;
            lengthInSamples = maxLengthMs * (float)sampleRate * 0.001;
            
            if(delayLine == NULL)
            {
                delayLine = (float*)malloc(lengthInSamples * sizeof(float));
                if(delayLine == NULL)
                    ErrorHandling::reportError("Delay Module", ERRMALLOC, true);
            }
            else
            {
                float *tmp_ptr = (float*)realloc(delayLine, lengthInSamples * sizeof(float));
                if (tmp_ptr == NULL)
                    ErrorHandling::reportError("Delay Module", ERRMALLOC, true);
                else
                    delayLine = tmp_ptr;
            }
        }
        
        for(int i = 0; i < lengthInSamples; i++)
            delayLine[i] = 0.0;
        writeIndex = readIndex1 = readIndex2 = 0;
        offset_int = 0;
        offset_frac = 0.0;
        
    }
    
    
    /** Use this function to overwrite the default 500ms delayline length. Pass a larger value in case a bigger delay is needed or pass a smaller value to save memory 
     @important: This will only take effect if prepareToPlay() is called afterwards
     */
    void specifyMaxDelayLength(float delayLengthInMs)
    {
        maxLengthMs = delayLengthInMs;
    }

    
private:
    
    int sampleRate;
    float maxLengthMs;
    int lengthInSamples;
    
    
    
    //delayline stuff
    float* delayLine;
    int writeIndex;
    int readIndex1, readIndex2;
    float offset_frac;
    int offset_int;
    bool forceReallocationFlag = false;
    
};

