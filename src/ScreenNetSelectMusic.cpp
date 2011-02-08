#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenNetSelectMusic.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "CodeDetector.h"
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

AutoScreenMessage( SM_NoSongs )
AutoScreenMessage( SM_ChangeSong )
AutoScreenMessage( SM_SMOnlinePack )
AutoScreenMessage( SM_SetWheelSong )
AutoScreenMessage( SM_RefreshWheelLocation )
AutoScreenMessage( SM_SongChanged )
AutoScreenMessage( SM_UsersUpdate )
AutoScreenMessage( SM_BackFromPlayerOptions )

REGISTER_SCREEN_CLASS( ScreenNetSelectMusic );

void ScreenNetSelectMusic::Init()
{
	// Finish any previous stage. It's OK to call this when we haven't played a stage yet.
	GAMESTATE->FinishStage();

	ScreenNetSelectBase::Init();

	SAMPLE_MUSIC_PREVIEW_MODE.Load( m_sName, "SampleMusicPreviewMode" );
	CODES.Load( m_sName, "Codes" );

	FOREACH_EnabledPlayer (p)
	{
		/*
		m_DifficultyIcon[p].SetName( ssprintf("DifficultyIconP%d",p+1) );
		m_DifficultyIcon[p].Load( THEME->GetPathG( "ScreenSelectMusic",
												   ssprintf("difficulty icons 1x%d",
															NUM_Difficulty)) );
		LOAD_ALL_COMMANDS_AND_SET_XY( m_DifficultyIcon[p] );
		this->AddChild( &m_DifficultyIcon[p] );
		ON_COMMAND( m_DifficultyIcon[p] );
		*/
		m_Difficulty[p] = GAMESTATE->m_PreferredDifficulty[p];

		m_StepsDisplays[p].SetName( ssprintf("StepsDisplayP%d",p+1) );
		m_StepsDisplays[p].Load( "StepsDisplayNet", NULL );
		LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( m_StepsDisplays[p] );
		this->AddChild( &m_StepsDisplays[p] );
	}

	m_MusicWheel.SetName( "MusicWheel" );
	m_MusicWheel.Load( "OnlineMusicWheel" );
	LOAD_ALL_COMMANDS_AND_SET_XY( m_MusicWheel );
	m_MusicWheel.BeginScreen();
	ON_COMMAND( m_MusicWheel );
	this->AddChild( &m_MusicWheel );
	this->MoveToHead( &m_MusicWheel );

	// todo: handle me theme-side -aj
	m_BPMDisplay.SetName( "BPMDisplay" );
	m_BPMDisplay.LoadFromFont( THEME->GetPathF("BPMDisplay","bpm") );
	m_BPMDisplay.Load();
	LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( m_BPMDisplay );
	this->AddChild( &m_BPMDisplay );

	// todo: handle me theme-side -aj
	FOREACH_EnabledPlayer( p )
	{
		m_ModIconRow[p].SetName( ssprintf("ModIconsP%d",p+1) );
		m_ModIconRow[p].Load( "ModIconRowSelectMusic", p );
		m_ModIconRow[p].SetFromGameState();
		LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND( m_ModIconRow[p] );
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

	ZERO( m_iSelection );
}

void ScreenNetSelectMusic::Input( const InputEventPlus &input )
{
	if( !m_bAllowInput || IsTransitioning() )
		return;

	if( input.type == IET_RELEASE )
	{
		m_MusicWheel.Move(0);
		return;
	}

	if( input.type != IET_FIRST_PRESS && input.type != IET_REPEAT )
		return;

	bool bHoldingCtrl = 
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL)) ||
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL)) ||
		(!NSMAN->useSMserver); //If we are disconnected, assume no chatting

	wchar_t c = INPUTMAN->DeviceInputToChar(input.DeviceI,false);
	MakeUpper( &c, 1 );

	// Ctrl+[A-Z] to go to that letter of the alphabet
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
	}

	// todo: handle input here instead of in Menu* functions -aj
	if( input.MenuI == GAME_BUTTON_SELECT && input.type != IET_REPEAT )
	{
		NSMAN->ReportNSSOnOff(3);
		GAMESTATE->m_EditMode = EditMode_Full;
		SCREENMAN->AddNewScreenToTop( "ScreenPlayerOptions", SM_BackFromPlayerOptions );
	}

	// handle CodeDetector
	if( input.type == IET_FIRST_PRESS && DetectCodes(input) )
		return;

	ScreenNetSelectBase::Input( input );
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
		/* First check to see if this song is already selected. This is so that
		 * if you have multiple copies of the "same" song, you can choose which
		 * copy to play. */
		Song* CurSong = m_MusicWheel.GetSelectedSong();

		if(CurSong != NULL )

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
			case 2:	//Proper starting of song
			case 3:	//Blind starting of song
				StartSelectedSong();
				goto done;
			}
		}

		vector <Song *> AllSongs = SONGMAN->GetAllSongs();
		unsigned i;
		for( i=0; i < AllSongs.size(); i++ )
		{
			m_cSong = AllSongs[i];
			if( ( !m_cSong->GetTranslitArtist().CompareNoCase( NSMAN->m_sArtist ) ) &&
					( !m_cSong->GetTranslitMainTitle().CompareNoCase( NSMAN->m_sMainTitle ) ) &&
					( !m_cSong->GetTranslitSubTitle().CompareNoCase( NSMAN->m_sSubTitle ) ) )
					break;
		}

		bool haveSong = i != AllSongs.size();

		switch (NSMAN->m_iSelectMode)
		{
		case 3:
			StartSelectedSong();
			break;
		case 2: //We need to do cmd 1 as well here
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
		case 1:	//Scroll to song as well
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
			//don't break here
		case 0:	//See if client has song
			if(haveSong)
				NSMAN->m_iSelectMode = 0;
			else
				NSMAN->m_iSelectMode = 1;
			NSMAN->SelectUserSong();
		}
	}
	else if( SM == SM_SetWheelSong ) // After we're done the sort on wheel, select song.
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
	else if( SM == SM_GainFocus )
	{
		CodeDetector::RefreshCacheItems( CODES );
	}
	else if( SM == SM_LoseFocus )
	{
		CodeDetector::RefreshCacheItems(); // reset for other screens
	}

