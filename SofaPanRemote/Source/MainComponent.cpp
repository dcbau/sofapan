/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"


//==============================================================================
MainContentComponent::MainContentComponent(): panner2D_top(false), panner2D_rear(true)
{
    
    #if JUCE_ANDROID || JUCE_IOS
        auto screenBounds = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
        setSize (screenBounds.getWidth(), screenBounds.getHeight());
    #else
        setSize (800, 600);
    #endif
    
    rotaryKnob.setRange (0.0, 1.0);
    rotaryKnob.setSliderStyle (Slider::RotaryVerticalDrag);
    rotaryKnob.setTextBoxStyle (Slider::TextBoxBelow, true, 150, 25);
    addAndMakeVisible (rotaryKnob);
    rotaryKnob.addListener (this);
    
    editIPTextBox.setText("192.168.178.30");
    addAndMakeVisible(editIPTextBox);
    
    editPortTextBox.setText("9001");
    addAndMakeVisible(editPortTextBox);
    
    connectButton.setButtonText("Connect!");
    connectButton.addListener(this);
    addAndMakeVisible(connectButton);
    
    addAndMakeVisible(connectStateLabel);
    
    targetIPAdress = String("192.168.178.30");
    targetPortNumber = 9001;
    // Connect to sender socket
    if (!sender.connect (targetIPAdress, targetPortNumber)){
        showConnectionErrorMessage ("Error: could not connect to IP" + targetIPAdress +":" + String(targetPortNumber));
    }
    Array<IPAddress> addresses;
    IPAddress::findAllAddresses (addresses);
    for (int i = 0; i < addresses.size(); i++){
        if(addresses[i].toString().substring(0, 6).compare(targetIPAdress.substring(0, 6)) == 0)
            myIPAdress = addresses[i].toString();
    }
    
    //Connect to receiver socket
    if(!connect(9001))
        showConnectionErrorMessage("Error: could not connect to receiver UDP port 9001.");
    
    addListener(this, "/sofapan/connectHandshake");
    
    startTimer(1000);
}

MainContentComponent::~MainContentComponent()
{
}

void MainContentComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setFont (Font (16.0f));
    g.setColour (Colours::darkgrey);
    
    g.fillRect(topBox);
}

void MainContentComponent::resized()
{
    rotaryKnob.setBounds (100, 200, 180, 180);
    
    Rectangle<int> rect = getLocalBounds().withBottom(getBounds().getWidth()/10);
    topBox = rect;
    
    connectStateLabel.setBounds(rect.removeFromBottom(rect.getHeight()/3));
    connectStateLabel.setFont(Font(connectStateLabel.getBounds().getHeight() * 0.6));
    
    editIPTextBox.setBounds(rect.removeFromLeft(rect.getWidth()/2).reduced(5));
    editIPTextBox.setFont(Font(editIPTextBox.getBounds().getHeight() * 0.6));
    
    editPortTextBox.setBounds(rect.removeFromLeft(rect.getWidth()/2).reduced(5));
    editPortTextBox.setFont(Font(editPortTextBox.getBounds().getHeight() * 0.6));

    connectButton.setBounds(rect.reduced(5));
    
    
    
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

void MainContentComponent::sliderValueChanged (Slider* slider)
{
    if (slider == &rotaryKnob)
    {
        // create and send an OSC message with an address and a float value:
        if (! sender.send ("/sofapan/rotaryknob", (float) rotaryKnob.getValue()))
            showConnectionErrorMessage ("Error: could not send OSC message.");
    }
}

void MainContentComponent::buttonClicked (Button* button) {
    
    if(button == &connectButton){
        
        if (! sender.send ("/sofapan/connectHandshake", (float)10.0))
            showConnectionErrorMessage ("Error: could not send OSC message.");
        
        targetIPAdress = editIPTextBox.getText().trim();
        targetPortNumber = editPortTextBox.getText().getIntValue();
        
        sender.disconnect();
        

        if (sender.connect (targetIPAdress, targetPortNumber)){
            sender.send("/sofapan/handshake", (float)30.0);
        }else{
            showConnectionErrorMessage ("Error: could not connect to IP " + targetIPAdress + ", UDP port " + String(targetPortNumber));
        }
        
        Array<IPAddress> addresses;
        IPAddress::findAllAddresses (addresses);
        
        
        for (int i = 0; i < addresses.size(); i++){
            if(addresses[i].toString().substring(0, 6).compare(targetIPAdress.substring(0, 6)) == 0)
                myIPAdress = addresses[i].toString();
        }
        
    }
    
}

void MainContentComponent::oscMessageReceived(const OSCMessage& message) {
    std::cout << "MessageRecevied";
    
    if(message.getAddressPattern().matches("/sofapan/connectHandshake") && message[0].isString()){
        if(message[0].getString() == "ACK"){
            connectionTimeout = 4;
            connectStateLabel.setText("Connected to " + targetIPAdress + ":" + String(targetPortNumber), NotificationType::dontSendNotification);
        }
    }
}

void MainContentComponent::timerCallback(){
    
    if(connectionTimeout <= 0){
        connectStateLabel.setText("Not Connected", NotificationType::dontSendNotification);
    }else{
        connectionTimeout--;
    }
    sender.send("/sofapan/connectHandshake", String("SYN:" + myIPAdress));
//    if (! sender.send ("/sofapan/connectHandshake", String("SYN:" + myIPAdress)))
//        showConnectionErrorMessage ("Error: could not send OSC message.");
    
}
                                      

void MainContentComponent::showConnectionErrorMessage (const String& messageText)
{
    AlertWindow::showMessageBoxAsync (
                                      AlertWindow::WarningIcon,
                                      "Connection error",
                                      messageText,
                                      "OK");
    
    
}
