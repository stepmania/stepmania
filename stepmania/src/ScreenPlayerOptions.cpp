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
#include "NoteFieldPositioning.h"
#include "ScreenSongOptions.h"
#include "Notes.h"
#include "Course.h"

#define PREV_SCREEN( play_mode )		THEME->GetMetric ("ScreenPlayerOptions","PrevScreen"+Capitalize(PlayModeToString(play_mode)))
#define NEXT_SCREEN( play_mode )		THEME->GetMetric ("ScreenPlayerOptions","NextScreen"+Capitalize(PlayModeToString(play_mode)))

enum {
	PO_SPEED = 0,
	PO_ACCEL,
	PO_APPEAR,
	PO_TURN,
	PO_SCROLL,
	PO_HOLD_NOTES,
	PO_DARK,
	PO_STEP,
	NUM_PLAYER_OPTIONS_LINES
};
OptionRow g_PlayerOptionsLines[NUM_PLAYER_OPTIONS_LINES] = {
	OptionRow( "Speed",				"x0.25","x0.5","x0.75","x1","x1.5","x2","x3","x5","x8","C200","C300" ),	
	OptionRow( "Acceler\n-ation",	"OFF","BOOST","BRAKE","WAVE","EXPAND","BOOMERANG" ),	
	OptionRow( "Appear\n-ance",		"VISIBLE","HIDDEN","SUDDEN","STEALTH","BLINK", "R.VANISH" ),	
	OptionRow( "Turn",				"OFF","MIRROR","LEFT","RIGHT","SHUFFLE","S.SHUFFLE" ),	
	OptionRow( "Scroll",			"STANDARD","REVERSE" ),	
	OptionRow( "Holds",				"OFF","ON" ),	
	OptionRow( "Dark",				"OFF","ON" ),	
	OptionRow( "Step",				"" )
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
	// fill in difficulty names
	//
	m_OptionRow[PO_STEP].choices.clear();
	if( GAMESTATE->m_pCurCourse )	// playing a course
	{
		m_OptionRow[PO_STEP].choices.push_back( "REGULAR" ); 
		if( GAMESTATE->m_pCurCourse->HasDifficult() )
			m_OptionRow[PO_STEP].choices.push_back( "DIFFICULT" );
	}
	else
	{
		vector<Notes*> vNotes;
		GAMESTATE->m_pCurSong->GetNotes( vNotes, GAMESTATE->GetCurrentStyleDef()->m_NotesType );
		SortNotesArrayByDifficulty( vNotes );
		for( unsigned i=0; i<vNotes.size(); i++ )
		{
			CString s = vNotes[i]->GetDescription();
			s.MakeUpper();
			m_OptionRow[PO_STEP].choices.push_back( s ); 
		}
	}

	//
	// highlight currently selected values
	//
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		PlayerOptions &po = GAMESTATE->m_PlayerOptions[p];
		
		if(		 !po.m_bTimeSpacing && po.m_fScrollSpeed == 0.25f )		m_iSelectedOption[p][PO_SPEED] = 0;
		else if( !po.m_bTimeSpacing && po.m_fScrollSpeed == 0.5f )		m_iSelectedOption[p][PO_SPEED] = 1;
		else if( !po.m_bTimeSpacing && po.m_fScrollSpeed == 0.75f )		m_iSelectedOption[p][PO_SPEED] = 2;
		else if( !po.m_bTimeSpacing && po.m_fScrollSpeed == 1.0f )		m_iSelectedOption[p][PO_SPEED] = 3;
		else if( !po.m_bTimeSpacing && po.m_fScrollSpeed == 1.5f )		m_iSelectedOption[p][PO_SPEED] = 4;
		else if( !po.m_bTimeSpacing && po.m_fScrollSpeed == 2.0f )		m_iSelectedOption[p][PO_SPEED] = 5;
		else if( !po.m_bTimeSpacing && po.m_fScrollSpeed == 3.0f )		m_iSelectedOption[p][PO_SPEED] = 6;
		else if( !po.m_bTimeSpacing && po.m_fScrollSpeed == 5.0f )		m_iSelectedOption[p][PO_SPEED] = 7;
		else if( !po.m_bTimeSpacing && po.m_fScrollSpeed == 8.0f )		m_iSelectedOption[p][PO_SPEED] = 8;
		else if( po.m_bTimeSpacing  && po.m_fScrollBPM == 200 )			m_iSelectedOption[p][PO_SPEED] = 9;
		else if( po.m_bTimeSpacing  && po.m_fScrollBPM == 300 )			m_iSelectedOption[p][PO_SPEED] = 10;
		else									m_iSelectedOption[p][PO_SPEED] = 3;

		m_iSelectedOption[p][PO_ACCEL]		= po.GetFirstAccel()+1;
		m_iSelectedOption[p][PO_APPEAR]		= po.GetFirstAppearance()+1;
		m_iSelectedOption[p][PO_TURN]		= po.m_Turn;
		m_iSelectedOption[p][PO_SCROLL]		= po.m_fReverseScroll==1 ? 1 : 0 ;


		m_iSelectedOption[p][PO_HOLD_NOTES]	= po.m_bHoldNotes ? 1 : 0;
		m_iSelectedOption[p][PO_DARK]		= po.m_fDark ? 1 : 0;

		if( GAMESTATE->m_pCurCourse )	// playing a course
		{
			if( GAMESTATE->m_bDifficultCourses )
				m_iSelectedOption[p][PO_STEP] = 1;
			else
				m_iSelectedOption[p][PO_STEP] = 0;
		}
		else
		{
			vector<Notes*> vNotes;
			GAMESTATE->m_pCurSong->GetNotes( vNotes, GAMESTATE->GetCurrentStyleDef()->m_NotesType );
			SortNotesArrayByDifficulty( vNotes );
			for( unsigned i=0; i<vNotes.size(); i++ )
			{
				if( GAMESTATE->m_pCurNotes[p] == vNotes[i] )
					m_iSelectedOption[p][PO_STEP] = i;
			}
		}
	}
}

