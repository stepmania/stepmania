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

#include "MusicWheel.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"	// for sending SM_PlayMusicSample
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "RageMath.h"
#include <math.h>
#include "ThemeManager.h"
#include "song.h"
#include "Course.h"
#include "RageDisplay.h"
#include "RageTextureManager.h"
#include "Banner.h"
#include "Steps.h"
#include "UnlockSystem.h"
#include "ModeChoice.h"


#define FADE_SECONDS				THEME->GetMetricF("MusicWheel","FadeSeconds")
#define SWITCH_SECONDS				THEME->GetMetricF("MusicWheel","SwitchSeconds")
#define ROULETTE_SWITCH_SECONDS		THEME->GetMetricF("MusicWheel","RouletteSwitchSeconds")
#define ROULETTE_SLOW_DOWN_SWITCHES	THEME->GetMetricI("MusicWheel","RouletteSlowDownSwitches")
#define LOCKED_INITIAL_VELOCITY		THEME->GetMetricF("MusicWheel","LockedInitialVelocity")
#define SCROLL_BAR_X				THEME->GetMetricF("MusicWheel","ScrollBarX")
#define SCROLL_BAR_HEIGHT			THEME->GetMetricI("MusicWheel","ScrollBarHeight")
CachedThemeMetricF ITEM_CURVE_X		("MusicWheel","ItemCurveX");
CachedThemeMetricF ITEM_SPACING_Y	("MusicWheel","ItemSpacingY");
#define NUM_SECTION_COLORS			THEME->GetMetricI("MusicWheel","NumSectionColors")
#define SECTION_COLORS( i )			THEME->GetMetricC("MusicWheel",ssprintf("SectionColor%d",i+1))
#define SONG_REAL_EXTRA_COLOR		THEME->GetMetricC("MusicWheel","SongRealExtraColor")
#define SHOW_ROULETTE				THEME->GetMetricB("MusicWheel","ShowRoulette")
#define SHOW_RANDOM					THEME->GetMetricB("MusicWheel","ShowRandom")
CachedThemeMetricB	USE_3D			("MusicWheel","Use3D");
CachedThemeMetricI  NUM_WHEEL_ITEMS_METRIC	("MusicWheel","NumWheelItems");
#define NUM_WHEEL_ITEMS				min( MAX_WHEEL_ITEMS, (int) NUM_WHEEL_ITEMS_METRIC )
#define MENU_NAMES					THEME->GetMetric ("MusicWheel","MenuNames")
#define MENU_ACTIONS				THEME->GetMetric ("MusicWheel","MenuActions")

			
const int MAX_WHEEL_SOUND_SPEED = 15;


static const SongSortOrder SortOrder[] =
{
	SORT_GROUP, 
	SORT_TITLE, 
	SORT_BPM, 
	SORT_MOST_PLAYED, 
	SORT_ARTIST,
	SORT_INVALID
};


