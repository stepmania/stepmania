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

#define PREV_SCREEN( play_mode )		THEME->GetMetric ("ScreenPlayerOptions","PrevScreen"+Capitalize(PlayModeToString(play_mode)))
#define NEXT_SCREEN( play_mode )		THEME->GetMetric ("ScreenPlayerOptions","NextScreen"+Capitalize(PlayModeToString(play_mode)))

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
	PO_OTHER,
	PO_PERSPECTIVE,
	NUM_PLAYER_OPTIONS_LINES
};
OptionRow g_PlayerOptionsLines[NUM_PLAYER_OPTIONS_LINES] = {
	OptionRow( "Speed",				"x0.25","x0.5","x0.75","x1","x1.5","x2","x3","x4","x5","x8","x12" ),	
	OptionRow( "Acceler\n-ation",	"OFF","BOOST","BRAKE","WAVE","EXPAND","BOOMERANG" ),	
	OptionRow( "Effect",			"OFF","DRUNK","DIZZY","MINI","FLIP","TORNADO" ),	
	OptionRow( "Appear\n-ance",		"VISIBLE","HIDDEN","SUDDEN","STEALTH","BLINK" ),	
	OptionRow( "Turn",				"OFF","MIRROR","LEFT","RIGHT","SHUFFLE","SUPER SHUFFLE" ),	
	OptionRow( "Trans\n-form",		"OFF","LITTLE","WIDE","BIG","QUICK","SKIPPY" ),	
	OptionRow( "Scroll",			"STANDARD","REVERSE" ),	
	OptionRow( "Note\nSkin",		"" ),	
	OptionRow( "Holds",				"OFF","ON" ),	
	OptionRow( "Other",				"DARK","TIME SPACING" ),	
	OptionRow( "Perspec\n-tive",	"INCOMING","OVERHEAD","SPACE" ),	
};


ScreenPlayerOptions::ScreenPlayerOptions() :
	ScreenOptions("ScreenPlayerOptions",true)
{
	LOG->Trace( "ScreenPlayerOptions::ScreenPlayerOptions()" );
	
	Init( 
		INPUTMODE_PLAYERS, 
		g_PlayerOptionsLines, 
		NUM_PLAYER_OPTIONS_LINES,
		true, false );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("player options intro") );
}


