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
#include "RageSounds.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "GameState.h"
#include "CodeDetector.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "ActorUtil.h"
#include "RageDisplay.h"
#include "RageTextureManager.h"
#include "Course.h"
#include "ProfileManager.h"
#include "MenuTimer.h"
#include "LightsManager.h"
#include "StageStats.h"
#include "StepsUtil.h"


const int NUM_SCORE_DIGITS	=	9;

#define FOV									THEME->GetMetricF("ScreenSelectMusic","FOV")
#define FOV_CENTER_X						THEME->GetMetricF("ScreenSelectMusic","FOVCenterX")
#define FOV_CENTER_Y						THEME->GetMetricF("ScreenSelectMusic","FOVCenterY")
#define BANNER_WIDTH						THEME->GetMetricF("ScreenSelectMusic","BannerWidth")
#define BANNER_HEIGHT						THEME->GetMetricF("ScreenSelectMusic","BannerHeight")
#define SONG_OPTIONS_EXTRA_COMMAND			THEME->GetMetric ("ScreenSelectMusic","SongOptionsExtraCommand")
#define SAMPLE_MUSIC_DELAY					THEME->GetMetricF("ScreenSelectMusic","SampleMusicDelay")
#define SHOW_RADAR							THEME->GetMetricB("ScreenSelectMusic","ShowRadar")
#define SHOW_GRAPH							THEME->GetMetricB("ScreenSelectMusic","ShowGraph")
#define SHOW_PANES							THEME->GetMetricB("ScreenSelectMusic","ShowPanes")
#define SHOW_DIFFICULTY_LIST				THEME->GetMetricB("ScreenSelectMusic","ShowDifficultyList")
#define CDTITLE_SPIN_SECONDS				THEME->GetMetricF("ScreenSelectMusic","CDTitleSpinSeconds")
#define PREV_SCREEN							THEME->GetMetric ("ScreenSelectMusic","PrevScreen")
#define NEXT_SCREEN							THEME->GetMetric ("ScreenSelectMusic","NextScreen")
#define NEXT_OPTIONS_SCREEN					THEME->GetMetric ("ScreenSelectMusic","NextOptionsScreen")
#define SCORE_SORT_CHANGE_COMMAND(i) 		THEME->GetMetric ("ScreenSelectMusic",ssprintf("ScoreP%iSortChangeCommand", i+1))
#define SCORE_FRAME_SORT_CHANGE_COMMAND(i)	THEME->GetMetric ("ScreenSelectMusic",ssprintf("ScoreFrameP%iSortChangeCommand", i+1))
#define DO_ROULETTE_ON_MENU_TIMER			THEME->GetMetricB("ScreenSelectMusic","DoRouletteOnMenuTimer")
#define ALIGN_MUSIC_BEATS					THEME->GetMetricB("ScreenSelectMusic","AlignMusicBeat")

static const ScreenMessage	SM_AllowOptionsMenuRepeat	= ScreenMessage(SM_User+1);
CString g_sFallbackCDTitlePath;

/* We make a backface for the CDTitle by rotating it on Y and mirroring it
 * on Y by flipping texture coordinates. */
static void FlipSpriteHorizontally(Sprite &s)
{
	s.StopUsingCustomCoords();

	float Coords[8];
	s.GetActiveTextureCoords(Coords);
	swap(Coords[0], Coords[6]); /* top left X <-> top right X */
	swap(Coords[1], Coords[7]); /* top left Y <-> top right Y */
	swap(Coords[2], Coords[4]); /* bottom left X <-> bottom left X */
	swap(Coords[3], Coords[5]); /* bottom left Y <-> bottom left Y */
	s.SetCustomTextureCoords(Coords);
}

