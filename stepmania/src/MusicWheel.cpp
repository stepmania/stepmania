#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: MusicWheel

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "MusicWheel.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "GameManager.h"
#include "ThemeManager.h"
#include "RageMusic.h"
#include "ScreenManager.h"	// for sending SM_PlayMusicSample
#include "RageLog.h"
#include "GameConstantsAndTypes.h"


const float FADE_TIME	=	1.0f;

const float SWITCH_MUSIC_TIME	=	0.15f;
const float SAMPLE_MUSIC_DELAY	=	0.20f;
const float ROULETTE_SWITCH_MUSIC_TIME	=	SWITCH_MUSIC_TIME/2;
const int ROULETTE_SWITCHES_IN_SLOWING_DOWN = 5;

const float LOCKED_WHEEL_INITIAL_VELOCITY = 7;

const float SORT_ICON_ON_SCREEN_X	=	-140;
const float SORT_ICON_ON_SCREEN_Y	=	-180;

const float SORT_ICON_OFF_SCREEN_X	=	SORT_ICON_ON_SCREEN_X;
const float SORT_ICON_OFF_SCREEN_Y	=	SORT_ICON_ON_SCREEN_Y - 64;


const float SCORE_X				=	58;
const float SCORE_Y[NUM_PLAYERS] = { -40, 40 };

const float SCROLLBAR_X			=	150;

D3DXCOLOR COLOR_SECTION_TEXT = D3DXCOLOR(1,1,0.3f,1);


D3DXCOLOR SECTION_COLORS[] = { 
	D3DXCOLOR( 0.9f, 0.0f, 0.2f, 1 ),	// red
	D3DXCOLOR( 0.6f, 0.0f, 0.4f, 1 ),	// pink
	D3DXCOLOR( 0.2f, 0.1f, 0.3f, 1 ),	// purple
	D3DXCOLOR( 0.0f, 0.4f, 0.8f, 1 ),	// sky blue
	D3DXCOLOR( 0.0f, 0.6f, 0.6f, 1 ),	// sea green
	D3DXCOLOR( 0.0f, 0.6f, 0.2f, 1 ),	// green
	D3DXCOLOR( 0.8f, 0.6f, 0.0f, 1 ),	// orange
};
const int NUM_SECTION_COLORS = sizeof(SECTION_COLORS)/sizeof(D3DXCOLOR);



inline D3DXCOLOR GetNextSectionColor()
{
	static int i=0;
	i = i % NUM_SECTION_COLORS;
	return SECTION_COLORS[i++];
}

WheelItemData::WheelItemData()
{
	m_pSong = NULL;
	m_MusicStatusDisplayType = TYPE_NONE;
}

void WheelItemData::Load( WheelItemType wit, Song* pSong, const CString &sSectionName, Course* pCourse, const D3DXCOLOR color )
{
	m_WheelItemType = wit;
	m_pSong = pSong;
	m_sSectionName = sSectionName;
	m_pCourse = pCourse;
	m_color = color;
}

WheelItemDisplay::WheelItemDisplay()
{
	m_fPercentGray = 0;

	m_MusicStatusDisplay.SetXY( -136, 0 );
	
	m_TextBanner.SetHorizAlign( align_left );
	m_TextBanner.SetXY( -30, 0 );

	m_sprSongBar.Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_SONG_BAR) );
	m_sprSongBar.SetXY( 0, 0 );

	m_sprSectionBar.Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_SECTION_BAR) );
	m_sprSectionBar.SetXY( 0, 0 );

	m_textSectionName.Load( THEME->GetPathTo(FONT_HEADER1) );
	m_textSectionName.TurnShadowOff();
	m_textSectionName.SetHorizAlign( align_left );
	m_textSectionName.SetVertAlign( align_middle );
	m_textSectionName.SetXY( m_sprSectionBar.GetX() - m_sprSectionBar.GetUnzoomedWidth()/2 + 30, 0 );
	m_textSectionName.SetZoom( 1.0f );



	m_textRoulette.Load( THEME->GetPathTo(FONT_TEXT_BANNER) );
	m_textRoulette.TurnShadowOff();
	m_textRoulette.SetText( "ROULETTE" );
	m_textRoulette.TurnRainbowOn();
	m_textRoulette.SetZoom( 1.3f );
	m_textRoulette.SetHorizAlign( align_left );
	m_textRoulette.SetXY( -120, 0 );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_GradeDisplay[p].Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_SMALL_GRADES) );
		m_GradeDisplay[p].SetZoom( 1.0f );
		m_GradeDisplay[p].SetXY( 86.0f + p*26.0f, 0 );
	}

	m_textCourse.Load( THEME->GetPathTo(FONT_TEXT_BANNER) );
	m_textCourse.TurnShadowOff();
	m_textCourse.SetText( "ROULETTE" );
	m_textCourse.TurnRainbowOn();
	m_textCourse.SetZoom( 1.3f );
	m_textCourse.SetHorizAlign( align_left );
	m_textCourse.SetXY( -120, 0 );

}