MusicWheel::MusicWheel() 
{ 
	LOG->Trace( "MusicWheel::MusicWheel()" );
	if (GAMESTATE->m_pCurSong != NULL)
		LOG->Trace( "Current Song: %s", GAMESTATE->m_pCurSong->GetSongDir().c_str() );
	else
		LOG->Trace( "Current Song: NULL" );

	// update theme metric cache
	ITEM_CURVE_X.Refresh();
	ITEM_SPACING_Y.Refresh();
	USE_3D.Refresh();
	NUM_WHEEL_ITEMS_METRIC.Refresh();

	// for debugging
	if( GAMESTATE->m_CurStyle == STYLE_INVALID )
		GAMESTATE->m_CurStyle = STYLE_DANCE_SINGLE;

	m_sprSelectionOverlay.Load( THEME->GetPathToG("MusicWheel highlight") );
	m_sprSelectionOverlay.SetXY( 0, 0 );
	m_sprSelectionOverlay.SetDiffuse( RageColor(1,1,1,1) );
	m_sprSelectionOverlay.SetEffectGlowShift( 1.0f, RageColor(1,1,1,0.4f), RageColor(1,1,1,1) );
	AddChild( &m_sprSelectionOverlay );

	m_ScrollBar.SetX( SCROLL_BAR_X ); 
	m_ScrollBar.SetBarHeight( SCROLL_BAR_HEIGHT ); 
	this->AddChild( &m_ScrollBar );
	
	/* We play a lot of this one, so precache it. */
	m_soundChangeMusic.Load(	THEME->GetPathToS("MusicWheel change"), true );
	m_soundChangeSort.Load(		THEME->GetPathToS("MusicWheel sort") );
	m_soundExpand.Load(			THEME->GetPathToS("MusicWheel expand") );
	m_soundStart.Load(			THEME->GetPathToS("Common start") );
	m_soundLocked.Load(			THEME->GetPathToS("MusicWheel locked") );

	
	m_iSelection = 0;
	m_LastSongSortOrder = SORT_INVALID;

	m_WheelState = STATE_SELECTING_MUSIC;
	m_fTimeLeftInState = 0;
	m_fPositionOffsetFromSelection = 0;

	m_iSwitchesLeftInSpinDown = 0;
	m_Moving = 0;

	if( GAMESTATE->IsExtraStage() ||  GAMESTATE->IsExtraStage2() )
	{
		// make the preferred group the group of the last song played.
		if( GAMESTATE->m_sPreferredGroup==GROUP_ALL_MUSIC && !PREFSMAN->m_bPickExtraStage )
		{
			ASSERT(GAMESTATE->m_pCurSong);
			GAMESTATE->m_sPreferredGroup = GAMESTATE->m_pCurSong->m_sGroupName;
		}

		Song* pSong;
		Steps* pNotes;
		PlayerOptions po;
		SongOptions so;
		SONGMAN->GetExtraStageInfo(
			GAMESTATE->IsExtraStage2(),
			GAMESTATE->GetCurrentStyleDef(),
			pSong,
			pNotes,
			po,
			so );
		GAMESTATE->m_pCurSong = pSong;
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( GAMESTATE->IsHumanPlayer(p) )
			{
				GAMESTATE->m_pCurNotes[p] = pNotes;
				GAMESTATE->m_PlayerOptions[p] = po;
				GAMESTATE->m_PreferredDifficulty[p] = pNotes->GetDifficulty();
			}
		}
		GAMESTATE->m_SongOptions = so;
	}

	if( GAMESTATE->m_SongSortOrder == SORT_INVALID )
	{
		switch( GAMESTATE->m_PlayMode )
		{
		case PLAY_MODE_ONI:		GAMESTATE->m_SongSortOrder = SORT_ONI_COURSES; break;
		case PLAY_MODE_NONSTOP:	GAMESTATE->m_SongSortOrder = SORT_NONSTOP_COURSES; break;
		case PLAY_MODE_ENDLESS:	GAMESTATE->m_SongSortOrder = SORT_ENDLESS_COURSES; break;
		default:				GAMESTATE->m_SongSortOrder = SortOrder[0]; break;
		}
	}

	/* Update for SORT_MOST_PLAYED. */
	SONGMAN->UpdateBest();

	/* Build all of the wheel item data.  Do tihs after selecting
	 * the extra stage, so it knows to always display it. */
	for( int so=0; so<NUM_SORT_ORDERS; so++ )
		BuildWheelItemDatas( m_WheelItemDatas[so], SongSortOrder(so) );

	// HACK: invalidate currently selected song in the case that it
	// cannot be played due to lack of stages remaining
	if( GAMESTATE->m_pCurSong != NULL && 
		SongManager::GetNumStagesForSong( GAMESTATE->m_pCurSong ) + GAMESTATE->m_iCurrentStageIndex > PREFSMAN->m_iNumArcadeStages
		&& !GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2() )
			GAMESTATE->m_pCurSong = NULL;

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
			LOG->Trace("MusicWheel::MusicWheel() - No selectable songs found in WheelData");
	}


	// Select the the previously selected song (if any)
	bool selected = SelectSong(GAMESTATE->m_pCurSong);
	// Select the the previously selected course (if any)
	if(!selected) selected = SelectCourse(GAMESTATE->m_pCurCourse);
	if(!selected) SetOpenGroup("");

	// rebuild the WheelItems that appear on screen
	RebuildMusicWheelItems();
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

bool MusicWheel::SelectSort( SongSortOrder so )
{
	if(so == SORT_INVALID)
		return false;

	unsigned i;
	vector<WheelItemData> &from = m_WheelItemDatas[GAMESTATE->m_SongSortOrder];
	for( i=0; i<from.size(); i++ )
	{
		if( from[i].m_SongSortOrder != so )
			continue;
		if( !from[i].m_Action.DescribesCurrentMode() )
			continue;

		// make its group the currently expanded group
		SetOpenGroup(from[i].m_sSectionName);
		break;
	}

	if(i == from.size())
		return false;

	for( i=0; i<m_CurWheelItemData.size(); i++ )
	{
		if( m_CurWheelItemData[i]->m_SongSortOrder != so )
			continue;
		if( !m_CurWheelItemData[i]->m_Action.DescribesCurrentMode() )
			continue;
		m_iSelection = i;		// select it
		break;
	}

	return true;
}

void MusicWheel::GetSongList(vector<Song*> &arraySongs, SongSortOrder so, CString sPreferredGroup )
{
	vector<Song*> apAllSongs;
//	if( so==SORT_PREFERRED && GAMESTATE->m_sPreferredGroup!=GROUP_ALL_MUSIC)
//		SONGMAN->GetSongs( apAllSongs, GAMESTATE->m_sPreferredGroup, GAMESTATE->GetNumStagesLeft() );	
//	else
//		SONGMAN->GetSongs( apAllSongs, GAMESTATE->GetNumStagesLeft() );
	SONGMAN->GetSongs( apAllSongs, GAMESTATE->m_sPreferredGroup, GAMESTATE->GetNumStagesLeft() );	

	// copy only songs that have at least one Steps for the current GameMode
	for( unsigned i=0; i<apAllSongs.size(); i++ )
	{
		Song* pSong = apAllSongs[i];

		/* If we're on an extra stage, and this song is selected, ignore
			* #SELECTABLE. */
		if( pSong != GAMESTATE->m_pCurSong || 
			(!GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2()) ) {
			/* Hide songs that asked to be hidden via #SELECTABLE. */
			if( so!=SORT_ROULETTE && !pSong->NormallyDisplayed() )
				continue;
			if( so==SORT_ROULETTE && !(pSong->RouletteDisplayed()
				|| GAMESTATE->m_pUnlockingSys->SongIsRoulette( pSong )) )
				continue;
		}

		// If we're using unlocks, check it here to prevent from being shown
		if( so!=SORT_ROULETTE && GAMESTATE->m_pUnlockingSys->SongIsLocked(pSong) )
			continue;

		vector<Steps*> arraySteps;
		pSong->GetNotes( arraySteps, GAMESTATE->GetCurrentStyleDef()->m_StepsType, DIFFICULTY_INVALID, -1, -1, "", PREFSMAN->m_bAutogenMissingTypes );

		if( !arraySteps.empty() )
			arraySongs.push_back( pSong );
	}
}

