#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenNetSelectMusic.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "Style.h"
#include "Steps.h"
#include "RageTimer.h"
#include "ActorUtil.h"
#include "AnnouncerManager.h"
#include "MenuTimer.h"
#include "NetworkSyncManager.h"
#include "StepsUtil.h"
#include "RageUtil.h"
#include "MusicWheel.h"
#include "InputMapper.h"
#include "RageLog.h"
#include "Song.h"
#include "InputEventPlus.h"
#include "SongUtil.h"
#include "RageInput.h"
#include "SongManager.h"
#include "CodeDetector.h"

AutoScreenMessage( SM_NoSongs );
AutoScreenMessage( SM_ChangeSong );
AutoScreenMessage( SM_SMOnlinePack );
AutoScreenMessage( SM_SetWheelSong );
AutoScreenMessage( SM_RefreshWheelLocation );
AutoScreenMessage( SM_SongChanged );
AutoScreenMessage( SM_UsersUpdate );
AutoScreenMessage( SM_BackFromPlayerOptions );

REGISTER_SCREEN_CLASS( ScreenNetSelectMusic );

void ScreenNetSelectMusic::Init()
{
	ScreenNetSelectBase::Init();

	SAMPLE_MUSIC_PREVIEW_MODE.Load( m_sName, "SampleMusicPreviewMode" );
	MUSIC_WHEEL_TYPE.Load( m_sName, "MusicWheelType" );
	PLAYER_OPTIONS_SCREEN.Load( m_sName, "PlayerOptionsScreen" );

	FOREACH_EnabledPlayer (p)
	{
		m_DC[p] = GAMESTATE->m_PreferredDifficulty[p];

		m_StepsDisplays[p].SetName( ssprintf("StepsDisplayP%d",p+1) );
		m_StepsDisplays[p].Load( "StepsDisplayNet", nullptr );
		LOAD_ALL_COMMANDS_AND_SET_XY( m_StepsDisplays[p] );
		this->AddChild( &m_StepsDisplays[p] );
	}

	m_MusicWheel.SetName( "MusicWheel" );
	m_MusicWheel.Load( MUSIC_WHEEL_TYPE );
	LOAD_ALL_COMMANDS_AND_SET_XY( m_MusicWheel );
	m_MusicWheel.BeginScreen();
	ON_COMMAND( m_MusicWheel );
	this->AddChild( &m_MusicWheel );
	this->MoveToHead( &m_MusicWheel );

	// todo: handle me theme-side -aj
	FOREACH_EnabledPlayer( p )
	{
		m_ModIconRow[p].SetName( ssprintf("ModIconsP%d",p+1) );
		m_ModIconRow[p].Load( "ModIconRowSelectMusic", p );
		m_ModIconRow[p].SetFromGameState();
		LOAD_ALL_COMMANDS_AND_SET_XY( m_ModIconRow[p] );
		this->AddChild( &m_ModIconRow[p] );
	}

	// Load SFX and music
	m_soundChangeOpt.Load( THEME->GetPathS(m_sName,"change opt") );
	m_soundChangeSel.Load( THEME->GetPathS(m_sName,"change sel") );
	m_sSectionMusicPath =	THEME->GetPathS(m_sName,"section music");
	m_sRouletteMusicPath =	THEME->GetPathS(m_sName,"roulette music");
	m_sRandomMusicPath =	THEME->GetPathS(m_sName,"random music");

	NSMAN->ReportNSSOnOff(1);
	NSMAN->ReportPlayerOptions();

	m_bInitialSelect = false;
	m_bAllowInput = false;
}

bool ScreenNetSelectMusic::Input( const InputEventPlus &input )
{
	if( !m_bAllowInput || IsTransitioning() )
		return false;

	if( input.type == IET_RELEASE )
	{
		m_MusicWheel.Move(0);
		return true;
	}

	if( input.type != IET_FIRST_PRESS && input.type != IET_REPEAT )
		return false;

	bool bHoldingCtrl = 
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL)) ||
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL)) ||
		(!NSMAN->useSMserver); // If we are disconnected, assume no chatting

	wchar_t c = INPUTMAN->DeviceInputToChar(input.DeviceI,false);
	MakeUpper( &c, 1 );

	// Ctrl+[A-Z] to go to that letter of the alphabet
	bool handled = false;
	if( bHoldingCtrl && ( c >= 'A' ) && ( c <= 'Z' ) )
	{
		SortOrder so = GAMESTATE->m_SortOrder;
		if( ( so != SORT_TITLE ) && ( so != SORT_ARTIST ) )
		{
			so = SORT_TITLE;

			GAMESTATE->m_PreferredSortOrder = so;
			GAMESTATE->m_SortOrder.Set( so );
			// Odd, changing the sort order requires us to call SetOpenSection more than once
			m_MusicWheel.ChangeSort( so );
			m_MusicWheel.SetOpenSection( ssprintf("%c", c ) );
		}
		m_MusicWheel.SelectSection( ssprintf("%c", c ) );
		m_MusicWheel.ChangeSort( so );
		m_MusicWheel.SetOpenSection( ssprintf("%c", c ) );
		m_MusicWheel.Move(+1);
		handled = true;
	}

	return ScreenNetSelectBase::Input( input ) || handled;
}

void ScreenNetSelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToPrevScreen )
	{
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "PrevScreen") );
	}
	else if( SM == SM_GoToNextScreen )
	{
		SOUND->StopMusic();
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "NextScreen") );
	}
	else if( SM == SM_UsersUpdate )
	{
		m_MusicWheel.Move( 0 );
	}
	else if( SM == SM_NoSongs )
	{
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "NoSongsScreen") );
	}
	else if( SM == SM_ChangeSong )
	{
		// First check to see if this song is already selected. This is so that if
		// you have multiple copies of the "same" song you can chose which copy to play.
		Song* CurSong = m_MusicWheel.GetSelectedSong();

		if(CurSong != nullptr )

			if( ( !CurSong->GetTranslitArtist().CompareNoCase( NSMAN->m_sArtist ) ) &&
					( !CurSong->GetTranslitMainTitle().CompareNoCase( NSMAN->m_sMainTitle ) ) &&
					( !CurSong->GetTranslitSubTitle().CompareNoCase( NSMAN->m_sSubTitle ) ) )
		{
			switch ( NSMAN->m_iSelectMode )
			{
			case 0:
			case 1:
				NSMAN->m_iSelectMode = 0;
				NSMAN->SelectUserSong();
				break;
			case 2:	// Proper starting of song
			case 3:	// Blind starting of song
				StartSelectedSong();
				goto done;
			}
		}

		vector <Song *> AllSongs = SONGMAN->GetAllSongs();
		unsigned i;
		bool found = false;
		if (NSMAN->GetServerVersion() >= 129)
		{
			//Dont earch by filehash if none was sent
			if(!NSMAN->m_sFileHash.empty())
				for (i = 0; i < AllSongs.size(); i++)
				{
					m_cSong = AllSongs[i];
					if (NSMAN->m_sArtist == m_cSong->GetTranslitArtist() &&
						NSMAN->m_sMainTitle == m_cSong->GetTranslitMainTitle() &&
						NSMAN->m_sSubTitle == m_cSong->GetTranslitSubTitle() &&
						NSMAN->m_sFileHash == m_cSong->GetFileHash())
					{
						found = true;
						break;
					}
				}

		}
		//If we couldnt find it using file hash search for it without using it, if using SMSERVER < 129 it will go here
		if(!found)
			for (i = 0; i < AllSongs.size(); i++)
			{
				m_cSong = AllSongs[i];
				if (NSMAN->m_sArtist == m_cSong->GetTranslitArtist() &&
					NSMAN->m_sMainTitle == m_cSong->GetTranslitMainTitle() &&
					NSMAN->m_sSubTitle == m_cSong->GetTranslitSubTitle())
				{
					break;
				}
			}

		bool haveSong = i != AllSongs.size();

		switch (NSMAN->m_iSelectMode)
		{
		case 3:
			StartSelectedSong();
			break;
		case 2: // We need to do cmd 1 as well here
			if(haveSong)
			{
				if(!m_MusicWheel.SelectSong( m_cSong ) )
				{
					m_MusicWheel.ChangeSort( SORT_GROUP );
					m_MusicWheel.FinishTweening();
					SCREENMAN->PostMessageToTopScreen( SM_SetWheelSong, 0.710f );
				}
				m_MusicWheel.Select();
				m_MusicWheel.Move(-1);
				m_MusicWheel.Move(1);
				StartSelectedSong();
				m_MusicWheel.Select();
			}
			break;
		case 1:	// Scroll to song as well
			if(haveSong)
			{
				if(!m_MusicWheel.SelectSong( m_cSong ) )
				{
					//m_MusicWheel.ChangeSort( SORT_GROUP );
					//m_MusicWheel.FinishTweening();
					//SCREENMAN->PostMessageToTopScreen( SM_SetWheelSong, 0.710f );
					m_MusicWheel.ChangeSort( SORT_GROUP );
					m_MusicWheel.SetOpenSection( "" );
				}
				m_MusicWheel.SelectSong( m_cSong );
				m_MusicWheel.Select();
				m_MusicWheel.Move(-1);
				m_MusicWheel.Move(1);
				m_MusicWheel.Select();
			}
			// don't break here
		case 0:	// See if client has song
			if(haveSong)
				NSMAN->m_iSelectMode = 0;
			else
				NSMAN->m_iSelectMode = 1;
			NSMAN->SelectUserSong();
		}
	}
	else if( SM == SM_SetWheelSong ) // After we've done the sort on wheel, select song.
	{
		m_MusicWheel.SelectSong( m_cSong );
	}
	else if( SM == SM_RefreshWheelLocation )
	{
		m_MusicWheel.Select();
		m_MusicWheel.Move(-1);
		m_MusicWheel.Move(1);
		m_MusicWheel.Select();
		m_bAllowInput = true;
	}
	else if( SM == SM_BackFromPlayerOptions )
	{
		// XXX HACK: This will cause ScreenSelectOptions to go back here.
		NSMAN->ReportNSSOnOff(1);
		GAMESTATE->m_EditMode = EditMode_Invalid;
		NSMAN->ReportPlayerOptions();

		// Update changes
		FOREACH_EnabledPlayer(p)
			m_ModIconRow[p].SetFromGameState();
	}
	else if( SM == SM_SongChanged )
	{
		GAMESTATE->m_pCurSong.Set( m_MusicWheel.GetSelectedSong() );
		MusicChanged();
	}
	else if( SM == SM_SMOnlinePack )
	{
		if( NSMAN->m_SMOnlinePacket.Read1() == 1 )
		{
			switch ( NSMAN->m_SMOnlinePacket.Read1() )
			{
			case 0: // Room title Change
				{
					RString titleSub;
					titleSub = NSMAN->m_SMOnlinePacket.ReadNT() + "\n";
					titleSub += NSMAN->m_SMOnlinePacket.ReadNT();
					if( NSMAN->m_SMOnlinePacket.Read1() != 1 )
					{
						RString SMOnlineSelectScreen = THEME->GetMetric( m_sName, "RoomSelectScreen" );
						SCREENMAN->SetNewScreen( SMOnlineSelectScreen );
					}
				}
			}
		}
	}

