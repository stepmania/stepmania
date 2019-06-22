#include "global.h"
#include "StepsDisplay.h"
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
#include "PlayerState.h"
#include "RageLog.h"

REGISTER_ACTOR_CLASS( StepsDisplay );


StepsDisplay::StepsDisplay()
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

void StepsDisplay::Load( const RString &sMetricsGroup, const PlayerState *pPlayerState )
{
	m_sMetricsGroup = sMetricsGroup;

	/* We can't use global ThemeMetric<RString>s, because we can have multiple
	 * StepsDisplays on screen at once, with different names. */
	m_iNumTicks.Load(m_sMetricsGroup,"NumTicks");
	m_iMaxTicks.Load(m_sMetricsGroup,"MaxTicks");
	m_bShowTicks.Load(m_sMetricsGroup,"ShowTicks");
	m_bShowMeter.Load(m_sMetricsGroup,"ShowMeter");
	m_bShowDescription.Load(m_sMetricsGroup,"ShowDescription");
	m_bShowCredit.Load(m_sMetricsGroup,"ShowCredit");
	m_bShowAutogen.Load(m_sMetricsGroup,"ShowAutogen");
	m_bShowStepsType.Load(m_sMetricsGroup,"ShowStepsType");
	m_sZeroMeterString.Load(m_sMetricsGroup,"ZeroMeterString");
	m_sMeterFormatString.Load(m_sMetricsGroup,"MeterFormatString");

	m_sprFrame.Load( THEME->GetPathG(m_sMetricsGroup,"frame") );
	m_sprFrame->SetName( "Frame" );
	ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_sprFrame, m_sMetricsGroup );
	this->AddChild( m_sprFrame );

	if( m_bShowTicks )
	{
		RString sChars = "10"; // on, off (todo: make this metricable -aj)
		m_textTicks.SetName( "Ticks" );
		m_textTicks.LoadFromTextureAndChars( THEME->GetPathF(m_sMetricsGroup,"ticks"), sChars );
		ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_textTicks, m_sMetricsGroup );
		this->AddChild( &m_textTicks );
	}

	if( m_bShowMeter )
	{
		m_textMeter.SetName( "Meter" );
		m_textMeter.LoadFromFont( THEME->GetPathF(m_sMetricsGroup,"meter") );
		ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_textMeter, m_sMetricsGroup );
		this->AddChild( &m_textMeter );

		// These commands should have been loaded by SetXYAndOnCommand above.
		ASSERT( m_textMeter.HasCommand("Set") );
	}

	if( m_bShowDescription )
	{
		m_textDescription.SetName( "Description" );
		m_textDescription.LoadFromFont( THEME->GetPathF(m_sMetricsGroup,"Description") );
		ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_textDescription, m_sMetricsGroup );
		this->AddChild( &m_textDescription );
	}
	if( m_bShowCredit )
	{
		m_textAuthor.SetName( "Step Author" );
		m_textAuthor.LoadFromFont( THEME->GetPathF(m_sMetricsGroup,"Credit") );
		ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_textAuthor, m_sMetricsGroup );
		this->AddChild( &m_textAuthor );
	}

	if( m_bShowAutogen )
	{
		m_sprAutogen.Load( THEME->GetPathG(m_sMetricsGroup,"Autogen") );
		m_sprAutogen->SetName( "Autogen" );
		ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_sprAutogen, m_sMetricsGroup );
		this->AddChild( m_sprAutogen );
	}

	if( m_bShowStepsType )
	{
		m_sprStepsType.Load( THEME->GetPathG(m_sMetricsGroup,"StepsType") );
		m_sprStepsType->SetName( "StepsType" );
		ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_sprStepsType, m_sMetricsGroup );
		this->AddChild( m_sprStepsType );
	}

	// Play Load Command
	PlayerState* pPlayerState_ = const_cast<PlayerState*>(pPlayerState);
	Message msg("Load");
	if( pPlayerState_ )
		msg.SetParam( "PlayerState", LuaReference::CreateFromPush(*pPlayerState_) );
	this->HandleMessage( msg );

	Unset();
}

void StepsDisplay::SetFromGameState( PlayerNumber pn )
{
	if( GAMESTATE->IsCourseMode() )
	{
		// figure out what course type is selected somehow.
		const Trail* pTrail = GAMESTATE->m_pCurTrail[pn];
		if( pTrail )
			SetFromTrail( pTrail );
		else
			SetFromStepsTypeAndMeterAndDifficultyAndCourseType( StepsType_Invalid, 0, GAMESTATE->m_PreferredCourseDifficulty[pn], CourseType_Invalid );
	}
	else
	{
		const Steps* pSteps = GAMESTATE->m_pCurSteps[pn];
		if( pSteps )
			SetFromSteps( pSteps );
		else
			SetFromStepsTypeAndMeterAndDifficultyAndCourseType( StepsType_Invalid, 0, GAMESTATE->m_PreferredDifficulty[pn], CourseType_Invalid );
	}
}

void StepsDisplay::SetFromSteps( const Steps* pSteps )
{
	if( pSteps == nullptr )
	{
		Unset();
		return;
	}

	SetParams params = { pSteps, nullptr, pSteps->GetMeter(), pSteps->m_StepsType, pSteps->GetDifficulty(), CourseType_Invalid };
	SetInternal( params );
}