extern map<const Song*, CString> song_sort_val;
bool CompareSongPointersBySortVal(const Song *pSong1, const Song *pSong2);

void MusicWheel::SortSongPointerArrayBySectionName( vector<Song*> &arraySongPointers, SongSortOrder so )
{
	for(unsigned i = 0; i < arraySongPointers.size(); ++i)
	{
		CString val = MusicWheel::GetSectionNameFromSongAndSort( arraySongPointers[i], so );

		/* Make sure NUM comes first and OTHER comes last. */
		if( val == "NUM" )			val = "0";
		else if( val == "OTHER" )	val = "2";
		else						val = "1" + val;

		song_sort_val[arraySongPointers[i]] = val;
	}

	stable_sort( arraySongPointers.begin(), arraySongPointers.end(), CompareSongPointersBySortVal );
	song_sort_val.clear();
}


void MusicWheel::BuildWheelItemDatas( vector<WheelItemData> &arrayWheelItemDatas, SongSortOrder so )
{
	unsigned i;

	switch( so )
	{
	case SORT_MENU:
	{
		arrayWheelItemDatas.clear();	// clear out the previous wheel items 
		CString sNames = MENU_NAMES, sActions = MENU_ACTIONS;
		vector<CString> Names, Actions;
		split( sNames, ",", Names );
		split( sActions, ",", Actions );
		if( Names.size() != Actions.size() )
			RageException::Throw("MusicWheel::MenuNames and MusicWheel::MenuActions must have the same number of components");

		for( i=0; i<Names.size(); ++i )
		{
			/* Look for sort names. */
			vector<CString> parts;
			split( Actions[i], "-", parts );
		
			SongSortOrder so = SORT_INVALID;
			for( unsigned j = 0; so == SORT_INVALID && j < parts.size(); ++j )
				so = StringToSongSortOrder( parts[j] );

			if( so == SORT_INVALID )
				so = SORT_GROUP;

			RageColor c = RageColor(1,1,0,1);
			WheelItemData wid( TYPE_SORT, NULL, "", NULL, c, so );
			wid.m_sLabel = Names[i];
			wid.m_Action.FromString( Actions[i], true );

			switch( so )
			{
			case SORT_ALL_COURSES:
			case SORT_NONSTOP_COURSES:
			case SORT_ONI_COURSES:
			case SORT_ENDLESS_COURSES:
				/* Don't display course modes after the first stage. */
				if( !PREFSMAN->m_bEventMode && GAMESTATE->m_iCurrentStageIndex )
					continue;
			}

			arrayWheelItemDatas.push_back( wid );
		}		
		break;
	}
	case SORT_PREFERRED:
	case SORT_ROULETTE:
	case SORT_GROUP:
	case SORT_TITLE:
	case SORT_BPM:
	case SORT_MOST_PLAYED:
	case SORT_GRADE:
	case SORT_ARTIST:
	case SORT_EASY_METER:
	case SORT_MEDIUM_METER:
	case SORT_HARD_METER:
	{
		///////////////////////////////////
		// Make an array of Song*, then sort them
		///////////////////////////////////
		vector<Song*> arraySongs;
		
		GetSongList(arraySongs, so, GAMESTATE->m_sPreferredGroup );

		bool bUseSections = true;

		// sort the songs
		switch( so )
		{
		case SORT_PREFERRED:
		case SORT_ROULETTE:
			SortSongPointerArrayByGroupAndDifficulty( arraySongs );
			bUseSections = false;
			break;
		case SORT_GROUP:
			SortSongPointerArrayByGroupAndTitle( arraySongs );
			bUseSections = GAMESTATE->m_sPreferredGroup == GROUP_ALL_MUSIC;
			break;
		case SORT_TITLE:
			SortSongPointerArrayByTitle( arraySongs );
			break;
		case SORT_BPM:
			SortSongPointerArrayByBPM( arraySongs );
			break;
		case SORT_MOST_PLAYED:
			arraySongs = SONGMAN->GetBestSongs();
			if( arraySongs.size() > 30 )
				arraySongs.erase(arraySongs.begin()+30, arraySongs.end());
			bUseSections = false;
			break;
		case SORT_GRADE:
			SortSongPointerArrayByGrade( arraySongs );
			break;
		case SORT_ARTIST:
			SortSongPointerArrayByArtist( arraySongs );
			break;
		case SORT_EASY_METER:
			SortSongPointerArrayByMeter( arraySongs, DIFFICULTY_EASY );
			break;
		case SORT_MEDIUM_METER:
			SortSongPointerArrayByMeter( arraySongs, DIFFICULTY_MEDIUM );
			break;
		case SORT_HARD_METER:
			SortSongPointerArrayByMeter( arraySongs, DIFFICULTY_HARD );
			break;
		default:
			ASSERT(0);	// unhandled SortOrder
		}


		///////////////////////////////////
		// Build an array of WheelItemDatas from the sorted list of Song*'s
		///////////////////////////////////
		arrayWheelItemDatas.clear();	// clear out the previous wheel items 

		if( PREFSMAN->m_MusicWheelUsesSections == PrefsManager::NEVER || (so != SORT_TITLE && PREFSMAN->m_MusicWheelUsesSections == PrefsManager::ABC_ONLY ))
			bUseSections = false;

		if( bUseSections )
		{
			// Sorting twice isn't necessary.  Instead, modify the compatator functions 
			// in Song.cpp to have the desired effect. -Chris
			/* Keeping groups together with the sorts is tricky and brittle; we
				* keep getting OTHER split up without this.  However, it puts the 
				* Grade and BPM sorts in the wrong order, and they're already correct,
				* so don't re-sort for them. */
//				/* We're using sections, so use the section name as the top-level
//				 * sort. */
			if( so != SORT_GRADE && so != SORT_BPM )
				SortSongPointerArrayBySectionName(arraySongs, so);

			// make WheelItemDatas with sections
			CString sLastSection = "";
			RageColor colorSection;
			for( unsigned i=0; i< arraySongs.size(); i++ )
			{
				Song* pSong = arraySongs[i];
				CString sThisSection = GetSectionNameFromSongAndSort( pSong, so );
				int iSectionColorIndex = 0;

				if( sThisSection != sLastSection)	// new section, make a section item
				{
					colorSection = (so==SORT_GROUP) ? SONGMAN->GetGroupColor(pSong->m_sGroupName) : SECTION_COLORS(iSectionColorIndex);
					iSectionColorIndex = (iSectionColorIndex+1) % NUM_SECTION_COLORS;
					arrayWheelItemDatas.push_back( WheelItemData(TYPE_SECTION, NULL, sThisSection, NULL, colorSection, SORT_INVALID) );
					sLastSection = sThisSection;
				}

				arrayWheelItemDatas.push_back( WheelItemData( TYPE_SONG, pSong, sThisSection, NULL, SONGMAN->GetSongColor(pSong), SORT_INVALID) );
			}
		}
		else
		{
			for( unsigned i=0; i<arraySongs.size(); i++ )
			{
				Song* pSong = arraySongs[i];
				arrayWheelItemDatas.push_back( WheelItemData(TYPE_SONG, pSong, "", NULL, SONGMAN->GetSongColor(pSong), SORT_INVALID) );
			}
		}

		if( so != SORT_ROULETTE && SHOW_ROULETTE )
			arrayWheelItemDatas.push_back( WheelItemData(TYPE_ROULETTE, NULL, "", NULL, RageColor(1,0,0,1), SORT_INVALID) );
		if( so != SORT_ROULETTE && SHOW_RANDOM )
			arrayWheelItemDatas.push_back( WheelItemData(TYPE_RANDOM, NULL, "", NULL, RageColor(1,0,0,1), SORT_INVALID) );

		// HACK:  Add extra stage item if it isn't already present on the music wheel
		if( GAMESTATE->IsExtraStage() || GAMESTATE->IsExtraStage2() )
		{
			Song* pSong;
			Steps* pNotes;
			PlayerOptions po;
			SongOptions so;
			SONGMAN->GetExtraStageInfo( GAMESTATE->IsExtraStage2(), GAMESTATE->GetCurrentStyleDef(), pSong, pNotes, po, so );
			
			bool bFoundExtraSong = false;

			for( unsigned i=0; i<arrayWheelItemDatas.size(); i++ )
			{
				if( arrayWheelItemDatas[i].m_pSong == pSong )
				{
					/* Change the song color. */
					arrayWheelItemDatas[i].m_color = SONG_REAL_EXTRA_COLOR;
					bFoundExtraSong = true;
					break;
				}
			}

			if( !bFoundExtraSong )
				arrayWheelItemDatas.push_back( WheelItemData(TYPE_SONG, pSong, "", NULL, SONG_REAL_EXTRA_COLOR, SORT_INVALID) );
		}
		break;
	}
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
	{
		vector<Course*> apCourses;
		switch( so )
		{
		case SORT_NONSTOP_COURSES:	SONGMAN->GetNonstopCourses( apCourses, PREFSMAN->m_bAutogenGroupCourses );	break;
		case SORT_ONI_COURSES:		SONGMAN->GetOniCourses( apCourses, PREFSMAN->m_bAutogenGroupCourses );		break;
		case SORT_ENDLESS_COURSES:	SONGMAN->GetEndlessCourses( apCourses, PREFSMAN->m_bAutogenGroupCourses );	break;
		case SORT_ALL_COURSES:		SONGMAN->GetAllCourses( apCourses, PREFSMAN->m_bAutogenGroupCourses );	break;
		default: ASSERT(0); break;
		}

		// default sort order; other songs may alter this
		SortCoursePointerArrayByDifficulty( apCourses );

		if (PREFSMAN->m_iCourseSortOrder != PrefsManager::COURSE_SORT_SONGS)
		{
			if (PREFSMAN->m_iCourseSortOrder == PrefsManager::COURSE_SORT_METER)
				SortCoursePointerArrayByAvgDifficulty( apCourses );

			if (PREFSMAN->m_iCourseSortOrder == PrefsManager::COURSE_SORT_METER_SUM)
				SortCoursePointerArrayByTotalDifficulty( apCourses );

			if (PREFSMAN->m_iCourseSortOrder == PrefsManager::COURSE_SORT_RANK)
				SortCoursePointerArrayByRanking( apCourses );
		}

		if( so == SORT_ALL_COURSES )
			SortCoursePointerArrayByType( apCourses );

		arrayWheelItemDatas.clear();	// clear out the previous wheel items 

		CString sLastSection = "";
		int iSectionColorIndex = 0;
		for( unsigned c=0; c<apCourses.size(); c++ )	// foreach course
		{
			Course* pCourse = apCourses[c];

			// if unlocks are on, make sure it is unlocked
			if ( GAMESTATE->m_pUnlockingSys->CourseIsLocked(pCourse) )
				continue;

			CString sThisSection = "";
			if( so == SORT_ALL_COURSES )
			switch( pCourse->GetPlayMode() )
			{
			case PLAY_MODE_ONI:		sThisSection = "Oni";		break;
			case PLAY_MODE_NONSTOP:	sThisSection = "Nonstop";	break;
			case PLAY_MODE_ENDLESS:	sThisSection = "Endless";	break;
			}

			// check that this course has at least one song playable in the current style
			if( !pCourse->IsPlayableIn(GAMESTATE->GetCurrentStyleDef()->m_StepsType) )
				continue;

			if( sThisSection != sLastSection )	// new section, make a section item
			{
				RageColor c = SECTION_COLORS(iSectionColorIndex);
				iSectionColorIndex = (iSectionColorIndex+1) % NUM_SECTION_COLORS;
				arrayWheelItemDatas.push_back( WheelItemData(TYPE_SECTION, NULL, sThisSection, NULL, c, SORT_INVALID) );
				sLastSection = sThisSection;
			}

            arrayWheelItemDatas.push_back( WheelItemData(TYPE_COURSE, NULL, sThisSection, pCourse, pCourse->GetColor(), SORT_INVALID) );
		}
		break;
	}
	}

	// init music status icons
	for( i=0; i<arrayWheelItemDatas.size(); i++ )
	{
		Song* pSong = arrayWheelItemDatas[i].m_pSong;
		if( pSong == NULL )
			continue;

		WheelItemData& WID = arrayWheelItemDatas[i];
		WID.m_Flags.bHasBeginnerOr1Meter = pSong->IsEasy( STEPS_TYPE_DANCE_SINGLE );
		WID.m_Flags.bEdits = pSong->HasEdits( STEPS_TYPE_DANCE_SINGLE );
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
		arrayWheelItemDatas.push_back( WheelItemData(TYPE_SECTION, NULL, "- EMPTY -", NULL, RageColor(1,0,0,1), SORT_INVALID) );
	}
}

