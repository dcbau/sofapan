/*
  ==============================================================================

    ErrorHandling.h
    Created: 14 Aug 2017 4:19:43pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#define ERRMALLOC "Error while allocating memory"

class ErrorHandling{
private:
public:
    
    static void reportError(String errorSource, String errorMessage, bool critical) {
        
        String displayMessage;
        if(critical){
            displayMessage = String("Critical Error in " + errorSource + ":\n" + errorMessage + "\nClose and reload the app");
        }else{
            displayMessage = String("Error in " + errorSource + ":\n" + errorMessage);

        }
        
        AlertWindow::showNativeDialogBox("Binaural Renderer", displayMessage, false);
        
        
    }
    
};

