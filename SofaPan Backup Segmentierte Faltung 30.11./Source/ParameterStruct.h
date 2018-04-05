/*
  ==============================================================================

    ParameterStruct.h
    Created: 19 Aug 2017 12:22:34pm
    Author:  David Bau

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

typedef struct{
    AudioParameterFloat* azimuthParam;
    AudioParameterFloat* elevationParam;
    AudioParameterFloat* distanceParam;
    AudioParameterFloat* bypassParam;
    AudioParameterBool* testSwitchParam;
    AudioParameterBool* mirrorSourceParam;
    AudioParameterBool* distanceSimulationParam;
    AudioParameterBool* nearfieldSimulationParam;
    AudioParameterBool* ITDAdjustParam;
}parameterStruct;


