#include "global.h"
#include "ScreenEz2SelectMusic.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "Style.h"
#include "InputMapper.h"
#include "CodeDetector.h"
#include "Steps.h"
#include "RageTimer.h"
#include "ActorUtil.h"
#include "AnnouncerManager.h"
#include "MenuTimer.h"
#include "SongUtil.h"
#include "StepsUtil.h"
#include "ScreenDimensions.h"
#include "ScreenPrompt.h"
#include "PlayerState.h"
#include "RageLog.h"
#include "song.h"
#include "InputEventPlus.h"
#include "LocalizedString.h"

#define PREV_SCREEN				THEME->GetMetric ("ScreenEz2SelectMusic","PrevScreen")
#define SCROLLING_LIST_X		THEME->GetMetricF("ScreenEz2SelectMusic","ScrollingListX")
#define SCROLLING_LIST_Y		THEME->GetMetricF("ScreenEz2SelectMusic","ScrollingListY")
#define SCROLLING_LIST_ROT		THEME->GetMetricF("ScreenEz2SelectMusic","ScrollingListRotation")
#define PUMP_DIFF_X				THEME->GetMetricF("ScreenEz2SelectMusic","PumpDifficultyX")
#define PUMP_DIFF_Y				THEME->GetMetricF("ScreenEz2SelectMusic","PumpDifficultyY")
#define HELP_TEXT				THEME->GetString("ScreenSelectMusic","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenSelectMusic","TimerSeconds")
#define METER_X( p )			THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MeterP%dX",p+1))
#define METER_Y( p )			THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MeterP%dY",p+1))
#define GUIDE_X					THEME->GetMetricF("ScreenSelectMode","GuideX")
#define GUIDE_Y					THEME->GetMetricF("ScreenSelectMode","GuideY")
#define GROUPNAME_X				THEME->GetMetricF("ScreenEz2SelectMusic","GroupNameX")
#define GROUPNAME_Y				THEME->GetMetricF("ScreenEz2SelectMusic","GroupNameY")
#define SPEEDICON_X( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("SpeedIconP%dX",p+1))
#define SPEEDICON_Y( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("SpeedIconP%dY",p+1))
#define MIRRORICON_X( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MirrorIconP%dX",p+1))
#define MIRRORICON_Y( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MirrorIconP%dY",p+1))
#define HIDDENICON_X( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("HiddenIconP%dX",p+1))
#define HIDDENICON_Y( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("HiddenIconP%dY",p+1))
#define VANISHICON_X( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("VanishIconP%dX",p+1))
#define VANISHICON_Y( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("VanishIconP%dY",p+1))
#define SHUFFLEICON_X( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("ShuffleIconP%dX",p+1))
#define SHUFFLEICON_Y( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("ShuffleIconP%dY",p+1))
#define PREVIEWMUSICMODE		THEME->GetMetricI("ScreenEz2SelectMusic","PreviewMusicMode")
#define DIFFICULTYRATING_X		THEME->GetMetricF("ScreenEz2SelectMusic","DifficultyRatingX")
#define DIFFICULTYRATING_Y		THEME->GetMetricF("ScreenEz2SelectMusic","DifficultyRatingY")
#define DIFFICULTYRATING_ORIENTATION		THEME->GetMetricI("ScreenEz2SelectMusic","DifficultyRatingOrientation")
#define INFOFRAME_X		THEME->GetMetricF("ScreenEz2SelectMusic","InfoFrameX")
#define INFOFRAME_Y		THEME->GetMetricF("ScreenEz2SelectMusic","InfoFrameY")

#define ARTIST_X				THEME->GetMetricF("ScreenEz2SelectMusic","ArtistX")
#define ARTIST_Y				THEME->GetMetricF("ScreenEz2SelectMusic","ArtistY")
#define TITLE_X				THEME->GetMetricF("ScreenEz2SelectMusic","TitleX")
#define TITLE_Y				THEME->GetMetricF("ScreenEz2SelectMusic","TitleY")
#define SUBTITLE_X				THEME->GetMetricF("ScreenEz2SelectMusic","SubTitleX")
#define SUBTITLE_Y				THEME->GetMetricF("ScreenEz2SelectMusic","SubTitleY")