done:
	// Must be at end, as so it is last resort for SMOnline packets.
	// If it doesn't know what to do, then it'll just remove them.
	ScreenNetSelectBase::HandleScreenMessage( SM );
}

bool ScreenNetSelectMusic::LeftAndRightPressed( const PlayerNumber pn )
{
	return INPUTMAPPER->IsBeingPressed( GAME_BUTTON_LEFT, pn )
		&& INPUTMAPPER->IsBeingPressed( GAME_BUTTON_RIGHT, pn );
}

bool ScreenNetSelectMusic::MenuLeft( const InputEventPlus &input )
{
	PlayerNumber pn = input.pn;

	if( LeftAndRightPressed(pn) )
		m_MusicWheel.ChangeSort( SORT_MODE_MENU );
	else
		m_MusicWheel.Move( -1 );
	return true;
}

bool ScreenNetSelectMusic::MenuRight( const InputEventPlus &input )
{
	PlayerNumber pn = input.pn;

	if( LeftAndRightPressed(pn) )
		m_MusicWheel.ChangeSort( SORT_MODE_MENU );
	else
		m_MusicWheel.Move( +1 );
	return true;
}

bool ScreenNetSelectMusic::MenuUp( const InputEventPlus &input )
{
	NSMAN->ReportNSSOnOff(3);
	GAMESTATE->m_EditMode = EditMode_Full;
	SCREENMAN->AddNewScreenToTop( PLAYER_OPTIONS_SCREEN, SM_BackFromPlayerOptions );
	return true;
}

