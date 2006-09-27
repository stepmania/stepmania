/* MenuInput - An input event specific to a menu navigation.  This is generated based on a Game. */

#ifndef MENU_INPUT_H
#define MENU_INPUT_H

enum MenuButton
{
	MENU_BUTTON_LEFT = 0,
	MENU_BUTTON_RIGHT,
	MENU_BUTTON_UP,
	MENU_BUTTON_DOWN,
	MENU_BUTTON_START,
	MENU_BUTTON_SELECT,
	MENU_BUTTON_BACK,
	MENU_BUTTON_COIN,
	MENU_BUTTON_OPERATOR,
	NUM_MenuButton,		// leave this at the end
	MenuButton_INVALID
};
#define FOREACH_MenuButton( m ) FOREACH_ENUM2( MenuButton, m )

const RString &MenuButtonToString( MenuButton mb );
const RString &MenuButtonToLocalizedString( MenuButton mb );

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