void WheelItemDisplay::LoadFromWheelItemData( WheelItemData* pWID )
{
	ASSERT( pWID != NULL );
	
	
	// copy all data items
	this->m_WheelItemType	= pWID->m_WheelItemType;
	this->m_sSectionName	= pWID->m_sSectionName;
	this->m_pCourse			= pWID->m_pCourse;
	this->m_pSong			= pWID->m_pSong;
	this->m_color			= pWID->m_color;


	// init type specific stuff
	switch( pWID->m_WheelItemType )
	{

	case TYPE_SECTION:
		{
			CString sDisplayName = SONGMAN->ShortenGroupName(m_sSectionName);
			m_textSectionName.SetZoom( 1 );
			m_textSectionName.SetText( sDisplayName );
			m_textSectionName.SetDiffuseColor( m_color );
			m_textSectionName.TurnRainbowOff();

			float fSourcePixelWidth = (float)m_textSectionName.GetWidestLineWidthInSourcePixels();
			float fMaxTextWidth = 200;
			if( fSourcePixelWidth > fMaxTextWidth  )
				m_textSectionName.SetZoomX( fMaxTextWidth / fSourcePixelWidth );
		}
		break;
	case TYPE_SONG:
		{
			m_TextBanner.LoadFromSong( m_pSong );
			D3DXCOLOR color = m_color;
			color.r += 0.15f;
			color.g += 0.15f;
			color.b += 0.15f;
			m_TextBanner.SetDiffuseColor( color );
			m_MusicStatusDisplay.SetType( m_MusicStatusDisplayType );
			RefreshGrades();
		}
		break;
	case TYPE_ROULETTE:
		{
		}
		break;
	case TYPE_COURSE:
		{
			m_textCourse.SetZoom( 1 );
			m_textCourse.SetText( m_pCourse->m_sName );
			m_textCourse.SetDiffuseColor( m_color );
			m_textCourse.TurnRainbowOff();

			float fSourcePixelWidth = (float)m_textCourse.GetWidestLineWidthInSourcePixels();
			float fMaxTextWidth = 200;
			if( fSourcePixelWidth > fMaxTextWidth  )
				m_textCourse.SetZoomX( fMaxTextWidth / fSourcePixelWidth );
		}
		break;
	default:
		ASSERT( false );	// invalid type
	}
}

void WheelItemDisplay::RefreshGrades()
{
	// Refresh Grades
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled( (PlayerNumber)p ) )
		{
			m_GradeDisplay[p].SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
			continue;
		}

		if( m_pSong )	// this is a song display
		{
			const DifficultyClass dc = PREFSMAN->m_PreferredDifficultyClass[p];
			const Grade grade = m_pSong->GetGradeForDifficultyClass( GAMEMAN->GetCurrentStyleDef()->m_NotesType, dc );
			m_GradeDisplay[p].SetGrade( grade );
			m_GradeDisplay[p].SetDiffuseColor( PlayerToColor((PlayerNumber)p) );
		}
		else	// this is a section display
		{
			m_GradeDisplay[p].SetGrade( GRADE_NO_DATA );
		}
	}

}


