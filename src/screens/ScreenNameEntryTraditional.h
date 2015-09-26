#ifndef SCREEN_NAME_ENTRY_TRADITIONAL_H
#define SCREEN_NAME_ENTRY_TRADITIONAL_H

#include "ScreenWithMenuElements.h"
#include "ThemeMetric.h"
#include "LocalizedString.h"
#include "InputQueue.h"
/** @brief Enter a name for a new high score. */
class ScreenNameEntryTraditional: public ScreenWithMenuElements
{
public:
	virtual void Init();
	virtual void BeginScreen();

	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual bool Input( const InputEventPlus &input );

	bool EnterKey( PlayerNumber pn, wchar_t sLetter );
	bool Backspace( PlayerNumber pn );

	// Lua
	void PushSelf( lua_State *L );

	bool AnyStillEntering() const;
	bool AnyEntering() const;
	bool Finish( PlayerNumber pn );
	void UpdateSelectionText( PlayerNumber pn );
	void SelectChar( PlayerNumber pn, const RString &sKey );
	/** @brief How long can the name be for ranking purposes? */
	ThemeMetric<int>	MAX_RANKING_NAME_LENGTH;

	wstring			m_sSelection[NUM_PLAYERS];
	bool			m_bEnteringName[NUM_PLAYERS];
	bool			m_bFinalized[NUM_PLAYERS];
};

#endif

/**
 * @file
 * @author Glenn Maynard, Chris Danford (c) 2001-2007
 * @section LICENSE
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
