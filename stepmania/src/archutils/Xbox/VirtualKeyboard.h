/* Allows an Xbox user to input profile names or IP addresses using a keypad
 * Commands are mapped as follows:
 * D-PAD left - delete
 * D-PAD right - move cursor right (with space as default character)
 * D-PAD up/down - next/previous character
 * Y - change characters to lower-case/upper-case/numbers/symbols
 * A - make current character a space
 */

#if !defined(VIRTUAL_KEYBOARD_H)
#define VIRTUAL_KEYBOARD_H

#pragma once

enum VirtualKeyboardMode { VKMODE_PROFILE, VKMODE_IP };
enum VirtualKeyboardType { VKTYPE_LOWER, VKTYPE_UPPER, VKTYPE_NUMBER, VKTYPE_SYMBOL };

class VirtualKeyboard
{
public:
	VirtualKeyboard();
	~VirtualKeyboard();

	void Reset(VirtualKeyboardMode mode);
	int Translate(int button, const wstring &cur_string, bool *nextChar);

protected:
	VirtualKeyboardMode currentMode;
	VirtualKeyboardType currentType;
};

extern VirtualKeyboard XBOX_VKB;

#endif

/*
 * (c) 2004 Ryan Dortmans
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
