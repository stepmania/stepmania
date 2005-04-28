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

	SetName( "MusicWheelItem" );

	m_fPercentGray = 0;
	m_WheelNotifyIcon.SetXY( ICON_X, ICON_Y );
	m_WheelNotifyIcon.RunCommands( ICON_ON_COMMAND );
	this->AddChild( &m_WheelNotifyIcon );
	
	m_TextBanner.Load( "TextBanner" );
	m_TextBanner.SetXY( SONG_NAME_X, SONG_NAME_Y );
	m_TextBanner.RunCommands( SONG_NAME_ON_COMMAND );
	this->AddChild( &m_TextBanner );

	m_sprSongBar.Load( THEME->GetPathG("MusicWheelItem","song") );
	m_sprSongBar.SetXY( 0, 0 );
	this->AddChild( &m_sprSongBar );

	m_sprSectionBar.Load( THEME->GetPathG("MusicWheelItem","section") );
	m_sprSectionBar.SetXY( 0, 0 );
	this->AddChild( &m_sprSectionBar );

	m_textSectionName.LoadFromFont( THEME->GetPathF("MusicWheelItem","section") );
	m_textSectionName.SetShadowLength( 0 );
	m_textSectionName.SetXY( SECTION_X, SECTION_Y );
	m_textSectionName.RunCommands( SECTION_ON_COMMAND );
	this->AddChild( &m_textSectionName );

	m_textRoulette.LoadFromFont( THEME->GetPathF("MusicWheelItem","roulette") );
	m_textRoulette.SetShadowLength( 0 );
	m_textRoulette.TurnRainbowOn();
	m_textRoulette.SetXY( ROULETTE_X, ROULETTE_Y );
	m_textRoulette.RunCommands( ROULETTE_ON_COMMAND );
	this->AddChild( &m_textRoulette );

	FOREACH_PlayerNumber( p )
	{
		m_GradeDisplay[p].Load( THEME->GetPathG("MusicWheelItem","grades") );
		m_GradeDisplay[p].SetZoom( 1.0f );
		m_GradeDisplay[p].SetXY( GRADE_X.GetValue(p), 0 );
		this->AddChild( &m_GradeDisplay[p] );
	}

	m_textCourse.SetName( "CourseName" );
	m_textCourse.LoadFromFont( THEME->GetPathF("MusicWheelItem","course") );
	SET_XY_AND_ON_COMMAND( &m_textCourse );
	this->AddChild( &m_textCourse );

	m_textSort.SetName( "Sort" );
	m_textSort.LoadFromFont( THEME->GetPathF("MusicWheelItem","sort") );
	SET_XY_AND_ON_COMMAND( &m_textSort );
	this->AddChild( &m_textSort );
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
	case TYPE_SORT:
		{
			CString sDisplayName, sTranslitName;
			BitmapText *bt = NULL;
			switch( pWID->m_Type )
			{
				case TYPE_SECTION:
					sDisplayName = SONGMAN->ShortenGroupName(data->m_sSectionName);
					bt = &m_textSectionName;
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
		m_textRoulette.SetText( THEME->GetMetric("MusicWheel","Roulette") );
		break;

	case TYPE_RANDOM:
		m_textRoulette.SetText( THEME->GetMetric("MusicWheel","Random") );
		break;

	case TYPE_PORTAL:
		m_textRoulette.SetText( THEME->GetMetric("MusicWheel","Portal") );
		break;

	default:
		ASSERT( 0 );	// invalid type
	}
}

void MusicWheelItem::RefreshGrades()
{
	// Refresh Grades
	FOREACH_PlayerNumber( p )
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
		if( PROFILEMAN->IsUsingProfile(p) )
			grade = PROFILEMAN->GetHighScoreForDifficulty( data->m_pSong, GAMESTATE->GetCurrentStyle(), (ProfileSlot)p, dc ).grade;
		else
			grade = PROFILEMAN->GetHighScoreForDifficulty( data->m_pSong, GAMESTATE->GetCurrentStyle(), PROFILE_SLOT_MACHINE, dc ).grade;

		m_GradeDisplay[p].SetGrade( p, grade );
	}

}


void MusicWheelItem::Update( float fDeltaTime )
{
	Actor::Update( fDeltaTime );
	// update manually

	switch( data->m_Type )
	{
	case TYPE_SECTION:
		m_sprSectionBar.Update( fDeltaTime );
		m_textSectionName.Update( fDeltaTime );
		break;
	case TYPE_ROULETTE:
	case TYPE_RANDOM:
	case TYPE_PORTAL:
		m_sprSectionBar.Update( fDeltaTime );
		m_textRoulette.Update( fDeltaTime );
		break;
	case TYPE_SONG:
		{
			m_sprSongBar.Update( fDeltaTime );
			m_WheelNotifyIcon.Update( fDeltaTime );
			m_TextBanner.Update( fDeltaTime );
			FOREACH_PlayerNumber( p )
				m_GradeDisplay[p].Update( fDeltaTime );
		}
		break;
	case TYPE_COURSE:
		m_sprSongBar.Update( fDeltaTime );
		m_textCourse.Update( fDeltaTime );
		break;
	case TYPE_SORT:
		m_sprSectionBar.Update( fDeltaTime );
		m_textSort.Update( fDeltaTime );
		break;
	default:
		ASSERT(0);
	}
}

void MusicWheelItem::DrawPrimitives()
{
	// draw manually

	Sprite *bar = NULL;
	switch( data->m_Type )
	{
	case TYPE_SECTION: 
	case TYPE_ROULETTE:
	case TYPE_RANDOM:
	case TYPE_PORTAL:
	case TYPE_SORT:
		bar = &m_sprSectionBar; 
		break;
	case TYPE_SONG:		
	case TYPE_COURSE:
		bar = &m_sprSongBar; 
		break;
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
	case TYPE_PORTAL:
		m_textRoulette.Draw();
		break;
	case TYPE_SONG:		
		m_TextBanner.Draw();
		m_WheelNotifyIcon.Draw();
		FOREACH_PlayerNumber( p )
			m_GradeDisplay[p].Draw();
		break;
	case TYPE_COURSE:
		m_textCourse.Draw();
		break;
	case TYPE_SORT:
		m_textSort.Draw();
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
