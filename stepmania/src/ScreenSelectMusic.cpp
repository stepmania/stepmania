#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenSelectMusic

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "ScreenSelectMusic.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "GameManager.h"
#include "RageSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "InputQueue.h"
#include "AnnouncerManager.h"
#include "InputMapper.h"
#include "GameState.h"
#include "CodeDetector.h"
#include <math.h>
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "Notes.h"
 
const int NUM_SCORE_DIGITS	=	9;

#define BANNER_FRAME_ON_COMMAND				THEME->GetMetric ("ScreenSelectMusic","BannerFrameOnCommand")
#define BANNER_FRAME_OFF_COMMAND			THEME->GetMetric ("ScreenSelectMusic","BannerFrameOffCommand")
#define BANNER_ON_COMMAND					THEME->GetMetric ("ScreenSelectMusic","BannerOnCommand")
#define BANNER_OFF_COMMAND					THEME->GetMetric ("ScreenSelectMusic","BannerOffCommand")
#define BANNER_WIDTH						THEME->GetMetricF("ScreenSelectMusic","BannerWidth")
#define BANNER_HEIGHT						THEME->GetMetricF("ScreenSelectMusic","BannerHeight")
#define BPM_ON_COMMAND						THEME->GetMetric ("ScreenSelectMusic","BPMOnCommand")
#define BPM_OFF_COMMAND						THEME->GetMetric ("ScreenSelectMusic","BPMOffCommand")
#define STAGE_ON_COMMAND					THEME->GetMetric ("ScreenSelectMusic","StageOnCommand")
#define STAGE_OFF_COMMAND					THEME->GetMetric ("ScreenSelectMusic","StageOffCommand")
#define CD_TITLE_ON_COMMAND					THEME->GetMetric ("ScreenSelectMusic","CDTitleOnCommand")
#define CD_TITLE_OFF_COMMAND				THEME->GetMetric ("ScreenSelectMusic","CDTitleOffCommand")
#define DIFFICULTY_FRAME_ON_COMMAND( p )	THEME->GetMetric ("ScreenSelectMusic",ssprintf("DifficultyFrameP%dOnCommand",p+1))
#define DIFFICULTY_FRAME_OFF_COMMAND( p )	THEME->GetMetric ("ScreenSelectMusic",ssprintf("DifficultyFrameP%dOffCommand",p+1))
#define DIFFICULTY_ICON_ON_COMMAND( p )		THEME->GetMetric ("ScreenSelectMusic",ssprintf("DifficultyIconP%dOnCommand",p+1))
#define DIFFICULTY_ICON_OFF_COMMAND( p )	THEME->GetMetric ("ScreenSelectMusic",ssprintf("DifficultyIconP%dOffCommand",p+1))
#define AUTOGEN_ICON_ON_COMMAND( p )		THEME->GetMetric ("ScreenSelectMusic",ssprintf("AutogenIconP%dOnCommand",p+1))
#define AUTOGEN_ICON_OFF_COMMAND( p )		THEME->GetMetric ("ScreenSelectMusic",ssprintf("AutogenIconP%dOffCommand",p+1))
#define RADAR_ON_COMMAND					THEME->GetMetric ("ScreenSelectMusic","RadarOnCommand")
#define RADAR_OFF_COMMAND					THEME->GetMetric ("ScreenSelectMusic","RadarOffCommand")
#define SORT_ICON_ON_COMMAND				THEME->GetMetric ("ScreenSelectMusic","SortIconOnCommand")
#define SORT_ICON_OFF_COMMAND				THEME->GetMetric ("ScreenSelectMusic","SortIconOffCommand")
#define SCORE_FRAME_ON_COMMAND( p )			THEME->GetMetric ("ScreenSelectMusic",ssprintf("ScoreFrameP%dOnCommand",p+1))
#define SCORE_FRAME_OFF_COMMAND( p )		THEME->GetMetric ("ScreenSelectMusic",ssprintf("ScoreFrameP%dOffCommand",p+1))
#define SCORE_ON_COMMAND( p )				THEME->GetMetric ("ScreenSelectMusic",ssprintf("ScoreP%dOnCommand",p+1))
#define SCORE_OFF_COMMAND( p )				THEME->GetMetric ("ScreenSelectMusic",ssprintf("ScoreP%dOffCommand",p+1))
#define METER_FRAME_ON_COMMAND( p )			THEME->GetMetric ("ScreenSelectMusic",ssprintf("MeterFrameP%dOnCommand",p+1))
#define METER_FRAME_OFF_COMMAND( p )		THEME->GetMetric ("ScreenSelectMusic",ssprintf("MeterFrameP%dOffCommand",p+1))
#define METER_ON_COMMAND( p )				THEME->GetMetric ("ScreenSelectMusic",ssprintf("MeterP%dOnCommand",p+1))
#define METER_OFF_COMMAND( p )				THEME->GetMetric ("ScreenSelectMusic",ssprintf("MeterP%dOffCommand",p+1))
#define WHEEL_ON_COMMAND					THEME->GetMetric ("ScreenSelectMusic","WheelOnCommand")
#define WHEEL_OFF_COMMAND					THEME->GetMetric ("ScreenSelectMusic","WheelOffCommand")
#define SONG_OPTIONS_ON_COMMAND				THEME->GetMetric ("ScreenSelectMusic","SongOptionsOnCommand")
#define SONG_OPTIONS_OFF_COMMAND			THEME->GetMetric ("ScreenSelectMusic","SongOptionsOffCommand")
#define SONG_OPTIONS_EXTRA_COMMAND			THEME->GetMetric ("ScreenSelectMusic","SongOptionsExtraCommand")
#define OPTION_ICONS_ON_COMMAND( p )		THEME->GetMetric ("ScreenSelectMusic",ssprintf("OptionIconsP%dOnCommand",p+1))
#define OPTION_ICONS_OFF_COMMAND( p )		THEME->GetMetric ("ScreenSelectMusic",ssprintf("OptionIconsP%dOffCommand",p+1))
#define BALLOON_ON_COMMAND					THEME->GetMetric ("ScreenSelectMusic","BalloonOnCommand")
#define BALLOON_OFF_COMMAND					THEME->GetMetric ("ScreenSelectMusic","BalloonOffCommand")
#define OPTIONS_MESSAGE_SHOW_COMMAND		THEME->GetMetric ("ScreenSelectMusic","OptionsMessageShowCommand")
#define SAMPLE_MUSIC_DELAY					THEME->GetMetricF("ScreenSelectMusic","SampleMusicDelay")


