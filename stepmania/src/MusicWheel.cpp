#include "global.h"
#include "MusicWheel.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"	// for sending SM_PlayMusicSample
#include "RageLog.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "song.h"
#include "Course.h"
#include "Steps.h"
#include "UnlockManager.h"
#include "GameCommand.h"
#include "ActorUtil.h"
#include "SongUtil.h"
#include "CourseUtil.h"
#include "Foreach.h"
#include "Style.h"
#include "ThemeMetric.h"
#include "PlayerState.h"

static Preference<bool> g_bMoveRandomToEnd( "MoveRandomToEnd", false );

#define NUM_WHEEL_ITEMS		((int)ceil(NUM_WHEEL_ITEMS_TO_DRAW+2))
#define WHEEL_TEXT(s)             THEME->GetString( "MusicWheel", ssprintf("%sText",s.c_str()) );

static RString SECTION_COLORS_NAME( size_t i )	{ return ssprintf("SectionColor%d",int(i+1)); }
static RString CHOICE_NAME( RString s )			{ return ssprintf("Choice%s",s.c_str()); }

AutoScreenMessage( SM_SongChanged )          // TODO: Replace this with a Message and MESSAGEMAN
AutoScreenMessage( SM_SortOrderChanging );
AutoScreenMessage( SM_SortOrderChanged );

static const SortOrder g_SongSortOrders[] =
{
	SORT_GROUP, 
	SORT_TITLE, 
	SORT_BPM, 
	SORT_POPULARITY, 
	SORT_ARTIST,
	SORT_GENRE,
};
const vector<SortOrder> SONG_SORT_ORDERS( g_SongSortOrders, g_SongSortOrders + ARRAYLEN(g_SongSortOrders) );
	
static SortOrder ForceAppropriateSort( PlayMode pm, SortOrder so )
{
	switch( pm )
	{
	// in course modes, force a particular sort
	case PLAY_MODE_ONI:		return SORT_ONI_COURSES;
	case PLAY_MODE_NONSTOP:	return SORT_NONSTOP_COURSES;
	case PLAY_MODE_ENDLESS:	return SORT_ENDLESS_COURSES;
	}

	/* If we're not in a course mode, don't start in a course sort. */
	switch( so )
	{
	case SORT_ONI_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ENDLESS_COURSES:
		so = SortOrder_INVALID;
		break;
	}

	return so;
}

WheelItemBase *MusicWheel::MakeItem()
{
	return new MusicWheelItem;
}

void MusicWheel::Load( RString sType ) 
{
	ROULETTE_SWITCH_SECONDS		.Load(sType,"RouletteSwitchSeconds");
	ROULETTE_SLOW_DOWN_SWITCHES	.Load(sType,"RouletteSlowDownSwitches");
	NUM_SECTION_COLORS		.Load(sType,"NumSectionColors");
	SONG_REAL_EXTRA_COLOR		.Load(sType,"SongRealExtraColor");
	SORT_MENU_COLOR			.Load(sType,"SortMenuColor");
	SHOW_ROULETTE			.Load(sType,"ShowRoulette");
	SHOW_RANDOM			.Load(sType,"ShowRandom");
	SHOW_PORTAL			.Load(sType,"ShowPortal");
	RANDOM_PICKS_LOCKED_SONGS	.Load(sType,"RandomPicksLockedSongs");
	MOST_PLAYED_SONGS_TO_SHOW	.Load(sType,"MostPlayedSongsToShow");
	MODE_MENU_CHOICE_NAMES		.Load(sType,"ModeMenuChoiceNames");
	vector<RString> vsModeChoiceNames;
	split( MODE_MENU_CHOICE_NAMES, ",", vsModeChoiceNames );
	CHOICE				.Load(sType,CHOICE_NAME,vsModeChoiceNames);
	SECTION_COLORS			.Load(sType,SECTION_COLORS_NAME,NUM_SECTION_COLORS);

	WheelBase::Load( sType );

	SONGMAN->UpdateRankingCourses();

	m_soundChangeSort.Load(	THEME->GetPathS(sType,"sort") );
	m_soundExpand.Load(	THEME->GetPathS(sType,"expand"), true );

	/* Update for SORT_MOST_PLAYED. */
	SONGMAN->UpdatePopular();

	/* Sort SONGMAN's songs by CompareSongPointersByTitle, so we can do other sorts (with
	 * stable_sort) from its output, and title will be the secondary sort, without having
	 * to re-sort by title each time. */
	SONGMAN->SortSongs();

	RageTimer timer;
	RString times;
	/* Build all of the wheel item data.  Do this after selecting
	 * the extra stage, so it knows to always display it. */
	FOREACH_SortOrder( so )
	{
		BuildWheelItemDatas( m_WheelItemDatas[so], so );
		times += ssprintf( "%i:%.3f ", so, timer.GetDeltaTime() );
	}
	LOG->Trace( "took: %s", times.c_str() );

	/* Set m_LastModeMenuItem to the first item that matches the current mode.  (Do this
	 * after building wheel item data.) */
	{
		const vector<WheelItemData *> &from = m_WheelItemDatas[SORT_MODE_MENU];
		for( unsigned i=0; i<from.size(); i++ )
		{
			ASSERT( &*from[i]->m_pAction );
			if( from[i]->m_pAction->DescribesCurrentModeForAllPlayers() )
			{
				m_sLastModeMenuItem = from[i]->m_pAction->m_sName;
				break;
			}
		}
	}
}

