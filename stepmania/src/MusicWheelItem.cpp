#include "global.h"
#include "MusicWheelItem.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"	// for sending SM_PlayMusicSample
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "song.h"
#include "Course.h"
#include "ProfileManager.h"
#include "ActorUtil.h"
#include "ThemeMetric.h"

CString GRADE_X_NAME( size_t p ) { return ssprintf("GradeP%dX",int(p+1)); }
CString GRADE_Y_NAME( size_t p ) { return ssprintf("GradeP%dY",int(p+1)); }

ThemeMetric<float>				ICON_X				("MusicWheelItem","IconX");
ThemeMetric<float>				ICON_Y				("MusicWheelItem","IconY");
ThemeMetric<apActorCommands>	ICON_ON_COMMAND		("MusicWheelItem","IconOnCommand");
ThemeMetric<float>				SONG_NAME_X			("MusicWheelItem","SongNameX");
ThemeMetric<float>				SONG_NAME_Y			("MusicWheelItem","SongNameY");
ThemeMetric<apActorCommands>	SONG_NAME_ON_COMMAND("MusicWheelItem","SongNameOnCommand");
ThemeMetric<float>				SECTION_X			("MusicWheelItem","SectionX");
ThemeMetric<float>				SECTION_Y			("MusicWheelItem","SectionY");
ThemeMetric<apActorCommands>	SECTION_ON_COMMAND	("MusicWheelItem","SectionOnCommand");
ThemeMetric<float>				ROULETTE_X			("MusicWheelItem","RouletteX");
ThemeMetric<float>				ROULETTE_Y			("MusicWheelItem","RouletteY");
ThemeMetric<apActorCommands>	ROULETTE_ON_COMMAND	("MusicWheelItem","RouletteOnCommand");
ThemeMetric1D<float>			GRADE_X				("MusicWheelItem",GRADE_X_NAME,NUM_PLAYERS);
ThemeMetric1D<float>			GRADE_Y				("MusicWheelItem",GRADE_Y_NAME,NUM_PLAYERS);


WheelItemData::WheelItemData( WheelItemType wit, Song* pSong, CString sSectionName, Course* pCourse, RageColor color )
	:WheelItemBaseData(wit, sSectionName, color)
{
	m_pSong = pSong;
	m_pCourse = pCourse;
}


MusicWheelItem::MusicWheelItem( CString sType ):
	WheelItemBase( sType )
{
	data = NULL;

	SetName( sType );

	m_sprSongBar.Load( THEME->GetPathG(sType,"song") );
	m_sprSongBar.SetXY( 0, 0 );
	this->AddChild( &m_sprSongBar );

	m_sprSectionBar.Load( THEME->GetPathG(sType,"section") );
	m_sprSectionBar.SetXY( 0, 0 );
	this->AddChild( &m_sprSectionBar );

	m_sprExpandedBar.Load( THEME->GetPathG(sType,"expanded") );
	m_sprExpandedBar.SetXY( 0, 0 );
	this->AddChild( &m_sprExpandedBar );

	m_sprModeBar.Load( THEME->GetPathG(sType,"mode") );
	m_sprModeBar.SetXY( 0, 0 );
	this->AddChild( &m_sprModeBar );

	m_sprSortBar.Load( THEME->GetPathG(sType,"sort") );
	m_sprSortBar.SetXY( 0, 0 );
	this->AddChild( &m_sprSortBar );

	m_fPercentGray = 0;
	m_WheelNotifyIcon.SetXY( ICON_X, ICON_Y );
	m_WheelNotifyIcon.RunCommands( ICON_ON_COMMAND );
	this->AddChild( &m_WheelNotifyIcon );
	
	m_TextBanner.Load( "TextBanner" );
	m_TextBanner.SetXY( SONG_NAME_X, SONG_NAME_Y );
	m_TextBanner.RunCommands( SONG_NAME_ON_COMMAND );
	this->AddChild( &m_TextBanner );

	m_textSection.LoadFromFont( THEME->GetPathF(sType,"section") );
	m_textSection.SetShadowLength( 0 );
	m_textSection.SetXY( SECTION_X, SECTION_Y );
	m_textSection.RunCommands( SECTION_ON_COMMAND );
	this->AddChild( &m_textSection );

	m_textRoulette.LoadFromFont( THEME->GetPathF(sType,"roulette") );
	m_textRoulette.SetShadowLength( 0 );
	m_textRoulette.TurnRainbowOn();
	m_textRoulette.SetXY( ROULETTE_X, ROULETTE_Y );
	m_textRoulette.RunCommands( ROULETTE_ON_COMMAND );
	this->AddChild( &m_textRoulette );

	m_textCourse.SetName( "CourseName" );
	m_textCourse.LoadFromFont( THEME->GetPathF(sType,"course") );
	SET_XY_AND_ON_COMMAND( &m_textCourse );
	this->AddChild( &m_textCourse );

	m_textSort.SetName( "Sort" );
	m_textSort.LoadFromFont( THEME->GetPathF(sType,"sort") );
	SET_XY_AND_ON_COMMAND( &m_textSort );
	this->AddChild( &m_textSort );

	FOREACH_PlayerNumber( p )
	{
		m_GradeDisplay[p].Load( THEME->GetPathG(sType,"grades") );
		m_GradeDisplay[p].SetZoom( 1.0f );
		m_GradeDisplay[p].SetXY( GRADE_X.GetValue(p), 0 );
		this->AddChild( &m_GradeDisplay[p] );
	}
}


