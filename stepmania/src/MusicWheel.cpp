#include "stdafx.h"
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

#include "MusicWheel.h"
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


// MusicWheel stuff
#define FADE_SECONDS				THEME->GetMetricF("MusicWheel","FadeSeconds")
#define SWITCH_SECONDS				THEME->GetMetricF("MusicWheel","SwitchSeconds")
#define ROULETTE_SWITCH_SECONDS		THEME->GetMetricF("MusicWheel","RouletteSwitchSeconds")
#define ROULETTE_SLOW_DOWN_SWITCHES	THEME->GetMetricI("MusicWheel","RouletteSlowDownSwitches")
#define LOCKED_INITIAL_VELOCITY		THEME->GetMetricF("MusicWheel","LockedInitialVelocity")
#define SCROLL_BAR_X				THEME->GetMetricF("MusicWheel","ScrollBarX")
#define SCROLL_BAR_HEIGHT			THEME->GetMetricI("MusicWheel","ScrollBarHeight")
#define ITEM_CURVE_X				THEME->GetMetricF("MusicWheel","ItemCurveX")
#define ITEM_SPACING_Y				THEME->GetMetricF("MusicWheel","ItemSpacingY")
#define NUM_SECTION_COLORS			THEME->GetMetricI("MusicWheel","NumSectionColors")
#define SECTION_COLORS( i )			THEME->GetMetricC("MusicWheel",ssprintf("SectionColor%d",i+1))
#define DEFAULT_SCROLL_DIRECTION	THEME->GetMetricI("Notes","DefaultScrollDirection")


const int MAX_WHEEL_SOUND_SPEED = 15;
float g_fItemSpacingY, g_fItemCurveX;	// cache

inline RageColor GetNextSectionColor() {
	static int i=0;
	i = i % NUM_SECTION_COLORS;
	return SECTION_COLORS(i++);
}


