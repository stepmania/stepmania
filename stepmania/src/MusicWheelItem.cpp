#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: MusicWheelItem

 Desc: See header.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Chris Gomez
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "MusicWheelItem.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "RageSoundManager.h"
#include "ScreenManager.h"	// for sending SM_PlayMusicSample
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include <math.h>
#include "ThemeManager.h"
#include "Notes.h"
#include "Song.h"


// WheelItem stuff
#define ICON_X			THEME->GetMetricF("MusicWheelItem","IconX")
#define SONG_NAME_X		THEME->GetMetricF("MusicWheelItem","SongNameX")
#define SECTION_NAME_X	THEME->GetMetricF("MusicWheelItem","SectionNameX")
#define SECTION_ZOOM	THEME->GetMetricF("MusicWheelItem","SectionZoom")
#define ROULETTE_X		THEME->GetMetricF("MusicWheelItem","RouletteX")
#define ROULETTE_ZOOM	THEME->GetMetricF("MusicWheelItem","RouletteZoom")
#define COURSE_X		THEME->GetMetricF("MusicWheelItem","CourseX")
#define COURSE_ZOOM		THEME->GetMetricF("MusicWheelItem","CourseZoom")
#define GRADE_X( p )	THEME->GetMetricF("MusicWheelItem",ssprintf("GradeP%dX",p+1))




WheelItemData::WheelItemData( WheelItemType wit, Song* pSong, CString sSectionName, Course* pCourse, RageColor color )
{
	m_Type = wit;
	m_pSong = pSong;
	m_sSectionName = sSectionName;
	m_pCourse = pCourse;
	m_color = color;
	m_Flags = WheelNotifyIcon::Flags();
}


MusicWheelItem::MusicWheelItem()
{
	data = NULL;

	m_fPercentGray = 0;
	m_WheelNotifyIcon.SetXY( ICON_X, 0 );
	
	m_TextBanner.SetHorizAlign( align_left );
	m_TextBanner.SetXY( SONG_NAME_X, 0 );

	m_sprSongBar.Load( THEME->GetPathTo("Graphics","MusicWheelItem song") );
	m_sprSongBar.SetXY( 0, 0 );

	m_sprSectionBar.Load( THEME->GetPathTo("Graphics","MusicWheelItem section") );
	m_sprSectionBar.SetXY( 0, 0 );

	m_textSectionName.LoadFromFont( THEME->GetPathTo("Fonts","MusicWheelItem section") );
	m_textSectionName.EnableShadow( false );
	m_textSectionName.SetVertAlign( align_middle );
	m_textSectionName.SetXY( SECTION_NAME_X, 0 );
	m_textSectionName.SetZoom( SECTION_ZOOM );


	m_textRoulette.LoadFromFont( THEME->GetPathTo("Fonts","MusicWheelItem roulette") );
	m_textRoulette.EnableShadow( false );
	m_textRoulette.TurnRainbowOn();
	m_textRoulette.SetZoom( ROULETTE_ZOOM );
	m_textRoulette.SetXY( ROULETTE_X, 0 );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_GradeDisplay[p].Load( THEME->GetPathTo("Graphics","MusicWheelItem grades") );
		m_GradeDisplay[p].SetZoom( 1.0f );
		m_GradeDisplay[p].SetXY( GRADE_X(p), 0 );
	}

	m_textCourse.LoadFromFont( THEME->GetPathTo("Fonts","MusicWheelItem course") );
	m_textCourse.EnableShadow( false );
	m_textCourse.SetZoom( COURSE_ZOOM );
	m_textCourse.SetHorizAlign( align_left );
	m_textCourse.SetXY( COURSE_X, 0 );
}


