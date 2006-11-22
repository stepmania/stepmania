#include "global.h"
#include "ScreenWithMenuElements.h"
#include "MenuTimer.h"
#include "HelpDisplay.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "Style.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "MemoryCardDisplay.h"
#include "InputEventPlus.h"

#define TIMER_STEALTH				THEME->GetMetricB(m_sName,"TimerStealth")
#define SHOW_STAGE				THEME->GetMetricB(m_sName,"ShowStage")
#define MEMORY_CARD_ICONS			THEME->GetMetricB(m_sName,"MemoryCardIcons")
#define FORCE_TIMER				THEME->GetMetricB(m_sName,"ForceTimer")
#define STOP_MUSIC_ON_BACK			THEME->GetMetricB(m_sName,"StopMusicOnBack")
#define WAIT_FOR_CHILDREN_BEFORE_TWEENING_OUT	THEME->GetMetricB(m_sName,"WaitForChildrenBeforeTweeningOut")

//REGISTER_SCREEN_CLASS( ScreenWithMenuElements );
ScreenWithMenuElements::ScreenWithMenuElements()
{
	m_MenuTimer = NULL;
	m_textHelp = new HelpDisplay;
	FOREACH_PlayerNumber( p )
		m_MemoryCardDisplay[p] = NULL;
	m_MenuTimer = NULL;
}

void ScreenWithMenuElements::Init()
{
	LOG->Trace( "ScreenWithMenuElements::Init()" );

	PLAY_MUSIC.Load( m_sName, "PlayMusic" );
	CANCEL_TRANSITIONS_OUT.Load( m_sName, "CancelTransitionsOut" );
	TIMER_SECONDS.Load( m_sName, "TimerSeconds" );

	Screen::Init();

	ASSERT( this->m_SubActors.empty() );	// don't call Init twice!

	m_autoHeader.Load( THEME->GetPathG(m_sName,"header") );
	m_autoHeader->SetName("Header");
	SET_XY( m_autoHeader );
	this->AddChild( m_autoHeader );

	if( SHOW_STAGE )
	{
		m_sprStage.Load( THEME->GetPathG(m_sName,"stage") );
		m_sprStage->SetName( "Stage" );
		SET_XY( m_sprStage );
		this->AddChild( m_sprStage );
	}
	
	if( MEMORY_CARD_ICONS )
	{
		FOREACH_PlayerNumber( p )
		{
			ASSERT( m_MemoryCardDisplay[p] == NULL );
			m_MemoryCardDisplay[p] = new MemoryCardDisplay;
			m_MemoryCardDisplay[p]->Load( p );
			m_MemoryCardDisplay[p]->SetName( ssprintf("MemoryCardDisplayP%d",p+1) );
			SET_XY( m_MemoryCardDisplay[p] );
			this->AddChild( m_MemoryCardDisplay[p] );
		}
	}

	if( TIMER_SECONDS != -1 )
	{
		ASSERT( m_MenuTimer == NULL );	// don't load twice
		m_MenuTimer = new MenuTimer;
		m_MenuTimer->Load();
		m_MenuTimer->SetName( "Timer" );
		if( TIMER_STEALTH )
			m_MenuTimer->EnableStealth( true );
		SET_XY( m_MenuTimer );
		ResetTimer();
		this->AddChild( m_MenuTimer );
	}

	m_autoFooter.Load( THEME->GetPathG(m_sName,"footer") );
	m_autoFooter->SetName("Footer");
	SET_XY( m_autoFooter );
	this->AddChild( m_autoFooter );

	m_textHelp->SetName( "Help" );
	m_textHelp->Load( "HelpDisplay" );
	m_textHelp->LoadFromFont( THEME->GetPathF("HelpDisplay","text") );
	SET_XY( m_textHelp );
	LoadHelpText();
	this->AddChild( m_textHelp );

	m_sprUnderlay.Load( THEME->GetPathB(m_sName,"underlay") );
	m_sprUnderlay->SetName("Underlay");
	m_sprUnderlay->SetDrawOrder( DRAW_ORDER_UNDERLAY );
	SET_XY( m_sprUnderlay );
	this->AddChild( m_sprUnderlay );
	
	m_sprOverlay.Load( THEME->GetPathB(m_sName,"overlay") );
	m_sprOverlay->SetName("Overlay");
	m_sprOverlay->SetDrawOrder( DRAW_ORDER_OVERLAY );
	SET_XY( m_sprOverlay );
	this->AddChild( m_sprOverlay );

	m_In.SetName( "In" );
	m_In.Load( THEME->GetPathB(m_sName,"in") );
	m_In.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_In );

	m_Out.SetName( "Out" );
	m_Out.Load( THEME->GetPathB(m_sName,"out") );
	m_Out.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_Out );

	m_Cancel.SetName( "Cancel" );
	m_Cancel.Load( THEME->GetPathB(m_sName,"cancel") );
	m_Cancel.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_Cancel );

	/* Grab the music path here; don't GetPath during BeginScreen. */
	if( PLAY_MUSIC )
		m_sPathToMusic = THEME->GetPathS( m_sName, "music" );
}