ScreenSelectMusic::ScreenSelectMusic( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenSelectMusic::ScreenSelectMusic()" );

	LIGHTSMAN->SetLightsMode( LIGHTSMODE_MENU );

	m_DisplayMode = GAMESTATE->IsCourseMode() ? DISPLAY_COURSES : DISPLAY_SONGS;

	/* Finish any previous stage.  It's OK to call this when we havn't played a stage yet. 
	 * Do this before anything that might look at GAMESTATE->m_iCurrentStageIndex. */
	GAMESTATE->FinishStage();

	/* Cache: */
	g_sFallbackCDTitlePath = THEME->GetPathToG("ScreenSelectMusic fallback cdtitle");

	if( GAMESTATE->m_CurStyle == STYLE_INVALID )
		RageException::Throw( "The Style has not been set.  A theme must set the Style before loading ScreenSelectMusic." );

	if( GAMESTATE->m_PlayMode == PLAY_MODE_INVALID )
		RageException::Throw( "The PlayMode has not been set.  A theme must set the PlayMode before loading ScreenSelectMusic." );

	m_MusicWheel.Load();

	int p;

	// pop'n music has this under the songwheel...
	for( p=0; p<NUM_PLAYERS; p++ )
	{
		Character* pChar = GAMESTATE->m_pCurCharacters[p];
		m_sprCharacterIcon[p].SetName( ssprintf("CharacterIconP%d",p+1) );
		m_sprCharacterIcon[p].Load( pChar->GetSongSelectIconPath() );
		SET_XY( m_sprCharacterIcon[p] );
		this->AddChild( &m_sprCharacterIcon[p] );
	}



	m_MusicWheelUnder.Load( THEME->GetPathToG("ScreenSelectMusic wheel under") );
	m_MusicWheelUnder->SetName( "WheelUnder" );
	SET_XY( m_MusicWheelUnder );
	this->AddChild( m_MusicWheelUnder );

	m_MusicWheel.SetName( "MusicWheel", "Wheel" );
	SET_XY( m_MusicWheel );
	this->AddChild( &m_MusicWheel );

	m_sprBannerMask.SetName( "Banner" );	// use the same metrics and animation as Banner
	m_sprBannerMask.Load( THEME->GetPathToG("ScreenSelectMusic banner mask") );
	m_sprBannerMask.SetBlendMode( BLEND_NO_EFFECT );	// don't draw to color buffer
	m_sprBannerMask.SetZWrite( true );	// do draw to the zbuffer
	SET_XY( m_sprBannerMask );
	this->AddChild( &m_sprBannerMask );

	// this is loaded SetSong and TweenToSong
	m_Banner.SetName( "Banner" );
	m_Banner.SetZTest( true );	// do have to pass the z test
	m_Banner.ScaleToClipped( BANNER_WIDTH, BANNER_HEIGHT );
	SET_XY( m_Banner );
	this->AddChild( &m_Banner );

	m_sprBannerFrame.Load( THEME->GetPathToG("ScreenSelectMusic banner frame") );
	m_sprBannerFrame->SetName( "BannerFrame" );
	SET_XY( m_sprBannerFrame );
	this->AddChild( m_sprBannerFrame );

	m_BPMDisplay.SetName( "BPMDisplay" );
	m_BPMDisplay.Load();
	SET_XY( m_BPMDisplay );
	this->AddChild( &m_BPMDisplay );

	m_DifficultyDisplay.SetName( "DifficultyDisplay" );
	m_DifficultyDisplay.SetShadowLength( 0 );
	SET_XY( m_DifficultyDisplay );
	this->AddChild( &m_DifficultyDisplay );

	{
		CStringArray StageTexts;
		GAMESTATE->GetAllStageTexts( StageTexts );
		for( unsigned i = 0; i < StageTexts.size(); ++i )
		{
			CString path = THEME->GetPathToG( "ScreenSelectMusic stage "+StageTexts[i], true );
			if( path != "" )
				TEXTUREMAN->CacheTexture( path );
		}
	}

	// loaded in AfterMusicChange
	m_sprStage.SetName( "Stage" );
	SET_XY( m_sprStage );
	this->AddChild( &m_sprStage );

	m_sprCDTitleFront.SetName( "CDTitle" );
	m_sprCDTitleFront.Load( THEME->GetPathToG("ScreenSelectMusic fallback cdtitle") );
	m_sprCDTitleFront.SetCullMode( CULL_BACK );
	m_sprCDTitleFront.SetDiffuse( RageColor(1,1,1,1) );
	m_sprCDTitleFront.SetEffectSpin( RageVector3(0, 360/CDTITLE_SPIN_SECONDS, 0) );
	SET_XY( m_sprCDTitleFront );
	this->AddChild( &m_sprCDTitleFront );

	m_sprCDTitleBack.SetName( "CDTitle" );
	m_sprCDTitleBack.Load( THEME->GetPathToG("ScreenSelectMusic fallback cdtitle") );
	FlipSpriteHorizontally(m_sprCDTitleBack);
	m_sprCDTitleBack.SetCullMode( CULL_BACK );
	m_sprCDTitleBack.SetDiffuse( RageColor(0.2f,0.2f,0.2f,1) );
	m_sprCDTitleBack.SetRotationY( 180 );
	m_sprCDTitleBack.SetEffectSpin( RageVector3(0, 360/CDTITLE_SPIN_SECONDS, 0) );
	SET_XY( m_sprCDTitleBack );
	this->AddChild( &m_sprCDTitleBack );

	m_GrooveRadar.SetName( "Radar" );
	SET_XY( m_GrooveRadar );
	if( SHOW_RADAR )
		this->AddChild( &m_GrooveRadar );

	m_GrooveGraph.SetName( "Graph" );
	SET_XY( m_GrooveGraph );
	if( SHOW_GRAPH )
		this->AddChild( &m_GrooveGraph );

	m_textSongOptions.SetName( "SongOptions" );
	m_textSongOptions.LoadFromFont( THEME->GetPathToF("Common normal") );
	SET_XY( m_textSongOptions );
	this->AddChild( &m_textSongOptions );

	m_CourseContentsFrame.SetName( "CourseContents" );
	SET_XY( m_CourseContentsFrame );
	this->AddChild( &m_CourseContentsFrame );

	m_Artist.SetName( "ArtistDisplay" );
	m_Artist.Load();
	SET_XY( m_Artist );
	this->AddChild( &m_Artist );
		
	m_MachineRank.SetName( "MachineRank" );
	m_MachineRank.LoadFromFont( THEME->GetPathToF("ScreenSelectMusic rank") );
	SET_XY( m_MachineRank );
	this->AddChild( &m_MachineRank );

	if( SHOW_DIFFICULTY_LIST )
	{
		m_DifficultyList.SetName( "DifficultyList" );
		m_DifficultyList.Load();
		SET_XY_AND_ON_COMMAND( m_DifficultyList );
		this->AddChild( &m_DifficultyList );
	}

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		m_sprDifficultyFrame[p].SetName( ssprintf("DifficultyFrameP%d",p+1) );
		m_sprDifficultyFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenSelectMusic difficulty frame p%d",p+1)) );
		m_sprDifficultyFrame[p].StopAnimating();
		SET_XY( m_sprDifficultyFrame[p] );
		this->AddChild( &m_sprDifficultyFrame[p] );

		m_DifficultyIcon[p].SetName( ssprintf("DifficultyIconP%d",p+1) );
		m_DifficultyIcon[p].Load( THEME->GetPathToG(ssprintf("ScreenSelectMusic difficulty icons 1x%d",NUM_DIFFICULTIES)) );
		SET_XY( m_DifficultyIcon[p] );
		this->AddChild( &m_DifficultyIcon[p] );

		m_AutoGenIcon[p].SetName( ssprintf("AutogenIconP%d",p+1) );
		m_AutoGenIcon[p].Load( THEME->GetPathToG("ScreenSelectMusic autogen") );
		SET_XY( m_AutoGenIcon[p] );
		this->AddChild( &m_AutoGenIcon[p] );

		m_OptionIconRow[p].SetName( ssprintf("OptionIconsP%d",p+1) );
		m_OptionIconRow[p].Load( (PlayerNumber)p );
		m_OptionIconRow[p].Refresh();
		SET_XY( m_OptionIconRow[p] );
		this->AddChild( &m_OptionIconRow[p] );

		m_sprMeterFrame[p].SetName( ssprintf("MeterFrameP%d",p+1) );
		m_sprMeterFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenSelectMusic meter frame p%d",p+1)) );
		SET_XY( m_sprMeterFrame[p] );
		this->AddChild( &m_sprMeterFrame[p] );

		if( SHOW_PANES )
		{
			m_PaneDisplay[p].SetName( "PaneDisplay", ssprintf("PaneDisplayP%d",p+1) );
			m_PaneDisplay[p].Load( (PlayerNumber) p );
			this->AddChild( &m_PaneDisplay[p] );
		}

		m_DifficultyMeter[p].SetName( "DifficultyMeter", ssprintf("MeterP%d",p+1) );
		m_DifficultyMeter[p].Load();
		SET_XY_AND_ON_COMMAND( m_DifficultyMeter[p] );
		this->AddChild( &m_DifficultyMeter[p] );

		// add an icon onto the song select to show what
		// character they're using.

		m_sprHighScoreFrame[p].SetName( ssprintf("ScoreFrameP%d",p+1) );
		m_sprHighScoreFrame[p].Load( THEME->GetPathToG(ssprintf("ScreenSelectMusic score frame p%d",p+1)) );
		SET_XY( m_sprHighScoreFrame[p] );
		this->AddChild( &m_sprHighScoreFrame[p] );

		m_textHighScore[p].SetName( ssprintf("ScoreP%d",p+1) );
		m_textHighScore[p].LoadFromNumbers( THEME->GetPathToN("ScreenSelectMusic score") );
		m_textHighScore[p].SetShadowLength( 0 );
		m_textHighScore[p].SetDiffuse( PlayerToColor(p) );
		SET_XY( m_textHighScore[p] );
		this->AddChild( &m_textHighScore[p] );
	}	

	m_MusicSortDisplay.SetName( "SortIcon" );
	m_MusicSortDisplay.Set( GAMESTATE->m_SortOrder );
	SET_XY( m_MusicSortDisplay );
	this->AddChild( &m_MusicSortDisplay );

	m_sprBalloon.SetName( "Balloon" );
	TEXTUREMAN->CacheTexture( THEME->GetPathToG("ScreenSelectMusic balloon long") );
	TEXTUREMAN->CacheTexture( THEME->GetPathToG("ScreenSelectMusic balloon marathon") );
	this->AddChild( &m_sprBalloon );

	m_sprCourseHasMods.LoadAndSetName( m_sName, "CourseHasMods" );
	this->AddChild( m_sprCourseHasMods );

	m_sprOptionsMessage.SetName( "OptionsMessage" );
	m_sprOptionsMessage.Load( THEME->GetPathToG("ScreenSelectMusic options message 1x2") );
	m_sprOptionsMessage.StopAnimating();
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );	// invisible
	//this->AddChild( &m_sprOptionsMessage );	// we have to draw this manually over the top of transitions

	{
		FOREACH_PlayerNumber( p )
		{
			m_sprNonPresence[p].SetName( ssprintf("NonPresenceP%d",p+1) );
			m_sprNonPresence[p].Load( THEME->GetPathToG(ssprintf("ScreenSelectMusic nonpresence p%d",p+1)) );
			SET_XY( m_sprNonPresence[p] );
			this->AddChild( &m_sprNonPresence[p] );
		}
	}

	m_Overlay.SetName( "Overlay");
	m_Overlay.LoadFromAniDir( THEME->GetPathB(m_sName, "overlay"));
	this->AddChild( &m_Overlay );

	m_bgOptionsOut.Load( THEME->GetPathToB(m_sName+" options out") );
