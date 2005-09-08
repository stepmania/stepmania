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
#include "AnnouncerManager.h"

#define TIMER_STEALTH			THEME->GetMetricB(m_sName,"TimerStealth")
#define STYLE_ICON				THEME->GetMetricB(m_sName,"StyleIcon")
#define SHOW_STAGE				THEME->GetMetricB(m_sName,"ShowStage")
#define MEMORY_CARD_ICONS		THEME->GetMetricB(m_sName,"MemoryCardIcons")
#define FORCE_TIMER				THEME->GetMetricB(m_sName,"ForceTimer")
#define STOP_MUSIC_ON_BACK		THEME->GetMetricB(m_sName,"StopMusicOnBack")
#define WAIT_FOR_CHILDREN_BEFORE_TWEENING_OUT		THEME->GetMetricB(m_sName,"WaitForChildrenBeforeTweeningOut")

//REGISTER_SCREEN_CLASS( ScreenWithMenuElements );
ScreenWithMenuElements::ScreenWithMenuElements( CString sClassName ) : Screen( sClassName )
{
	m_MenuTimer = NULL;
	m_textHelp = new HelpDisplay;
	FOREACH_PlayerNumber( p )
		m_MemoryCardDisplay[p] = NULL;
	m_MenuTimer = NULL;

	// Needs to be in the constructor in case a derivitive decides to skip 
	// itself and sends SM_GoToNextScreen to ScreenAttract.
	PLAY_MUSIC		.Load( m_sName, "PlayMusic" );
	TIMER_SECONDS	.Load( m_sName, "TimerSeconds" );
}

void ScreenWithMenuElements::Init()
{
	LOG->Trace( "ScreenWithMenuElements::Init()" );

	Screen::Init();

	ASSERT( this->m_SubActors.empty() );	// don't call Init twice!

	m_autoHeader.Load( THEME->GetPathG(m_sName,"header") );
	m_autoHeader->SetName("Header");
	SET_XY( m_autoHeader );
	this->AddChild( m_autoHeader );

	if( STYLE_ICON && GAMESTATE->m_pCurStyle )
	{
		m_sprStyleIcon.SetName( "StyleIcon" );
		m_sprStyleIcon.Load( THEME->GetPathG("MenuElements",CString("icon ")+GAMESTATE->GetCurrentStyle()->m_szName) );
		m_sprStyleIcon.StopAnimating();
		m_sprStyleIcon.SetState( GAMESTATE->m_MasterPlayerNumber );
		SET_XY( m_sprStyleIcon );
		this->AddChild( &m_sprStyleIcon );
	}
	
	if( SHOW_STAGE && GAMESTATE->m_pCurStyle )
	{
		FOREACH_Stage( s )
		{
			if( !GAMESTATE->IsStagePossible(s) )
				continue;
			m_sprStage[s].Load( THEME->GetPathG(m_sName,"stage "+StageToString(s)) );
			m_sprStage[s]->SetName( "Stage" );
		}

		// UpdateStage to hide/show the appropriate stage graphic.  Do this before
		// running the OnCommand so that it won't override the OnCommand if On chooses
		// to hide the graphic.
		UpdateStage();
		
		FOREACH_Stage( s )
		{
			if( !GAMESTATE->IsStagePossible(s) )
				continue;
			SET_XY( m_sprStage[s] );
			this->AddChild( m_sprStage[s] );
		}
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
}

void ScreenWithMenuElements::BeginScreen()
{
	m_In.Reset();
	m_Out.Reset();
	m_Cancel.Reset();

	TweenOnScreen();
	m_In.StartTransitioning( SM_DoneFadingIn );

	Screen::BeginScreen();

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo(m_sName+" intro") );
	StartPlayingMusic();

	/* Evaluate FirstUpdateCommand. */
	this->PlayCommand( "FirstUpdate" );
}

void ScreenWithMenuElements::TweenOnScreen()
{
	ON_COMMAND( m_autoHeader );
	if( STYLE_ICON && GAMESTATE->m_pCurStyle )
		ON_COMMAND( m_sprStyleIcon );

	if( SHOW_STAGE && GAMESTATE->m_pCurStyle )
	{
		// UpdateStage to hide/show the appropriate stage graphic.  Do this before
		// running the OnCommand so that it won't override the OnCommand if On chooses
		// to hide the graphic.
		UpdateStage();

		FOREACH_Stage( s )
		{
			if( !GAMESTATE->IsStagePossible(s) )
				continue;
			ON_COMMAND( m_sprStage[s] );
		}
	}

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

	this->SortByDrawOrder();
}