#define USE_MODE_SWITCHER THEME->GetMetricI("ScreenEz2SelectMusic","UseModeSwitcher")

const float TWEEN_TIME		= 0.5f;

AutoScreenMessage( SM_NoSongs )

REGISTER_SCREEN_CLASS( ScreenEz2SelectMusic );

void ScreenEz2SelectMusic::Init()
{
	/* Finish any previous stage.  It's OK to call this when we havn't played a stage yet. */
	GAMESTATE->FinishStage();

	ScreenWithMenuElements::Init();

	i_SkipAheadOffset = 0;
	LastInputTime = 0;
	ScrollStartTime = 0;
	m_bTransitioning = false;
	m_bScanning = false;
	m_fRemainingWaitTime = 0.0f;
	i_ErrorDetected=0;

	if(PREVIEWMUSICMODE == 4)
	{
		m_soundBackMusic.Load( THEME->GetPathS("ScreenEz2SelectMusic","music"));
		m_soundBackMusic.Play();
	}

	if(PREVIEWMUSICMODE == 1 || PREVIEWMUSICMODE == 3)
	{
		if(PREVIEWMUSICMODE == 1)
			SOUND->StopMusic();
		iConfirmSelection = 0;
	}



	m_soundButtonPress.Load( THEME->GetPathS("ScreenEz2SelectMusic","buttonpress"));
	m_soundMusicChange.Load( THEME->GetPathS("ScreenEz2SelectMusic","change"));
	m_soundMusicCycle.Load( THEME->GetPathS("ScreenEz2SelectMusic","cycle"));

	m_ChoiceListFrame.Load( THEME->GetPathG("ScreenEz2SelectMusic","list frame"));
	m_ChoiceListFrame.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y);
	this->AddChild( &m_ChoiceListFrame );

	m_MusicBannerWheel.SetX(SCROLLING_LIST_X);
	m_MusicBannerWheel.SetY(SCROLLING_LIST_Y);
	m_MusicBannerWheel.SetRotationZ(SCROLLING_LIST_ROT);

	if(m_MusicBannerWheel.CheckSongsExist() != 0)
	{
		this->AddChild( &m_MusicBannerWheel );


		m_ChoiceListHighlight.Load( THEME->GetPathG("ScreenEz2SelectMusic","list highlight"));
		m_ChoiceListHighlight.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y);
		this->AddChild( &m_ChoiceListHighlight );


		FOREACH_PlayerNumber( p )
		{
		//	m_FootMeter[p].SetXY( METER_X(p), METER_Y(p) );
		//	m_FootMeter[p].SetShadowLength( 2 );
		//	this->AddChild( &m_FootMeter[p] );

			m_SpeedIcon[p].Load( THEME->GetPathG("ScreenEz2SelectMusic","speedicon"));
			m_SpeedIcon[p].SetXY( SPEEDICON_X(p), SPEEDICON_Y(p) );
			m_SpeedIcon[p].SetDiffuse( RageColor(0,0,0,0) );
			this->AddChild(&m_SpeedIcon[p] );

			m_MirrorIcon[p].Load( THEME->GetPathG("ScreenEz2SelectMusic","mirroricon"));
			m_MirrorIcon[p].SetXY( MIRRORICON_X(p), MIRRORICON_Y(p) );
			m_MirrorIcon[p].SetDiffuse( RageColor(0,0,0,0) );
			this->AddChild(&m_MirrorIcon[p] );

			m_ShuffleIcon[p].Load( THEME->GetPathG("ScreenEz2SelectMusic","shuffleicon"));
			m_ShuffleIcon[p].SetXY( SHUFFLEICON_X(p), SHUFFLEICON_Y(p) );
			m_ShuffleIcon[p].SetDiffuse( RageColor(0,0,0,0) );
			this->AddChild(&m_ShuffleIcon[p] );

			m_HiddenIcon[p].Load( THEME->GetPathG("ScreenEz2SelectMusic","hiddenicon"));
			m_HiddenIcon[p].SetXY( HIDDENICON_X(p), HIDDENICON_Y(p) );
			m_HiddenIcon[p].SetDiffuse( RageColor(0,0,0,0) );
			this->AddChild(&m_HiddenIcon[p] );

			m_VanishIcon[p].Load( THEME->GetPathG("ScreenEz2SelectMusic","vanishicon"));
			m_VanishIcon[p].SetXY( VANISHICON_X(p), VANISHICON_Y(p) );
			m_VanishIcon[p].SetDiffuse( RageColor(0,0,0,0) );
			this->AddChild(&m_VanishIcon[p] );

			UpdateOptions( p,0);

			m_iSelection[p] = 0;
		}

		m_Guide.Load( THEME->GetPathG("ScreenEz2SelectMusic","guide"));
		m_Guide.SetXY( GUIDE_X, GUIDE_Y );
		this->AddChild( &m_Guide );

		m_InfoFrame.Load( THEME->GetPathG("ScreenEz2SelectMusic","infoframe") );
		m_InfoFrame.SetXY( INFOFRAME_X, INFOFRAME_Y );
		this->AddChild( &m_InfoFrame );