//	this->AddChild( &m_bgOptionsOut ); // drawn on top
	m_bgNoOptionsOut.Load( THEME->GetPathToB(m_sName+" no options out") );
//	this->AddChild( &m_bgNoOptionsOut ); // drawn on top

	m_soundDifficultyEasier.Load( THEME->GetPathToS("ScreenSelectMusic difficulty easier") );
	m_soundDifficultyHarder.Load( THEME->GetPathToS("ScreenSelectMusic difficulty harder") );
	m_soundOptionsChange.Load( THEME->GetPathToS("ScreenSelectMusic options") );
	m_soundLocked.Load( THEME->GetPathToS("ScreenSelectMusic locked") );

	SOUND->PlayOnceFromAnnouncer( "select music intro" );

	m_bMadeChoice = false;
	m_bGoToOptions = false;
	m_fPlaySampleCountdown = 0;
	m_bAllowOptionsMenu = m_bAllowOptionsMenuRepeat = false;

	UpdateOptionsDisplays();

	AfterMusicChange();
	TweenOnScreen();

	this->SortByDrawOrder();
}


ScreenSelectMusic::~ScreenSelectMusic()
{
	LOG->Trace( "ScreenSelectMusic::~ScreenSelectMusic()" );

}

void ScreenSelectMusic::DrawPrimitives()
{
	DISPLAY->CameraPushMatrix();
	DISPLAY->LoadMenuPerspective( FOV, FOV_CENTER_X, FOV_CENTER_Y );

	Screen::DrawPrimitives();
	m_sprOptionsMessage.Draw();
	m_bgOptionsOut.Draw();
	m_bgNoOptionsOut.Draw();
	
	DISPLAY->CameraPopMatrix();
}

void ScreenSelectMusic::TweenSongPartsOnScreen( bool Initial )
{
	m_GrooveRadar.StopTweening();
	m_GrooveGraph.StopTweening();
	m_GrooveRadar.TweenOnScreen();
	m_GrooveGraph.TweenOnScreen();
	if( SHOW_DIFFICULTY_LIST )
	{
		if( Initial )
		{
			ON_COMMAND( m_DifficultyList );
			m_DifficultyList.TweenOnScreen();
		}
//		else // do this after SM_SortOrderChanged
//			m_DifficultyList.Show();
	}

	{
		FOREACH_HumanPlayer( p )
		{
			ON_COMMAND( m_sprDifficultyFrame[p] );
			ON_COMMAND( m_sprMeterFrame[p] );
			ON_COMMAND( m_DifficultyIcon[p] );
			ON_COMMAND( m_AutoGenIcon[p] );
		}
	}

	{
		FOREACH_PlayerNumber( p )
		{
			ON_COMMAND( m_sprNonPresence[p] );
		}
	}
}

void ScreenSelectMusic::TweenSongPartsOffScreen( bool Final )
{
	m_GrooveRadar.TweenOffScreen();
	m_GrooveGraph.TweenOffScreen();
	if( SHOW_DIFFICULTY_LIST )
	{
		if( Final )
		{
			OFF_COMMAND( m_DifficultyList );
			m_DifficultyList.TweenOffScreen();
		}
		else
			m_DifficultyList.Hide();
	}

	{
		FOREACH_HumanPlayer( p )
		{
			OFF_COMMAND( m_sprDifficultyFrame[p] );
			OFF_COMMAND( m_sprMeterFrame[p] );
			OFF_COMMAND( m_DifficultyIcon[p] );
			OFF_COMMAND( m_AutoGenIcon[p] );
		}
	}

	{
		FOREACH_PlayerNumber( p )
		{
			OFF_COMMAND( m_sprNonPresence[p] );
		}
	}
}

void ScreenSelectMusic::TweenCoursePartsOnScreen( bool Initial )
{
	m_CourseContentsFrame.SetZoomY( 1 );
	if( Initial )
	{
		m_CourseContentsFrame.Command( "ZoomY,0;BounceEnd,0.3;Zoom,1" );
		COMMAND( m_CourseContentsFrame, "On" );
	}
	else
	{
		m_CourseContentsFrame.SetFromCourse(NULL);
		COMMAND( m_CourseContentsFrame, "Show" );
	}
	// XXX: if !Initial, m_CourseContentsFrame.Hide?
}

void ScreenSelectMusic::TweenCoursePartsOffScreen( bool Final )
{
	if( Final )
	{
		m_CourseContentsFrame.SetZoomY( 1 );
		m_CourseContentsFrame.Command( "BounceBegin,0.3;ZoomY,0" );
		OFF_COMMAND( m_CourseContentsFrame );
	}
	else
	{
		COMMAND( m_CourseContentsFrame, "Hide" );
	}
}

void ScreenSelectMusic::SkipSongPartTweens()
{
	m_GrooveRadar.FinishTweening();
	m_GrooveGraph.FinishTweening();
	if( SHOW_DIFFICULTY_LIST )
		m_DifficultyList.FinishTweening();

	for( int p=0; p<NUM_PLAYERS; p++ )
	{		
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		m_sprDifficultyFrame[p].FinishTweening();
		m_sprMeterFrame[p].FinishTweening();
		m_DifficultyIcon[p].FinishTweening();
		m_AutoGenIcon[p].FinishTweening();
	}
}

