//
//  InputHandler_NSEvent.hpp
//  StepMania
//
//  Created by heshuimu on 12/22/19.
//

#ifndef InputHandler_NSEvent_hpp
#define InputHandler_NSEvent_hpp

#include "InputHandler.h"

#include <cstdio>
#include <vector>


class InputHandler_NSEvent : public InputHandler
{
public:
    InputHandler_NSEvent();
    ~InputHandler_NSEvent();

    void GetDevicesAndDescriptions( std::vector<InputDeviceInfo>& vDevicesOut );
};

#endif /* InputHandler_NSEvent_hpp */
