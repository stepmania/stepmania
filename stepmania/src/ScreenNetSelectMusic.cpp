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
#include "Actor.h"
#include "AnnouncerManager.h"
#include "MenuTimer.h"
#include "NetworkSyncManager.h"
#include "StepsUtil.h"
#include "RageUtil.h"
#include "MusicWheel.h"
#include "InputMapper.h"
#include "RageLog.h"

const ScreenMessage SM_NoSongs		= ScreenMessage(SM_User+3);
const ScreenMessage SM_ChangeSong	= ScreenMessage(SM_User+5);
const ScreenMessage SM_BackFromOpts	= ScreenMessage(SM_User+6);
const ScreenMessage SM_SMOnlinePack	= ScreenMessage(SM_User+8);	//Unused, but should be known
const ScreenMessage	SM_SetWheelSong = ScreenMessage(SM_User+19);
const ScreenMessage SM_RefreshWheelLocation = ScreenMessage(SM_User+20);

const CString AllGroups			= "[ALL MUSIC]";

#define DIFFBG_WIDTH				THEME->GetMetricF(m_sName,"DiffBGWidth")
#define DIFFBG_HEIGHT				THEME->GetMetricF(m_sName,"DiffBGHeight")

REGISTER_SCREEN_CLASS( ScreenNetSelectMusic );
ScreenNetSelectMusic::ScreenNetSelectMusic( const CString& sName ) : ScreenNetSelectBase( sName )
{
	/* Finish any previous stage.  It's OK to call this when we havn't played a stage yet. */
	GAMESTATE->FinishStage();
}

void ScreenNetSelectMusic::Init()
{
	ScreenNetSelectBase::Init();

	//Diff Icon background
	m_sprDiff.Load( THEME->GetPathG( m_sName, "DiffBG" ) );
	m_sprDiff.SetName( "DiffBG" );
	m_sprDiff.SetWidth( DIFFBG_WIDTH );
	m_sprDiff.SetHeight( DIFFBG_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_sprDiff );
	this->AddChild( &m_sprDiff);

	FOREACH_EnabledPlayer (p)
	{
		m_DifficultyIcon[p].SetName( ssprintf("DifficultyIconP%d",p+1) );
		m_DifficultyIcon[p].Load( THEME->GetPathG( "ScreenSelectMusic",
												   ssprintf("difficulty icons 1x%d",
															NUM_DIFFICULTIES)) );
		SET_XY( m_DifficultyIcon[p] );
		this->AddChild( &m_DifficultyIcon[p] );
		ON_COMMAND( m_DifficultyIcon[p] );
		m_DC[p] = GAMESTATE->m_PreferredDifficulty[p];

		m_DifficultyMeters[p].SetName( ssprintf("MeterP%d",p+1) );
		m_DifficultyMeters[p].Load( "DifficultyMeter" );
		SET_XY_AND_ON_COMMAND( m_DifficultyMeters[p] );
		this->AddChild( &m_DifficultyMeters[p] );
	}

	m_MusicWheel.Load();
	m_MusicWheel.SetName( "MusicWheel", "Wheel" );
	SET_XY( m_MusicWheel );
	m_MusicWheel.TweenOnScreen();
	this->AddChild( &m_MusicWheel );
	this->MoveToHead( &m_MusicWheel );
	ON_COMMAND( m_MusicWheel );	

	m_BPMDisplay.SetName( "BPMDisplay" );
	m_BPMDisplay.Load();
	SET_XY_AND_ON_COMMAND( m_BPMDisplay );
	this->AddChild( &m_BPMDisplay );

	FOREACH_EnabledPlayer( p )
	{
		m_OptionIconRow[p].SetName( ssprintf("OptionIconsP%d",p+1) );
		m_OptionIconRow[p].Load( p );
		m_OptionIconRow[p].Refresh();
		SET_XY_AND_ON_COMMAND( m_OptionIconRow[p] );
		this->AddChild( &m_OptionIconRow[p] );
	}

	//Load SFX next
	m_soundChangeOpt.Load( THEME->GetPathS(m_sName,"change opt") );
	m_soundChangeSel.Load( THEME->GetPathS(m_sName,"change sel") );

	NSMAN->ReportNSSOnOff(1);
	NSMAN->ReportPlayerOptions();

	m_bInitialSelect = false;
	m_bAllowInput = false;
}