void ScreenSelectMusic::SkipCoursePartTweens()
{
	m_CourseContentsFrame.FinishTweening();
}

void ScreenSelectMusic::TweenOnScreen()
{
	TweenSongPartsOnScreen( true );
	TweenCoursePartsOnScreen( true );

	switch( GAMESTATE->m_SortOrder )
	{
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
		TweenSongPartsOffScreen( false );
		SkipSongPartTweens();
		break;
	default:
		TweenCoursePartsOffScreen( false );
		SkipCoursePartTweens();
		break;
	}

	ON_COMMAND( m_sprBannerMask );
	ON_COMMAND( m_Banner );
	ON_COMMAND( m_sprBannerFrame );
	ON_COMMAND( m_BPMDisplay );
	ON_COMMAND( m_DifficultyDisplay );
	ON_COMMAND( m_sprStage );
	ON_COMMAND( m_sprCDTitleFront );
	ON_COMMAND( m_sprCDTitleBack );
	ON_COMMAND( m_GrooveRadar );
	ON_COMMAND( m_GrooveGraph );
	ON_COMMAND( m_textSongOptions );
	ON_COMMAND( m_MusicSortDisplay );
	ON_COMMAND( m_MusicWheelUnder );
	m_MusicWheel.TweenOnScreen();
	ON_COMMAND( m_MusicWheel );
	ON_COMMAND( m_Artist );
	ON_COMMAND( m_MachineRank );
	ON_COMMAND( m_Overlay );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{		
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		ON_COMMAND( m_sprCharacterIcon[p] );
		ON_COMMAND( m_OptionIconRow[p] );
		ON_COMMAND( m_sprHighScoreFrame[p] );
		ON_COMMAND( m_textHighScore[p] );
		if( SHOW_PANES )
			ON_COMMAND( m_PaneDisplay[p] );
		ON_COMMAND( m_DifficultyMeter[p] );
	}

	if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
		m_textSongOptions.Command( SONG_OPTIONS_EXTRA_COMMAND );
}

void ScreenSelectMusic::TweenOffScreen()
{
	switch( GAMESTATE->m_SortOrder )
	{
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
		TweenCoursePartsOffScreen( true );
		break;
	default:
		TweenSongPartsOffScreen( true );
		break;
	}

	OFF_COMMAND( m_sprBannerMask );
	OFF_COMMAND( m_Banner );
	OFF_COMMAND( m_sprBannerFrame );
	OFF_COMMAND( m_BPMDisplay );
	OFF_COMMAND( m_DifficultyDisplay );
	OFF_COMMAND( m_sprStage );
	OFF_COMMAND( m_sprCDTitleFront );
	OFF_COMMAND( m_sprCDTitleBack );
	OFF_COMMAND( m_GrooveRadar );
	OFF_COMMAND( m_GrooveGraph );
	OFF_COMMAND( m_textSongOptions );
	OFF_COMMAND( m_MusicSortDisplay );
	m_MusicWheel.TweenOffScreen();
	OFF_COMMAND( m_MusicWheelUnder );
	OFF_COMMAND( m_MusicWheel );
	OFF_COMMAND( m_sprBalloon );
	OFF_COMMAND( m_sprCourseHasMods );
	OFF_COMMAND( m_Artist );
	OFF_COMMAND( m_MachineRank );
	OFF_COMMAND( m_Overlay );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{		
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip

		OFF_COMMAND(m_sprCharacterIcon[p]);
		OFF_COMMAND( m_OptionIconRow[p] );
		OFF_COMMAND( m_sprHighScoreFrame[p] );
		OFF_COMMAND( m_textHighScore[p] );
		if( SHOW_PANES )
			OFF_COMMAND( m_PaneDisplay[p] );
		OFF_COMMAND( m_DifficultyMeter[p] );
	}
}


/* This hides elements that are only relevant when displaying a single song,
 * and shows elements for course display.  XXX: Allow different tween commands. */
void ScreenSelectMusic::SwitchDisplayMode( DisplayMode dm )
{
	if( m_DisplayMode == dm )
		return;

	// tween off
	switch( m_DisplayMode )
	{
	case DISPLAY_SONGS:
		TweenSongPartsOffScreen( false );
		break;
	case DISPLAY_COURSES:
		TweenCoursePartsOffScreen( false );
		break;
	case DISPLAY_MODES:
		break;
	}

	// tween on
	m_DisplayMode = dm;
	switch( m_DisplayMode )
	{
	case DISPLAY_SONGS:
		TweenSongPartsOnScreen( false );
		break;
	case DISPLAY_COURSES:
		TweenCoursePartsOnScreen( false );
		break;
	case DISPLAY_MODES:
		break;
	}
}

void ScreenSelectMusic::TweenScoreOnAndOffAfterChangeSort()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer(p) )
			continue;	// skip
		m_textHighScore[p].Command( SCORE_SORT_CHANGE_COMMAND(p) );
		m_sprHighScoreFrame[p].Command( SCORE_FRAME_SORT_CHANGE_COMMAND(p) );
	}

	switch( GAMESTATE->m_SortOrder )
	{
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
		SwitchDisplayMode( DISPLAY_COURSES );
		break;
	case SORT_SORT_MENU:
	case SORT_MODE_MENU:
		SwitchDisplayMode( DISPLAY_MODES );
		break;
	default:
		SwitchDisplayMode( DISPLAY_SONGS );
		break;
	}
}

void ScreenSelectMusic::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );
	m_bgOptionsOut.Update( fDeltaTime );
	m_bgNoOptionsOut.Update( fDeltaTime );
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
				SOUND->PlayMusic(
					m_sSampleMusicToPlay, 
					m_sSampleMusicTimingData,
					true,
					m_fSampleStartSeconds,
					m_fSampleLengthSeconds,
					1.5f, /* fade out for 1.5 seconds */
					ALIGN_MUSIC_BEATS );
			}
		}
	}
}