void WheelItemDisplay::Update( float fDeltaTime )
{
	Actor::Update( fDeltaTime );

	switch( m_WheelItemType )
	{
	case TYPE_SECTION:
		m_sprSectionBar.Update( fDeltaTime );
		m_textSectionName.Update( fDeltaTime );
		break;
	case TYPE_ROULETTE:
		m_sprSectionBar.Update( fDeltaTime );
		m_textRoulette.Update( fDeltaTime );
		break;
	case TYPE_SONG:
		{
			m_sprSongBar.Update( fDeltaTime );
			m_MusicStatusDisplay.Update( fDeltaTime );
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

void WheelItemDisplay::DrawPrimitives()
{
	switch( m_WheelItemType )
	{
	case TYPE_SECTION:
		m_sprSectionBar.Draw();
		m_textSectionName.Draw();
		break;
	case TYPE_ROULETTE:
		m_sprSectionBar.Draw();
		m_textRoulette.Draw();
		break;
	case TYPE_SONG:
		{
			m_sprSongBar.Draw();
			m_MusicStatusDisplay.Draw();
			m_TextBanner.Draw();
			for( int p=0; p<NUM_PLAYERS; p++ )
				m_GradeDisplay[p].Draw();
			
			if( m_fPercentGray > 0 )
			{
				m_sprSongBar.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
				m_sprSongBar.SetAddColor( D3DXCOLOR(0,0,0,m_fPercentGray) );
				m_sprSongBar.Draw();
				m_sprSongBar.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
				m_sprSongBar.SetAddColor( D3DXCOLOR(0,0,0,0) );
			}
		}
		break;
	case TYPE_COURSE:
		m_sprSongBar.Draw();
		m_textCourse.Draw();
		break;
	default:
		ASSERT(0);
	}
}



MusicWheel::MusicWheel() 
{ 
	LOG->WriteLine( "MusicWheel::MusicWheel()" );

	
	// for debugging
	if( GAMEMAN->m_CurStyle == STYLE_NONE )
		GAMEMAN->m_CurStyle = STYLE_DANCE_SINGLE;


	m_frameOverlay.SetXY( 0, 0 );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !GAMEMAN->IsPlayerEnabled((PlayerNumber)p) )
			continue;	// skip

		m_sprHighScoreFrame[p].Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_SCORE_FRAME) );
		m_sprHighScoreFrame[p].SetXY( SCORE_X, SCORE_Y[p] );
		m_frameOverlay.AddActor( &m_sprHighScoreFrame[p] );

		m_HighScore[p].SetXY( SCORE_X, SCORE_Y[p]*0.97f );
		m_HighScore[p].SetZoom( 0.6f );
		m_HighScore[p].SetDiffuseColor( PlayerToColor(p) );
		m_frameOverlay.AddActor( &m_HighScore[p] );
	}
	
	m_sprHighScoreFrame[1].SetZoomY( -1 );	// flip vertically


	m_sprSelectionOverlay.Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_SONG_HIGHLIGHT) );
	m_sprSelectionOverlay.SetXY( 0, 0 );
	m_sprSelectionOverlay.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	m_sprSelectionOverlay.SetEffectGlowing( 1.0f, D3DXCOLOR(1,1,1,0.4f), D3DXCOLOR(1,1,1,1) );
	m_frameOverlay.AddActor( &m_sprSelectionOverlay );

	m_MusicSortDisplay.SetXY( SORT_ICON_ON_SCREEN_X, SORT_ICON_ON_SCREEN_Y );
	m_MusicSortDisplay.SetEffectGlowing( 1.0f );
	m_frameOverlay.AddActor( &m_MusicSortDisplay );

	this->AddActor( &m_frameOverlay );


	m_ScrollBar.SetX( SCROLLBAR_X ); 
	this->AddActor( &m_ScrollBar );
	

	m_soundChangeMusic.Load( THEME->GetPathTo(SOUND_SELECT_MUSIC_CHANGE_MUSIC), 16 );
	m_soundChangeSort.Load( THEME->GetPathTo(SOUND_SELECT_MUSIC_CHANGE_SORT) );
	m_soundExpand.Load( THEME->GetPathTo(SOUND_SELECT_MUSIC_SECTION_EXPAND) );
	m_soundStart.Load( THEME->GetPathTo(SOUND_MENU_START) );
	m_soundLocked.Load( THEME->GetPathTo(SOUND_SELECT_MUSIC_WHEEL_LOCKED) );


	m_fTimeLeftBeforePlayMusicSample = 0;

	// init m_mapGroupNameToBannerColor

	CArray<Song*, Song*> arraySongs;
	arraySongs.Copy( SONGMAN->m_pSongs );
	SortSongPointerArrayByGroup( arraySongs );
	


	m_SortOrder = PREFSMAN->m_SongSortOrder;
	m_MusicSortDisplay.Set( m_SortOrder );
	m_MusicSortDisplay.SetXY( SORT_ICON_ON_SCREEN_X, SORT_ICON_ON_SCREEN_Y );


	m_sExpandedSectionName = "";

	m_iSelection = 0;

	
	m_WheelState = STATE_SELECTING_MUSIC;
	m_fTimeLeftInState = 0;
	m_fPositionOffsetFromSelection = 0;

	m_iSwitchesLeftInSpinDown = 0;

	// build all of the wheel item datas
	for( int so=0; so<NUM_SORT_ORDERS; so++ )
		BuildWheelItemDatas( m_WheelItemDatas[so], SongSortOrder(so) );

	// select a song if none are selected
	if( SONGMAN->m_pCurSong == NULL && 	// if there is no currently selected song
		SONGMAN->m_pSongs.GetSize() > 0 )		// and there is at least one song
	{
		CArray<Song*, Song*> arraySongs;
		SONGMAN->GetSongsInGroup( SONGMAN->m_sPreferredGroup, arraySongs );
	
		if( arraySongs.GetSize() > 0 )
			SONGMAN->SetCurrentSong( arraySongs[0] );	// select the first song
	}


	if( SONGMAN->m_pCurSong != NULL )
	{
		// find the previously selected song (if any)
		for( int i=0; i<GetCurWheelItemDatas().GetSize(); i++ )
		{
			if( GetCurWheelItemDatas()[i].m_pSong == SONGMAN->GetCurrentSong() )
			{
				m_iSelection = i;		// select it
				m_sExpandedSectionName = GetCurWheelItemDatas()[m_iSelection].m_sSectionName;	// make its group the currently expanded group
				break;
			}
		}
	}

	// rebuild the WheelItems that appear on screen
	RebuildWheelItemDisplays();

}

MusicWheel::~MusicWheel()
{
	PREFSMAN->m_SongSortOrder = m_SortOrder;
}