MusicWheel::MusicWheel() 
{ 
	LOG->Trace( "MusicWheel::MusicWheel()" );
	if (GAMESTATE->m_pCurSong != NULL)
		LOG->Trace( "Current Song: %s", GAMESTATE->m_pCurSong->GetSongDir().GetString() );
	else
		LOG->Trace( "Current Song: NULL" );

	if(DEFAULT_SCROLL_DIRECTION && GAMESTATE->m_pCurSong == NULL) /* check the song is null... incase they have just come back from a song and changed their PlayerOptions */
	{
		for(int i=0; i<NUM_PLAYERS; i++)
			GAMESTATE->m_PlayerOptions[i].m_bReverseScroll = true;
	}


	// update theme metric cache
	g_fItemSpacingY = ITEM_SPACING_Y;
	g_fItemCurveX = ITEM_CURVE_X;

	// for debugging
	if( GAMESTATE->m_CurStyle == STYLE_INVALID )
		GAMESTATE->m_CurStyle = STYLE_DANCE_SINGLE;

	m_sprSelectionOverlay.Load( THEME->GetPathTo("Graphics","select music song highlight") );
	m_sprSelectionOverlay.SetXY( 0, 0 );
	m_sprSelectionOverlay.SetDiffuse( RageColor(1,1,1,1) );
	m_sprSelectionOverlay.SetEffectGlowing( 1.0f, RageColor(1,1,1,0.4f), RageColor(1,1,1,1) );
	AddChild( &m_sprSelectionOverlay );

	m_ScrollBar.SetX( SCROLL_BAR_X ); 
	m_ScrollBar.SetBarHeight( SCROLL_BAR_HEIGHT ); 
	this->AddChild( &m_ScrollBar );
	
	/* We play a lot of this one, so precache it. */
	m_soundChangeMusic.Load(	THEME->GetPathTo("Sounds","select music change"), true );
	m_soundChangeSort.Load(		THEME->GetPathTo("Sounds","select music sort") );
	m_soundExpand.Load(			THEME->GetPathTo("Sounds","select music expand") );
	m_soundStart.Load(			THEME->GetPathTo("Sounds","menu start") );
	m_soundLocked.Load(			THEME->GetPathTo("Sounds","select music locked") );


	// init m_mapGroupNameToBannerColor

	vector<Song*> arraySongs;
	SONGMAN->GetSongs( arraySongs, GAMESTATE->GetNumStagesLeft() );
	SortSongPointerArrayByGroup( arraySongs );
	
	m_iSelection = 0;

	m_WheelState = STATE_SELECTING_MUSIC;
	m_fTimeLeftInState = 0;
	m_fPositionOffsetFromSelection = 0;

	m_iSwitchesLeftInSpinDown = 0;
	
	if( GAMESTATE->IsExtraStage() ||  GAMESTATE->IsExtraStage2() )
	{
		// make the preferred group the group of the last song played.
		if( GAMESTATE->m_sPreferredGroup == "ALL MUSIC" && !PREFSMAN->m_bPickExtraStage )
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
	BuildWheelItemDatas( m_WheelItemDatas[SORT_ROULETTE], SongSortOrder(SORT_ROULETTE) );

	// If there is no currently selected song, select one.
	if( GAMESTATE->m_pCurSong == NULL )
	{
		//Select the first selectable song based on the sort order...
		vector<WheelItemData> &wiWheelItems = m_WheelItemDatas[GAMESTATE->m_SongSortOrder];
		for( unsigned i = 0; i < wiWheelItems.size(); i++ )
		{
			if( wiWheelItems[i].m_pSong != NULL )
			{
				GAMESTATE->m_pCurSong = wiWheelItems[i].m_pSong;
				break;
			}
		}
	
		if( GAMESTATE->m_pCurSong == NULL )
		{
			//XXX What to do here?
			//No selectable songs from the wheel data... 
			//Cursor will be on Random/Roulette, but GAMESTATE->m_pCurSong will be NULL
			//Should we error out? The user won't have an enjoyable game experience at this point... 
			//Should we find the pointer to the roulette entry? (the old way did that - maybe not intentional though)
			//For now, write out to the debug log...
			LOG->Trace("MusicWheel::MusicWheel() - No selectable songs found in WheelData!");
		}
	}


	// Select the the previously selected song (if any)
	bool selected = SelectSong(GAMESTATE->m_pCurSong);
	// Select the the previously selected course (if any)
	if(!selected) selected = SelectCourse(GAMESTATE->m_pCurCourse);
	if(!selected) SetOpenGroup("");

	// rebuild the WheelItems that appear on screen
	RebuildWheelItemDisplays();
}

MusicWheel::~MusicWheel()
{
}

bool MusicWheel::SelectSong( const Song *p )
{
	if(p == NULL)
		return false;

	unsigned i;
	vector<WheelItemData> &from = m_WheelItemDatas[GAMESTATE->m_SongSortOrder];
	for( i=0; i<from.size(); i++ )
	{
		if( from[i].m_pSong == p )
		{
			// make its group the currently expanded group
			SetOpenGroup(from[i].m_sSectionName);
			break;
		}
	}

	if(i == from.size())
		return false;

	for( i=0; i<m_CurWheelItemData.size(); i++ )
	{
		if( m_CurWheelItemData[i]->m_pSong == p )
			m_iSelection = i;		// select it
	}
	return true;
}

bool MusicWheel::SelectCourse( const Course *p )
{
	if(p == NULL)
		return false;

	unsigned i;
	vector<WheelItemData> &from = m_WheelItemDatas[GAMESTATE->m_SongSortOrder];
	for( i=0; i<from.size(); i++ )
	{
		if( from[i].m_pCourse == p )
		{
			// make its group the currently expanded group
			SetOpenGroup(from[i].m_sSectionName);
			break;
		}
	}

	if(i == from.size())
		return false;

	for( i=0; i<m_CurWheelItemData.size(); i++ )
	{
		if( m_CurWheelItemData[i]->m_pCourse == p )
			m_iSelection = i;		// select it
	}

	return true;
}

void MusicWheel::GetSongList(vector<Song*> &arraySongs, bool bRoulette )
{
	vector<Song*> apAllSongs;
	SONGMAN->GetSongs( apAllSongs, GAMESTATE->GetNumStagesLeft() );

	// copy only songs that have at least one Notes for the current GameMode
	for( unsigned i=0; i<apAllSongs.size(); i++ )
	{
		Song* pSong = apAllSongs[i];

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

		vector<Notes*> arraySteps;
		pSong->GetNotes( arraySteps, GAMESTATE->GetCurrentStyleDef()->m_NotesType );

		if( !arraySteps.empty() )
			arraySongs.push_back( pSong );
	}
}

void MusicWheel::BuildWheelItemDatas( vector<WheelItemData> &arrayWheelItemDatas, SongSortOrder so )
{
	unsigned i;

	if(so == SongSortOrder(SORT_ROULETTE) && GAMESTATE->m_PlayMode != PLAY_MODE_ARCADE)
		return; /* only used in arcade */

	switch( GAMESTATE->m_PlayMode )
	{
	case PLAY_MODE_ARCADE:
		{
			///////////////////////////////////
			// Make an array of Song*, then sort them
			///////////////////////////////////
			vector<Song*> arraySongs;
			
			GetSongList(arraySongs, so == SORT_ROULETTE);

			// sort the songs
			switch( so )
			{
			case SORT_GROUP:
			case SORT_ROULETTE:
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
				if( arraySongs.size() > 30 )
					arraySongs.erase(arraySongs.begin()+30, arraySongs.end());
				break;
			default:
				ASSERT(0);	// unhandled SortOrder
			}



			///////////////////////////////////
			// Build an array of WheelItemDatas from the sorted list of Song*'s
			///////////////////////////////////
			arrayWheelItemDatas.clear();	// clear out the previous wheel items 

			bool bUseSections = false;
			switch( so )
			{
//			case SORT_GROUP_NOHEADER:	bUseSections = false; break;
			case SORT_MOST_PLAYED:	bUseSections = false;	break;
			case SORT_BPM:			bUseSections = false;	break;
			case SORT_GROUP:		bUseSections = GAMESTATE->m_sPreferredGroup == "ALL MUSIC";	break;
			case SORT_TITLE:		bUseSections = true;	break;
			case SORT_ROULETTE:		bUseSections = false;	break;
			default:		ASSERT( false );
			}

			if( PREFSMAN->m_MusicWheelUsesSections == PrefsManager::NEVER || (so != SORT_TITLE && PREFSMAN->m_MusicWheelUsesSections == PrefsManager::ABC_ONLY ))
				bUseSections = false;

			if( bUseSections )
			{
				// make WheelItemDatas with sections
				CString sLastSection = "";
				RageColor colorSection;
				for( unsigned i=0; i< arraySongs.size(); i++ )
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
						arrayWheelItemDatas.push_back( WheelItemData(TYPE_SECTION, NULL, sThisSection, NULL, colorSection) );
						sLastSection = sThisSection;
					}

					arrayWheelItemDatas.push_back( WheelItemData( TYPE_SONG, pSong, sThisSection, NULL, SONGMAN->GetSongColor(pSong)) );
				}
			}
			else
			{
				for( unsigned i=0; i<arraySongs.size(); i++ )
				{
					Song* pSong = arraySongs[i];
					if( GAMESTATE->m_sPreferredGroup != "ALL MUSIC"  &&  pSong->m_sGroupName != GAMESTATE->m_sPreferredGroup )
						continue;	// skip
					arrayWheelItemDatas.push_back( WheelItemData(TYPE_SONG, pSong, "", NULL, SONGMAN->GetSongColor(pSong)) );
				}
			}
		}


		if( so != SORT_ROULETTE )
		{
			arrayWheelItemDatas.push_back( WheelItemData(TYPE_ROULETTE, NULL, "", NULL, RageColor(1,0,0,1)) );
			arrayWheelItemDatas.push_back( WheelItemData(TYPE_RANDOM, NULL, "", NULL, RageColor(1,0,0,1)) );
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

			for( unsigned i=0; i<arrayWheelItemDatas.size(); i++ )
			{
				if( arrayWheelItemDatas[i].m_pSong == pSong )
				{
					bFoundExtraSong = true;
					break;
				}
			}
			
			if( !bFoundExtraSong )
				arrayWheelItemDatas.push_back( WheelItemData(TYPE_SONG, pSong, "", NULL, GAMESTATE->GetStageColor()) );
		}

		break;
	case PLAY_MODE_NONSTOP:
	case PLAY_MODE_ONI:
	case PLAY_MODE_ENDLESS:
		{
			vector<Course*> apCourses;
			switch( GAMESTATE->m_PlayMode )
			{
			case PLAY_MODE_NONSTOP:	SONGMAN->GetNonstopCourses( apCourses );	break;
			case PLAY_MODE_ONI:		SONGMAN->GetOniCourses( apCourses );		break;
			case PLAY_MODE_ENDLESS:	SONGMAN->GetEndlessCourses( apCourses );	break;
			default:	ASSERT(0);
			}

			for( unsigned c=0; c<apCourses.size(); c++ )	// foreach course
			{
				Course* pCourse = apCourses[c];

				// check that this course has at least one song playable in the current style
				vector<Song*> apSongs;
				vector<Notes*> apNotes;
				CStringArray asModifiers;
				pCourse->GetSongAndNotesForCurrentStyle( apSongs, apNotes, asModifiers, false );

				if( !apNotes.empty() )
					arrayWheelItemDatas.push_back( WheelItemData(TYPE_COURSE, NULL, "", pCourse, pCourse->GetColor()) );
			}
		}
		break;
	default:
		ASSERT(0);	// invalid PlayMode
	}

	// init music status icons
	for( i=0; i<arrayWheelItemDatas.size(); i++ )
	{
		Song* pSong = arrayWheelItemDatas[i].m_pSong;
		if( pSong == NULL )
			continue;

		WheelItemData& WID = arrayWheelItemDatas[i];
		WID.m_Flags.bHasBeginnerOr1Meter = pSong->IsEasy( NOTES_TYPE_DANCE_SINGLE );
		WID.m_Flags.bEdits = pSong->HasEdits( NOTES_TYPE_DANCE_SINGLE );
		WID.m_Flags.iStagesForSong = SongManager::GetNumStagesForSong( pSong );
	}

	// init crowns
	if( so == SORT_MOST_PLAYED )
	{
		// init crown icons 
		for( i=0; i< min(3u,arrayWheelItemDatas.size()); i++ )
		{
			WheelItemData& WID = arrayWheelItemDatas[i];
			WID.m_Flags.iPlayersBestNumber = i+1;
		}
	}

	if( arrayWheelItemDatas.empty() )
	{
		arrayWheelItemDatas.push_back( WheelItemData(TYPE_SECTION, NULL, "- EMPTY -", NULL, RageColor(1,0,0,1)) );
	}
}