static const ScreenMessage	SM_AllowOptionsMenuRepeat	= ScreenMessage(SM_User+1);


ScreenSelectMusic::ScreenSelectMusic()
{
	LOG->Trace( "ScreenSelectMusic::ScreenSelectMusic()" );


	if( GAMESTATE->m_CurStyle == STYLE_INVALID )
		RageException::Throw( "The Style has not been set.  A theme must set the Style before loading ScreenSelectMusic." );

	if( GAMESTATE->m_PlayMode == PLAY_MODE_INVALID )
		RageException::Throw( "The PlayMode has not been set.  A theme must set the PlayMode before loading ScreenSelectMusic." );


	CodeDetector::RefreshCacheItems();

	int p;

	m_Menu.Load( "ScreenSelectMusic" );
	this->AddChild( &m_Menu );

	// these guys get loaded SetSong and TweenToSong
	m_Banner.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
	this->AddChild( &m_Banner );

	m_sprBannerFrame.Load( THEME->GetPathTo("Graphics","ScreenSelectMusic banner frame") );
	this->AddChild( &m_sprBannerFrame );

	this->AddChild( &m_BPMDisplay );

	m_sprStage.Load( THEME->GetPathTo("Graphics","ScreenSelectMusic stage "+GAMESTATE->GetStageText()) );
	this->AddChild( &m_sprStage );

	m_sprCDTitle.Load( THEME->GetPathTo("Graphics","ScreenSelectMusic fallback cdtitle") );
	this->AddChild( &m_sprCDTitle );

	this->AddChild( &m_GrooveRadar );

	m_textSongOptions.LoadFromFont( THEME->GetPathTo("Fonts","Common normal") );
	this->AddChild( &m_textSongOptions );

	this->AddChild( &m_MusicWheel );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		m_sprDifficultyFrame[p].Load( THEME->GetPathTo("Graphics","ScreenSelectMusic difficulty frame 2x1") );
		m_sprDifficultyFrame[p].StopAnimating();
		m_sprDifficultyFrame[p].SetState( p );
		this->AddChild( &m_sprDifficultyFrame[p] );

		m_DifficultyIcon[p].Load( THEME->GetPathTo("graphics","ScreenSelectMusic difficulty icons 1x5") );
		this->AddChild( &m_DifficultyIcon[p] );

		m_AutoGenIcon[p].Load( THEME->GetPathTo("graphics","ScreenSelectMusic autogen") );
		this->AddChild( &m_AutoGenIcon[p] );

		m_OptionIconRow[p].Refresh( (PlayerNumber)p );
		this->AddChild( &m_OptionIconRow[p] );

		m_sprMeterFrame[p].Load( THEME->GetPathTo("Graphics","ScreenSelectMusic meter frame") );
		m_sprMeterFrame[p].StopAnimating();
		m_sprMeterFrame[p].SetState( p );
		this->AddChild( &m_sprMeterFrame[p] );

		m_DifficultyMeter[p].SetShadowLength( 2 );
		this->AddChild( &m_DifficultyMeter[p] );
		
		m_sprHighScoreFrame[p].Load( THEME->GetPathTo("Graphics","ScreenSelectMusic score frame 1x2") );
		m_sprHighScoreFrame[p].StopAnimating();
		m_sprHighScoreFrame[p].SetState( p );
		this->AddChild( &m_sprHighScoreFrame[p] );

		m_textHighScore[p].LoadFromNumbers( THEME->GetPathTo("Numbers","ScreenSelectMusic score") );
		m_textHighScore[p].EnableShadow( false );
		m_textHighScore[p].SetDiffuse( PlayerToColor(p) );
		this->AddChild( &m_textHighScore[p] );
	}	

	m_MusicSortDisplay.Set( GAMESTATE->m_SongSortOrder );
	this->AddChild( &m_MusicSortDisplay );

	m_sprMarathonBalloon.Load( THEME->GetPathTo("Graphics","ScreenSelectMusic marathon") );
	m_sprMarathonBalloon.SetDiffuse( RageColor(1,1,1,0) );
	this->AddChild( &m_sprMarathonBalloon );

	m_sprLongBalloon.Load( THEME->GetPathTo("Graphics","ScreenSelectMusic long") );
	m_sprLongBalloon.SetDiffuse( RageColor(1,1,1,0) );
	this->AddChild( &m_sprLongBalloon );

	m_sprOptionsMessage.Load( THEME->GetPathTo("Graphics","ScreenSelectMusic options message 1x2") );
	m_sprOptionsMessage.StopAnimating();
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );	// invisible
	//this->AddChild( &m_sprOptionsMessage );	// we have to draw this manually over the top of transitions


	m_soundSelect.Load( THEME->GetPathTo("Sounds","Common start") );
	m_soundChangeNotes.Load( THEME->GetPathTo("Sounds","ScreenSelectMusic difficulty") );
	m_soundOptionsChange.Load( THEME->GetPathTo("Sounds","ScreenSelectMusic options") );
	m_soundLocked.Load( THEME->GetPathTo("Sounds","ScreenSelectMusic locked") );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select music intro") );

	m_bMadeChoice = false;
	m_bGoToOptions = false;
	m_fPlaySampleCountdown = 0;
	m_bAllowOptionsMenu = m_bAllowOptionsMenuRepeat = false;

	UpdateOptionsDisplays();

	AfterMusicChange();
	TweenOnScreen();
}


