#include "global.h"
#include "MusicWheel.h"
#include "RageUtil.h"
#include "SongManager.h"
#include "GameManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "NetworkSyncManager.h"
#include "ProfileManager.h"
#include "Song.h"
#include "Course.h"
#include "Steps.h"
#include "UnlockManager.h"
#include "GameCommand.h"
#include "ActorUtil.h"
#include "SongUtil.h"
#include "CourseUtil.h"
#include "Style.h"
#include "PlayerState.h"
#include "CommonMetrics.h"
#include "MessageManager.h"
#include "LocalizedString.h"

static Preference<bool> g_bMoveRandomToEnd( "MoveRandomToEnd", false );
static Preference<bool> g_bPrecacheAllSorts( "PreCacheAllWheelSorts", false);

#define NUM_WHEEL_ITEMS		((int)ceil(NUM_WHEEL_ITEMS_TO_DRAW+2))
#define WHEEL_TEXT(s)		THEME->GetString( "MusicWheel", ssprintf("%sText",s.c_str()) );
#define CUSTOM_ITEM_WHEEL_TEXT(s)		THEME->GetString( "MusicWheel", ssprintf("CustomItem%sText",s.c_str()) );

static RString SECTION_COLORS_NAME( size_t i )	{ return ssprintf("SectionColor%d",int(i+1)); }
static RString CHOICE_NAME( RString s )		{ return ssprintf("Choice%s",s.c_str()); }
static RString CUSTOM_WHEEL_ITEM_NAME( RString s )		{ return ssprintf("CustomWheelItem%s",s.c_str()); }
static RString CUSTOM_WHEEL_ITEM_COLOR( RString s )		{ return ssprintf("%sColor",s.c_str()); }

static LocalizedString EMPTY_STRING	( "MusicWheel", "Empty" );

AutoScreenMessage( SM_SongChanged ); // TODO: Replace this with a Message and MESSAGEMAN
AutoScreenMessage( SM_SortOrderChanging );
AutoScreenMessage( SM_SortOrderChanged );

static SortOrder ForceAppropriateSort( PlayMode pm, SortOrder so )
{
	switch( pm )
	{
		// in course modes, force a particular sort
		case PLAY_MODE_ONI:	return SORT_ONI_COURSES;
		case PLAY_MODE_NONSTOP:	return SORT_NONSTOP_COURSES;
		case PLAY_MODE_ENDLESS:	return SORT_ENDLESS_COURSES;
		default: break;
	}

	// If we're not in a course mode, don't start in a course sort.
	switch( so )
	{
		case SORT_ONI_COURSES:
		case SORT_NONSTOP_COURSES:
		case SORT_ENDLESS_COURSES:
			so = SortOrder_Invalid;
		default:
			return so;
	}
}

MusicWheelItem *MusicWheel::MakeItem()
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
	RECENT_SONGS_TO_SHOW		.Load(sType,"RecentSongsToShow");
	MODE_MENU_CHOICE_NAMES		.Load(sType,"ModeMenuChoiceNames");
	SORT_ORDERS			.Load(sType,"SortOrders");
	SHOW_EASY_FLAG			.Load(sType,"UseEasyMarkerFlag");
	USE_SECTIONS_WITH_PREFERRED_GROUP		.Load(sType,"UseSectionsWithPreferredGroup");
	HIDE_INACTIVE_SECTIONS		.Load(sType,"OnlyShowActiveSection");
	HIDE_ACTIVE_SECTION_TITLE		.Load(sType,"HideActiveSectionTitle");
	REMIND_WHEEL_POSITIONS		.Load(sType,"RemindWheelPositions");
	vector<RString> vsModeChoiceNames;
	split( MODE_MENU_CHOICE_NAMES, ",", vsModeChoiceNames );
	CHOICE				.Load(sType,CHOICE_NAME,vsModeChoiceNames);
	SECTION_COLORS			.Load(sType,SECTION_COLORS_NAME,NUM_SECTION_COLORS);

	CUSTOM_WHEEL_ITEM_NAMES		.Load(sType,"CustomWheelItemNames");
	vector<RString> vsCustomItemNames;
	split( CUSTOM_WHEEL_ITEM_NAMES, ",", vsCustomItemNames );
	CUSTOM_CHOICES.Load(sType,CUSTOM_WHEEL_ITEM_NAME,vsCustomItemNames);
	CUSTOM_CHOICE_COLORS.Load(sType,CUSTOM_WHEEL_ITEM_COLOR,vsCustomItemNames);

	ROULETTE_COLOR	.Load(sType,"RouletteColor");
	RANDOM_COLOR	.Load(sType,"RandomColor");
	PORTAL_COLOR	.Load(sType,"PortalColor");
	EMPTY_COLOR		.Load(sType,"EmptyColor");

	WheelBase::Load( sType );

	SONGMAN->UpdateRankingCourses();

	m_soundChangeSort.Load(	THEME->GetPathS(sType,"sort") );
	m_soundExpand.Load(	THEME->GetPathS(sType,"expand"), true );
	m_soundCollapse.Load(	THEME->GetPathS(sType,"collapse"), true );

	// Update for SORT_MOST_PLAYED.
	SONGMAN->UpdatePopular();

	/* Sort SONGMAN's songs by CompareSongPointersByTitle, so we can do other sorts (with
	 * stable_sort) from its output, and title will be the secondary sort, without having
	 * to re-sort by title each time. */
	SONGMAN->SortSongs();

	
	FOREACH_ENUM( SortOrder, so ) {
		m_WheelItemDatasStatus[so]=INVALID;
	}
}