void ScreenNetSelectMusic::Input( const DeviceInput& DeviceI, const InputEventType type,
								  const GameInput& GameI, const MenuInput& MenuI,
								  const StyleInput& StyleI )
{	
	if( !m_bAllowInput )
		return;

	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return;

	if ( type == IET_RELEASE )
	{
		m_MusicWheel.Move(0);
		return;
	}

	if( (type != IET_FIRST_PRESS) && (type != IET_SLOW_REPEAT) && (type != IET_FAST_REPEAT ) )
		return;

	bool bHoldingCtrl = 
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL)) ||
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL)) ||
		(!NSMAN->useSMserver);	//If we are disconnected, assume no chatting

	char c = (char) toupper(DeviceI.ToChar() );

	if ( bHoldingCtrl && ( c >= 'A' ) && ( c <= 'Z' ) )
	{
		SortOrder so = GAMESTATE->m_PreferredSortOrder;
		
		if ( ( so != SORT_TITLE ) && ( so != SORT_ARTIST ) )
			so = SORT_TITLE;

		GAMESTATE->m_PreferredSortOrder = so;
		m_MusicWheel.SetOpenGroup( ssprintf("%c", c ), so );
		m_MusicWheel.SelectSection( ssprintf("%c", c ) );
		m_MusicWheel.Move( +1 );
		//m_MusicWheel.Move( -1 );
	}

	ScreenNetSelectBase::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenNetSelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "PrevScreen") );
		break;
	case SM_GoToNextScreen:
		SOUND->StopMusic();
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "NextScreen") );
		break;
	case SM_NoSongs:
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "NoSongsScreen") );
		break;
	case SM_ChangeSong:
		{
			//First check to see if this song is already selected.
			//This is so that if you multiple copies of the "same" song
			//you can chose which copy to play.
			Song* CurSong = m_MusicWheel.GetSelectedSong();

			if (CurSong != NULL )

				if ( ( !CurSong->GetTranslitArtist().CompareNoCase( NSMAN->m_sArtist ) ) &&
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
					break;
				}
				break;
			}

			vector <Song *> AllSongs = SONGMAN->GetAllSongs();
			unsigned i;
			for( i=0; i < AllSongs.size(); i++ )
			{
				m_cSong = AllSongs[i];
				if ( ( !m_cSong->GetTranslitArtist().CompareNoCase( NSMAN->m_sArtist ) ) &&
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
				if (haveSong)
				{
					if (!m_MusicWheel.SelectSong( m_cSong ) )
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
				if (haveSong)
				{
					if (!m_MusicWheel.SelectSong( m_cSong ) )
					{
						//m_MusicWheel.ChangeSort( SORT_GROUP );
						//m_MusicWheel.FinishTweening();
						//SCREENMAN->PostMessageToTopScreen( SM_SetWheelSong, 0.710f );
						m_MusicWheel.SetOpenGroup( "", SORT_GROUP );
					}
					m_MusicWheel.SelectSong( m_cSong );
					m_MusicWheel.Select();
					m_MusicWheel.Move(-1);
					m_MusicWheel.Move(1);
					m_MusicWheel.Select();
				}
				//don't break here
			case 0:	//See if client has song
				if (haveSong)
					NSMAN->m_iSelectMode = 0;
				else
					NSMAN->m_iSelectMode = 1;
				NSMAN->SelectUserSong();
			}
		}
		break;
		//After we're done the sort on wheel, select song.
	case SM_SetWheelSong:
		m_MusicWheel.SelectSong( m_cSong );
	case SM_RefreshWheelLocation:
		m_MusicWheel.Select();
		m_MusicWheel.Move(-1);
		m_MusicWheel.Move(1);
		m_MusicWheel.Select();
		m_bAllowInput = true;
		break;
	case SM_BackFromOpts:
		//XXX: HACK: This will causes ScreenSelectOptions to go back here.
		NSMAN->ReportNSSOnOff(1);
		GAMESTATE->m_bEditing = false;
		NSMAN->ReportPlayerOptions();

		//Update changes
		FOREACH_EnabledPlayer(p)
		{
			m_OptionIconRow[p].Load( p );
			m_OptionIconRow[p].Refresh();
		}
		break;
	case SM_SongChanged:
		GAMESTATE->m_pCurSong = m_MusicWheel.GetSelectedSong();
		MusicChanged();
		break;
	case SM_SMOnlinePack:
		if ( NSMAN->m_SMOnlinePacket.Read1() == 1 )
			switch ( NSMAN->m_SMOnlinePacket.Read1() )
			{
			case 0: //Room title Change
				{
					CString titleSub;
					titleSub = NSMAN->m_SMOnlinePacket.ReadNT() + "\n";
					titleSub += NSMAN->m_SMOnlinePacket.ReadNT();
					if ( NSMAN->m_SMOnlinePacket.Read1() != 1 )
					{
						CString SMOnlineSelectScreen;
						THEME->GetMetric( m_sName, "RoomSelectScreen", SMOnlineSelectScreen );
						SCREENMAN->SetNewScreen( SMOnlineSelectScreen );
					}
				}
			}
		break;
	}
	//Must be at end, as so it is last resort for SMOnline packets.
	//If it doens't know what to do, then it'll just remove them.
	ScreenNetSelectBase::HandleScreenMessage( SM );
}