#ifdef _XBOX
		//shorten filenames for FATX
		m_PumpDifficultyCircle.Load( THEME->GetPathG("ScreenEz2SelectMusic","diff frame"));
#else
		m_PumpDifficultyCircle.Load( THEME->GetPathG("ScreenEz2SelectMusic","difficulty frame"));
#endif

		m_PumpDifficultyCircle.SetXY( PUMP_DIFF_X, PUMP_DIFF_Y );
		this->AddChild( &m_PumpDifficultyCircle );

		m_PumpDifficultyRating.LoadFromFont( THEME->GetPathF("ScreenEz2SelectMusic","difficulty") );
		m_PumpDifficultyRating.SetXY( PUMP_DIFF_X, PUMP_DIFF_Y );
		this->AddChild(&m_PumpDifficultyRating);

		m_CurrentGroup.LoadFromFont( THEME->GetPathF("ScreenEz2SelectMusic","GroupName") );
		m_CurrentGroup.SetXY( GROUPNAME_X, GROUPNAME_Y );
		this->AddChild(&m_CurrentGroup );

		m_CurrentTitle.LoadFromFont( THEME->GetPathF("ScreenEz2SelectMusic","GroupName") );
		m_CurrentTitle.SetXY( TITLE_X, TITLE_Y );
		this->AddChild(&m_CurrentTitle );

		m_CurrentSubTitle.LoadFromFont( THEME->GetPathF("ScreenEz2SelectMusic","GroupName") );
		m_CurrentSubTitle.SetXY( SUBTITLE_X, SUBTITLE_Y );
		m_CurrentSubTitle.SetZoom(0.8f);
		this->AddChild(&m_CurrentTitle );

		m_CurrentArtist.LoadFromFont( THEME->GetPathF("ScreenEz2SelectMusic","GroupName") );
		m_CurrentArtist.SetXY( ARTIST_X, ARTIST_Y );
		this->AddChild(&m_CurrentArtist );

		m_DifficultyRating.SetOrientation(DIFFICULTYRATING_ORIENTATION);
		m_DifficultyRating.SetX(DIFFICULTYRATING_X);
		m_DifficultyRating.SetY(DIFFICULTYRATING_Y);
		this->AddChild(&m_DifficultyRating);

		m_sprOptionsMessage.Load( THEME->GetPathG("ScreenEz2SelectMusic","options message") );
		m_sprOptionsMessage.StopAnimating();
		m_sprOptionsMessage.SetXY( SCREEN_CENTER_X, SCREEN_CENTER_Y );
		m_sprOptionsMessage.SetZoom( 1 );
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
		this->AddChild( &m_sprOptionsMessage );

		if(USE_MODE_SWITCHER == 1)
		{
			m_ModeSwitcher.SetXY(SCREEN_CENTER_X,SCREEN_CENTER_Y);
			this->AddChild( &m_ModeSwitcher );
		}

		m_sprBalloon.SetName( "Balloon" );
		m_TexturePreload.Load( THEME->GetPathG("ScreenSelectMusic","balloon long") );
		m_TexturePreload.Load( THEME->GetPathG("ScreenSelectMusic","balloon marathon") );
		this->AddChild( &m_sprBalloon );

		m_soundOptionsChange.Load( THEME->GetPathS("ScreenEz2SelectMusic","options") );

		m_bGoToOptions = false;
		m_bMadeChoice = false;

		MusicChanged();
	}
	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("select music intro") );
}