void MusicWheel::BeginScreen()
{
	WheelBase::BeginScreen();

	if( (GAMESTATE->IsExtraStage() && !PREFSMAN->m_bPickExtraStage) || GAMESTATE->IsExtraStage2() )
	{
		m_WheelState = STATE_LOCKED;
		SCREENMAN->PlayStartSound();
		m_fLockedWheelVelocity = 0;
	}

	GAMESTATE->m_SortOrder.Set( GAMESTATE->m_PreferredSortOrder );

	/* Never start in the mode menu; some elements may not initialize correctly. */
	if( GAMESTATE->m_SortOrder == SORT_MODE_MENU )
		GAMESTATE->m_SortOrder.Set( SortOrder_INVALID );

	GAMESTATE->m_SortOrder.Set( ForceAppropriateSort(GAMESTATE->m_PlayMode, GAMESTATE->m_SortOrder) );

	/* Only save the sort order if the player didn't already have one.  If he did, don't
	 * overwrite it. */
	if( GAMESTATE->m_PreferredSortOrder == SortOrder_INVALID )
		GAMESTATE->m_PreferredSortOrder = GAMESTATE->m_SortOrder;

	// HACK: invalidate currently selected song in the case that it
	// cannot be played due to lack of stages remaining
	// checking for event mode shouldn't be necessary here
	// but someone mentioned it does it sometimes.
	if( GAMESTATE->m_pCurSong != NULL && 
		SongManager::GetNumStagesForSong( GAMESTATE->m_pCurSong ) + GAMESTATE->m_iCurrentStageIndex > PREFSMAN->m_iSongsPerPlay
		&& !GAMESTATE->IsEventMode()
		&& !GAMESTATE->IsAnExtraStage() )
	{
		GAMESTATE->m_pCurSong.Set( NULL );
	}

	// Select the the previously selected song (if any)
	if( !SelectSongOrCourse() )
		SetOpenGroup("");

	// rebuild the WheelItems that appear on screen
	RebuildWheelItems();
}

MusicWheel::~MusicWheel()
{
	FOREACH_SortOrder( so )
		FOREACH( WheelItemData*, m_WheelItemDatas[so], i )
			delete *i;
}

/* If a song or course is set in GAMESTATE and available, select it.  Otherwise, choose the
 * first available song or course.  Return true if an item was set, false if no items are
 * available. */
bool MusicWheel::SelectSongOrCourse()
{
	if( GAMESTATE->m_pPreferredSong && SelectSong( GAMESTATE->m_pPreferredSong ) )
		return true;
	if( GAMESTATE->m_pCurSong && SelectSong( GAMESTATE->m_pCurSong ) )
		return true;
	if( GAMESTATE->m_pPreferredCourse && SelectCourse( GAMESTATE->m_pPreferredCourse ) )
		return true;
	if( GAMESTATE->m_pCurCourse && SelectCourse( GAMESTATE->m_pCurCourse ) )
		return true;

	// Select the first selectable song based on the sort order...
	vector<WheelItemData *> &wiWheelItems = m_WheelItemDatas[GAMESTATE->m_SortOrder];
	for( unsigned i = 0; i < wiWheelItems.size(); i++ )
	{
		if( wiWheelItems[i]->m_pSong )
			return SelectSong( wiWheelItems[i]->m_pSong );
		else if ( wiWheelItems[i]->m_pCourse )
			return SelectCourse( wiWheelItems[i]->m_pCourse );
	}

	LOG->Trace( "MusicWheel::MusicWheel() - No selectable songs or courses found in WheelData" );
	return false;
}

bool MusicWheel::SelectSection( const RString & SectionName )
{
	for( unsigned int i = 0; i < m_CurWheelItemData.size(); ++i )
	{
		if( m_CurWheelItemData[i]->m_sText == SectionName )
		{
			m_iSelection = i;		// select it
			return true;
		}
	}

	return false;
}

bool MusicWheel::SelectSong( const Song *p )
{
	if(p == NULL)
		return false;

	unsigned i;
	vector<WheelItemData *> &from = m_WheelItemDatas[GAMESTATE->m_SortOrder];
	for( i=0; i<from.size(); i++ )
	{
		if( from[i]->m_pSong == p )
		{
			// make its group the currently expanded group
			SetOpenGroup( from[i]->m_sText );
			break;
		}
	}

	if( i == from.size() )
		return false;

	for( i=0; i<m_CurWheelItemData.size(); i++ )
	{
		if( GetCurWheelItemData(i)->m_pSong == p )
			m_iSelection = i;		// select it
	}
	return true;
}