void StepsDisplay::SetFromTrail( const Trail* pTrail )
{
	if( pTrail == nullptr )
	{
		Unset();
		return;
	}

	SetParams params = { nullptr, pTrail, pTrail->GetMeter(), pTrail->m_StepsType, pTrail->m_CourseDifficulty, pTrail->m_CourseType };
	SetInternal( params );
}

void StepsDisplay::Unset()
{
	this->SetVisible( false );
}

void StepsDisplay::SetFromStepsTypeAndMeterAndDifficultyAndCourseType( StepsType st, int iMeter, Difficulty dc, CourseType ct )
{
	SetParams params = { nullptr, nullptr, iMeter, st, dc, ct };
	SetInternal( params );
}

void StepsDisplay::SetInternal( const SetParams &params )
{
	this->SetVisible( true );
	Message msg( "Set" );

	RString sCustomDifficulty;
	if( params.pSteps )
		sCustomDifficulty = StepsToCustomDifficulty(params.pSteps);
	else if( params.pTrail )
		sCustomDifficulty = TrailToCustomDifficulty(params.pTrail);
	else
		sCustomDifficulty = GetCustomDifficulty( params.st, params.dc, params.ct );
	msg.SetParam( "CustomDifficulty", sCustomDifficulty );

	RString sDisplayDescription;
	if( params.pSteps  &&  params.pSteps->IsAnEdit() )
		sDisplayDescription = params.pSteps->GetDescription();
	else if( sCustomDifficulty.empty() )
		sDisplayDescription = RString();
	else
		sDisplayDescription = CustomDifficultyToLocalizedString( sCustomDifficulty );
	msg.SetParam( "DisplayDescription", sDisplayDescription );
	
	RString sDisplayCredit;
	if( params.pSteps )
		sDisplayCredit = params.pSteps->GetCredit();

	if( params.pSteps )
		msg.SetParam( "Steps", LuaReference::CreateFromPush(*(Steps*)params.pSteps) );
	if( params.pTrail )
		msg.SetParam( "Trail", LuaReference::CreateFromPush(*(Trail*)params.pTrail) );
	msg.SetParam( "Meter", params.iMeter );
	msg.SetParam( "StepsType", params.st );

	m_sprFrame->HandleMessage( msg );

	if( m_bShowTicks )
	{
		// todo: let themers handle the logic of tick text. -aj
		char on = char('1');
		char off = '0';

		RString sNewText;
		int iNumOn = min( (int)m_iMaxTicks, params.iMeter );
		sNewText.insert( sNewText.end(), iNumOn, on );
		int iNumOff = max( 0, m_iNumTicks-iNumOn );
		sNewText.insert( sNewText.end(), iNumOff, off );
		m_textTicks.SetText( sNewText );
		m_textTicks.HandleMessage( msg );
	}

	if( m_bShowMeter )
	{
		if( params.iMeter == 0 )	// Unset calls with this
		{
			m_textMeter.SetText( m_sZeroMeterString );
		}
		else
		{
			const RString sMeter = ssprintf( m_sMeterFormatString.GetValue().c_str(), params.iMeter );
			m_textMeter.SetText( sMeter );
			m_textMeter.HandleMessage( msg );
		}
	}

	if( m_bShowDescription )
	{
		m_textDescription.SetText( sDisplayDescription );
		m_textDescription.HandleMessage( msg );
	}
	if( m_bShowCredit )
	{
		m_textAuthor.SetText( sDisplayCredit );
		m_textAuthor.HandleMessage( msg );
	}

	if( m_bShowAutogen )
	{
		bool b = params.pSteps && params.pSteps->IsAutogen();
		m_sprAutogen->HandleMessage( msg );
		m_sprAutogen->SetVisible( b );
	}

	if( m_bShowStepsType )
	{
		if( params.st != StepsType_Invalid )
		{
			/*
			RString sStepsType = GAMEMAN->GetStepsTypeInfo(params.st).szName;
			m_sprStepsType.Load( THEME->GetPathG(m_sMetricsGroup,"StepsType "+sStepsType) );
			*/
			m_sprStepsType->HandleMessage( msg );
		}
	}

	this->HandleMessage( msg );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the StepsDisplay. */ 
class LunaStepsDisplay: public Luna<StepsDisplay>
{
public:
	static int Load( T* p, lua_State *L )		{ p->Load( SArg(1), nullptr ); COMMON_RETURN_SELF; }
	static int SetFromSteps( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) )
		{
			p->SetFromSteps(nullptr);
		}
		else
		{
			Steps *pS = Luna<Steps>::check(L,1);
			p->SetFromSteps( pS );
		}
		COMMON_RETURN_SELF;
	}
	static int SetFromTrail( T* p, lua_State *L )
	{ 
		if( lua_isnil(L,1) )
		{
			p->SetFromTrail(nullptr);
		}
		else
		{
			Trail *pT = Luna<Trail>::check(L,1);
			p->SetFromTrail( pT );
		}
		COMMON_RETURN_SELF;
	}
	static int SetFromGameState( T* p, lua_State *L )
	{
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		p->SetFromGameState( pn );
		COMMON_RETURN_SELF;
	}

	LunaStepsDisplay()
	{
		ADD_METHOD( Load );
		ADD_METHOD( SetFromSteps );
		ADD_METHOD( SetFromTrail );
		ADD_METHOD( SetFromGameState );
	}
};

LUA_REGISTER_DERIVED_CLASS( StepsDisplay, ActorFrame )
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