void ScreenWithMenuElements::BeginScreen()
{
	Screen::BeginScreen();

	m_In.Reset();
	m_Out.Reset();
	m_Cancel.Reset();

	if( GenericTweenOn() )
		this->PlayCommand( "On" );
	else
		TweenOnScreen(); // deprecated
	this->SortByDrawOrder();
	m_In.StartTransitioning( SM_DoneFadingIn );

	SOUND->PlayOnceFromAnnouncer( m_sName+" intro" );
	StartPlayingMusic();

	/* Evaluate FirstUpdateCommand. */
	this->PlayCommand( "FirstUpdate" );
}

void ScreenWithMenuElements::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_MenuTimer )
	{
		FOREACH_HumanPlayer(p)
		{
			InputEventPlus iep;
			iep.pn = p;
			MenuStart( iep );
		}
	}

	Screen::HandleScreenMessage( SM );
}

void ScreenWithMenuElements::TweenOnScreen()
{
	ON_COMMAND( m_autoHeader );

	if( SHOW_STAGE && !m_sprStage->GetName().empty() )
		ON_COMMAND( m_sprStage );

	if( MEMORY_CARD_ICONS )
	{
		FOREACH_PlayerNumber( p )
			ON_COMMAND( m_MemoryCardDisplay[p] );
	}

	if( m_MenuTimer )
		ON_COMMAND( m_MenuTimer );

	ON_COMMAND( m_autoFooter );
	ON_COMMAND( m_textHelp );
	ON_COMMAND( m_sprUnderlay );
	ON_COMMAND( m_sprOverlay );
}

ScreenWithMenuElements::~ScreenWithMenuElements()
{
	SAFE_DELETE( m_MenuTimer );
	SAFE_DELETE( m_textHelp );
	FOREACH_PlayerNumber( p )
	{
		if( m_MemoryCardDisplay[p] != NULL )
			SAFE_DELETE( m_MemoryCardDisplay[p] );
	}
}

void ScreenWithMenuElements::LoadHelpText()
{
	vector<RString> vs;
	RString s = THEME->GetString(m_sName,"HelpText");
	split( s, "::", vs );

	m_textHelp->SetTips( vs );
	m_textHelp->PlayCommand( "Changed" );
}

void ScreenWithMenuElements::StartPlayingMusic()
{
	/* Some screens should leave the music alone (eg. ScreenPlayerOptions music 
	 * sample left over from ScreenSelectMusic). */
	if( PLAY_MUSIC )
		SOUND->PlayMusic( m_sPathToMusic );
}

void ScreenWithMenuElements::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}

void ScreenWithMenuElements::ResetTimer()
{
	if( m_MenuTimer == NULL )
		return;

	if( TIMER_SECONDS > 0.0f && (PREFSMAN->m_bMenuTimer || FORCE_TIMER) )
	{
		m_MenuTimer->SetSeconds( TIMER_SECONDS );
		m_MenuTimer->Start();
	}
	else
	{
		m_MenuTimer->Disable();
	}
}