void MusicWheel::BeginScreen()
{
	RageTimer timer;
	RString times;
	FOREACH_ENUM( SortOrder, so ) {	
		if(m_WheelItemDatasStatus[so]!=INVALID) {
			m_WheelItemDatasStatus[so]=NEEDREFILTER;
			
		}

		if(g_bPrecacheAllSorts) {
			readyWheelItemsData(so);
			times += ssprintf( "%i:%.3f ", so, timer.GetDeltaTime() );
		}
	}
	if(g_bPrecacheAllSorts) {
		LOG->Trace( "MusicWheel sorting took: %s", times.c_str() );
	}

	// Set m_LastModeMenuItem to the first item that matches the current mode.  (Do this
	// after building wheel item data.) 
	{
		const vector<MusicWheelItemData *> &from = getWheelItemsData(SORT_MODE_MENU);
		for( unsigned i=0; i<from.size(); i++ )
		{
			ASSERT( &*from[i]->m_pAction != nullptr );
			if( from[i]->m_pAction->DescribesCurrentModeForAllPlayers() )
			{
				m_sLastModeMenuItem = from[i]->m_pAction->m_sName;
				break;
			}
		}
	}

	WheelBase::BeginScreen();

	if( GAMESTATE->IsAnExtraStageAndSelectionLocked() )
	{
		m_WheelState = STATE_LOCKED;
		SCREENMAN->PlayStartSound();
		m_fLockedWheelVelocity = 0;
	}

	GAMESTATE->m_SortOrder.Set( GAMESTATE->m_PreferredSortOrder );

	// Never start in the mode menu; some elements may not initialize correctly.
	if( GAMESTATE->m_SortOrder == SORT_MODE_MENU )
		GAMESTATE->m_SortOrder.Set( SortOrder_Invalid );

	GAMESTATE->m_SortOrder.Set( ForceAppropriateSort(GAMESTATE->m_PlayMode, GAMESTATE->m_SortOrder) );

	/* Only save the sort order if the player didn't already have one.
	 * If he did, don't overwrite it. */
	if( GAMESTATE->m_PreferredSortOrder == SortOrder_Invalid )
		GAMESTATE->m_PreferredSortOrder = GAMESTATE->m_SortOrder;

	if(GAMESTATE->m_sPreferredSongGroup != GROUP_ALL)
	{
		// If a preferred song group is set, open the group and select the
		// first song in the group. -aj
		if(!GAMESTATE->IsCourseMode())
		{
			vector<Song*> vTemp = SONGMAN->GetSongs(GAMESTATE->m_sPreferredSongGroup);
			ASSERT(vTemp.size() > 0);
			GAMESTATE->m_pCurSong.Set(vTemp[0]);
		};
		SetOpenSection(GAMESTATE->m_sPreferredSongGroup);
		SelectSongOrCourse();
	}
	else if( !SelectSongOrCourse() )
	{
		// Select the the previously selected song (if any)
		SetOpenSection("");
	}

	if( REMIND_WHEEL_POSITIONS && HIDE_INACTIVE_SECTIONS )
	{
		// store the group song index, run this also here because it forgets the current position when
		// not changing the song if you came back from gameplay or your last round song (profiles)
		// is not the first one in the group.
		for( unsigned idx = 0 ; idx < m_viWheelPositions.size() ; idx++ )
		{
			if( m_sExpandedSectionName == SONGMAN->GetSongGroupByIndex(idx) )
			{
				m_viWheelPositions[idx] = m_iSelection;
			}
		}
	}

	// rebuild the WheelItems that appear on screen
	RebuildWheelItems();

	/* Invalidate current Song if it can't be played
	 * because there are not enough stages remaining. */
	if(GAMESTATE->m_pCurSong != nullptr &&
		GameState::GetNumStagesMultiplierForSong(GAMESTATE->m_pCurSong) >
		GAMESTATE->GetSmallestNumStagesLeftForAnyHumanPlayer())
	{
		GAMESTATE->m_pCurSong.Set(nullptr);
	}

	/* Invalidate current Steps if it can't be played
	 * because there are not enough stages remaining. */
	FOREACH_ENUM(PlayerNumber, p)
	{
		if(GAMESTATE->m_pCurSteps[p] != nullptr)
		{
			vector<Steps*> vpPossibleSteps;
			if(GAMESTATE->m_pCurSong != nullptr)
			{
				SongUtil::GetPlayableSteps(GAMESTATE->m_pCurSong, vpPossibleSteps);
			}
			bool bStepsIsPossible = find(vpPossibleSteps.begin(), vpPossibleSteps.end(), GAMESTATE->m_pCurSteps[p]) == vpPossibleSteps.end();
			if(!bStepsIsPossible)
			{
				GAMESTATE->m_pCurSteps[p].Set(nullptr);
			}
		}
	}
}

MusicWheel::~MusicWheel()
{
	FOREACH_ENUM( SortOrder, so ) {
		vector<MusicWheelItemData*>::iterator i = m__UnFilteredWheelItemDatas[so].begin();
		vector<MusicWheelItemData*>::iterator iEnd = m__UnFilteredWheelItemDatas[so].end();
		for( ; i != iEnd; ++i ) {
			delete *i;
		}

	}
}

