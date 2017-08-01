/*
  ==============================================================================

    PannerComponent.h
    Created: 7 Jul 2017 10:05:05am
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "sofaPanLookAndFeel.h"
#include "HeadTopHexData.h"
#include "HeadRearHexData.h"
#include "PluginEditor.h"

//==============================================================================
/*
*/

#define ROOMRADIUS 3.0
#define NONUNIFORMSCALE true


static struct {
    float azimuth, elevation, distance;
    void set(float _azimuth, float _elevation, float _distance){
        azimuth = _azimuth; elevation = _elevation; distance= _distance;
    }
}spherical;

static bool mouseInComponent = false;


typedef struct {
    float x, y;
    void set(float _x, float _y){
        x = _x; y = _y;
    }
}sourcePositionPixel;





class PannerComponent    : public Component, public InterprocessConnection
{
public:
    PannerComponent(SofaPanAudioProcessor& p, bool _isSideView)
    : processor(p)
    {
        
        //sourcePositionCartesian= std::vector<float>(0.0, 1.0, 0.0);
        spherical.set(0.0, 0.0, 1.0);
        isSideView = _isSideView;
        
        if(!isSideView)
            createPipe("1234", 10);
        else{
            sleep(1);
            connectToPipe("1234", 10);
        }

    }

    ~PannerComponent()
    {
    }

    void paint (Graphics& g) override
    {
        Rectangle<float> bounds = getLocalBounds().toFloat();
        const float boarderSize = 4.0;
        Rectangle<float> boarderBox = bounds.reduced(boarderSize / 2.0, boarderSize/2.0);

        g.fillAll (Colours::transparentWhite);   // clear the background
        g.setColour (Colours::grey);
        g.fillRoundedRectangle(bounds, 10.0);
        
        g.setColour (Colours::black);
        const float dashes[2] = {5, 10};
        g.drawDashedLine(Line<float>(Point<float>(boarderBox.getX(), boarderBox.getCentreY()), boarderBox.getCentre()), dashes, 2, 1, 1);
        g.drawDashedLine(Line<float>(Point<float>(boarderBox.getRight(), boarderBox.getCentreY()), boarderBox.getCentre()), dashes, 2, 1, 1);
        if(!isSideView){
            g.drawDashedLine(Line<float>(Point<float>(boarderBox.getCentreX(), boarderBox.getY()), boarderBox.getCentre()), dashes, 2, 1, 1);
            g.drawDashedLine(Line<float>(Point<float>(boarderBox.getCentreX(), boarderBox.getBottom()), boarderBox.getCentre()), dashes, 2, 1, 1);
        }
        g.setColour(Colours::darkgrey);
        g.drawRoundedRectangle (boarderBox.reduced(2, 2).translated(1, 1), 10.0,  5.0); //inner shadow
        g.setColour (sofaPanLookAndFeel.mainCyan);
        g.drawRoundedRectangle (boarderBox, 10.0,  5.0);
        
        Path p = Path();
        p.addEllipse(bounds.getWidth()/4.0, bounds.getWidth()/4.0,  bounds.getWidth()/2.0, bounds.getHeight()/2.0);
        PathStrokeType stroke(.5);
        const float dash[2] = {2.0, 5.0};
        stroke.createDashedStroke(p, p, dash, 2);
        g.strokePath(p, stroke);
        
        g.setColour(Colours::black);
        g.drawFittedText("1m", bounds.getWidth()*0.75 - 7, bounds.getHeight()*0.5, 14, 10, Justification::centred, 1);
        
        const int imageSize = boarderBox.getWidth() / 7;
        float scaleFor3DEffect;
        if(isSideView){
            scaleFor3DEffect = -y;
        }else{
            scaleFor3DEffect = z;
//printf("\nNorm Height = %f", scaleFor3DEffect);

        }
        scaleFor3DEffect /= 3.0;
        float sourceSize = 10 + scaleFor3DEffect * 2;
        float scaleFor3DEffect_pos = scaleFor3DEffect * 0.5 + 0.5;
        g.setColour(Colours::orange.withBrightness(0.7 + 0.1*scaleFor3DEffect_pos));
        Rectangle<float> source = Rectangle<float>(pixel.x - sourceSize/2., pixel.y - sourceSize/2., sourceSize, sourceSize);
        if(isSideView){
            if(scaleFor3DEffect > 0 ){
                g.drawImage(headRearImage, boarderBox.getCentreX()-imageSize/2, boarderBox.getCentreY()-imageSize/2 - 1, imageSize, imageSize, 0, 0, 200, 200);
                g.fillRoundedRectangle(source, sourceSize/3.);
            }else{
                g.fillRoundedRectangle(source, sourceSize/3.);
                g.drawImage(headRearImage, boarderBox.getCentreX()-imageSize/2, boarderBox.getCentreY()-imageSize/2 - 1, imageSize, imageSize, 0, 0, 200, 200);
            }
        }else{
            if(scaleFor3DEffect > 0 ){
                g.drawImage(headTopImage, boarderBox.getCentreX()-imageSize/2, boarderBox.getCentreY()-imageSize/2 - 1, imageSize, imageSize, 0, 0, 200, 200);
                g.fillRoundedRectangle(source, sourceSize/3.);
                
                
            }else{
                g.fillRoundedRectangle(source, sourceSize/3.);
                g.drawImage(headTopImage, boarderBox.getCentreX()-imageSize/2, boarderBox.getCentreY()-imageSize/2 - 1, imageSize, imageSize, 0, 0, 200, 200);
                
            }
        }
        
    
        g.setColour(Colours::black);
        String position = String(/*"X: " + String(sourcePositionMeter.x) + " | Y: " + String(sourcePositionMeter.y) + " \n  */ "A: " + shortenFloatString(String(spherical.azimuth), 2) + " | D: " + shortenFloatString(String(spherical.distance), 2));
        g.drawText(position, 0.0, boarderBox.getBottom() - 30., 150., 25., Justification::centred);
        
    }