bool MusicWheel::SelectCourse( const Course *p )
{
	if(p == NULL)
		return false;

	unsigned i;
	vector<WheelItemData *> &from = m_WheelItemDatas[GAMESTATE->m_SortOrder];
	for( i=0; i<from.size(); i++ )
	{
		if( from[i]->m_pCourse == p )
		{
			// make its group the currently expanded group
			SetOpenGroup( from[i]->m_sText );
			break;
		}
	}

	if( i == from.size() )
		return false;

	for( i=0; i<m_CurWheelItemData.size(); i++ )
	{
		if( GetCurWheelItemData(i)->m_pCourse == p )
			m_iSelection = i;		// select it
	}

	return true;
}

bool MusicWheel::SelectModeMenuItem()
{
	/* Select the last-chosen option. */
	ASSERT( GAMESTATE->m_SortOrder == SORT_MODE_MENU );
	const vector<WheelItemData *> &from = m_WheelItemDatas[GAMESTATE->m_SortOrder];
	unsigned i;
	for( i=0; i<from.size(); i++ )
	{
		const GameCommand &gc = *from[i]->m_pAction;
		if( gc.m_sName == m_sLastModeMenuItem )
			break;
	}
	if( i == from.size() )
		return false;

	// make its group the currently expanded group
	SetOpenGroup( from[i]->m_sText );

	for( i=0; i<m_CurWheelItemData.size(); i++ )
	{
		if( GetCurWheelItemData(i)->m_pAction->m_sName != m_sLastModeMenuItem )
			continue;
		m_iSelection = i;		// select it
		break;
	}

	return true;
}

void MusicWheel::GetSongList(vector<Song*> &arraySongs, SortOrder so, const RString &sPreferredGroup )
{
	vector<Song*> apAllSongs;
	switch( so )
	{
	case SORT_PREFERRED:
		SONGMAN->GetPreferredSortSongs( apAllSongs, GAMESTATE->GetNumStagesLeft() );
		break;
	case SORT_POPULARITY:
		SONGMAN->GetPopularSongs( apAllSongs, GAMESTATE->m_sPreferredSongGroup, GAMESTATE->GetNumStagesLeft() );
		break;
	default:
		SONGMAN->GetSongs( apAllSongs, GAMESTATE->m_sPreferredSongGroup, GAMESTATE->GetNumStagesLeft() );
		break;
	}

	// copy only songs that have at least one Steps for the current GameMode
	for( unsigned i=0; i<apAllSongs.size(); i++ )
	{
		Song* pSong = apAllSongs[i];

		/* If we're on an extra stage, and this song is selected, ignore #SELECTABLE. */
		if( pSong != GAMESTATE->m_pCurSong || 
			(!GAMESTATE->IsExtraStage() && !GAMESTATE->IsExtraStage2()) )
		{
			/* Hide songs that asked to be hidden via #SELECTABLE. */
			if( so!=SORT_ROULETTE && !pSong->NormallyDisplayed() )
				continue;
			if( so!=SORT_ROULETTE && UNLOCKMAN->SongIsRouletteOnly( pSong ) )
				continue;
			/* Don't show in roulette if #SELECTABLE:NO. */
			if( so==SORT_ROULETTE && !pSong->RouletteDisplayed() )
				continue;
		}

		/* Hide locked songs.  If RANDOM_PICKS_LOCKED_SONGS, hide in Roulette and Random,
		 * too. */
		if( (so!=SORT_ROULETTE || !RANDOM_PICKS_LOCKED_SONGS) && UNLOCKMAN->SongIsLocked(pSong) )
			continue;

		// If the song has at least one steps, add it.
		if( pSong->HasStepsType(GAMESTATE->GetCurrentStyle()->m_StepsType) )
			arraySongs.push_back( pSong );
	}

	/* Hack: Add extra stage item if it was eliminated for any reason (eg. it's a long
	 * song). */
	if( GAMESTATE->IsAnExtraStage() )
	{
		Song* pSong;
		Steps* pSteps;
		SONGMAN->GetExtraStageInfo( GAMESTATE->IsExtraStage2(), GAMESTATE->GetCurrentStyle(), pSong, pSteps, NULL, NULL );

		if( find( arraySongs.begin(), arraySongs.end(), pSong ) == arraySongs.end() )
			arraySongs.push_back( pSong );
	}
}

