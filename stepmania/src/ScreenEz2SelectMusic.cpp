#include "global.h"
/*
-----------------------------------------------------------------------------
 File: ScreenEz2SelectMusic.cpp

 Desc: See Header

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.

  Andrew Livy
-----------------------------------------------------------------------------
*/

#include "ScreenEz2SelectMusic.h"
#include "RageDisplay.h"
#include <math.h>
#include "SDL.h"
#include "SDL_opengl.h"
#include "ScreenManager.h"
#include "RageSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "StyleDef.h"
#include "InputMapper.h"
#include "CodeDetector.h"
#include "Notes.h"


#define SCROLLING_LIST_X		THEME->GetMetricF("ScreenEz2SelectMusic","ScrollingListX")
#define SCROLLING_LIST_Y		THEME->GetMetricF("ScreenEz2SelectMusic","ScrollingListY")
#define PUMP_DIFF_X				THEME->GetMetricF("ScreenEz2SelectMusic","PumpDifficultyX")
#define PUMP_DIFF_Y				THEME->GetMetricF("ScreenEz2SelectMusic","PumpDifficultyY")
#define HELP_TEXT				THEME->GetMetric("ScreenSelectMusic","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenSelectMusic","TimerSeconds")
#define METER_X( p )			THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MeterP%dX",p+1))
#define METER_Y( p )			THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MeterP%dY",p+1))
#define GUIDE_X					THEME->GetMetricF("ScreenSelectMode","GuideX")
#define GUIDE_Y					THEME->GetMetricF("ScreenSelectMode","GuideY")
#define SPEEDICON_X( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("SpeedIconP%dX",p+1))
#define SPEEDICON_Y( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("SpeedIconP%dY",p+1))
#define MIRRORICON_X( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MirrorIconP%dX",p+1))
#define MIRRORICON_Y( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("MirrorIconP%dY",p+1))
#define SHUFFLEICON_X( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("ShuffleIconP%dX",p+1))
#define SHUFFLEICON_Y( p )		THEME->GetMetricF("ScreenEz2SelectMusic",ssprintf("ShuffleIconP%dY",p+1))
#define PREVIEWMUSICMODE		THEME->GetMetricI("ScreenEz2SelectMusic","PreviewMusicMode")
#define DIFFICULTYRATING_X		THEME->GetMetricF("ScreenEz2SelectMusic","DifficultyRatingX")
#define DIFFICULTYRATING_Y		THEME->GetMetricF("ScreenEz2SelectMusic","DifficultyRatingY")
#define DIFFICULTYRATING_ORIENTATION		THEME->GetMetricI("ScreenEz2SelectMusic","DifficultyRatingOrientation")
#define INFOFRAME_X		THEME->GetMetricF("ScreenEz2SelectMusic","InfoFrameX")
#define INFOFRAME_Y		THEME->GetMetricF("ScreenEz2SelectMusic","InfoFrameY")

const float TWEEN_TIME		= 0.5f;

const ScreenMessage SM_NoSongs	= ScreenMessage(SM_User+3);