    void resized() override
    {
        coordsToPixel();
    }
    
//    void positionToPixelCoordinates(){
//        if(NONUNIFORMSCALE){
//            Rectangle<float> bounds = getLocalBounds().toFloat();
//            float normX = cartesian.x / ROOMRADIUS;
//            float normY = - cartesian.y / ROOMRADIUS;
//            pixel.set(bounds.getCentreX() + normX * bounds.getWidth()/2.0, bounds.getCentreY() + normY * bounds.getHeight()/2.0);
//        }else{
//            Rectangle<float> bounds = getLocalBounds().toFloat();
//            float normX = cartesian.x / ROOMRADIUS;
//            float normY = - cartesian.y / ROOMRADIUS;
//            pixel.set(bounds.getCentreX() + normX * bounds.getWidth()/2.0, bounds.getCentreY() + normY * bounds.getHeight()/2.0);
//        }
//        
//    }
//    
//    void pixelCoordinatesToPosition(){
//        if(NONUNIFORMSCALE){
//            Rectangle<float> bounds = getLocalBounds().toFloat();
//            float normX = pixel.x / bounds.getWidth(); // 0 - 1
//            float normY = pixel.y / bounds.getHeight();
//            cartesian.x = (normX * 2.0 - 1.0) * ROOMRADIUS;
//            cartesian.y = - (normY * 2.0 - 1.0) * ROOMRADIUS;
//        }else{
//            Rectangle<float> bounds = getLocalBounds().toFloat();
//            float normX = pixel.x / bounds.getWidth(); // 0 - 1
//            float normY = pixel.y / bounds.getHeight();
//            cartesian.x = (normX * 2.0 - 1.0) * ROOMRADIUS;
//            cartesian.y = - (normY * 2.0 - 1.0) * ROOMRADIUS;
//        }
//    }

    void pixelToCoords(){
        
        //float x, y, z;
        if(isSideView){
            Rectangle<float> bounds = getLocalBounds().toFloat();
            float normX = pixel.x / bounds.getWidth(); // 0 - 1
            float normZ = pixel.y / bounds.getHeight();
            x =  (normX * 2.0 - 1.0) * ROOMRADIUS;
            z = - (normZ * 2.0 - 1.0) * ROOMRADIUS;
            //x = sinf(spherical.azimuth * M_PI / 180.0) * cosf(spherical.elevation * M_PI / 180.0) * spherical.distance;
            printf("\nSIDE: x: %.2f | y: %.2f | z: %.2f", x, y, z);
        }else{
            Rectangle<float> bounds = getLocalBounds().toFloat();
            float normX = pixel.x / bounds.getWidth(); // 0 - 1
            float normY = pixel.y / bounds.getHeight();
            x = (normX * 2.0 - 1.0) * ROOMRADIUS;
            y = - (normY * 2.0 - 1.0) * ROOMRADIUS;
            //z = spherical.distance * sinf(spherical.elevation * M_PI / 180.);
            printf("\nTOP: x: %.2f | y: %.2f | z: %.2f", x, y, z);

        }
        spherical.azimuth = atan2f(y , x) *  180.0/ M_PI - 90.0;
        spherical.azimuth = spherical.azimuth > 0.0 ? 360.0 - spherical.azimuth : -spherical.azimuth;
        spherical.elevation = atan2f(z, sqrtf(x*x + y*y))* 180.0/ M_PI;
        //printf("elevation = %f", spherical.elevation);

        spherical.distance = sqrtf(y * y  + x * x + z * z);

        if(NONUNIFORMSCALE){
            const float d = spherical.distance;
            const float r = ROOMRADIUS;
            const float r2 = r/2.0;
            
            if(d < r2){
                spherical.distance = d/r2; //nearfield: 0m ... 1m
            }else{
                spherical.distance = (d/r2 - 1.0) * (r-1.0) + 1; // farfield: 1m ... Rm
            }
        }
        
        
    }
    