void MusicWheel::BuildWheelItemDatas( CArray<WheelItemData, WheelItemData&> &arrayWheelItemDatas, SongSortOrder so, bool bRoulette )
{
	int i;

	switch( PREFSMAN->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		{
			///////////////////////////////////
			// Make an array of Song*, then sort them
			///////////////////////////////////
			CArray<Song*, Song*> arraySongs;
			
			// copy only song that have at least one Notes for the current GameMode
			for( i=0; i<SONGMAN->m_pSongs.GetSize(); i++ )
			{
				Song* pSong = SONGMAN->m_pSongs[i];

				CArray<Notes*, Notes*> arraySteps;
				pSong->GetNotesThatMatch( GAMEMAN->GetCurrentStyleDef()->m_NotesType, arraySteps );

				if( arraySteps.GetSize() > 0 )
					arraySongs.Add( pSong );
			}


			// sort the SONGMAN
			switch( so )
			{
			case SORT_GROUP:
				SortSongPointerArrayByGroup( arraySongs );
				break;
			case SORT_TITLE:
				SortSongPointerArrayByTitle( arraySongs );
				break;
			case SORT_BPM:
				SortSongPointerArrayByBPM( arraySongs );
				break;
		//	case SORT_ARTIST:
		//		SortSongPointerArrayByArtist( arraySongs );
		//		break;
			case SORT_MOST_PLAYED:
				SortSongPointerArrayByMostPlayed( arraySongs );
				break;
			default:
				ASSERT( false );	// unhandled SORT_ORDER
			}



			///////////////////////////////////
			// Build an array of WheelItemDatas from the sorted list of Song*
			///////////////////////////////////
			arrayWheelItemDatas.RemoveAll();	// clear out the previous wheel items...

			// ...and load new ones

			bool bUseSections;
			switch( so )
			{
			case SORT_MOST_PLAYED:	bUseSections = false;	break;
			case SORT_BPM:			bUseSections = false;	break;
			case SORT_GROUP:		bUseSections = SONGMAN->m_sPreferredGroup != "ALL MUSIC";	break;
			case SORT_TITLE:		bUseSections = true;	break;
			default:		ASSERT( false );
			}
			if( bRoulette )
				bUseSections = false;


			if( bUseSections )
			{
				// make WheelItemDatas with sections
				arrayWheelItemDatas.SetSize( arraySongs.GetSize()*2 );	// make sure we have enough room for all music and section items

				CString sLastSection = "";
				D3DXCOLOR colorSection;
				int iCurWheelItem = 0;
				for( int i=0; i< arraySongs.GetSize(); i++ )
				{
					Song* pSong = arraySongs[i];
					CString sThisSection = GetSectionNameFromSongAndSort( pSong, so );
					int iSectionColorIndex = 0;
					if( sThisSection != sLastSection )	// new section, make a section item
					{
						WheelItemData &WID = arrayWheelItemDatas[iCurWheelItem++];
						colorSection = (so==SORT_GROUP) ? SONGMAN->GetGroupColor(pSong->m_sGroupName) : SECTION_COLORS[iSectionColorIndex];
						iSectionColorIndex = (iSectionColorIndex+1) % NUM_SECTION_COLORS;
						WID.Load( TYPE_SECTION, NULL, sThisSection, NULL, colorSection );
						sLastSection = sThisSection;
					}

					WheelItemData &WID = arrayWheelItemDatas[iCurWheelItem++];
					WID.Load( TYPE_SONG, pSong, sThisSection, NULL, SONGMAN->GetGroupColor(pSong->m_sGroupName) );
				}
				arrayWheelItemDatas.SetSize( iCurWheelItem );	// make sure we have enough room for all music and section items	
			}
			else
			{
				arrayWheelItemDatas.SetSize( arraySongs.GetSize() );
				{
					for( int i=0; i<arraySongs.GetSize(); i++ )
					{
						Song* pSong = arraySongs[i];
						WheelItemData &WID = arrayWheelItemDatas[i];
						WID.Load( TYPE_SONG, pSong, "", NULL, SONGMAN->GetGroupColor(pSong->m_sGroupName) );
					}
				}
			}
		}
		break;
	case PLAY_MODE_ONI:
		{
			arrayWheelItemDatas.SetSize( 0, 20 );	// clear out the previous wheel items...
			for( int c=0; c<SONGMAN->m_aCourses.GetSize(); c++ )	// foreach course
			{
				Course* pCourse = &SONGMAN->m_aCourses[c];
				arrayWheelItemDatas.Add( WheelItemData() );
				WheelItemData &WID = arrayWheelItemDatas[arrayWheelItemDatas.GetSize()-1];
				WID.Load( TYPE_COURSE, NULL, "", pCourse, D3DXCOLOR(1,1,1,1) );
			}
		}
		break;
	default:
		ASSERT(0);	// invalid PlayMode
	}

	// init crowns
	for( i=0; i<arrayWheelItemDatas.GetSize(); i++ )
	{
		Song* pSong = arrayWheelItemDatas[i].m_pSong;
		if( pSong != NULL )
		{
			arrayWheelItemDatas[i].m_MusicStatusDisplayType = (pSong->GetNumTimesPlayed()==0) ? TYPE_NEW : TYPE_NONE;
		}		
	}

	if( so == SORT_MOST_PLAYED )
	{
		// init crown icons 
		for( int i=0; i<arrayWheelItemDatas.GetSize() && i<3; i++ )
		{
			arrayWheelItemDatas[i].m_MusicStatusDisplayType = MusicStatusDisplayType(TYPE_CROWN1 + i);
		}
	}



	if( arrayWheelItemDatas.GetSize() == 0 )
	{
		arrayWheelItemDatas.SetSize( 1 );
		arrayWheelItemDatas[0].Load( TYPE_SECTION, NULL, "NO SONGS", NULL, D3DXCOLOR(1,0,0,1) );
	}
	else if( PREFSMAN->m_PlayMode == PLAY_MODE_ARCADE  &&  !bRoulette )
	{
		arrayWheelItemDatas.SetSize( arrayWheelItemDatas.GetSize()+1 );
		arrayWheelItemDatas[arrayWheelItemDatas.GetSize()-1].Load( TYPE_ROULETTE, NULL, "", NULL, D3DXCOLOR(1,0,0,1) );
	}
}

void MusicWheel::SwitchSortOrder()
{
	
}

float MusicWheel::GetBannerY( float fPosOffsetsFromMiddle )
{
	return (float)roundf( fPosOffsetsFromMiddle*44 );
}

