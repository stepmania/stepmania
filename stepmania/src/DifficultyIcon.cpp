#include "global.h"
#include "DifficultyIcon.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "Steps.h"
#include "GameState.h"
#include "RageDisplay.h"
#include "arch/Dialog/Dialog.h"
#include "Trail.h"
#include "ActorUtil.h"
#include "XmlFile.h"

REGISTER_ACTOR_CLASS(DifficultyIcon)

DifficultyIcon::DifficultyIcon()
{
	m_bBlank = false;
}

bool DifficultyIcon::Load( CString sPath )
{
	Sprite::Load( sPath );
	int iStates = GetNumStates();
	bool bWarn = iStates != NUM_Difficulty  &&  iStates != NUM_Difficulty*2;
	if( sPath.find("_blank") != string::npos )
		bWarn = false;
	if( bWarn )
	{
		CString sError = ssprintf(
			"The difficulty icon graphic '%s' must have %d or %d frames.  It has %d states.", 
			sPath.c_str(), 
			NUM_Difficulty,
			NUM_Difficulty*2,
			iStates );
		Dialog::OK( sError );
	}
	StopAnimating();
	return true;
}

void DifficultyIcon::LoadFromNode( const CString& sDir, const XNode* pNode )
{
	CString sFile;
	if( !pNode->GetAttrValue( "File", sFile ) )
		RageException::Throw( "MeterDisplay in " + sDir + " missing File attribute" );

	sFile = sDir + sFile;
	ActorUtil::ResolvePath( sFile, sDir );

	Load( sFile );

	// skip Sprite::LoadFromNode
	Actor::LoadFromNode( sDir, pNode );
}

void DifficultyIcon::SetFromSteps( PlayerNumber pn, const Steps* pSteps )
{
	if( pSteps == NULL )
		m_bBlank = true;
	else
		SetFromDifficulty( pn, pSteps->GetDifficulty() );
}

void DifficultyIcon::SetFromTrail( PlayerNumber pn, const Trail* pTrail )
{
	if( pTrail == NULL )
		m_bBlank = true;
	else
		SetFromDifficulty( pn, pTrail->m_CourseDifficulty );
}

void DifficultyIcon::SetFromDifficulty( PlayerNumber pn, Difficulty dc )
{
	m_bBlank = false;
	switch( GetNumStates() )
	{
	case NUM_Difficulty:		SetState( dc );			break;
	case NUM_Difficulty*2:	SetState( dc*2+pn );	break;
	default:					m_bBlank = true;		break;
	}	
}

// lua start
#include "LuaBinding.h"

class LunaDifficultyIcon: public Luna<DifficultyIcon>
{
public:
	LunaDifficultyIcon() { LUA->Register( Register ); }

	static int SetFromSteps( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) )
		{
			p->SetFromSteps( PLAYER_1, NULL );
		}
		else
		{
			Steps *pS = Luna<Steps>::check(L,1);
			p->SetFromSteps( PLAYER_1, pS );
		}
		return 0;
	}
	static int SetFromTrail( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) )
		{
			p->SetFromTrail( PLAYER_1, NULL );
		}
		else
		{
			Trail *pT = Luna<Trail>::check(L,1);
			p->SetFromTrail( PLAYER_1, pT );
		}
		return 0;
	}
	static int SetFromDifficulty( T* p, lua_State *L )		{ p->SetFromDifficulty( PLAYER_1, (Difficulty)IArg(1) ); return 0; }

	static void Register(lua_State *L) 
	{
		ADD_METHOD( SetFromSteps );
		ADD_METHOD( SetFromTrail );
		ADD_METHOD( SetFromDifficulty );

		Luna<T>::Register( L );
	}
};

LUA_REGISTER_DERIVED_CLASS( DifficultyIcon, Sprite )
// lua end

/*
 * (c) 2001-2004 Chris Danford
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