ScreenSelectMusic::~ScreenSelectMusic()
{
	LOG->Trace( "ScreenSelectMusic::~ScreenSelectMusic()" );

}

void ScreenSelectMusic::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
	m_sprOptionsMessage.Draw();
}

void ScreenSelectMusic::TweenOnScreen()
{
	m_sprBannerFrame.Command( BANNER_FRAME_ON_COMMAND );
	m_Banner.Command( BANNER_ON_COMMAND );
	m_BPMDisplay.Command( BPM_ON_COMMAND );
	m_sprStage.Command( STAGE_ON_COMMAND );
	m_sprCDTitle.Command( CD_TITLE_ON_COMMAND );
	m_GrooveRadar.TweenOnScreen();
	m_GrooveRadar.Command( RADAR_ON_COMMAND );
	m_textSongOptions.Command( SONG_OPTIONS_ON_COMMAND );
	m_MusicSortDisplay.Command( SORT_ICON_ON_COMMAND );
	m_MusicWheel.TweenOnScreen();
	m_MusicWheel.Command( WHEEL_ON_COMMAND );
	
	for( int p=0; p<NUM_PLAYERS; p++ )
	{		
		m_sprDifficultyFrame[p].Command( DIFFICULTY_FRAME_ON_COMMAND(p) );
		m_sprMeterFrame[p].Command( METER_FRAME_ON_COMMAND(p) );
		m_OptionIconRow[p].Command( OPTION_ICONS_ON_COMMAND(p) );
		m_DifficultyIcon[p].Command( DIFFICULTY_ICON_ON_COMMAND(p) );
		m_AutoGenIcon[p].Command( AUTOGEN_ICON_ON_COMMAND(p) );
		m_DifficultyMeter[p].Command( METER_ON_COMMAND(p) );
		m_sprHighScoreFrame[p].Command( SCORE_FRAME_ON_COMMAND(p) );
		m_textHighScore[p].Command( SCORE_ON_COMMAND(p) );
	}

	if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
		m_textSongOptions.Command( SONG_OPTIONS_EXTRA_COMMAND );
}

