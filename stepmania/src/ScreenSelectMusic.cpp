#include "stdafx.h"
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
#include "RageMusic.h"
#include "ScreenTitleMenu.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "ScreenGameplay.h"
#include "ScreenPrompt.h"
#include "ScreenPlayerOptions.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "InputQueue.h"
#include "ScreenStage.h"
#include "AnnouncerManager.h"


const float SONG_INFO_FRAME_X	= 160;
const float SONG_INFO_FRAME_Y	= SCREEN_TOP+118;

const float DIFFICULTY_X		= SONG_INFO_FRAME_X;
const float DIFFICULTY_Y		= CENTER_Y-26;

const float ICON_X[NUM_PLAYERS]	= { DIFFICULTY_X - 106, DIFFICULTY_X + 106 };
const float ICON_Y				= DIFFICULTY_Y;

const float RADAR_X				= SONG_INFO_FRAME_X;
const float RADAR_Y				= CENTER_Y+58;

const float METER_FRAME_X		= SONG_INFO_FRAME_X;
const float METER_FRAME_Y		= SCREEN_BOTTOM-64;

const float METER_X[NUM_PLAYERS]	= { METER_FRAME_X-66, METER_FRAME_X+66 };
const float METER_Y[NUM_PLAYERS]	= { METER_FRAME_Y-12, METER_FRAME_Y+11 };

const float WHEEL_X		= CENTER_X+160;
const float WHEEL_Y		= CENTER_Y+8;

const float TWEEN_TIME		= 0.5f;


const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User+1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User+2);
const ScreenMessage SM_ConfirmChange		=	ScreenMessage(SM_User+3);



ScreenSelectMusic::ScreenSelectMusic()
{
	LOG->WriteLine( "ScreenSelectMusic::ScreenSelectMusic()" );

	// for debugging
	if( GAMEMAN->m_CurStyle == STYLE_NONE )
		GAMEMAN->m_CurStyle = STYLE_DANCE_SINGLE;

	int p;

	m_Menu.Load(
		THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_BACKGROUND), 
		THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_TOP_EDGE),
		ssprintf("%c or %c change music    %c%c easier difficulty     %c%c harder difficulty      %c%c%c%c change sort", 
		char(1), char(2), char(3), char(3), char(4), char(4), char(3), char(4), char(3), char(4) )
				);
	this->AddActor( &m_Menu );

	m_SongInfoFrame.SetXY( SONG_INFO_FRAME_X, SONG_INFO_FRAME_Y );
	this->AddActor( &m_SongInfoFrame );

	m_sprDifficultyFrame.Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_DIFFICULTY_FRAME) );
	m_sprDifficultyFrame.SetXY( DIFFICULTY_X, DIFFICULTY_Y );
	this->AddActor( &m_sprDifficultyFrame );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_DifficultyIcon[p].SetXY( ICON_X[p], ICON_Y );
		this->AddActor( &m_DifficultyIcon[p] );
	}

	m_GrooveRadar.SetXY( RADAR_X, RADAR_Y );
	this->AddActor( &m_GrooveRadar );

	m_sprMeterFrame.Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_METER_FRAME) );
	m_sprMeterFrame.SetXY( METER_FRAME_X, METER_FRAME_Y );
	this->AddActor( &m_sprMeterFrame );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_FootMeter[p].Load( THEME->GetPathTo(FONT_METER) );
		m_FootMeter[p].SetXY( METER_X[p], METER_Y[p] );
		m_FootMeter[p].SetShadowLength( 2 );
		this->AddActor( &m_FootMeter[p] );
	}

	m_MusicWheel.SetXY( WHEEL_X, WHEEL_Y );
	this->AddActor( &m_MusicWheel );

	m_textHoldForOptions.Load( THEME->GetPathTo(FONT_STAGE) );
	m_textHoldForOptions.SetXY( CENTER_X, CENTER_Y );
	m_textHoldForOptions.SetText( "press START again for options" );
	m_textHoldForOptions.SetZoom( 1 );
	m_textHoldForOptions.SetZoomY( 0 );
	m_textHoldForOptions.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	m_textHoldForOptions.SetZ( -2 );
	this->AddActor( &m_textHoldForOptions );


	m_soundSelect.Load( THEME->GetPathTo(SOUND_MENU_START) );
	m_soundChangeNotes.Load( THEME->GetPathTo(SOUND_SELECT_MUSIC_CHANGE_NOTES) );

	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_MUSIC_INTRO) );

	m_bMadeChoice = false;
	m_bGoToOptions = false;

	AfterMusicChange();
	TweenOnScreen();
	m_Menu.TweenOnScreenFromMenu( SM_None );
}


