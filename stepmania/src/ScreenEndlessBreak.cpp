#include "global.h"
#include "RageLog.h"
#include "ScreenManager.h"
#include "ScreenEndlessBreak.h"

//TODO:: Add scripting support

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
			PlayerNumber pn;
			do
			{
				pn = (PlayerNumber)(rand()%NUM_PLAYERS);
				if( GAMESTATE->IsPlayerEnabled(pn) && (GAMESTATE->m_pCurCharacters[pn] != NULL) )
				{
					m_sprBreakPicture.LoadTABreakFromCharacter( GAMESTATE->m_pCurCharacters[pn] );
					break;
				}
			}
			while( !GAMESTATE->IsPlayerEnabled(pn) );
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

/*
 * (c) 2001-2003 Kevin Slaughter
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

