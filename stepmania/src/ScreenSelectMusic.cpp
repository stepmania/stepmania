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
#include "ActorUtil.h"
#include "RageDisplay.h"
#include "RageTextureManager.h"


const int NUM_SCORE_DIGITS	=	9;

#define FOV									THEME->GetMetricF("ScreenSelectMusic","FOV")
#define BANNER_WIDTH						THEME->GetMetricF("ScreenSelectMusic","BannerWidth")
#define BANNER_HEIGHT						THEME->GetMetricF("ScreenSelectMusic","BannerHeight")
#define SONG_OPTIONS_EXTRA_COMMAND			THEME->GetMetric ("ScreenSelectMusic","SongOptionsExtraCommand")
#define SAMPLE_MUSIC_DELAY					THEME->GetMetricF("ScreenSelectMusic","SampleMusicDelay")
#define SHOW_RADAR							THEME->GetMetricB("ScreenSelectMusic","ShowRadar")
#define SHOW_GRAPH							THEME->GetMetricB("ScreenSelectMusic","ShowGraph")

#define CDTITLE_SPIN_SECONDS				THEME->GetMetricF("ScreenSelectMusic","CDTitleSpinSeconds")

static const ScreenMessage	SM_AllowOptionsMenuRepeat	= ScreenMessage(SM_User+1);

/* We make a backface for the CDTitle by rotating it on Y and mirroring it
 * on Y by flipping texture coordinates. */
static void FlipSpriteHorizontally(Sprite &s)
{
	float Coords[8];
	s.GetCurrentTextureCoords(Coords);
	swap(Coords[0], Coords[6]); /* top left X <-> top right X */
	swap(Coords[1], Coords[7]); /* top left Y <-> top right Y */
	swap(Coords[2], Coords[4]); /* bottom left X <-> bottom left X */
	swap(Coords[3], Coords[5]); /* bottom left Y <-> bottom left Y */
	s.SetCustomTextureCoords(Coords);
}

