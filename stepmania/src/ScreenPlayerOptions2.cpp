#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenPlayerOptions2

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenPlayerOptions2.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "NoteSkinManager.h"
#include "NoteFieldPositioning.h"
#include "ScreenSongOptions.h"
#include "Character.h"

#define PREV_SCREEN( play_mode )		THEME->GetMetric ("ScreenPlayerOptions2","PrevScreen"+Capitalize(PlayModeToString(play_mode)))
#define NEXT_SCREEN( play_mode )		THEME->GetMetric ("ScreenPlayerOptions2","NextScreen"+Capitalize(PlayModeToString(play_mode)))

enum {
	PO_EFFECT,
	PO_TRANSFORM,
	PO_NOTE_SKIN,
	PO_PERSPECTIVE,
	PO_CHARACTER,
	NUM_PLAYER_OPTIONS2_LINES
};
OptionRow g_PlayerOptions2Lines[NUM_PLAYER_OPTIONS2_LINES] = {
	OptionRow( "Effect",			"OFF","DRUNK","DIZZY","MINI","FLIP","TORNADO" ),	
	OptionRow( "Trans\n-form",		"OFF","LITTLE","WIDE","BIG","QUICK","SKIPPY" ),	
	OptionRow( "Note\nSkin",		"" ),	
	OptionRow( "Perspec\n-tive",	"" ),
	OptionRow( "Character",			"" ),
};


ScreenPlayerOptions2::ScreenPlayerOptions2() :
	ScreenOptions("ScreenPlayerOptions2",true)
{
	LOG->Trace( "ScreenPlayerOptions2::ScreenPlayerOptions2()" );
	
	Init( 
		INPUTMODE_PLAYERS, 
		g_PlayerOptions2Lines, 
		NUM_PLAYER_OPTIONS2_LINES,
		true, false );

	/* If we're going to "press start for more options" or skipping options
	 * entirely, we need a different fade out. XXX: this is a hack */
	if(PREFSMAN->m_ShowSongOptions == PrefsManager::NO)
		m_Menu.m_Out.Load( THEME->GetPathToB("ScreenPlayerOptions2 direct out") ); /* direct to stage */
	else if(PREFSMAN->m_ShowSongOptions == PrefsManager::ASK)
		m_Menu.m_Out.Load( THEME->GetPathToB("ScreenPlayerOptions2 option out") ); /* optional song options */

	m_sprOptionsMessage.Load( THEME->GetPathToG("ScreenPlayerOptions2 options") );
	m_sprOptionsMessage.StopAnimating();
	m_sprOptionsMessage.SetXY( CENTER_X, CENTER_Y );
	m_sprOptionsMessage.SetZoom( 1 );
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
	//this->AddChild( &m_sprOptionsMessage );       // we have to draw this manually over the top of transitions

	m_bAcceptedChoices = false;
	m_bGoToOptions = ( PREFSMAN->m_ShowSongOptions == PrefsManager::YES );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("player options intro") );
}


void ScreenPlayerOptions2::ImportOptions()
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

	//
	// fill in perspective names
	//
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

	//
	// fill in character names
	//
	m_OptionRow[PO_CHARACTER].choices.clear();
	m_OptionRow[PO_CHARACTER].choices.push_back( "OFF" ); 
	for( i=0; i<GAMESTATE->m_pCharacters.size(); i++ )
	{
		CString s = GAMESTATE->m_pCharacters[i]->m_sName;
		s.MakeUpper();
		m_OptionRow[PO_CHARACTER].choices.push_back( s ); 
	}


	//
	// Init options
	//
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip
		
		PlayerOptions &po = GAMESTATE->m_PlayerOptions[p];
		po.Init();
		
		m_iSelectedOption[p][PO_EFFECT]		= po.GetFirstEffect()+1;
		m_iSelectedOption[p][PO_TRANSFORM]	= po.m_Transform;


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

		for( i=0; i<GAMESTATE->m_pCharacters.size(); i++ )
			if( GAMESTATE->m_pCurCharacters[p] == GAMESTATE->m_pCharacters[i] )
				m_iSelectedOption[p][PO_CHARACTER] = i+1;
	}
}

void ScreenPlayerOptions2::ExportOptions()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		PlayerOptions &po = GAMESTATE->m_PlayerOptions[p];
		po.Init();

		if( m_iSelectedOption[p][PO_EFFECT] != 0 )
			po.SetOneEffect( (PlayerOptions::Effect)(m_iSelectedOption[p][PO_EFFECT]-1) );

		po.m_Transform		= (PlayerOptions::Transform)m_iSelectedOption[p][PO_TRANSFORM];


		int iSelectedSkin = m_iSelectedOption[p][PO_NOTE_SKIN];
		CString sNewSkin = m_OptionRow[PO_NOTE_SKIN].choices[iSelectedSkin];
		po.m_sNoteSkin = sNewSkin;


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

		if(m_iSelectedOption[p][PO_PERSPECTIVE] > 2)
		{
			const int choice = m_iSelectedOption[p][PO_PERSPECTIVE];
			GAMESTATE->m_PlayerOptions[p].m_sPositioning = m_OptionRow[PO_PERSPECTIVE].choices[choice];
		}

		if( m_iSelectedOption[p][PO_CHARACTER] > 0 )
		{
			int choice = m_iSelectedOption[p][PO_CHARACTER] - 1;
			GAMESTATE->m_pCurCharacters[p] = GAMESTATE->m_pCharacters[choice];
		}
		else
			GAMESTATE->m_pCurCharacters[p] = NULL;
	}
}

void ScreenPlayerOptions2::GoToPrevState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else
		SCREENMAN->SetNewScreen( PREV_SCREEN(GAMESTATE->m_PlayMode) );
}

void ScreenPlayerOptions2::GoToNextState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else if( m_bGoToOptions )
		SCREENMAN->SetNewScreen( NEXT_SCREEN(GAMESTATE->m_PlayMode) );
	else
		SCREENMAN->SetNewScreen( ScreenSongOptions::GetNextScreen() );
}


void ScreenPlayerOptions2::Update( float fDelta )
{
	ScreenOptions::Update( fDelta );
	m_sprOptionsMessage.Update( fDelta );
}

void ScreenPlayerOptions2::DrawPrimitives()
{
	ScreenOptions::DrawPrimitives();
	m_sprOptionsMessage.Draw();
}


void ScreenPlayerOptions2::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
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

void ScreenPlayerOptions2::HandleScreenMessage( const ScreenMessage SM )
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