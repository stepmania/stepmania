#include "global.h"
#include "DifficultyDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "Course.h"
#include "SongManager.h"
#include "ActorUtil.h"
#include "Style.h"
#include "XmlFile.h"
#include "LuaBinding.h"
#include "GameManager.h"

REGISTER_ACTOR_CLASS( DifficultyDisplay );


DifficultyDisplay::DifficultyDisplay()
{
}

/* sID experiment:
 *
 * Names of an actor, "Foo":
 * [Foo]
 * Metric=abc
 *
 * [ScreenSomething]
 * FooP1X=20
 * FooP2Y=30
 *
 * Graphics\Foo under p1
 *
 * We want to call it different things in different contexts: we may only want one
 * set of internal metrics for a given use, but separate metrics for each player at
 * the screen level, and we may or may not want separate names at the asset level.
 *
 * As is, we tend to end up having to either duplicate [Foo] to [FooP1] and [FooP2]
 * or not use m_sName for [Foo], which limits its use.  Let's try using a separate
 * name for internal metrics.  I'm not sure if this will cause more confusion than good,
 * so I'm trying it first in only this object.
 */

void DifficultyDisplay::Load( const RString &sType )
{
	/* We can't use global ThemeMetric<RString>s, because we can have multiple
	 * DifficultyDisplays on screen at once, with different names. */
	m_iNumTicks.Load(sType,"NumTicks");
	m_iMaxTicks.Load(sType,"MaxTicks");
	m_bShowTicks.Load(sType,"ShowTicks");
	m_bShowMeter.Load(sType,"ShowMeter");
	m_bShowDescription.Load(sType,"ShowDescription");
	m_sZeroMeterString.Load(sType,"ZeroMeterString");


	m_sprFrame.Load( THEME->GetPathG(sType,"frame") );
	m_sprFrame->SetName( "Frame" );
	ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_sprFrame, sType );
	this->AddChild( m_sprFrame );

	if( m_bShowTicks )
	{
		RString sChars = "10";	// on, off
		m_textTicks.SetName( "Ticks" );
		m_textTicks.LoadFromTextureAndChars( THEME->GetPathF(sType,"ticks"), sChars );
		ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_textTicks, sType );
		this->AddChild( &m_textTicks );
	}

	if( m_bShowMeter )
	{
		m_textMeter.SetName( "Meter" );
		m_textMeter.LoadFromFont( THEME->GetPathF(sType,"meter") );
		ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_textMeter, sType );
		this->AddChild( &m_textMeter );

		// These commands should have been loaded by SetXYAndOnCommand above.
		ASSERT( m_textMeter.HasCommand("Set") );
	}
	
	if( m_bShowDescription )
	{
		m_textDescription.SetName( "Description" );
		m_textDescription.LoadFromFont( THEME->GetPathF(sType,"Description") );
		ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_textDescription, sType );
		this->AddChild( &m_textDescription );
	}
	
	m_sprAutogen.Load( THEME->GetPathG(sType,"Autogen") );
	m_sprAutogen->SetName( "Autogen" );
	ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_sprAutogen, sType );
	this->AddChild( m_sprAutogen );
	

	Unset();
}

void DifficultyDisplay::LoadFromNode( const XNode* pNode )
{
	ActorFrame::LoadFromNode( pNode );

	RString s;
	if( !pNode->GetAttrValue("Type", s) )
		RageException::Throw( "%s: DifficultyDisplay: missing the \"Type\" attribute", ActorUtil::GetWhere(pNode).c_str() );
	Load( s );
}

void DifficultyDisplay::SetFromGameState( PlayerNumber pn )
{
	if( GAMESTATE->IsCourseMode() )
	{
		const Trail* pTrail = GAMESTATE->m_pCurTrail[pn];
		if( pTrail )
			SetFromTrail( pTrail );
		else
			SetFromStepsTypeAndMeterAndCourseDifficulty( StepsType_Invalid, 0, GAMESTATE->m_PreferredCourseDifficulty[pn] );
	}
	else
	{
		const Steps* pSteps = GAMESTATE->m_pCurSteps[pn];
		if( pSteps )
			SetFromSteps( pSteps );
		else
			SetFromStepsTypeAndMeterAndDifficulty( StepsType_Invalid, 0, GAMESTATE->m_PreferredDifficulty[pn] );
	}
}