void ScreenSelectMusic::TweenOffScreen()
{
	m_sprBannerFrame.Command( BANNER_FRAME_OFF_COMMAND );
	m_Banner.Command( BANNER_OFF_COMMAND );
	m_BPMDisplay.Command( BPM_OFF_COMMAND );
	m_sprStage.Command( STAGE_OFF_COMMAND );
	m_sprCDTitle.Command( CD_TITLE_OFF_COMMAND );
	m_GrooveRadar.TweenOffScreen();
	m_GrooveRadar.Command( RADAR_OFF_COMMAND );
	m_textSongOptions.Command( SONG_OPTIONS_OFF_COMMAND );
	m_MusicSortDisplay.Command( SORT_ICON_OFF_COMMAND );
	m_MusicWheel.TweenOffScreen();
	m_MusicWheel.Command( WHEEL_OFF_COMMAND );
	
	for( int p=0; p<NUM_PLAYERS; p++ )
	{		
		m_sprDifficultyFrame[p].Command( DIFFICULTY_FRAME_OFF_COMMAND(p) );
		m_sprMeterFrame[p].Command( METER_FRAME_OFF_COMMAND(p) );
		m_OptionIconRow[p].Command( OPTION_ICONS_OFF_COMMAND(p) );
		m_DifficultyIcon[p].Command( DIFFICULTY_ICON_OFF_COMMAND(p) );
		m_AutoGenIcon[p].Command( AUTOGEN_ICON_OFF_COMMAND(p) );
		m_DifficultyMeter[p].Command( METER_OFF_COMMAND(p) );
		m_sprHighScoreFrame[p].Command( SCORE_FRAME_OFF_COMMAND(p) );
		m_textHighScore[p].Command( SCORE_OFF_COMMAND(p) );
	}
}

void ScreenSelectMusic::TweenScoreOnAndOffAfterChangeSort()
{
	/* XXX metric this with MusicWheel::TweenOnScreen */
	float factor = 0.25f;

	vector<Actor*> apActorsInScore;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		apActorsInScore.push_back( &m_sprHighScoreFrame[p] );
		apActorsInScore.push_back( &m_textHighScore[p] );
	}
	for( unsigned i=0; i<apActorsInScore.size(); i++ )
	{
		/* Grab the tween destination.  (If we're tweening, this is where
		 * it'll end up; otherwise it's the static position.) */
		Actor::TweenState original = apActorsInScore[i]->DestTweenState();

		apActorsInScore[i]->StopTweening();

		float fOriginalX = apActorsInScore[i]->GetX();
		apActorsInScore[i]->BeginTweening( factor*0.5f, TWEEN_DECELERATE );		// tween off screen
		apActorsInScore[i]->SetTweenX( fOriginalX+400 );
		
		apActorsInScore[i]->BeginTweening( factor*0.5f );		// sleep

		/* Go back to where we were (or to where we were going.) */
		apActorsInScore[i]->BeginTweening( factor*1, TWEEN_ACCELERATE );		// tween back on screen
		apActorsInScore[i]->SetLatestTween(original);
	}
}

void ScreenSelectMusic::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
	m_sprOptionsMessage.Update( fDeltaTime );


	if( m_fPlaySampleCountdown > 0 )
	{
		m_fPlaySampleCountdown -= fDeltaTime;
		/* Make sure we don't start the sample when rouletting is
		 * spinning down. */
		if( m_fPlaySampleCountdown <= 0 && !m_MusicWheel.IsRouletting() )
		{
			if( !m_sSampleMusicToPlay.empty() )
			{
				SOUNDMAN->PlayMusic(
					m_sSampleMusicToPlay, 
					true,
					m_fSampleStartSeconds,
					m_fSampleLengthSeconds,
					1.5f); /* fade out for 1.5 seconds */
			}
		}
	}

	float fNewRotation = m_sprCDTitle.GetRotationY()+180*fDeltaTime;
	fNewRotation = fmodf( fNewRotation, 360 );
	m_sprCDTitle.SetRotationY( fNewRotation );
	if( fNewRotation > 90  &&  fNewRotation <= 270 )
		m_sprCDTitle.SetDiffuse( RageColor(0.2f,0.2f,0.2f,1) );
	else
		m_sprCDTitle.SetDiffuse( RageColor(1,1,1,1) );
}

