/* ScreenGameplayMultiplayer */

#ifndef ScreenGameplayMultiplayer_H
#define ScreenGameplayMultiplayer_H

#include "Screen.h"
#include "Background.h"
#include "Foreground.h"
#include "ScoreDisplay.h"
#include "Transition.h"
#include "Player.h"
#include "EnumHelper.h"
#include "AutoKeysounds.h"
#include "PlayerState.h"
#include "StageStats.h"

enum MultiPlayer {
	MPLAYER_1 = 0,
	MPLAYER_2,
	MPLAYER_3,
	MPLAYER_4,
	MPLAYER_5,
	MPLAYER_6,
	MPLAYER_7,
	MPLAYER_8,
	MPLAYER_9,
	MPLAYER_10,
	MPLAYER_11,
	MPLAYER_12,
	MPLAYER_13,
	MPLAYER_14,
	MPLAYER_15,
	MPLAYER_16,
	NUM_MULTI_PLAYERS,	// leave this at the end
	MULTI_PLAYER_INVALID
};
#define FOREACH_MultiPlayer( pn ) FOREACH_ENUM( MultiPlayer, NUM_MULTI_PLAYERS, pn )

class Song;
class Steps;

class ScreenGameplayMultiplayer : public Screen
{
public:
	ScreenGameplayMultiplayer( CString sName, bool bDemonstration = false );
	virtual void Init();
	virtual ~ScreenGameplayMultiplayer();

	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual bool UsesBackground() const { return false; }

protected:
	void TweenOursOnScreen();
	void TweenOursOffScreen();

	bool IsLastSong();
	void SetupSong( MultiPlayer p, int iSongIndex );
	void LoadNextSong();
	float StartPlayingSong(float MinTimeToNotes, float MinTimeToMusic);
	void ShowSavePrompt( ScreenMessage SM_SendWhenDone );

	void UpdateSongPosition( float fDeltaTime );
	void StageFinished( bool bBackedOut );

	vector<Song*>		m_vpSongsQueue;
	vector<Steps*>		m_vpStepsQueue;
	vector<AttackArray>	m_vModifiersQueue;

	Background			m_Background;
	Foreground			m_Foreground;

	ScoreDisplay*		m_pPrimaryScoreDisplay[NUM_MULTI_PLAYERS];
	ScoreKeeper*		m_pPrimaryScoreKeeper[NUM_MULTI_PLAYERS];

	Transition		m_In;
	Transition		m_Out;
	Transition		m_Cancel;

	Player				m_AutoPlayer;
	PlayerState			m_PlayerState[NUM_MULTI_PLAYERS];
	PlayerStageStats	m_PlayerStageStats[NUM_MULTI_PLAYERS];
	Player				m_HumanPlayer[NUM_MULTI_PLAYERS];

	AutoKeysounds	m_AutoKeysounds;
	RageSound		*m_pSoundMusic;
};


#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
