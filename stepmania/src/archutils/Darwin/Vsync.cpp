/*
 *  Vsync.cpp
 *  stepmania
 *
 *  Created by Steve Checkoway on Mon Sep 22 2003.
 *  Copyright (c) 2003 Steve Checkoway. All rights reserved.
 *
 */

#include "Vsync.h"
#include <OpenGl/OpenGl.h>


bool wglSwapIntervalEXT(int swapInterval)
{
    /* I really have no idea what the expected return value for success. If it was an int
    * I would return 0, since it is a bool, I'm going to guess that true is success. */
    return !CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &(long(swapInterval)));
}
