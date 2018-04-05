/*
  ==============================================================================

    ITDToolkit.h
    Created: 13 Oct 2017 10:48:13am
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#define MAX_ITD 1

class ITDToolkit{
public:
    
    //Negative Values represent a position to the left of the head (delay left is smaller), positive values for right of the head
    
    //headradius in m, returnvalue: ITD in ms
    static float woodworthSphericalITD(float headRadius, float azimuth, float elevation){
        
        float ITD, ITDnf;
        float r = headRadius;
        float c = 340.29;
        
        //calculate lateral angle as demanded for woodworths formula (-> where medianplane is fundamental plane of spherical coordinate system)
        float omegaLateral = asinf(sinf(M_PI * azimuth / 180.0) * cosf(M_PI * elevation / 180.0));
        
        //Apply woodworths formula
        ITD = 1000 * (r /c) * (sinf(omegaLateral) + omegaLateral);
        ITDnf = 1000 * (r /c) * 2 * omegaLateral;
        
        if(ITD > MAX_ITD) ITD = MAX_ITD;
        if(ITD < -MAX_ITD) ITD = -MAX_ITD;

        printf("\nZ: %.3f ITD: %f", omegaLateral, ITD);

        return ITD;
    }
    
    //[ITD = 3*r/c * sin(phi)], for lower frequencies, (ka)^2 << 1
    static float kuhnSphericalITD(float headRadius, float azimuth, float elevation){
        float ITD;
        float r = headRadius;
        float c = 340.29;
        //calculate lateral angle as demanded for woodworths formula (-> where medianplane is fundamental plane of spherical coordinate system)
        float omegaLateral = asinf(sinf(M_PI * azimuth / 180.0) * cosf(M_PI * elevation / 180.0));
        //Apply woodworths formula
        ITD = 1000 * (3*r /c) * sinf(omegaLateral);
        
        if(ITD > MAX_ITD) ITD = MAX_ITD;
        if(ITD < -MAX_ITD) ITD = -MAX_ITD;
        
        
        return ITD;
    }
    
    
    static int crossCorr(float* buffer1, float* buffer2, int bufLength){
        int tau;
        float corr;
        float maxCorr = 0.0;
        
        for(int t = -bufLength/2; t < bufLength/2; t++){
            corr = 0.0;
            for(int i = 0; i < bufLength; i++){
                int sampleIndex = (i+t + bufLength) % bufLength;
                corr += buffer2[i] * buffer1[sampleIndex];
            }
            if (corr > maxCorr){
                maxCorr = corr;
                tau = t;
            }
        }
        
        return tau;
    }
    
    static int detectOnset(float* buffer, int bufLength, float threshold, bool goToPrecedingZeroCrossing){
        
        //Determine maximum amplitude
        float maxValue = 0.0;
        for(int i = 0; i< bufLength; i++){
            if (fabs(buffer[i]) > maxValue)
                maxValue = fabs(buffer[i]);
        }
        
        //Determine Onset Threshold
        maxValue *= threshold;
        
        //Determine Onset Positions
        int onsetIndex = 0;
        for(int i = 0; i< bufLength; i++){
            if (fabs(buffer[i]) > maxValue){
                onsetIndex = i;
                break;
            }
        }
        
        if(goToPrecedingZeroCrossing){
            if(buffer[onsetIndex] > 0.0){
                while(buffer[onsetIndex - 1] > 0)
                    onsetIndex -= 1;
                
            }else if(buffer[onsetIndex] > 0.0){
                while(buffer[onsetIndex - 1] < 0)
                    onsetIndex -= 1;
            }
            
        }
        
        return onsetIndex;
    }
        
};