void MusicWheel::BuildWheelItemDatas( vector<WheelItemData *> &arrayWheelItemDatas, SortOrder so )
{
	switch( so )
	{
	case SORT_MODE_MENU:
		{
			arrayWheelItemDatas.clear();	// clear out the previous wheel items 
			vector<RString> vsNames;
			split( MODE_MENU_CHOICE_NAMES, ",", vsNames );
			for( unsigned i=0; i<vsNames.size(); ++i )
			{
				switch( so )
				{
				case SORT_ALL_COURSES:
				case SORT_NONSTOP_COURSES:
				case SORT_ONI_COURSES:
				case SORT_ENDLESS_COURSES:
					/* Don't display course modes after the first stage. */
					if( !GAMESTATE->IsEventMode() && GAMESTATE->m_iCurrentStageIndex )
						continue;
				}

				WheelItemData wid( TYPE_SORT, NULL, "", NULL, SORT_MENU_COLOR );
				wid.m_pAction = HiddenPtr<GameCommand>( new GameCommand );
				wid.m_pAction->m_sName = vsNames[i];
				wid.m_pAction->Load( i, ParseCommands(CHOICE.GetValue(vsNames[i])) );
				wid.m_sLabel = WHEEL_TEXT(vsNames[i]);

				if( !wid.m_pAction->IsPlayable() )
					continue;

				arrayWheelItemDatas.push_back( new WheelItemData(wid) );
			}
			break;
		}
	case SORT_PREFERRED:
	case SORT_ROULETTE:
	case SORT_GROUP:
	case SORT_TITLE:
	case SORT_BPM:
	case SORT_POPULARITY:
	case SORT_TOP_GRADES:
	case SORT_ARTIST:
	case SORT_GENRE:
	case SORT_EASY_METER:
	case SORT_MEDIUM_METER:
	case SORT_HARD_METER:
	case SORT_CHALLENGE_METER:
		{
			///////////////////////////////////
			// Make an array of Song*, then sort them
			///////////////////////////////////
			vector<Song*> arraySongs;
			
			GetSongList(arraySongs, so, GAMESTATE->m_sPreferredSongGroup );

			bool bUseSections = true;

			// sort the songs
			switch( so )
			{
			case SORT_PREFERRED:
				// obey order specified by the preferred sort list
				break;
			case SORT_ROULETTE:
				SongUtil::SortSongPointerArrayByMeter( arraySongs, DIFFICULTY_EASY );
				if( (bool)PREFSMAN->m_bPreferredSortUsesGroups )
					stable_sort( arraySongs.begin(), arraySongs.end(), SongUtil::CompareSongPointersByGroup );
				bUseSections = false;
				break;
			case SORT_GROUP:
				SongUtil::SortSongPointerArrayByGroupAndTitle( arraySongs );
				bUseSections = GAMESTATE->m_sPreferredSongGroup == GROUP_ALL;
				break;
			case SORT_TITLE:
				SongUtil::SortSongPointerArrayByTitle( arraySongs );
				break;
			case SORT_BPM:
				SongUtil::SortSongPointerArrayByBPM( arraySongs );
				break;
			case SORT_POPULARITY:
				if( (int) arraySongs.size() > MOST_PLAYED_SONGS_TO_SHOW )
					arraySongs.erase( arraySongs.begin()+MOST_PLAYED_SONGS_TO_SHOW, arraySongs.end() );
				bUseSections = false;
				break;
			case SORT_TOP_GRADES:
				SongUtil::SortSongPointerArrayByGrades( arraySongs, true );
				break;
			case SORT_ARTIST:
				SongUtil::SortSongPointerArrayByArtist( arraySongs );
				break;
			case SORT_GENRE:
				SongUtil::SortSongPointerArrayByGenre( arraySongs );
				break;
			case SORT_EASY_METER:
				SongUtil::SortSongPointerArrayByMeter( arraySongs, DIFFICULTY_EASY );
				break;
			case SORT_MEDIUM_METER:
				SongUtil::SortSongPointerArrayByMeter( arraySongs, DIFFICULTY_MEDIUM );
				break;
			case SORT_HARD_METER:
				SongUtil::SortSongPointerArrayByMeter( arraySongs, DIFFICULTY_HARD );
				break;
			case SORT_CHALLENGE_METER:
				SongUtil::SortSongPointerArrayByMeter( arraySongs, DIFFICULTY_CHALLENGE );
				break;
			default:
				ASSERT(0);	// unhandled SortOrder
			}


			///////////////////////////////////
			// Build an array of WheelItemDatas from the sorted list of Song*'s
			///////////////////////////////////
			arrayWheelItemDatas.clear();	// clear out the previous wheel items 
			arrayWheelItemDatas.reserve( arraySongs.size() );

			switch( PREFSMAN->m_MusicWheelUsesSections )
			{
			case PrefsManager::NEVER:
				bUseSections = false;
				break;
			case PrefsManager::ABC_ONLY:
				if( so != SORT_TITLE && so != SORT_GROUP )
					bUseSections = false;
				break;
			}

			if( bUseSections )
			{
				// Sorting twice isn't necessary.  Instead, modify the compatator functions 
				// in Song.cpp to have the desired effect. -Chris
				/* Keeping groups together with the sorts is tricky and brittle; we
				 * keep getting OTHER split up without this.  However, it puts the 
				 * Grade and BPM sorts in the wrong order, and they're already correct,
				 * so don't re-sort for them. */
				/* We're using sections, so use the section name as the top-level sort. */
				if( so != SORT_TOP_GRADES && so != SORT_BPM )
					SongUtil::SortSongPointerArrayBySectionName(arraySongs, so);
			}

			// make WheelItemDatas with sections
			RString sLastSection = "";
			int iSectionColorIndex = 0;
			for( unsigned i=0; i< arraySongs.size(); i++ )
			{
				Song* pSong = arraySongs[i];
				if( bUseSections )
				{
					RString sThisSection = SongUtil::GetSectionNameFromSongAndSort( pSong, so );

					if( sThisSection != sLastSection )
					{
						// new section, make a section item
						RageColor colorSection = (so==SORT_GROUP) ? SONGMAN->GetSongGroupColor(pSong->m_sGroupName) : SECTION_COLORS.GetValue(iSectionColorIndex);
						iSectionColorIndex = (iSectionColorIndex+1) % NUM_SECTION_COLORS;
						arrayWheelItemDatas.push_back( new WheelItemData(TYPE_SECTION, NULL, sThisSection, NULL, colorSection) );
						sLastSection = sThisSection;
					}
				}
				arrayWheelItemDatas.push_back( new WheelItemData(TYPE_SONG, pSong, sLastSection, NULL, SONGMAN->GetSongColor(pSong)) );
			}

			if( so != SORT_ROULETTE )
			{
				if( SHOW_ROULETTE )
					arrayWheelItemDatas.push_back( new WheelItemData(TYPE_ROULETTE, NULL, "", NULL, RageColor(1,0,0,1)) );
				/* Only add TYPE_PORTAL if there's at least one song on the list. */
				bool bFoundAnySong = false;
				for( unsigned i=0; !bFoundAnySong && i < arrayWheelItemDatas.size(); i++ )
					if( arrayWheelItemDatas[i]->m_Type == TYPE_SONG )
						bFoundAnySong = true;

				if( SHOW_RANDOM && bFoundAnySong )
					arrayWheelItemDatas.push_back( new WheelItemData(TYPE_RANDOM, NULL, "", NULL, RageColor(1,0,0,1)) );

				if( SHOW_PORTAL && bFoundAnySong )
					arrayWheelItemDatas.push_back( new WheelItemData(TYPE_PORTAL, NULL, "", NULL, RageColor(1,0,0,1)) );
			}

			if( GAMESTATE->IsAnExtraStage() )
			{
				Song* pSong;
				Steps* pSteps;
				SONGMAN->GetExtraStageInfo( GAMESTATE->IsExtraStage2(), GAMESTATE->GetCurrentStyle(), pSong, pSteps, NULL, NULL );
				
				for( unsigned i=0; i<arrayWheelItemDatas.size(); i++ )
				{
					if( arrayWheelItemDatas[i]->m_pSong == pSong )
					{
						/* Change the song color. */
						arrayWheelItemDatas[i]->m_color = SONG_REAL_EXTRA_COLOR;
						break;
					}
				}
			}
			break;
		}
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
		{
			bool bOnlyPreferred = PREFSMAN->m_CourseSortOrder == PrefsManager::COURSE_SORT_PREFERRED;

			vector<CourseType> vct;
			switch( so )
			{
			case SORT_NONSTOP_COURSES:
				vct.push_back( COURSE_TYPE_NONSTOP );
				break;
			case SORT_ONI_COURSES:
				vct.push_back( COURSE_TYPE_ONI );
				vct.push_back( COURSE_TYPE_SURVIVAL );
				break;
			case SORT_ENDLESS_COURSES:
				vct.push_back( COURSE_TYPE_ENDLESS );
				break;
			case SORT_ALL_COURSES:
				FOREACH_ENUM2( CourseType, i )
					vct.push_back( i );
				break;
			default: ASSERT(0); break;
			}

			vector<Course*> apCourses;
			FOREACH_CONST( CourseType, vct, ct )
			{
				if( bOnlyPreferred )
					SONGMAN->GetPreferredSortCourses( *ct, apCourses, PREFSMAN->m_bAutogenGroupCourses );
				else
					SONGMAN->GetCourses( *ct, apCourses, PREFSMAN->m_bAutogenGroupCourses );
			}

			if( PREFSMAN->m_CourseSortOrder == PrefsManager::COURSE_SORT_SONGS )
			{
				CourseUtil::SortCoursePointerArrayByDifficulty( apCourses );
			}
			else
			{
				switch( PREFSMAN->m_CourseSortOrder )
				{
				case PrefsManager::COURSE_SORT_PREFERRED:
					break;
				case PrefsManager::COURSE_SORT_METER:
					CourseUtil::SortCoursePointerArrayByAvgDifficulty( apCourses );
					break;
				case PrefsManager::COURSE_SORT_METER_SUM:
					CourseUtil::SortCoursePointerArrayByTotalDifficulty( apCourses );
					break;
				case PrefsManager::COURSE_SORT_RANK:
					CourseUtil::SortCoursePointerArrayByRanking( apCourses );
					break;
				default:	ASSERT(0);
				}

				// since we can't agree, make it an option
				if( g_bMoveRandomToEnd )
					CourseUtil::MoveRandomToEnd( apCourses );
			}

			if( so == SORT_ALL_COURSES )
				CourseUtil::SortCoursePointerArrayByType( apCourses );

			arrayWheelItemDatas.clear();	// clear out the previous wheel items 

			RString sLastSection = "";
			int iSectionColorIndex = 0;
			for( unsigned i=0; i<apCourses.size(); i++ )	// foreach course
			{
				Course* pCourse = apCourses[i];

				// if unlocks are on, make sure it is unlocked
				if ( UNLOCKMAN->CourseIsLocked(pCourse) )
					continue;

				RString sThisSection = "";
				if( so == SORT_ALL_COURSES )
				{
					switch( pCourse->GetPlayMode() )
					{
					case PLAY_MODE_ONI:	sThisSection = "Oni";		break;
					case PLAY_MODE_NONSTOP:	sThisSection = "Nonstop";	break;
					case PLAY_MODE_ENDLESS:	sThisSection = "Endless";	break;
					}
				}

				// check that this course has at least one song playable in the current style
				if( !pCourse->IsPlayableIn(GAMESTATE->GetCurrentStyle()->m_StepsType) )
					continue;

				if( sThisSection != sLastSection )	// new section, make a section item
				{
					RageColor c = SECTION_COLORS.GetValue(iSectionColorIndex);
					iSectionColorIndex = (iSectionColorIndex+1) % NUM_SECTION_COLORS;
					arrayWheelItemDatas.push_back( new WheelItemData(TYPE_SECTION, NULL, sThisSection, NULL, c) );
					sLastSection = sThisSection;
				}

				RageColor c = ( pCourse->m_sGroupName.size() == 0 ) ? pCourse->GetColor() : SONGMAN->GetCourseColor(pCourse);
				arrayWheelItemDatas.push_back( new WheelItemData(TYPE_COURSE, NULL, sThisSection, pCourse, c) );
			}
			break;
		}
	}

	// init music status icons
	for( unsigned i=0; i<arrayWheelItemDatas.size(); i++ )
	{
		WheelItemData& WID = *arrayWheelItemDatas[i];
		if( WID.m_pSong != NULL )
		{
			WID.m_Flags.bHasBeginnerOr1Meter = WID.m_pSong->IsEasy( GAMESTATE->GetCurrentStyle()->m_StepsType );
			WID.m_Flags.bEdits = WID.m_pSong->HasEdits( GAMESTATE->GetCurrentStyle()->m_StepsType );
			WID.m_Flags.iStagesForSong = SongManager::GetNumStagesForSong( WID.m_pSong );
		}
		else if( WID.m_pCourse != NULL )
		{
			WID.m_Flags.bHasBeginnerOr1Meter = false;
			WID.m_Flags.bEdits = WID.m_pCourse->IsAnEdit();
			WID.m_Flags.iStagesForSong = 1;
		}
	}

	// init crowns
	if( so == SORT_POPULARITY )
	{
		// init crown icons 
		for( unsigned i=0; i< min(3u,arrayWheelItemDatas.size()); i++ )
		{
			WheelItemData& WID = *arrayWheelItemDatas[i];
			WID.m_Flags.iPlayersBestNumber = i+1;
		}
	}

	if( arrayWheelItemDatas.empty() )
	{
		arrayWheelItemDatas.push_back( new WheelItemData(TYPE_SECTION, NULL, "- EMPTY -", NULL, RageColor(1,0,0,1)) );
	}
}

