#include "global.h"
#include "ScreenGameplaySyncMachine.h"
#include "NotesLoaderSM.h"
#include "GameState.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "AdjustSync.h"
#include "ScreenDimensions.h"
#include "InputEventPlus.h"
#include "SongUtil.h"

REGISTER_SCREEN_CLASS( ScreenGameplaySyncMachine );

void ScreenGameplaySyncMachine::Init()
{
	GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );
	GAMESTATE->m_pCurStyle.Set( GAMEMAN->GetHowToPlayStyleForGame(GAMESTATE->m_pCurGame) );
	AdjustSync::ResetOriginalSyncData();
	
	SMLoader ld;
	RString sFile = THEME->GetPathO("ScreenGameplaySyncMachine","music.sm");
	ld.LoadFromSMFile( sFile, m_Song );
	m_Song.SetSongDir( Dirname(sFile) );
	m_Song.TidyUpData();

	GAMESTATE->m_pCurSong.Set( &m_Song );
	Steps *pSteps = SongUtil::GetOneSteps( &m_Song );
	ASSERT( pSteps );
	GAMESTATE->m_pCurSteps[0].Set( pSteps );

	PREFSMAN->m_AutoPlay.Set( PC_HUMAN );

	ScreenGameplayNormal::Init( false );

	SO_GROUP_ASSIGN( GAMESTATE->m_SongOptions, ModsLevel_Stage, m_AutosyncType, SongOptions::AUTOSYNC_MACHINE );

	ClearMessageQueue();	// remove all of the messages set in ScreenGameplay that animate "ready", "here we go", etc.

	GAMESTATE->m_bGameplayLeadIn.Set( false );

	m_DancingState = STATE_DANCING;

	m_textSyncInfo.SetName( "SyncInfo" );
	m_textSyncInfo.LoadFromFont( THEME->GetPathF("Common","normal") );
	ActorUtil::LoadAllCommands( m_textSyncInfo, m_sName );
	this->AddChild( &m_textSyncInfo );

	this->SubscribeToMessage( Message_AutosyncChanged );

	RefreshText();
}

void ScreenGameplaySyncMachine::Update( float fDelta )
{
	ScreenGameplayNormal::Update( fDelta );
	RefreshText();
}

void ScreenGameplaySyncMachine::Input( const InputEventPlus &input )
{
	// Hack to make this work from Player2's controls
	InputEventPlus _input = input;

	if( _input.GameI.controller != GAME_CONTROLLER_INVALID )
		_input.GameI.controller = GAME_CONTROLLER_1;
	if( _input.pn != PLAYER_INVALID )
		_input.pn = PLAYER_1;

	ScreenGameplay::Input( _input );
}

void ScreenGameplaySyncMachine::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_NotesEnded )
	{
		ResetAndRestartCurrentSong();
		return;	// handled
	}
	
	ScreenGameplayNormal::HandleScreenMessage( SM );

	if( SM == SM_GoToPrevScreen || SM == SM_GoToNextScreen )
	{
		GAMESTATE->m_PlayMode.Set( PLAY_MODE_INVALID );
		GAMESTATE->m_pCurStyle.Set( NULL );
		GAMESTATE->m_pCurSong.Set( NULL );
	}
}

void ScreenGameplaySyncMachine::ResetAndRestartCurrentSong()
{
	m_pSoundMusic->Stop();
	ReloadCurrentSong();
	StartPlayingSong( 4, 0 );

	// reset autosync
	AdjustSync::s_iAutosyncOffsetSample = 0;
}

static LocalizedString OLD_OFFSET	( "ScreenGameplaySyncMachine", "Old offset" );
static LocalizedString NEW_OFFSET	( "ScreenGameplaySyncMachine", "New offset" );
static LocalizedString COLLECTING_SAMPLE( "ScreenGameplaySyncMachine", "Collecting sample" );
static LocalizedString STANDARD_DEVIATION( "ScreenGameplaySyncMachine", "Standard deviation" );
void ScreenGameplaySyncMachine::RefreshText()
{
	float fNew = PREFSMAN->m_fGlobalOffsetSeconds;
	float fOld = AdjustSync::s_fGlobalOffsetSecondsOriginal;
	float fStdDev = AdjustSync::s_fStandardDeviation;
	RString s;
	s += OLD_OFFSET.GetValue() + ssprintf( ": %0.3f\n", fOld );
	s += NEW_OFFSET.GetValue() + ssprintf( ": %0.3f\n", fNew );
	s += STANDARD_DEVIATION.GetValue() + ssprintf( ": %0.3f\n", fStdDev );
	s += COLLECTING_SAMPLE.GetValue() + ssprintf( ": %d / %d", AdjustSync::s_iAutosyncOffsetSample+1, SAMPLE_COUNT );

	m_textSyncInfo.SetText( s );
}


/*
 * (c) 2001-2004 Chris Danford
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