void MusicWheel::GetItemPosition( float fPosOffsetsFromMiddle, float& fX_out, float& fY_out, float& fZ_out, float& fRotationX_out )
{
	if( USE_3D )
	{
		fRotationX_out = SCALE(fPosOffsetsFromMiddle,-7,+7,-PI/2.f,+PI/2.f);

//		printf( "fRotationX_out = %f\n", fRotationX_out );

		float radius = 200;

		fX_out = 0;
		fY_out = radius*sinf(fRotationX_out);
		fZ_out = -100+radius*cosf(fRotationX_out);

		fRotationX_out *= 180.f/PI;	// to degrees
	}
	else
	{
		fX_out = (1-cosf(fPosOffsetsFromMiddle/PI))*ITEM_CURVE_X;
		fY_out = fPosOffsetsFromMiddle*ITEM_SPACING_Y;
		fZ_out = 0;
		fRotationX_out = 0;

		fX_out = roundf( fX_out );
		fY_out = roundf( fY_out );
		fZ_out = roundf( fZ_out );
	}
}

void MusicWheel::RebuildMusicWheelItems()
{
	// rewind to first index that will be displayed;
	int iIndex = m_iSelection;
	if( m_iSelection > int(m_CurWheelItemData.size()-1) )
		m_iSelection = 0;
	
	iIndex -= NUM_WHEEL_ITEMS/2;

	ASSERT(m_CurWheelItemData.size());
	while(iIndex < 0)
		iIndex += m_CurWheelItemData.size();

	// iIndex is now the index of the lowest WheelItem to draw
	for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
	{
		WheelItemData     *data   = m_CurWheelItemData[iIndex];
		MusicWheelItem& display = m_MusicWheelItems[i];

		display.LoadFromWheelItemData( data );

		// increment iIndex
		iIndex++;
		if( iIndex > int(m_CurWheelItemData.size()-1) )
			iIndex = 0;
	}

}

