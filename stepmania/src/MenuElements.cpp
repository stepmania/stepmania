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
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameState.h"
#include "ThemeManager.h"


#define HEADER_ON_COMMAND		THEME->GetMetric("MenuElements","HeaderOnCommand")
#define HEADER_OFF_COMMAND		THEME->GetMetric("MenuElements","HeaderOffCommand")
#define FOOTER_ON_COMMAND		THEME->GetMetric("MenuElements","FooterOnCommand")
#define FOOTER_OFF_COMMAND		THEME->GetMetric("MenuElements","FooterOffCommand")
#define STYLE_ICON_ON_COMMAND	THEME->GetMetric("MenuElements","StyleIconOnCommand")
#define STYLE_ICON_OFF_COMMAND	THEME->GetMetric("MenuElements","StyleIconOffCommand")
#define TIMER_ON_COMMAND		THEME->GetMetric("MenuElements","TimerOnCommand")
#define TIMER_OFF_COMMAND		THEME->GetMetric("MenuElements","TimerOffCommand")
#define HELP_ON_COMMAND			THEME->GetMetric("MenuElements","HelpOnCommand")
#define HELP_OFF_COMMAND		THEME->GetMetric("MenuElements","HelpOffCommand")


MenuElements::MenuElements()
{
}

void MenuElements::Load( CString sClassName, bool bEnableTimer, bool bLoadStyleIcon )
{
	LOG->Trace( "MenuElements::MenuElements()" );

	ASSERT( this->m_SubActors.empty() );	// don't call Load twice!

	m_sClassName = sClassName;

	m_Background.LoadFromAniDir( THEME->GetPathTo("BGAnimations",m_sClassName+" background") );
	this->AddChild( &m_Background );

	m_sprHeader.Load( THEME->GetPathTo("Graphics",m_sClassName+" header") );
	m_sprHeader.Command( HEADER_ON_COMMAND );
	this->AddChild( &m_sprHeader );

	if( bLoadStyleIcon  &&  GAMESTATE->m_CurStyle != STYLE_INVALID )
	{
		CString sIconFileName = ssprintf("MenuElements icon %s", GAMESTATE->GetCurrentStyleDef()->m_szName );
		m_sprStyleIcon.Load( THEME->GetPathTo("Graphics",sIconFileName) );
		m_sprStyleIcon.StopAnimating();
		m_sprStyleIcon.Command( STYLE_ICON_ON_COMMAND );
		this->AddChild( &m_sprStyleIcon );
	}
	
	m_MenuTimer.Command( TIMER_ON_COMMAND );
	if( bEnableTimer  &&  PREFSMAN->m_bMenuTimer  &&  !GAMESTATE->m_bEditing )
		m_MenuTimer.SetSeconds( THEME->GetMetricI(m_sClassName,"TimerSeconds") );
	else
		m_MenuTimer.Disable();
	this->AddChild( &m_MenuTimer );

	m_sprFooter.Load( THEME->GetPathTo("Graphics",m_sClassName+" footer") );
	m_sprFooter.Command( FOOTER_ON_COMMAND );
	this->AddChild( &m_sprFooter );

	m_textHelp.Command( HELP_ON_COMMAND );
	CStringArray asHelpTips;
	split( THEME->GetMetric(m_sClassName,"HelpText"), "\n", asHelpTips );
	m_textHelp.SetTips( asHelpTips );
	this->AddChild( &m_textHelp );


	m_In.Load( THEME->GetPathTo("BGAnimations",m_sClassName+" in") );
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathTo("BGAnimations",m_sClassName+" out") );
	this->AddChild( &m_Out );

	m_Back.Load( THEME->GetPathTo("BGAnimations","Common back") );
	this->AddChild( &m_Back );


	m_soundBack.Load( THEME->GetPathTo("Sounds","Common back") );


	m_In.StartTransitioning( SM_None );
}

void MenuElements::StartTransitioning( ScreenMessage smSendWhenDone )
{
	m_MenuTimer.SetSeconds( 0 );
	m_MenuTimer.Stop();

	m_sprHeader.Command( HEADER_OFF_COMMAND );
	m_sprStyleIcon.Command( STYLE_ICON_OFF_COMMAND );
	m_MenuTimer.Command( TIMER_OFF_COMMAND );
	m_sprFooter.Command( FOOTER_OFF_COMMAND );
	m_textHelp.Command( HELP_OFF_COMMAND );

	m_Out.StartTransitioning( smSendWhenDone );
}

void MenuElements::Back( ScreenMessage smSendWhenDone )
{
	if( m_Back.IsTransitioning() )
		return;	// ignore

	m_MenuTimer.Stop();
	m_Back.StartTransitioning( smSendWhenDone );
	m_soundBack.Play();
}

void MenuElements::DrawPrimitives()
{
	// do nothing.  Call DrawBottomLayer() and DrawTopLayer() instead.
}

void MenuElements::DrawTopLayer()
{
	BeginDraw();

	m_sprHeader.Draw();
	m_sprStyleIcon.Draw();
	m_MenuTimer.Draw();
	m_sprFooter.Draw();
	m_textHelp.Draw();
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