void ScreenEz2SelectMusic::Input( const InputEventPlus &input )
{
//	if( type != IET_FIRST_PRESS )
//		return;	// ignore

	if(i_ErrorDetected) return; // don't let the user do anything if theres an error

	if( input.type == IET_RELEASE )	return;		// don't care

	if( IsTransitioning() )	return;		// ignore

	if( !input.GameI.IsValid() )		return;		// don't care

	if( m_bMadeChoice && !m_bGoToOptions && input.MenuI == MENU_BUTTON_START )
	{
		SCREENMAN->PlayStartSound();
		m_bGoToOptions = true;
		m_sprOptionsMessage.SetState( 1 );
	}

	if( m_bMadeChoice )
		return;

	PlayerNumber pn = input.pn;

	if( CodeDetector::EnteredEasierDifficulty(input.GameI.controller) )
	{
		EasierDifficulty( pn );
		return;
	}
	if( CodeDetector::EnteredHarderDifficulty(input.GameI.controller) )
	{
		HarderDifficulty( pn );
		return;
	}
	if( CodeDetector::DetectAndAdjustMusicOptions(input.GameI.controller) )
	{
		UpdateOptions(pn,1);
	}

	if( CodeDetector::EnteredNextBannerGroup(input.GameI.controller))
	{
		m_MusicBannerWheel.ScanToNextGroup();
		MusicChanged();
		return;
	}

	if( input.type != IET_FIRST_PRESS )
	{
		m_bScanning = true;
		m_soundMusicCycle.Play();
		i_SkipAheadOffset = 0;
		if(ScrollStartTime == 0)
		{
			i_SkipAheadOffset = 0;
			ScrollStartTime = RageTimer::GetTimeSinceStartFast();
		}
		else
		{
			if(RageTimer::GetTimeSinceStartFast() - ScrollStartTime > 5) // been cycling for more than 5 seconds
			{
				i_SkipAheadOffset = 2; // start skipping ahead in twos
			}
			else if (RageTimer::GetTimeSinceStartFast() - ScrollStartTime > 10) // been cycling for more than 10 seconds
			{
				i_SkipAheadOffset = 3; // start skipping ahead 3 at a time
			}
		}
	}
	else
	{
		m_bScanning = false;
		i_SkipAheadOffset = 0;
		ScrollStartTime = 0;
	//	m_soundMusicChange.Play();
		m_soundButtonPress.Play();
	}
	LastInputTime = RageTimer::GetTimeSinceStartFast();
	Screen::Input( input );
}

void ScreenEz2SelectMusic::UpdateOptions(PlayerNumber pn, int nosound)
{
	sOptions = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetPreferred().GetString();

	LOG->Trace( "DEBUG: " + sOptions );

	asOptions.clear();
	split( sOptions, ", ", asOptions, true );


	if(asOptions.size() > 0) // check it's not empty!
	{
		m_MirrorIcon[pn].SetDiffuse( RageColor(0,0,0,0) );
	    m_ShuffleIcon[pn].SetDiffuse( RageColor(0,0,0,0) );

		for(unsigned i=0; i<asOptions.size(); i++)
		{
			if(asOptions[0] == "2X" || asOptions[0] == "1.5X" || asOptions[0] == "3X" || asOptions[0] == "4X" || asOptions[0] == "5X" || asOptions[0] == "8X" || asOptions[0] == "0.5X" || asOptions[0] == "0.75X")
			{
				m_SpeedIcon[pn].SetDiffuse( RageColor(1,1,1,1) );
			}
			else
			{
				m_SpeedIcon[pn].SetDiffuse( RageColor(0,0,0,0) );
			}

			if(asOptions[i] == "Mirror")
			{
				m_MirrorIcon[pn].SetDiffuse( RageColor(1,1,1,1) );
			}
			else if(asOptions[i] == "Shuffle" || asOptions[i] == "SuperShuffle" )
			{
				m_ShuffleIcon[pn].SetDiffuse( RageColor(1,1,1,1) );
			}
			else if(asOptions[i] == "Hidden")
			{
				m_HiddenIcon[pn].SetDiffuse( RageColor(1,1,1,1) );
			}
			else if(asOptions[i] == "RandomVanish")
			{
				m_VanishIcon[pn].SetDiffuse( RageColor(1,1,1,1) );
			}
		}
	}
	else
	{
		m_SpeedIcon[pn].SetDiffuse( RageColor(0,0,0,0) );
		m_MirrorIcon[pn].SetDiffuse( RageColor(0,0,0,0) );
		m_ShuffleIcon[pn].SetDiffuse( RageColor(0,0,0,0) );
		m_HiddenIcon[pn].SetDiffuse( RageColor(0,0,0,0) );
		m_VanishIcon[pn].SetDiffuse( RageColor(0,0,0,0) );
	}
	if(nosound !=0)
		m_soundOptionsChange.Play();
}