void ScreenNetSelectMusic::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	bool bLeftPressed = INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_LEFT) );
	bool bRightPressed = INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_RIGHT) );
	bool bLeftAndRightPressed = bLeftPressed && bRightPressed;

	if ( type == IET_FIRST_PRESS )
		if ( bLeftAndRightPressed )
			m_MusicWheel.ChangeSort( SORT_MODE_MENU );		
		else
			m_MusicWheel.Move( -1 );
}

void ScreenNetSelectMusic::MenuRight( PlayerNumber pn, const InputEventType type )
{
	bool bLeftPressed = INPUTMAPPER->IsButtonDown( MenuInput ( pn, MENU_BUTTON_LEFT ) );
	bool bRightPressed = INPUTMAPPER->IsButtonDown( MenuInput(  pn, MENU_BUTTON_RIGHT )) ;
	bool bLeftAndRightPressed = bLeftPressed && bRightPressed;

	if ( type == IET_FIRST_PRESS )
		if ( bLeftAndRightPressed )
			m_MusicWheel.ChangeSort( SORT_MODE_MENU );		
		else
			m_MusicWheel.Move( +1 );
}

void ScreenNetSelectMusic::MenuUp( PlayerNumber pn, const InputEventType type )
{
	NSMAN->ReportNSSOnOff(3);
	GAMESTATE->m_bEditing = true;
	SCREENMAN->AddNewScreenToTop( "ScreenPlayerOptions", SM_BackFromOpts );
}

void ScreenNetSelectMusic::MenuDown( PlayerNumber pn, const InputEventType type )
{
	if ( GAMESTATE->m_pCurSong == NULL )
		return;
	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
	vector <Steps *> MultiSteps;
	MultiSteps = GAMESTATE->m_pCurSong->GetStepsByStepsType( st );
	if (MultiSteps.size() == 0)
		m_DC[pn] = NUM_DIFFICULTIES;
	else
	{
		int i;

		bool dcs[NUM_DIFFICULTIES];

		for ( i=0; i<NUM_DIFFICULTIES; ++i )
			dcs[i] = false;

		for ( i=0; i<(int)MultiSteps.size(); ++i )
			dcs[MultiSteps[i]->GetDifficulty()] = true;

		for ( i=0; i<NUM_DIFFICULTIES; ++i )
		{
			if ( (dcs[i]) && (i > m_DC[pn]) )
			{
				m_DC[pn] = (Difficulty)i;
				break;
			}
		}
		//If failed to go up, loop
		if ( i == NUM_DIFFICULTIES )
			for (i = 0;i<NUM_DIFFICULTIES;i++)
			if (dcs[i])
			{
				m_DC[pn] = (Difficulty)i;
				break;
			}

	}
	UpdateDifficulties( pn );
	GAMESTATE->m_PreferredDifficulty[pn] = m_DC[pn];
}

void ScreenNetSelectMusic::MenuStart( PlayerNumber pn )
{
	bool bResult = m_MusicWheel.Select();

	if( !bResult )
		return;

	if ( m_MusicWheel.GetSelectedType() != TYPE_SONG )
		return;

	Song * pSong = m_MusicWheel.GetSelectedSong();

	if ( pSong == NULL )
		return;

	GAMESTATE->m_pCurSong = pSong;

	if ( NSMAN->useSMserver )
	{
		//int j = m_iSongNum % m_vSongs.size();
		NSMAN->m_sArtist = pSong->GetTranslitArtist();
		NSMAN->m_sMainTitle = pSong->GetTranslitMainTitle();
		NSMAN->m_sSubTitle = pSong->GetTranslitSubTitle();
		NSMAN->m_iSelectMode = 2; //Command for user selecting song
		NSMAN->SelectUserSong ();
	}
	else
		StartSelectedSong();
}

void ScreenNetSelectMusic::MenuBack( PlayerNumber pn )
{
	SOUND->StopMusic();
	TweenOffScreen();

	Back( SM_GoToPrevScreen );
}