void MusicWheelItem::LoadFromWheelItemData( WheelItemData* pWID, bool bExpanded )
{
	ASSERT( pWID != NULL );
	data = pWID;


	// hide all
	m_WheelNotifyIcon.SetHidden( true );
	m_TextBanner.SetHidden( true );
	m_sprSongBar.SetHidden( true );
	m_sprSectionBar.SetHidden( true );
	m_sprExpandedBar.SetHidden( true );
	m_sprModeBar.SetHidden( true );
	m_sprSortBar.SetHidden( true );
	m_textSection.SetHidden( true );
	m_textRoulette.SetHidden( true );
	FOREACH_PlayerNumber( p )
		m_GradeDisplay[p].SetHidden( true );
	m_textCourse.SetHidden( true );
	m_textSort.SetHidden( true );


	// init and unhide type specific stuff
	switch( pWID->m_Type )
	{
	case TYPE_SECTION:
	case TYPE_COURSE:
	case TYPE_SORT:
		{
			CString sDisplayName, sTranslitName;
			BitmapText *bt = NULL;
			switch( pWID->m_Type )
			{
				case TYPE_SECTION:
					sDisplayName = SONGMAN->ShortenGroupName(data->m_sText);
					bt = &m_textSection;
					break;
				case TYPE_COURSE:
					sDisplayName = data->m_pCourse->GetFullDisplayTitle();
					sTranslitName = data->m_pCourse->GetFullTranslitTitle();
					bt = &m_textCourse;
					break;
				case TYPE_SORT:
					sDisplayName = data->m_sLabel;
					bt = &m_textSort;
					break;
				default:
					ASSERT(0);
			}

			bt->SetText( sDisplayName, sTranslitName );
			bt->SetDiffuse( data->m_color );
			bt->TurnRainbowOff();
			bt->SetHidden( false );
		}
		break;
	case TYPE_SONG:
		{
			m_TextBanner.LoadFromSong( data->m_pSong );
			m_TextBanner.SetDiffuse( data->m_color );
			m_TextBanner.SetHidden( false );

			m_WheelNotifyIcon.SetFlags( data->m_Flags );
			m_WheelNotifyIcon.SetHidden( false );
			RefreshGrades();
		}
		break;
	case TYPE_ROULETTE:
		m_textRoulette.SetText( THEME->GetMetric("MusicWheel","Roulette") );
		m_textRoulette.SetHidden( false );
		break;

	case TYPE_RANDOM:
		m_textRoulette.SetText( THEME->GetMetric("MusicWheel","Random") );
		m_textRoulette.SetHidden( false );
		break;

	case TYPE_PORTAL:
		m_textRoulette.SetText( THEME->GetMetric("MusicWheel","Portal") );
		m_textRoulette.SetHidden( false );
		break;

	default:
		ASSERT( 0 );	// invalid type
	}

	Actor *pBars[] = { &m_sprBar, &m_sprExpandedBar, &m_sprSectionBar, &m_sprModeBar, &m_sprSortBar, &m_sprSongBar, NULL };
	for( unsigned i = 0; pBars[i] != NULL; ++i )
		pBars[i]->SetVisible( false );

	switch( data->m_Type )
	{
	case TYPE_SECTION: 
	case TYPE_ROULETTE:
	case TYPE_RANDOM:
	case TYPE_PORTAL:
		if( bExpanded )
			m_sprExpandedBar.SetHidden( false );
		else
			m_sprSectionBar.SetHidden( false );
		break;
	case TYPE_SORT:
		if( pWID->m_Action.m_pm != PLAY_MODE_INVALID )
			m_sprModeBar.SetHidden( false );
		else
			m_sprSortBar.SetHidden( false );
		break;
	case TYPE_SONG:		
	case TYPE_COURSE:
		m_sprSongBar.SetHidden( false );
		break;
	default: ASSERT(0);
	}

	for( unsigned i = 0; pBars[i] != NULL; ++i )
	{
		if( !pBars[i]->GetVisible() )
			continue;
		SetGrayBar( pBars[i] );
		break;
	}
}

void MusicWheelItem::RefreshGrades()
{
	// Refresh Grades
	FOREACH_HumanPlayer( p )
	{
		if( !data->m_pSong  ||	// this isn't a song display
			!GAMESTATE->IsHumanPlayer(p) )
		{
			m_GradeDisplay[p].SetDiffuse( RageColor(1,1,1,0) );
			continue;
		}

		Difficulty dc;
		if( GAMESTATE->m_pCurSteps[p] )
			dc = GAMESTATE->m_pCurSteps[p]->GetDifficulty();
		else
			dc = GAMESTATE->m_PreferredDifficulty[p];
		Grade grade;
		if( PROFILEMAN->IsPersistentProfile(p) )
			grade = PROFILEMAN->GetHighScoreForDifficulty( data->m_pSong, GAMESTATE->GetCurrentStyle(), (ProfileSlot)p, dc ).grade;
		else
			grade = PROFILEMAN->GetHighScoreForDifficulty( data->m_pSong, GAMESTATE->GetCurrentStyle(), PROFILE_SLOT_MACHINE, dc ).grade;

		m_GradeDisplay[p].SetGrade( p, grade );
	}
}

void MusicWheelItem::DrawPrimitives()
{
	WheelItemBase::DrawPrimitives();
}

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez, Glenn Maynard
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
