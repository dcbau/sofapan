/*
  ==============================================================================

    AccelerometerManager.cpp
    Created: 19 Dec 2017 1:30:17pm
    Author:  David Bau

  ==============================================================================
*/

//  Created by David Rowland on 22/01/2012.
//  Copyright (c) 2012 dRowAudio. All rights reserved.
//
#import "AccelerometerManager.h"
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <UIKit/UIKit.h>




//==============================================================================
@interface AccelerometerManager : NSObject <UIAccelerometerDelegate>
{
    Accelerometer* owner;
}
@end
//==============================================================================
@implementation AccelerometerManager
- (void) accelerometer: (UIAccelerometer *) accelerometer
didAccelerate: (UIAcceleration *) acceleration
{
    owner->accelerometerChanged (acceleration.x, acceleration.y, acceleration.z);
}
- (void) startAccelerometer
{
    UIAccelerometer *accelerometer = [UIAccelerometer sharedAccelerometer];
    accelerometer.delegate = self;
    accelerometer.updateInterval = 0.25;
}
- (void) stopAccelerometer
{
    UIAccelerometer *accelerometer = [UIAccelerometer sharedAccelerometer];
    accelerometer.delegate = nil;
}
- (id) initWithOwner: (Accelerometer*) owner_
{
    if ((self = [super init]) != nil)
    {
        owner = owner_;
        [self startAccelerometer];
    };
    return self;
}
- (void) dealloc
{
    [self stopAccelerometer];
}
@end

//==============================================================================
Accelerometer::Accelerometer()
{
    AccelerometerManager* newManager = [[AccelerometerManager alloc] initWithOwner: this];
    [newManager retain];
    accelerometerManager = newManager;
}
Accelerometer::~Accelerometer()
{
    [((AccelerometerManager*) accelerometerManager) release];
}