void ScreenSelectMusic::Input( const DeviceInput& DeviceI, InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenSelectMusic::Input()" );
	
	if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == SDLK_F9 )
	{
		if( type != IET_FIRST_PRESS ) return;
		PREFSMAN->m_bShowTranslations ^= 1;
		m_MusicWheel.RebuildMusicWheelItems();
		return;
	}

	if( MenuI.button == MENU_BUTTON_RIGHT || MenuI.button == MENU_BUTTON_LEFT )
	{
		if( !MenuI.IsValid() ) return;
		if( !GAMESTATE->IsHumanPlayer(MenuI.player) ) return;

		/* If we're rouletting, hands off. */
		if(m_MusicWheel.IsRouletting())
			return;

		int dir = 0;
		if(INPUTMAPPER->IsButtonDown( MenuInput(MenuI.player, MENU_BUTTON_RIGHT) ) )
			dir++;
		if(INPUTMAPPER->IsButtonDown( MenuInput(MenuI.player, MENU_BUTTON_LEFT) ) )
			dir--;
		
		m_MusicWheel.Move(dir);
		return;
	}

	if( type == IET_RELEASE )	return;		// don't care

	if( !GameI.IsValid() )		return;		// don't care

	if( m_bMadeChoice  &&  MenuI.IsValid()  &&  MenuI.button == MENU_BUTTON_START  &&  !GAMESTATE->IsExtraStage()  &&  !GAMESTATE->IsExtraStage2() )
	{
		if(m_bGoToOptions) return; /* got it already */
		if(!m_bAllowOptionsMenu) return; /* not allowed */

		if( !m_bAllowOptionsMenuRepeat &&
			(type == IET_SLOW_REPEAT || type == IET_FAST_REPEAT ))
			return; /* not allowed yet */
		
		m_bGoToOptions = true;
		m_sprOptionsMessage.SetState( 1 );
		SOUNDMAN->PlayOnce( THEME->GetPathTo("Sounds","Common start") );
		return;
	}
	
	if( m_Menu.IsTransitioning() )	return;		// ignore

	if( m_bMadeChoice )
		return;

	PlayerNumber pn = GAMESTATE->GetCurrentStyleDef()->ControllerToPlayerNumber( GameI.controller );

	if( CodeDetector::EnteredEasierDifficulty(GameI.controller) )
	{
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_soundLocked.Play();
		else
			EasierDifficulty( pn );
		return;
	}
	if( CodeDetector::EnteredHarderDifficulty(GameI.controller) )
	{
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			m_soundLocked.Play();
		else
			HarderDifficulty( pn );
		return;
	}
	if( CodeDetector::EnteredNextSort(GameI.controller) )
	{
		if( ( GAMESTATE->IsExtraStage() && !PREFSMAN->m_bPickExtraStage ) || GAMESTATE->IsExtraStage2() )
			m_soundLocked.Play();
		else
			if( m_MusicWheel.NextSort() )
			{
				SOUNDMAN->StopMusic();

//				m_MusicSortDisplay.FadeOff( 0, "fade", TWEEN_TIME );

				TweenScoreOnAndOffAfterChangeSort();
			}
		return;
	}
	if( !GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2() && CodeDetector::DetectAndAdjustOptions(GameI.controller) )
	{
		m_soundOptionsChange.Play();
		UpdateOptionsDisplays();
		return;
	}

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}


void ScreenSelectMusic::EasierDifficulty( PlayerNumber pn )
{
	LOG->Trace( "ScreenSelectMusic::EasierDifficulty( %d )", pn );

	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;
	if( m_arrayNotes[pn].empty() )
		return;
	if( m_iSelection[pn] == 0 )
		return;

	m_iSelection[pn]--;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficulty[pn] = m_arrayNotes[pn][ m_iSelection[pn] ]->GetDifficulty();

	m_soundChangeNotes.Play();

	AfterNotesChange( pn );
}

void ScreenSelectMusic::HarderDifficulty( PlayerNumber pn )
{
	LOG->Trace( "ScreenSelectMusic::HarderDifficulty( %d )", pn );

	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;
	if( m_arrayNotes[pn].empty() )
		return;
	if( m_iSelection[pn] == int(m_arrayNotes[pn].size()-1) )
		return;

	m_iSelection[pn]++;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficulty[pn] = m_arrayNotes[pn][ m_iSelection[pn] ]->GetDifficulty();

	m_soundChangeNotes.Play();

	AfterNotesChange( pn );
}

/* Adjust game options.  These settings may be overridden again later by the
 * SongOptions menu. */
void ScreenSelectMusic::AdjustOptions()
{
	/* Find the easiest difficulty notes selected by either player. */
	Difficulty dc = DIFFICULTY_INVALID;
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		dc = min(dc, GAMESTATE->m_pCurNotes[p]->GetDifficulty());
	}

	/* This can still interfere a bit with the song options menu; eg. if a
	 * player changes to a mode easier than the preference setting, we might
	 * reset it to the preference later. XXX */

	/* Never set the FailType harder than the preference. */
	SongOptions::FailType ft = SongOptions::FAIL_ARCADE;

	/* Easy and beginner are never harder than FAIL_END_OF_SONG. */
	if(dc <= DIFFICULTY_EASY)
		ft = SongOptions::FAIL_END_OF_SONG;
	/* If beginner's steps were chosen, and this is the first stage,
	 * turn off failure completely--always give a second try. */
	if(dc == DIFFICULTY_BEGINNER &&
		!PREFSMAN->m_bEventMode && /* stage index is meaningless in event mode */
		GAMESTATE->m_iCurrentStageIndex == 0)
		ft = SongOptions::FAIL_OFF;