float MusicWheel::GetBannerY( float fPosOffsetsFromMiddle )
{
	return roundf( fPosOffsetsFromMiddle*g_fItemSpacingY );
}

float MusicWheel::GetBannerX( float fPosOffsetsFromMiddle )
{	
	float fX = (1-cosf(fPosOffsetsFromMiddle/PI))*g_fItemCurveX;
	
	return roundf( fX );
}

void MusicWheel::RebuildWheelItemDisplays()
{
	// rewind to first index that will be displayed;
	int iIndex = m_iSelection;
	if( m_iSelection > int(m_CurWheelItemData.size()-1) )
		m_iSelection = 0;
	
	iIndex -= NUM_WHEEL_ITEMS_TO_DRAW/2;
	while(iIndex < 0)
		iIndex += m_CurWheelItemData.size();

	// iIndex is now the index of the lowest WheelItem to draw
	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemData     *data   = m_CurWheelItemData[iIndex];
		WheelItemDisplay& display = m_WheelItemDisplays[i];

		display.LoadFromWheelItemData( data );

		// increment iIndex
		iIndex++;
		if( iIndex > int(m_CurWheelItemData.size()-1) )
			iIndex = 0;
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
		case STATE_RANDOM_SPINNING:
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
		}

		if( m_WheelState == STATE_LOCKED  &&  i != NUM_WHEEL_ITEMS_TO_DRAW/2 )
			display.m_fPercentGray = 0.5f;
		else
			display.m_fPercentGray = 0;

		display.Draw();
	}

	ActorFrame::DrawPrimitives();
}