void MusicWheel::UpdateSwitch()
{
	switch( m_WheelState )
	{
	case STATE_FLYING_OFF_BEFORE_NEXT_SORT:
		{
			const Song* pPrevSelectedSong = GetCurWheelItemData(m_iSelection)->m_pSong;

			SCREENMAN->PostMessageToTopScreen( SM_SortOrderChanged, 0 );
			
			SetOpenGroup( SongUtil::GetSectionNameFromSongAndSort(pPrevSelectedSong, GAMESTATE->m_SortOrder) );

			m_iSelection = 0;

			//
			// Select the previously selected item
			//
			switch( GAMESTATE->m_SortOrder )
			{
			default:
				// Look for the last selected song or course
				SelectSongOrCourse();
				break;
			case SORT_MODE_MENU:
				SelectModeMenuItem();
				break;
			}

			//
			// Change difficulty for sorts by meter - XXX: do this with GameCommand?
			//
			Difficulty dc = DIFFICULTY_Invalid;
			switch( GAMESTATE->m_SortOrder )
			{
			case SORT_EASY_METER:		dc = DIFFICULTY_EASY;		break;
			case SORT_MEDIUM_METER:		dc = DIFFICULTY_MEDIUM;		break;
			case SORT_HARD_METER:		dc = DIFFICULTY_HARD;		break;
			case SORT_CHALLENGE_METER:	dc = DIFFICULTY_CHALLENGE;	break;
			}
			if( dc != DIFFICULTY_Invalid )
			{
				FOREACH_PlayerNumber( p )
					if( GAMESTATE->IsPlayerEnabled(p) )
						GAMESTATE->m_PreferredDifficulty[p].Set( dc );
			}

			SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );
			RebuildWheelItems();
			TweenOnScreenForSort();

			SCREENMAN->ZeroNextUpdate();
		}
		break;

	case STATE_FLYING_ON_AFTER_NEXT_SORT:
		m_WheelState = STATE_SELECTING;	// now, wait for input
		break;

	case STATE_SELECTING:
		m_fTimeLeftInState = 0;
		break;
	case STATE_ROULETTE_SPINNING:
	case STATE_RANDOM_SPINNING:
		break;
	case STATE_LOCKED:
		break;
	case STATE_ROULETTE_SLOWING_DOWN:
		if( m_iSwitchesLeftInSpinDown == 0 )
		{
			m_WheelState = STATE_LOCKED;
			m_fTimeLeftInState = 0;
			SCREENMAN->PlayStartSound();
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
		ASSERT(0);	// all state changes should be handled explicitly
		break;
	}
}

