/* ScreenNameEntry - Enter a name for a new high score. */

#ifndef SCREEN_NAME_ENTRY_H
#define SCREEN_NAME_ENTRY_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"
#include "ReceptorArrowRow.h"
#include "RageSound.h"

class ScreenNameEntry : public ScreenWithMenuElements
{
public:
	ScreenNameEntry();
	virtual void Init();
	virtual void Update( float fDeltaTime );
	virtual void Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual bool GenericTweenOn() const { return true; }
	virtual bool GenericTweenOff() const { return true; }

	virtual void MenuStart( const InputEventPlus &input );

private:
	class ScrollingText : public Actor
	{
	public:
		ScrollingText() : m_bDone(true) { }
		inline void SetDone() { m_bDone = true; }
		void Init( const RString &sName, const vector<float> &xs );
		virtual bool EarlyAbortDraw() const { return m_bDone; }
		virtual void DrawPrimitives();
		char GetClosestChar( float fFakeBeat ) const;

	private:
		float GetClosestCharYOffset( float fFakeBeat ) const;

		vector<float>	m_Xs;
		bool		m_bDone;
		BitmapText	m_Stamp;
		static RString	g_sNameChars;
	};
	
	static const int ABS_MAX_RANKING_NAME_LENGTH = 10;
	bool AnyStillEntering() const;

	ReceptorArrowRow	m_ReceptorArrowRow[NUM_PLAYERS];
	BitmapText		m_textSelectedChars[NUM_PLAYERS][ABS_MAX_RANKING_NAME_LENGTH];
	BitmapText		m_textCategory[NUM_PLAYERS];
	RageSound		m_soundStep;

	float			m_fFakeBeat;
	RString			m_sSelectedName[NUM_PLAYERS];
	bool			m_bStillEnteringName[NUM_PLAYERS];

	ScrollingText		m_Text[NUM_PLAYERS];
	vector<int>		m_ColToStringIndex[NUM_PLAYERS];
};

#endif

/*
 * (c) 2001-2006 Chris Danford, Steve Checkoway
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