done:
	// Must be at end, as so it is last resort for SMOnline packets.
	// If it doesn't know what to do, then it'll just remove them.
	ScreenNetSelectBase::HandleScreenMessage( SM );
}

void ScreenNetSelectMusic::ChangeSteps( const InputEventPlus &input, int dir )
{
	if( GAMESTATE->m_pCurSong == NULL )
		return;

	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
	m_vpSteps = GAMESTATE->m_pCurSong->GetStepsByStepsType( st );

	if( m_vpSteps.size() == 0 )
		m_Difficulty[input.pn] = NUM_Difficulty;
	else
	{
		m_iSelection[input.pn] += dir;
		wrap( m_iSelection[input.pn], m_vpSteps.size() );

		Steps *pSteps = m_vpSteps[m_iSelection[input.pn]];
		m_Difficulty[input.pn] = (Difficulty)pSteps->GetDifficulty();
		GAMESTATE->ChangePreferredDifficultyAndStepsType( input.pn, m_Difficulty[input.pn], st );
	}

	UpdateDifficulty( input.pn );
}

bool ScreenNetSelectMusic::DetectCodes( const InputEventPlus &input )
{
	if( CodeDetector::EnteredPrevSteps(input.GameI.controller) )
	{
		ChangeSteps( input, -1 );
	}
	else if( CodeDetector::EnteredNextSteps(input.GameI.controller) )
	{
		ChangeSteps( input, +1 );
	}
	else if( CodeDetector::EnteredModeMenu(input.GameI.controller) )
	{
		m_MusicWheel.ChangeSort( SORT_MODE_MENU );
	}
	// todo: support NextGroup/PrevGroup
	else if( CodeDetector::EnteredCloseFolder(input.GameI.controller) )
	{
		RString sCurSection = m_MusicWheel.GetSelectedSection();
		m_MusicWheel.SelectSection(sCurSection);
		m_MusicWheel.SetOpenSection("");
		MusicChanged();
	}
	else
	{
		return false;
	}
	return true;
}

void ScreenNetSelectMusic::MenuLeft( const InputEventPlus &input )
{
	MESSAGEMAN->Broadcast( "PreviousSong" );
	m_MusicWheel.Move( -1 );
}

void ScreenNetSelectMusic::MenuRight( const InputEventPlus &input )
{
	MESSAGEMAN->Broadcast( "NextSong" );
	m_MusicWheel.Move( +1 );
}

void ScreenNetSelectMusic::MenuStart( const InputEventPlus &input )
{
	bool bResult = m_MusicWheel.Select();

	if( !bResult )
		return;

	if( m_MusicWheel.GetSelectedType() != TYPE_SONG )
		return;

	Song * pSong = m_MusicWheel.GetSelectedSong();

	if( pSong == NULL )
		return;

	GAMESTATE->m_pCurSong.Set( pSong );

	if( NSMAN->useSMserver )
	{
		NSMAN->m_sArtist = pSong->GetTranslitArtist();
		NSMAN->m_sMainTitle = pSong->GetTranslitMainTitle();
		NSMAN->m_sSubTitle = pSong->GetTranslitSubTitle();
		NSMAN->m_iSelectMode = 2; // Command for user selecting song
		NSMAN->SelectUserSong ();
	}
	else
		StartSelectedSong();
}