void MusicWheel::UpdateScrollbar()
{
	int total_num_items = m_CurWheelItemData.size();
	float item_at=m_iSelection - m_fPositionOffsetFromSelection;

	if(NUM_WHEEL_ITEMS_TO_DRAW > total_num_items) {
		m_ScrollBar.SetPercentage( 0, 1 );
	} else {
		float size = float(NUM_WHEEL_ITEMS_TO_DRAW) / total_num_items;
		float center = item_at / total_num_items;
		size *= 0.5f;

		m_ScrollBar.SetPercentage( center - size, center + size );
	}
}

void MusicWheel::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	unsigned i;
	for( i=0; i<int(NUM_WHEEL_ITEMS_TO_DRAW); i++ )
	{
		WheelItemDisplay& display = m_WheelItemDisplays[i];

		display.Update( fDeltaTime );
	}

	UpdateScrollbar();

	if( m_Moving )
	{
		m_TimeBeforeMovingBegins -= fDeltaTime;
		m_TimeBeforeMovingBegins = max(m_TimeBeforeMovingBegins, 0);
	}

	// update wheel state
	m_fTimeLeftInState -= fDeltaTime;
	if( m_fTimeLeftInState <= 0 )	// time to go to a new state
	{
		switch( m_WheelState )
		{
		case STATE_FLYING_OFF_BEFORE_NEXT_SORT:
			{
				Song* pPrevSelectedSong = m_CurWheelItemData[m_iSelection]->m_pSong;
				CString sPrevSelectedSection = m_CurWheelItemData[m_iSelection]->m_sSectionName;

				// change the sort order
				GAMESTATE->m_SongSortOrder = SongSortOrder( (GAMESTATE->m_SongSortOrder+1) % NUM_SORT_ORDERS );

				SCREENMAN->SendMessageToTopScreen( SM_SortOrderChanged, 0 );
				SetOpenGroup(GetSectionNameFromSongAndSort( pPrevSelectedSong, GAMESTATE->m_SongSortOrder ));

				//RebuildWheelItems();

				m_iSelection = 0;

				if( pPrevSelectedSong != NULL )		// the previous selected item was a song
					SelectSong(pPrevSelectedSong);
				else	// the previously selected item was a section
				{
					// find the previously selected song, and select it
					for( i=0; i<m_CurWheelItemData.size(); i++ )
					{
						if( m_CurWheelItemData[i]->m_sSectionName == sPrevSelectedSection )
						{
							m_iSelection = i;
							break;
						}
					}
				}

				// If changed sort to "BEST", put selection on most popular song
				if( GAMESTATE->m_SongSortOrder == SORT_MOST_PLAYED )
				{
					for( i=0; i<m_CurWheelItemData.size(); i++ )
					{
						if( m_CurWheelItemData[i]->m_pSong != NULL )
						{
							m_iSelection = i;
							break;
						}
					}
				}

				SCREENMAN->SendMessageToTopScreen( SM_SongChanged, 0 );
				RebuildWheelItemDisplays();
				TweenOnScreen(true);
				m_WheelState = STATE_FLYING_ON_AFTER_NEXT_SORT;
			}
			break;

		case STATE_FLYING_ON_AFTER_NEXT_SORT:
			m_WheelState = STATE_SELECTING_MUSIC;	// now, wait for input
			break;

		case STATE_TWEENING_ON_SCREEN:
			m_fTimeLeftInState = 0;
			if( (GAMESTATE->IsExtraStage() && !PREFSMAN->m_bPickExtraStage) || GAMESTATE->IsExtraStage2() )
			{
//				if ( m_bUseRandomExtra )
//				{
//					SOUNDMAN->StopMusic();
//					m_soundExpand.Play();
//					m_WheelState = STATE_ROULETTE_SPINNING;
//					m_SortOrder = SORT_GROUP;
//					m_MusicSortDisplay.SetDiffuse( RageColor(1,1,1,0) );
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
		case STATE_RANDOM_SPINNING:
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

				/* Send this again so the screen starts sample music. */
				SCREENMAN->SendMessageToTopScreen( SM_SongChanged, 0 );
			}
			else
			{
				m_iSwitchesLeftInSpinDown--;
				const float SwitchTimes[] = { 0.5f, 1.3f, 0.8f, 0.4f, 0.2f };
				ASSERT(m_iSwitchesLeftInSpinDown >= 0 && m_iSwitchesLeftInSpinDown <= 4);
				m_fTimeLeftInState = SwitchTimes[m_iSwitchesLeftInSpinDown];

				LOG->Trace( "m_iSwitchesLeftInSpinDown id %d, m_fTimeLeftInState is %f", m_iSwitchesLeftInSpinDown, m_fTimeLeftInState );

				if( m_iSwitchesLeftInSpinDown < 2 )
					ChangeMusic(randomf(0,1) >= 0.5f? 1:-1);
				else
					ChangeMusic(1);
			}
			break;
		default:
			ASSERT(0);	// all state changes should be handled explitily
			break;
		}
	}

	if( m_WheelState == STATE_LOCKED )
	{
		/* Do this in at most .1 sec chunks, so we don't get weird if we
		 * stop for some reason (and so it behaves the same when being
		 * single stepped). */
		float tm = fDeltaTime;
		while(tm > 0)
		{
			float t = min(tm, 0.1f);
			tm -= t;

			m_fPositionOffsetFromSelection = clamp( m_fPositionOffsetFromSelection, -0.3f, +0.3f );

			float fSpringForce = - m_fPositionOffsetFromSelection * LOCKED_INITIAL_VELOCITY;
			m_fLockedWheelVelocity += fSpringForce;

			float fDrag = -m_fLockedWheelVelocity * t*4;
			m_fLockedWheelVelocity += fDrag;

			m_fPositionOffsetFromSelection  += m_fLockedWheelVelocity*t;

			if( fabsf(m_fPositionOffsetFromSelection) < 0.01f  &&  fabsf(m_fLockedWheelVelocity) < 0.01f )
			{
				m_fPositionOffsetFromSelection = 0;
				m_fLockedWheelVelocity = 0;
			}
		}
	}
	else if( IsMoving() )
	{
		/* We're automatically moving.  Move linearly, and don't clamp
		 * to the selection. */
		float fSpinSpeed = m_SpinSpeed*m_Moving;
		m_fPositionOffsetFromSelection -= fSpinSpeed*fDeltaTime;

		/* Make sure that we don't go further than 1 away, in case the
		 * speed is very high or we miss a lot of frames. */
		m_fPositionOffsetFromSelection  = clamp(m_fPositionOffsetFromSelection, -1.0f, 1.0f);
		
		/* If it passed the selection, move again. */
		if((m_Moving == -1 && m_fPositionOffsetFromSelection >= 0) ||
		   (m_Moving == 1 && m_fPositionOffsetFromSelection <= 0))
		{
			ChangeMusic(m_Moving);

			if(PREFSMAN->m_iMusicWheelSwitchSpeed < MAX_WHEEL_SOUND_SPEED)
				m_soundChangeMusic.Play();
		}

		if(PREFSMAN->m_iMusicWheelSwitchSpeed >= MAX_WHEEL_SOUND_SPEED &&
			m_MovingSoundTimer.PeekDeltaTime() >= 1.0f / MAX_WHEEL_SOUND_SPEED)
		{
			m_MovingSoundTimer.GetDeltaTime();
			m_soundChangeMusic.Play();
		}
	}
	else
	{
		// "rotate" wheel toward selected song
		float fSpinSpeed = 0.2f + fabsf(m_fPositionOffsetFromSelection)/SWITCH_SECONDS;

		if( m_fPositionOffsetFromSelection > 0 )
		{
			m_fPositionOffsetFromSelection -= fSpinSpeed*fDeltaTime;
			if( m_fPositionOffsetFromSelection < 0 )
				m_fPositionOffsetFromSelection = 0;
		}
		else if( m_fPositionOffsetFromSelection < 0 )
		{
			m_fPositionOffsetFromSelection += fSpinSpeed*fDeltaTime;
			if( m_fPositionOffsetFromSelection > 0 )
				m_fPositionOffsetFromSelection = 0;
		}
	}
}


