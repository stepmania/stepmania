#include "global.h"
#include "ScreenDemonstration.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "SongManager.h"
#include "StepMania.h"
#include "ScreenAttract.h"	// for AttractInput()
#include "ScreenManager.h"
#include "RageSoundManager.h"
#include "RageSounds.h"


#define SECONDS_TO_SHOW			THEME->GetMetricF("ScreenDemonstration","SecondsToShow")
#define NEXT_SCREEN				THEME->GetMetric("ScreenDemonstration","NextScreen")

const ScreenMessage	SM_NotesEnded				= ScreenMessage(SM_User+10);	// MUST be same as in ScreenGameplay


bool PrepareForDemonstration()		// always return true.
{
	switch( GAMESTATE->m_CurGame )
	{
	case GAME_DANCE:	GAMESTATE->m_CurStyle = STYLE_DANCE_VERSUS;			break; 
	case GAME_PUMP:		GAMESTATE->m_CurStyle = STYLE_PUMP_VERSUS;			break; 
	case GAME_EZ2:		GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_VERSUS;	break; 
	case GAME_PARA:		GAMESTATE->m_CurStyle = STYLE_PARA_SINGLE;			break; 
	case GAME_DS3DDX:	GAMESTATE->m_CurStyle = STYLE_DS3DDX_SINGLE;		break;
	case GAME_BM:		GAMESTATE->m_CurStyle = STYLE_BM_SINGLE;			break;
	case GAME_MANIAX:	GAMESTATE->m_CurStyle = STYLE_MANIAX_SINGLE;		break;
	case GAME_TECHNO:	GAMESTATE->m_CurStyle = STYLE_TECHNO_VERSUS8;
	case GAME_PNM:		GAMESTATE->m_CurStyle = STYLE_PNM_NINE;				break;
	default:	ASSERT(0);
	}

	GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;

	/* If needed, turn sound off.  We need to do this before the ScreenGameplay ctor,
	 * since changes to sound volume aren't guaranteed to take effect if done *after*
	 * the sound starts playing. */
	if( !GAMESTATE->IsTimeToPlayAttractSounds() )
		SOUNDMAN->SetPrefs( 0 );	// silent

	return true;
}

ScreenDemonstration::ScreenDemonstration( CString sName ) : ScreenJukebox( sName, PrepareForDemonstration() )	// this is a hack to get some code to execute before the ScreenGameplay constructor
{
	LOG->Trace( "ScreenDemonstration::ScreenDemonstration()" );

	if( GAMESTATE->m_pCurSong == NULL )	// we didn't find a song.
	{
		HandleScreenMessage( SM_GoToNextScreen );	// Abort demonstration.
		return;
	}


	m_Overlay.LoadFromAniDir( THEME->GetPathToB("ScreenDemonstration overlay") );
	this->AddChild( &m_Overlay );

	this->MoveToTail( &m_In );
	this->MoveToTail( &m_Out );

	ClearMessageQueue();	// remove all of the messages set in ScreenGameplay that drive "ready", "go", etc.

	GAMESTATE->m_bPastHereWeGo = true;

	m_DancingState = STATE_DANCING;
	this->PostScreenMessage( SM_BeginFadingOut, SECONDS_TO_SHOW );	
}

void ScreenDemonstration::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_NotesEnded:
	case SM_BeginFadingOut:
		if(!m_Out.IsTransitioning())
			m_Out.StartTransitioning( SM_GoToNextScreen );
		return;

	case SM_GainFocus:
		if( !GAMESTATE->IsTimeToPlayAttractSounds() )
			SOUNDMAN->SetPrefs( 0 );	// silent
		break;

	case SM_LoseFocus:
		SOUNDMAN->SetPrefs( PREFSMAN->m_fSoundVolume );	// turn volume back on
		break;

	case SM_GoToNextScreen:
		m_soundMusic.Stop();
		GAMESTATE->Reset();
		SOUNDMAN->SetPrefs( PREFSMAN->m_fSoundVolume );	// turn volume back on
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		return;
	}

	ScreenGameplay::HandleScreenMessage( SM );
}

/*
 * (c) 2003-2004 Chris Danford
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
