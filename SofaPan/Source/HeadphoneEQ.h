/*
  ==============================================================================

    HeadphoneEQ.h
    Created: 16 Jan 2018 2:46:22pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "fftw3.h"


class HeadphoneEQ{
    
public:
    HeadphoneEQ(){
        hptf_l = NULL;
        hptf_r = NULL;
        fftInputBufferL = NULL;
        fftInputBufferR = NULL;
        fftOutputBufferL = NULL;
        fftOutputBufferR = NULL;
        magSpectrum = NULL;
    
        formatManager.registerBasicFormats();


    }
    ~HeadphoneEQ(){
        if(hptf_l != NULL) fftwf_free(hptf_l);
        if(hptf_r != NULL) fftwf_free(hptf_r);
        if(fftInputBufferL != NULL) fftwf_free(fftInputBufferL);
        if(fftInputBufferR != NULL) fftwf_free(fftInputBufferR);
        if(fftOutputBufferL != NULL) fftwf_free(fftOutputBufferL);
        if(fftOutputBufferR != NULL) fftwf_free(fftOutputBufferR);
        if(magSpectrum != NULL) fftwf_free(magSpectrum);

    }
    
    
    void process(float* inOutBuffer_L, float* inOutBuffer_R, int numSamples){
        
        
        for(int sample = 0; sample < numSamples; sample++){
            
            fftInputBufferL[fifoIndex] = inOutBuffer_L[sample];
            fftInputBufferR[fifoIndex] = inOutBuffer_R[sample];

            inOutBuffer_L[sample] = fftOutputBufferL[fifoIndex];
            inOutBuffer_R[sample] = fftOutputBufferR[fifoIndex];
            
            fifoIndex++;
            
            if(fifoIndex == lengthOfFIR){
                fifoIndex = 0;
                filterL.processBlockMono(fftInputBufferL, fftOutputBufferL, hptf_l);
                filterR.processBlockMono(fftInputBufferR, fftOutputBufferR, hptf_r);


            }
        }
    }
    
    void prepareToPlay(){
        fifoIndex = 0;
        filterL.prepareToPlay(hptf_l);
        filterR.prepareToPlay(hptf_r);

    }
    
    int initWithPathToHPIR(File filterDataWaveFile, int _sampleRate){
        
        if(hptf_l != NULL) fftwf_free(hptf_l);
        if(hptf_r != NULL) fftwf_free(hptf_r);
        if(fftInputBufferL != NULL) fftwf_free(fftInputBufferL);
        if(fftInputBufferR != NULL) fftwf_free(fftInputBufferR);
        if(fftOutputBufferL != NULL) fftwf_free(fftOutputBufferL);
        if(fftOutputBufferR != NULL) fftwf_free(fftOutputBufferR);
        if(magSpectrum != NULL) fftwf_free(magSpectrum);

        sampleRate = _sampleRate;
                
        
        //Load file
        AudioSampleBuffer fileBuffer;
        ScopedPointer<AudioFormatReader> reader (formatManager.createReaderFor (filterDataWaveFile));
        
        int numSamples;
        int originalSampleRate;
        
        if (reader != nullptr)
        {
            numSamples = reader->lengthInSamples;
            originalSampleRate = reader->sampleRate;
            
            fileBuffer.setSize (reader->numChannels, reader->lengthInSamples);
            reader->read (&fileBuffer,
                          0,
                          reader->lengthInSamples,
                          0,
                          true,
                          true);
            
        }
        
        
        printf("\n[HEQ] numSamples: %d, fileSampleRate: %d, hostSampleRate: %d", numSamples, originalSampleRate, sampleRate);

        lengthOfFIR = getIRLengthForNewSampleRate(numSamples, originalSampleRate, sampleRate);
        lengthOfFFT = 2 * lengthOfFIR;
        lengthOfTF = (lengthOfFFT * 0.5) + 1;
        
        printf("\n[HEQ] lengthOfFIR: %d, lengthOfFFT: %d", lengthOfFIR, lengthOfFFT);
        
        float* hpir_l = fftwf_alloc_real(lengthOfFFT);
        float* hpir_r = fftwf_alloc_real(lengthOfFFT);
        
        hptf_l = fftwf_alloc_complex(lengthOfTF);
        hptf_r = fftwf_alloc_complex(lengthOfTF);
        
        magSpectrum = (float*)malloc(lengthOfTF * 2 * sizeof(float));

        
        //Interpolate and store in hpir
        const float* readPointer = fileBuffer.getReadPointer(0);
        sampleRateConversion(fileBuffer.getReadPointer(0), hpir_l, numSamples, lengthOfFIR, originalSampleRate, sampleRate);
        sampleRateConversion(fileBuffer.getReadPointer(1), hpir_r, numSamples, lengthOfFIR, originalSampleRate, sampleRate);

        //Zeropadding
        for(int i = 0; i < lengthOfFIR; i++){
            hpir_l[i + lengthOfFIR] = 0.0;
            hpir_r[i + lengthOfFIR] = 0.0;
        }
        
        fftwf_plan FFT = fftwf_plan_dft_r2c_1d(lengthOfFFT, hpir_l, hptf_l, FFTW_ESTIMATE);
        fftwf_execute(FFT);
        fftwf_destroy_plan(FFT);

        FFT = fftwf_plan_dft_r2c_1d(lengthOfFFT, hpir_r, hptf_r, FFTW_ESTIMATE);
        fftwf_execute(FFT);
        fftwf_destroy_plan(FFT);
        
        //Save Spectrum for plotting
        for(int k = 0; k < lengthOfTF; k++){
            magSpectrum[k] = (sqrtf(hptf_l[k][0] * hptf_l[k][0] + hptf_l[k][1] * hptf_l[k][1]));
            magSpectrum[k + lengthOfTF] = (sqrtf(hptf_r[k][0] * hptf_r[k][0] + hptf_r[k][1] * hptf_r[k][1]));
        }
        
        fftwf_free(hpir_l);
        fftwf_free(hpir_r);
        
        fftInputBufferL = fftwf_alloc_real(lengthOfFIR);
        fftInputBufferR = fftwf_alloc_real(lengthOfFIR);
        fftOutputBufferL = fftwf_alloc_real(lengthOfFIR);
        fftOutputBufferR = fftwf_alloc_real(lengthOfFIR);
        
        filterL.init(lengthOfFIR);
        filterR.init(lengthOfFIR);

        
        return 0;
        
    }
    
    int getLengthOfHPIR(){
        return lengthOfFIR;
    }
    
    float* getMagSpectrum(){
        return magSpectrum;
    }
    
private:
    int sampleRate;
    
    int lengthOfFIR = 0;
    int lengthOfTF;
    int lengthOfFFT;
    
    float* fftInputBufferL;
    float* fftInputBufferR;

    float* fftOutputBufferL;
    float* fftOutputBufferR;
    
    int fifoIndex;
    
    void releaseResources();
    
    FilterKernel filterL;
    FilterKernel filterR;
    
    
    fftwf_complex* hptf_l;
    fftwf_complex* hptf_r;
    
    float* magSpectrum;

    
    AudioFormatManager formatManager;
    
    
    int sampleRateConversion(const float* inBuffer, float* outBuffer, int n_inBuffer, int n_outBuffer, int originalSampleRate, int newSampleRate){
        
        
        float frequenzFaktor = (float)originalSampleRate / (float)newSampleRate;
        float f_index = 0.0f;
        int i_index = 0;
        float f_bruch = 0;
        int i_fganzezahl = 0;
        while ((i_fganzezahl+1) < n_inBuffer){
            if(i_index >= n_outBuffer || (i_fganzezahl+1) >= n_inBuffer) return 1;
            //Linear interpolieren
            outBuffer[i_index] = inBuffer[i_fganzezahl] * (1.0f - f_bruch) + inBuffer[i_fganzezahl + 1] * f_bruch;
            outBuffer[i_index] *= frequenzFaktor;
            //Berechnungen fuer naechste Runde.
            i_index++;
            f_index = i_index * frequenzFaktor;
            i_fganzezahl = (int)f_index;
            f_bruch = f_index - i_fganzezahl;
        }
        while(i_index < n_outBuffer){
            outBuffer[i_index] = 0.0;
            i_index++;
        }
        
        return 0;
        
    }
    
    
    int getIRLengthForNewSampleRate(int IR_Length, int original_SR, int new_SR){
        
        float frequenzFaktor = (float)original_SR / (float)new_SR;
        float newFirLength_f = (float)(IR_Length) / frequenzFaktor; //z.B. 128 bei 44.1kHz ---> 128/(44.1/96) = 278.6 bei 96kHz
        int fir_length = 2;
        while (fir_length < newFirLength_f) {
            fir_length *= 2;
        }
        
        return fir_length;
    }
    
};

