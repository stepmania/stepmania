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
#include "PrefsManager.h"
#include "RageMusic.h"
#include "ScreenManager.h"	// for sending SM_PlayMusicSample
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"


// WheelItem stuff
#define ICON_X			THEME->GetMetricF("WheelItemDisplay","IconX")
#define SONG_NAME_X		THEME->GetMetricF("WheelItemDisplay","SongNameX")
#define SECTION_NAME_X	THEME->GetMetricF("WheelItemDisplay","SectionNameX")
#define SECTION_ZOOM	THEME->GetMetricF("WheelItemDisplay","SectionZoom")
#define ROULETTE_X		THEME->GetMetricF("WheelItemDisplay","RouletteX")
#define ROULETTE_ZOOM	THEME->GetMetricF("WheelItemDisplay","RouletteZoom")
#define COURSE_X		THEME->GetMetricF("WheelItemDisplay","CourseX")
#define COURSE_ZOOM		THEME->GetMetricF("WheelItemDisplay","CourseZoom")
#define GRADE_X( p )	THEME->GetMetricF("WheelItemDisplay",ssprintf("GradeP%dX",p+1))


// MusicWheel stuff
#define FADE_SECONDS				THEME->GetMetricF("MusicWheel","FadeSeconds")
#define SWITCH_SECONDS				THEME->GetMetricF("MusicWheel","SwitchSeconds")
#define ROULETTE_SWITCH_SECONDS		THEME->GetMetricF("MusicWheel","RouletteSwitchSeconds")
#define ROULETTE_SLOW_DOWN_SWITCHES	THEME->GetMetricI("MusicWheel","RouletteSlowDownSwitches")
#define LOCKED_INITIAL_VELOCITY		THEME->GetMetricF("MusicWheel","LockedInitialVelocity")
#define SCROLL_BAR_X				THEME->GetMetricF("MusicWheel","ScrollBarX")
#define SCROLL_BAR_HEIGHT			THEME->GetMetricI("MusicWheel","ScrollBarHeight")
#define ITEM_SPACING_Y				THEME->GetMetricF("MusicWheel","ItemSpacingY")
#define NUM_SECTION_COLORS			THEME->GetMetricI("MusicWheel","NumSectionColors")
#define SECTION_COLORS( i )			THEME->GetMetricC("MusicWheel",ssprintf("SectionColor%d",i+1))

float g_fItemSpacingY;	// cache

inline D3DXCOLOR GetNextSectionColor() {
	static int i=0;
	i = i % NUM_SECTION_COLORS;
	return SECTION_COLORS(i++);
}


WheelItemData::WheelItemData( WheelItemType wit, Song* pSong, const CString &sSectionName, Course* pCourse, const D3DXCOLOR color )
{
	m_WheelItemType = wit;
	m_pSong = pSong;
	m_sSectionName = sSectionName;
	m_pCourse = pCourse;
	m_color = color;
	m_IconType = MusicStatusDisplay::none;
}