void MusicWheel::NotesChanged( PlayerNumber pn )	// update grade graphics and top score
{
	for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
	{
		MusicWheelItem& display = m_MusicWheelItems[i];
		display.RefreshGrades();
	}
}



void MusicWheel::DrawPrimitives()
{
	if( USE_3D )
	{
//		DISPLAY->PushMatrix();
//		DISPLAY->EnterPerspective(45, false);

		// construct view and project matrix
//		RageVector3 Up( 0.0f, 1.0f, 0.0f );
//		RageVector3 Eye( CENTER_X, CENTER_Y, 550 );
//		RageVector3 At( CENTER_X, CENTER_Y, 0 );

//		DISPLAY->LookAt(Eye, At, Up);
	}

	// draw outside->inside
	int i;
	for( i=0; i<NUM_WHEEL_ITEMS/2; i++ )
		DrawItem( i );
	for( i=NUM_WHEEL_ITEMS-1; i>=NUM_WHEEL_ITEMS/2; i-- )
		DrawItem( i );


	ActorFrame::DrawPrimitives();
	
	if( USE_3D )
	{
//		DISPLAY->ExitPerspective();
//		DISPLAY->PopMatrix();
	}
}


void MusicWheel::DrawItem( int i )
{
	MusicWheelItem& display = m_MusicWheelItems[i];

	switch( m_WheelState )
	{
	case STATE_SELECTING_MUSIC:
	case STATE_ROULETTE_SPINNING:
	case STATE_ROULETTE_SLOWING_DOWN:
	case STATE_RANDOM_SPINNING:
	case STATE_LOCKED:
		{
			float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS/2 + m_fPositionOffsetFromSelection;

			float fX, fY, fZ, fRotationX;
			GetItemPosition( fThisBannerPositionOffsetFromSelection, fX, fY, fZ, fRotationX );

//				if( fY < -SCREEN_HEIGHT/2  ||  fY > SCREEN_HEIGHT/2 )
//					continue; // skip

			display.SetXY( fX, fY );
			display.SetZ( fZ );
			display.SetRotationX( fRotationX );
		}
		break;
	}

	if( m_WheelState == STATE_LOCKED  &&  i != NUM_WHEEL_ITEMS/2 )
		display.m_fPercentGray = 0.5f;
	else
		display.m_fPercentGray = 0;

	display.Draw();
}