void MusicWheel::ChangeMusic(int dist)
{
	m_iSelection += dist;
	if( m_iSelection < 0 )
		m_iSelection = m_CurWheelItemData.size()-1;
	else if( m_iSelection > int(m_CurWheelItemData.size()-1) )
		m_iSelection = 0;

	RebuildWheelItemDisplays();

	m_fPositionOffsetFromSelection += dist;

	SCREENMAN->SendMessageToTopScreen( SM_SongChanged, 0 );

	/* If we're moving automatically, don't play this; it'll be called in Update. */
	if(!IsMoving())
		m_soundChangeMusic.Play();
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

	TweenOffScreen(true);

	m_WheelState = STATE_FLYING_OFF_BEFORE_NEXT_SORT;
	return true;
}
bool MusicWheel::Select()	// return true of a playable item was chosen
{
	LOG->Trace( "MusicWheel::Select()" );

	if( m_WheelState == STATE_ROULETTE_SLOWING_DOWN )
		return false;

	m_Moving = 0;

	if( m_WheelState == STATE_ROULETTE_SPINNING )
	{
		m_WheelState = STATE_ROULETTE_SLOWING_DOWN;
		m_iSwitchesLeftInSpinDown = ROULETTE_SLOW_DOWN_SWITCHES/2+1 + rand()%(ROULETTE_SLOW_DOWN_SWITCHES/2);
		m_fTimeLeftInState = 0.1f;
		return false;
	}


	if( m_WheelState == STATE_RANDOM_SPINNING )
	{
		m_fPositionOffsetFromSelection = max(m_fPositionOffsetFromSelection, 0.3f);
		m_WheelState = STATE_LOCKED;
		m_soundStart.Play();
		m_fLockedWheelVelocity = 0;
		SCREENMAN->SendMessageToTopScreen( SM_SongChanged, 0 );
		return false;
	}

	switch( m_CurWheelItemData[m_iSelection]->m_WheelItemType )
	{
	case TYPE_SECTION:
		{
		CString sThisItemSectionName = m_CurWheelItemData[m_iSelection]->m_sSectionName;
		if( m_sExpandedSectionName == sThisItemSectionName )	// already expanded
			m_sExpandedSectionName = "";		// collapse it
		else				// already collapsed
			m_sExpandedSectionName = sThisItemSectionName;	// expand it

		SetOpenGroup(m_sExpandedSectionName);
	
		RebuildWheelItemDisplays();


		m_soundExpand.Play();


		m_iSelection = 0;	// reset in case we can't find the last selected song
		// find the section header and select it
		for( unsigned  i=0; i<m_CurWheelItemData.size(); i++ )
		{
			if( m_CurWheelItemData[i]->m_WheelItemType == TYPE_SECTION  
				&&  m_CurWheelItemData[i]->m_sSectionName == sThisItemSectionName )
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
	case TYPE_RANDOM:
		StartRandom();
		return false;

	case TYPE_SONG:
	default:
		
		return true;
	}
}

void MusicWheel::StartRoulette() 
{
	m_WheelState = STATE_ROULETTE_SPINNING;
	m_Moving = 1;
	m_TimeBeforeMovingBegins = 0;
	m_SpinSpeed = 1.0f/ROULETTE_SWITCH_SECONDS;
	SetOpenGroup("", SongSortOrder(SORT_ROULETTE));
}

void MusicWheel::StartRandom()
{
	/* Shuffle the roulette wheel. */
	unsigned total =  m_WheelItemDatas[SORT_ROULETTE].size();
	for(unsigned i = 0; i < total; ++i)
		swap(m_WheelItemDatas[SORT_ROULETTE][i], m_WheelItemDatas[SORT_ROULETTE][rand() % total]);

	SetOpenGroup("", SongSortOrder(SORT_ROULETTE));

	m_Moving = -1;
	m_TimeBeforeMovingBegins = 0;
	m_SpinSpeed = 1.0f/ROULETTE_SWITCH_SECONDS;
	m_SpinSpeed *= 2.0f; /* faster! */
	m_WheelState = STATE_RANDOM_SPINNING;
	RebuildWheelItemDisplays();
}

void MusicWheel::SetOpenGroup(CString group, SongSortOrder so)
{
	if(so == NUM_SORT_ORDERS)
		so = GAMESTATE->m_SongSortOrder;

	m_sExpandedSectionName = group;

	WheelItemData *old = NULL;
	if(!m_CurWheelItemData.empty())
		old = m_CurWheelItemData[m_iSelection];

	m_CurWheelItemData.clear();
	vector<WheelItemData> &from = m_WheelItemDatas[so];
	unsigned i;
	for(i = 0; i < from.size(); ++i)
	{
		if((from[i].m_WheelItemType == TYPE_SONG ||
			from[i].m_WheelItemType == TYPE_COURSE) &&
		     !from[i].m_sSectionName.empty() &&
			 from[i].m_sSectionName != group)
			 continue;
		m_CurWheelItemData.push_back(&from[i]);

	}

	m_iSelection = 0;
	for(i = 0; i < m_CurWheelItemData.size(); ++i)
	{
		if(m_CurWheelItemData[i] == old)
			m_iSelection=i;
	}

	RebuildWheelItemDisplays();
}

bool MusicWheel::IsRouletting() const
{
	return m_WheelState == STATE_ROULETTE_SPINNING || m_WheelState == STATE_ROULETTE_SLOWING_DOWN ||
		   m_WheelState == STATE_RANDOM_SPINNING;
}

int MusicWheel::IsMoving() const
{
	return m_Moving && m_TimeBeforeMovingBegins == 0;
}

void MusicWheel::TweenOnScreen(bool changing_sort)
{
	float factor = 1.0f;
	if(changing_sort) factor = 0.25;

	m_WheelState = STATE_TWEENING_ON_SCREEN;

	float fX = GetBannerX(0), fY = GetBannerY(0);
	
	m_sprSelectionOverlay.SetXY( fX+320, fY );

	if(changing_sort) {
		m_sprSelectionOverlay.BeginTweening( 0.04f * NUM_WHEEL_ITEMS_TO_DRAW/2 * factor );	// sleep
		m_sprSelectionOverlay.BeginTweening( 0.2f * factor, Actor::TWEEN_BIAS_BEGIN );
	} else {
		m_sprSelectionOverlay.BeginTweening( 0.05f * factor );	// sleep
		m_sprSelectionOverlay.BeginTweening( 0.4f * factor, Actor::TWEEN_BIAS_BEGIN );
	}
	m_sprSelectionOverlay.SetTweenX( fX );

	m_ScrollBar.SetX( SCROLL_BAR_X+30 );
	if(changing_sort)
		m_ScrollBar.BeginTweening( 0.2f * factor );	// sleep
	else
		m_ScrollBar.BeginTweening( 0.7f * factor );	// sleep
	m_ScrollBar.BeginTweening( 0.2f * factor , Actor::TWEEN_BIAS_BEGIN );
	m_ScrollBar.SetTweenX( SCROLL_BAR_X );	

	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemDisplay& display = m_WheelItemDisplays[i];
		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS_TO_DRAW/2 + m_fPositionOffsetFromSelection;

		float fX = GetBannerX(fThisBannerPositionOffsetFromSelection);
		float fY = GetBannerY(fThisBannerPositionOffsetFromSelection);
		display.SetXY( fX+320, fY );
		display.BeginTweening( 0.04f*i * factor );	// sleep
		display.BeginTweening( 0.2f * factor, Actor::TWEEN_BIAS_BEGIN );
		display.SetTweenX( fX );
	}

	m_fTimeLeftInState = TweenTime() + 0.100f;
}
						   
void MusicWheel::TweenOffScreen(bool changing_sort)
{
	float factor = 1.0f;
	if(changing_sort) factor = 0.25;

	m_WheelState = STATE_TWEENING_OFF_SCREEN;

	float fX, fY;
	fX = GetBannerX(0);
	fY = GetBannerY(0);
	m_sprSelectionOverlay.SetXY( fX, fY );

	if(changing_sort) {
		/* When changing sort, tween the overlay with the item in the center;
		 * having it separate looks messy when we're moving fast. */
		m_sprSelectionOverlay.BeginTweening( 0.04f * NUM_WHEEL_ITEMS_TO_DRAW/2 * factor );	// sleep
		m_sprSelectionOverlay.BeginTweening( 0.2f * factor, Actor::TWEEN_BIAS_END );
	} else {
		m_sprSelectionOverlay.BeginTweening( 0 );	// sleep
		m_sprSelectionOverlay.BeginTweening( 0.2f * factor, Actor::TWEEN_BIAS_END );
	}
	m_sprSelectionOverlay.SetTweenX( fX+320 );

	m_ScrollBar.BeginTweening( 0 );
	m_ScrollBar.BeginTweening( 0.2f * factor, Actor::TWEEN_BIAS_BEGIN );
	m_ScrollBar.SetTweenX( SCROLL_BAR_X+30 );	

	for( int i=0; i<NUM_WHEEL_ITEMS_TO_DRAW; i++ )
	{
		WheelItemDisplay& display = m_WheelItemDisplays[i];
		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS_TO_DRAW/2 + m_fPositionOffsetFromSelection;

		float fX = GetBannerX(fThisBannerPositionOffsetFromSelection);
		float fY = GetBannerY(fThisBannerPositionOffsetFromSelection);
		display.SetXY( fX, fY );
		display.BeginTweening( 0.04f*i * factor );	// sleep
		display.BeginTweening( 0.2f * factor, Actor::TWEEN_BIAS_END );
		display.SetTweenX( fX+320 );
	}

	m_fTimeLeftInState = TweenTime() + 0.100f;
}

CString MusicWheel::GetSectionNameFromSongAndSort( Song* pSong, SongSortOrder so )
{
	if( pSong == NULL )
		return "";

	CString sTemp;

	switch( so )
	{
	case SORT_GROUP:	
		sTemp = pSong->m_sGroupName;
		return sTemp;
//		case SORT_ARTIST:	
//			sTemp = pSong->m_sArtist;
//			sTemp.MakeUpper();
//			sTemp =  (sTemp.GetLength() > 0) ? sTemp.Left(1) : "";
//			if( IsAnInt(sTemp) )
//				sTemp = "NUM";
//			return sTemp;
	case SORT_TITLE:
		sTemp = pSong->GetSortTitle();
		sTemp.MakeUpper();
		if(sTemp.empty()) return "";

		sTemp = sTemp[0];
		if( IsAnInt(sTemp) )
			sTemp = "NUM";
		else if(toupper(sTemp[0]) < 'A' || toupper(sTemp[0]) > 'Z')
			sTemp = "OTHER";
		return sTemp;
	case SORT_BPM:
	case SORT_MOST_PLAYED:
	default:
		return "";
	}
}

void MusicWheel::Move(int n)
{
	if(n == m_Moving)
		return;

	if( m_WheelState == STATE_LOCKED )
	{
		if(n)
		{
			m_fLockedWheelVelocity = LOCKED_INITIAL_VELOCITY;
			m_soundLocked.Play();
		}
		return;
	}

	/* If we're not selecting, discard this.  We won't ignore it; we'll
	 * get called again every time the key is repeated. */
	if( m_WheelState != STATE_SELECTING_MUSIC )
		return;

	if(m_Moving != 0 && n == 0 && m_TimeBeforeMovingBegins == 0)
	{
		/* We were moving, and now we're stopping.  If we're really close to
		 * the selection, move to the next one, so we have a chance to spin down
		 * smoothly. */
		if(fabsf(m_fPositionOffsetFromSelection) < 0.25f )
			ChangeMusic(m_Moving);

		/* Make sure the user always gets an SM_SongChanged when
		 * Moving() is 0, so the final banner, etc. always gets set. */
		SCREENMAN->SendMessageToTopScreen( SM_SongChanged, 0 );
	}

	m_TimeBeforeMovingBegins = TIME_BEFORE_SLOW_REPEATS;
	m_SpinSpeed = float(PREFSMAN->m_iMusicWheelSwitchSpeed);
	m_Moving = n;

	if( fabsf(m_fPositionOffsetFromSelection) > 0.5f )	// wheel is very busy spinning
		return;
	
	if(m_Moving)
		ChangeMusic(m_Moving);
}