ScreenEz2SelectMusic::ScreenEz2SelectMusic() : Screen("ScreenEz2SelectMusic")
{
	i_ErrorDetected=0;
	CodeDetector::RefreshCacheItems();

	if(PREVIEWMUSICMODE == 1)
	{
		SOUNDMAN->StopMusic();
		iConfirmSelection = 0;
	}


	m_Menu.Load("ScreenSelectMusic");
	this->AddChild( &m_Menu );

	m_soundMusicChange.Load( THEME->GetPathToS("ScreenEz2SelectMusic change"));
	m_soundMusicCycle.Load( THEME->GetPathToS("ScreenEz2SelectMusic cycle"));
	m_soundSelect.Load( THEME->GetPathToS("Common start") );

	m_ChoiceListFrame.Load( THEME->GetPathToG("ScreenEz2SelectMusic list frame"));
	m_ChoiceListFrame.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y);
	this->AddChild( &m_ChoiceListFrame );

	m_Guide.Load( THEME->GetPathToG("ScreenEz2SelectMusic guide"));
	m_Guide.SetXY( GUIDE_X, GUIDE_Y );
	this->AddChild( &m_Guide );


	m_MusicBannerWheel.SetX(SCROLLING_LIST_X);
	m_MusicBannerWheel.SetY(SCROLLING_LIST_Y);

	if(m_MusicBannerWheel.CheckSongsExist() != 0)
	{
		this->AddChild( &m_MusicBannerWheel );


		m_ChoiceListHighlight.Load( THEME->GetPathToG("ScreenEz2SelectMusic list highlight"));
		m_ChoiceListHighlight.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y);
		this->AddChild( &m_ChoiceListHighlight );

		#ifdef DEBUG
		m_debugtext.LoadFromFont( THEME->GetPathToF("small titles") );
		m_debugtext.SetXY( CENTER_X, CENTER_Y );
		this->AddChild(&m_debugtext);
		#endif

		for(int p=0; p<NUM_PLAYERS; p++ )
		{
		//	m_FootMeter[p].SetXY( METER_X(p), METER_Y(p) );
		//	m_FootMeter[p].SetShadowLength( 2 );
		//	this->AddChild( &m_FootMeter[p] );

			m_SpeedIcon[p].Load( THEME->GetPathToG("ScreenEz2SelectMusic speedicon"));
			m_SpeedIcon[p].SetXY( SPEEDICON_X(p), SPEEDICON_Y(p) );
			m_SpeedIcon[p].SetDiffuse( RageColor(0,0,0,0) );
			this->AddChild(&m_SpeedIcon[p] );

			m_MirrorIcon[p].Load( THEME->GetPathToG("ScreenEz2SelectMusic mirroricon"));
			m_MirrorIcon[p].SetXY( MIRRORICON_X(p), MIRRORICON_Y(p) );
			m_MirrorIcon[p].SetDiffuse( RageColor(0,0,0,0) );
			this->AddChild(&m_MirrorIcon[p] );

			m_ShuffleIcon[p].Load( THEME->GetPathToG("ScreenEz2SelectMusic shuffleicon"));
			m_ShuffleIcon[p].SetXY( SHUFFLEICON_X(p), SHUFFLEICON_Y(p) );
			m_ShuffleIcon[p].SetDiffuse( RageColor(0,0,0,0) );
			this->AddChild(&m_ShuffleIcon[p] );

			UpdateOptions((PlayerNumber) p,0);

			m_iSelection[p] = 0;
		}

		m_InfoFrame.Load( THEME->GetPathToG("ScreenEz2SelectMusic infoframe") );
		m_InfoFrame.SetXY( INFOFRAME_X, INFOFRAME_Y );
		this->AddChild( &m_InfoFrame );

		m_PumpDifficultyCircle.Load( THEME->GetPathToG("ScreenEz2SelectMusic difficulty frame"));
		m_PumpDifficultyCircle.SetXY( PUMP_DIFF_X, PUMP_DIFF_Y );
		this->AddChild( &m_PumpDifficultyCircle );

		m_PumpDifficultyRating.LoadFromFont( THEME->GetPathToF("ScreenEz2SelectMusic difficulty") );
		m_PumpDifficultyRating.SetXY( PUMP_DIFF_X, PUMP_DIFF_Y );
		this->AddChild(&m_PumpDifficultyRating);

		m_DifficultyRating.SetOrientation(DIFFICULTYRATING_ORIENTATION);
		m_DifficultyRating.SetX(DIFFICULTYRATING_X);
		m_DifficultyRating.SetY(DIFFICULTYRATING_Y);
		this->AddChild(&m_DifficultyRating);

		m_sprOptionsMessage.Load( THEME->GetPathToG("ScreenEz2SelectMusic options message") );
		m_sprOptionsMessage.StopAnimating();
		m_sprOptionsMessage.SetXY( CENTER_X, CENTER_Y );
		m_sprOptionsMessage.SetZoom( 1 );
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
		this->AddChild( &m_sprOptionsMessage );

		m_soundOptionsChange.Load( THEME->GetPathToS("ScreenEz2SelectMusic options") );

		m_bGoToOptions = false;
		m_bMadeChoice = false;

		MusicChanged();
	}
}