ScreenSelectMusic::~ScreenSelectMusic()
{
	LOG->WriteLine( "ScreenSelectMusic::~ScreenSelectMusic()" );

}

void ScreenSelectMusic::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenSelectMusic::TweenOnScreen()
{
	float fOriginalZoomY;

	m_SongInfoFrame.SetXY( SONG_INFO_FRAME_X - 400, SONG_INFO_FRAME_Y );
	m_SongInfoFrame.BeginTweening( TWEEN_TIME, Actor::TWEEN_BIAS_END );
	m_SongInfoFrame.SetTweenXY( SONG_INFO_FRAME_X, SONG_INFO_FRAME_Y );

	fOriginalZoomY = m_sprDifficultyFrame.GetZoomY();
	m_sprDifficultyFrame.BeginTweening( TWEEN_TIME );
	m_sprDifficultyFrame.SetTweenZoomY( fOriginalZoomY );

	fOriginalZoomY = m_sprMeterFrame.GetZoomY();
	m_sprMeterFrame.BeginTweening( TWEEN_TIME );
	m_sprMeterFrame.SetTweenZoomY( fOriginalZoomY );

	m_GrooveRadar.TweenOnScreen();


	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		fOriginalZoomY = m_DifficultyIcon[p].GetZoomY();
		m_DifficultyIcon[p].BeginTweening( TWEEN_TIME );
		m_DifficultyIcon[p].SetTweenZoomY( fOriginalZoomY );

		fOriginalZoomY = m_FootMeter[p].GetZoomY();
		m_FootMeter[p].BeginTweening( TWEEN_TIME );
		m_FootMeter[p].SetTweenZoomY( fOriginalZoomY );
	}

	m_MusicWheel.TweenOnScreen();
}

void ScreenSelectMusic::TweenOffScreen()
{
	m_SongInfoFrame.BeginTweening( TWEEN_TIME, Actor::TWEEN_BOUNCE_END );
	m_SongInfoFrame.SetTweenXY( SONG_INFO_FRAME_X - 400, SONG_INFO_FRAME_Y );

	m_sprDifficultyFrame.BeginTweening( TWEEN_TIME );
	m_sprDifficultyFrame.SetTweenZoomY( 0 );

	m_sprMeterFrame.BeginTweening( TWEEN_TIME );
	m_sprMeterFrame.SetTweenZoomY( 0 );

	m_GrooveRadar.TweenOffScreen();

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_DifficultyIcon[p].BeginTweening( TWEEN_TIME );
		m_DifficultyIcon[p].SetTweenZoomY( 0 );

		m_FootMeter[p].BeginTweening( TWEEN_TIME );
		m_FootMeter[p].SetTweenZoomY( 0 );
	}

	m_MusicWheel.TweenOffScreen();
}

const MenuButton EASIER_DIFFICULTY_PATTERN[] = { MENU_BUTTON_UP, MENU_BUTTON_UP };
const int EASIER_DIFFICULTY_PATTERN_SIZE = sizeof(EASIER_DIFFICULTY_PATTERN) / sizeof(MenuButton);

const MenuButton HARDER_DIFFICULTY_PATTERN[] = { MENU_BUTTON_DOWN, MENU_BUTTON_DOWN };
const int HARDER_DIFFICULTY_PATTERN_SIZE = sizeof(HARDER_DIFFICULTY_PATTERN) / sizeof(MenuButton);

const MenuButton NEXT_SORT_PATTERN[] = { MENU_BUTTON_UP, MENU_BUTTON_DOWN, MENU_BUTTON_UP, MENU_BUTTON_DOWN };
const int NEXT_SORT_PATTERN_SIZE = sizeof(NEXT_SORT_PATTERN) / sizeof(MenuButton);