void ScreenNetSelectMusic::MenuBack( const InputEventPlus &input )
{
	SOUND->StopMusic();
	TweenOffScreen();

	Cancel( SM_GoToPrevScreen );
}

void ScreenNetSelectMusic::TweenOffScreen()
{
	ScreenNetSelectBase::TweenOffScreen();

	OFF_COMMAND( m_MusicWheel );

	OFF_COMMAND( m_BPMDisplay );

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
	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType; //StepsType_dance_single;
	FOREACH_EnabledPlayer (pn)
	{
		GAMESTATE->m_PreferredDifficulty[pn].Set( m_Difficulty[pn] );
		Steps *pSteps = SongUtil::GetStepsByDifficulty(pSong, st, m_Difficulty[pn]);
		GAMESTATE->m_pCurSteps[pn].Set( pSteps );
	}

	GAMESTATE->m_PreferredSortOrder = GAMESTATE->m_SortOrder;
	GAMESTATE->m_pPreferredSong = pSong;

	// force event mode
	GAMESTATE->m_bTemporaryEventMode = true;

	TweenOffScreen();
	StartTransitioningScreen( SM_GoToNextScreen );
}

void ScreenNetSelectMusic::UpdateDifficulty( PlayerNumber pn )
{
	if( GAMESTATE->m_pCurSong == NULL )
	{
		m_StepsDisplays[pn].SetFromStepsTypeAndMeterAndDifficultyAndCourseType( StepsType_Invalid, 0, Difficulty_Beginner, CourseType_Invalid );
		return;
	}

	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
	Steps* pSteps = SongUtil::GetStepsByDifficulty( GAMESTATE->m_pCurSong, st, m_Difficulty[pn] );
	GAMESTATE->m_pCurSteps[pn].Set( pSteps );

	if( ( m_Difficulty[pn] < NUM_Difficulty ) && ( m_Difficulty[pn] >= Difficulty_Beginner ) )
		m_StepsDisplays[pn].SetFromSteps( pSteps );
	else
		m_StepsDisplays[pn].SetFromStepsTypeAndMeterAndDifficultyAndCourseType( StepsType_Invalid, 0, Difficulty_Beginner, CourseType_Invalid );
}

void ScreenNetSelectMusic::MusicChanged()
{
	if( GAMESTATE->m_pCurSong == NULL )
	{
		m_BPMDisplay.NoBPM();
		FOREACH_EnabledPlayer (pn)
			UpdateDifficulty( pn );

		SOUND->StopMusic();
		// todo: handle playing section music correctly. -aj
		// SOUND->PlayMusic( m_sSectionMusicPath, NULL, true, 0, -1 );
		return;
	} 
	m_BPMDisplay.SetBpmFromSong( GAMESTATE->m_pCurSong );

	SongUtil::GetPlayableSteps( GAMESTATE->m_pCurSong, m_vpSteps );

	FOREACH_EnabledPlayer(pn)
	{
		// xxx: using the old difficulty reset code -aj
		m_Difficulty[pn] = GAMESTATE->m_PreferredDifficulty[pn];
		StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
		vector<Steps*> MultiSteps;
		MultiSteps = GAMESTATE->m_pCurSong->GetStepsByStepsType( st );
		if(MultiSteps.size() == 0)
			m_Difficulty[pn] = NUM_Difficulty;
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
					if( i >= m_Difficulty[pn] )
					{
						m_Difficulty[pn] = (Difficulty)i;
						break;
					}
				}

			if( i == NUM_Difficulty )
				m_Difficulty[pn] = Target;
		}
		UpdateDifficulty( pn );
	}

	// Copied from ScreenSelectMusic
	// TODO: Update me! -aj
	SOUND->StopMusic();
	if( GAMESTATE->m_pCurSong->HasMusic() )
	{
		if(SOUND->GetMusicPath().CompareNoCase(GAMESTATE->m_pCurSong->GetMusicPath())) // dont play the same sound over and over
		{
			SOUND->StopMusic();
			SOUND->PlayMusic(
				GAMESTATE->m_pCurSong->GetMusicPath(), 
				NULL,
				true,
				GAMESTATE->m_pCurSong->m_fMusicSampleStartSeconds,
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
