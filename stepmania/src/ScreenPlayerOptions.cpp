#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenPlayerOptions

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenPlayerOptions.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "NoteSkinManager.h"

#define SONGSEL_SCREEN				THEME->GetMetric("ScreenGameplay","SongSelectScreen")

enum {
	PO_SPEED = 0,
	PO_ACCEL,
	PO_EFFECT,
	PO_APPEAR,
	PO_TURN,
	PO_TRANSFORM,
	PO_SCROLL,
	PO_NOTE_SKIN,
	PO_HOLD_NOTES,
	PO_DARK,
	NUM_PLAYER_OPTIONS_LINES
};
OptionRowData g_PlayerOptionsLines[NUM_PLAYER_OPTIONS_LINES] = {
	{ "Speed",			11, {"x0.25","x0.5","x0.75","x1","x1.5","x2","x3","x4","x5","x8", "x12"} },	
	{ "Acceler\n-ation",6, {"OFF","BOOST","LAND","WAVE","EXPAND","BOOMERANG"} },	
	{ "Effect",			7, {"OFF","DRUNK","DIZZY","SPACE","MINI","FLIP","TORNADO"} },	
	{ "Appear\n-ance",	5, {"VISIBLE","HIDDEN","SUDDEN","STEALTH","BLINK"} },	
	{ "Turn",			6, {"OFF","MIRROR","LEFT","RIGHT","SHUFFLE","SUPER SHUFFLE"} },	
	{ "Trans\n-form",	6, {"OFF","LITTLE","WIDE","BIG","QUICK","SKIPPY"} },	
	{ "Scroll",			2, {"STANDARD","REVERSE"} },	
	{ "Note\nSkin",		0, {""} },	
	{ "Holds",			2, {"OFF","ON"} },	
	{ "Dark",			2, {"OFF","ON"} },	
};


ScreenPlayerOptions::ScreenPlayerOptions() :
	ScreenOptions("ScreenPlayerOptions",true)
{
	LOG->Trace( "ScreenPlayerOptions::ScreenPlayerOptions()" );
	
	Init( 
		INPUTMODE_PLAYERS, 
		g_PlayerOptionsLines, 
		NUM_PLAYER_OPTIONS_LINES,
		true );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("player options intro") );

	m_sprOptionsMessage.Load( THEME->GetPathTo("Graphics","ScreenPlayerOptions options") );
	m_sprOptionsMessage.StopAnimating();
	m_sprOptionsMessage.SetXY( CENTER_X, CENTER_Y );
	m_sprOptionsMessage.SetZoom( 1 );
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
	//this->AddChild( &m_sprOptionsMessage );	// we have to draw this manually over the top of transitions

	m_bAcceptedChoices = false;
	m_bGoToOptions = false;
}


void ScreenPlayerOptions::ImportOptions()
{
	//
	// fill in skin names
	//
	CStringArray arraySkinNames;
	NOTESKIN->GetNoteSkinNames( arraySkinNames );

	m_OptionRowData[PO_NOTE_SKIN].iNumOptions	=	arraySkinNames.size(); 

	for( unsigned i=0; i<arraySkinNames.size(); i++ )
	{
		arraySkinNames[i].MakeUpper();
		strcpy( m_OptionRowData[PO_NOTE_SKIN].szOptionsText[i], arraySkinNames[i] ); 
	}


	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		PlayerOptions &po = GAMESTATE->m_PlayerOptions[p];
		
		if(		 po.m_fScrollSpeed == 0.25f )	m_iSelectedOption[p][PO_SPEED] = 0;
		else if( po.m_fScrollSpeed == 0.5f )	m_iSelectedOption[p][PO_SPEED] = 1;
		else if( po.m_fScrollSpeed == 0.75f)	m_iSelectedOption[p][PO_SPEED] = 2;
		else if( po.m_fScrollSpeed == 1.0f )	m_iSelectedOption[p][PO_SPEED] = 3;
		else if( po.m_fScrollSpeed == 1.5f )	m_iSelectedOption[p][PO_SPEED] = 4;
		else if( po.m_fScrollSpeed == 2.0f )	m_iSelectedOption[p][PO_SPEED] = 5;
		else if( po.m_fScrollSpeed == 3.0f )	m_iSelectedOption[p][PO_SPEED] = 6;
		else if( po.m_fScrollSpeed == 4.0f )	m_iSelectedOption[p][PO_SPEED] = 7;
		else if( po.m_fScrollSpeed == 5.0f )	m_iSelectedOption[p][PO_SPEED] = 8;
		else if( po.m_fScrollSpeed == 8.0f )	m_iSelectedOption[p][PO_SPEED] = 9;
		else if( po.m_fScrollSpeed == 12.0f)	m_iSelectedOption[p][PO_SPEED] = 10;
		else									m_iSelectedOption[p][PO_SPEED] = 3;

		m_iSelectedOption[p][PO_ACCEL]		= po.GetFirstAccel()+1;
		m_iSelectedOption[p][PO_EFFECT]		= po.GetFirstEffect()+1;
		m_iSelectedOption[p][PO_APPEAR]		= po.GetFirstAppearance()+1;
		m_iSelectedOption[p][PO_TURN]		= po.m_Turn;
		m_iSelectedOption[p][PO_TRANSFORM]	= po.m_Transform;
		m_iSelectedOption[p][PO_SCROLL]		= po.m_fReverseScroll==1 ? 1 : 0 ;


		// highlight currently selected skin
		m_iSelectedOption[p][PO_NOTE_SKIN] = -1;
		for( unsigned j=0; j<m_OptionRowData[PO_NOTE_SKIN].iNumOptions; j++ )
		{
			if( 0==stricmp(m_OptionRowData[PO_NOTE_SKIN].szOptionsText[j], NOTESKIN->GetCurNoteSkinName((PlayerNumber)p)) )
			{
				m_iSelectedOption[p][PO_NOTE_SKIN] = j;
				break;
			}
		}
		if( m_iSelectedOption[p][PO_NOTE_SKIN] == -1 )
			m_iSelectedOption[p][PO_NOTE_SKIN] = 0;


		m_iSelectedOption[p][PO_HOLD_NOTES]	= po.m_bHoldNotes ? 1 : 0;
		m_iSelectedOption[p][PO_DARK]		= po.m_fDark==1 ? 1 : 0;


		po.Init();
	}
}

