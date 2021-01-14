//
//  InputHandler_NSEvent.hpp
//  StepMania
//
//  Created by heshuimu on 12/22/19.
//

#ifndef InputHandler_NSEvent_hpp
#define InputHandler_NSEvent_hpp

#include "InputHandler.h"

#include <stdio.h>

class InputHandler_NSEvent : public InputHandler
{
public:
    InputHandler_NSEvent();
    ~InputHandler_NSEvent();
    
    void GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut );
};

#endif /* InputHandler_NSEvent_hpp */