float MusicWheel::GetBannerBrightness( float fPosOffsetsFromMiddle )
{
	//return 1 - fabsf(fPosOffsetsFromMiddle)*0.11f;
	return 1;
}

float MusicWheel::GetBannerAlpha( float fPosOffsetsFromMiddle )
{
	/*
	if( m_WheelState == STATE_FLYING_OFF_BEFORE_NEXT_SORT 
	 || m_WheelState == STATE_TWEENING_OFF_SCREEN  )
	{
		return m_fTimeLeftInState / FADE_TIME;
	}
	else if( m_WheelState == STATE_FLYING_ON_AFTER_NEXT_SORT
		  || m_WheelState == STATE_TWEENING_ON_SCREEN )
	{
		return 1 - (m_fTimeLeftInState / FADE_TIME);
	}
	else if( m_WheelState == STATE_WAITING_OFF_SCREEN )
	{
		return 0;
	}
	else
	{
		return 1;
	}
	*/
	return 1;
}

float MusicWheel::GetBannerX( float fPosOffsetsFromMiddle )
{	
	float fX = (1-cosf((fPosOffsetsFromMiddle)/3))*95.0f;
	
	return (float)roundf( fX );
}

void MusicWheel::RebuildWheelItemDisplays()
{
	// rewind to first index that will be displayed;
	int iIndex = m_iSelection;
	if( m_iSelection > GetCurWheelItemDatas().GetSize()-1 )
		m_iSelection = 0;

	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW/2; i++ )
	{
		do
		{
			iIndex--;
			if( iIndex < 0 )
				iIndex = GetCurWheelItemDatas().GetSize()-1;
		} 
		while( GetCurWheelItemDatas()[iIndex].m_WheelItemType == TYPE_SONG 
			&& GetCurWheelItemDatas()[iIndex].m_sSectionName != ""
			&& GetCurWheelItemDatas()[iIndex].m_sSectionName != m_sExpandedSectionName );
	}

	// iIndex is now the index of the lowest WheelItem to draw
	for( i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemData&    data    = GetCurWheelItemDatas()[iIndex];
		WheelItemDisplay& display = m_WheelItemDisplays[i];

		display.LoadFromWheelItemData( &data );

		// increment iIndex
		do
		{
			iIndex++;
			if( iIndex > GetCurWheelItemDatas().GetSize()-1 )
				iIndex = 0;
		} 
		while( GetCurWheelItemDatas()[iIndex].m_WheelItemType == TYPE_SONG 
			&& GetCurWheelItemDatas()[iIndex].m_sSectionName != ""
			&& GetCurWheelItemDatas()[iIndex].m_sSectionName != m_sExpandedSectionName );
	}

}

void MusicWheel::NotesChanged( PlayerNumber pn )	// update grade graphics and top score
{
	DifficultyClass dc = PREFSMAN->m_PreferredDifficultyClass[pn];
	Song* pSong = SONGMAN->GetCurrentSong();
	Notes* m_pNotes = SONGMAN->GetCurrentNotes( pn );
	
	if( m_pNotes )
		m_HighScore[pn].SetScore( (float)m_pNotes->m_iTopScore );

	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemDisplay& display = m_WheelItemDisplays[i];
		display.RefreshGrades();
	}
}



void MusicWheel::DrawPrimitives()
{
	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemDisplay& display = m_WheelItemDisplays[i];

		switch( m_WheelState )
		{
		case STATE_SELECTING_MUSIC:
		case STATE_ROULETTE_SPINNING:
		case STATE_ROULETTE_SLOWING_DOWN:
		case STATE_LOCKED:
			{
				float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS_TO_DRAW/2 + m_fPositionOffsetFromSelection;

				float fY = GetBannerY( fThisBannerPositionOffsetFromSelection );
				if( fY < -SCREEN_HEIGHT/2  ||  fY > SCREEN_HEIGHT/2 )
					continue; // skip

				float fX = GetBannerX( fThisBannerPositionOffsetFromSelection );
				display.SetXY( fX, fY );
			}
			break;
		};

		if( m_WheelState == STATE_LOCKED  &&  i != NUM_WHEEL_ITEMS_TO_DRAW/2 )
			display.m_fPercentGray = 0.5f;
		else
			display.m_fPercentGray = 0;

		display.Draw();
	}


	ActorFrame::DrawPrimitives();
}

