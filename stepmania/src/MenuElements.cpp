#include "global.h"
/*
-----------------------------------------------------------------------------
 File: MenuElements.h

 Desc: Base class for menu Screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "MenuElements.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "MenuTimer.h"
#include "HelpDisplay.h"


#define TIMER_SECONDS			THEME->GetMetricI(m_sName,"TimerSeconds")
#define STYLE_ICON				THEME->GetMetricB(m_sName,"StyleIcon")
#define MEMORY_CARD_ICONS		THEME->GetMetricB(m_sName,"MemoryCardIcons")


MenuElements::MenuElements()
{
	m_MenuTimer = new MenuTimer;
	m_textHelp = new HelpDisplay;
}

MenuElements::~MenuElements()
{
	delete m_MenuTimer;
	delete m_textHelp;
}

void MenuElements::Load( CString sClassName )
{
	LOG->Trace( "MenuElements::MenuElements()" );

	ASSERT( this->m_SubActors.empty() );	// don't call Load twice!

	this->SetName( sClassName );

	m_Background.LoadFromAniDir( THEME->GetPathToB(m_sName+" background") );
	this->AddChild( &m_Background );

	m_autoHeader.Load( THEME->GetPathToG(m_sName+" header") );
	m_autoHeader->SetName("Header");
	UtilSetXY( m_autoHeader, "MenuElements" );
	UtilOnCommand( m_autoHeader, "MenuElements" );
	this->AddChild( m_autoHeader );

	if( STYLE_ICON && GAMESTATE->m_CurStyle != STYLE_INVALID )
	{
		CString sIconFileName = ssprintf("MenuElements icon %s", GAMESTATE->GetCurrentStyleDef()->m_szName );
		m_sprStyleIcon.SetName( "StyleIcon" );
		m_sprStyleIcon.Load( THEME->GetPathToG(sIconFileName) );
		m_sprStyleIcon.StopAnimating();
		UtilSetXY( m_sprStyleIcon, "MenuElements" );
		UtilOnCommand( m_sprStyleIcon, "MenuElements" );
		this->AddChild( &m_sprStyleIcon );
	}
	
	if( MEMORY_CARD_ICONS )
	{
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			m_MemoryCardDisplay[p].Load( (PlayerNumber)p );
			m_MemoryCardDisplay[p].SetName( ssprintf("MemoryCardDisplayP%d",p+1) );
			UtilSetXY( m_MemoryCardDisplay[p], "MenuElements" );
			UtilOnCommand( m_MemoryCardDisplay[p], "MenuElements" );
			this->AddChild( &m_MemoryCardDisplay[p] );
		}
	}

	m_bTimerEnabled = (TIMER_SECONDS != -1);
	if( m_bTimerEnabled )
	{
		m_MenuTimer->SetName( "Timer" );
		UtilSetXY( m_MenuTimer, "MenuElements" );
		UtilOnCommand( m_MenuTimer, "MenuElements" );
		if( TIMER_SECONDS > 0 && PREFSMAN->m_bMenuTimer  &&  !GAMESTATE->m_bEditing )
			m_MenuTimer->SetSeconds( TIMER_SECONDS );
		else
			m_MenuTimer->Disable();
		this->AddChild( m_MenuTimer );
	}

	m_autoFooter.Load( THEME->GetPathToG(m_sName+" footer") );
	m_autoFooter->SetName("Footer");
	UtilSetXY( m_autoFooter, "MenuElements" );
	UtilOnCommand( m_autoFooter, "MenuElements" );
	this->AddChild( m_autoFooter );

	m_textHelp->SetName( "HelpDisplay", "Help" );
	m_textHelp->Load();
	UtilSetXY( m_textHelp, "MenuElements" );
	UtilOnCommand( m_textHelp, "MenuElements" );
	CStringArray asHelpTips;
	split( THEME->GetMetric(m_sName,"HelpText"), "\n", asHelpTips );
	m_textHelp->SetTips( asHelpTips );
	this->AddChild( m_textHelp );


	m_In.Load( THEME->GetPathToB(m_sName+" in") );
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathToB(m_sName+" out") );
	this->AddChild( &m_Out );

	m_Back.Load( THEME->GetPathToB("Common back") );
	this->AddChild( &m_Back );


	m_soundBack.Load( THEME->GetPathToS("Common back") );

	m_In.StartTransitioning();
}

void MenuElements::Update( float fDeltaTime )
{
	ActorFrame::Update(fDeltaTime);
}

void MenuElements::StartTransitioning( ScreenMessage smSendWhenDone )
{
	if( m_bTimerEnabled )
	{
		m_MenuTimer->SetSeconds( 0 );
		m_MenuTimer->Stop();
		UtilOffCommand( m_MenuTimer, "MenuElements" );
	}

	UtilOffCommand( m_autoHeader, "MenuElements" );
	UtilOffCommand( m_sprStyleIcon, "MenuElements" );
	for( int p=0; p<NUM_PLAYERS; p++ )
		UtilOffCommand( m_MemoryCardDisplay[p], "MenuElements" );
	UtilOffCommand( m_autoFooter, "MenuElements" );
	UtilOffCommand( m_textHelp, "MenuElements" );

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

void MenuElements::Back( ScreenMessage smSendWhenDone )
{
	if( m_Back.IsTransitioning() )
		return;	// ignore

	m_MenuTimer->Stop();
	m_Back.StartTransitioning( smSendWhenDone );
	m_soundBack.Play();
}

void MenuElements::DrawPrimitives()
{
	// do nothing.  Call DrawBottomLayer() and DrawTopLayer() instead.
}

void MenuElements::DrawTopLayer()
{
	ASSERT( !this->m_SubActors.empty() );	// Load first
	BeginDraw();

	m_autoHeader->Draw();
	m_sprStyleIcon.Draw();
	m_autoFooter->Draw();
	for( int p=0; p<NUM_PLAYERS; p++ )
		m_MemoryCardDisplay[p].Draw();
	if( m_bTimerEnabled )
		m_MenuTimer->Draw();
	m_textHelp->Draw();
	m_In.Draw();
	m_Out.Draw();
	m_Back.Draw();

	EndDraw();
}

void MenuElements::DrawBottomLayer()
{
	BeginDraw();

	m_Background.Draw();

	EndDraw();
}

bool MenuElements::IsTransitioning()
{
	return m_In.IsTransitioning() || m_Out.IsTransitioning() || m_Back.IsTransitioning();
}

void MenuElements::StopTimer()
{
	m_MenuTimer->Stop();
}