void ScreenEz2SelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	if( SM == SM_GoToNextScreen )
	{
		if( m_bGoToOptions )
		{
			SCREENMAN->SetNewScreen( "ScreenPlayerOptions" );
		}
		else
		{
			SOUND->StopMusic();
			SCREENMAN->SetNewScreen( "ScreenStage" );
		}
	}
	else if( SM == SM_NoSongs )
	{
		SCREENMAN->SetNewScreen( PREV_SCREEN );
	}
}


void ScreenEz2SelectMusic::MenuRight( const InputEventPlus &input )
{
	m_MenuTimer->Stall();
	m_MusicBannerWheel.BannersRight();
	for(int i=i_SkipAheadOffset; i>0; i--)
		m_MusicBannerWheel.BannersRight();
	MusicChanged();
}

void ScreenEz2SelectMusic::MenuBack( const InputEventPlus &input )
{
	SOUND->StopMusic();

	Cancel( SM_GoToPrevScreen );
}


void ScreenEz2SelectMusic::TweenOffScreen()
{
	apActorCommands cmds = ActorUtil::ParseActorCommands( "linear,0.5;zoomy,0" );
	m_MusicBannerWheel.RunCommands(		 cmds );

	apActorCommands cmds2 = ActorUtil::ParseActorCommands( "Linear,1;DiffuseAlpha,0" );
	m_PumpDifficultyCircle.RunCommands( cmds2 );
	m_Guide.RunCommands(				cmds2 );
	m_PumpDifficultyRating.RunCommands( cmds2 );
	m_Guide.RunCommands(				cmds2 );
	m_ChoiceListFrame.RunCommands(		cmds2 );
	m_ChoiceListHighlight.RunCommands(	cmds2 );
	m_CurrentGroup.RunCommands(			cmds2 );
	m_CurrentTitle.RunCommands(			cmds2 );
	m_CurrentArtist.RunCommands(		cmds2 );
	//This should be fixed and changed to OFF_COMMAND

	for(int i=0; i<NUM_PLAYERS; i++)
	{
		m_SpeedIcon[i].RunCommands(		cmds2 );
		m_MirrorIcon[i].RunCommands(	cmds2 );
		m_ShuffleIcon[i].RunCommands(	cmds2 );
		m_HiddenIcon[i].RunCommands(	cmds2 );
		m_VanishIcon[i].RunCommands(	cmds2 );
	}
}


void ScreenEz2SelectMusic::MenuLeft( const InputEventPlus &input )
{
	m_MenuTimer->Stall();
	m_MusicBannerWheel.BannersLeft();
	for(int i=i_SkipAheadOffset; i>0; i--)
		m_MusicBannerWheel.BannersLeft();
	MusicChanged();
}