void ScreenSelectMusic::Input( const DeviceInput& DeviceI, InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	LOG->Trace( "ScreenSelectMusic::Input()" );

	// debugging?
	if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == SDLK_F9 )
	{
		if( type != IET_FIRST_PRESS ) return;
		PREFSMAN->m_bShowNative ^= 1;
		m_MusicWheel.RebuildMusicWheelItems();
		Course* pCourse = m_MusicWheel.GetSelectedCourse();
		if(pCourse)
			m_CourseContentsFrame.SetFromCourse( pCourse );
		return;
	}

	if( !GameI.IsValid() )		return;		// don't care


	/* XXX: What's the difference between this and StyleI.player? */
	/* StyleI won't be valid if it's a menu button that's pressed.  
	 * There's got to be a better way of doing this.  -Chris */
	PlayerNumber pn = GAMESTATE->GetCurrentStyleDef()->ControllerToPlayerNumber( GameI.controller );
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;

	if( m_bMadeChoice  &&
		MenuI.IsValid()  &&
		MenuI.button == MENU_BUTTON_START  &&
		type != IET_RELEASE  &&
		IsTransitioning() &&
		!GAMESTATE->IsExtraStage()  &&
		!GAMESTATE->IsExtraStage2() )
	{
		if(m_bGoToOptions) return; /* got it already */
		if(!m_bAllowOptionsMenu) return; /* not allowed */

		if( !m_bAllowOptionsMenuRepeat &&
			(type == IET_SLOW_REPEAT || type == IET_FAST_REPEAT ))
			return; /* not allowed yet */
		
		m_bGoToOptions = true;
		m_sprOptionsMessage.SetState( 1 );
		SCREENMAN->PlayStartSound();
		return;
	}

	if( IsTransitioning() )
		return;		// ignore

	if( m_bMadeChoice )		return;		// ignore

	if( MenuI.button == MENU_BUTTON_RIGHT || MenuI.button == MENU_BUTTON_LEFT )
	{

		/* If we're rouletting, hands off. */
		if(m_MusicWheel.IsRouletting())
			return;

		// TRICKY:  There's lots of weirdness that can happen here when tapping 
		// Left and Right quickly, like when changing sort.
		bool bLeftPressed = INPUTMAPPER->IsButtonDown( MenuInput(MenuI.player, MENU_BUTTON_LEFT) );
		bool bRightPressed = INPUTMAPPER->IsButtonDown( MenuInput(MenuI.player, MENU_BUTTON_RIGHT) );
		bool bLeftAndRightPressed = bLeftPressed && bRightPressed;
		bool bLeftOrRightPressed = bLeftPressed || bRightPressed;

		switch( type )
		{
		case IET_RELEASE:
			// when a key is released, stop moving the wheel
			if( !bLeftOrRightPressed )
				m_MusicWheel.Move( 0 );
			
			// Reset the repeat timer when a key is released.
			// This fixes jumping when you release Left and Right at the same 
			// time (e.g. after tapping Left+Right to change sort).
			INPUTMAPPER->ResetKeyRepeat( MenuInput(MenuI.player, MENU_BUTTON_LEFT) );
			INPUTMAPPER->ResetKeyRepeat( MenuInput(MenuI.player, MENU_BUTTON_RIGHT) );
			break;
		case IET_FIRST_PRESS:
			if( MenuI.button == MENU_BUTTON_RIGHT )
				m_MusicWheel.Move( +1 );
			else
				m_MusicWheel.Move( -1 );

			// The wheel moves faster than one item between FIRST_PRESS
			// and SLOW_REPEAT.  Stop the wheel immediately after moving one 
			// item if both Left and Right are held.  This way, we won't move
			// another item 
			if( bLeftAndRightPressed )
				m_MusicWheel.Move( 0 );
			break;
		case IET_SLOW_REPEAT:
		case IET_FAST_REPEAT:
			// We need to handle the repeat events to start the wheel spinning again
			// when Left and Right are being held, then one is released.
			if( bLeftAndRightPressed )
			{
				// Don't spin if holding both buttons
				m_MusicWheel.Move( 0 );
			}
			else
			{
				if( MenuI.button == MENU_BUTTON_RIGHT )
					m_MusicWheel.Move( +1 );
				else
					m_MusicWheel.Move( -1 );
			}
			break;
		}
	}



	// TRICKY:  Do default processing of MenuLeft and MenuRight before detecting 
	// codes.  Do default processing of Start AFTER detecting codes.  This gives us a 
	// change to return if Start is part of a code because we don't want to process 
	// Start as "move to the next screen" if it was just part of a code.
	switch( MenuI.button )
	{
	case MENU_BUTTON_UP:	this->MenuUp( MenuI.player, type );		break;
	case MENU_BUTTON_DOWN:	this->MenuDown( MenuI.player, type );	break;
	case MENU_BUTTON_LEFT:	this->MenuLeft( MenuI.player, type );	break;
	case MENU_BUTTON_RIGHT:	this->MenuRight( MenuI.player, type );	break;
	case MENU_BUTTON_BACK:
		/* Don't make the user hold the back button if they're pressing escape and escape is the back button. */
		if( DeviceI.device == DEVICE_KEYBOARD  &&  DeviceI.button == SDLK_ESCAPE )
			this->MenuBack( MenuI.player );
		else
			Screen::MenuBack( MenuI.player, type );
		break;
	// Do the default handler for Start after detecting codes.
//	case MENU_BUTTON_START:	this->MenuStart( MenuI.player, type );	break;
	case MENU_BUTTON_COIN:	this->MenuCoin( MenuI.player, type );	break;
	}


	if( type == IET_FIRST_PRESS )
	{
		if( CodeDetector::EnteredEasierDifficulty(GameI.controller) )
		{
			if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
				m_soundLocked.Play();
			else
				ChangeDifficulty( pn, -1 );
			return;
		}
		if( CodeDetector::EnteredHarderDifficulty(GameI.controller) )
		{
			if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
				m_soundLocked.Play();
			else
				ChangeDifficulty( pn, +1 );
			return;
		}
		if( CodeDetector::EnteredSortMenu(GameI.controller) )
		{
			/* Ignore the SortMenu when in course mode.  However, still check for the code, so
			 * if people try pressing left+right+start in course mode, we don't pick the selected
			 * course on them. */
			if( GAMESTATE->IsCourseMode() )
				; /* nothing */
			else if( ( GAMESTATE->IsExtraStage() && !PREFSMAN->m_bPickExtraStage ) || GAMESTATE->IsExtraStage2() )
				m_soundLocked.Play();
			else
				m_MusicWheel.ChangeSort( SORT_SORT_MENU );
			return;
		}
		if( CodeDetector::EnteredModeMenu(GameI.controller) )
		{
			if( ( GAMESTATE->IsExtraStage() && !PREFSMAN->m_bPickExtraStage ) || GAMESTATE->IsExtraStage2() )
				m_soundLocked.Play();
			else
				m_MusicWheel.ChangeSort( SORT_MODE_MENU );
			return;
		}
		if( CodeDetector::EnteredNextSort(GameI.controller) )
		{
			if( ( GAMESTATE->IsExtraStage() && !PREFSMAN->m_bPickExtraStage ) || GAMESTATE->IsExtraStage2() )
				m_soundLocked.Play();
			else
				m_MusicWheel.NextSort();
			return;
		}
		if( !GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2() && CodeDetector::DetectAndAdjustMusicOptions(GameI.controller) )
		{
			m_soundOptionsChange.Play();
			UpdateOptionsDisplays();
			return;
		}
	}

	switch( MenuI.button )
	{
	case MENU_BUTTON_START:	Screen::MenuStart( MenuI.player, type );	break;
	}
}