void ScreenNetSelectMusic::TweenOffScreen()
{
	ScreenNetSelectBase::TweenOffScreen();

	OFF_COMMAND( m_MusicWheel );

	OFF_COMMAND( m_BPMDisplay );

	OFF_COMMAND( m_sprDiff );

	FOREACH_EnabledPlayer (pn)
	{
		OFF_COMMAND( m_DifficultyMeters[pn] );
		OFF_COMMAND( m_DifficultyIcon[pn] );
		OFF_COMMAND( m_OptionIconRow[pn] );
	}

	m_MusicWheel.TweenOffScreen();
	OFF_COMMAND( m_MusicWheel );

	NSMAN->ReportNSSOnOff(0);
}

void ScreenNetSelectMusic::StartSelectedSong()
{
	Song * pSong = m_MusicWheel.GetSelectedSong();
	GAMESTATE->m_pCurSong = pSong;
	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType; //STEPS_TYPE_DANCE_SINGLE;
	FOREACH_EnabledPlayer (pn)
	{
		GAMESTATE->m_PreferredDifficulty[pn] = m_DC[pn];
		Steps * pSteps = pSong->GetStepsByDifficulty(st,m_DC[pn]);
		GAMESTATE->m_pCurSteps[pn] = pSteps;
	}
	
	TweenOffScreen();
	StartTransitioning( SM_GoToNextScreen );
}

void ScreenNetSelectMusic::UpdateDifficulties( PlayerNumber pn )
{
	if ( GAMESTATE->m_pCurSong == NULL )
	{
		m_DifficultyMeters[pn].SetFromMeterAndDifficulty( 0, DIFFICULTY_BEGINNER ); 
		m_DifficultyIcon[pn].SetFromSteps( pn, NULL );	//It will blank it out 
		return;
	}
	if ( ( m_DC[pn] < DIFFICULTY_EDIT ) && ( m_DC[pn] >= DIFFICULTY_BEGINNER ) )
		m_DifficultyIcon[pn].SetFromDifficulty( pn, m_DC[pn] );
	else
		m_DifficultyIcon[pn].SetFromSteps( pn, NULL );	//It will blank it out 

	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;

	if ( ( m_DC[pn] < NUM_DIFFICULTIES ) && ( m_DC[pn] >= DIFFICULTY_BEGINNER ) )
		m_DifficultyMeters[pn].SetFromSteps( GAMESTATE->m_pCurSong->GetStepsByDifficulty( st, m_DC[pn] ) );
	else
		m_DifficultyMeters[pn].SetFromMeterAndDifficulty( 0, DIFFICULTY_BEGINNER ); 
}

void ScreenNetSelectMusic::MusicChanged()
{
	if ( GAMESTATE->m_pCurSong == NULL )
	{
		m_BPMDisplay.NoBPM();
		FOREACH_EnabledPlayer (pn)
			UpdateDifficulties( pn );
		return;
	} 
	m_BPMDisplay.SetBPM( GAMESTATE->m_pCurSong );

	FOREACH_EnabledPlayer (pn)
	{
		m_DC[pn] = GAMESTATE->m_PreferredDifficulty[pn];
		StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
		vector <Steps *> MultiSteps;
		MultiSteps = GAMESTATE->m_pCurSong->GetStepsByStepsType( st );
		if (MultiSteps.size() == 0)
			m_DC[pn] = NUM_DIFFICULTIES;
		else
		{
			int i;
			Difficulty Target = DIFFICULTY_EASY;

			bool dcs[NUM_DIFFICULTIES];

			for ( i=0; i<NUM_DIFFICULTIES; ++i )
				dcs[i] = false;

			for ( i=0; i<(int)MultiSteps.size(); ++i )
				dcs[MultiSteps[i]->GetDifficulty()] = true;

			for ( i=0; i<NUM_DIFFICULTIES; ++i )
				if ( dcs[i] )
				{
					Target = (Difficulty)i;
					if ( i >= m_DC[pn] )
					{
						m_DC[pn] = (Difficulty)i;
						break;
					}
				}

			if ( i == NUM_DIFFICULTIES )
				m_DC[pn] = Target;
		}
		UpdateDifficulties( pn );
	}
	//Copied from ScreenSelectMusic
	SOUND->StopMusic();
	if( GAMESTATE->m_pCurSong->HasMusic() )
	{
		if(SOUND->GetMusicPath().CompareNoCase(GAMESTATE->m_pCurSong->GetMusicPath())) // dont play the same sound over and over
		{
			
			SOUND->StopMusic();
			SOUND->PlayMusic(GAMESTATE->m_pCurSong->GetMusicPath(), true,
				GAMESTATE->m_pCurSong->m_fMusicSampleStartSeconds,
				GAMESTATE->m_pCurSong->m_fMusicSampleLengthSeconds);
		}
	}
}

void ScreenNetSelectMusic::Update( float fDeltaTime )
{
	if (!m_bInitialSelect)
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