void MusicWheel::ReloadSongList()
{
	int songIdxToPreserve = m_iSelection;
	// Remove the song from any sorting caches:
	FOREACH_ENUM( SortOrder, so ) {
		m_WheelItemDatasStatus[so]=INVALID;
	}
	// rebuild the info associated with this sort order
	readyWheelItemsData(GAMESTATE->m_SortOrder);
	// re-open the section to refresh song counts, etc.
	SetOpenSection(m_sExpandedSectionName);
	// navigate to the song nearest to what was previously selected
	m_iSelection = songIdxToPreserve;
	RebuildWheelItems();
	// refresh the song preview
	SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );
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
	vector<MusicWheelItemData *> &wiWheelItems = getWheelItemsData(GAMESTATE->m_SortOrder);
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
	if( p == nullptr )
		return false;

	unsigned i;
	vector<MusicWheelItemData *> &from = getWheelItemsData(GAMESTATE->m_SortOrder);
	for( i=0; i<from.size(); i++ )
	{
		if( from[i]->m_pSong == p )
		{
			// make its group the currently expanded group
			SetOpenSection( from[i]->m_sText );
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
	if( p == nullptr )
		return false;

	unsigned i;
	vector<MusicWheelItemData *> &from = getWheelItemsData(GAMESTATE->m_SortOrder);
	for( i=0; i<from.size(); i++ )
	{
		if( from[i]->m_pCourse == p )
		{
			// make its group the currently expanded group
			SetOpenSection( from[i]->m_sText );
			break;
		}
	}

	if( i == from.size() )
		return false;

	for( i=0; i<m_CurWheelItemData.size(); i++ )
	{
		if( GetCurWheelItemData(i)->m_pCourse == p )
			m_iSelection = i; // select it
	}

	return true;
}

bool MusicWheel::SelectModeMenuItem()
{
	// Select the last-chosen option.
	ASSERT( GAMESTATE->m_SortOrder == SORT_MODE_MENU );
	const vector<MusicWheelItemData *> &from = getWheelItemsData(GAMESTATE->m_SortOrder);
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
	SetOpenSection( from[i]->m_sText );

	for( i=0; i<m_CurWheelItemData.size(); i++ )
	{
		if( GetCurWheelItemData(i)->m_pAction->m_sName != m_sLastModeMenuItem )
			continue;
		m_iSelection = i;		// select it
		break;
	}

	return true;
}

// bool MusicWheel::SelectCustomItem()

void MusicWheel::GetSongList( vector<Song*> &arraySongs, SortOrder so )
{
	vector<Song*> apAllSongs;
	switch( so )
	{
	case SORT_PREFERRED:
		SONGMAN->GetPreferredSortSongs( apAllSongs );
		break;
	case SORT_POPULARITY:
		apAllSongs = SONGMAN->GetPopularSongs();
		break;
	case SORT_GROUP:
		// if we're not using sections with a preferred song group, and there
		// is a group to load, only load those songs. -aj
		if(GAMESTATE->m_sPreferredSongGroup != GROUP_ALL && !USE_SECTIONS_WITH_PREFERRED_GROUP )
		{
			apAllSongs = SONGMAN->GetSongs(GAMESTATE->m_sPreferredSongGroup);
			break;
		}
		// otherwise fall through
	default:
		apAllSongs = SONGMAN->GetAllSongs();
		break;
	}

	FOREACH_PlayerNumber(pn)
	{
		if(GAMESTATE->IsPlayerEnabled(pn))
		{
			Profile* prof= PROFILEMAN->GetProfile(pn);
			for(size_t i= 0; i < prof->m_songs.size(); ++i)
			{
				apAllSongs.push_back(prof->m_songs[i]);
			}
		}
	}

	// filter songs that we don't have enough stages to play
	{
		vector<Song*> vTempSongs;
		SongCriteria sc;
		sc.m_iMaxStagesForSong = GAMESTATE->GetSmallestNumStagesLeftForAnyHumanPlayer();
		SongUtil::FilterSongs( sc, apAllSongs, vTempSongs );
		apAllSongs = vTempSongs;
	}

	// copy only songs that have at least one Steps for the current GameMode
	for( unsigned i=0; i<apAllSongs.size(); i++ )
	{
		Song* pSong = apAllSongs[i];

		int iLocked = UNLOCKMAN->SongIsLocked( pSong );
		if( iLocked & LOCKED_DISABLED )
			continue;

		// If we're on an extra stage, and this song is selected, ignore #SELECTABLE.
		if( pSong != GAMESTATE->m_pCurSong || !GAMESTATE->IsAnExtraStage() )
		{
			// Hide songs that asked to be hidden via #SELECTABLE.
			if( iLocked & LOCKED_SELECTABLE )
				continue;
			if( so != SORT_ROULETTE && iLocked & LOCKED_ROULETTE )
				continue;
		}

		/* Hide locked songs. If RANDOM_PICKS_LOCKED_SONGS, hide in Roulette
		 * and Random, too. */
		if( (so!=SORT_ROULETTE || !RANDOM_PICKS_LOCKED_SONGS) && iLocked )
			continue;

		if( PREFSMAN->m_bOnlyPreferredDifficulties )
		{
			// if the song has steps that fit the preferred difficulty of the default player
			if( pSong->HasStepsTypeAndDifficulty( GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType,GAMESTATE->m_PreferredDifficulty[GAMESTATE->GetFirstHumanPlayer()] ) )
				arraySongs.push_back( pSong );
		}
		else
		{
			// Online mode doesn't support auto set style.  A song that only has
			// dance-double steps will show up when dance-single was selected, with
			// no playable steps.  Then the game will crash when trying to play it.
			// -Kyz
			if(CommonMetrics::AUTO_SET_STYLE && !NSMAN->isSMOnline)
			{
				// with AUTO_SET_STYLE on and Autogen off, some songs may get
				// hidden. Search through every playable StepsType until you
				// find one, then add the song.
				// see Issue 147 for more information. -aj
				// http://ssc.ajworld.net/sm-ssc/bugtracker/view.php?id=147
				set<StepsType> vStepsType;
				SongUtil::GetPlayableStepsTypes( pSong, vStepsType );

				for (StepsType const &type : vStepsType)
				{
					if(pSong->HasStepsType(type))
					{
						arraySongs.push_back( pSong );
						break;
					}
				}
			}
			else
			{
				// If the song has at least one steps, add it.
				if( pSong->HasStepsType(GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType) )
					arraySongs.push_back( pSong );
			}
		}
	}

	/* Hack: Add extra stage item if it was eliminated for any reason
	 * (eg. it's a long song). */
	if( GAMESTATE->IsAnExtraStage() )
	{
		Song* pSong;
		Steps* pSteps;
		SONGMAN->GetExtraStageInfo( GAMESTATE->IsExtraStage2(), GAMESTATE->GetCurrentStyle(PLAYER_INVALID), pSong, pSteps );

		if( find( arraySongs.begin(), arraySongs.end(), pSong ) == arraySongs.end() )
			arraySongs.push_back( pSong );
	}
}

void MusicWheel::BuildWheelItemDatas( vector<MusicWheelItemData *> &arrayWheelItemDatas, SortOrder so )
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
				MusicWheelItemData wid( WheelItemDataType_Sort, nullptr, "", nullptr, SORT_MENU_COLOR, 0 );
				wid.m_pAction = HiddenPtr<GameCommand>( new GameCommand );
				wid.m_pAction->m_sName = vsNames[i];
				wid.m_pAction->Load( i, ParseCommands(CHOICE.GetValue(vsNames[i])) );
				wid.m_sLabel = WHEEL_TEXT( vsNames[i] );

				if( !wid.m_pAction->IsPlayable() )
					continue;

				arrayWheelItemDatas.push_back( new MusicWheelItemData(wid) );
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
		case SORT_BEGINNER_METER:
		case SORT_EASY_METER:
		case SORT_MEDIUM_METER:
		case SORT_HARD_METER:
		case SORT_CHALLENGE_METER:
		case SORT_DOUBLE_EASY_METER:
		case SORT_DOUBLE_MEDIUM_METER:
		case SORT_DOUBLE_HARD_METER:
		case SORT_DOUBLE_CHALLENGE_METER:
		case SORT_LENGTH:
		case SORT_RECENT:
		{
			// Make an array of Song*, then sort them
			vector<Song*> arraySongs;
			GetSongList( arraySongs, so );

			bool bUseSections = true;

			// sort the songs
			switch( so )
			{
				case SORT_PREFERRED:
					// obey order specified by the preferred sort list
					break;
				case SORT_ROULETTE:
				{
					StepsType st;
					Difficulty dc;
					SongUtil::GetStepsTypeAndDifficultyFromSortOrder( SORT_EASY_METER, st, dc );
					SongUtil::SortSongPointerArrayByStepsTypeAndMeter( arraySongs, st, dc );
					if( (bool)PREFSMAN->m_bPreferredSortUsesGroups )
						stable_sort( arraySongs.begin(), arraySongs.end(), SongUtil::CompareSongPointersByGroup );
					bUseSections = false;
					break;
				}
				case SORT_GROUP:
					SongUtil::SortSongPointerArrayByGroupAndTitle( arraySongs );
					if(USE_SECTIONS_WITH_PREFERRED_GROUP)
						bUseSections = true;
					else
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
				case SORT_LENGTH:
					SongUtil::SortSongPointerArrayByLength( arraySongs );
					break;
				case SORT_RECENT:
					SongUtil::SortByMostRecentlyPlayedForMachine( arraySongs );
					if( (int) arraySongs.size() > RECENT_SONGS_TO_SHOW )
						arraySongs.erase( arraySongs.begin()+RECENT_SONGS_TO_SHOW, arraySongs.end() );
					bUseSections = false;
					break;
				case SORT_BEGINNER_METER:
				case SORT_EASY_METER:
				case SORT_MEDIUM_METER:
				case SORT_HARD_METER:
				case SORT_CHALLENGE_METER:
				case SORT_DOUBLE_EASY_METER:
				case SORT_DOUBLE_MEDIUM_METER:
				case SORT_DOUBLE_HARD_METER:
				case SORT_DOUBLE_CHALLENGE_METER:
					StepsType st;
					Difficulty dc;
					SongUtil::GetStepsTypeAndDifficultyFromSortOrder( so, st, dc );
					SongUtil::SortSongPointerArrayByStepsTypeAndMeter( arraySongs, st, dc );
					break;
				default:
					FAIL_M("Unhandled sort order! Aborting...");
			}

			// Build an array of WheelItemDatas from the sorted list of Song*'s
			arrayWheelItemDatas.clear();	// clear out the previous wheel items 
			arrayWheelItemDatas.reserve( arraySongs.size() );

			switch( PREFSMAN->m_MusicWheelUsesSections )
			{
				case MusicWheelUsesSections_NEVER:
					bUseSections = false;
					break;
				case MusicWheelUsesSections_ABC_ONLY:
					if( so != SORT_TITLE && so != SORT_GROUP )
						bUseSections = false;
					break;
				default:
					break;
			}

			if( bUseSections )
			{
				// Sorting twice isn't necessary. Instead, modify the compatator
				// functions in Song.cpp to have the desired effect. -Chris
				/* Keeping groups together with the sorts is tricky and brittle; we
				 * keep getting OTHER split up without this. However, it puts the 
				 * Grade and BPM sorts in the wrong order, and they're already correct,
				 * so don't re-sort for them. */
				/* We're using sections, so use the section name as the top-level sort. */
				switch( so )
				{
					case SORT_PREFERRED:
					case SORT_TOP_GRADES:
					case SORT_BPM:
					case SORT_LENGTH:
						break;	// don't sort by section
					default:
						SongUtil::SortSongPointerArrayBySectionName(arraySongs, so);
						break;
				}
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
						int iSectionCount = 0;
						// Count songs in this section
						unsigned j;
						for( j=i; j < arraySongs.size(); j++ )
						{
							if( SongUtil::GetSectionNameFromSongAndSort( arraySongs[j], so ) != sThisSection )
								break;
						}
						iSectionCount = j-i;

						// new section, make a section item
						// todo: preferred sort section color handling? -aj
						RageColor colorSection = (so==SORT_GROUP) ? SONGMAN->GetSongGroupColor(pSong->m_sGroupName) : SECTION_COLORS.GetValue(iSectionColorIndex);
						iSectionColorIndex = (iSectionColorIndex+1) % NUM_SECTION_COLORS;
						arrayWheelItemDatas.push_back( new MusicWheelItemData(WheelItemDataType_Section, nullptr, sThisSection, nullptr, colorSection, iSectionCount) );
						sLastSection = sThisSection;
					}
				}
				arrayWheelItemDatas.push_back( new MusicWheelItemData(WheelItemDataType_Song, pSong, sLastSection, nullptr, SONGMAN->GetSongColor(pSong), 0) );
			}

			if( so != SORT_ROULETTE )
			{
				// todo: allow themers to change the order of the items. -aj
				if( SHOW_ROULETTE )
					arrayWheelItemDatas.push_back( new MusicWheelItemData(WheelItemDataType_Roulette, nullptr, "", nullptr, ROULETTE_COLOR, 0) );

				// Only add WheelItemDataType_Random and WheelItemDataType_Portal if there's at least
				// one song on the list.
				bool bFoundAnySong = false;
				for( unsigned i=0; !bFoundAnySong && i < arrayWheelItemDatas.size(); i++ )
					if( arrayWheelItemDatas[i]->m_Type == WheelItemDataType_Song )
						bFoundAnySong = true;

				if( SHOW_RANDOM && bFoundAnySong )
					arrayWheelItemDatas.push_back( new MusicWheelItemData(WheelItemDataType_Random, nullptr, "", nullptr, RANDOM_COLOR, 0) );

				if( SHOW_PORTAL && bFoundAnySong )
					arrayWheelItemDatas.push_back( new MusicWheelItemData(WheelItemDataType_Portal, nullptr, "", nullptr, PORTAL_COLOR, 0) );

				// add custom wheel items
				vector<RString> vsNames;
				split( CUSTOM_WHEEL_ITEM_NAMES, ",", vsNames );
				for( unsigned i=0; i<vsNames.size(); ++i )
				{
					MusicWheelItemData wid( WheelItemDataType_Custom, nullptr, "", nullptr, CUSTOM_CHOICE_COLORS.GetValue(vsNames[i]), 0 );
					wid.m_pAction = HiddenPtr<GameCommand>( new GameCommand );
					wid.m_pAction->m_sName = vsNames[i];
					wid.m_pAction->Load( i, ParseCommands(CUSTOM_CHOICES.GetValue(vsNames[i])) );
					wid.m_sLabel = CUSTOM_ITEM_WHEEL_TEXT( vsNames[i] );

					if( !wid.m_pAction->IsPlayable() )
						continue;

					arrayWheelItemDatas.push_back( new MusicWheelItemData(wid) );
				}
			}

			if( GAMESTATE->IsAnExtraStageAndSelectionLocked() )
			{
				Song* pSong;
				Steps* pSteps;
				SONGMAN->GetExtraStageInfo( GAMESTATE->IsExtraStage2(), GAMESTATE->GetCurrentStyle(PLAYER_INVALID), pSong, pSteps );
				
				for( unsigned i=0; i<arrayWheelItemDatas.size(); i++ )
				{
					if( arrayWheelItemDatas[i]->m_pSong == pSong )
					{
						// Change the song color.
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
			bool bOnlyPreferred = PREFSMAN->m_CourseSortOrder == COURSE_SORT_PREFERRED;

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
				FOREACH_ENUM( CourseType, i )
					vct.push_back( i );
				break;
			default:
				FAIL_M(ssprintf("Wrong sort order: %i", so));
			}

			vector<Course*> apCourses;
			for (CourseType const &ct : vct)
			{
				if( bOnlyPreferred )
					SONGMAN->GetPreferredSortCourses( ct, apCourses, PREFSMAN->m_bAutogenGroupCourses );
				else
					SONGMAN->GetCourses( ct, apCourses, PREFSMAN->m_bAutogenGroupCourses );
			}

			switch( PREFSMAN->m_CourseSortOrder )
			{
				case COURSE_SORT_SONGS:
					CourseUtil::SortCoursePointerArrayByDifficulty( apCourses );
					break;
				case COURSE_SORT_PREFERRED:
					break;
				case COURSE_SORT_METER:
					CourseUtil::SortCoursePointerArrayByAvgDifficulty( apCourses );
					break;
				case COURSE_SORT_METER_SUM:
					CourseUtil::SortCoursePointerArrayByTotalDifficulty( apCourses );
					break;
				case COURSE_SORT_RANK:
					CourseUtil::SortCoursePointerArrayByRanking( apCourses );
					break;
				default:	FAIL_M("Impossible to sort the courses! Aborting...");
			}

			// since we can't agree, make it an option
			if( PREFSMAN->m_CourseSortOrder != COURSE_SORT_SONGS && g_bMoveRandomToEnd )
				CourseUtil::MoveRandomToEnd( apCourses );

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
						default: break;
					}
				}

				// check that this course has at least one song playable in the current style
				if( !pCourse->IsPlayableIn(GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType) )
					continue;

				if( sThisSection != sLastSection )	// new section, make a section item
				{
					RageColor c = SECTION_COLORS.GetValue(iSectionColorIndex);
					iSectionColorIndex = (iSectionColorIndex+1) % NUM_SECTION_COLORS;
					arrayWheelItemDatas.push_back( new MusicWheelItemData(WheelItemDataType_Section, nullptr, sThisSection, nullptr, c, 0) );
					sLastSection = sThisSection;
				}

				RageColor c = ( pCourse->m_sGroupName.size() == 0 ) ? pCourse->GetColor() : SONGMAN->GetCourseColor(pCourse);
				arrayWheelItemDatas.push_back( new MusicWheelItemData(WheelItemDataType_Course, nullptr, sThisSection, pCourse, c, 0) );
			}
			break;
		}
		default:
			break;
	}

	// init music status icons
	for (MusicWheelItemData *WID : arrayWheelItemDatas)
	{
		if( WID->m_pSong != nullptr )
		{
			WID->m_Flags.bHasBeginnerOr1Meter = WID->m_pSong->IsEasy( GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType ) && SHOW_EASY_FLAG;
			WID->m_Flags.bEdits = false;
			set<StepsType> vStepsType;
			SongUtil::GetPlayableStepsTypes( WID->m_pSong, vStepsType );
			for (StepsType const &type : vStepsType)
				WID->m_Flags.bEdits |= WID->m_pSong->HasEdits( type );
			WID->m_Flags.iStagesForSong = GameState::GetNumStagesMultiplierForSong( WID->m_pSong );
		}
		else if( WID->m_pCourse != nullptr )
		{
			WID->m_Flags.bHasBeginnerOr1Meter = false;
			WID->m_Flags.bEdits = WID->m_pCourse->IsAnEdit();
			WID->m_Flags.iStagesForSong = 1;
		}
	}
}