//  Redundant.   -Chris
//	else if(GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2())
//	{
//		/* Extra stage.  We need to make sure we undo any changes above from
//		 * previous rounds; eg. where one player is on beginner and the other
//		 * is on hard, we've changed the fail mode in previous rounds and we
//		 * want to reset it for the extra stage.
//		 *
//		 * Besides, extra stage should probably always be FAIL_ARCADE anyway,
//		 * unless the extra stage course says otherwise. */
//		ft = SongOptions::FAIL_ARCADE;
//	}

	GAMESTATE->m_SongOptions.m_FailType = max( ft, GAMESTATE->m_SongOptions.m_FailType );
}

void ScreenSelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_AllowOptionsMenuRepeat:
		m_bAllowOptionsMenuRepeat = true;
		break;
	case SM_MenuTimer:
		if( m_MusicWheel.IsRouletting() )
		{
			MenuStart(PLAYER_INVALID);
			m_Menu.m_MenuTimer.SetSeconds( 15 );
		}
		else if( m_MusicWheel.GetSelectedType() != TYPE_SONG )
		{
			m_MusicWheel.StartRoulette();
			m_Menu.m_MenuTimer.SetSeconds( 15 );
		}
		else
		{
			MenuStart(PLAYER_INVALID);
		}
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		/* We may have stray SM_SongChanged messages from the music wheel.  We can't
		 * handle them anymore, since the title menu (and attract screens) reset
		 * the game state, so just discard them. */
		ClearMessageQueue();
		break;
	case SM_GoToNextScreen:
		if( m_bGoToOptions )
		{
			SCREENMAN->SetNewScreen( "ScreenPlayerOptions" );
		}
		else
		{
			SOUNDMAN->StopMusic();
			SCREENMAN->SetNewScreen( "ScreenStage" );
		}
		break;
	case SM_SongChanged:
		AfterMusicChange();
		break;
	case SM_SortOrderChanged:
		SortOrderChanged();
		break;
	}
}

void ScreenSelectMusic::MenuStart( PlayerNumber pn )
{
	if( pn != PLAYER_INVALID  &&
		INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_LEFT) )  &&
		INPUTMAPPER->IsButtonDown( MenuInput(pn, MENU_BUTTON_RIGHT) ) )
	{
//		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
//			m_soundLocked.Play();
//		else
		{
			if( m_MusicWheel.NextSort() )
			{
				SOUNDMAN->StopMusic();

//				m_MusicSortDisplay.FadeOff( 0, "fade", TWEEN_TIME );

				TweenScoreOnAndOffAfterChangeSort();
			}
		}
		return;
	}


	// this needs to check whether valid Notes are selected!
	bool bResult = m_MusicWheel.Select();

	/* If false, we don't have a selection just yet. */
	if( !bResult )
		return;

	// a song was selected
	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SONG: {
		if( !m_MusicWheel.GetSelectedSong()->HasMusic() )
		{
			/* TODO: gray these out. 
				*
				* XXX: also, make sure they're not selected by roulette */
			SCREENMAN->Prompt( SM_None, "ERROR:\n \nThis song does not have a music file\n and cannot be played." );
			return;
		}

		bool bIsNew = m_MusicWheel.GetSelectedSong()->IsNew();
		bool bIsHard = false;
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsHumanPlayer( (PlayerNumber)p ) )
				continue;	// skip
			if( GAMESTATE->m_pCurNotes[p]  &&  GAMESTATE->m_pCurNotes[p]->GetMeter() >= 10 )
				bIsHard = true;
		}

		if( bIsNew )
			SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select music comment new") );
		else if( bIsHard )
			SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select music comment hard") );
		else
			SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select music comment general") );


		TweenOffScreen();

		m_bMadeChoice = true;

		m_soundSelect.Play();

		if( !GAMESTATE->IsExtraStage()  &&  !GAMESTATE->IsExtraStage2() )
		{
//			float fShowSeconds = m_Menu.m_Out.GetLengthSeconds();

			// show "hold START for options"
			m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,1) );	// visible
			m_sprOptionsMessage.Command( OPTIONS_MESSAGE_SHOW_COMMAND );
