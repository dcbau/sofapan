/*
  ==============================================================================

    NFSimToolkit.h
    Created: 14 Oct 2017 11:23:39am
    Author:  David Bau

  ==============================================================================
*/

#pragma once

class NFSimToolkit{
public:
    //Nearfield Simulation: Increasing IID effect
    static float getAdaptedIIDGain(float distance, float azimuth, float elevation, int ear){
       
        if(distance < 1.0){
           
            //limit to avoid earbursting gain
            if(distance < 0.2) distance = 0.2;
            
            //alpha_az determines how much the increasing IID effect will be applied, because it is dependent on the azimuth angle
            float alpha_az = powf( sinf(juce::degreesToRadians(azimuth)  ), 3 ); //Running from 0 -> 1 -> 0 -> -1 -> 0 for a full circle. The ^3 results in a steeper sine-function, so that the effect will be less present in the areas around 0° or 180°, but emphasized for angles 90° or 270°, where the source is cleary more present to one ear, while the head masks the other ear
           
            //alpha_el weakens the effect, if the source is elevated above or below the head, because shadowing of the head will be less present. It is 1 for zero elevation and moves towards zero for +90° and -90°
            float alpha_el = cosf(juce::degreesToRadians(elevation));
           
            //normGain will run exponentially from +0db to ~+5db when distance goes from 1m to 0.2m
            float normGain = 7.8 * (1.0 - distance) * (1.0 - distance);
           
            if(ear == 0)
                return Decibels::decibelsToGain(normGain * -alpha_az * alpha_el); // 0db -> 0...-5db -> 0db -> 0..5db
            else if(ear == 1)
                return Decibels::decibelsToGain(normGain * alpha_az * alpha_el); // 0db -> 0...5db -> 0db -> 0..-5db
            else
                return 1.0;
            
        }else{
            
            return 1.0;
        }
    }
    
    
        
    /* This simulation of nearfield hrtfs aims to model the acoustic parallax effect. The idea is that if a source approaches the head, the angle between the individual ears and the source will deviate from the orignal angle between the head centre and the source. With simple geometrical methods, the exact angle for every ear can be derivated (=> calculateNFAngleOffset). The used HRTF-pair will then consist of two different left and right HRTFs from different angles
     
     This idea was first discussed by Douglas S. Brungart in his article "AUDITORY PARALLAX EFFECTS IN THE HRTF FOR NEARBY SOURCES", published in 1999 in the Proceedings of the "IEEE Engineering in Medicine and Biology Society", Volume 20(3), pp.1101-1104.
     
     For further information, the dissertation of Fotis Georgiou constains a short introduction to the acoustic parallax effect on pages 15/16. It is available for free on researchgate.net and the title is "Relative Distance Perception Of Sound Sources In Critical Listening Environment Via Binaural Reproduction"
     
     */
    
    /**
     angle = angle from headcenter to source in degrees
     distance = distance from headcenter to source in m
     earPosition = distance from ear to headcenter in m, positive = left shift, negative = rightshift
     (e.g. 0.09 for left ear, -0.09 for right ear)
     return value = new angle in degrees
     */
    static float calculateNFAngleOffset(float angle, float distance, float earPosition){
        
        angle = juce::degreesToRadians(angle);
        
        float d = distance;
        float r = earPosition;
        
        float newAngle = 0.0;
        
        if( angle < M_PI / 2.0)
            newAngle = atan( (sin(angle) * d + r) / (cos(angle) * d) );
        else if(angle < M_PI)
            newAngle = M_PI - atan( (sin(M_PI-angle) * d + r) / (cos(M_PI-angle) * d) );
        else if(angle < M_PI*3/2)
            newAngle = M_PI + atan( (sin(angle) * d + r) / (cos(angle) * d) );
        else if(angle < M_PI * 2)
            newAngle = 2*M_PI - atan( (sin(M_PI-angle) * d + r) / (cos(M_PI-angle) * d) );
        
        if(newAngle < 0)
            newAngle = newAngle + 2*M_PI;
        if(newAngle > 2*M_PI)
            newAngle = newAngle - 2*M_PI;
        
        return juce::radiansToDegrees(newAngle);
        
    }
                                   
};
