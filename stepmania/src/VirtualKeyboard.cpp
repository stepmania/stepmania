/*
 * Commands are mapped as follows:
 * left - delete
 * right - move cursor right (with space as default character)
 * up/down - change right most character to next/previous character
 */

#include "global.h"
#include "VirtualKeyboard.h"
#include "RageUtil.h"

VirtualKeyboard VIRTUALKB;

VirtualKeyboard::VirtualKeyboard()
{
	currentMode = VKMODE_PROFILE;
}

VirtualKeyboard::~VirtualKeyboard()
{
}

void VirtualKeyboard::Reset( VirtualKeyboardMode mode )
{
	currentMode = mode;
}

wchar_t VirtualKeyboard::Translate( const DeviceInput& DeviceI, const MenuInput &MenuI, const wstring &cur_string, bool *nextChar )
{
	*nextChar = false;
	MenuButton button = MenuI.button;

	switch( button )
	{
	case MENU_BUTTON_LEFT: return KEY_BACK;
	case MENU_BUTTON_START: return KEY_ENTER;
	case MENU_BUTTON_BACK: return KEY_ESC;
	}

	if(button == MENU_BUTTON_RIGHT)
	{
		*nextChar = true;
		switch( currentMode )
		{
		case VKMODE_PROFILE: return ' ';
		case VKMODE_IP: return '0';
		default: FAIL_M( ssprintf("%i", currentMode) );
		}
	}
	else if(button == MENU_BUTTON_UP)
	{
		if(cur_string.empty())
		{
			*nextChar = true;
			return 'a';
		}
		else
		{
			wchar_t c = cur_string[cur_string.size() - 1];

			if(c == ' ')
				c = 'a';
			else if(c == 'z')
				c = 'A';
			else if(c == 'Z')
				c = '0';
			else if(c == '9' && currentMode != VKMODE_IP)
				c = '!';
			else if(c == '9' && currentMode == VKMODE_IP)
				c = '.';
			else if(c == '.' && currentMode == VKMODE_IP)
				c = '0';
			else if(c == '/')
					c = ':';
			else if(c == '@')
				c = '[';
			else if(c == '`')
				c = '{';
			else if(c == '~')
				c = ' ';
			else
				c++;

			return c;
		}
	}
	else if(button == MENU_BUTTON_DOWN)
	{
		if(cur_string.empty())
		{
			*nextChar = true;
			return 'z';
		}
		else
		{
			wchar_t c = cur_string[cur_string.size() - 1];

			if(c == ' ')
				c = '~';
			else if(c == 'a')
				c = ' ';
			else if(c == 'A')
				c = 'z';
			else if(c == '0' && currentMode != VKMODE_IP)
				c = 'Z';
			else if(c == '0' && currentMode == VKMODE_IP)
				c = '.';
			else if(c == '.' && currentMode == VKMODE_IP)
				c = '9';
			else if(c == '!')
					c = '9';
			else if(c == ':')
				c = '/';
			else if(c == '[')
				c = '@';
			else if(c == '{')
				c = '`';
			else
				c--;

			return c;
		}
	}
	else
	{
		// platform specific handlers here
#if defined(XBOX)
		int dButton = DeviceI.button;

		if(dButton == JOY_1) // A
		{
			// make a space or .

			switch( currentMode )
			{
			case VKMODE_PROFILE:
			{
				if(cur_string.empty())
					*nextChar = true;
				
				return ' ';
			}
			case VKMODE_IP:
			{
				if(!cur_string.empty())
					return '.';
				else
					return 0;
			}
		}
		else if(dButton == JOY_4) // Y
		{
			// switch to next character type
			if(cur_string.empty())
				return 0;

			int c = cur_string[cur_string.size() - 1];

			if(c >= 'a' && c <= 'z')
			{
				c = toupper(c);
				return c;
			}
			else if(c >= 'A' && c <= 'Z')
			{
				return '0';
			}
			else if(c >= '0' && c <= '9' && currentMode != VKMODE_IP)
			{
				return '!';
			}
			else if((c >= '!' && c <= '/') || (c >= ':' && c <= '@') || 
				(c >= '[' && c <= '`') || (c >= '{' || c <= '~'))
			{
				return 'a';
			}
		}
		else if(button == JOY_2) // B
		{
			// switch to previous character type
			if(cur_string.empty())
				return 0;

			int c = cur_string[cur_string.size() - 1];

			if(c >= 'a' && c <= 'z')
			{
				return '!';
			}
			else if(c >= 'A' && c <= 'Z')
			{
				return 'a';
			}
			else if(c >= '0' && c <= '9' && currentMode != VKMODE_IP)
			{
				return 'A';
			}
			else if((c >= '!' && c <= '/') || (c >= ':' && c <= '@') || 
				(c >= '[' && c <= '`') || (c >= '{' || c <= '~'))
			{
				return '0';
			}
		}
#endif
		return 0;
	}
}	

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