bool ScreenNetSelectMusic::MenuDown( const InputEventPlus &input )
{
	/* Tricky: If we have a player on player 2, and there is only player 2,
	 * allow them to use player 1's controls to change their difficulty. */
	/* Why?  Nothing else allows that. (-who?) */
	// I agree, that's a stupid idea -aj

	// Funny story:  If the arrow keys are mapped to Player 2, but the person
	// is playing as Player 1, then hitting down to change the difficulty will
	// crash in UpdateDifficulties.  So pretend the input came from the player
	// that is enabled. -Kyz

	PlayerNumber pn = input.pn;
	if(!GAMESTATE->IsPlayerEnabled(pn))
	{
		if(pn == PLAYER_1)
		{
			pn= PLAYER_2;
		}
		else
		{
			pn= PLAYER_1;
		}
	}

	if( GAMESTATE->m_pCurSong == nullptr )
		return false;
	StepsType st = GAMESTATE->GetCurrentStyle(pn)->m_StepsType;
	vector <Steps *> MultiSteps;
	MultiSteps = GAMESTATE->m_pCurSong->GetStepsByStepsType( st );
	if(MultiSteps.size() == 0)
		m_DC[pn] = NUM_Difficulty;
	else
	{
		int i;

		bool dcs[NUM_Difficulty];

		for( i=0; i<NUM_Difficulty; ++i )
			dcs[i] = false;

		for( i=0; i<(int)MultiSteps.size(); ++i )
			dcs[MultiSteps[i]->GetDifficulty()] = true;

		for( i=0; i<NUM_Difficulty; ++i )
		{
			if( (dcs[i]) && (i > m_DC[pn]) )
			{
				m_DC[pn] = (Difficulty)i;
				break;
			}
		}
		// If failed to go up, loop
		if( i == NUM_Difficulty )
		{
			for(i = 0;i<NUM_Difficulty;i++)
			{
				if(dcs[i])
				{
					m_DC[pn] = (Difficulty)i;
					break;
				}
			}
		}

	}
	UpdateDifficulties( pn );
	GAMESTATE->m_PreferredDifficulty[pn].Set( m_DC[pn] );
	return true;
}

bool ScreenNetSelectMusic::MenuStart(const InputEventPlus &input)
{
	return SelectCurrent();
}
bool ScreenNetSelectMusic::SelectCurrent()
{

	bool bResult = m_MusicWheel.Select();

	if (!bResult)
		return true;

	if (m_MusicWheel.GetSelectedType() != WheelItemDataType_Song)
		return true;

	Song * pSong = m_MusicWheel.GetSelectedSong();

	if (pSong == nullptr)
		return false;

	GAMESTATE->m_pCurSong.Set(pSong);

	if (NSMAN->useSMserver)
	{
		NSMAN->m_sArtist = pSong->GetTranslitArtist();
		NSMAN->m_sMainTitle = pSong->GetTranslitMainTitle();
		NSMAN->m_sSubTitle = pSong->GetTranslitSubTitle();
		NSMAN->m_iSelectMode = 2; // Command for user selecting song
		NSMAN->SelectUserSong();
	}
	else
		StartSelectedSong();
	return true;
}
bool ScreenNetSelectMusic::MenuBack( const InputEventPlus &input )
{
	SOUND->StopMusic();
	TweenOffScreen();

	Cancel( SM_GoToPrevScreen );
	return true;
}

void ScreenNetSelectMusic::TweenOffScreen()
{
	ScreenNetSelectBase::TweenOffScreen();

	OFF_COMMAND( m_MusicWheel );

	FOREACH_EnabledPlayer (pn)
	{
		OFF_COMMAND( m_StepsDisplays[pn] );
		//OFF_COMMAND( m_DifficultyIcon[pn] );
		OFF_COMMAND( m_ModIconRow[pn] );
	}

	OFF_COMMAND( m_MusicWheel );

	NSMAN->ReportNSSOnOff(0);
}

void ScreenNetSelectMusic::StartSelectedSong()
{
	Song * pSong = m_MusicWheel.GetSelectedSong();
	GAMESTATE->m_pCurSong.Set( pSong );
	FOREACH_EnabledPlayer (pn)
	{
		StepsType st = GAMESTATE->GetCurrentStyle(pn)->m_StepsType; //StepsType_dance_single;
		GAMESTATE->m_PreferredDifficulty[pn].Set( m_DC[pn] );
		Steps *pSteps = SongUtil::GetStepsByDifficulty(pSong, st, m_DC[pn]);
		GAMESTATE->m_pCurSteps[pn].Set( pSteps );
	}

	GAMESTATE->m_PreferredSortOrder = GAMESTATE->m_SortOrder;
	GAMESTATE->m_pPreferredSong = pSong;

	// force event mode
	GAMESTATE->m_bTemporaryEventMode = true;

	TweenOffScreen();
	StartTransitioningScreen( SM_GoToNextScreen );
}