void ScreenSelectMusic::ChangeDifficulty( PlayerNumber pn, int dir )
{
	LOG->Trace( "ScreenSelectMusic::ChangeDifficulty( %d, %d )", pn, dir );

	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;

	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SONG:
		if( dir > 0 && m_iSelection[pn] == int(m_arrayNotes.size()-1) )
			return;
		if( dir < 0 && m_iSelection[pn] == 0 )
			return;

		m_iSelection[pn] += dir;
		
		// the user explicity switched difficulties.  Update the preferred difficulty
		GAMESTATE->ChangeDifficulty( pn, m_arrayNotes[ m_iSelection[pn] ]->GetDifficulty() );

		if( dir < 0 )
			m_soundDifficultyEasier.Play();
		else
			m_soundDifficultyHarder.Play();

		{
			FOREACH_HumanPlayer( p )
			{
				if( pn == p || GAMESTATE->DifficultiesLocked() )
				{
					m_iSelection[p] = m_iSelection[pn];
					AfterNotesChange( p );
				}
			}
		}
		break;

	case TYPE_COURSE:
		if( GAMESTATE->ChangeCourseDifficulty( pn, dir ) )
		{
			if( dir < 0 )
				m_soundDifficultyEasier.Play();
			else
				m_soundDifficultyHarder.Play();
			AfterMusicChange();
		}
		break;

	case TYPE_RANDOM:
	case TYPE_ROULETTE:
	case TYPE_LEAP:
		if( GAMESTATE->ChangeDifficulty( pn, dir ) )
		{
			if( dir < 0 )
				m_soundDifficultyEasier.Play();
			else
				m_soundDifficultyHarder.Play();
			AfterMusicChange();
		}
		break;
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
			m_MenuTimer->SetSeconds( 15 );
			m_MenuTimer->Start();
		}
		else if( DO_ROULETTE_ON_MENU_TIMER )
		{
			if( m_MusicWheel.GetSelectedType() != TYPE_SONG )
			{
				m_MusicWheel.StartRoulette();
				m_MenuTimer->SetSeconds( 15 );
				m_MenuTimer->Start();
			}
			else
			{
				MenuStart(PLAYER_INVALID);
			}
		}
		else
		{
			if( m_MusicWheel.GetSelectedType() != TYPE_SONG && m_MusicWheel.GetSelectedType() != TYPE_COURSE )
				m_MusicWheel.StartRandom();
			MenuStart(PLAYER_INVALID);
		}
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( PREV_SCREEN );
		/* We may have stray SM_SongChanged messages from the music wheel.  We can't
		 * handle them anymore, since the title menu (and attract screens) reset
		 * the game state, so just discard them. */
		ClearMessageQueue();
		break;
	case SM_BeginFadingOut:
		/* XXX: yuck.  Later on, maybe this can be done in one BGA with lua ... */
		if( m_bGoToOptions )
			m_bgOptionsOut.StartTransitioning( SM_GoToNextScreen );
		else
			m_bgNoOptionsOut.StartTransitioning( SM_GoToNextScreen );
		break;
	case SM_GoToNextScreen:
		if( m_bGoToOptions )
		{
			SCREENMAN->SetNewScreen( NEXT_OPTIONS_SCREEN );
		}
		else
		{
			GAMESTATE->AdjustFailType();
			SOUND->StopMusic();
			SCREENMAN->SetNewScreen( NEXT_SCREEN );
		}
		break;
	case SM_SongChanged:
		AfterMusicChange();
		break;
	case SM_SortOrderChanging: /* happens immediately */
//				m_MusicSortDisplay.FadeOff( 0, "fade", TWEEN_TIME );
		TweenScoreOnAndOffAfterChangeSort();
		break;
	case SM_SortOrderChanged: /* happens after the wheel is off and the new song is selected */
		SortOrderChanged();
		break;
	}
}

void ScreenSelectMusic::MenuStart( PlayerNumber pn )
{
	// this needs to check whether valid Steps are selected!
	bool bResult = m_MusicWheel.Select();

	/* If false, we don't have a selection just yet. */
	if( !bResult )
		return;

	// a song was selected
	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SONG: {
		const bool bIsNew = PROFILEMAN->IsSongNew( m_MusicWheel.GetSelectedSong() );
		bool bIsHard = false;
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsHumanPlayer( (PlayerNumber)p ) )
				continue;	// skip
			if( GAMESTATE->m_pCurNotes[p]  &&  GAMESTATE->m_pCurNotes[p]->GetMeter() >= 10 )
				bIsHard = true;
		}

		/* See if this song is a repeat.  If we're in event mode, only check the last five songs. */
		bool bIsRepeat = false;
		int i = 0;
		if( PREFSMAN->m_bEventMode )
			i = max( 0, int(g_vPlayedStageStats.size())-5 );
		for( ; i < (int)g_vPlayedStageStats.size(); ++i )
			if( g_vPlayedStageStats[i].pSong == m_MusicWheel.GetSelectedSong() )
				bIsRepeat = true;

		/* Don't complain about repeats if the user didn't get to pick. */
		if( GAMESTATE->IsExtraStage() && !PREFSMAN->m_bPickExtraStage )
			bIsRepeat = false;

		if( bIsRepeat )
			SOUND->PlayOnceFromAnnouncer( "select music comment repeat" );
		else if( bIsNew )
			SOUND->PlayOnceFromAnnouncer( "select music comment new" );
		else if( bIsHard )
			SOUND->PlayOnceFromAnnouncer( "select music comment hard" );
		else
			SOUND->PlayOnceFromAnnouncer( "select music comment general" );

		m_bMadeChoice = true;

		/* If we're in event mode, we may have just played a course (putting us
		 * in course mode).  Make sure we're in a single song mode. */
		if( GAMESTATE->IsCourseMode() )
			GAMESTATE->m_PlayMode = PLAY_MODE_ARCADE;

		break;
	}
	case TYPE_COURSE:
	{
		SOUND->PlayOnceFromAnnouncer( "select course comment general" );

		Course *pCourse = m_MusicWheel.GetSelectedCourse();
		ASSERT( pCourse );
		GAMESTATE->m_PlayMode = pCourse->GetPlayMode();

		// apply #LIVES
		if( pCourse->m_iLives != -1 )
		{
			GAMESTATE->m_SongOptions.m_LifeType = SongOptions::LIFE_BATTERY;
			GAMESTATE->m_SongOptions.m_iBatteryLives = pCourse->m_iLives;
		}

		m_bMadeChoice = true;

		break;
	}
	case TYPE_SECTION:
	case TYPE_ROULETTE:
	case TYPE_SORT:
		break;
	default:
		ASSERT(0);
	}

	if( m_bMadeChoice )
	{
		TweenOffScreen();
		SCREENMAN->PlayStartSound();

		if( !GAMESTATE->IsExtraStage()  &&  !GAMESTATE->IsExtraStage2() )
		{
//			float fShowSeconds = m_Out.GetLengthSeconds();

			// show "hold START for options"
			m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,1) );	// visible
			SET_XY_AND_ON_COMMAND( m_sprOptionsMessage );

			m_bAllowOptionsMenu = true;
			/* Don't accept a held START for a little while, so it's not
			 * hit accidentally.  Accept an initial START right away, though,
			 * so we don't ignore deliberate fast presses (which would be
			 * annoying). */
			this->PostScreenMessage( SM_AllowOptionsMenuRepeat, 0.5f );
		}

		StartTransitioning( SM_BeginFadingOut );
	}

	if( GAMESTATE->IsExtraStage() && PREFSMAN->m_bPickExtraStage )
	{
		/* Check if user selected the real extra stage. */
		Song* pSong;
		Steps* pNotes;
		PlayerOptions po;
		SongOptions so;
		SONGMAN->GetExtraStageInfo( false, GAMESTATE->GetCurrentStyleDef(), pSong, pNotes, po, so );
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
	SOUND->StopMusic();

	Back( SM_GoToPrevScreen );
}