vector<MusicWheelItemData *> & MusicWheel::getWheelItemsData(SortOrder so) {
	// Update the popularity and init icons.
	readyWheelItemsData(so);	
	return m__WheelItemDatas[so];
}

void MusicWheel::readyWheelItemsData(SortOrder so) {
	if(m_WheelItemDatasStatus[so]!=VALID) {
		RageTimer timer;

		vector<MusicWheelItemData *> &aUnFilteredDatas=m__UnFilteredWheelItemDatas[so];

		if(m_WheelItemDatasStatus[so]==INVALID) {
			BuildWheelItemDatas(  aUnFilteredDatas, so );
		}
		FilterWheelItemDatas( aUnFilteredDatas, m__WheelItemDatas[so], so );
		m_WheelItemDatasStatus[so]=VALID;

		LOG->Trace( "MusicWheel sorting took: %f", timer.GetTimeSinceStart() );
	}

}

void MusicWheel::FilterWheelItemDatas(vector<MusicWheelItemData *> &aUnFilteredDatas, vector<MusicWheelItemData *> &aFilteredData, SortOrder so )
{
	aFilteredData.clear();

	unsigned unfilteredSize=aUnFilteredDatas.size();

	/* Only add WheelItemDataType_Portal if there's at least one song on the list. */
	bool bFoundAnySong = false;
	for( unsigned i=0; i < unfilteredSize; i++ ) {
		if( aUnFilteredDatas[i]->m_Type == WheelItemDataType_Song ) {
			bFoundAnySong = true;
			break;
		}
	}

	vector<bool> aiRemove;
	aiRemove.insert( aiRemove.begin(), unfilteredSize, false );

	const int iMaxStagesForSong = GAMESTATE->GetSmallestNumStagesLeftForAnyHumanPlayer();

	Song *pExtraStageSong = nullptr;
	if( GAMESTATE->IsAnExtraStage() )
	{
		Steps *pSteps;
		SONGMAN->GetExtraStageInfo( GAMESTATE->IsExtraStage2(), GAMESTATE->GetCurrentStyle(PLAYER_INVALID), pExtraStageSong, pSteps );
	}

	/* Mark any songs that aren't playable in aiRemove. */

	for( unsigned i=0; i< unfilteredSize; i++ )
	{
		MusicWheelItemData& WID = *aUnFilteredDatas[i];

		/* If we have no songs, remove Random and Portal. */
		if( WID.m_Type == WheelItemDataType_Random || WID.m_Type == WheelItemDataType_Portal )
		{
			if( !bFoundAnySong )
				aiRemove[i] = true;
			continue;
		}

		/* Filter songs that we don't have enough stages to play. */
		if( WID.m_Type == WheelItemDataType_Song )
		{
			Song* pSong = WID.m_pSong;

			/* Never remove the extra stage song. */
			if( pExtraStageSong && WID.m_pSong == pExtraStageSong )
				continue;

			/* Check that we have enough stages to play this song. */
			if( GAMESTATE->GetNumStagesMultiplierForSong(WID.m_pSong) > iMaxStagesForSong )
			{
				aiRemove[i] = true;
				continue;
			}

			int iLocked = UNLOCKMAN->SongIsLocked( pSong );
			if( iLocked & LOCKED_DISABLED )
			{
				aiRemove[i] = true;
				continue;
			}

			/* If we're on an extra stage, and this song is selected, ignore #SELECTABLE. */
			if( pSong != GAMESTATE->m_pCurSong || !GAMESTATE->IsAnExtraStage() )
			{
				/* Hide songs that asked to be hidden via #SELECTABLE. */
				if( iLocked & LOCKED_SELECTABLE )
				{
					aiRemove[i] = true;
					continue;
				}
				if( so != SORT_ROULETTE && iLocked & LOCKED_ROULETTE )
				{
					aiRemove[i] = true;
					continue;
				}
			}

			/* Hide locked songs.  If RANDOM_PICKS_LOCKED_SONGS, hide in Roulette and Random,
			 * too. */
			if( (so!=SORT_ROULETTE || !RANDOM_PICKS_LOCKED_SONGS) && iLocked )
			{
				aiRemove[i] = true;
				continue;
			}

			/* If the song has no steps for the current style, remove it. */
			if( !CommonMetrics::AUTO_SET_STYLE && !pSong->HasStepsType(GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType) )
			{
				aiRemove[i] = true;
				continue;
			}
			
			// if AutoSetStyle, make sure the song is playable in the end.
			if (!SongUtil::IsSongPlayable(pSong))
			{
				aiRemove[i] = true;
				continue;
			}
		}

		if( WID.m_Type == WheelItemDataType_Course )
		{
			if( !WID.m_pCourse->IsPlayableIn(GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType) )
				aiRemove[i] = true;
		}
	}

	/* Filter out the songs we're removing. */
	 
	aFilteredData.reserve( unfilteredSize );
	for( unsigned i=0; i< unfilteredSize; i++ )
	{
		if( aiRemove[i] )
			continue;
		aFilteredData.push_back( aUnFilteredDatas[i] );
	}

	// Update the song count in each section header.
	unsigned filteredSize=aFilteredData.size();
	for( unsigned i=0; i < filteredSize; )
	{
		MusicWheelItemData& WID = *aFilteredData[i];
		++i;
		if( WID.m_Type != WheelItemDataType_Section )
			continue;

		// Count songs in this section
		WID.m_iSectionCount = 0;
		for( ; i < filteredSize && aFilteredData[i]->m_sText == WID.m_sText; ++i )
			++WID.m_iSectionCount;
	}

	// If we have any section headers with no songs, then we filtered all of the songs in that group,
	// so remove it.  This isn't optimized like the above since this is a rare case.
	for( unsigned i=0; i < filteredSize; ++i )
	{
		MusicWheelItemData& WID = *aFilteredData[i];
		if( WID.m_Type != WheelItemDataType_Section )
			continue;
		if( WID.m_iSectionCount > 0 )
			continue;
		aFilteredData.erase( aFilteredData.begin()+i, aFilteredData.begin()+i+1 );
		--i;
		--filteredSize;
	}

	/* Update the popularity.  This is affected by filtering. */
	if( so == SORT_POPULARITY )
	{
		for( unsigned i=0; i< min(3u,aFilteredData.size()); i++ )
		{
			MusicWheelItemData& WID = *aFilteredData[i];
			WID.m_Flags.iPlayersBestNumber = i+1;
		}
	}

	// If we've filtered all items, insert a dummy.
	if( aFilteredData.empty() )
		aFilteredData.push_back( new MusicWheelItemData(WheelItemDataType_Section, nullptr, EMPTY_STRING, nullptr, EMPTY_COLOR, 0) );
}

