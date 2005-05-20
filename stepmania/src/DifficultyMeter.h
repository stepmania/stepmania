/* DifficultyMeter - A meter represention of how hard a Steps is. */

#ifndef DIFFICULTY_METER_H
#define DIFFICULTY_METER_H

#include "BitmapText.h"
#include "PlayerNumber.h"
#include "AutoActor.h"
#include "GameConstantsAndTypes.h"
#include "ActorUtil.h"
#include "Difficulty.h"
#include "ActorFrame.h"
#include "ThemeMetric.h"

class Steps;
class Trail;

template<class T>
class LunaDifficultyMeter : public LunaActorFrame<T>
{
public:
	LunaDifficultyMeter() { LUA->Register( Register ); }

	static int Load( T* p, lua_State *L )		{ p->Load( SArg(1) ); return 0; }
	static int SetFromMeterAndDifficulty( T* p, lua_State *L )		{ p->SetFromMeterAndDifficulty( IArg(1), (Difficulty)IArg(2) ); return 0; }
	static int SetFromSteps( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) )
		{
			p->SetFromSteps( NULL );
		}
		else
		{
			Steps *pS = Luna<Steps>::check(L,1);
			p->SetFromSteps( pS );
		}
		return 0;
	}
	static int SetFromTrail( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) )
		{
			p->SetFromTrail( NULL );
		}
		else
		{
			Trail *pT = Luna<Trail>::check(L,1);
			p->SetFromTrail( pT );
		}
		return 0;
	}

	static void Register(lua_State *L) 
	{
		ADD_METHOD( Load )
		ADD_METHOD( SetFromMeterAndDifficulty )
		ADD_METHOD( SetFromSteps )
		ADD_METHOD( SetFromTrail )
		LunaActor<T>::Register( L );
	}
};

class DifficultyMeter: public ActorFrame
{
public:
	DifficultyMeter();

	void Load( const CString &sType );

	void LoadFromNode( const CString& sDir, const XNode* pNode );

	void SetFromGameState( PlayerNumber pn );
	void SetFromMeterAndDifficulty( int iMeter, Difficulty dc );
	void SetFromMeterAndCourseDifficulty( int iMeter, CourseDifficulty cd );
	void SetFromSteps( const Steps* pSteps );
	void SetFromTrail( const Trail* pTrail );
	void Unset();

	// Lua
	void PushSelf( lua_State *L );

private:
	void SetInternal( int iMeter, Difficulty dc, const CString &sDifficultyCommand, const CString &sDescription );

	BitmapText		m_textFeet;		// bar
	CString			m_sCurDifficultyCommand;
	AutoActor		m_Difficulty;	/* "easy", "hard" */
	BitmapText		m_textMeter;	/* 3, 9 */
	BitmapText		m_textEditDescription;

	ThemeMetric<int>	m_iNumFeetInMeter;
	ThemeMetric<int>	m_iMaxFeetInMeter;
	ThemeMetric<int>	m_iGlowIfMeterGreaterThan;
	ThemeMetric<bool>	m_bShowFeet;
	ThemeMetric<bool>	m_bShowDifficulty;
	ThemeMetric<bool>	m_bShowMeter;
	ThemeMetric<bool>	m_bShowEditDescription;
	ThemeMetric<bool>	m_bAutoColorFeet;
	ThemeMetric<CString> m_sZeroMeterString;
};

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