void ScreenEz2SelectMusic::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
//	if( type != IET_FIRST_PRESS )
//		return;	// ignore

	if(i_ErrorDetected) return; // don't let the user do anything if theres an error

	if( type == IET_RELEASE )	return;		// don't care

	if( m_Menu.IsTransitioning() )	return;		// ignore

	if( !GameI.IsValid() )		return;		// don't care

	if( m_bMadeChoice  &&  !m_bGoToOptions  &&  MenuI.IsValid()  &&  MenuI.button == MENU_BUTTON_START )
	{
		SOUNDMAN->PlayOnce( THEME->GetPathToS("Common start") );
		m_bGoToOptions = true;
		m_sprOptionsMessage.SetState( 1 );
	}

	if( m_bMadeChoice )
		return;

	PlayerNumber pn = GAMESTATE->GetCurrentStyleDef()->ControllerToPlayerNumber( GameI.controller );

	if( CodeDetector::EnteredEasierDifficulty(GameI.controller) )
	{
		EasierDifficulty( pn );
		return;
	}
	if( CodeDetector::EnteredHarderDifficulty(GameI.controller) )
	{
		HarderDifficulty( pn );
		return;
	}
	if( CodeDetector::DetectAndAdjustMusicOptions(GameI.controller) )
	{
		UpdateOptions(pn,1);
	}
	if( type != IET_FIRST_PRESS )
	{
		m_soundMusicCycle.Play();
	}
	else
	{
		m_soundMusicChange.Play();
	}


	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenEz2SelectMusic::UpdateOptions(PlayerNumber pn, int nosound)
{
	sOptions = GAMESTATE->m_PlayerOptions[pn].GetString();

	#ifdef DEBUG
		m_debugtext.SetText( "DEBUG: " + sOptions );
	#endif

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
		}
	}
	else
	{
		m_SpeedIcon[pn].SetDiffuse( RageColor(0,0,0,0) );
		m_MirrorIcon[pn].SetDiffuse( RageColor(0,0,0,0) );
		m_ShuffleIcon[pn].SetDiffuse( RageColor(0,0,0,0) );
	}
	if(nosound !=0)
		m_soundOptionsChange.Play();
}

void ScreenEz2SelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
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
	case SM_NoSongs:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
	break;
	}

}


void ScreenEz2SelectMusic::MenuRight( PlayerNumber pn, const InputEventType type )
{
	m_Menu.m_MenuTimer.Stall();
	m_MusicBannerWheel.BannersRight();
	MusicChanged();
}

void ScreenEz2SelectMusic::MenuBack( PlayerNumber pn )
{
	SOUNDMAN->StopMusic();

	m_Menu.Back( SM_GoToPrevScreen );
}


void ScreenEz2SelectMusic::TweenOffScreen()
{
	m_MusicBannerWheel.FadeOff( 0, "foldy", TWEEN_TIME );
	m_PumpDifficultyCircle.FadeOff( 0, "fade", TWEEN_TIME*2 );
	m_Guide.FadeOff( 0, "fade", TWEEN_TIME*2 );
	m_PumpDifficultyRating.FadeOff( 0, "fade", TWEEN_TIME*2 );
	m_Guide.FadeOff( 0, "fade", TWEEN_TIME*2 );
	m_ChoiceListFrame.FadeOff( 0, "fade", TWEEN_TIME*2 );
	m_ChoiceListHighlight.FadeOff( 0, "fade", TWEEN_TIME*2 );
}


void ScreenEz2SelectMusic::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	m_Menu.m_MenuTimer.Stall();
	m_MusicBannerWheel.BannersLeft();
	MusicChanged();
}

void ScreenEz2SelectMusic::MenuStart( PlayerNumber pn )
{
	if( !m_MusicBannerWheel.GetSelectedSong()->HasMusic() )
	{
		SCREENMAN->Prompt( SM_None, "ERROR:\n \nThis song does not have a music file\n and cannot be played." );
		return;
	}

	m_soundSelect.Play();

	if(PREVIEWMUSICMODE == 1 && iConfirmSelection == 0)
	{
		iConfirmSelection = 1;
		m_MusicBannerWheel.StartBouncing();
		m_MusicBannerWheel.PlayMusicSample();
		return;
	}

	m_bMadeChoice = true;

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

	m_Menu.StartTransitioning( SM_GoToNextScreen );
}


