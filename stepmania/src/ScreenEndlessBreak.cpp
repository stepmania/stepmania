#include "global.h"

/*
-----------------------------------------------------------------------------
 Class: ScreenEndlessBreak

 Desc: See header

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Kevin Slaughter
-----------------------------------------------------------------------------
*/


//TODO:: Add scripting support
#include "RageLog.h"
#include "ScreenManager.h"
#include "ScreenEndlessBreak.h"

ScreenEndlessBreak::ScreenEndlessBreak( CString sName ) : Screen( sName )
{
	LOG->Trace("ScreenEndlessBreak()");
	ASSERT(GAMESTATE->GetNumPlayersEnabled() > 0);	// This should never happen.. but just in case
	PostScreenMessage( SM_BreakInitiated, 0 );

	bool	m_bPicLoaded = false;	// I don't know of a better way to see if we failed to load a break picture. If someone does, please do it.

	if( (int)PREFSMAN->m_ShowDancingCharacters != 0 )
	{
		if( GAMESTATE->GetNumPlayersEnabled() == 1 )
			if( GAMESTATE->m_pCurCharacters[0] != NULL )
				m_sprBreakPicture.LoadTABreakFromCharacter( GAMESTATE->m_pCurCharacters[0] );
			else
				m_sprBreakPicture.Load( THEME->GetPathToG("Common fallback takingabreak") );
		else if( GAMESTATE->GetNumPlayersEnabled() > 1 ) // More than 1 player is present.
		{
			int PlayerToUse = 999;	/* If this was 0 by default, the first player would
										always be selected. Make it an insane number so
										we always generate a random player. */
			while (!GAMESTATE->IsPlayerEnabled(PlayerToUse))
			{
				PlayerToUse = (int)(rand()*NUM_PLAYERS);	// Is there a danger of this becoming an endless loop??
				if( (GAMESTATE->IsPlayerEnabled(PlayerToUse)) && (GAMESTATE->m_pCurCharacters[PlayerToUse] != NULL) )
				{
					m_sprBreakPicture.LoadTABreakFromCharacter( GAMESTATE->m_pCurCharacters[PlayerToUse] );
					break;
				}
			}
		}
	}
	else	// Characters not enabled.
		m_sprBreakPicture.Load( THEME->GetPathToG("Common fallback takingabreak") );
	m_sprBreakPicture.SetX( CENTER_X );
	m_sprBreakPicture.SetY( CENTER_Y );
		this->AddChild(&m_sprBreakPicture);

	// Set up our countdown clock.
	m_fCountdownSecs = (float)(PREFSMAN->m_iEndlessBreakLength*60);	// Stored in minutes.

	//BitmapText stuff
	m_textTimeRemaining.LoadFromFont( THEME->GetPathToF("Common Normal") );
	m_textTimeRemaining.SetText( SecondsToMMSSMsMs(m_fCountdownSecs) );
	m_textTimeRemaining.SetX( CENTER_X );
	m_textTimeRemaining.SetY( CENTER_Y );

	//Transition stuff
	m_In.Load( THEME->GetPathToB("ScreenEndlessBreak In") );
		this->AddChild(&m_In);
	m_In.StartTransitioning();
	m_Out.Load( THEME->GetPathToB("ScreenEndlessBreak Out") );
		this->AddChild(&m_Out);
	m_bExiting = false;
}

void ScreenEndlessBreak::Update(float fDeltaTime)
{
	m_textTimeRemaining.SetText( SecondsToMMSSMsMs(m_fCountdownSecs) );
	if( !m_bExiting )
		if(m_fCountdownSecs <= 0)
		{		
			m_bExiting = true;
			SCREENMAN->PopTopScreen(SM_BreakCompleted);	// Destroy this screen and let ScreenGameplay get the message.
			m_Out.StartTransitioning();
		}
		else
			//m_fCountdownSecs--;
			m_fCountdownSecs = (m_fCountdownSecs - fDeltaTime);
		Screen::Update( fDeltaTime );
}

void ScreenEndlessBreak::DrawPrimitives()
{
	Screen::DrawPrimitives();	// Draw all other elements below our TakingABreak pic.
	m_sprBreakPicture.Draw();
	m_textTimeRemaining.Draw();	// Draw the time remaining on top of everything.
}

void ScreenEndlessBreak::Input(const DeviceInput &DeviceI, const InputEventType type, const GameInput *GameI, const MenuInput &MenuI, const StyleInput &StyleI)
{
	//Why is this not working..?
	switch( MenuI.button )
	{
		case MENU_BUTTON_START: m_fCountdownSecs = 0; break;
	}
	//Compiler bitching over this..
	//Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}