void ScreenPlayerOptions::ExportOptions()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		PlayerOptions &po = GAMESTATE->m_PlayerOptions[p];
		po.Init();

		switch( m_iSelectedOption[p][PO_SPEED] )
		{
		case 0:	po.m_bTimeSpacing = false;	po.m_fScrollSpeed = 0.25f;	break;
		case 1:	po.m_bTimeSpacing = false;	po.m_fScrollSpeed = 0.5f;	break;
		case 2:	po.m_bTimeSpacing = false;	po.m_fScrollSpeed = 0.75f;	break;
		case 3:	po.m_bTimeSpacing = false;	po.m_fScrollSpeed = 1.0f;	break;
		case 4:	po.m_bTimeSpacing = false;	po.m_fScrollSpeed = 1.5f;	break;
		case 5:	po.m_bTimeSpacing = false;	po.m_fScrollSpeed = 2.0f;	break;
		case 6:	po.m_bTimeSpacing = false;	po.m_fScrollSpeed = 3.0f;	break;
		case 7:	po.m_bTimeSpacing = false;	po.m_fScrollSpeed = 5.0f;	break;
		case 8:	po.m_bTimeSpacing = false;	po.m_fScrollSpeed = 8.0f;	break;
		case 9: po.m_bTimeSpacing = true;	po.m_fScrollBPM = 200;		break;
		case 10:po.m_bTimeSpacing = true;	po.m_fScrollBPM = 300;		break;
		default:	ASSERT(0);
		}

		if( m_iSelectedOption[p][PO_ACCEL] != 0 )
			po.SetOneAccel( (PlayerOptions::Accel)(m_iSelectedOption[p][PO_ACCEL]-1) );
		if( m_iSelectedOption[p][PO_APPEAR] != 0 )
			po.SetOneAppearance( (PlayerOptions::Appearance)(m_iSelectedOption[p][PO_APPEAR]-1) );

		po.m_Turn			= (PlayerOptions::Turn)m_iSelectedOption[p][PO_TURN];
		po.m_fReverseScroll	= (m_iSelectedOption[p][PO_SCROLL] == 1) ? 1.f : 0.f;


		po.m_bHoldNotes			= (m_iSelectedOption[p][PO_HOLD_NOTES] == 1);
		po.m_fDark				= (m_iSelectedOption[p][PO_DARK] == 1) ? 1.f : 0.f;

		//
		// apply difficulty selection
		//
		if( GAMESTATE->m_pCurCourse )	// playing a course
		{
			if( m_iSelectedOption[p][PO_STEP] == 1 )
				GAMESTATE->m_bDifficultCourses = true;
			else
				GAMESTATE->m_bDifficultCourses = false;
		}
		else
		{
			vector<Notes*> vNotes;
			GAMESTATE->m_pCurSong->GetNotes( vNotes, GAMESTATE->GetCurrentStyleDef()->m_NotesType );
			SortNotesArrayByDifficulty( vNotes );
			GAMESTATE->m_pCurNotes[p] = vNotes[ m_iSelectedOption[p][PO_STEP] ];
		}
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
	else
		SCREENMAN->SetNewScreen( NEXT_SCREEN(GAMESTATE->m_PlayMode) );
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