static LocalizedString DOES_NOT_HAVE_MUSIC_FILE( "ScreenEz2SelectMusic", "This song does not have a music file and cannot be played." );
void ScreenEz2SelectMusic::MenuStart( const InputEventPlus &input )
{
	if( !m_MusicBannerWheel.GetSelectedSong()->HasMusic() )
	{
		ScreenPrompt::Prompt( SM_None, DOES_NOT_HAVE_MUSIC_FILE );
		return;
	}

	SCREENMAN->PlayStartSound();

	if( (PREVIEWMUSICMODE == 1 || PREVIEWMUSICMODE == 3) && iConfirmSelection == 0 )
	{
		iConfirmSelection = 1;
		m_MusicBannerWheel.StartBouncing();
		m_MusicBannerWheel.PlayMusicSample();
		return;
	}

	m_bMadeChoice = true;
	m_fRemainingWaitTime = RageTimer::GetTimeSinceStartFast();

	TweenOffScreen();

	// show "hold START for options"
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
	m_sprOptionsMessage.BeginTweening( 0.25f );	// fade in
	m_sprOptionsMessage.SetZoomY( 1 );
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,1) );
	m_sprOptionsMessage.BeginTweening( 2.0f );	// sleep
	m_sprOptionsMessage.BeginTweening( 0.25f );	// fade out
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
	m_sprOptionsMessage.SetZoomY( 0 );
}

static LocalizedString NO_SONGS_AVAILABLE( "ScreenEz2SelectMusic", "There are no songs available for play." );
void ScreenEz2SelectMusic::Update( float fDeltaTime )
{
	m_DifficultyRating.Update(fDeltaTime);

	if(i_SkipAheadOffset > 0 && RageTimer::GetTimeSinceStartFast() - LastInputTime < 0.5)
	{
		m_MusicBannerWheel.SetScanMode(true);
	}
	else
	{
		m_MusicBannerWheel.SetScanMode(false);
	}
	
	if(m_MusicBannerWheel.CheckSongsExist() == 0 && ! i_ErrorDetected)
	{
		ScreenPrompt::Prompt( SM_NoSongs, NO_SONGS_AVAILABLE );
		i_ErrorDetected=1;
		this->PostScreenMessage( SM_NoSongs, 5.5f ); // timeout incase the user decides to do nothing :D
	}

	if(m_bMadeChoice && RageTimer::GetTimeSinceStartFast() > m_fRemainingWaitTime + 2 && !m_bTransitioning)
	{
		m_bTransitioning = true;
		StartTransitioningScreen( SM_GoToNextScreen );
	}

	Screen::Update( fDeltaTime );
}

void ScreenEz2SelectMusic::EasierDifficulty( PlayerNumber pn )
{
	if(USE_MODE_SWITCHER == 1)
	{
		m_ModeSwitcher.ChangeMode(pn,-1);
		MusicChanged();
		m_MusicBannerWheel.StopBouncing();
		SOUND->StopMusic();
		return;
	}

	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;
	if( m_arrayNotes[pn].empty() )
		return;
	if( m_iSelection[pn] == 0 )
		return;

	if((PREVIEWMUSICMODE == 1 || PREVIEWMUSICMODE == 3) && iConfirmSelection == 1)
	{
		iConfirmSelection = 0;
		m_MusicBannerWheel.StopBouncing();
		if(PREVIEWMUSICMODE == 1)
			SOUND->StopMusic();
	}

	m_iSelection[pn]--;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficulty[pn].Set( m_arrayNotes[pn][ m_iSelection[pn] ]->GetDifficulty() );

//	m_soundChangeNotes.Play();

	AfterNotesChange( pn );
}



void ScreenEz2SelectMusic::HarderDifficulty( PlayerNumber pn )
{
	if(USE_MODE_SWITCHER == 1)
	{
		m_ModeSwitcher.ChangeMode(pn,+1);
		MusicChanged();
		m_MusicBannerWheel.StopBouncing();
		SOUND->StopMusic();
		return;
	}

	if( !GAMESTATE->IsHumanPlayer(pn
		) )
		return;
	if( m_arrayNotes[pn].empty() )
		return;
	if( m_iSelection[pn] == int(m_arrayNotes[pn].size()-1) )
		return;

	if( m_iSelection[pn] > int(m_arrayNotes[pn].size() - 1) )
	{
		m_iSelection[pn] = int(m_arrayNotes[pn].size()-1 );
		return;
	}

	if((PREVIEWMUSICMODE == 1 || PREVIEWMUSICMODE == 3) && iConfirmSelection == 1)
	{
		iConfirmSelection = 0;

		m_MusicBannerWheel.StopBouncing();
		if(PREVIEWMUSICMODE == 1)
			SOUND->StopMusic();
	}

	m_iSelection[pn]++;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficulty[pn].Set( m_arrayNotes[pn][ m_iSelection[pn] ]->GetDifficulty() );

//	m_soundChangeNotes.Play();

	AfterNotesChange( pn );

}