    void coordsToPixel(){
        
        float distance;
        if(NONUNIFORMSCALE){
            const float d = spherical.distance;
            const float r = ROOMRADIUS;
            const float r2 = r/2.0;
            
            if(d < 1.0){
                distance = d * r2;
            }else{
                distance = ((d-1)/(r-1) + 1) * r2;
                //                spherical.distance = d/r2 * (r-1.0) - r + 2;
            }
        }
        z = distance * sinf(spherical.elevation * M_PI / 180.);
        x = sinf(spherical.azimuth * M_PI / 180.0) * cosf(spherical.elevation * M_PI / 180.0) * distance;
        y = cosf(spherical.azimuth * M_PI / 180.0) * cosf(spherical.elevation * M_PI / 180.0) * distance;
        
        //z = spherical.distance * sinf(spherical.elevation * M_PI / 180.);
        //x = sinf(spherical.azimuth * M_PI / 180.0) * cosf(spherical.elevation * M_PI / 180.0) * spherical.distance;
        //y = cosf(spherical.azimuth * M_PI / 180.0) * cosf(spherical.elevation * M_PI / 180.0) * spherical.distance;
//
        if(isSideView){
            Rectangle<float> bounds = getLocalBounds().toFloat();
            float normZ = - z / ROOMRADIUS;
            float normX = x / ROOMRADIUS;
            pixel.set(bounds.getCentreX() + normX * bounds.getWidth()/2.0, bounds.getCentreY() + normZ * bounds.getHeight()/2.0);
        }else{
            Rectangle<float> bounds = getLocalBounds().toFloat();
            float normX = x / ROOMRADIUS;
            float normY = - y / ROOMRADIUS;
            pixel.set(bounds.getCentreX() + normX * bounds.getWidth()/2.0, bounds.getCentreY() + normY * bounds.getHeight()/2.0);
        }
    }

    void mouseDrag(const MouseEvent &  	event) override{
        
        Rectangle<float> box = getLocalBounds().toFloat().reduced(4, 4);
        
        pixel.set(event.getPosition().x, event.getPosition().y);
        if(pixel.x > box.getRight())    pixel.x = box.getRight();
        if(pixel.x < 4.0)   pixel.x = 4.0;
        if(pixel.y > box.getBottom())   pixel.y = box.getBottom();
        if(pixel.y < 4.0)   pixel.y = 4.0;

        pixelToCoords();
        
//        //clipping if distance is too narrow or too far
//        if(spherical.distance < 0.2 || spherical.distance > ROOMRADIUS) {
//            spherical.distance = spherical.distance < 0.2 ? 0.2 : ROOMRADIUS;
//            sphericalToPixel();
//        }
        
        
        
        setParameterValue("elevation", (spherical.elevation / 180.0) + 0.5);
        setParameterValue("azimuth", spherical.azimuth/360.0);
        setParameterValue("distance", spherical.distance);
        
        repaint();
        MemoryBlock emptyMessage = MemoryBlock(8, true);// = new MemoryBlock
        sendMessage(emptyMessage);
    }
    
    void mouseEnter(const MouseEvent &event)override{
        mouseInComponent = true;
    }
    void mouseExit(const MouseEvent &event)override{
        mouseInComponent = false;
        
    }
    String shortenFloatString(String _string, int decimalValues){
        if(_string.containsChar('.'))
            return _string.substring(0, _string.indexOfChar('.') + decimalValues + 1);
        else{
            _string.append(".00000000000", decimalValues + 1);
            return _string;
        }
    }
    
    AudioProcessorParameter* getParameter (const String& paramId)
    {
        const OwnedArray<AudioProcessorParameter>& params = processor.getParameters();
        
        for (int i = 0; i < params.size(); ++i){
            if (AudioProcessorParameterWithID* param = dynamic_cast<AudioProcessorParameterWithID*> (params[i])){
                if (param->paramID == paramId)
                    return param;
            }
        }
        return nullptr;
    }
    
    
    void setParameterValue (const String& paramId, float value)
    {
        if (AudioProcessorParameter* param = getParameter (paramId))
            param->setValueNotifyingHost(value);
    }
    
    void setDistanceAndAngle( float _distance, float _angle, float _angle_elevation){
        if(!mouseInComponent){ //to prevent redundant calls to both windows
//            if(isSideView)
//                printf("Set Distance And Angle SIDE \n");
//            else
//                printf("Set Distance And Angle TOP \n");

            //if(!isSideView){ //DANGER: this is a small workaround. The values are always set first on the top-panner, so they dont need to be set on the side-panner again
                spherical.azimuth = _angle;
                spherical.distance = _distance;
                spherical.elevation = _angle_elevation;
            //}
            coordsToPixel();
            repaint();
        }
    }
    
    
    void connectionMade() override{
        printf("SUPER!!!");
    }
    void connectionLost() override{
    }
    void messageReceived (const MemoryBlock &message) override{
        printf(isSideView ? "Message Received Side \n" : "Message Received Top \n");
        coordsToPixel();
        repaint();
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PannerComponent)
    SofaPanLookAndFeel sofaPanLookAndFeel;
    const Image headTopImage = ImageCache::getFromMemory(headTopPicto, headTopPicto_Size);
    const Image headRearImage = ImageCache::getFromMemory(headRearPicto, headRearPicto_Size);
    bool isSideView;
    float x, y, z;
    
    sourcePositionPixel pixel;
    //static sourcePositionSpherical spherical;

    SofaPanAudioProcessor& processor;

};