void MusicWheel::UpdateScrollbar()
{
	int total_num_items = m_CurWheelItemData.size();
	float item_at=m_iSelection - m_fPositionOffsetFromSelection;

	if(NUM_WHEEL_ITEMS >= total_num_items) {
		m_ScrollBar.SetPercentage( 0, 1 );
	} else {
		float size = float(NUM_WHEEL_ITEMS) / total_num_items;
		float center = item_at / total_num_items;
		size *= 0.5f;

		m_ScrollBar.SetPercentage( center - size, center + size );
	}
}

void MusicWheel::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	unsigned i;
	for( i=0; i<unsigned(NUM_WHEEL_ITEMS); i++ )
	{
		MusicWheelItem& display = m_MusicWheelItems[i];

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

				SCREENMAN->PostMessageToTopScreen( SM_SortOrderChanged, 0 );
				
				SetOpenGroup(GetSectionNameFromSongAndSort( pPrevSelectedSong, GAMESTATE->m_SongSortOrder ));

				m_iSelection = 0;

				switch( GAMESTATE->m_SongSortOrder )
				{
				case SORT_PREFERRED:
				case SORT_GROUP:
				case SORT_TITLE:
				case SORT_BPM:
				case SORT_GRADE:
				case SORT_ARTIST:
				case SORT_MOST_PLAYED:
				case SORT_ROULETTE:
				case SORT_EASY_METER:
				case SORT_MEDIUM_METER:
				case SORT_HARD_METER:
				case SORT_ALL_COURSES:
				case SORT_NONSTOP_COURSES:
				case SORT_ONI_COURSES:
				case SORT_ENDLESS_COURSES:
					// Look for the last selected song or course
					if( GAMESTATE->m_pCurCourse )
						SelectCourse( GAMESTATE->m_pCurCourse );
					if( GAMESTATE->m_pCurSong )
						SelectSong( GAMESTATE->m_pCurSong );
					break;
				case SORT_MENU:
					SelectSort( m_LastSongSortOrder );
					break;
				default:
					ASSERT(0);
				}

				// Change difficulty for sorts by meter
				switch( GAMESTATE->m_SongSortOrder )
				{
				case SORT_EASY_METER:
					{
						for( int p=0; p<NUM_PLAYERS; p++ )
							if( GAMESTATE->IsPlayerEnabled(p) )
								GAMESTATE->m_PreferredDifficulty[p] = DIFFICULTY_EASY;
					}
					break;
				case SORT_MEDIUM_METER:
					{
						for( int p=0; p<NUM_PLAYERS; p++ )
							if( GAMESTATE->IsPlayerEnabled(p) )
								GAMESTATE->m_PreferredDifficulty[p] = DIFFICULTY_MEDIUM;
					}
					break;
				case SORT_HARD_METER:
					{
						for( int p=0; p<NUM_PLAYERS; p++ )
							if( GAMESTATE->IsPlayerEnabled(p) )
								GAMESTATE->m_PreferredDifficulty[p] = DIFFICULTY_HARD;
					}
					break;
				}

				SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );
				RebuildMusicWheelItems();
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
				m_WheelState = STATE_LOCKED;
				m_soundStart.Play();
				m_fLockedWheelVelocity = 0;
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
				SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );
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

	RebuildMusicWheelItems();

	m_fPositionOffsetFromSelection += dist;

	SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );

	/* If we're moving automatically, don't play this; it'll be called in Update. */
	if(!IsMoving())
		m_soundChangeMusic.Play();
}

bool MusicWheel::ChangeSort( SongSortOrder new_so )	// return true if change successful
{
	ASSERT( new_so < NUM_SORT_ORDERS );
	if( GAMESTATE->m_SongSortOrder == new_so )
		return false;

	switch( m_WheelState )
	{
	case STATE_SELECTING_MUSIC:
	case STATE_FLYING_ON_AFTER_NEXT_SORT:
		break;	// fall through
	default:
		return false;	// don't continue
	}

	SCREENMAN->PostMessageToTopScreen( SM_SortOrderChanging, 0 );

	m_soundChangeSort.Play();

	TweenOffScreen(true);

	m_LastSongSortOrder = GAMESTATE->m_SongSortOrder;
	GAMESTATE->m_SongSortOrder = new_so;

	m_WheelState = STATE_FLYING_OFF_BEFORE_NEXT_SORT;
	return true;
}