void ScreenWithMenuElements::StartTransitioningScreen( ScreenMessage smSendWhenDone )
{
	if( GenericTweenOff() )
	{
		this->PlayCommand( "Off" );

		// If we're a stacked screen, then there's someone else between us and the
		// background, so don't tween it off.
		if( !SCREENMAN->IsStackedScreen(this) )
			SCREENMAN->PlaySharedBackgroundOffCommand();
	}
	else
	{
		TweenOffScreen();
	}

	m_Out.StartTransitioning( smSendWhenDone );
	if( WAIT_FOR_CHILDREN_BEFORE_TWEENING_OUT )
	{
		// Time the transition so that it finishes exactly when all actors have 
		// finished tweening.
		float fSecondsUntilFinished = GetTweenTimeLeft();
		float fSecondsUntilBeginOff = max( fSecondsUntilFinished - m_Out.GetTweenTimeLeft(), 0 );
		m_Out.SetHibernate( fSecondsUntilBeginOff );
	}
}

void ScreenWithMenuElements::TweenOffScreen()
{
	if( m_MenuTimer )
	{
		m_MenuTimer->SetSeconds( 0 );
		m_MenuTimer->Stop();
		OFF_COMMAND( m_MenuTimer );
	}

	OFF_COMMAND( m_autoHeader );
	OFF_COMMAND( m_sprStage );
	FOREACH_PlayerNumber( p )
		if( m_MemoryCardDisplay[p] )
			OFF_COMMAND( m_MemoryCardDisplay[p] );
	OFF_COMMAND( m_autoFooter );
	OFF_COMMAND( m_textHelp );
	OFF_COMMAND( m_sprUnderlay );
	OFF_COMMAND( m_sprOverlay );

	// If we're a stacked screen, then there's someone else between us and the
	// background, so don't tween it off.
	if( !SCREENMAN->IsStackedScreen(this) )
		SCREENMAN->PlaySharedBackgroundOffCommand();
}

void ScreenWithMenuElements::Cancel( ScreenMessage smSendWhenDone )
{
	m_sNextScreen = GetPrevScreen();

	if( CANCEL_TRANSITIONS_OUT )
	{
		StartTransitioningScreen( smSendWhenDone );
		COMMAND( m_Out, "Cancel" );
		return;
	}

	if( m_Cancel.IsTransitioning() )
		return;	// ignore

	if( STOP_MUSIC_ON_BACK )
		SOUND->StopMusic();

	if( m_MenuTimer )
		m_MenuTimer->Stop();
	m_Cancel.StartTransitioning( smSendWhenDone );
	COMMAND( m_Cancel, "Cancel" );
}

bool ScreenWithMenuElements::IsTransitioning()
{
	return m_In.IsTransitioning() || m_Out.IsTransitioning() || m_Cancel.IsTransitioning();
}

void ScreenWithMenuElements::StopTimer()
{
	if( m_MenuTimer )
		m_MenuTimer->Stop();
}

REGISTER_SCREEN_CLASS( ScreenWithMenuElementsSimple );

void ScreenWithMenuElementsSimple::MenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;
	if( m_fLockInputSecs > 0 )
		return;

	StartTransitioningScreen( SM_GoToNextScreen );
}

void ScreenWithMenuElementsSimple::MenuBack( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return;
	if( m_fLockInputSecs > 0 )
		return;

	Cancel( SM_GoToPrevScreen );
}

// lua start
#include "LuaBinding.h"

class LunaScreenWithMenuElements: public Luna<ScreenWithMenuElements>
{
public:
	LunaScreenWithMenuElements()
	{
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenWithMenuElements, Screen )

class LunaScreenWithMenuElementsSimple: public Luna<ScreenWithMenuElementsSimple>
{
public:
	LunaScreenWithMenuElementsSimple()
	{
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenWithMenuElementsSimple, ScreenWithMenuElements )

// lua end

/*
 * (c) 2004 Chris Danford
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
