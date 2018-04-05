/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "RoomPannerComponent.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   :  public Component,
                                private Slider::Listener,
                                private Button::Listener,
                                private OSCReceiver,
                                private OSCReceiver::ListenerWithOSCAddress<OSCReceiver::MessageLoopCallback>,
                                private Timer

{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    void sliderValueChanged (Slider* slider) override;
    void buttonClicked (Button* button) override;
    void showConnectionErrorMessage (const String& messageText);
    void oscMessageReceived(const OSCMessage& message) override;
    void timerCallback() override;

    
    Slider rotaryKnob;
    OSCSender sender;
    TextEditor editIPTextBox;
    TextEditor editPortTextBox;
    TextButton connectButton;
    Label connectStateLabel;
    
    Rectangle<int> topBox;
    RoomPannerComponent panner2D_top;
    RoomPannerComponent panner2D_rear;
    
    int connectionTimeout = 0;
    String targetIPAdress;
    int targetPortNumber;
    String myIPAdress;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