void ScreenSelectMusic::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->WriteLine( "ScreenSelectMusic::Input()" );

	if( MenuI.player == PLAYER_NONE )
		return;

	if( m_Menu.IsClosing() )
		return;		// ignore

	if( m_bMadeChoice && !m_bGoToOptions && MenuI.button == MENU_BUTTON_START )
	{
		m_bGoToOptions = true;
		m_textHoldForOptions.SetText( "Entering Options..." );
		SOUND->PlayOnceStreamed( THEME->GetPathTo(SOUND_MENU_START) );
		return;
	}
	
	if( m_bMadeChoice )
		return;

	if( INPUTQUEUE->MatchesPattern(MenuI.player, EASIER_DIFFICULTY_PATTERN, EASIER_DIFFICULTY_PATTERN_SIZE) )
	{
		EasierDifficulty( MenuI.player );
		return;
	}
	if( INPUTQUEUE->MatchesPattern(MenuI.player, HARDER_DIFFICULTY_PATTERN, HARDER_DIFFICULTY_PATTERN_SIZE) )
	{
		HarderDifficulty( MenuI.player );
		return;
	}
	if( INPUTQUEUE->MatchesPattern(MenuI.player, NEXT_SORT_PATTERN, NEXT_SORT_PATTERN_SIZE) )
	{
		m_MusicWheel.NextSort();
		return;
	}
	

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}


void ScreenSelectMusic::EasierDifficulty( const PlayerNumber p )
{
	LOG->WriteLine( "ScreenSelectMusic::EasierDifficulty( %d )", p );

	if( !GAMEMAN->IsPlayerEnabled(p) )
		return;
	if( m_arrayNotes.GetSize() == 0 )
		return;
	if( m_iSelection[p] == 0 )
		return;

	m_iSelection[p]--;
	m_soundChangeNotes.PlayRandom();

	AfterNotesChange( p );
}

void ScreenSelectMusic::HarderDifficulty( const PlayerNumber p )
{
	LOG->WriteLine( "ScreenSelectMusic::HarderDifficulty( %d )", p );

	if( !GAMEMAN->IsPlayerEnabled(p) )
		return;
	if( m_arrayNotes.GetSize() == 0 )
		return;
	if( m_iSelection[p] == m_arrayNotes.GetSize()-1 )
		return;

	m_iSelection[p]++;
	m_soundChangeNotes.PlayRandom();

	AfterNotesChange( p );
}


void ScreenSelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_MenuTimer:
		MenuStart(PLAYER_1);
		break;
	case SM_GoToPrevState:
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:

		// find out if the Next button is being held down on any of the pads
		bool bIsHoldingNext;
		bIsHoldingNext = false;
		int player;
		for( player=0; player<NUM_PLAYERS; player++ )
		{
			MenuInput mi( (PlayerNumber)player, MENU_BUTTON_START );
			if( INPUTMAPPER->IsButtonDown( mi ) )
				bIsHoldingNext = true;
		}

		if( bIsHoldingNext || m_bGoToOptions )
		{
			SCREENMAN->SetNewScreen( new ScreenPlayerOptions );
		}
		else
		{
			MUSIC->Stop();

			SCREENMAN->SetNewScreen( new ScreenStage(false) );
		}
		break;
	case SM_PlaySongSample:
		PlayMusicSample();
		break;
	}
}

void ScreenSelectMusic::MenuLeft( const PlayerNumber p, const InputEventType type )
{
	m_MusicWheel.PrevMusic();
	
	AfterMusicChange();
}


void ScreenSelectMusic::MenuRight( const PlayerNumber p, const InputEventType type )
{
	m_MusicWheel.NextMusic();

	AfterMusicChange();
}