void MusicWheel::ChangeMusic( int iDist )
{
	m_iSelection += iDist;
	wrap( m_iSelection, m_CurWheelItemData.size() );

	RebuildWheelItems( iDist );

	m_fPositionOffsetFromSelection += iDist;

	SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );

	/* If we're moving automatically, don't play this; it'll be called in Update. */
	if(!IsMoving())
		m_soundChangeMusic.Play();
}


bool MusicWheel::ChangeSort( SortOrder new_so )	// return true if change successful
{
	ASSERT( new_so < NUM_SortOrder );
	if( GAMESTATE->m_SortOrder == new_so )
		return false;

	/* Don't change to SORT_MODE_MENU if it doesn't have at least two choices. */
	if( new_so == SORT_MODE_MENU && m_WheelItemDatas[new_so].size() < 2 )
		return false;

	switch( m_WheelState )
	{
	case STATE_SELECTING:
	case STATE_FLYING_ON_AFTER_NEXT_SORT:
		break;	// fall through
	default:
		return false;	// don't continue
	}

	SCREENMAN->PostMessageToTopScreen( SM_SortOrderChanging, 0 );

	m_soundChangeSort.Play();

	TweenOffScreenForSort();

	/* Save the new preference. */
	if( IsSongSort(new_so) )
		GAMESTATE->m_PreferredSortOrder = new_so;
	GAMESTATE->m_SortOrder.Set( new_so );
	
	return true;
}

