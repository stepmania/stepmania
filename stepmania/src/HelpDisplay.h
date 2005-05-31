/* HelpDisplay - A BitmapText that cycles through messages. */

#ifndef HELP_DISPLAY_H
#define HELP_DISPLAY_H

#include "BitmapText.h"
#include "ThemeMetric.h"
#include "MessageManager.h"

struct lua_State;

class HelpDisplay : public BitmapText
{
public:
	HelpDisplay();
	void Load( const CString &sType );

	void LoadFromNode( const CString& sDir, const XNode* pNode );

	void SetTips( const CStringArray &arrayTips ) { SetTips( arrayTips, arrayTips ); }
	void SetTips( const CStringArray &arrayTips, const CStringArray &arrayTipsAlt );
	void GetTips( CStringArray &arrayTipsOut, CStringArray &arrayTipsAltOut ) const { arrayTipsOut = m_arrayTips; arrayTipsAltOut = m_arrayTipsAlt; }

	virtual void Update( float fDeltaTime );
	virtual void SetName( const CString &sName, const CString &sID = "" );

	// Lua
	virtual void PushSelf( lua_State *L );

protected:
	ThemeMetric<float> TIP_SHOW_TIME;

	CStringArray m_arrayTips, m_arrayTipsAlt;
	int m_iCurTipIndex;
	
	float m_fSecsUntilSwitch;
};

class GenreDisplay : public HelpDisplay
{
public:
	GenreDisplay();
	~GenreDisplay();
	void PlayCommand( const CString &sCommandName );
};

#endif

/*
 * (c) 2001-2003 Chris Danford, Glenn Maynard
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