WheelItemDisplay::WheelItemDisplay()
{
	m_fPercentGray = 0;

	m_MusicStatusDisplay.SetXY( ICON_X, 0 );
	
	m_TextBanner.SetHorizAlign( align_left );
	m_TextBanner.SetXY( SONG_NAME_X, 0 );

	m_sprSongBar.Load( THEME->GetPathTo("Graphics","select music song bar") );
	m_sprSongBar.SetXY( 0, 0 );

	m_sprSectionBar.Load( THEME->GetPathTo("Graphics","select music section bar") );
	m_sprSectionBar.SetXY( 0, 0 );

	m_textSectionName.LoadFromFont( THEME->GetPathTo("Fonts","musicwheel section") );
	m_textSectionName.TurnShadowOff();
	m_textSectionName.SetVertAlign( align_middle );
	m_textSectionName.SetXY( SECTION_NAME_X, 0 );
	m_textSectionName.SetZoom( SECTION_ZOOM );


	m_textRoulette.LoadFromFont( THEME->GetPathTo("Fonts","musicwheel roulette") );
	m_textRoulette.TurnShadowOff();
	m_textRoulette.SetText( "ROULETTE" );
	m_textRoulette.TurnRainbowOn();
	m_textRoulette.SetZoom( ROULETTE_ZOOM );
	m_textRoulette.SetXY( ROULETTE_X, 0 );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_GradeDisplay[p].Load( THEME->GetPathTo("Graphics","select music small grades 2x8") );
		m_GradeDisplay[p].SetZoom( 1.0f );
		m_GradeDisplay[p].SetXY( GRADE_X(p), 0 );
	}

	m_textCourse.LoadFromFont( THEME->GetPathTo("Fonts","musicwheel course") );
	m_textCourse.TurnShadowOff();
	m_textCourse.SetZoom( COURSE_ZOOM );
	m_textCourse.SetHorizAlign( align_left );
	m_textCourse.SetXY( COURSE_X, 0 );
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
	this->m_IconType		= pWID->m_IconType;


	// init type specific stuff
	switch( pWID->m_WheelItemType )
	{

	case TYPE_SECTION:
		{
			CString sDisplayName = SONGMAN->ShortenGroupName(m_sSectionName);
			m_textSectionName.SetZoom( 1 );
			m_textSectionName.SetText( sDisplayName );
			m_textSectionName.SetDiffuse( m_color );
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
			m_TextBanner.SetDiffuse( color );
			m_MusicStatusDisplay.SetType( m_IconType );
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
			m_textCourse.SetDiffuse( m_color );
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
		if( !GAMESTATE->IsPlayerEnabled( (PlayerNumber)p ) )
		{
			m_GradeDisplay[p].SetDiffuse( D3DXCOLOR(1,1,1,0) );
			continue;
		}

		if( m_pSong )	// this is a song display
		{
			if( m_pSong == GAMESTATE->m_pCurSong )
			{
				Notes* pNotes = GAMESTATE->m_pCurNotes[p];
				m_GradeDisplay[p].SetGrade( (PlayerNumber)p, pNotes ? pNotes->m_TopGrade : GRADE_NO_DATA );
			}
			else
			{
				const Difficulty dc = GAMESTATE->m_PreferredDifficulty[p];
				const Grade grade = m_pSong->GetGradeForDifficulty( GAMESTATE->GetCurrentStyleDef(), p, dc );
				m_GradeDisplay[p].SetGrade( (PlayerNumber)p, grade );
			}
		}
		else	// this is a section display
		{
			m_GradeDisplay[p].SetGrade( (PlayerNumber)p, GRADE_NO_DATA );
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
		m_sprSongBar.Draw();
		m_TextBanner.Draw();
		m_MusicStatusDisplay.Draw();
		int p;
		for( p=0; p<NUM_PLAYERS; p++ )
			m_GradeDisplay[p].Draw();
		if( m_fPercentGray > 0 )
		{
			m_sprSongBar.SetGlow( D3DXCOLOR(0,0,0,m_fPercentGray) );
			m_sprSongBar.SetDiffuse( D3DXCOLOR(0,0,0,0) );
			m_sprSongBar.Draw();
			m_sprSongBar.SetDiffuse( D3DXCOLOR(0,0,0,1) );
			m_sprSongBar.SetGlow( D3DXCOLOR(0,0,0,0) );
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
	LOG->Trace( "MusicWheel::MusicWheel()" );


	// update theme metric cache
	g_fItemSpacingY = ITEM_SPACING_Y;


	// for debugging
	if( GAMESTATE->m_CurStyle == STYLE_NONE )
		GAMESTATE->m_CurStyle = STYLE_DANCE_SINGLE;

	m_sprSelectionOverlay.Load( THEME->GetPathTo("Graphics","select music song highlight") );
	m_sprSelectionOverlay.SetXY( 0, 0 );
	m_sprSelectionOverlay.SetDiffuse( D3DXCOLOR(1,1,1,1) );
	m_sprSelectionOverlay.SetEffectGlowing( 1.0f, D3DXCOLOR(1,1,1,0.4f), D3DXCOLOR(1,1,1,1) );
	AddChild( &m_sprSelectionOverlay );

	m_ScrollBar.SetX( SCROLL_BAR_X ); 
	m_ScrollBar.SetBarHeight( SCROLL_BAR_HEIGHT ); 
	this->AddChild( &m_ScrollBar );
	
	m_soundChangeMusic.Load(	THEME->GetPathTo("Sounds","select music change music"), 16 );
	m_soundChangeSort.Load(		THEME->GetPathTo("Sounds","select music change sort") );
	m_soundExpand.Load(			THEME->GetPathTo("Sounds","select music section expand") );
	m_soundStart.Load(			THEME->GetPathTo("Sounds","menu start") );
	m_soundLocked.Load(			THEME->GetPathTo("Sounds","select music wheel locked") );


	// init m_mapGroupNameToBannerColor

	CArray<Song*, Song*> arraySongs;
	arraySongs.Copy( SONGMAN->m_pSongs );
	SortSongPointerArrayByGroup( arraySongs );
	
	m_sExpandedSectionName = "";

	m_iSelection = 0;

	m_WheelState = STATE_SELECTING_MUSIC;
	m_fTimeLeftInState = 0;
	m_fPositionOffsetFromSelection = 0;

	m_iSwitchesLeftInSpinDown = 0;

	if( GAMESTATE->IsExtraStage()  ||  GAMESTATE->IsExtraStage2() )
	{
		// make the preferred group the group of the last song played.
		if( GAMESTATE->m_sPreferredGroup == "ALL MUSIC" )
			GAMESTATE->m_sPreferredGroup = GAMESTATE->m_pCurSong->m_sGroupName;

		Song* pSong;
		Notes* pNotes;
		PlayerOptions po;
		SongOptions so;
		SONGMAN->GetExtraStageInfo(
			GAMESTATE->IsExtraStage2(),
			GAMESTATE->m_sPreferredGroup,
			GAMESTATE->GetCurrentStyleDef(),
			pSong,
			pNotes,
			po,
			so );
		GAMESTATE->m_pCurSong = pSong;
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( GAMESTATE->IsPlayerEnabled(p) )
			{
				GAMESTATE->m_pCurNotes[p] = pNotes;
				GAMESTATE->m_PlayerOptions[p] = po;
			}
		}
		GAMESTATE->m_SongOptions = so;
	}

	/* Build all of the wheel item data.  Do tihs after selecting
	 * the extra stage, so it knows to always display it. */
	for( int so=0; so<NUM_SORT_ORDERS; so++ )
		BuildWheelItemDatas( m_WheelItemDatas[so], SongSortOrder(so) );

	// If there is no currently selected song, select one.
	if( GAMESTATE->m_pCurSong == NULL )
	{
		CStringArray asGroupNames;
		SONGMAN->GetGroupNames( asGroupNames );
		if( asGroupNames.GetSize() > 0 )
		{
			/* XXX: Do groups get added if they're empty?
			 * This will select songs we can't use; we want the first song
			 * that's actually visible.  Should we select out of wheel data? 
			 * -glenn */
			CArray<Song*, Song*> arraySongs;
			SONGMAN->GetSongsInGroup( asGroupNames[0], arraySongs );
			if( arraySongs.GetSize() > 0 ) // still nothing selected
				GAMESTATE->m_pCurSong = arraySongs[0];	// select the first song
		}
	}


	// Select the the previously selected song (if any)
	if( GAMESTATE->m_pCurSong )
	{
		for( int i=0; i<GetCurWheelItemDatas().GetSize(); i++ )
		{
			if( GetCurWheelItemDatas()[i].m_pSong == GAMESTATE->m_pCurSong )
			{
				m_iSelection = i;		// select it
				m_sExpandedSectionName = GetCurWheelItemDatas()[m_iSelection].m_sSectionName;	// make its group the currently expanded group
				break;
			}
		}
	}

	// Select the the previously selected course (if any)
	if( GAMESTATE->m_pCurCourse != NULL )
	{
		for( int i=0; i<GetCurWheelItemDatas().GetSize(); i++ )
		{
			if( GetCurWheelItemDatas()[i].m_pCourse == GAMESTATE->m_pCurCourse )
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
}

CArray<WheelItemData, WheelItemData&>& MusicWheel::GetCurWheelItemDatas()
{
	return m_WheelItemDatas[GAMESTATE->m_SongSortOrder];
}

void MusicWheel::BuildWheelItemDatas( CArray<WheelItemData, WheelItemData&> &arrayWheelItemDatas, SongSortOrder so, bool bRoulette )
{
	int i;

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		{
			///////////////////////////////////
			// Make an array of Song*, then sort them
			///////////////////////////////////
			CArray<Song*, Song*> arraySongs;
			
			// copy only songs that have at least one Notes for the current GameMode
			for( i=0; i<SONGMAN->m_pSongs.GetSize(); i++ )
			{
				Song* pSong = SONGMAN->m_pSongs[i];

				/* If we're on an extra stage, and this song is selected, ignore
				 * #SELECTABLE. */
				if( pSong != GAMESTATE->m_pCurSong || 
					(!GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2()) ) {
					/* Hide songs that asked to be hidden via #SELECTABLE. */
					if( !bRoulette && !pSong->NormallyDisplayed() )
						continue;
					if( bRoulette && !pSong->RouletteDisplayed() )
						continue;
				}

				CArray<Notes*, Notes*> arraySteps;
				pSong->GetNotesThatMatch( GAMESTATE->GetCurrentStyleDef()->m_NotesType, arraySteps );

				if( arraySteps.GetSize() > 0 )
					arraySongs.Add( pSong );
			}


			// sort the songs
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
				if( arraySongs.GetSize() > 30 )
					arraySongs.SetSize( 30 );
				break;
			default:
				ASSERT(0);	// unhandled SortOrder
			}



			///////////////////////////////////
			// Build an array of WheelItemDatas from the sorted list of Song*s
			///////////////////////////////////
			arrayWheelItemDatas.SetSize( 0, 300 );	// clear out the previous wheel items and set large capacity jumps

			bool bUseSections = false;
			switch( so )
			{
			case SORT_MOST_PLAYED:	bUseSections = false;	break;
			case SORT_BPM:			bUseSections = false;	break;
			case SORT_GROUP:		bUseSections = GAMESTATE->m_sPreferredGroup == "ALL MUSIC";	break;
			case SORT_TITLE:		bUseSections = true;	break;
			default:		ASSERT( false );
			}
			if( bRoulette )
				bUseSections = false;


			if( bUseSections )
			{
				// make WheelItemDatas with sections
				CString sLastSection = "";
				D3DXCOLOR colorSection;
				for( int i=0; i< arraySongs.GetSize(); i++ )
				{
					Song* pSong = arraySongs[i];
					CString sThisSection = GetSectionNameFromSongAndSort( pSong, so );
					int iSectionColorIndex = 0;

					if( GAMESTATE->m_sPreferredGroup != "ALL MUSIC"  &&  pSong->m_sGroupName != GAMESTATE->m_sPreferredGroup )
							continue;
					if( sThisSection != sLastSection)	// new section, make a section item
					{
						colorSection = (so==SORT_GROUP) ? SONGMAN->GetGroupColor(pSong->m_sGroupName) : SECTION_COLORS(iSectionColorIndex);
						iSectionColorIndex = (iSectionColorIndex+1) % NUM_SECTION_COLORS;
						arrayWheelItemDatas.Add( WheelItemData(TYPE_SECTION, NULL, sThisSection, NULL, colorSection) );
						sLastSection = sThisSection;
					}

					arrayWheelItemDatas.Add( WheelItemData( TYPE_SONG, pSong, sThisSection, NULL, SONGMAN->GetSongColor(pSong)) );
				}
			}
			else
			{
				for( int i=0; i<arraySongs.GetSize(); i++ )
				{
					Song* pSong = arraySongs[i];
					if( GAMESTATE->m_sPreferredGroup != "ALL MUSIC"  &&  pSong->m_sGroupName != GAMESTATE->m_sPreferredGroup )
						continue;	// skip
					arrayWheelItemDatas.Add( WheelItemData(TYPE_SONG, pSong, "", NULL, SONGMAN->GetSongColor(pSong)) );
				}
			}
		}


		if( !bRoulette )
		{
			arrayWheelItemDatas.Add( WheelItemData(TYPE_ROULETTE, NULL, "", NULL, D3DXCOLOR(1,0,0,1)) );
		}

		// HACK:  Add extra stage item if it isn't already present on the music wheel
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
		{
			Song* pSong;
			Notes* pNotes;
			PlayerOptions po;
			SongOptions so;
			SONGMAN->GetExtraStageInfo( GAMESTATE->IsExtraStage2(), GAMESTATE->m_pCurSong->m_sGroupName, GAMESTATE->GetCurrentStyleDef(), pSong, pNotes, po, so );
			
			bool bFoundExtraSong = false;

			for( int i=0; i<arrayWheelItemDatas.GetSize(); i++ )
			{
				if( arrayWheelItemDatas[i].m_pSong == pSong )
				{
					bFoundExtraSong = true;
					break;
				}
			}
			
			if( !bFoundExtraSong )
				arrayWheelItemDatas.Add( WheelItemData(TYPE_SONG, pSong, "", NULL, GAMESTATE->GetStageColor()) );
		}

		break;
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			int i;

			CArray<Course*,Course*> apCourses;
			switch( GAMESTATE->m_PlayMode )
			{
			case PLAY_MODE_ONI:
				for( i=0; i<SONGMAN->m_aOniCourses.GetSize(); i++ )
					apCourses.Add( &SONGMAN->m_aOniCourses[i] );
				SortCoursePointerArrayByDifficulty( apCourses );
				break;
			case PLAY_MODE_ENDLESS:
				for( i=0; i<SONGMAN->m_aEndlessCourses.GetSize(); i++ )
					apCourses.Add( &SONGMAN->m_aEndlessCourses[i] );
				break;
			}

			for( int c=0; c<apCourses.GetSize(); c++ )	// foreach course
			{
				Course* pCourse = apCourses[c];

				// check that this course has at least one song playable in the current style
				CArray<Song*,Song*> apSongs;
				CArray<Notes*,Notes*> apNotes;
				CStringArray asModifiers;
				pCourse->GetSongAndNotesForCurrentStyle( apSongs, apNotes, asModifiers, false );

				if( apNotes.GetSize() > 0 )
					arrayWheelItemDatas.Add( WheelItemData(TYPE_COURSE, NULL, "", pCourse, pCourse->GetColor()) );
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
		if( pSong == NULL )
			continue;

		bool bIsEasy = pSong->IsEasy( GAMESTATE->GetCurrentStyleDef()->m_NotesType ); 
		WheelItemData& WID = arrayWheelItemDatas[i];
		WID.m_IconType = bIsEasy ? MusicStatusDisplay::easy : MusicStatusDisplay::none;
	}

	if( so == SORT_MOST_PLAYED )
	{
		// init crown icons 
		for( int i=0; i< min(3,arrayWheelItemDatas.GetSize()); i++ )
		{
			WheelItemData& WID = arrayWheelItemDatas[i];
			WID.m_IconType = MusicStatusDisplay::IconType(MusicStatusDisplay::crown1 + i);
		}
	}



	if( arrayWheelItemDatas.GetSize() == 0 )
	{
		arrayWheelItemDatas.Add( WheelItemData(TYPE_SECTION, NULL, "- EMPTY -", NULL, D3DXCOLOR(1,0,0,1)) );
	}
}

void MusicWheel::SwitchSortOrder()
{
	
}

float MusicWheel::GetBannerY( float fPosOffsetsFromMiddle )
{
	return roundf( fPosOffsetsFromMiddle*g_fItemSpacingY );
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
		return m_fTimeLeftInState / FADE_SECONDS;
	}
	else if( m_WheelState == STATE_FLYING_ON_AFTER_NEXT_SORT
		  || m_WheelState == STATE_TWEENING_ON_SCREEN )
	{
		return 1 - (m_fTimeLeftInState / FADE_SECONDS);
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
	
	return roundf( fX );
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
	float fPercentItemsShowing = NUM_WHEEL_ITEMS_TO_DRAW / (float)GetCurWheelItemDatas().GetSize();
	m_ScrollBar.SetPercentage( fScrollPercentage-fPercentItemsShowing/2, fScrollPercentage+fPercentItemsShowing/2 );

	if( m_WheelState == STATE_ROULETTE_SPINNING )
	{
		NextMusic( false );	// spin as fast as possible
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
				m_fTimeLeftInState = FADE_SECONDS;

				Song* pPrevSelectedSong = GetCurWheelItemDatas()[m_iSelection].m_pSong;
				CString sPrevSelectedSection = GetCurWheelItemDatas()[m_iSelection].m_sSectionName;

				// change the sort order
				GAMESTATE->m_SongSortOrder = SongSortOrder( (GAMESTATE->m_SongSortOrder+1) % NUM_SORT_ORDERS );
				SCREENMAN->SendMessageToTopScreen( SM_SortOrderChanged, 0 );
				m_sExpandedSectionName = GetSectionNameFromSongAndSort( pPrevSelectedSong, GAMESTATE->m_SongSortOrder );
				//RebuildWheelItems();

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

				// If changed sort to "BEST", put selection on most popular song
				if( GAMESTATE->m_SongSortOrder == SORT_MOST_PLAYED )
				{
					for( i=0; i<GetCurWheelItemDatas().GetSize(); i++ )
					{
						if( GetCurWheelItemDatas()[i].m_pSong != NULL )
						{
							m_iSelection = i;
							break;
						}
					}
				}

				SCREENMAN->SendMessageToTopScreen( SM_SongChanged, 0 );

				RebuildWheelItemDisplays();

				TweenOnScreen();
			}
			break;
		case STATE_FLYING_ON_AFTER_NEXT_SORT:
			m_WheelState = STATE_SELECTING_MUSIC;	// now, wait for input
			break;
		case STATE_TWEENING_ON_SCREEN:
			m_fTimeLeftInState = 0;
			if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
			{
//				if ( m_bUseRandomExtra )
//				{
//					MUSIC->Stop();
//					m_soundExpand.Play();
//					m_WheelState = STATE_ROULETTE_SPINNING;
//					m_SortOrder = SORT_GROUP;
//					m_MusicSortDisplay.SetDiffuse( D3DXCOLOR(1,1,1,0) );
//					m_MusicSortDisplay.SetEffectNone();
//					BuildWheelItemDatas( m_WheelItemDatas[SORT_GROUP], SORT_GROUP, true );
//				}
//				else
				{
					m_WheelState = STATE_LOCKED;
					m_soundStart.Play();
					m_fLockedWheelVelocity = 0;
				}
			}
			else
			{
				m_WheelState = STATE_SELECTING_MUSIC;

			}
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
				const float SwitchTimes[] = { 0.5f, 1.3f, 0.8f, 0.4f, 0.2f };
				ASSERT(m_iSwitchesLeftInSpinDown >= 0 && m_iSwitchesLeftInSpinDown <= 4);
				m_fTimeLeftInState = SwitchTimes[m_iSwitchesLeftInSpinDown];

				LOG->Trace( "m_iSwitchesLeftInSpinDown id %d, m_fTimeLeftInState is %f", m_iSwitchesLeftInSpinDown, m_fTimeLeftInState );

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
		
		float fSpringForce = - m_fPositionOffsetFromSelection * LOCKED_INITIAL_VELOCITY;
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
			fSpinSpeed = 1.0f/ROULETTE_SWITCH_SECONDS;
		else
			fSpinSpeed = 0.2f + fabsf(m_fPositionOffsetFromSelection)/SWITCH_SECONDS;

		if( m_fPositionOffsetFromSelection > 0 )
		{
			m_fPositionOffsetFromSelection -= fSpinSpeed*fDeltaTime;
			if( m_fPositionOffsetFromSelection < 0 )
			{
				m_fPositionOffsetFromSelection = 0;
//				m_fTimeLeftBeforePlayMusicSample = SAMPLE_MUSIC_DELAY;
			}
		}
		else if( m_fPositionOffsetFromSelection < 0 )
		{
			m_fPositionOffsetFromSelection += fSpinSpeed*fDeltaTime;
			if( m_fPositionOffsetFromSelection > 0 )
			{
				m_fPositionOffsetFromSelection = 0;
//				m_fTimeLeftBeforePlayMusicSample = SAMPLE_MUSIC_DELAY;
			}
		}
	}
}


void MusicWheel::PrevMusic( bool bSendSongChangedMessage )
{
	if( m_WheelState == STATE_LOCKED )
	{
		m_fLockedWheelVelocity = LOCKED_INITIAL_VELOCITY;
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
		m_fLockedWheelVelocity = -LOCKED_INITIAL_VELOCITY;
		m_soundLocked.Play();
		return;
	}

	if( fabsf(m_fPositionOffsetFromSelection) > 0.5f )	// wheel is very busy spinning
		return;
	
	switch( m_WheelState )
	{
	case STATE_SELECTING_MUSIC:
	case STATE_ROULETTE_SPINNING:
	case STATE_ROULETTE_SLOWING_DOWN:
		break;
	default:
		LOG->Trace( "NextMusic() ignored" );
		return;	// don't continue
	}


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

bool MusicWheel::PrevSort()
{
	return NextSort();
}

bool MusicWheel::NextSort()
{
	switch( m_WheelState )
	{
	case STATE_SELECTING_MUSIC:
	case STATE_FLYING_ON_AFTER_NEXT_SORT:
		break;	// fall through
	default:
		return false;	// don't continue
	}

	m_soundChangeSort.Play();

	TweenOffScreen();

	m_WheelState = STATE_FLYING_OFF_BEFORE_NEXT_SORT;
	m_fTimeLeftInState = FADE_SECONDS;
	return true;
}

bool MusicWheel::Select()	// return true of a playable item was chosen
{
	LOG->Trace( "MusicWheel::Select()" );

	if( m_WheelState == STATE_ROULETTE_SPINNING )
	{
		m_WheelState = STATE_ROULETTE_SLOWING_DOWN;
		m_iSwitchesLeftInSpinDown = ROULETTE_SLOW_DOWN_SWITCHES/2+1 + rand()%(ROULETTE_SLOW_DOWN_SWITCHES/2);
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
		StartRoulette();
		return false;

	case TYPE_SONG:
	default:
		
		return true;
	}
}

void MusicWheel::StartRoulette() 
{
	m_WheelState = STATE_ROULETTE_SPINNING;
	GAMESTATE->m_SongSortOrder = SORT_GROUP;
    BuildWheelItemDatas( m_WheelItemDatas[SORT_GROUP], SORT_GROUP, true );
}

bool MusicWheel::IsRouletting() 
{
	return m_WheelState == STATE_ROULETTE_SPINNING;
}

void MusicWheel::TweenOnScreen() 
{
	m_WheelState = STATE_TWEENING_ON_SCREEN;
	m_fTimeLeftInState = FADE_SECONDS; 


	float fX, fY;
	
	fX = GetBannerX(0);
	fY = GetBannerY(0);
	m_sprSelectionOverlay.SetXY( fX+320, fY );
	m_sprSelectionOverlay.BeginTweening( 0.5f );	// sleep
	m_sprSelectionOverlay.BeginTweening( 0.4f, Actor::TWEEN_BIAS_BEGIN );
	m_sprSelectionOverlay.SetTweenX( fX );

	m_ScrollBar.SetX( SCROLL_BAR_X+30 );
	m_ScrollBar.BeginTweening( 0.7f );	// sleep
	m_ScrollBar.BeginTweening( 0.2f, Actor::TWEEN_BIAS_BEGIN );
	m_ScrollBar.SetTweenX( SCROLL_BAR_X );	

	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemDisplay& display = m_WheelItemDisplays[i];
		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS_TO_DRAW/2 + m_fPositionOffsetFromSelection;

		float fX = GetBannerX(fThisBannerPositionOffsetFromSelection);
		float fY = GetBannerY(fThisBannerPositionOffsetFromSelection);
		display.SetXY( fX+320, fY );
		display.BeginTweening( 0.04f*i );	// sleep
		display.BeginTweening( 0.2f, Actor::TWEEN_BIAS_BEGIN );
		display.SetTweenX( fX );
	}
}
						   
void MusicWheel::TweenOffScreen()
{
	m_WheelState = STATE_TWEENING_OFF_SCREEN;
	m_fTimeLeftInState = FADE_SECONDS;


	float fX, fY;
	fX = GetBannerX(0);
	fY = GetBannerY(0);
	m_sprSelectionOverlay.SetXY( fX, fY );
	m_sprSelectionOverlay.BeginTweening( 0 );	// sleep
	m_sprSelectionOverlay.BeginTweening( 0.2f, Actor::TWEEN_BIAS_END );
	m_sprSelectionOverlay.SetTweenX( fX+320 );

	m_ScrollBar.BeginTweening( 0 );
	m_ScrollBar.BeginTweening( 0.2f, Actor::TWEEN_BIAS_BEGIN );
	m_ScrollBar.SetTweenX( SCROLL_BAR_X+30 );	

	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemDisplay& display = m_WheelItemDisplays[i];
		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS_TO_DRAW/2 + m_fPositionOffsetFromSelection;

		float fX = GetBannerX(fThisBannerPositionOffsetFromSelection);
		float fY = GetBannerY(fThisBannerPositionOffsetFromSelection);
		display.SetXY( fX, fY );
		display.BeginTweening( 0.04f*i );	// sleep
		display.BeginTweening( 0.2f, Actor::TWEEN_BIAS_END );
		display.SetTweenX( fX+320 );
	}



}