bool MusicWheel::NextSort()		// return true if change successful
{
	/* Is the current sort in the default sort order? */
	int cur = 0;
	while( SortOrder[cur] != SORT_INVALID && SortOrder[cur] != GAMESTATE->m_SongSortOrder )
		++cur;

	SongSortOrder so;
	if( SortOrder[cur] == SORT_INVALID )
	{
		/* It isn't, which means we're either in the sort menu or in a sort selected
		 * from the sort menu.  If we're in the sort menu, return to the first sort. 
		 * Otherwise, return to the sort menu. */
		if( GAMESTATE->m_SongSortOrder == SORT_MENU )
			so = SortOrder[0];
		else
			so = SORT_MENU;
	} else {
		++cur;
		if( SortOrder[cur] == SORT_INVALID )
			cur = 0;
		so = SortOrder[cur];
	}

	return ChangeSort( so );
}

bool MusicWheel::Select()	// return true if this selection ends the screen
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
		SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );
		return false;
	}

	switch( m_CurWheelItemData[m_iSelection]->m_Type )
	{
	case TYPE_SECTION:
		{
			CString sThisItemSectionName = m_CurWheelItemData[m_iSelection]->m_sSectionName;
			if( m_sExpandedSectionName == sThisItemSectionName )	// already expanded
				m_sExpandedSectionName = "";		// collapse it
			else				// already collapsed
				m_sExpandedSectionName = sThisItemSectionName;	// expand it

			m_soundExpand.Play();

			SetOpenGroup(m_sExpandedSectionName);
		}
		return false;
	case TYPE_ROULETTE:  
		StartRoulette();
		return false;
	case TYPE_RANDOM:
		StartRandom();
		return false;
	case TYPE_SONG:
		if (PREFSMAN->m_bUseUnlockSystem)
			GAMESTATE->m_pUnlockingSys->RouletteUnlock( m_CurWheelItemData[m_iSelection]->m_pSong );
		// fall-through - we want to check for unlocking only if its a song
	case TYPE_COURSE:
		return true;
	case TYPE_SORT:
		m_CurWheelItemData[m_iSelection]->m_Action.ApplyToAllPlayers();
		ChangeSort( m_CurWheelItemData[m_iSelection]->m_SongSortOrder );
		return false;
	default:
		ASSERT(0);
		return false;
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
	m_SpinSpeed *= 20.0f; /* faster! */
	m_WheelState = STATE_RANDOM_SPINNING;

	this->Select();
	RebuildMusicWheelItems();
}

void MusicWheel::SetOpenGroup(CString group, SongSortOrder so)
{
	if( so == SORT_INVALID)
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
		if((from[i].m_Type == TYPE_SONG ||
			from[i].m_Type == TYPE_COURSE) &&
		     !from[i].m_sSectionName.empty() &&
			 from[i].m_sSectionName != group)
			 continue;
		m_CurWheelItemData.push_back(&from[i]);
	}


	//
	// Try to select the item that was selected before chaning groups
	//
	m_iSelection = 0;

	for( i=0; i<m_CurWheelItemData.size(); i++ )
	{
		if( m_CurWheelItemData[i] == old )
		{
			m_iSelection=i;
			break;
		}
	}

	RebuildMusicWheelItems();
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

	float fX, fY, fZ, fRotationX;
	GetItemPosition( 0, fX, fY, fZ, fRotationX );
	
	m_sprSelectionOverlay.SetXY( fX+320, fY );
	m_sprSelectionOverlay.SetZ( fZ );
	m_sprSelectionOverlay.SetRotationX( fRotationX );

	if(changing_sort) {
		m_sprSelectionOverlay.BeginTweening( 0.04f * NUM_WHEEL_ITEMS/2 * factor );	// sleep
		m_sprSelectionOverlay.BeginTweening( 0.2f * factor, Actor::TWEEN_ACCELERATE );
	} else {
		m_sprSelectionOverlay.BeginTweening( 0.05f * factor );	// sleep
		m_sprSelectionOverlay.BeginTweening( 0.4f * factor, Actor::TWEEN_ACCELERATE );
	}
	m_sprSelectionOverlay.SetX( fX );

	m_ScrollBar.SetX( SCROLL_BAR_X+30 );
	if(changing_sort)
		m_ScrollBar.BeginTweening( 0.2f * factor );	// sleep
	else
		m_ScrollBar.BeginTweening( 0.7f * factor );	// sleep
	m_ScrollBar.BeginTweening( 0.2f * factor , Actor::TWEEN_ACCELERATE );
	m_ScrollBar.SetX( SCROLL_BAR_X );	

	for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
	{
		MusicWheelItem& display = m_MusicWheelItems[i];
		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS/2 + m_fPositionOffsetFromSelection;

		float fX, fY, fZ, fRotationX;
		GetItemPosition( fThisBannerPositionOffsetFromSelection, fX, fY, fZ, fRotationX );
		display.SetXY( fX+320, fY );
		display.SetZ( fZ );
		display.SetRotationX( fRotationX );
		display.BeginTweening( 0.04f*i * factor );	// sleep
		display.BeginTweening( 0.2f * factor, Actor::TWEEN_ACCELERATE );
		display.SetX( fX );
	}

	m_fTimeLeftInState = GetTweenTimeLeft() + 0.100f;
}
						   