void ScreenSelectMusic::MenuStart( const PlayerNumber p )
{
	// this needs to check whether valid Notes are selected!
	m_MusicWheel.Select();

	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SONG:
		{
			if( !m_MusicWheel.GetSelectedSong()->HasMusic() )
			{
				SCREENMAN->AddScreenToTop( new ScreenPrompt( "ERROR:\n \nThis song does not have a music file\n and cannot be played.", PROMPT_OK) );
				return;
			}

			bool bIsNew = m_MusicWheel.GetSelectedSong()->IsNew();
			bool bIsHard = false;
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMEMAN->IsPlayerEnabled( (PlayerNumber)p ) )
					continue;	// skip
				if( SONGMAN->GetCurrentNotes((PlayerNumber)p)  &&  SONGMAN->GetCurrentNotes((PlayerNumber)p)->m_iMeter >= 9 )
					bIsHard = true;
			}

			if( bIsNew )
				SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_MUSIC_COMMENT_NEW) );
			else if( bIsHard )
				SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_MUSIC_COMMENT_HARD) );
			else
				SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_MUSIC_COMMENT_GENERAL) );


			TweenOffScreen();

			m_bMadeChoice = true;

			m_soundSelect.PlayRandom();

			// show "hold START for options"
			m_textHoldForOptions.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
			m_textHoldForOptions.BeginTweeningQueued( 0.25f );	// fade in
			m_textHoldForOptions.SetTweenZoomY( 1 );
			m_textHoldForOptions.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );
			m_textHoldForOptions.BeginTweeningQueued( 2.0f );	// sleep
			m_textHoldForOptions.BeginTweeningQueued( 0.25f );	// fade out
			m_textHoldForOptions.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );
			m_textHoldForOptions.SetTweenZoomY( 0 );

			m_Menu.TweenOffScreenToBlack( SM_None, false );

			this->SendScreenMessage( SM_GoToNextState, 2.5f );
		}
		break;
	case TYPE_SECTION:
		
		break;
	case TYPE_ROULETTE:

		break;
	}
}


void ScreenSelectMusic::MenuBack( const PlayerNumber p )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevState, true );
}

void ScreenSelectMusic::AfterNotesChange( const PlayerNumber p )
{
	if( !GAMEMAN->IsPlayerEnabled(p) )
		return;
	
	m_iSelection[p] = clamp( m_iSelection[p], 0, m_arrayNotes.GetSize()-1 );	// bounds clamping

	Notes* pNotes = m_arrayNotes.GetSize()>0 ? m_arrayNotes[m_iSelection[p]] : NULL;

	if( pNotes )
		PREFSMAN->m_PreferredDifficultyClass[p] = pNotes->m_DifficultyClass;

	SONGMAN->SetCurrentNotes( p, pNotes );

	m_DifficultyIcon[p].SetFromNotes( pNotes );
	m_FootMeter[p].SetFromNotes( pNotes );
	m_GrooveRadar.SetFromNotes( p, pNotes );
	m_MusicWheel.NotesChanged( p );
}

void ScreenSelectMusic::AfterMusicChange()
{
	Song* pSong = m_MusicWheel.GetSelectedSong();
	SONGMAN->SetCurrentSong( pSong );

	m_arrayNotes.RemoveAll();

	switch( m_MusicWheel.GetSelectedType() )
	{
	case TYPE_SECTION:
		{	
			CString sGroup = m_MusicWheel.GetSelectedSection();
			m_SongInfoFrame.SetFromGroup( sGroup );	// if this isn't a group, it'll default to the fallback banner
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				m_iSelection[p] = -1;
			}
		}
		break;
	case TYPE_SONG:
		{
			pSong->GetNotesThatMatch( GAMEMAN->GetCurrentStyleDef()->m_NotesType, m_arrayNotes );
			SortNotesArrayByDifficultyClass( m_arrayNotes );
			m_SongInfoFrame.SetFromSong( pSong );
			for( int p=0; p<NUM_PLAYERS; p++ )
			{
				if( !GAMEMAN->IsPlayerEnabled( PlayerNumber(p) ) )
					continue;
				m_iSelection[p] = clamp( m_iSelection[p], 0, m_arrayNotes.GetSize() ) ;
			}
		}
		break;
	case TYPE_ROULETTE:
		m_SongInfoFrame.SetRoulette();
		break;
	}

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		AfterNotesChange( (PlayerNumber)p );
	}
}


void ScreenSelectMusic::PlayMusicSample()
{
	//LOG->WriteLine( "ScreenSelectSong::PlaySONGample()" );

	MUSIC->Stop();

	Song* pSong = m_MusicWheel.GetSelectedSong();
	if( pSong == NULL )
		return;

	CString sSongToPlay = pSong->GetMusicPath();
 
	if( pSong->HasMusic() )
	{
		float fStartSeconds, fEndSeconds;
		pSong->GetMusicSampleRange( fStartSeconds, fEndSeconds );
		MUSIC->Load( sSongToPlay );
		MUSIC->Play( true, fStartSeconds, fEndSeconds );
	}
}