/*			m_sprOptionsMessage.BeginTweening( 0.15f );	// fade in
			m_sprOptionsMessage.SetTweenZoomY( 1 );
			m_sprOptionsMessage.SetTweenDiffuse( RageColor(1,1,1,1) );
			m_sprOptionsMessage.BeginTweening( fShowSeconds-0.35f );	// sleep
			m_sprOptionsMessage.BeginTweening( 0.15f );	// fade out
			m_sprOptionsMessage.SetTweenDiffuse( RageColor(1,1,1,0) );
			m_sprOptionsMessage.SetTweenZoomY( 0 );
*/
			m_bAllowOptionsMenu = true;
			/* Don't accept a held START for a little while, so it's not
			 * hit accidentally.  Accept an initial START right away, though,
			 * so we don't ignore deliberate fast presses (which would be
			 * annoying). */
			this->PostScreenMessage( SM_AllowOptionsMenuRepeat, 0.5f );
		}

		m_Menu.StartTransitioning( SM_GoToNextScreen );
		AdjustOptions();
		break;
	}
	case TYPE_SECTION:
		break;
	case TYPE_ROULETTE:
		break;
	}

	if( GAMESTATE->IsExtraStage() && PREFSMAN->m_bPickExtraStage )
	{
		/* Check if user selected the real extra stage. */
		Song* pSong;
		Notes* pNotes;
		PlayerOptions po;
		SongOptions so;
		SONGMAN->GetExtraStageInfo( false, GAMESTATE->m_pCurSong->m_sGroupName, GAMESTATE->GetCurrentStyleDef(), pSong, pNotes, po, so );
		ASSERT(pSong);
		
		/* Enable 2nd extra stage if user chose the correct song */
		if( m_MusicWheel.GetSelectedSong() == pSong )
			GAMESTATE->m_bAllow2ndExtraStage = true;
		else
			GAMESTATE->m_bAllow2ndExtraStage = false;
	}
}


void ScreenSelectMusic::MenuBack( PlayerNumber pn )
{
	SOUNDMAN->StopMusic();

	m_Menu.Back( SM_GoToPrevScreen );
}

void ScreenSelectMusic::AfterNotesChange( PlayerNumber pn )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;
	
	m_iSelection[pn] = clamp( m_iSelection[pn], 0, int(m_arrayNotes[pn].size()-1) );	// bounds clamping

	Notes* pNotes = m_arrayNotes[pn].empty()? NULL: m_arrayNotes[pn][m_iSelection[pn]];

	GAMESTATE->m_pCurNotes[pn] = pNotes;

//	m_BPMDisplay.SetZoomY( 0 );
//	m_BPMDisplay.BeginTweening( 0.2f );
//	m_BPMDisplay.SetTweenZoomY( 1.2f );

	Notes* m_pNotes = GAMESTATE->m_pCurNotes[pn];
	
	if( m_pNotes && SONGMAN->IsUsingMemoryCard(pn) )
		m_textHighScore[pn].SetText( ssprintf("%*.0f", NUM_SCORE_DIGITS, m_pNotes->m_MemCardScores[pn].fScore) );

	m_DifficultyIcon[pn].SetFromNotes( pn, pNotes );
	if( pNotes && pNotes->IsAutogen() )
	{
		m_AutoGenIcon[pn].SetEffectDiffuseShift();
	}
	else
	{
		m_AutoGenIcon[pn].SetEffectNone();
		m_AutoGenIcon[pn].SetDiffuse( RageColor(1,1,1,0) );
	}
	m_DifficultyMeter[pn].SetFromNotes( pNotes );
	m_GrooveRadar.SetFromNotes( pn, pNotes );
	m_MusicWheel.NotesChanged( pn );
}

