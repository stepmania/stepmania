#include "global.h"
#include "VirtualKeyboard.h"
#include "RageInputDevice.h"

VirtualKeyboard XBOX_VKB;

VirtualKeyboard::VirtualKeyboard()
{
	currentMode = VKMODE_PROFILE;
	currentType = VKTYPE_LOWER;
}

VirtualKeyboard::~VirtualKeyboard()
{
}

void VirtualKeyboard::Reset(VirtualKeyboardMode mode)
{
	currentMode = mode;
	if(currentMode == VKMODE_PROFILE)
		currentType = VKTYPE_LOWER;
	else if(currentMode == VKMODE_IP)
		currentType = VKTYPE_NUMBER;
}

int VirtualKeyboard::Translate(int button, const wstring &cur_string, bool *nextChar)
{
	*nextChar = false;

	if(button == JOY_HAT_LEFT)
		return KEY_BACK;
	else if(button == JOY_HAT_RIGHT)
	{
		*nextChar = true;
		if(currentType == VKTYPE_LOWER || currentType == VKTYPE_UPPER)
			return ' ';
		else if(currentType == VKTYPE_NUMBER)
			return '0';
		else if(currentType == VKTYPE_SYMBOL)
			return '!';

		return 0;
	}
	else if(button == JOY_HAT_UP)
	{
		if(currentType == VKTYPE_LOWER)
		{
			if(cur_string.empty())
			{
				*nextChar = true;
				return 'a';
			}
			else
			{
				int c = cur_string[cur_string.size() - 1];

				if(c == ' ')
					c = 'a';
				else if(c == 'z')
					c = ' ';
				else
					c++;

				return c;
			}
		}
		else if(currentType == VKTYPE_UPPER)
		{
			if(cur_string.empty())
			{
				*nextChar = true;
				return 'A';				
			}
			else
			{
				int c = cur_string[cur_string.size() - 1];

				if(c == ' ')
					c = 'A';
				else if(c == 'Z')
					c = ' ';
				else
					c++;

				return c;
			}
		}
		else if(currentType == VKTYPE_NUMBER)
		{
			if(cur_string.empty())
			{
				*nextChar = true;
				return '0';
			}
			else
			{
				int c = cur_string[cur_string.size() - 1];
				if(c == '9' || c == '.')
					c = '0';
				else
					c++;

				return c;
			}
		}
		else if(currentType == VKTYPE_SYMBOL)
		{
			if(cur_string.empty())
			{
				*nextChar = true;
				return '!';
			}
			else
			{
				int c = cur_string[cur_string.size() - 1];

				if(c == '/')
					c = ':';
				else if(c == '@')
					c = '[';
				else if(c == '`')
					c = '{';
				else if(c == '~')
					c = '!';
				else
					c++;

				return c;
			}
		}
		
		return 0;
	}
	else if(button == JOY_HAT_DOWN)
	{
		if(currentType == VKTYPE_LOWER)
		{
			if(cur_string.empty())
			{
				*nextChar = true;
				return 'z';
			}
			else
			{
				int c = cur_string[cur_string.size() - 1];

				if(c == ' ')
					c = 'z';
				else if(c == 'a')
					c = ' ';
				else
					c--;

				return c;
			}
		}
		else if(currentType == VKTYPE_UPPER)
		{
			if(cur_string.empty())
			{
				*nextChar = true;
				return 'Z';
			}
			else
			{
				int c = cur_string[cur_string.size() - 1];

				if(c == ' ')
					c = 'Z';
				else if(c == 'A')
					c = ' ';
				else
					c--;

				return c;
			}
		}
		else if(currentType == VKTYPE_NUMBER)
		{
			if(cur_string.empty())
			{
				*nextChar = true;
				return '9';
			}
			else
			{
				int c = cur_string[cur_string.size() - 1];

				if(c == '0' || c == '.')
					c = '9';
				else
					c--;

				return c;
			}
		}
		else if(currentType == VKTYPE_SYMBOL)
		{
			if(cur_string.empty())
			{
				*nextChar = true;
				return '~';
			}
			else
			{
				int c = cur_string[cur_string.size() - 1];

				if(c == ':')
					c = '/';
				else if(c == '[')
					c = '@';
				else if(c == '{')
					c = '`';
				else if(c == '!')
					c = '~';
				else 
					c--;

				return c;
			}
		}
		
		return 0;
	}
	else if(button == JOY_AUX_1)
		return KEY_ENTER;
	else if(button == JOY_AUX_2)
		return KEY_ESC;
	else if(button == JOY_1) // A
	{
		if(currentMode != VKMODE_IP && (currentType == VKTYPE_LOWER || currentType == VKTYPE_UPPER))
		{
			if(cur_string.empty())
				*nextChar = true;
			
			return ' ';
		}
		else if(currentMode == VKMODE_IP)
		{
			if(!cur_string.empty())
				return '.';
		}
	}
	else if(button == JOY_4) // Y
	{
		if(currentType == VKTYPE_LOWER)
		{
			currentType = VKTYPE_UPPER;

			if(cur_string.empty())
			{
				*nextChar = true;
				return 'A';
			}
			else
			{
				int c = cur_string[cur_string.size() - 1];
				
				c = toupper(c);
				return c;
			}
		}
		else if(currentType == VKTYPE_UPPER)
		{
			currentType = VKTYPE_NUMBER;

			if(cur_string.empty())
				*nextChar = true;
			
			return '0';
		}
		else if(currentType == VKTYPE_NUMBER && currentMode != VKMODE_IP)
		{
			currentType = VKTYPE_SYMBOL;

			if(cur_string.empty())
				*nextChar = true;

			return '!';
		}
		else if(currentType == VKTYPE_SYMBOL)
		{
			currentType = VKTYPE_LOWER;

			if(cur_string.empty())
				*nextChar = true;
				
			return 'a';
		}
	}
	else if(button == JOY_2) // B
	{
		if(currentType == VKTYPE_UPPER)
		{
			currentType = VKTYPE_LOWER;

			if(cur_string.empty())
			{
                *nextChar = true;
				return 'a';
			}
			else
			{
				int c = cur_string[cur_string.size() - 1];

				c = tolower(c);
				return c;
			}
		}
		else if(currentType == VKTYPE_SYMBOL)
		{
			currentType = VKTYPE_NUMBER;
			
			if(cur_string.empty())
				*nextChar = true;
				
			return '0';
		}
		else if(currentType == VKTYPE_LOWER)
		{
			currentType = VKTYPE_SYMBOL;

			if(cur_string.empty())
                *nextChar = true;

			return '!';
		}
		else if(currentType == VKTYPE_NUMBER && currentMode != VKMODE_IP)
		{
			currentType = VKTYPE_UPPER;

			if(cur_string.empty())
				*nextChar = true;
				
			return 'A';
		}
	}
	
	return 0;
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