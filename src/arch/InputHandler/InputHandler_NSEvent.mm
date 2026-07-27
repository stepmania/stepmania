//
//  InputHandler_NSEvent.cpp
//  StepMania
//
//  Created by heshuimu on 12/22/19.
//

#include "InputHandler_NSEvent.hpp"

#include <vector>

#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>


DeviceButton NSKeyCodeMap[0x100];

float DownOrUp(bool down)
{
    return down ? 1 : 0;
}

void InitKeyCodeMap()
{
    NSKeyCodeMap[kVK_ANSI_A] = KEY_Ca;
    NSKeyCodeMap[kVK_ANSI_B] = KEY_Cb;
    NSKeyCodeMap[kVK_ANSI_C] = KEY_Cc;
    NSKeyCodeMap[kVK_ANSI_D] = KEY_Cd;
    NSKeyCodeMap[kVK_ANSI_E] = KEY_Ce;
    NSKeyCodeMap[kVK_ANSI_F] = KEY_Cf;
    NSKeyCodeMap[kVK_ANSI_G] = KEY_Cg;
    NSKeyCodeMap[kVK_ANSI_H] = KEY_Ch;
    NSKeyCodeMap[kVK_ANSI_I] = KEY_Ci;
    NSKeyCodeMap[kVK_ANSI_J] = KEY_Cj;
    NSKeyCodeMap[kVK_ANSI_K] = KEY_Ck;
    NSKeyCodeMap[kVK_ANSI_L] = KEY_Cl;
    NSKeyCodeMap[kVK_ANSI_M] = KEY_Cm;
    NSKeyCodeMap[kVK_ANSI_N] = KEY_Cn;
    NSKeyCodeMap[kVK_ANSI_O] = KEY_Co;
    NSKeyCodeMap[kVK_ANSI_P] = KEY_Cp;
    NSKeyCodeMap[kVK_ANSI_Q] = KEY_Cq;
    NSKeyCodeMap[kVK_ANSI_R] = KEY_Cr;
    NSKeyCodeMap[kVK_ANSI_S] = KEY_Cs;
    NSKeyCodeMap[kVK_ANSI_T] = KEY_Ct;
    NSKeyCodeMap[kVK_ANSI_U] = KEY_Cu;
    NSKeyCodeMap[kVK_ANSI_V] = KEY_Cv;
    NSKeyCodeMap[kVK_ANSI_W] = KEY_Cw;
    NSKeyCodeMap[kVK_ANSI_X] = KEY_Cx;
    NSKeyCodeMap[kVK_ANSI_Y] = KEY_Cy;
    NSKeyCodeMap[kVK_ANSI_Z] = KEY_Cz;

    NSKeyCodeMap[kVK_ANSI_0] = KEY_C0;
    NSKeyCodeMap[kVK_ANSI_1] = KEY_C1;
    NSKeyCodeMap[kVK_ANSI_2] = KEY_C2;
    NSKeyCodeMap[kVK_ANSI_3] = KEY_C3;
    NSKeyCodeMap[kVK_ANSI_4] = KEY_C4;
    NSKeyCodeMap[kVK_ANSI_5] = KEY_C5;
    NSKeyCodeMap[kVK_ANSI_6] = KEY_C6;
    NSKeyCodeMap[kVK_ANSI_7] = KEY_C7;
    NSKeyCodeMap[kVK_ANSI_8] = KEY_C8;
    NSKeyCodeMap[kVK_ANSI_9] = KEY_C9;

    NSKeyCodeMap[kVK_ANSI_Keypad0] = KEY_KP_C0;
    NSKeyCodeMap[kVK_ANSI_Keypad1] = KEY_KP_C1;
    NSKeyCodeMap[kVK_ANSI_Keypad2] = KEY_KP_C2;
    NSKeyCodeMap[kVK_ANSI_Keypad3] = KEY_KP_C3;
    NSKeyCodeMap[kVK_ANSI_Keypad4] = KEY_KP_C4;
    NSKeyCodeMap[kVK_ANSI_Keypad5] = KEY_KP_C5;
    NSKeyCodeMap[kVK_ANSI_Keypad6] = KEY_KP_C6;
    NSKeyCodeMap[kVK_ANSI_Keypad7] = KEY_KP_C7;
    NSKeyCodeMap[kVK_ANSI_Keypad8] = KEY_KP_C8;
    NSKeyCodeMap[kVK_ANSI_Keypad9] = KEY_KP_C9;

    NSKeyCodeMap[kVK_ANSI_KeypadDivide] = KEY_KP_SLASH;
    NSKeyCodeMap[kVK_ANSI_KeypadMultiply] = KEY_KP_ASTERISK;
    NSKeyCodeMap[kVK_ANSI_KeypadEnter] = KEY_KP_ENTER;
    NSKeyCodeMap[kVK_ANSI_KeypadEquals] = KEY_KP_EQUAL;
    NSKeyCodeMap[kVK_ANSI_KeypadMinus] = KEY_KP_HYPHEN;
    NSKeyCodeMap[kVK_ANSI_KeypadPlus] = KEY_KP_PLUS;
    NSKeyCodeMap[kVK_ANSI_KeypadDecimal] = KEY_KP_PERIOD;

    NSKeyCodeMap[kVK_Home] = KEY_HOME;
    NSKeyCodeMap[kVK_End] = KEY_END;
    NSKeyCodeMap[kVK_PageDown] = KEY_PGDN;
    NSKeyCodeMap[kVK_PageUp] = KEY_PGUP;

    NSKeyCodeMap[kVK_F1] = KEY_F1;
    NSKeyCodeMap[kVK_F2] = KEY_F2;
    NSKeyCodeMap[kVK_F3] = KEY_F3;
    NSKeyCodeMap[kVK_F4] = KEY_F4;
    NSKeyCodeMap[kVK_F5] = KEY_F5;
    NSKeyCodeMap[kVK_F6] = KEY_F6;
    NSKeyCodeMap[kVK_F7] = KEY_F7;
    NSKeyCodeMap[kVK_F8] = KEY_F8;
    NSKeyCodeMap[kVK_F9] = KEY_F9;
    NSKeyCodeMap[kVK_F10] = KEY_F10;
    NSKeyCodeMap[kVK_F11] = KEY_F11;
    NSKeyCodeMap[kVK_F12] = KEY_F12;
    NSKeyCodeMap[kVK_F13] = KEY_F13;
    NSKeyCodeMap[kVK_F14] = KEY_F14;
    NSKeyCodeMap[kVK_F15] = KEY_F15;
    NSKeyCodeMap[kVK_F16] = KEY_F16;

    NSKeyCodeMap[kVK_ANSI_Quote] = KEY_QUOTE;
    NSKeyCodeMap[kVK_ANSI_Backslash] = KEY_BACKSLASH;
    NSKeyCodeMap[kVK_CapsLock] = KEY_CAPSLOCK;
    NSKeyCodeMap[kVK_ANSI_Comma] = KEY_COMMA;
    NSKeyCodeMap[kVK_ForwardDelete] = KEY_DEL;
    NSKeyCodeMap[kVK_Delete] = KEY_BACK;
    NSKeyCodeMap[kVK_ANSI_Equal] = KEY_EQUAL;

    NSKeyCodeMap[kVK_Escape] = KEY_ESC;
    NSKeyCodeMap[kVK_ANSI_LeftBracket] = KEY_LBRACKET;
    NSKeyCodeMap[kVK_ANSI_Minus] = KEY_HYPHEN;
    NSKeyCodeMap[kVK_ANSI_Period] = KEY_PERIOD;
    NSKeyCodeMap[kVK_Return] = KEY_ENTER;
    NSKeyCodeMap[kVK_ANSI_RightBracket] = KEY_RBRACKET;
    NSKeyCodeMap[kVK_ANSI_Semicolon] = KEY_SEMICOLON;
    NSKeyCodeMap[kVK_Space] = KEY_SPACE;
    NSKeyCodeMap[kVK_Tab] = KEY_TAB;

    NSKeyCodeMap[kVK_Command] = KEY_LMETA;
    NSKeyCodeMap[kVK_RightCommand] = KEY_RMETA;
    NSKeyCodeMap[kVK_Control] = KEY_LCTRL;
    NSKeyCodeMap[kVK_RightControl] = KEY_RCTRL;
    NSKeyCodeMap[kVK_Function] = KEY_FN;
    NSKeyCodeMap[kVK_Option] = KEY_LALT;
    NSKeyCodeMap[kVK_RightOption] = KEY_RALT;
    NSKeyCodeMap[kVK_Shift] = KEY_LSHIFT;
    NSKeyCodeMap[kVK_RightShift] = KEY_RSHIFT;

    NSKeyCodeMap[kVK_DownArrow] = KEY_DOWN;
    NSKeyCodeMap[kVK_LeftArrow] = KEY_LEFT;
    NSKeyCodeMap[kVK_RightArrow] = KEY_RIGHT;
    NSKeyCodeMap[kVK_UpArrow] = KEY_UP;
}