void MusicWheel::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );


	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemDisplay& display = m_WheelItemDisplays[i];

		display.Update( fDeltaTime );
	}

	float fScrollPercentage = (m_iSelection-m_fPositionOffsetFromSelection) / (float)GetCurWheelItemDatas().GetSize();
	m_ScrollBar.SetPercentage( fScrollPercentage );

	if( m_WheelState == STATE_ROULETTE_SPINNING )
	{
		NextMusic( false );	// spin as fast as possible
	}

	if( m_fTimeLeftBeforePlayMusicSample > 0 )
	{
		m_fTimeLeftBeforePlayMusicSample -= fDeltaTime;
		if( m_fTimeLeftBeforePlayMusicSample < 0 )
			SCREENMAN->SendMessageToTopScreen( SM_PlaySongSample, 0 );
	}

	// update wheel state
	m_fTimeLeftInState -= fDeltaTime;
	if( m_fTimeLeftInState <= 0 )	// time to go to a new state
	{
		switch( m_WheelState )
		{
		case STATE_FLYING_OFF_BEFORE_NEXT_SORT:
			{
				m_WheelState = STATE_FLYING_ON_AFTER_NEXT_SORT;
				m_fTimeLeftInState = FADE_TIME;

				Song* pPrevSelectedSong = GetCurWheelItemDatas()[m_iSelection].m_pSong;
				CString sPrevSelectedSection = GetCurWheelItemDatas()[m_iSelection].m_sSectionName;

				// change the sort order
				m_SortOrder = SongSortOrder(m_SortOrder+1);
				if( m_SortOrder > NUM_SORT_ORDERS-1 )
					m_SortOrder = (SongSortOrder)0;
				m_sExpandedSectionName = GetSectionNameFromSongAndSort( pPrevSelectedSong, m_SortOrder );
				//RebuildWheelItems();

				m_MusicSortDisplay.Set( m_SortOrder );
				m_MusicSortDisplay.BeginTweening( FADE_TIME, TWEEN_BIAS_BEGIN );
				m_MusicSortDisplay.SetTweenXY( SORT_ICON_ON_SCREEN_X, SORT_ICON_ON_SCREEN_Y );


				m_iSelection = 0;

				if( pPrevSelectedSong != NULL )		// the previous selected item was a song
				{
					// find the previously selected song, and select it
					for( i=0; i<GetCurWheelItemDatas().GetSize(); i++ )
					{
						if( GetCurWheelItemDatas()[i].m_pSong == pPrevSelectedSong )
						{
							m_iSelection = i;
							break;
						}
					}
				}
				else	// the previously selected item was a section
				{
					// find the previously selected song, and select it
					for( i=0; i<GetCurWheelItemDatas().GetSize(); i++ )
					{
						if( GetCurWheelItemDatas()[i].m_sSectionName == sPrevSelectedSection )
						{
							m_iSelection = i;
							break;
						}
					}
				}

				RebuildWheelItemDisplays();

				TweenOnScreen();
			}
			break;
		case STATE_FLYING_ON_AFTER_NEXT_SORT:
			SCREENMAN->SendMessageToTopScreen( SM_PlaySongSample, 0 );
			m_WheelState = STATE_SELECTING_MUSIC;	// now, wait for input
			break;
		case STATE_TWEENING_ON_SCREEN:
			SCREENMAN->SendMessageToTopScreen( SM_PlaySongSample, 0 );
			m_WheelState = STATE_SELECTING_MUSIC;
			m_fTimeLeftInState = 0;
			break;
		case STATE_TWEENING_OFF_SCREEN:
			m_WheelState = STATE_WAITING_OFF_SCREEN;
			m_fTimeLeftInState = 0;
			break;
		case STATE_SELECTING_MUSIC:
			m_fTimeLeftInState = 0;
			break;
		case STATE_ROULETTE_SPINNING:
			break;
		case STATE_WAITING_OFF_SCREEN:
			break;
		case STATE_LOCKED:
			break;
		case STATE_ROULETTE_SLOWING_DOWN:
			if( m_iSwitchesLeftInSpinDown == 0 )
			{
				m_WheelState = STATE_LOCKED;
				m_fTimeLeftInState = 0;
				m_soundStart.Play();
				m_fLockedWheelVelocity = 0;
			}
			else
			{
				m_iSwitchesLeftInSpinDown--;
				switch( m_iSwitchesLeftInSpinDown )
				{
				case 4:		m_fTimeLeftInState = 0.2f;	break;
				case 3:		m_fTimeLeftInState = 0.4f;	break;
				case 2:		m_fTimeLeftInState = 0.8f;	break;
				case 1:		m_fTimeLeftInState = 1.3f;	break;
				case 0:		m_fTimeLeftInState = 0.5f;	break;
				default:	ASSERT(0);
				}
				
				LOG->WriteLine( "m_iSwitchesLeftInSpinDown id %d, m_fTimeLeftInState is %f", m_iSwitchesLeftInSpinDown, m_fTimeLeftInState );

				if( m_iSwitchesLeftInSpinDown < 2 )
					randomf(0,1) >= 0.5f ? NextMusic() : PrevMusic();
				else
					NextMusic();
			}
			break;
		default:
			ASSERT(0);	// all state changes should be handled explitily
			break;
		}
	}


	if( m_WheelState == STATE_LOCKED )
	{
		m_fPositionOffsetFromSelection = clamp( m_fPositionOffsetFromSelection, -0.3f, +0.3f );
		
		float fSpringForce = - m_fPositionOffsetFromSelection*LOCKED_WHEEL_INITIAL_VELOCITY;
		m_fLockedWheelVelocity += fSpringForce;

		float fDrag = -m_fLockedWheelVelocity * fDeltaTime*4;
		m_fLockedWheelVelocity += fDrag;

		m_fPositionOffsetFromSelection  += m_fLockedWheelVelocity*fDeltaTime;

		if( fabsf(m_fPositionOffsetFromSelection) < 0.01f  &&  fabsf(m_fLockedWheelVelocity) < 0.01f )
		{
			m_fPositionOffsetFromSelection = 0;
			m_fLockedWheelVelocity = 0;
		}
	}
	else
	{
		// "rotate" wheel toward selected song
		float fSpinSpeed;
		if( m_WheelState == STATE_ROULETTE_SPINNING )
			fSpinSpeed = 1.0f/ROULETTE_SWITCH_MUSIC_TIME;
		else
			fSpinSpeed = 0.6f + fabsf(m_fPositionOffsetFromSelection)/SWITCH_MUSIC_TIME;

		if( m_fPositionOffsetFromSelection > 0 )
		{
			m_fPositionOffsetFromSelection -= fSpinSpeed*fDeltaTime;
			if( m_fPositionOffsetFromSelection < 0 )
			{
				m_fPositionOffsetFromSelection = 0;
				m_fTimeLeftBeforePlayMusicSample = SAMPLE_MUSIC_DELAY;
			}
		}
		else if( m_fPositionOffsetFromSelection < 0 )
		{
			m_fPositionOffsetFromSelection += fSpinSpeed*fDeltaTime;
			if( m_fPositionOffsetFromSelection > 0 )
			{
				m_fPositionOffsetFromSelection = 0;
				m_fTimeLeftBeforePlayMusicSample = SAMPLE_MUSIC_DELAY;
			}
		}
	}
}