ScreenSelectMusic::ScreenSelectMusic() : Screen("ScreenSelectMusic")
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

	m_MusicWheel.SetName( "Wheel" );
	this->AddChild( &m_MusicWheel );

	m_sprBannerMask.SetName( "Banner" );	// use the same metrics and animation as Banner
	m_sprBannerMask.Load( THEME->GetPathToG("ScreenSelectMusic banner mask") );
	m_sprBannerMask.SetBlendMode( BLEND_NO_EFFECT );	// don't draw to color buffer
	m_sprBannerMask.SetUseZBuffer( true );	// do draw to the zbuffer
	m_sprBannerMask.SetZ( m_sprBannerMask.GetZ()+0.05f );
	this->AddChild( &m_sprBannerMask );

	// this is loaded SetSong and TweenToSong
	m_Banner.SetName( "Banner" );
	m_Banner.SetCroppedSize( BANNER_WIDTH, BANNER_HEIGHT );
	m_Banner.SetUseZBuffer( true );	// do have to pass the z test
	this->AddChild( &m_Banner );

	m_sprBannerFrame.SetName( "BannerFrame" );
	m_sprBannerFrame.Load( THEME->GetPathToG("ScreenSelectMusic banner frame") );
	this->AddChild( &m_sprBannerFrame );

	m_BPMDisplay.SetName( "BPM" );
	this->AddChild( &m_BPMDisplay );

	m_DifficultyDisplay.SetName( "DifficultyDisplay" );
	m_DifficultyDisplay.EnableShadow( false );
	this->AddChild( &m_DifficultyDisplay );

	m_sprStage.SetName( "Stage" );
	m_sprStage.Load( THEME->GetPathToG("ScreenSelectMusic stage "+GAMESTATE->GetStageText()) );
	this->AddChild( &m_sprStage );

	m_sprCDTitleFront.SetName( "CDTitle" );
	m_sprCDTitleFront.Load( THEME->GetPathToG("ScreenSelectMusic fallback cdtitle") );
	m_sprCDTitleFront.SetUseBackfaceCull(true);
	m_sprCDTitleFront.SetDiffuse( RageColor(1,1,1,1) );
	m_sprCDTitleFront.SetEffectSpin( RageVector3(0, 360/CDTITLE_SPIN_SECONDS, 0) );
	this->AddChild( &m_sprCDTitleFront );

	m_sprCDTitleBack.SetName( "CDTitle" );
	m_sprCDTitleBack.Load( THEME->GetPathToG("ScreenSelectMusic fallback cdtitle") );
	FlipSpriteHorizontally(m_sprCDTitleBack);
	m_sprCDTitleBack.SetUseBackfaceCull(true);
	m_sprCDTitleBack.SetDiffuse( RageColor(0.2f,0.2f,0.2f,1) );
	m_sprCDTitleBack.SetRotationY( 180 );
	m_sprCDTitleBack.SetEffectSpin( RageVector3(0, 360/CDTITLE_SPIN_SECONDS, 0) );
	this->AddChild( &m_sprCDTitleBack );

	m_GrooveRadar.SetName( "Radar" );
	if( SHOW_RADAR )
		this->AddChild( &m_GrooveRadar );

	m_GrooveGraph.SetName( "Graph" );
	if( SHOW_GRAPH )
		this->AddChild( &m_GrooveGraph );

	m_textSongOptions.SetName( "SongOptions" );
	m_textSongOptions.LoadFromFont( THEME->GetPathToF("Common normal") );
	this->AddChild( &m_textSongOptions );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		m_sprDifficultyFrame[p].SetName( ssprintf("DifficultyFrameP%d",p+1) );
		m_sprDifficultyFrame[p].Load( THEME->GetPathToG("ScreenSelectMusic difficulty frame 2x1") );
		m_sprDifficultyFrame[p].StopAnimating();
		m_sprDifficultyFrame[p].SetState( p );
		this->AddChild( &m_sprDifficultyFrame[p] );

		m_DifficultyIcon[p].SetName( ssprintf("DifficultyIconP%d",p+1) );
		m_DifficultyIcon[p].Load( THEME->GetPathToG("ScreenSelectMusic difficulty icons 1x5") );
		this->AddChild( &m_DifficultyIcon[p] );

		m_AutoGenIcon[p].SetName( ssprintf("AutogenIconP%d",p+1) );
		m_AutoGenIcon[p].Load( THEME->GetPathToG("ScreenSelectMusic autogen") );
		this->AddChild( &m_AutoGenIcon[p] );

		m_OptionIconRow[p].SetName( ssprintf("OptionIconsP%d",p+1) );
		m_OptionIconRow[p].Refresh( (PlayerNumber)p );
		this->AddChild( &m_OptionIconRow[p] );

		m_sprMeterFrame[p].SetName( ssprintf("MeterFrameP%d",p+1) );
		m_sprMeterFrame[p].Load( THEME->GetPathToG("ScreenSelectMusic meter frame") );
		m_sprMeterFrame[p].StopAnimating();
		m_sprMeterFrame[p].SetState( p );
		this->AddChild( &m_sprMeterFrame[p] );

		m_DifficultyMeter[p].SetName( ssprintf("MeterP%d",p+1) );
		m_DifficultyMeter[p].SetShadowLength( 2 );
		this->AddChild( &m_DifficultyMeter[p] );
		
		m_sprHighScoreFrame[p].SetName( ssprintf("ScoreFrameP%d",p+1) );
		m_sprHighScoreFrame[p].Load( THEME->GetPathToG("ScreenSelectMusic score frame 1x2") );
		m_sprHighScoreFrame[p].StopAnimating();
		m_sprHighScoreFrame[p].SetState( p );
		this->AddChild( &m_sprHighScoreFrame[p] );

		m_textHighScore[p].SetName( ssprintf("ScoreP%d",p+1) );
		m_textHighScore[p].LoadFromNumbers( THEME->GetPathToN("ScreenSelectMusic score") );
		m_textHighScore[p].EnableShadow( false );
		m_textHighScore[p].SetDiffuse( PlayerToColor(p) );
		this->AddChild( &m_textHighScore[p] );
	}	

	m_MusicSortDisplay.SetName( "SortIcon" );
	m_MusicSortDisplay.Set( GAMESTATE->m_SongSortOrder );
	this->AddChild( &m_MusicSortDisplay );

	m_sprBalloon.SetName( "Balloon" );
	TEXTUREMAN->CacheTexture( THEME->GetPathToG("ScreenSelectMusic balloon long") );
	TEXTUREMAN->CacheTexture( THEME->GetPathToG("ScreenSelectMusic balloon marathon") );
	this->AddChild( &m_sprBalloon );

	m_sprOptionsMessage.SetName( "OptionsMessage" );
	m_sprOptionsMessage.Load( THEME->GetPathToG("ScreenSelectMusic options message 1x2") );
	m_sprOptionsMessage.StopAnimating();
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );	// invisible
	//this->AddChild( &m_sprOptionsMessage );	// we have to draw this manually over the top of transitions


	m_soundSelect.Load( THEME->GetPathToS("Common start") );
	m_soundChangeNotes.Load( THEME->GetPathToS("ScreenSelectMusic difficulty") );
	m_soundOptionsChange.Load( THEME->GetPathToS("ScreenSelectMusic options") );
	m_soundLocked.Load( THEME->GetPathToS("ScreenSelectMusic locked") );

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
//	DISPLAY->LoadMenuPerspective(FOV);

	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
	m_sprOptionsMessage.Draw();
	