void MusicWheel::UpdateSwitch()
{
	switch( m_WheelState )
	{
	case STATE_FLYING_OFF_BEFORE_NEXT_SORT:
		{
			const Song* pPrevSelectedSong = GetCurWheelItemData(m_iSelection)->m_pSong;

			SCREENMAN->PostMessageToTopScreen( SM_SortOrderChanged, 0 );

			SetOpenSection( SongUtil::GetSectionNameFromSongAndSort(pPrevSelectedSong, GAMESTATE->m_SortOrder) );

			m_iSelection = 0;

			// Select the previously selected item
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

			// Change difficulty for sorts by meter
			// XXX: do this with GameCommand?
			StepsType st;
			Difficulty dc;
			if( SongUtil::GetStepsTypeAndDifficultyFromSortOrder( GAMESTATE->m_SortOrder, st, dc ) )
			{
				ASSERT( dc != Difficulty_Invalid );
				FOREACH_PlayerNumber( p )
					if( GAMESTATE->IsPlayerEnabled(p) )
						GAMESTATE->m_PreferredDifficulty[p].Set( dc );
			}

			SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );
			RebuildWheelItems();
			TweenOnScreenForSort();
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

			// Send this again so the screen starts sample music.
			SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );
			MESSAGEMAN->Broadcast("RouletteStopped");
		}
		else
		{
			--m_iSwitchesLeftInSpinDown;
			const float SwitchTimes[] = { 0.5f, 1.3f, 0.8f, 0.4f, 0.2f };
			ASSERT( m_iSwitchesLeftInSpinDown >= 0 && m_iSwitchesLeftInSpinDown <= 4 );
			m_fTimeLeftInState = SwitchTimes[m_iSwitchesLeftInSpinDown];
			m_Moving = 0;

			LOG->Trace( "m_iSwitchesLeftInSpinDown id %d, m_fTimeLeftInState is %f", m_iSwitchesLeftInSpinDown, m_fTimeLeftInState );

			if( m_iSwitchesLeftInSpinDown == 0 )
				ChangeMusic( randomf(0,1) >= 0.5f? 1:-1 );
			else
				ChangeMusic( 1 );
		}
		break;
	default:
		FAIL_M(ssprintf("Invalid wheel state: %i", m_WheelState));
	}
}