void ScreenPlayerOptions::ExportOptions()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		PlayerOptions &po = GAMESTATE->m_PlayerOptions[p];
		po.Init();

		switch( m_iSelectedOption[p][PO_SPEED] )
		{
		case 0:	po.m_fScrollSpeed = 0.25f;	break;
		case 1:	po.m_fScrollSpeed = 0.5f;	break;
		case 2:	po.m_fScrollSpeed = 0.75f;	break;
		case 3:	po.m_fScrollSpeed = 1.0f;	break;
		case 4:	po.m_fScrollSpeed = 1.5f;	break;
		case 5:	po.m_fScrollSpeed = 2.0f;	break;
		case 6:	po.m_fScrollSpeed = 3.0f;	break;
		case 7:	po.m_fScrollSpeed = 4.0f;	break;
		case 8:	po.m_fScrollSpeed = 5.0f;	break;
		case 9:	po.m_fScrollSpeed = 8.0f;	break;
		case 10: po.m_fScrollSpeed = 12.0f;	break;
		}

		if( m_iSelectedOption[p][PO_ACCEL] != 0 )
			po.SetOneAccel( (PlayerOptions::Accel)(m_iSelectedOption[p][PO_ACCEL]-1) );
		if( m_iSelectedOption[p][PO_EFFECT] != 0 )
			po.SetOneEffect( (PlayerOptions::Effect)(m_iSelectedOption[p][PO_EFFECT]-1) );
		if( m_iSelectedOption[p][PO_APPEAR] != 0 )
			po.SetOneAppearance( (PlayerOptions::Appearance)(m_iSelectedOption[p][PO_APPEAR]-1) );

		po.m_Turn			= (PlayerOptions::Turn)m_iSelectedOption[p][PO_TURN];
		po.m_Transform		= (PlayerOptions::Transform)m_iSelectedOption[p][PO_TRANSFORM];
		po.m_fReverseScroll	= (m_iSelectedOption[p][PO_SCROLL] == 1) ? 1.f : 0.f;


		int iSelectedSkin = m_iSelectedOption[p][PO_NOTE_SKIN];
		CString sNewSkin = m_OptionRowData[PO_NOTE_SKIN].szOptionsText[iSelectedSkin];
		NOTESKIN->SwitchNoteSkin( (PlayerNumber)p, sNewSkin );


		po.m_bHoldNotes		= (m_iSelectedOption[p][PO_HOLD_NOTES] == 1);
		po.m_fDark			= (m_iSelectedOption[p][PO_DARK] == 1) ? 1.f : 0.f;
	}
}

void ScreenPlayerOptions::GoToPrevState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else if( GAMESTATE->m_PlayMode == PLAY_MODE_NONSTOP  ||
		GAMESTATE->m_PlayMode == PLAY_MODE_ONI  ||
		GAMESTATE->m_PlayMode == PLAY_MODE_ENDLESS)
		SCREENMAN->SetNewScreen( "ScreenSelectCourse" );
	else
		SCREENMAN->SetNewScreen( SONGSEL_SCREEN );
}

void ScreenPlayerOptions::GoToNextState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else if( m_bGoToOptions )
		SCREENMAN->SetNewScreen( "ScreenSongOptions" );
	else
		SCREENMAN->SetNewScreen( "ScreenStage" );
}


void ScreenPlayerOptions::Update( float fDelta )
{
	ScreenOptions::Update( fDelta );
	m_sprOptionsMessage.Update( fDelta );
}

void ScreenPlayerOptions::DrawPrimitives()
{
	ScreenOptions::DrawPrimitives();
	m_sprOptionsMessage.Draw();
}


void ScreenPlayerOptions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( !GAMESTATE->m_bEditing  &&
		!m_Menu.m_In.IsTransitioning()  &&
		MenuI.IsValid()  &&
		MenuI.button == MENU_BUTTON_START  &&
		type != IET_RELEASE  &&
		PREFSMAN->m_bShowSongOptions )
	{
		if( m_bAcceptedChoices  &&  !m_bGoToOptions )
		{
			m_bGoToOptions = true;
			m_sprOptionsMessage.SetState( 1 );
			SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","Common start") );
		}
	}

	ScreenOptions::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenPlayerOptions::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_BeginFadingOut:	// when the user accepts the page of options
		{
			m_bAcceptedChoices = true;

			if( !GAMESTATE->m_bEditing )
			{
				float fShowSeconds = m_Menu.m_Out.GetLengthSeconds();

				// show "hold START for options"
				m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
				m_sprOptionsMessage.BeginTweening( 0.15f );	// fade in
				m_sprOptionsMessage.SetTweenZoomY( 1 );
				m_sprOptionsMessage.SetTweenDiffuse( RageColor(1,1,1,1) );
				m_sprOptionsMessage.BeginTweening( fShowSeconds-0.3f );	// sleep
				m_sprOptionsMessage.BeginTweening( 0.15f );	// fade out
				m_sprOptionsMessage.SetTweenDiffuse( RageColor(1,1,1,0) );
				m_sprOptionsMessage.SetTweenZoomY( 0 );
			}
		}
		break;
	}
	ScreenOptions::HandleScreenMessage( SM );
}