void ScreenEz2SelectMusic::Update( float fDeltaTime )
{
	if(m_MusicBannerWheel.CheckSongsExist() == 0 && ! i_ErrorDetected)
	{
		SCREENMAN->Prompt( SM_NoSongs, "ERROR:\n \nThere are no songs available for play!" );
		i_ErrorDetected=1;
		this->PostScreenMessage( SM_NoSongs, 5.5f ); // timeout incase the user decides to do nothing :D
	}

	Screen::Update( fDeltaTime );
}

void ScreenEz2SelectMusic::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenEz2SelectMusic::EasierDifficulty( PlayerNumber pn )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;
	if( m_arrayNotes[pn].empty() )
		return;
	if( m_iSelection[pn] == 0 )
		return;

	if(PREVIEWMUSICMODE == 1 && iConfirmSelection == 1)
	{
		iConfirmSelection = 0;
		m_MusicBannerWheel.StopBouncing();
		SOUNDMAN->StopMusic();
	}

	m_iSelection[pn]--;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficulty[pn] = m_arrayNotes[pn][ m_iSelection[pn] ]->GetDifficulty();

//	m_soundChangeNotes.Play();

	AfterNotesChange( pn );
}



void ScreenEz2SelectMusic::HarderDifficulty( PlayerNumber pn )
{
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

	if(PREVIEWMUSICMODE == 1 && iConfirmSelection == 1)
	{
		iConfirmSelection = 0;

		m_MusicBannerWheel.StopBouncing();
		SOUNDMAN->StopMusic();
	}

	m_iSelection[pn]++;
	// the user explicity switched difficulties.  Update the preferred difficulty
	GAMESTATE->m_PreferredDifficulty[pn] = m_arrayNotes[pn][ m_iSelection[pn] ]->GetDifficulty();

//	m_soundChangeNotes.Play();

	AfterNotesChange( pn );

}



void ScreenEz2SelectMusic::MusicChanged()
{
	Song* pSong = m_MusicBannerWheel.GetSelectedSong();
	GAMESTATE->m_pCurSong = pSong;

	if(PREVIEWMUSICMODE == 1 && iConfirmSelection == 1)
	{
		iConfirmSelection = 0;
		SOUNDMAN->StopMusic();
	}

	int pn;
	for( pn = 0; pn < NUM_PLAYERS; ++pn)
		m_arrayNotes[pn].clear();


	for( pn = 0; pn < NUM_PLAYERS; ++pn)
	{
		pSong->GetNotes( m_arrayNotes[pn], GAMESTATE->GetCurrentStyleDef()->m_NotesType );
		SortNotesArrayByDifficulty( m_arrayNotes[pn] );
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

	for( pn=0; pn<NUM_PLAYERS; pn++ )
	{
		AfterNotesChange( (PlayerNumber)pn );
	}
}

void ScreenEz2SelectMusic::AfterNotesChange( PlayerNumber pn )
{

	m_iSelection[pn] = clamp( m_iSelection[pn], 0, int(m_arrayNotes[pn].size()-1) );	// bounds clamping

	Notes* pNotes = m_arrayNotes[pn].empty()? NULL: m_arrayNotes[pn][m_iSelection[pn]];

	GAMESTATE->m_pCurNotes[pn] = pNotes;


	if( pNotes != NULL && pn == GAMESTATE->m_MasterPlayerNumber )
	{
		m_PumpDifficultyRating.SetText(ssprintf("Lv.%d",pNotes->GetMeter()));
		m_DifficultyRating.SetDifficulty(pNotes->GetMeter());
	}

//	GAMESTATE->m_pCurNotes[pn] = pNotes;

//	Notes* m_pNotes = GAMESTATE->m_pCurNotes[pn];

//	m_FootMeter[pn].SetFromNotes( pNotes );
}