void MusicWheel::ChangeMusic( int iDist )
{
	m_iSelection += iDist;
	wrap( m_iSelection, m_CurWheelItemData.size() );

	if( REMIND_WHEEL_POSITIONS && HIDE_INACTIVE_SECTIONS )
	{
		// store the group song index
		for( unsigned idx = 0 ; idx < m_viWheelPositions.size() ; idx++ )
		{
			if( m_sExpandedSectionName == SONGMAN->GetSongGroupByIndex(idx) )
			{
				m_viWheelPositions[idx] = m_iSelection;
			}
		}
	}

	RebuildWheelItems( iDist );

	m_fPositionOffsetFromSelection += iDist;

	SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );

	// If we're moving automatically, don't play this; it'll be called in Update.
	if(!IsMoving())
		m_soundChangeMusic.Play(true);
}


bool MusicWheel::ChangeSort( SortOrder new_so, bool allowSameSort )	// return true if change successful
{
	ASSERT( new_so < NUM_SortOrder );
	if( GAMESTATE->m_SortOrder == new_so && !allowSameSort )
	{
		return false;
	}

	// Don't change to SORT_MODE_MENU if it doesn't have at least two choices.
	if( new_so == SORT_MODE_MENU && getWheelItemsData(new_so).size() < 2 )
	{
		return false;
	}

	switch( m_WheelState )
	{
	case STATE_SELECTING:
	case STATE_FLYING_ON_AFTER_NEXT_SORT:
		break;	// fall through
	default:
		return false;	// don't continue
	}

	SCREENMAN->PostMessageToTopScreen( SM_SortOrderChanging, 0 );

	m_soundChangeSort.Play(true);

	TweenOffScreenForSort();

	// Save the new preference.
	if( IsSongSort(new_so) )
		GAMESTATE->m_PreferredSortOrder = new_so;
	GAMESTATE->m_SortOrder.Set( new_so );

	return true;
}

bool MusicWheel::NextSort()		// return true if change successful
{
	// don't allow NextSort when on the mode menu
	if( GAMESTATE->m_SortOrder == SORT_MODE_MENU )
		return false;

	vector<SortOrder> aSortOrders;
	{
		Lua *L = LUA->Get();
		SORT_ORDERS.PushSelf( L );
		FOREACH_LUATABLEI( L, -1, i )
		{
			SortOrder so = Enum::Check<SortOrder>( L, -1, true );
			aSortOrders.push_back( so );
		}
		lua_pop( L, 1 );
		LUA->Release(L);
	}

	// find the index of the current sort
	int cur = 0;
	while( cur < int(aSortOrders.size()) && aSortOrders[cur] != GAMESTATE->m_SortOrder )
		++cur;

	// move to the next sort with wrapping
	++cur;
	wrap( cur, aSortOrders.size() );

	// apply new sort
	SortOrder soNew = aSortOrders[cur];
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
		case STATE_ROULETTE_SPINNING:
			m_WheelState = STATE_ROULETTE_SLOWING_DOWN;
			m_iSwitchesLeftInSpinDown = ROULETTE_SLOW_DOWN_SWITCHES/2+1 + RandomInt( ROULETTE_SLOW_DOWN_SWITCHES/2 );
			m_fTimeLeftInState = 0.1f;
			return false;
		case STATE_RANDOM_SPINNING:
			m_fPositionOffsetFromSelection = max(m_fPositionOffsetFromSelection, 0.3f);
			m_WheelState = STATE_LOCKED;
			SCREENMAN->PlayStartSound();
			m_fLockedWheelVelocity = 0;
			// Set m_Moving to zero to stop the sounds from playing.
			m_Moving = 0;
			SCREENMAN->PostMessageToTopScreen( SM_SongChanged, 0 );
			return true;
		default: break;
	}

	if( !WheelBase::Select() )
		return false;

	switch( m_CurWheelItemData[m_iSelection]->m_Type )
	{
		case WheelItemDataType_Roulette:  
			StartRoulette();
			return false;
		case WheelItemDataType_Random:
			StartRandom();
			return false;
		case WheelItemDataType_Sort:
			GetCurWheelItemData(m_iSelection)->m_pAction->ApplyToAllPlayers();
			ChangeSort( GAMESTATE->m_PreferredSortOrder );
			m_sLastModeMenuItem = GetCurWheelItemData(m_iSelection)->m_pAction->m_sName;
			return false;
		case WheelItemDataType_Custom:
			GetCurWheelItemData(m_iSelection)->m_pAction->ApplyToAllPlayers();
			if( GetCurWheelItemData(m_iSelection)->m_pAction->m_sScreen != "" )
				return true;
			else
				return false;
		default: return true;
	}
}