void MusicWheel::PrevMusic( bool bSendSongChangedMessage )
{
	if( m_WheelState == STATE_LOCKED )
	{
		m_fLockedWheelVelocity = LOCKED_WHEEL_INITIAL_VELOCITY;
		m_soundLocked.Play();
		return;
	}

	if( fabsf(m_fPositionOffsetFromSelection) > 0.5f )	// wheel is busy spinning
		return;
	
	switch( m_WheelState )
	{
	case STATE_SELECTING_MUSIC:
	case STATE_ROULETTE_SPINNING:
	case STATE_ROULETTE_SLOWING_DOWN:
		break;	// fall through
	default:
		return;	// don't fall through
	}

	MUSIC->Stop();

	// decrement m_iSelection
	do
	{
		m_iSelection--;
		if( m_iSelection < 0 )
			m_iSelection = GetCurWheelItemDatas().GetSize()-1;
	} 
	while( (GetCurWheelItemDatas()[m_iSelection].m_WheelItemType == TYPE_SONG || GetCurWheelItemDatas()[m_iSelection].m_WheelItemType == TYPE_COURSE)
		&& GetCurWheelItemDatas()[m_iSelection].m_sSectionName != ""
		&& GetCurWheelItemDatas()[m_iSelection].m_sSectionName != m_sExpandedSectionName );

	RebuildWheelItemDisplays();

	m_fPositionOffsetFromSelection -= 1;

	m_soundChangeMusic.Play();

	if( bSendSongChangedMessage )
		SCREENMAN->SendMessageToTopScreen( SM_SongChanged, 0 );
}

void MusicWheel::NextMusic( bool bSendSongChangedMessage )
{
	if( m_WheelState == STATE_LOCKED )
	{
		m_fLockedWheelVelocity = -LOCKED_WHEEL_INITIAL_VELOCITY;
		m_soundLocked.Play();
		return;
	}

	if( fabsf(m_fPositionOffsetFromSelection) > 0.5 )	// wheel is very busy spinning
		return;
	
	switch( m_WheelState )
	{
	case STATE_SELECTING_MUSIC:
	case STATE_ROULETTE_SPINNING:
	case STATE_ROULETTE_SLOWING_DOWN:
		break;	// fall through
	default:
		LOG->WriteLine( "NextMusic() ignored" );
		return;	// don't continue
	}

	MUSIC->Stop();

	// increment m_iSelection
	do
	{
		m_iSelection++;
		if( m_iSelection > GetCurWheelItemDatas().GetSize()-1 )
			m_iSelection = 0;
	} 
	while( (GetCurWheelItemDatas()[m_iSelection].m_WheelItemType == TYPE_SONG || GetCurWheelItemDatas()[m_iSelection].m_WheelItemType == TYPE_COURSE)
		&& GetCurWheelItemDatas()[m_iSelection].m_sSectionName != ""
		&& GetCurWheelItemDatas()[m_iSelection].m_sSectionName != m_sExpandedSectionName );

	RebuildWheelItemDisplays();

	m_fPositionOffsetFromSelection += 1;

	m_soundChangeMusic.Play();

	if( bSendSongChangedMessage )
		SCREENMAN->SendMessageToTopScreen( SM_SongChanged, 0 );
}

void MusicWheel::PrevSort()
{
	NextSort();
}

void MusicWheel::NextSort()
{
	switch( m_WheelState )
	{
	case STATE_SELECTING_MUSIC:
	case STATE_FLYING_ON_AFTER_NEXT_SORT:
		break;	// fall through
	default:
		return;	// don't continue
	}

	MUSIC->Stop();

	m_soundChangeSort.Play();
	m_MusicSortDisplay.BeginTweening( FADE_TIME, TWEEN_BIAS_END );
	m_MusicSortDisplay.SetTweenXY( SORT_ICON_OFF_SCREEN_X, SORT_ICON_OFF_SCREEN_Y );

	TweenOffScreen();

	m_WheelState = STATE_FLYING_OFF_BEFORE_NEXT_SORT;
	m_fTimeLeftInState = FADE_TIME;
}