void ScreenNetSelectMusic::UpdateDifficulties( PlayerNumber pn )
{
	if( GAMESTATE->m_pCurSong == nullptr )
	{
		m_StepsDisplays[pn].SetFromStepsTypeAndMeterAndDifficultyAndCourseType( StepsType_Invalid, 0, Difficulty_Beginner, CourseType_Invalid ); 
		//m_DifficultyIcon[pn].SetFromSteps( pn, nullptr );	// It will blank it out 
		return;
	}

	StepsType st = GAMESTATE->GetCurrentStyle(pn)->m_StepsType;

	Steps * pSteps = SongUtil::GetStepsByDifficulty( GAMESTATE->m_pCurSong, st, m_DC[pn] );
	GAMESTATE->m_pCurSteps[pn].Set( pSteps );

	if( ( m_DC[pn] < NUM_Difficulty ) && ( m_DC[pn] >= Difficulty_Beginner ) )
		m_StepsDisplays[pn].SetFromSteps( pSteps );
	else
		m_StepsDisplays[pn].SetFromStepsTypeAndMeterAndDifficultyAndCourseType( StepsType_Invalid, 0, Difficulty_Beginner, CourseType_Invalid );
}

void ScreenNetSelectMusic::MusicChanged()
{
	if( GAMESTATE->m_pCurSong == nullptr )
	{
		FOREACH_EnabledPlayer (pn)
			UpdateDifficulties( pn );

		SOUND->StopMusic();
		// todo: handle playing section music correctly. -aj
		// SOUND->PlayMusic( m_sSectionMusicPath, nullptr, true, 0, -1 );
		return;
	} 

	FOREACH_EnabledPlayer (pn)
	{
		m_DC[pn] = GAMESTATE->m_PreferredDifficulty[pn];
		StepsType st = GAMESTATE->GetCurrentStyle(pn)->m_StepsType;
		vector <Steps *> MultiSteps;
		MultiSteps = GAMESTATE->m_pCurSong->GetStepsByStepsType( st );
		if(MultiSteps.size() == 0)
			m_DC[pn] = NUM_Difficulty;
		else
		{
			int i;
			Difficulty Target = Difficulty_Easy;

			bool dcs[NUM_Difficulty];

			for( i=0; i<NUM_Difficulty; ++i )
				dcs[i] = false;

			for( i=0; i<(int)MultiSteps.size(); ++i )
				dcs[MultiSteps[i]->GetDifficulty()] = true;

			for( i=0; i<NUM_Difficulty; ++i )
				if( dcs[i] )
				{
					Target = (Difficulty)i;
					if( i >= m_DC[pn] )
					{
						m_DC[pn] = (Difficulty)i;
						break;
					}
				}

			if( i == NUM_Difficulty )
				m_DC[pn] = Target;
		}
		UpdateDifficulties( pn );
	}

	// Copied from ScreenSelectMusic
	// TODO: Update me! -aj
	SOUND->StopMusic();
	if( GAMESTATE->m_pCurSong->HasMusic() )
	{
		// don't play the same sound over and over
		if(SOUND->GetMusicPath().CompareNoCase(GAMESTATE->m_pCurSong->GetMusicPath()))
		{
			SOUND->StopMusic();
			SOUND->PlayMusic(
				GAMESTATE->m_pCurSong->GetPreviewMusicPath(),
				nullptr,
				true,
				GAMESTATE->m_pCurSong->GetPreviewStartSeconds(),
				GAMESTATE->m_pCurSong->m_fMusicSampleLengthSeconds );
		}
	}
}

void ScreenNetSelectMusic::Update( float fDeltaTime )
{
	if(!m_bInitialSelect)
	{
		m_bInitialSelect = true;
		SCREENMAN->PostMessageToTopScreen( SM_RefreshWheelLocation, 1.0f );
	}
	ScreenNetSelectBase::Update( fDeltaTime );
}

MusicWheel* ScreenNetSelectMusic::GetMusicWheel()
{
	return &m_MusicWheel;
}


// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the PlayerState. */
class LunaScreenNetSelectMusic : public Luna<ScreenNetSelectMusic>
{
public:
	static int GetMusicWheel(T* p, lua_State *L) {
		p->GetMusicWheel()->PushSelf(L);
		return 1;
	}
	static int SelectCurrent(T* p, lua_State *L) {
		p->SelectCurrent();
		return 1;
	}
	LunaScreenNetSelectMusic()
	{
		ADD_METHOD(GetMusicWheel);
		ADD_METHOD(SelectCurrent);
	}
};

LUA_REGISTER_DERIVED_CLASS(ScreenNetSelectMusic, ScreenNetSelectBase)
// lua end

#endif

/*
 * (c) 2004-2005 Charles Lohr
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
