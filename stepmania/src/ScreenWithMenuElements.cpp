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

#define TIMER_SECONDS			THEME->GetMetricI(m_sName,"TimerSeconds")
#define STYLE_ICON				THEME->GetMetricB(m_sName,"StyleIcon")
#define MEMORY_CARD_ICONS		THEME->GetMetricB(m_sName,"MemoryCardIcons")

ScreenWithMenuElements::ScreenWithMenuElements( CString sClassName ) : Screen( sClassName )
{
	LOG->Trace( "ScreenWithMenuElements::ScreenWithMenuElements()" );
	
	m_MenuTimer = new MenuTimer;
	m_textHelp = new HelpDisplay;


	LOG->Trace( "MenuElements::MenuElements()" );

	ASSERT( this->m_SubActors.empty() );	// don't call Load twice!

	this->SetName( sClassName );

	m_Background.LoadFromAniDir( THEME->GetPathToB(m_sName+" background") );
	this->AddChild( &m_Background );

	m_autoHeader.Load( THEME->GetPathToG(m_sName+" header") );
	m_autoHeader->SetName("Header");
	UtilSetXY( m_autoHeader, m_sName );
	UtilOnCommand( m_autoHeader, m_sName );
	this->AddChild( m_autoHeader );

	if( STYLE_ICON && GAMESTATE->m_pCurStyle )
	{
		CString sIconFileName = CString("MenuElements icon ") + GAMESTATE->GetCurrentStyle()->m_szName;
		m_sprStyleIcon.SetName( "StyleIcon" );
		m_sprStyleIcon.Load( THEME->GetPathToG(sIconFileName) );
		m_sprStyleIcon.StopAnimating();
		m_sprStyleIcon.SetState( GAMESTATE->m_MasterPlayerNumber );
		UtilSetXY( m_sprStyleIcon, m_sName );
		UtilOnCommand( m_sprStyleIcon, m_sName );
		this->AddChild( &m_sprStyleIcon );
	}
	
	if( MEMORY_CARD_ICONS )
	{
		FOREACH_PlayerNumber( p )
		{
			m_MemoryCardDisplay[p].Load( (PlayerNumber)p );
			m_MemoryCardDisplay[p].SetName( ssprintf("MemoryCardDisplayP%d",p+1) );
			UtilSetXY( m_MemoryCardDisplay[p], m_sName );
			UtilOnCommand( m_MemoryCardDisplay[p], m_sName );
			this->AddChild( &m_MemoryCardDisplay[p] );
		}
	}

	m_bTimerEnabled = (TIMER_SECONDS != -1);
	if( m_bTimerEnabled )
	{
		m_MenuTimer->SetName( "Timer" );
		UtilSetXY( m_MenuTimer, m_sName );
		UtilOnCommand( m_MenuTimer, m_sName );
		ResetTimer();
		this->AddChild( m_MenuTimer );
	}

	m_autoFooter.Load( THEME->GetPathToG(m_sName+" footer") );
	m_autoFooter->SetName("Footer");
	UtilSetXY( m_autoFooter, m_sName );
	UtilOnCommand( m_autoFooter, m_sName );
	this->AddChild( m_autoFooter );

	m_textHelp->SetName( "HelpDisplay", "Help" );
	m_textHelp->Load();
	UtilSetXY( m_textHelp, m_sName );
	UtilOnCommand( m_textHelp, m_sName );
	CStringArray asHelpTips;
	split( THEME->GetMetric(m_sName,"HelpText"), "\n", asHelpTips );
	m_textHelp->SetTips( asHelpTips );
	this->AddChild( m_textHelp );


	m_In.Load( THEME->GetPathToB(m_sName+" in") );
	m_In.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathToB(m_sName+" out") );
	m_Out.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_Out );

	m_Back.Load( THEME->GetPathToB("Common back") );
	m_Back.SetDrawOrder( DRAW_ORDER_TRANSITIONS );
	this->AddChild( &m_Back );

	m_In.StartTransitioning();
}

ScreenWithMenuElements::~ScreenWithMenuElements()
{
	SAFE_DELETE( m_MenuTimer );
	SAFE_DELETE( m_textHelp );
}

void ScreenWithMenuElements::ResetTimer()
{
	if( TIMER_SECONDS > 0 && PREFSMAN->m_bMenuTimer  &&  !GAMESTATE->m_bEditing )
	{
		m_MenuTimer->SetSeconds( TIMER_SECONDS );
		m_MenuTimer->Start();
	}
	else
		m_MenuTimer->Disable();
}

void ScreenWithMenuElements::StartTransitioning( ScreenMessage smSendWhenDone )
{
	if( m_bTimerEnabled )
	{
		m_MenuTimer->SetSeconds( 0 );
		m_MenuTimer->Stop();
		UtilOffCommand( m_MenuTimer, m_sName );
	}

	UtilOffCommand( m_autoHeader, m_sName );
	UtilOffCommand( m_sprStyleIcon, m_sName );
	FOREACH_PlayerNumber( p )
		UtilOffCommand( m_MemoryCardDisplay[p], m_sName );
	UtilOffCommand( m_autoFooter, m_sName );
	UtilOffCommand( m_textHelp, m_sName );

	m_Background.PlayCommand("Off");

	m_Out.StartTransitioning(smSendWhenDone);

	/* Ack.  If the transition finishes transparent (eg. _options to options),
	 * then we don't want to send the message until all of the *actors* are
	 * done tweening.  However, if it finishes with something onscreen (most
	 * of the rest), we have to send the message immediately after it finishes,
	 * or we'll draw a frame without the transition.
	 *
	 * For now, I'll make the SMMAX2 option tweening faster. */
	/* This includes all of the actors: */
//	float TimeUntilFinished = GetTweenTimeLeft();
//	TimeUntilFinished = max(TimeUntilFinished, m_Out.GetLengthSeconds());
//	SCREENMAN->PostMessageToTopScreen( smSendWhenDone, TimeUntilFinished );
}

void ScreenWithMenuElements::Back( ScreenMessage smSendWhenDone )
{
	if( m_Back.IsTransitioning() )
		return;	// ignore

	m_MenuTimer->Stop();
	m_Back.StartTransitioning( smSendWhenDone );
	SCREENMAN->PlayBackSound();
}

bool ScreenWithMenuElements::IsTransitioning()
{
	return m_In.IsTransitioning() || m_Out.IsTransitioning() || m_Back.IsTransitioning();
}

void ScreenWithMenuElements::StopTimer()
{
	m_MenuTimer->Stop();
}

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