//	DISPLAY->LoadMenuPerspective(0);
}

void ScreenSelectMusic::TweenOnScreen()
{
	SET_XY_AND_ON_COMMAND( m_sprBannerMask );
	SET_XY_AND_ON_COMMAND( m_Banner );
	SET_XY_AND_ON_COMMAND( m_sprBannerFrame );
	SET_XY_AND_ON_COMMAND( m_BPMDisplay );
	SET_XY_AND_ON_COMMAND( m_DifficultyDisplay );
	SET_XY_AND_ON_COMMAND( m_sprStage );
	SET_XY_AND_ON_COMMAND( m_sprCDTitleFront );
	SET_XY_AND_ON_COMMAND( m_sprCDTitleBack );
	m_GrooveRadar.TweenOnScreen();
	SET_XY_AND_ON_COMMAND( m_GrooveRadar );
	m_GrooveGraph.TweenOnScreen();
	SET_XY_AND_ON_COMMAND( m_GrooveGraph );
	SET_XY_AND_ON_COMMAND( m_textSongOptions );
	SET_XY_AND_ON_COMMAND( m_MusicSortDisplay );
	m_MusicWheel.TweenOnScreen();
	SET_XY_AND_ON_COMMAND( m_MusicWheel );
	
	for( int p=0; p<NUM_PLAYERS; p++ )
	{		
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		SET_XY_AND_ON_COMMAND( m_sprDifficultyFrame[p] );
		SET_XY_AND_ON_COMMAND( m_sprMeterFrame[p] );
		SET_XY_AND_ON_COMMAND( m_OptionIconRow[p] );
		SET_XY_AND_ON_COMMAND( m_DifficultyIcon[p] );
		SET_XY_AND_ON_COMMAND( m_AutoGenIcon[p] );
		SET_XY_AND_ON_COMMAND( m_DifficultyMeter[p] );
		SET_XY_AND_ON_COMMAND( m_sprHighScoreFrame[p] );
		SET_XY_AND_ON_COMMAND( m_textHighScore[p] );
	}

	if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
		m_textSongOptions.Command( SONG_OPTIONS_EXTRA_COMMAND );
}