void MusicWheel::StartRoulette() 
{
	MESSAGEMAN->Broadcast("StartRoulette");
	m_WheelState = STATE_ROULETTE_SPINNING;
	m_Moving = 1;
	m_TimeBeforeMovingBegins = 0;
	m_SpinSpeed = 1.0f/ROULETTE_SWITCH_SECONDS;
	GAMESTATE->m_SortOrder.Set( SORT_ROULETTE );
	SetOpenSection( "" );
	RebuildWheelItems();
}

void MusicWheel::StartRandom()
{
	MESSAGEMAN->Broadcast("StartRandom");
	/* If RANDOM_PICKS_LOCKED_SONGS is disabled, pick a song from the active sort and
	 * section.  If enabled, picking from the section makes it too easy to trick the
	 * game into picking a locked song, so pick from SORT_ROULETTE. */
	if( RANDOM_PICKS_LOCKED_SONGS )
	{
		// Shuffle and use the roulette wheel.
		RandomGen rnd;
		random_shuffle( getWheelItemsData(SORT_ROULETTE).begin(), getWheelItemsData(SORT_ROULETTE).end(), rnd );
		GAMESTATE->m_SortOrder.Set( SORT_ROULETTE );
	}
	else
	{
		GAMESTATE->m_SortOrder.Set( GAMESTATE->m_PreferredSortOrder );
	}
	SetOpenSection( "" );

	m_Moving = -1;
	m_TimeBeforeMovingBegins = 0;
	m_SpinSpeed = 1.0f/ROULETTE_SWITCH_SECONDS;
	m_SpinSpeed *= 20.0f; /* faster! */
	m_WheelState = STATE_RANDOM_SPINNING;

	SelectSong( GetPreferredSelectionForRandomOrPortal() );

	RebuildWheelItems();
}

void MusicWheel::SetOpenSection( RString group )
{
	//LOG->Trace( "SetOpenSection %s", group.c_str() );
	m_sExpandedSectionName = group;
	GAMESTATE->sExpandedSectionName = group;

	// wheel positions = num song groups
	if ( REMIND_WHEEL_POSITIONS && HIDE_INACTIVE_SECTIONS )
		m_viWheelPositions.resize( SONGMAN->GetNumSongGroups() );

	const WheelItemBaseData *old = nullptr;
	if( !m_CurWheelItemData.empty() )
		old = GetCurWheelItemData(m_iSelection);

	vector<const Style*> vpPossibleStyles;
	if( CommonMetrics::AUTO_SET_STYLE )
		GAMEMAN->GetCompatibleStyles( GAMESTATE->m_pCurGame, GAMESTATE->GetNumPlayersEnabled(), vpPossibleStyles );

	m_CurWheelItemData.clear();
	vector<MusicWheelItemData *> &from = getWheelItemsData(GAMESTATE->m_SortOrder);
	m_CurWheelItemData.reserve( from.size() );
	for( unsigned i = 0; i < from.size(); ++i )
	{
		MusicWheelItemData &d = *from[i];

		// Hide songs/courses which are not in the active section
		if( (d.m_Type == WheelItemDataType_Song || d.m_Type == WheelItemDataType_Course) && !d.m_sText.empty() &&
			 d.m_sText != group )
			 continue;

		// In certain situations (e.g. simulating Pump it Up or IIDX),
		// themes may want to hide inactive section headings as well.
		if( HIDE_INACTIVE_SECTIONS && d.m_Type == WheelItemDataType_Section && group != "" ) {
			// Based on the HideActiveSectionTitle metric, we either
			// hide all section titles, or only those which are not
			// currently open.
			if ( HIDE_ACTIVE_SECTION_TITLE || d.m_sText != group )
				continue;
		}

		// If AUTO_SET_STYLE, hide courses that prefer a style that isn't available.
		if( d.m_Type == WheelItemDataType_Course && CommonMetrics::AUTO_SET_STYLE )
		{
			const Style *pStyle = d.m_pCourse->GetCourseStyle( GAMESTATE->m_pCurGame, GAMESTATE->GetNumSidesJoined() );
			if( pStyle )
			{
				if( find( vpPossibleStyles.begin(), vpPossibleStyles.end(), pStyle ) == vpPossibleStyles.end() )
					continue;
			}
		}

		// Only show tutorial songs in arcade
		if( GAMESTATE->m_PlayMode!=PLAY_MODE_REGULAR && 
			d.m_pSong &&
			d.m_pSong->IsTutorial() )
			continue;

		m_CurWheelItemData.push_back(&d);
	}

	//restore the past group song index
	if( REMIND_WHEEL_POSITIONS && HIDE_INACTIVE_SECTIONS )
	{
		for( unsigned idx = 0 ; idx < m_viWheelPositions.size() ; idx++ )
		{
			if( m_sExpandedSectionName == SONGMAN->GetSongGroupByIndex(idx) )
			{
				m_iSelection = m_viWheelPositions[idx];
			}
		}
	}
	else
	{
		// Try to select the item that was selected before changing groups
		m_iSelection = 0;

		for( unsigned i=0; i<m_CurWheelItemData.size(); i++ )
		{
			if( m_CurWheelItemData[i] == old )
			{
				m_iSelection=i;
				break;
			}
		}
	}

	RebuildWheelItems();
}

void MusicWheel::GetCurrentSections(vector<RString> &sections)
{
	vector<MusicWheelItemData *> &wiWheelItems = getWheelItemsData(GAMESTATE->m_SortOrder);
	for( unsigned i = 0; i < wiWheelItems.size(); i++ )
	{
		if ( wiWheelItems[i]->m_Type == WheelItemDataType_Section && !wiWheelItems[i]->m_sText.empty())
			sections.push_back(wiWheelItems[i]->m_sText);
	}
}

// sm-ssc additions: jump to group
RString MusicWheel::JumpToNextGroup()
{
	// Thanks to Juanelote for this logic:
	if( HIDE_INACTIVE_SECTIONS )
	{
		//todo: make it work with other sort types
		unsigned iNumGroups = SONGMAN->GetNumSongGroups();

		for(unsigned i = 0 ; i < iNumGroups ; i++)
		{
			if( m_sExpandedSectionName == SONGMAN->GetSongGroupByIndex(i) )
			{
				if ( i < iNumGroups - 1 )
					return SONGMAN->GetSongGroupByIndex(i+1);
				else
				{
					//i = 0;
					return SONGMAN->GetSongGroupByIndex(0);
				}
			}
		}
	}
	else
	{
		unsigned int iLastSelection = m_iSelection;
		for( unsigned int i = m_iSelection; i < m_CurWheelItemData.size(); ++i )
		{
			if( m_CurWheelItemData[i]->m_Type == WheelItemDataType_Section && i != (unsigned int)m_iSelection )
			{
				m_iSelection = i;
				return m_CurWheelItemData[i]->m_sText;
			}
		}
		// it should not get down here, but it might happen... only search up to
		// the previous selection.
		for( unsigned int i = 0; i < iLastSelection; ++i )
		{
			if( m_CurWheelItemData[i]->m_Type == WheelItemDataType_Section && i != (unsigned int)m_iSelection )
			{
				m_iSelection = i;
				return m_CurWheelItemData[i]->m_sText;
			}
		}
	}
	// it shouldn't get here, but just in case...
	return "";
}