void ScreenPlayerOptions::ImportOptions()
{
	//
	// fill in skin names
	//
	CStringArray arraySkinNames;
	NOTESKIN->GetNoteSkinNames( arraySkinNames );

	m_OptionRow[PO_NOTE_SKIN].choices.clear();

	for( unsigned i=0; i<arraySkinNames.size(); i++ )
	{
		arraySkinNames[i].MakeUpper();
		m_OptionRow[PO_NOTE_SKIN].choices.push_back( arraySkinNames[i] ); 
	}


	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		PlayerOptions &po = GAMESTATE->m_PlayerOptions[p];
		
		if(		 /*!po.m_bUseScrollBPM &&*/ po.m_fScrollSpeed == 0.25f )	m_iSelectedOption[p][PO_SPEED] = 0;
		else if( /*!po.m_bUseScrollBPM &&*/ po.m_fScrollSpeed == 0.5f )		m_iSelectedOption[p][PO_SPEED] = 1;
		else if( /*!po.m_bUseScrollBPM &&*/ po.m_fScrollSpeed == 0.75f )	m_iSelectedOption[p][PO_SPEED] = 2;
		else if( /*!po.m_bUseScrollBPM &&*/ po.m_fScrollSpeed == 1.0f )		m_iSelectedOption[p][PO_SPEED] = 3;
		else if( /*!po.m_bUseScrollBPM &&*/ po.m_fScrollSpeed == 1.5f )		m_iSelectedOption[p][PO_SPEED] = 4;
		else if( /*!po.m_bUseScrollBPM &&*/ po.m_fScrollSpeed == 2.0f )		m_iSelectedOption[p][PO_SPEED] = 5;
		else if( /*!po.m_bUseScrollBPM &&*/ po.m_fScrollSpeed == 3.0f )		m_iSelectedOption[p][PO_SPEED] = 6;
		else if( /*!po.m_bUseScrollBPM &&*/ po.m_fScrollSpeed == 5.0f )		m_iSelectedOption[p][PO_SPEED] = 7;
		else if( /*!po.m_bUseScrollBPM &&*/ po.m_fScrollSpeed == 4.0f )		m_iSelectedOption[p][PO_SPEED] = 8;
		else if( /*!po.m_bUseScrollBPM &&*/ po.m_fScrollSpeed == 8.0f )		m_iSelectedOption[p][PO_SPEED] = 9;
		else if( /*!po.m_bUseScrollBPM &&*/ po.m_fScrollSpeed == 12.0f )	m_iSelectedOption[p][PO_SPEED] = 10;
//		else if( po.m_bUseScrollBPM  && po.m_fScrollBPM == 200 )		m_iSelectedOption[p][PO_SPEED] = 8;
//		else if( po.m_bUseScrollBPM  && po.m_fScrollBPM == 300 )		m_iSelectedOption[p][PO_SPEED] = 9;
//		else if( po.m_bUseScrollBPM  && po.m_fScrollBPM == 450 )		m_iSelectedOption[p][PO_SPEED] = 10;
		else									m_iSelectedOption[p][PO_SPEED] = 3;

		m_iSelectedOption[p][PO_ACCEL]		= po.GetFirstAccel()+1;
		m_iSelectedOption[p][PO_EFFECT]		= po.GetFirstEffect()+1;
		m_iSelectedOption[p][PO_APPEAR]		= po.GetFirstAppearance()+1;
		m_iSelectedOption[p][PO_TURN]		= po.m_Turn;
		m_iSelectedOption[p][PO_TRANSFORM]	= po.m_Transform;
		m_iSelectedOption[p][PO_SCROLL]		= po.m_fReverseScroll==1 ? 1 : 0 ;


		// highlight currently selected skin
		m_iSelectedOption[p][PO_NOTE_SKIN] = -1;
		for( unsigned j=0; j<m_OptionRow[PO_NOTE_SKIN].choices.size(); j++ )
		{
			if( 0==stricmp(m_OptionRow[PO_NOTE_SKIN].choices[j], NOTESKIN->GetCurNoteSkinName((PlayerNumber)p)) )
			{
				m_iSelectedOption[p][PO_NOTE_SKIN] = j;
				break;
			}
		}
		if( m_iSelectedOption[p][PO_NOTE_SKIN] == -1 )
			m_iSelectedOption[p][PO_NOTE_SKIN] = 0;


		m_iSelectedOption[p][PO_HOLD_NOTES]	= po.m_bHoldNotes ? 1 : 0;

		if( po.m_fDark==1 )
			m_iSelectedOption[p][PO_OTHER] = 1;
		else if( po.m_bTimeSpacing )
			m_iSelectedOption[p][PO_OTHER] = 2;
		else
			m_iSelectedOption[p][PO_OTHER] = 0;

		m_iSelectedOption[p][PO_PERSPECTIVE]= (int)po.m_fPerspectiveTilt + 1;


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
		case 0:	/*po.m_bUseScrollBPM = false;*/	po.m_fScrollSpeed = 0.25f;	break;
		case 1:	/*po.m_bUseScrollBPM = false;*/	po.m_fScrollSpeed = 0.5f;	break;
		case 2:	/*po.m_bUseScrollBPM = false;*/	po.m_fScrollSpeed = 0.75f;	break;
		case 3:	/*po.m_bUseScrollBPM = false;*/	po.m_fScrollSpeed = 1.0f;	break;
		case 4:	/*po.m_bUseScrollBPM = false;*/	po.m_fScrollSpeed = 1.5f;	break;
		case 5:	/*po.m_bUseScrollBPM = false;*/	po.m_fScrollSpeed = 2.0f;	break;
		case 6:	/*po.m_bUseScrollBPM = false;*/	po.m_fScrollSpeed = 3.0f;	break;
		case 7:	/*po.m_bUseScrollBPM = false;*/	po.m_fScrollSpeed = 4.0f;	break;
		case 8:	/*po.m_bUseScrollBPM = false;*/	po.m_fScrollSpeed = 5.0f;	break;
		case 9:	/*po.m_bUseScrollBPM = false;*/	po.m_fScrollSpeed = 8.0f;	break;
		case 10:/*po.m_bUseScrollBPM = false;*/	po.m_fScrollSpeed = 12.0f;	break;
//		case 9: po.m_bUseScrollBPM = true;	po.m_fScrollBPM = 200;		break;
//		case 10: po.m_bUseScrollBPM = true;	po.m_fScrollBPM = 300;		break;
//		case 11:po.m_bUseScrollBPM = true;	po.m_fScrollBPM = 450;		break;
		default:	ASSERT(0);
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
		CString sNewSkin = m_OptionRow[PO_NOTE_SKIN].choices[iSelectedSkin];
		NOTESKIN->SwitchNoteSkin( (PlayerNumber)p, sNewSkin );


		po.m_bHoldNotes			= (m_iSelectedOption[p][PO_HOLD_NOTES] == 1);
		po.m_fDark				= (m_iSelectedOption[p][PO_OTHER] == 1) ? 1.f : 0.f;
		po.m_bTimeSpacing		= (m_iSelectedOption[p][PO_OTHER] == 2);
		po.m_fPerspectiveTilt	= (float)m_iSelectedOption[p][PO_PERSPECTIVE] - 1;
	}
}

void ScreenPlayerOptions::GoToPrevState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else
		SCREENMAN->SetNewScreen( PREV_SCREEN(GAMESTATE->m_PlayMode) );
}

void ScreenPlayerOptions::GoToNextState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else if( PREFSMAN->m_bShowSongOptions )
		SCREENMAN->SetNewScreen( "ScreenSongOptions" );
	else
		SCREENMAN->SetNewScreen( "ScreenStage" );
}


void ScreenPlayerOptions::Update( float fDelta )
{
	ScreenOptions::Update( fDelta );
}

void ScreenPlayerOptions::DrawPrimitives()
{
	ScreenOptions::DrawPrimitives();
}


void ScreenPlayerOptions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	ScreenOptions::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenPlayerOptions::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenOptions::HandleScreenMessage( SM );
}