void ScreenSelectMusic::AfterNotesChange( PlayerNumber pn )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;
	
	m_iSelection[pn] = clamp( m_iSelection[pn], 0, int(m_arrayNotes.size()-1) );	// bounds clamping

	Song* pSong = GAMESTATE->m_pCurSong;
	Steps* pSteps = m_arrayNotes.empty()? NULL: m_arrayNotes[m_iSelection[pn]];

	GAMESTATE->m_pCurNotes[pn] = pSteps;

	if( pSteps )
	{
		int iScore = 0;
		if( PROFILEMAN->IsUsingProfile(pn) )
			iScore = PROFILEMAN->GetProfile(pn)->GetStepsHighScoreList(pSong,pSteps).GetTopScore().iScore;
		else
			iScore = PROFILEMAN->GetMachineProfile()->GetStepsHighScoreList(pSong,pSteps).GetTopScore().iScore;
		m_textHighScore[pn].SetText( ssprintf("%*i", NUM_SCORE_DIGITS, iScore) );
	}
	else
	{
		m_textHighScore[pn].SetText( ssprintf("%*i", NUM_SCORE_DIGITS, 0) );
	}

	m_DifficultyIcon[pn].SetFromNotes( pn, pSteps );
	if( pSteps && pSteps->IsAutogen() )
	{
		m_AutoGenIcon[pn].SetEffectDiffuseShift();
	}
	else
	{
		m_AutoGenIcon[pn].SetEffectNone();
		m_AutoGenIcon[pn].SetDiffuse( RageColor(1,1,1,0) );
	}
	m_DifficultyMeter[pn].SetFromGameState( pn );
	if( SHOW_DIFFICULTY_LIST )
		m_DifficultyList.SetFromGameState();
	m_GrooveRadar.SetFromNotes( pn, pSteps );
	m_MusicWheel.NotesChanged( pn );
	if( SHOW_PANES )
		m_PaneDisplay[pn].SetFromGameState();
}

void ScreenSelectMusic::SwitchToPreferredDifficulty()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMESTATE->IsHumanPlayer( PlayerNumber(p) ) )
			continue;

		/* Find the closest match to the user's preferred difficulty. */
		int CurDifference = -1;
		for( unsigned i=0; i<m_arrayNotes.size(); i++ )
		{
			int Diff = abs(m_arrayNotes[i]->GetDifficulty() - GAMESTATE->m_PreferredDifficulty[p]);

			if( CurDifference == -1 || Diff < CurDifference )
			{
				m_iSelection[p] = i;
				CurDifference = Diff;
			}
		}

		m_iSelection[p] = clamp( m_iSelection[p], 0, int(m_arrayNotes.size()) ) ;
	}
}

template<class T>
int FindCourseIndexOfSameMode( T begin, T end, const Course *p )
{
	const PlayMode pm = p->GetPlayMode();
	
	int n = 0;
	for( T it = begin; it != end; ++it )
	{
		if( *it == p )
			return n;

		/* If it's not playable in this mode, don't increment.  It might result in 
		 * different output in different modes, but that's better than having holes. */
		if( !(*it)->IsPlayableIn( GAMESTATE->GetCurrentStyleDef()->m_StepsType ) )
			continue;
		if( (*it)->GetPlayMode() != pm )
			continue;
		++n;
	}

	return -1;
}

