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
	PO_DARK,
	PO_PERSPECTIVE,
	NUM_PLAYER_OPTIONS_LINES
};
OptionRow g_PlayerOptionsLines[NUM_PLAYER_OPTIONS_LINES] = {
	OptionRow( "Speed",				"x0.25","x0.5","x0.75","x1","x1.5","x2","x3","x5","x8","C200","C300" ),	
	OptionRow( "Acceler\n-ation",	"OFF","BOOST","BRAKE","WAVE","EXPAND","BOOMERANG" ),	
	OptionRow( "Effect",			"OFF","DRUNK","DIZZY","MINI","FLIP","TORNADO" ),	
	OptionRow( "Appear\n-ance",		"VISIBLE","HIDDEN","SUDDEN","STEALTH","BLINK", "R.VANISH" ),	
	OptionRow( "Turn",				"OFF","MIRROR","LEFT","RIGHT","SHUFFLE","S.SHUFFLE" ),	
	OptionRow( "Trans\n-form",		"OFF","LITTLE","WIDE","BIG","QUICK","SKIPPY" ),	
	OptionRow( "Scroll",			"STANDARD","REVERSE" ),	
	OptionRow( "Note\nSkin",		"" ),	
	OptionRow( "Holds",				"OFF","ON" ),	
	OptionRow( "Dark",				"OFF","ON" ),	
	OptionRow( "Perspec\n-tive",	"" ),
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

	/* If we're going to "press start for more options" or skipping options
	 * entirely, we need a different fade out. XXX: this is a hack */
	if(PREFSMAN->m_ShowSongOptions == PrefsManager::NO)
		m_Menu.m_Out.Load( THEME->GetPathToB("ScreenPlayerOptions direct out") ); /* direct to stage */
	else if(PREFSMAN->m_ShowSongOptions == PrefsManager::ASK)
		m_Menu.m_Out.Load( THEME->GetPathToB("ScreenPlayerOptions option out") ); /* optional song options */

	m_sprOptionsMessage.Load( THEME->GetPathToG("ScreenPlayerOptions options") );
	m_sprOptionsMessage.StopAnimating();
	m_sprOptionsMessage.SetXY( CENTER_X, CENTER_Y );
	m_sprOptionsMessage.SetZoom( 1 );
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
	//this->AddChild( &m_sprOptionsMessage );       // we have to draw this manually over the top of transitions

	m_bAcceptedChoices = false;
	m_bGoToOptions = ( PREFSMAN->m_ShowSongOptions == PrefsManager::YES );

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

	unsigned i;
	for( i=0; i<arraySkinNames.size(); i++ )
	{
		arraySkinNames[i].MakeUpper();
		m_OptionRow[PO_NOTE_SKIN].choices.push_back( arraySkinNames[i] ); 
	}

	m_OptionRow[PO_PERSPECTIVE].choices.clear();
	m_OptionRow[PO_PERSPECTIVE].choices.push_back( "INCOMING" ); 
	m_OptionRow[PO_PERSPECTIVE].choices.push_back( "OVERHEAD" ); 
	m_OptionRow[PO_PERSPECTIVE].choices.push_back( "SPACE" ); 

	CStringArray arrayPosNames;
	GAMESTATE->m_pPosition->GetNamesForCurrentGame(arrayPosNames);
	for( i=0; i<arrayPosNames.size(); i++ )
	{
		arrayPosNames[i].MakeUpper();
		m_OptionRow[PO_PERSPECTIVE].choices.push_back( arrayPosNames[i] ); 
	}


	for( int p=0; p<NUM_PLAYERS; p++ )
	{
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
		m_iSelectedOption[p][PO_EFFECT]		= po.GetFirstEffect()+1;
		m_iSelectedOption[p][PO_APPEAR]		= po.GetFirstAppearance()+1;
		m_iSelectedOption[p][PO_TURN]		= po.m_Turn;
		m_iSelectedOption[p][PO_TRANSFORM]	= po.m_Transform;
		m_iSelectedOption[p][PO_SCROLL]		= po.m_fReverseScroll==1 ? 1 : 0 ;


		// highlight currently selected skin
		m_iSelectedOption[p][PO_NOTE_SKIN] = -1;
		for( unsigned j=0; j<m_OptionRow[PO_NOTE_SKIN].choices.size(); j++ )
		{
			if( 0==stricmp(m_OptionRow[PO_NOTE_SKIN].choices[j], po.m_sNoteSkin) )
			{
				m_iSelectedOption[p][PO_NOTE_SKIN] = j;
				break;
			}
		}
		if( m_iSelectedOption[p][PO_NOTE_SKIN] == -1 )
			m_iSelectedOption[p][PO_NOTE_SKIN] = 0;


		m_iSelectedOption[p][PO_HOLD_NOTES]	= po.m_bHoldNotes ? 1 : 0;
		m_iSelectedOption[p][PO_DARK]		= po.m_fDark ? 1 : 0;

		/* Default: */
		m_iSelectedOption[p][PO_PERSPECTIVE] = 1;
		if(po.m_fPerspectiveTilt == -1)
			m_iSelectedOption[p][PO_PERSPECTIVE] = 0;
		else if(po.m_fPerspectiveTilt == 1)
			m_iSelectedOption[p][PO_PERSPECTIVE] = 2;
		else /* po.m_fPerspectiveTilt == 0 */
		{
			vector<CString> &choices = m_OptionRow[PO_PERSPECTIVE].choices;
			for(unsigned n = 3; n < choices.size(); ++n)
				if(!choices[n].CompareNoCase(GAMESTATE->m_PlayerOptions[p].m_sPositioning))
					m_iSelectedOption[p][PO_PERSPECTIVE] = n;
		}

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
		if( m_iSelectedOption[p][PO_EFFECT] != 0 )
			po.SetOneEffect( (PlayerOptions::Effect)(m_iSelectedOption[p][PO_EFFECT]-1) );
		if( m_iSelectedOption[p][PO_APPEAR] != 0 )
			po.SetOneAppearance( (PlayerOptions::Appearance)(m_iSelectedOption[p][PO_APPEAR]-1) );

		po.m_Turn			= (PlayerOptions::Turn)m_iSelectedOption[p][PO_TURN];
		po.m_Transform		= (PlayerOptions::Transform)m_iSelectedOption[p][PO_TRANSFORM];
		po.m_fReverseScroll	= (m_iSelectedOption[p][PO_SCROLL] == 1) ? 1.f : 0.f;


		int iSelectedSkin = m_iSelectedOption[p][PO_NOTE_SKIN];
		CString sNewSkin = m_OptionRow[PO_NOTE_SKIN].choices[iSelectedSkin];
		po.m_sNoteSkin = sNewSkin;


		po.m_bHoldNotes			= (m_iSelectedOption[p][PO_HOLD_NOTES] == 1);
		po.m_fDark				= (m_iSelectedOption[p][PO_DARK] == 1) ? 1.f : 0.f;
		
		switch(m_iSelectedOption[p][PO_PERSPECTIVE])
		{
		case 0: po.m_fPerspectiveTilt = -1; break;
		case 2: po.m_fPerspectiveTilt =  1; break;
		default:po.m_fPerspectiveTilt =  0; break;
		}
		if(m_iSelectedOption[p][PO_PERSPECTIVE] > 2)
		{
			const int choice = m_iSelectedOption[p][PO_PERSPECTIVE];
			GAMESTATE->m_PlayerOptions[p].m_sPositioning = m_OptionRow[PO_PERSPECTIVE].choices[choice];
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
	else if( m_bGoToOptions )
		SCREENMAN->SetNewScreen( NEXT_SCREEN(GAMESTATE->m_PlayMode) );
	else
		SCREENMAN->SetNewScreen( ScreenSongOptions::GetNextScreen() );
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
	if( !GAMESTATE->m_bEditing &&
		type == IET_FIRST_PRESS  &&
		!m_Menu.m_In.IsTransitioning()  &&
		MenuI.IsValid()  &&
		MenuI.button == MENU_BUTTON_START  &&
		PREFSMAN->m_ShowSongOptions == PrefsManager::ASK )
	{
		if( m_bAcceptedChoices  &&  !m_bGoToOptions )
		{
			m_bGoToOptions = true;
			m_sprOptionsMessage.SetState( 1 );
			SOUNDMAN->PlayOnce( THEME->GetPathToS("Common start") );
		}
	}

	ScreenOptions::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenPlayerOptions::HandleScreenMessage( const ScreenMessage SM )
{
	if(PREFSMAN->m_ShowSongOptions == PrefsManager::ASK)
	switch( SM )
	{
	case SM_BeginFadingOut: // when the user accepts the page of options
	{
		m_bAcceptedChoices = true;

		float fShowSeconds = m_Menu.m_Out.GetLengthSeconds();

		// show "hold START for options"
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
		m_sprOptionsMessage.BeginTweening( 0.15f );     // fade in
		m_sprOptionsMessage.SetZoomY( 1 );
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,1) );
		m_sprOptionsMessage.BeginTweening( fShowSeconds-0.3f ); // sleep
		m_sprOptionsMessage.BeginTweening( 0.15f );     // fade out
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
		m_sprOptionsMessage.SetZoomY( 0 );
	}
		break;
	}
	ScreenOptions::HandleScreenMessage( SM );
}