RString MusicWheel::JumpToPrevGroup()
{
	if( HIDE_INACTIVE_SECTIONS )
	{
		unsigned iNumGroups = SONGMAN->GetNumSongGroups();

		for(unsigned i = 0 ; i < iNumGroups ; i++)
		{
			if( m_sExpandedSectionName == SONGMAN->GetSongGroupByIndex(i) )
			{
				if ( i > 0 )
					return SONGMAN->GetSongGroupByIndex(i-1);
				else
				{
					//i = iNumGroups - 1;
					return SONGMAN->GetSongGroupByIndex(iNumGroups - 1);
				}
			}
		}
	}
	else
	{
		for( unsigned int i = m_iSelection; i > 0; --i )
		{
			if( m_CurWheelItemData[i]->m_Type == WheelItemDataType_Section && i != (unsigned int)m_iSelection )
			{
				m_iSelection = i;
				return m_CurWheelItemData[i]->m_sText;
			}
		}
		// in case it wasn't found above:
		for( unsigned int i = m_CurWheelItemData.size()-1; i > 0; --i )
		{
			LOG->Trace( "JumpToPrevGroup iteration 2 | i = %u",i );
			if( m_CurWheelItemData[i]->m_Type == WheelItemDataType_Section )
			{
				m_iSelection = i;
				LOG->Trace( "finding it in #2 | i = %u | text = %s",i, m_CurWheelItemData[i]->m_sText.c_str() );
				return m_CurWheelItemData[i]->m_sText;
			}
		}
	}
	// it shouldn't get here, but just in case...
	return "";
}

// Called on late join. Selectable courses may have changed; reopen the section.
void MusicWheel::PlayerJoined()
{
	// If someone joins, there may be songs on the wheel that should not be
	// selectable, or there may be songs that have become selectable.
	// Set the status of all the wheel item data vectors to invalid so that
	// readyWheelItemsData will rebuild all the data next time
	// getWheelItemsData is called for that SortOrder.  SetOpenSection calls
	// readyWheelItemsData to get the items, and RebuildWheelItems when its
	// done, so invalidating and calling SetOpenSection is all we need to do.
	// -Kyz
	// Also removed the weird checks for course mode and autogen because
	// it seems weird that courses wouldn't also be affected by a player
	// joining, and not doing it in autogen causes other weird problems. -Kyz
	FOREACH_ENUM(SortOrder, so)
	{
		m_WheelItemDatasStatus[so] = INVALID;
	}
	SetOpenSection(m_sExpandedSectionName);
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
		case WheelItemDataType_Portal:
			return GetPreferredSelectionForRandomOrPortal();
		default:
			return GetCurWheelItemData(m_iSelection)->m_pSong;
	}
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
		if( GAMESTATE->m_PreferredDifficulty[p] == Difficulty_Invalid )
			continue;	// skip

		// TRICKY: Don't require that edits be present if perferred 
		// difficulty is Difficulty_Edit.  Otherwise, players could use this 
		// to set up a 100% chance of getting a particular locked song by 
		// having a single edit for a locked song.
		if( GAMESTATE->m_PreferredDifficulty[p] == Difficulty_Edit )
			continue;	// skip

		vDifficultiesToRequire.push_back( GAMESTATE->m_PreferredDifficulty[p] );
	}

	RString sPreferredGroup = m_sExpandedSectionName;
	vector<MusicWheelItemData *> &wid = getWheelItemsData(GAMESTATE->m_SortOrder);

	StepsType st = GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType;

#define NUM_PROBES 1000
	for( int i=0; i<NUM_PROBES; i++ )
	{
		bool isValid = true;
		/* Maintaining difficulties is higher priority than maintaining
		 * the current group. */
		if( i == NUM_PROBES/4 )
			sPreferredGroup = "";
		if( i == NUM_PROBES/2 )
			vDifficultiesToRequire.clear();

		int iSelection = RandomInt( wid.size() );
		if( wid[iSelection]->m_Type != WheelItemDataType_Song )
			continue;

		const Song *pSong = wid[iSelection]->m_pSong;

		if( !sPreferredGroup.empty() && wid[iSelection]->m_sText != sPreferredGroup )
			continue;

		// There's an off possibility that somebody might have only one song with only beginner steps.
		if( i < 900 && pSong->IsTutorial() )
			continue;

		isValid = std::none_of(vDifficultiesToRequire.begin(), vDifficultiesToRequire.end(), [&](Difficulty const &d) {
			return !pSong->HasStepsTypeAndDifficulty(st, d);
		});

		if (isValid)
		{
			return wid[iSelection]->m_pSong;
		}
	}
	LuaHelpers::ReportScriptError( "Couldn't find any songs" );
	return wid[0]->m_pSong;
}

void MusicWheel::FinishChangingSorts()
{
	FinishTweening();
	m_WheelState = STATE_SELECTING;
	m_fTimeLeftInState = 0;
}

// lua start
#include "LuaBinding.h"

class LunaMusicWheel: public Luna<MusicWheel>
{
public:
	static int ChangeSort( T* p, lua_State *L )
	{
		if( lua_isnil(L,1) ) { lua_pushboolean( L, false ); }
		else
		{
			SortOrder so = Enum::Check<SortOrder>(L, 1);
			lua_pushboolean( L, p->ChangeSort( so ) );
		}
		return 1;
	}
	DEFINE_METHOD(GetSelectedSection, GetSelectedSection());
	static int GetCurrentSections( T* p, lua_State *L )
	{
		vector<RString> v;
		p->GetCurrentSections(v);
		LuaHelpers::CreateTableFromArray<RString>( v, L );
		return 1;
	}
	static int IsRouletting( T* p, lua_State *L ){ lua_pushboolean( L, p->IsRouletting() ); return 1; }
	static int SelectSong( T* p, lua_State *L )
	{
		if( lua_isnil(L,1) ) { lua_pushboolean( L, false ); }
		else
		{
			Song *pS = Luna<Song>::check( L, 1, true );
			lua_pushboolean( L, p->SelectSong( pS ) );
		}
		return 1;
	}
	static int SelectCourse( T* p, lua_State *L )
	{
		if( lua_isnil(L,1) ) { lua_pushboolean( L, false ); }
		else
		{
			Course *pC = Luna<Course>::check( L, 1, true );
			lua_pushboolean( L, p->SelectCourse( pC ) );
		}
		return 1;
	}

	static int Move(T* p, lua_State *L)
	{
		if (lua_isnil(L, 1)) { p->Move(0); }
		else
		{
			p->Move(IArg(1));
		}
		return 1;
	}

	LunaMusicWheel()
	{
		ADD_METHOD( ChangeSort );
		ADD_METHOD( GetSelectedSection );
		ADD_METHOD( IsRouletting );
		ADD_METHOD( SelectSong );
		ADD_METHOD( SelectCourse );
		ADD_METHOD( Move );
		ADD_METHOD( GetCurrentSections );
	}
};

LUA_REGISTER_DERIVED_CLASS( MusicWheel, WheelBase )
// lua end

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