void MusicWheel::TweenOffScreen(bool changing_sort)
{
	float factor = 1.0f;
	if(changing_sort) factor = 0.25;

	m_WheelState = STATE_TWEENING_OFF_SCREEN;

	float fX, fY, fZ, fRotationX;
	GetItemPosition( 0, fX, fY, fZ, fRotationX );
	m_sprSelectionOverlay.SetXY( fX, fY );
	m_sprSelectionOverlay.SetZ( fZ );
	m_sprSelectionOverlay.SetRotationX( fRotationX );

	if(changing_sort) {
		/* When changing sort, tween the overlay with the item in the center;
		 * having it separate looks messy when we're moving fast. */
		m_sprSelectionOverlay.BeginTweening( 0.04f * NUM_WHEEL_ITEMS/2 * factor );	// sleep
		m_sprSelectionOverlay.BeginTweening( 0.2f * factor, Actor::TWEEN_DECELERATE );
	} else {
		m_sprSelectionOverlay.BeginTweening( 0 );	// sleep
		m_sprSelectionOverlay.BeginTweening( 0.2f * factor, Actor::TWEEN_DECELERATE );
	}
	m_sprSelectionOverlay.SetX( fX+320 );

	m_ScrollBar.BeginTweening( 0 );
	m_ScrollBar.BeginTweening( 0.2f * factor, Actor::TWEEN_ACCELERATE );
	m_ScrollBar.SetX( SCROLL_BAR_X+30 );	

	for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
	{
		MusicWheelItem& display = m_MusicWheelItems[i];
		float fThisBannerPositionOffsetFromSelection = i - NUM_WHEEL_ITEMS/2 + m_fPositionOffsetFromSelection;

		float fX, fY, fZ, fRotationX;
		GetItemPosition( fThisBannerPositionOffsetFromSelection, fX, fY, fZ, fRotationX );
		display.SetXY( fX, fY );
		display.SetZ( fZ );
		display.SetRotationX( fRotationX );
		display.BeginTweening( 0.04f*i * factor );	// sleep
		display.BeginTweening( 0.2f * factor, Actor::TWEEN_DECELERATE );
		display.SetX( fX+320 );
	}

	m_fTimeLeftInState = GetTweenTimeLeft() + 0.100f;
}

CString MusicWheel::GetSectionNameFromSongAndSort( const Song* pSong, SongSortOrder so )
{
	if( pSong == NULL )
		return "";

	switch( so )
	{
	case SORT_PREFERRED:
		return "";
	case SORT_GROUP:	
		return pSong->m_sGroupName;
	case SORT_TITLE:
	case SORT_ARTIST:	
		{
			CString s;
			switch( so )
			{
			case SORT_TITLE:	s = pSong->GetTranslitMainTitle();	break;
			case SORT_ARTIST:	s = pSong->GetTranslitArtist();		break;
			default:	ASSERT(0);
			}
			s = MakeSortString(s);	// resulting string will be uppercase
			
			if( s.empty() )
				return "";
			else if( s[0] >= '0' && s[0] <= '9' )
				return "NUM";
			else if( s[0] < 'A' || s[0] > 'Z')
				return "OTHER";
			else
				return s.Left(1);
		}
	case SORT_BPM:
		{
			const int iBPMGroupSize = 20;
			float fMinBPM, fMaxBPM;
			pSong->GetDisplayBPM( fMinBPM, fMaxBPM );
			int iMaxBPM = (int)fMaxBPM;
			iMaxBPM += iBPMGroupSize - (iMaxBPM%iBPMGroupSize) - 1;
			return ssprintf("%03d-%03d",iMaxBPM-(iBPMGroupSize-1), iMaxBPM);
		}
	case SORT_MOST_PLAYED:
		return "";
	case SORT_GRADE:
		{
			for( int i=NUM_GRADES; i>GRADE_NO_DATA; i-- )
			{
				Grade g = (Grade)i;
				int iCount = pSong->GetNumNotesWithGrade( g );
				if( iCount > 0 )
					return ssprintf( "%4s x %d", GradeToString(g).c_str(), iCount );
			}
			return "NO DATA";
		}
	case SORT_EASY_METER:
		{
			Steps* pNotes = pSong->GetNotes(GAMESTATE->GetCurrentStyleDef()->m_StepsType,DIFFICULTY_EASY);
			if( pNotes )	
				return ssprintf("%02d", pNotes->GetMeter() );
			return "N/A";
		}
	case SORT_MEDIUM_METER:
		{
			Steps* pNotes = pSong->GetNotes(GAMESTATE->GetCurrentStyleDef()->m_StepsType,DIFFICULTY_MEDIUM);
			if( pNotes )	
				return ssprintf("%02d", pNotes->GetMeter() );
			return "N/A";
		}
	case SORT_HARD_METER:
		{
			Steps* pNotes = pSong->GetNotes(GAMESTATE->GetCurrentStyleDef()->m_StepsType,DIFFICULTY_HARD);
			if( pNotes )	
				return ssprintf("%02d", pNotes->GetMeter() );
			return "N/A";
		}
	case SORT_MENU:
		return "";
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
	default:
		ASSERT(0);
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
			int iSign = n/abs(n);
			m_fLockedWheelVelocity = iSign*LOCKED_INITIAL_VELOCITY;
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
		SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );
	}

	m_TimeBeforeMovingBegins = TIME_BEFORE_SLOW_REPEATS;
	m_SpinSpeed = float(PREFSMAN->m_iMusicWheelSwitchSpeed);
	m_Moving = n;

	if( fabsf(m_fPositionOffsetFromSelection) > 0.5f )	// wheel is very busy spinning
		return;
	
	if(m_Moving)
		ChangeMusic(m_Moving);
}