void ScreenSelectMusic::AfterMusicChange()
{
	if( !m_MusicWheel.IsRouletting() )
		m_MenuTimer->Stall();

	// lock difficulties.  When switching from arcade to rave, we need to 
	// enforce that all players are at the same difficulty.
	if( GAMESTATE->DifficultiesLocked() )
	{
		FOREACH_HumanPlayer( p )
		{
			m_iSelection[p] = m_iSelection[0];
			GAMESTATE->m_PreferredDifficulty[p] = GAMESTATE->m_PreferredDifficulty[0];
		}
	}

	Song* pSong = m_MusicWheel.GetSelectedSong();
	GAMESTATE->m_pCurSong = pSong;

	m_GrooveGraph.SetFromSong( pSong );

	Course* pCourse = m_MusicWheel.GetSelectedCourse();
	if( pCourse )
		GAMESTATE->m_pCurCourse = pCourse;

	int pn;
	for( pn = 0; pn < NUM_PLAYERS; ++pn)
		m_arrayNotes.clear();

	m_Banner.SetMovingFast( !!m_MusicWheel.IsMoving() );

	CString SampleMusicToPlay, SampleMusicTimingData;
	vector<CString> m_Artists, m_AltArtists;

	m_MachineRank.SetText( "" );
	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SECTION:
	case TYPE_SORT:
		{	
			CString sGroup = m_MusicWheel.GetSelectedSection();
			for( int p=0; p<NUM_PLAYERS; p++ )
				m_iSelection[p] = -1;

			m_BPMDisplay.NoBPM();
			m_sprCDTitleFront.UnloadTexture();
			m_sprCDTitleBack.UnloadTexture();
			m_DifficultyDisplay.UnsetDifficulties();

			m_fSampleStartSeconds = 0;
			m_fSampleLengthSeconds = -1;
			switch( m_MusicWheel.GetSelectedType() )
			{
			case TYPE_SECTION:
				m_Banner.LoadFromGroup( sGroup );	// if this isn't a group, it'll default to the fallback banner
				SampleMusicToPlay = THEME->GetPathToS("ScreenSelectMusic section music");
				break;
			case TYPE_SORT:
				switch( GAMESTATE->m_SortOrder )
				{
				case SORT_SORT_MENU:
					m_Banner.LoadSort();
					break;
				case SORT_MODE_MENU:
					m_Banner.LoadMode();
					break;
				}
				SampleMusicToPlay = THEME->GetPathToS("ScreenSelectMusic sort music");
				break;
			default:
				ASSERT(0);
			}

			m_sprBalloon.StopTweening();
			OFF_COMMAND( m_sprBalloon );
			
			m_sprCourseHasMods->StopTweening();
			OFF_COMMAND( m_sprCourseHasMods );
		}
		break;
	case TYPE_SONG:
		{
			SampleMusicToPlay = pSong->GetMusicPath();
			SampleMusicTimingData = pSong->GetCacheFilePath();
			m_fSampleStartSeconds = pSong->m_fMusicSampleStartSeconds;
			m_fSampleLengthSeconds = pSong->m_fMusicSampleLengthSeconds;

			pSong->GetSteps( m_arrayNotes, GAMESTATE->GetCurrentStyleDef()->m_StepsType );
			StepsUtil::SortNotesArrayByDifficulty( m_arrayNotes );

			if ( PREFSMAN->m_bShowBanners )
				m_Banner.LoadFromSong( pSong );
			else
				m_Banner.LoadFallback() ;

			if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			{
				m_BPMDisplay.CycleRandomly();				
			}
			else
			{
				m_BPMDisplay.SetBPM( pSong );
			}

			const CString CDTitlePath = pSong->HasCDTitle()? pSong->GetCDTitlePath():g_sFallbackCDTitlePath;
			TEXTUREMAN->DisableOddDimensionWarning();
			m_sprCDTitleFront.Load( CDTitlePath );
			m_sprCDTitleBack.Load( CDTitlePath );
			TEXTUREMAN->EnableOddDimensionWarning();
			FlipSpriteHorizontally(m_sprCDTitleBack);

			const vector<Song*> best = SONGMAN->GetBestSongs( PROFILE_SLOT_MACHINE );
			const int index = FindIndex( best.begin(), best.end(), pSong );
			if( index != -1 )
				m_MachineRank.SetText( ssprintf("%i", index+1) );

			m_DifficultyDisplay.SetDifficulties( pSong, GAMESTATE->GetCurrentStyleDef()->m_StepsType );

			SwitchToPreferredDifficulty();

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

			m_sprCourseHasMods->StopTweening();
			OFF_COMMAND( m_sprCourseHasMods );

			m_Artists.push_back( pSong->GetDisplayArtist() );
			m_AltArtists.push_back( pSong->GetTranslitArtist() );
		}
		break;
	case TYPE_ROULETTE:
	case TYPE_RANDOM:
	case TYPE_LEAP:
		switch(m_MusicWheel.GetSelectedType())
		{
		case TYPE_ROULETTE:	m_Banner.LoadRoulette();	break;
		case TYPE_RANDOM: 	m_Banner.LoadRandom();		break;
		case TYPE_LEAP: 	m_Banner.LoadLeap();		break;
		default: ASSERT(0);
		}

		m_BPMDisplay.NoBPM();
		m_sprCDTitleFront.UnloadTexture();
		m_sprCDTitleBack.UnloadTexture();
		m_DifficultyDisplay.UnsetDifficulties();

		m_fSampleStartSeconds = 0;
		m_fSampleLengthSeconds = -1;
		switch( m_MusicWheel.GetSelectedType() )
		{
		case TYPE_ROULETTE:
			SampleMusicToPlay = THEME->GetPathToS("ScreenSelectMusic roulette music");
			break;
		case TYPE_RANDOM:
			SampleMusicToPlay = THEME->GetPathToS("ScreenSelectMusic random music");
			break;
		case TYPE_LEAP:
			SampleMusicToPlay = THEME->GetPathToS("ScreenSelectMusic leap music");
			break;
		default:
			ASSERT(0);
		}

		m_sprBalloon.StopTweening();
		OFF_COMMAND( m_sprBalloon );
		
		m_sprCourseHasMods->StopTweening();
		OFF_COMMAND( m_sprCourseHasMods );
		
		break;
	case TYPE_COURSE:
	{
		Course* pCourse = m_MusicWheel.GetSelectedCourse();

		SampleMusicToPlay = THEME->GetPathToS("ScreenSelectMusic course music");
		m_fSampleStartSeconds = 0;
		m_fSampleLengthSeconds = -1;

//		m_textNumSongs.SetText( ssprintf("%d", pCourse->GetEstimatedNumStages()) );
//		float fTotalSeconds;
//		if( pCourse->GetTotalSeconds(fTotalSeconds) )
//			m_textTime.SetText( SecondsToTime(fTotalSeconds) );
//		else
//			m_textTime.SetText( "xx:xx:xx" );	// The numbers format doesn't have a '?'.  Is there a better solution?

		m_Banner.LoadFromCourse( pCourse );
		m_BPMDisplay.SetBPM( pCourse );

		m_CourseContentsFrame.SetFromCourse( pCourse );
		m_CourseContentsFrame.TweenInAfterChangedCourse();
		m_DifficultyDisplay.UnsetDifficulties();

		vector<Course::Info> ci;
		pCourse->GetCourseInfo( GAMESTATE->GetCurrentStyleDef()->m_StepsType, ci );

		for( unsigned i = 0; i < ci.size(); ++i )
		{
			if( ci[i].Mystery )
			{
				m_Artists.push_back( "???" );
				m_AltArtists.push_back( "???" );
			} else {
				m_Artists.push_back( ci[i].pSong->GetDisplayArtist() );
				m_AltArtists.push_back( ci[i].pSong->GetTranslitArtist() );
			}
		}

		const vector<Course*> best = SONGMAN->GetBestCourses( PROFILE_SLOT_MACHINE );
		const int index = FindCourseIndexOfSameMode( best.begin(), best.end(), pCourse );
		if( index != -1 )
			m_MachineRank.SetText( ssprintf("%i", index+1) );



		m_sprBalloon.StopTweening();
		OFF_COMMAND( m_sprBalloon );

		if( pCourse->HasMods() )
		{
			m_sprCourseHasMods->StopTweening();
			SET_XY_AND_ON_COMMAND( m_sprCourseHasMods );
		}
		else
		{
			m_sprCourseHasMods->StopTweening();
			OFF_COMMAND( m_sprCourseHasMods );
		}

		break;
	}
	default:
		ASSERT(0);
	}

	// update stage counter display (long versions/marathons)
	m_sprStage.Load( THEME->GetPathToG("ScreenSelectMusic stage "+GAMESTATE->GetStageText()) );

	// Don't stop music if it's already playing the right file.
	if( SampleMusicToPlay == "" )
		SOUND->StopMusic();
	else if( SOUND->GetMusicPath() != SampleMusicToPlay )
	{
		SOUND->StopMusic();
		m_sSampleMusicToPlay = SampleMusicToPlay;
		m_sSampleMusicTimingData = SampleMusicTimingData;
		m_fPlaySampleCountdown = SAMPLE_MUSIC_DELAY;
	}

	m_Artist.SetTips( m_Artists, m_AltArtists );

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
		if( GAMESTATE->IsHumanPlayer(p) )
		{
			m_OptionIconRow[p].Refresh();

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
	m_MusicSortDisplay.Set( GAMESTATE->m_SortOrder );

	switch( GAMESTATE->m_SortOrder )
	{
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
	case SORT_SORT_MENU:
	case SORT_MODE_MENU:
		// do nothing
		break;
	default:
		if( SHOW_DIFFICULTY_LIST )
			m_DifficultyList.Show();
		break;
	}

	// tween music sort on screen
//	m_MusicSortDisplay.FadeOn( 0, "fade", TWEEN_TIME );
}