//static let backApostrophe = UInt16(kVK_ANSI_Grave)
//static let keypadClear = UInt16(kVK_ANSI_KeypadClear)
//static let mute = UInt16(kVK_Mute)
//static let volumeDown = UInt16(kVK_VolumeDown)
//static let volumeUp = UInt16(kVK_VolumeUp)
//
//static let downArrow = UInt16(kVK_DownArrow)
//static let leftArrow = UInt16(kVK_LeftArrow)
//static let rightArrow = UInt16(kVK_RightArrow)
//static let upArrow = UInt16(kVK_UpArrow)

InputHandler_NSEvent::InputHandler_NSEvent()
{
    InitKeyCodeMap();

    [NSEvent addLocalMonitorForEventsMatchingMask:(NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged) handler:^(NSEvent* e) {
        unsigned short keyCode = [e keyCode];

        if([e type] == NSEventTypeKeyUp)
        {
            ButtonPressed(DeviceInput(DEVICE_KEYBOARD, NSKeyCodeMap[keyCode], 1));
            ButtonPressed(DeviceInput(DEVICE_KEYBOARD, NSKeyCodeMap[keyCode], 0));
        }
        else if([e type] == NSEventTypeFlagsChanged)
        {
            NSEventModifierFlags flags = [e modifierFlags];
            switch(keyCode)
            {
            case kVK_CapsLock:
                ButtonPressed(DeviceInput(DEVICE_KEYBOARD, NSKeyCodeMap[keyCode], DownOrUp(flags & NSEventModifierFlagCapsLock)));
                break;
            case kVK_Shift:
            case kVK_RightShift:
                ButtonPressed(DeviceInput(DEVICE_KEYBOARD, NSKeyCodeMap[keyCode], DownOrUp(flags & NSEventModifierFlagShift)));
                break;
            case kVK_Control:
            case kVK_RightControl:
                ButtonPressed(DeviceInput(DEVICE_KEYBOARD, NSKeyCodeMap[keyCode], DownOrUp(flags & NSEventModifierFlagControl)));
                break;
            case kVK_Option:
            case kVK_RightOption:
                ButtonPressed(DeviceInput(DEVICE_KEYBOARD, NSKeyCodeMap[keyCode], DownOrUp(flags & NSEventModifierFlagOption)));
                break;
            case kVK_Command:
            case kVK_RightCommand:
                ButtonPressed(DeviceInput(DEVICE_KEYBOARD, NSKeyCodeMap[keyCode], DownOrUp(flags & NSEventModifierFlagCommand)));
                break;
            case kVK_Function:
                ButtonPressed(DeviceInput(DEVICE_KEYBOARD, NSKeyCodeMap[keyCode], DownOrUp(flags & NSEventModifierFlagFunction)));
                break;
            }
        }

        return e;
    }];
}

InputHandler_NSEvent::~InputHandler_NSEvent()
{
}

void InputHandler_NSEvent::GetDevicesAndDescriptions( std::vector<InputDeviceInfo>& vDevicesOut )
{
    vDevicesOut.push_back(InputDeviceInfo(DEVICE_KEYBOARD, "NSEventKeyboard"));
}