void ScreenSelectMusic::TweenOffScreen()
{
	OFF_COMMAND( m_sprBannerMask );
	OFF_COMMAND( m_Banner );
	OFF_COMMAND( m_sprBannerFrame );
	OFF_COMMAND( m_BPMDisplay );
	OFF_COMMAND( m_DifficultyDisplay );
	OFF_COMMAND( m_sprStage );
	OFF_COMMAND( m_sprCDTitleFront );
	OFF_COMMAND( m_sprCDTitleBack );
	m_GrooveRadar.TweenOffScreen();
	OFF_COMMAND( m_GrooveRadar );
	m_GrooveGraph.TweenOffScreen();
	OFF_COMMAND( m_GrooveGraph );
	OFF_COMMAND( m_textSongOptions );
	OFF_COMMAND( m_MusicSortDisplay );
	m_MusicWheel.TweenOffScreen();
	OFF_COMMAND( m_MusicWheel );
	OFF_COMMAND( m_sprBalloon );
	
	for( int p=0; p<NUM_PLAYERS; p++ )
	{		
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		OFF_COMMAND( m_sprDifficultyFrame[p] );
		OFF_COMMAND( m_sprMeterFrame[p] );
		OFF_COMMAND( m_OptionIconRow[p] );
		OFF_COMMAND( m_DifficultyIcon[p] );
		OFF_COMMAND( m_AutoGenIcon[p] );
		OFF_COMMAND( m_DifficultyMeter[p] );
		OFF_COMMAND( m_sprHighScoreFrame[p] );
		OFF_COMMAND( m_textHighScore[p] );
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
		apActorsInScore[i]->SetX( fOriginalX+400 );
		
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
}

void ScreenSelectMusic::Input( const DeviceInput& DeviceI, InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenSelectMusic::Input()" );
	
	if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == SDLK_F9 )
	{
		if( type != IET_FIRST_PRESS ) return;
		PREFSMAN->m_bShowNative ^= 1;
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
		SOUNDMAN->PlayOnce( THEME->GetPathToS("Common start") );
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
			m_MusicWheel.ChangeSort( SORT_SORT );
//			if( m_MusicWheel.NextSort() )
//			{
//				SOUNDMAN->StopMusic();
//				TweenScoreOnAndOffAfterChangeSort();
//			}
		return;
	}
	if( !GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2() && CodeDetector::DetectAndAdjustMusicOptions(GameI.controller) )
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

	LOG->Trace( "AdjustOptions: difficulty %i", dc );
	if( !GAMESTATE->m_bChangedFailMode )
	{
		GAMESTATE->m_SongOptions.m_FailType = SongOptions::FAIL_ARCADE;

        /* Easy and beginner are never harder than FAIL_END_OF_SONG. */
		if(dc <= DIFFICULTY_EASY)
		{
			LOG->Trace( "AdjustOptions: EOS" );
			GAMESTATE->m_SongOptions.m_FailType = SongOptions::FAIL_END_OF_SONG;
		}

		/* If beginner's steps were chosen, and this is the first stage,
		 * turn off failure completely--always give a second try. */
		if(dc == DIFFICULTY_BEGINNER &&
			!PREFSMAN->m_bEventMode && /* stage index is meaningless in event mode */
			GAMESTATE->m_iCurrentStageIndex == 0)
			GAMESTATE->m_SongOptions.m_FailType = SongOptions::FAIL_OFF;
	}
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
			SET_XY_AND_ON_COMMAND( m_sprOptionsMessage );
/*			m_sprOptionsMessage.BeginTweening( 0.15f );	// fade in
			m_sprOptionsMessage.SetZoomY( 1 );
			m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,1) );
			m_sprOptionsMessage.BeginTweening( fShowSeconds-0.35f );	// sleep
			m_sprOptionsMessage.BeginTweening( 0.15f );	// fade out
			m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
			m_sprOptionsMessage.SetZoomY( 0 );
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
//	m_BPMDisplay.SetZoomY( 1.2f );

	Notes* m_pNotes = GAMESTATE->m_pCurNotes[pn];
	
	if( m_pNotes && SONGMAN->IsUsingMemoryCard(pn) )
		m_textHighScore[pn].SetText( ssprintf("%*i", NUM_SCORE_DIGITS, m_pNotes->m_MemCardScores[pn].iScore) );

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
	if( pSong )
		GAMESTATE->m_pCurSong = pSong;

	m_sprStage.Load( THEME->GetPathToG("ScreenSelectMusic stage "+GAMESTATE->GetStageText()) );

	m_GrooveGraph.SetFromSong( pSong );

	int pn;
	for( pn = 0; pn < NUM_PLAYERS; ++pn)
		m_arrayNotes[pn].clear();

	bool no_banner_change = false;
	m_Banner.SetMovingFast( !!m_MusicWheel.IsMoving() );
//	if(PREFSMAN->m_BannerCacheType == PREFSMAN->preload_none && m_MusicWheel.IsMoving()) 
//	{
		/* If we're moving fast and we didn't preload banners, don't touch it. */
//		no_banner_change = true;
//	}

	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SECTION:
	case TYPE_SORT:
		{	
			CString sGroup = m_MusicWheel.GetSelectedSection();
			for( int p=0; p<NUM_PLAYERS; p++ )
				m_iSelection[p] = -1;

			if(!no_banner_change)
				m_Banner.LoadFromGroup( sGroup );	// if this isn't a group, it'll default to the fallback banner
			m_BPMDisplay.NoBPM();
			m_sprCDTitleFront.UnloadTexture();
			m_sprCDTitleBack.UnloadTexture();

			m_sprBalloon.StopTweening();
			OFF_COMMAND( m_sprBalloon );
		}
		break;
	case TYPE_SONG:
		{
			// Don't stop music if it's already playing the right file.
//			SOUNDMAN->StopMusic();
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
				m_BPMDisplay.SetBPM( pSong );
			}

			const CString CDTitlePath = pSong->HasCDTitle()? pSong->GetCDTitlePath():THEME->GetPathToG("ScreenSelectMusic fallback cdtitle");
			m_sprCDTitleFront.Load( CDTitlePath );
			m_sprCDTitleBack.Load( CDTitlePath );
			FlipSpriteHorizontally(m_sprCDTitleBack);

			m_DifficultyDisplay.SetDifficulties( pSong, GAMESTATE->GetCurrentStyleDef()->m_NotesType );
 
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
				m_sprBalloon.StopTweening();
				m_sprBalloon.Load( THEME->GetPathToG("ScreenSelectMusic balloon marathon") );
				SET_XY_AND_ON_COMMAND( m_sprBalloon );
			}
			else if( pSong->m_fMusicLengthSeconds > PREFSMAN->m_fLongVerSongSeconds )
			{
				m_sprBalloon.StopTweening();
				m_sprBalloon.Load( THEME->GetPathToG("ScreenSelectMusic balloon long") );
				SET_XY_AND_ON_COMMAND( m_sprBalloon );
			}
			else
			{
				m_sprBalloon.StopTweening();
				OFF_COMMAND( m_sprBalloon );
			}
		}
		break;
	case TYPE_ROULETTE:
	case TYPE_RANDOM:
		if(!no_banner_change)
			m_Banner.LoadRoulette();
		m_BPMDisplay.NoBPM();
		m_sprCDTitleFront.UnloadTexture();
		m_sprCDTitleBack.UnloadTexture();

		SOUNDMAN->StopMusic();
		m_fPlaySampleCountdown = SAMPLE_MUSIC_DELAY;
		switch( m_MusicWheel.GetSelectedType() )
		{
		case TYPE_ROULETTE:
			m_sSampleMusicToPlay = THEME->GetPathToS("ScreenSelectMusic roulette music");
			break;
		case TYPE_RANDOM:
			m_sSampleMusicToPlay = THEME->GetPathToS("ScreenSelectMusic random music");
			break;
		default:
			ASSERT(0);
		}

		m_fSampleStartSeconds = -1;
		m_fSampleLengthSeconds = -1;

		m_sprBalloon.StopTweening();
		OFF_COMMAND( m_sprBalloon );
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