ScreenWithMenuElements::~ScreenWithMenuElements()
{
	SAFE_DELETE( m_MenuTimer );
	SAFE_DELETE( m_textHelp );
}

void ScreenWithMenuElements::LoadHelpText()
{
	CStringArray vs;
	CString s = THEME->GetMetric(m_sName,"HelpText");
	split( s, "\n", vs );

	// hack to get newlines in a tip
	FOREACH( CString, vs, s )
		s->Replace( "\\n", "\n" );

	m_textHelp->SetTips( vs );
	m_textHelp->PlayCommand( "Changed" );
}

void ScreenWithMenuElements::StartPlayingMusic()
{
	/* Some screens should leave the music alone (eg. ScreenPlayerOptions music 
	 * sample left over from ScreenSelectMusic). */
	if( PLAY_MUSIC )
		SOUND->PlayMusic( THEME->GetPathS(m_sName,"music") );
}

void ScreenWithMenuElements::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
}

void ScreenWithMenuElements::ResetTimer()
{
	if( m_MenuTimer == NULL )
		return;

	if( TIMER_SECONDS > 0.0f  &&  (PREFSMAN->m_bMenuTimer || FORCE_TIMER)  &&  !GAMESTATE->IsEditing() )
	{
		m_MenuTimer->SetSeconds( TIMER_SECONDS );
		m_MenuTimer->Start();
	}
	else
	{
		m_MenuTimer->Disable();
	}
}

void ScreenWithMenuElements::StartTransitioning( ScreenMessage smSendWhenDone )
{
	TweenOffScreen();

	if( WAIT_FOR_CHILDREN_BEFORE_TWEENING_OUT )
	{
		// Time the transition so that it finishes exactly when all actors have 
		// finished tweening.
		float fSecondsUntilFinished = GetTweenTimeLeft();
		float fSecondsUntilBeginOff = max( fSecondsUntilFinished - m_Out.GetLengthSeconds(), 0 );
		m_Out.SetHibernate( fSecondsUntilBeginOff );
	}
	m_Out.StartTransitioning( smSendWhenDone );
}

void ScreenWithMenuElements::TweenOffScreen()
{
	if( m_MenuTimer )
	{
		m_MenuTimer->SetSeconds( 0 );
		m_MenuTimer->Stop();
		ActorUtil::OffCommand( m_MenuTimer, m_sName );
	}

	ActorUtil::OffCommand( m_autoHeader, m_sName );
	ActorUtil::OffCommand( m_sprStyleIcon, m_sName );
	OFF_COMMAND( m_sprStage[GAMESTATE->GetCurrentStage()] );
	FOREACH_PlayerNumber( p )
		if( m_MemoryCardDisplay[p] )
			ActorUtil::OffCommand( m_MemoryCardDisplay[p], m_sName );
	ActorUtil::OffCommand( m_autoFooter, m_sName );
	ActorUtil::OffCommand( m_textHelp, m_sName );
	OFF_COMMAND( m_sprUnderlay );
	OFF_COMMAND( m_sprOverlay );

	SCREENMAN->PlaySharedBackgroundOffCommand();

}

void ScreenWithMenuElements::Cancel( ScreenMessage smSendWhenDone )
{
	if( m_Cancel.IsTransitioning() )
		return;	// ignore

	if( STOP_MUSIC_ON_BACK )
		SOUND->StopMusic();

	if( m_MenuTimer )
		m_MenuTimer->Stop();
	m_Cancel.StartTransitioning( smSendWhenDone );
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

void ScreenWithMenuElements::HandleMessage( const CString& sMessage )
{
	if( sMessage == MessageToString(Message_CurrentSongChanged) )
		UpdateStage();
}
	
void ScreenWithMenuElements::UpdateStage()
{
	// Refresh the stage counter display.  This is useful when changing 
	// between long versions/marathons songs on the wheel.
	FOREACH_Stage( s )
		m_sprStage[s]->SetHidden( s != GAMESTATE->GetCurrentStage() );
}

// lua start
#include "LuaBinding.h"

class LunaScreenWithMenuElements: public Luna<ScreenWithMenuElements>
{
public:
	LunaScreenWithMenuElements() { LUA->Register( Register ); }

	static void Register( Lua *L )
	{
		Luna<T>::Register( L );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenWithMenuElements, Screen )
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