bool MusicWheel::NextSort()		// return true if change successful
{
	// don't allow NextSort when on the sort menu or mode menu
	if( GAMESTATE->m_SortOrder == SORT_MODE_MENU )
		return false;

	// find the index of the current sort
	int cur = 0;
	while( cur < int(SONG_SORT_ORDERS.size()) && SONG_SORT_ORDERS[cur] != GAMESTATE->m_SortOrder )
		++cur;

	// move to the next sort with wrapping
	++cur;
	wrap( cur, SONG_SORT_ORDERS.size() );

	// apply new sort
	SortOrder soNew = SONG_SORT_ORDERS[cur];
	return ChangeSort( soNew );
}

bool MusicWheel::Select()	// return true if this selection ends the screen
{
	LOG->Trace( "MusicWheel::Select()" );

	switch( m_WheelState )
	{
	case STATE_FLYING_OFF_BEFORE_NEXT_SORT:
	case STATE_ROULETTE_SLOWING_DOWN:
		return false;
	}

	if( m_WheelState == STATE_ROULETTE_SPINNING )
	{
		m_WheelState = STATE_ROULETTE_SLOWING_DOWN;
		m_iSwitchesLeftInSpinDown = ROULETTE_SLOW_DOWN_SWITCHES/2+1 + RandomInt( ROULETTE_SLOW_DOWN_SWITCHES/2 );
		m_fTimeLeftInState = 0.1f;
		return false;
	}


	if( m_WheelState == STATE_RANDOM_SPINNING )
	{
		m_fPositionOffsetFromSelection = max(m_fPositionOffsetFromSelection, 0.3f);
		m_WheelState = STATE_LOCKED;
		SCREENMAN->PlayStartSound();
		m_fLockedWheelVelocity = 0;
		SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );
		return false;
	}

	if( !WheelBase::Select() )
		return false;

	switch( m_CurWheelItemData[m_iSelection]->m_Type )
	{
	case TYPE_ROULETTE:  
		StartRoulette();
		break;
	case TYPE_RANDOM:
		StartRandom();
		break;
	case TYPE_SONG:
	case TYPE_COURSE:
	case TYPE_PORTAL:
		break;
	case TYPE_SORT:
		LOG->Trace("New sort order selected: %s - %s", 
			GetCurWheelItemData(m_iSelection)->m_sLabel.c_str(), 
			SortOrderToString(GetCurWheelItemData(m_iSelection)->m_pAction->m_SortOrder).c_str() );
		GetCurWheelItemData(m_iSelection)->m_pAction->ApplyToAllPlayers();
		ChangeSort( GAMESTATE->m_PreferredSortOrder );
		m_sLastModeMenuItem = GetCurWheelItemData(m_iSelection)->m_pAction->m_sName;
		break;
	default:
		break;
	}
	return true;
}

void MusicWheel::StartRoulette() 
{
	m_WheelState = STATE_ROULETTE_SPINNING;
	m_Moving = 1;
	m_TimeBeforeMovingBegins = 0;
	m_SpinSpeed = 1.0f/ROULETTE_SWITCH_SECONDS;
	GAMESTATE->m_SortOrder.Set( SORT_ROULETTE );
	SetOpenGroup( "" );
}

void MusicWheel::StartRandom()
{
	/* If RANDOM_PICKS_LOCKED_SONGS is disabled, pick a song from the active sort and
	 * section.  If enabled, picking from the section makes it too easy to trick the
	 * game into picking a locked song, so pick from SORT_ROULETTE. */
	if( RANDOM_PICKS_LOCKED_SONGS )
	{
		/* Shuffle and use the roulette wheel. */
		RandomGen rnd;
		random_shuffle( m_WheelItemDatas[SORT_ROULETTE].begin(), m_WheelItemDatas[SORT_ROULETTE].end(), rnd );
		GAMESTATE->m_SortOrder.Set( SORT_ROULETTE );
	}
	else
	{
		GAMESTATE->m_SortOrder.Set( GAMESTATE->m_PreferredSortOrder );
	}
	SetOpenGroup( "" );

	m_Moving = -1;
	m_TimeBeforeMovingBegins = 0;
	m_SpinSpeed = 1.0f/ROULETTE_SWITCH_SECONDS;
	m_SpinSpeed *= 20.0f; /* faster! */
	m_WheelState = STATE_RANDOM_SPINNING;

	SelectSong( GetPreferredSelectionForRandomOrPortal() );

	this->Select();
	RebuildWheelItems();
}