void DifficultyDisplay::SetFromSteps( const Steps* pSteps )
{
	if( pSteps == NULL )
	{
		Unset();
		return;
	}

	SetParams params = { pSteps, NULL, pSteps->GetMeter(), pSteps->m_StepsType, pSteps->GetDifficulty(), false, pSteps->GetDescription() };
	SetInternal( params );
}

void DifficultyDisplay::SetFromTrail( const Trail* pTrail )
{
	if( pTrail == NULL )
	{
		Unset();
		return;
	}

	SetParams params = { NULL, pTrail, pTrail->GetMeter(), pTrail->m_StepsType, pTrail->m_CourseDifficulty, true, RString() };
	SetInternal( params );
}

void DifficultyDisplay::Unset()
{
	SetParams params = { NULL, NULL, 0, StepsType_Invalid, Difficulty_Invalid, false, RString() };
	SetInternal( params );
}

void DifficultyDisplay::SetFromStepsTypeAndMeterAndDifficulty( StepsType st, int iMeter, Difficulty dc )
{
	SetParams params = { NULL, NULL, iMeter, st, dc, false, RString() };
	SetInternal( params );
}

void DifficultyDisplay::SetFromStepsTypeAndMeterAndCourseDifficulty( StepsType st, int iMeter, CourseDifficulty cd )
{
	SetParams params = { NULL, NULL, 0, st, cd, true, RString() };
	SetInternal( params );
}

void DifficultyDisplay::SetInternal( const SetParams &params )
{
	Message msg( "Set" );
	if( params.pSteps )
		msg.SetParam( "Steps", LuaReference::CreateFromPush(*(Steps*)params.pSteps) );
	if( params.pTrail )
		msg.SetParam( "Trail", LuaReference::CreateFromPush(*(Trail*)params.pTrail) );
	msg.SetParam( "Meter", params.iMeter );
	msg.SetParam( "StepsType", params.st );
	msg.SetParam( "Difficulty", params.dc );
	msg.SetParam( "IsCourseDifficulty", params.bIsCourseDifficulty );
	msg.SetParam( "Description", params.sDescription );

	m_sprFrame->HandleMessage( msg );

	if( m_bShowTicks )
	{
		char on = char('1');
		char off = '0';

		RString sNewText;
		int iNumOn = min( (int)m_iMaxTicks, params.iMeter );
		sNewText.insert( sNewText.end(), iNumOn, on );
		int iNumOff = max( 0, m_iNumTicks-iNumOn );
		sNewText.insert( sNewText.end(), iNumOff, off );
		m_textTicks.SetText( sNewText );
	}

	if( m_bShowMeter )
	{
		if( params.iMeter == 0 )	// Unset calls with this
		{
			m_textMeter.SetText( m_sZeroMeterString );
		}
		else
		{
			const RString sMeter = ssprintf( "%i", params.iMeter );
			m_textMeter.SetText( sMeter );
		}
	}

	if( m_bShowDescription )
	{
		if( params.dc == Difficulty_Edit )
			m_textDescription.SetText( params.sDescription );
		else if( params.bIsCourseDifficulty )
			m_textDescription.SetText( CourseDifficultyToLocalizedString(params.dc) );
		else
			m_textDescription.SetText( DifficultyToLocalizedString(params.dc) );
	}
	
	this->HandleMessage( msg );
}

// lua start
#include "LuaBinding.h"

class LunaDifficultyDisplay: public Luna<DifficultyDisplay>
{
public:
	static int Load( T* p, lua_State *L )		{ p->Load( SArg(1) ); return 0; }
	static int SetFromStepsTypeAndMeterAndDifficulty( T* p, lua_State *L )		{ p->SetFromStepsTypeAndMeterAndDifficulty( Enum::Check<StepsType>(L, 1), IArg(2), Enum::Check<Difficulty>(L, 3) ); return 0; }
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
	static int SetFromGameState( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		p->SetFromGameState( pn );
		return 0;
	}

	LunaDifficultyDisplay()
	{
		ADD_METHOD( Load );
		ADD_METHOD( SetFromStepsTypeAndMeterAndDifficulty );
		ADD_METHOD( SetFromSteps );
		ADD_METHOD( SetFromTrail );
		ADD_METHOD( SetFromGameState );
	}
};

LUA_REGISTER_DERIVED_CLASS( DifficultyDisplay, ActorFrame )
// lua end

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