void ScreenEz2SelectMusic::MusicChanged()
{
	Song* pSong = m_MusicBannerWheel.GetSelectedSong();
	GAMESTATE->m_pCurSong.Set( pSong );

	m_CurrentGroup.SetText( SONGMAN->ShortenGroupName( pSong->m_sGroupName ) , "");
	m_CurrentGroup.SetDiffuse( SONGMAN->GetSongGroupColor(pSong->m_sGroupName) );

	m_CurrentTitle.SetText( pSong->m_sMainTitle, "");
	m_CurrentSubTitle.SetText( pSong->m_sSubTitle, "");
	m_CurrentArtist.SetText( pSong->m_sArtist, "");

	if( pSong->IsMarathon() )
	{
		m_sprBalloon.StopTweening();
		m_sprBalloon.Load( THEME->GetPathG("ScreenSelectMusic","balloon marathon") );
		SET_XY_AND_ON_COMMAND( m_sprBalloon );
	}
	else if( pSong->IsLong() )
	{
		m_sprBalloon.StopTweening();
		m_sprBalloon.Load( THEME->GetPathG("ScreenSelectMusic","balloon long") );
		SET_XY_AND_ON_COMMAND( m_sprBalloon );
	}
	else
	{
		m_sprBalloon.StopTweening();
		OFF_COMMAND( m_sprBalloon );
	}
	
	if(	!m_bScanning )
		m_soundMusicChange.Play();

	if((PREVIEWMUSICMODE == 1 || PREVIEWMUSICMODE == 3) && iConfirmSelection == 1)
	{
		iConfirmSelection = 0;
		if(PREVIEWMUSICMODE == 1)
			SOUND->StopMusic();
	}

	int pn;
	for( pn = 0; pn < NUM_PLAYERS; ++pn)
		m_arrayNotes[pn].clear();


	for( pn = 0; pn < NUM_PLAYERS; ++pn)
	{
		SongUtil::GetSteps( pSong, m_arrayNotes[pn], GAMESTATE->GetCurrentStyle()->m_StepsType );
		StepsUtil::SortNotesArrayByDifficulty( m_arrayNotes[pn] );
	}

	for( pn=0; pn<NUM_PLAYERS; pn++ )
	{
		if( !GAMESTATE->IsHumanPlayer( PlayerNumber(pn) ) )
			continue;
		for( unsigned i=0; i<m_arrayNotes[pn].size(); i++ )
		{
			if( m_arrayNotes[pn][i]->GetDifficulty() == GAMESTATE->m_PreferredDifficulty[pn] )
			{
				m_iSelection[pn] = i;
				break;
			}
		}

		m_iSelection[pn] = clamp( m_iSelection[pn], 0, int(m_arrayNotes[pn].size()) ) ;
	}

	FOREACH_PlayerNumber( pn )
	{
		AfterNotesChange( pn );
	}
}

void ScreenEz2SelectMusic::AfterNotesChange( PlayerNumber pn )
{

	m_iSelection[pn] = clamp( m_iSelection[pn], 0, int(m_arrayNotes[pn].size()-1) );	// bounds clamping

	Steps* pSteps = m_arrayNotes[pn].empty()? NULL: m_arrayNotes[pn][m_iSelection[pn]];

	GAMESTATE->m_pCurSteps[pn].Set( pSteps );


	if( pSteps != NULL && pn == GAMESTATE->m_MasterPlayerNumber )
	{
		m_PumpDifficultyRating.SetText(ssprintf("Lv.%d",pSteps->GetMeter()));
		// XXX
//		m_PumpDifficultyRating.SetDiffuse(  SONGMAN->GetDifficultyColor(pSteps->GetDifficulty()) );
		m_DifficultyRating.SetDifficulty(pSteps->GetMeter());
	}

//	GAMESTATE->m_pCurSteps[pn] = pSteps;

//	Steps* m_pSteps = GAMESTATE->m_pCurSteps[pn];

//	m_FootMeter[pn].SetFromSteps( pSteps );
}

/*
 * (c) 2002-2003 "Frieza"
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
