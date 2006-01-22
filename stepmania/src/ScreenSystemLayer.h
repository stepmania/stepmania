/* ScreenSystemLayer - credits and statistics drawn on top of everything else. */

#ifndef ScreenSystemLayer_H
#define ScreenSystemLayer_H

#include "Screen.h"
#include "BitmapText.h"
#include "Quad.h"
#include "ThemeMetric.h"
#include "AutoActor.h"
#include "LocalizedString.h"

class ScreenSystemLayer : public Screen
{
public:
	virtual void Init();
	void HandleMessage( const RString &sCommandName );

	void ReloadCreditsText();
	void Update( float fDeltaTime );

private:
	AutoActor  m_sprMessageFrame;
	BitmapText m_textMessage;
	BitmapText m_textCredits[NUM_PLAYERS];
	Quad m_quadBrightnessAdd;

	RString GetCreditsMessage( PlayerNumber pn ) const;

	LocalizedString CREDITS_PRESS_START;
	LocalizedString CREDITS_INSERT_CARD;
	LocalizedString CREDITS_CARD_TOO_LATE;
	LocalizedString CREDITS_CARD_NO_NAME;
	LocalizedString CREDITS_CARD_READY;
	LocalizedString CREDITS_CARD_CHECKING;
	LocalizedString CREDITS_CARD_REMOVED;
	LocalizedString CREDITS_FREE_PLAY;
	LocalizedString CREDITS_CREDITS;
	LocalizedString CREDITS_NOT_PRESENT;
	LocalizedString CREDITS_LOAD_FAILED;
	LocalizedString CREDITS_LOADED_FROM_LAST_GOOD_APPEND;
	ThemeMetric<bool> CREDITS_JOIN_ONLY;
};


#endif

/*
 * (c) 2001-2005 Chris Danford, Glenn Maynard
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
