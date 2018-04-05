

//  Created by David Rowland on 22/01/2012.
//  Copyright (c) 2012 dRowAudio. All rights reserved.
//
//==============================================================================

#pragma once

class Accelerometer
{
public:
    //==============================================================================
    Accelerometer();
    virtual ~Accelerometer();
    virtual void accelerometerChanged (float x, float y, float z) = 0;
private:
    //==============================================================================
    void* accelerometerManager;
};