bool MusicWheel::Select()	// return true of a playable item was chosen
{
	LOG->WriteLine( "MusicWheel::Select()" );

	if( m_WheelState == STATE_ROULETTE_SPINNING )
	{
		m_WheelState = STATE_ROULETTE_SLOWING_DOWN;
		m_iSwitchesLeftInSpinDown = ROULETTE_SWITCHES_IN_SLOWING_DOWN;
		m_fTimeLeftInState = 0.1f;
		return false;
	}

	switch( GetCurWheelItemDatas()[m_iSelection].m_WheelItemType )
	{
	case TYPE_SECTION:
		{
		CString sThisItemSectionName = GetCurWheelItemDatas()[m_iSelection].m_sSectionName;
		if( m_sExpandedSectionName == sThisItemSectionName )	// already expanded
			m_sExpandedSectionName = "";		// collapse it
		else				// already collapsed
			m_sExpandedSectionName = sThisItemSectionName;	// expand it

	
		RebuildWheelItemDisplays();


		m_soundExpand.Play();


		m_iSelection = 0;	// reset in case we can't find the last selected song
		// find the section header and select it
		for( int i=0; i<GetCurWheelItemDatas().GetSize(); i++ )
		{
			if( GetCurWheelItemDatas()[i].m_WheelItemType == TYPE_SECTION  
				&&  GetCurWheelItemDatas()[i].m_sSectionName == sThisItemSectionName )
			{
				m_iSelection = i;
				break;
			}
		}

		}
		return false;
	case TYPE_ROULETTE:
		m_soundExpand.Play();
		m_WheelState = STATE_ROULETTE_SPINNING;
		m_SortOrder = SORT_GROUP;
		m_MusicSortDisplay.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
		m_MusicSortDisplay.SetEffectNone();
        BuildWheelItemDatas( m_WheelItemDatas[SORT_GROUP], SORT_GROUP, true );
		return false;

	case TYPE_SONG:
	default:
		
		return true;
	}
}

void MusicWheel::TweenOnScreen() 
{
	m_WheelState = STATE_TWEENING_ON_SCREEN;
	m_fTimeLeftInState = FADE_TIME; 


	m_MusicSortDisplay.SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	m_MusicSortDisplay.BeginTweening( FADE_TIME );
	m_MusicSortDisplay.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,1) );



	float fX, fY;
	
	fX = GetBannerX(0);
	fY = GetBannerY(0);
	m_frameOverlay.SetXY( fX+320, fY );
	m_frameOverlay.BeginTweeningQueued( 0.5f );	// sleep
	m_frameOverlay.BeginTweeningQueued( 0.4f, Actor::TWEEN_BIAS_BEGIN );
	m_frameOverlay.SetTweenX( fX );



	fX = m_ScrollBar.GetX();
	fY = m_ScrollBar.GetY();
	m_ScrollBar.SetXY( fX+30, fY );
	m_ScrollBar.BeginTweeningQueued( 0.7f );	// sleep
	m_ScrollBar.BeginTweeningQueued( 0.2f, Actor::TWEEN_BIAS_BEGIN );
	m_ScrollBar.SetTweenX( fX );	


	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemDisplay& display = m_WheelItemDisplays[i];
		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS_TO_DRAW/2 + m_fPositionOffsetFromSelection;

		float fX = GetBannerX(fThisBannerPositionOffsetFromSelection);
		float fY = GetBannerY(fThisBannerPositionOffsetFromSelection);
		display.SetXY( fX+320, fY );
		display.BeginTweeningQueued( 0.04f*i );	// sleep
		display.BeginTweeningQueued( 0.2f, Actor::TWEEN_BIAS_BEGIN );
		display.SetTweenX( fX );
	}
}
						   
void MusicWheel::TweenOffScreen()
{
	m_WheelState = STATE_TWEENING_OFF_SCREEN;
	m_fTimeLeftInState = FADE_TIME;

	
	m_MusicSortDisplay.SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	m_MusicSortDisplay.SetEffectNone();
	m_MusicSortDisplay.BeginTweening( FADE_TIME );
	m_MusicSortDisplay.SetTweenDiffuseColor( D3DXCOLOR(1,1,1,0) );


	for( int p=0; p<NUM_PLAYERS; p++ )
	{
//		m_HighScore[p].BeginTweening( 0.2f, Actor::TWEEN_BIAS_END );
//		m_HighScore[p].SetTweenX( m_HighScore[p].GetX()+320 );
	}


	float fX, fY;
	fX = GetBannerX(0);
	fY = GetBannerY(0);
	m_frameOverlay.SetXY( fX, fY );
	m_frameOverlay.BeginTweeningQueued( 0 );	// sleep
	m_frameOverlay.BeginTweeningQueued( 0.2f, Actor::TWEEN_BIAS_END );
	m_frameOverlay.SetTweenX( fX+320 );


	m_ScrollBar.BeginTweeningQueued( 0 );
	m_ScrollBar.BeginTweeningQueued( 0.2f, Actor::TWEEN_BIAS_BEGIN );
	m_ScrollBar.SetTweenX( m_ScrollBar.GetX()+30 );	


	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemDisplay& display = m_WheelItemDisplays[i];
		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS_TO_DRAW/2 + m_fPositionOffsetFromSelection;

		float fX = GetBannerX(fThisBannerPositionOffsetFromSelection);
		float fY = GetBannerY(fThisBannerPositionOffsetFromSelection);
		display.SetXY( fX, fY );
		display.BeginTweeningQueued( 0.04f*i );	// sleep
		display.BeginTweeningQueued( 0.2f, Actor::TWEEN_BIAS_END );
		display.SetTweenX( fX+320 );
	}



}