void ScreenSelectMusic::AfterMusicChange()
{
	m_Menu.m_MenuTimer.Stall();

	Song* pSong = m_MusicWheel.GetSelectedSong();
	GAMESTATE->m_pCurSong = pSong;

	m_sprStage.Load( THEME->GetPathTo("Graphics","ScreenSelectMusic stage "+GAMESTATE->GetStageText()) );

	int pn;
	for( pn = 0; pn < NUM_PLAYERS; ++pn)
		m_arrayNotes[pn].clear();

	/* If we're rouletting, and we're moving fast, don't touch the banner. */
	bool no_banner_change = false;
	if(m_MusicWheel.IsMoving()) 
	{
		/* We're moving fast.  Don't change banners if we're rouletting, or if
		 * we've been told to never change banners when moving fast.
		 *
		 * XXX: When we're not changing banners and not rouletting, show some
		 * kind of "moving fast" fallback banner.  (When rouletting, just keep
		 * showing the roulette banner.) */
		if(m_MusicWheel.IsRouletting() ||
			(m_MusicWheel.IsMoving() && !PREFSMAN->m_bChangeBannersWhenFast))
			no_banner_change = true;
	}

	m_sprMarathonBalloon.Command( BALLOON_OFF_COMMAND );
	m_sprLongBalloon.Command( BALLOON_OFF_COMMAND );

	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SECTION:
		{	
			CString sGroup = m_MusicWheel.GetSelectedSection();
			for( int p=0; p<NUM_PLAYERS; p++ )
				m_iSelection[p] = -1;

			if(!no_banner_change)
				m_Banner.LoadFromGroup( sGroup );	// if this isn't a group, it'll default to the fallback banner
			m_BPMDisplay.NoBPM();
			m_sprCDTitle.UnloadTexture();
		}
		break;
	case TYPE_SONG:
		{
			SOUNDMAN->StopMusic();
			m_fPlaySampleCountdown = SAMPLE_MUSIC_DELAY;
			m_sSampleMusicToPlay = pSong->GetMusicPath();
			m_fSampleStartSeconds = pSong->m_fMusicSampleStartSeconds;
			m_fSampleLengthSeconds = pSong->m_fMusicSampleLengthSeconds;

			for( int pn = 0; pn < NUM_PLAYERS; ++pn) 
			{
				pSong->GetNotes( m_arrayNotes[pn], GAMESTATE->GetCurrentStyleDef()->m_NotesType );
				SortNotesArrayByDifficulty( m_arrayNotes[pn] );
			}

			if(!no_banner_change)
				m_Banner.LoadFromSong( pSong );

			if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			{
				m_BPMDisplay.CycleRandomly();				
			}
			else
			{
				float fMinBPM, fMaxBPM;
				pSong->GetMinMaxBPM( fMinBPM, fMaxBPM );
				m_BPMDisplay.SetBPMRange( fMinBPM, fMaxBPM );
			}

			if( pSong->HasCDTitle() )
				m_sprCDTitle.Load( pSong->GetCDTitlePath() );
			else
				m_sprCDTitle.Load( THEME->GetPathTo("Graphics","ScreenSelectMusic fallback cdtitle") );
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMESTATE->IsHumanPlayer( PlayerNumber(p) ) )
					continue;

				/* Find the closest match to the user's preferred difficulty. */
				int CurDifference = -1;
				for( unsigned i=0; i<m_arrayNotes[p].size(); i++ )
				{
					int Diff = abs(m_arrayNotes[p][i]->GetDifficulty() - GAMESTATE->m_PreferredDifficulty[p]);

					if( CurDifference == -1 || Diff < CurDifference )
					{
						m_iSelection[p] = i;
						CurDifference = Diff;
					}
				}

				m_iSelection[p] = clamp( m_iSelection[p], 0, int(m_arrayNotes[p].size()) ) ;
			}

			/* Short delay before actually showing these, so they don't show
			 * up when scrolling fast.  It'll still show up in "slow" scrolling,
			 * but it doesn't look at weird as it does in "fast", and I don't
			 * like the effect with a lot of delay. */
			if( pSong->m_fMusicLengthSeconds > PREFSMAN->m_fMarathonVerSongSeconds )
			{
				m_sprMarathonBalloon.Command( BALLOON_ON_COMMAND );
			}
			else if( pSong->m_fMusicLengthSeconds > PREFSMAN->m_fLongVerSongSeconds )
			{
				m_sprLongBalloon.Command( BALLOON_ON_COMMAND );
			}
		}
		break;
	case TYPE_ROULETTE:
		if(!no_banner_change)
			m_Banner.LoadRoulette();
		m_BPMDisplay.NoBPM();
		m_sprCDTitle.UnloadTexture();

		SOUNDMAN->StopMusic();
		m_fPlaySampleCountdown = SAMPLE_MUSIC_DELAY;
		m_sSampleMusicToPlay = THEME->GetPathTo("Sounds","ScreenSelectMusic roulette music");
		m_fSampleStartSeconds = -1;
		m_fSampleLengthSeconds = -1;

		break;
	case TYPE_RANDOM:
		if(!no_banner_change)
			m_Banner.LoadRandom();
		m_BPMDisplay.NoBPM();
		m_sprCDTitle.UnloadTexture();

		SOUNDMAN->StopMusic();
		m_fPlaySampleCountdown = SAMPLE_MUSIC_DELAY;
		m_sSampleMusicToPlay = THEME->GetPathTo("Sounds","ScreenSelectMusic random music");
		m_fSampleStartSeconds = -1;
		m_fSampleLengthSeconds = -1;
		break;
	default:
		ASSERT(0);
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		AfterNotesChange( (PlayerNumber)p );
	}

	/* Make sure we never start the sample when moving fast. */
	if(m_MusicWheel.IsMoving())
		m_fPlaySampleCountdown = 0;
}


void ScreenSelectMusic::UpdateOptionsDisplays()
{
//	m_OptionIcons.Load( GAMESTATE->m_PlayerOptions, &GAMESTATE->m_SongOptions );

//	m_PlayerOptionIcons.Refresh();

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_OptionIconRow[p].Refresh( (PlayerNumber)p  );

		if( GAMESTATE->IsHumanPlayer(p) )
		{
			CString s = GAMESTATE->m_PlayerOptions[p].GetString();
			s.Replace( ", ", "\n" );
//			m_textPlayerOptions[p].SetText( s );
		}
	}

	CString s = GAMESTATE->m_SongOptions.GetString();
	s.Replace( ", ", "\n" );
	m_textSongOptions.SetText( s );
}

void ScreenSelectMusic::SortOrderChanged()
{
	m_MusicSortDisplay.Set( GAMESTATE->m_SongSortOrder );

	// tween music sort on screen
//	m_MusicSortDisplay.FadeOn( 0, "fade", TWEEN_TIME );
}