void MusicWheelItem::LoadFromWheelItemData( WheelItemData* pWID )
{
	ASSERT( pWID != NULL );
	
	
	
	data = pWID;
	/*
	// copy all data items
	this->m_Type	= pWID->m_Type;
	this->m_sSectionName	= pWID->m_sSectionName;
	this->m_pCourse			= pWID->m_pCourse;
	this->m_pSong			= pWID->m_pSong;
	this->m_color			= pWID->m_color;
	this->m_Type		= pWID->m_Type; */


	// init type specific stuff
	switch( pWID->m_Type )
	{
	case TYPE_SECTION:
	case TYPE_COURSE:
		{
			CString sDisplayName;
			BitmapText *bt;
			if(pWID->m_Type == TYPE_SECTION)
			{
				sDisplayName = SONGMAN->ShortenGroupName(data->m_sSectionName);
				bt = &m_textSectionName;
			}
			else
			{
				sDisplayName = data->m_pCourse->m_sName;
				bt = &m_textCourse;
			}
			bt->SetZoom( 1 );
			bt->SetText( sDisplayName );
			bt->SetDiffuse( data->m_color );
			bt->TurnRainbowOff();

			float fSourcePixelWidth = (float)bt->GetWidestLineWidthInSourcePixels();
			float fMaxTextWidth = 200;
			if( fSourcePixelWidth > fMaxTextWidth  )
				bt->SetZoomX( fMaxTextWidth / fSourcePixelWidth );
		}
		break;
	case TYPE_SONG:
		{
			m_TextBanner.LoadFromSong( data->m_pSong );
			m_TextBanner.SetDiffuse( data->m_color );
			m_WheelNotifyIcon.SetFlags( data->m_Flags );
			RefreshGrades();
		}
		break;
	case TYPE_ROULETTE:
		m_textRoulette.SetText( "ROULETTE" );
		break;

	case TYPE_RANDOM:
		m_textRoulette.SetText( "RANDOM" );
		break;

	default:
		ASSERT( 0 );	// invalid type
	}
}

void MusicWheelItem::RefreshGrades()
{
	// Refresh Grades
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !data->m_pSong  ||	// this isn't a song display
			!GAMESTATE->IsPlayerEnabled(p)  ||
			!SONGMAN->IsUsingMemoryCard((PlayerNumber)p) )
		{
			m_GradeDisplay[p].SetDiffuse( RageColor(1,1,1,0) );
			continue;
		}

		Difficulty dc;
		if( GAMESTATE->m_pCurNotes[p] )
			dc = GAMESTATE->m_pCurNotes[p]->GetDifficulty();
		else
			dc = GAMESTATE->m_PreferredDifficulty[p];
		const Grade grade = data->m_pSong->GetGradeForDifficulty( GAMESTATE->GetCurrentStyleDef(), (PlayerNumber)p, dc );
		m_GradeDisplay[p].SetGrade( (PlayerNumber)p, grade );
	}

}


void MusicWheelItem::Update( float fDeltaTime )
{
	Actor::Update( fDeltaTime );

	switch( data->m_Type )
	{
	case TYPE_SECTION:
		m_sprSectionBar.Update( fDeltaTime );
		m_textSectionName.Update( fDeltaTime );
		break;
	case TYPE_ROULETTE:
	case TYPE_RANDOM:
		m_sprSectionBar.Update( fDeltaTime );
		m_textRoulette.Update( fDeltaTime );
		break;
	case TYPE_SONG:
		{
			m_sprSongBar.Update( fDeltaTime );
			m_WheelNotifyIcon.Update( fDeltaTime );
			m_TextBanner.Update( fDeltaTime );
			for( int p=0; p<NUM_PLAYERS; p++ )
				m_GradeDisplay[p].Update( fDeltaTime );
		}
		break;
	case TYPE_COURSE:
		m_sprSongBar.Update( fDeltaTime );
		m_textCourse.Update( fDeltaTime );
		break;
	default:
		ASSERT(0);
	}
}

void MusicWheelItem::DrawPrimitives()
{
	Sprite *bar = NULL;
	switch( data->m_Type )
	{
	case TYPE_SECTION: 
	case TYPE_ROULETTE:
	case TYPE_RANDOM: bar = &m_sprSectionBar; break;
	case TYPE_SONG:		
	case TYPE_COURSE: bar = &m_sprSongBar; break;
	default: ASSERT(0);
	}
	
	bar->Draw();

	switch( data->m_Type )
	{
	case TYPE_SECTION:
		m_textSectionName.Draw();
		break;
	case TYPE_ROULETTE:
	case TYPE_RANDOM:
		m_textRoulette.Draw();
		break;
	case TYPE_SONG:		
		m_TextBanner.Draw();
		m_WheelNotifyIcon.Draw();
		int p;
		for( p=0; p<NUM_PLAYERS; p++ )
			m_GradeDisplay[p].Draw();
		break;
	case TYPE_COURSE:
		m_textCourse.Draw();
		break;
	default:
		ASSERT(0);
	}

	if( m_fPercentGray > 0 )
	{
		bar->SetGlow( RageColor(0,0,0,m_fPercentGray) );
		bar->SetDiffuse( RageColor(0,0,0,0) );
		bar->Draw();
		bar->SetDiffuse( RageColor(0,0,0,1) );
		bar->SetGlow( RageColor(0,0,0,0) );
	}
}