void MusicWheel::SetOpenGroup( RString group )
{
	LOG->Trace( "SetOpenGroup %s", group.c_str() );
	m_sExpandedSectionName = group;

	const WheelItemBaseData *old = NULL;
	if( !m_CurWheelItemData.empty() )
		old = GetCurWheelItemData(m_iSelection);

	m_CurWheelItemData.clear();
	vector<WheelItemData *> &from = m_WheelItemDatas[GAMESTATE->m_SortOrder];
	for( unsigned i = 0; i < from.size(); ++i )
	{
		WheelItemData &d = *from[i];
		if( (d.m_Type == TYPE_SONG || d.m_Type == TYPE_COURSE) && !d.m_sText.empty() &&
			 d.m_sText != group )
			 continue;

		/* Only show tutorial songs in arcade */
		if( GAMESTATE->m_PlayMode!=PLAY_MODE_REGULAR && 
			d.m_pSong &&
			d.m_pSong->IsTutorial() )
			continue;

		m_CurWheelItemData.push_back(&d);
	}


	//
	// Try to select the item that was selected before changing groups
	//
	m_iSelection = 0;

	for( unsigned i=0; i<m_CurWheelItemData.size(); i++ )
	{
		if( m_CurWheelItemData[i] == old )
		{
			m_iSelection=i;
			break;
		}
	}

	RebuildWheelItems();
}

bool MusicWheel::IsRouletting() const
{
	return m_WheelState == STATE_ROULETTE_SPINNING || m_WheelState == STATE_ROULETTE_SLOWING_DOWN ||
		   m_WheelState == STATE_RANDOM_SPINNING;
}

Song* MusicWheel::GetSelectedSong()
{
	switch( m_CurWheelItemData[m_iSelection]->m_Type )
	{
	case TYPE_PORTAL:
		return GetPreferredSelectionForRandomOrPortal();
	}

	return GetCurWheelItemData(m_iSelection)->m_pSong;
}

/* Find a random song.  If possible, find one that has the preferred difficulties of
 * each player.  Prefer songs in the active group, if any. 
 *
 * Note that if this is called, we *must* find a song.  We will only be called if
 * the active sort has at least one song, but there may be no open group.  This means
 * that any filters and preferences applied here must be optional. */
Song *MusicWheel::GetPreferredSelectionForRandomOrPortal()
{
	// probe to find a song that has the preferred 
	// difficulties of each player
	vector<Difficulty> vDifficultiesToRequire;
	FOREACH_HumanPlayer(p)
	{
		if( GAMESTATE->m_PreferredDifficulty[p] == DIFFICULTY_Invalid )
			continue;	// skip

		// TRICKY: Don't require that edits be present if perferred 
		// difficulty is DIFFICULTY_EDIT.  Otherwise, players could use this 
		// to set up a 100% chance of getting a particular locked song by 
		// having a single edit for a locked song.
		if( GAMESTATE->m_PreferredDifficulty[p] == DIFFICULTY_EDIT )
			continue;	// skip

		vDifficultiesToRequire.push_back( GAMESTATE->m_PreferredDifficulty[p] );
	}

	RString sPreferredGroup = m_sExpandedSectionName;
	vector<WheelItemData *> &wid = m_WheelItemDatas[GAMESTATE->m_SortOrder];

	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;

#define NUM_PROBES 1000
	for( int i=0; i<NUM_PROBES; i++ )
	{
		/* Maintaining difficulties is higher priority than maintaining the current
		 * group. */
		if( i == NUM_PROBES/4 )
			sPreferredGroup = "";
		if( i == NUM_PROBES/2 )
			vDifficultiesToRequire.clear();

		int iSelection = RandomInt( wid.size() );
		if( wid[iSelection]->m_Type != TYPE_SONG )
			continue;

		const Song *pSong = wid[iSelection]->m_pSong;

		if( !sPreferredGroup.empty() && wid[iSelection]->m_sText != sPreferredGroup )
			continue;

		// There's an off possibility that somebody might have only one song with only beginner steps.
		if( i < 900 && pSong->IsTutorial() )
			continue;

		FOREACH( Difficulty, vDifficultiesToRequire, d )
			if( !pSong->HasStepsTypeAndDifficulty(st,*d) )
				goto try_next;
		return wid[iSelection]->m_pSong;
try_next:
		;
	}
	LOG->Warn( "Couldn't find any songs" );
	return wid[0]->m_pSong;
}

void MusicWheel::FinishChangingSorts()
{
	FinishTweening();
	m_WheelState = STATE_SELECTING;
	m_fTimeLeftInState = 0;
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
