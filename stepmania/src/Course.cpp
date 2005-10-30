#include "global.h"
#include "Course.h"
#include "PrefsManager.h"
#include "song.h"
#include "GameManager.h"
#include "SongManager.h"
#include "SongUtil.h"
#include "GameState.h"
#include "RageException.h"
#include "RageLog.h"
#include "PlayerOptions.h"
#include "SongOptions.h"
#include "SongCacheIndex.h"
#include "RageUtil.h"
#include "TitleSubstitution.h"
#include "Steps.h"
#include "BannerCache.h"
#include "ThemeManager.h"
#include "ProfileManager.h"
#include "Foreach.h"
#include "UnlockManager.h"
#include <limits.h>
#include "CourseLoaderCRS.h"
#include "LuaFunctions.h"


static const CString CourseTypeNames[] = {
	"Nonstop",
	"Oni",
	"Endless",
	"Survival",
};
XToString( CourseType, NUM_CourseType );
XToThemedString( CourseType, NUM_CourseType );

LuaFunction( CourseTypeToThemedString, CourseTypeToThemedString((CourseType) IArg(1)) );


static const CString SongSortNames[] = {
	"Randomize",
	"MostPlays",
	"FewestPlays",
	"TopGrades",
	"LowestGrades",
};
XToString( SongSort, NUM_SongSort );
XToThemedString( SongSort, NUM_SongSort );


/* Maximum lower value of ranges when difficult: */
const int MAX_BOTTOM_RANGE = 10;

#define SORT_LEVEL1_COLOR		THEME->GetMetricC("Course","SortLevel1Color")
#define SORT_LEVEL2_COLOR		THEME->GetMetricC("Course","SortLevel2Color")
#define SORT_LEVEL3_COLOR		THEME->GetMetricC("Course","SortLevel3Color")
#define SORT_LEVEL4_COLOR		THEME->GetMetricC("Course","SortLevel4Color")
#define SORT_LEVEL5_COLOR		THEME->GetMetricC("Course","SortLevel5Color")


CString CourseEntry::GetTextDescription() const
{
	vector<CString> vsEntryDescription;
	if( pSong )
		vsEntryDescription.push_back( pSong->GetTranslitFullTitle() ); 
	else
		vsEntryDescription.push_back( "Random" );
	if( !sSongGroup.empty() )
		vsEntryDescription.push_back( sSongGroup );
	if( baseDifficulty != DIFFICULTY_INVALID && baseDifficulty != DIFFICULTY_MEDIUM )
		vsEntryDescription.push_back( DifficultyToThemedString(baseDifficulty) );
	if( iLowMeter != -1 )
		vsEntryDescription.push_back( ssprintf("Low meter: %d", iLowMeter) );
	if( iHighMeter != -1 )
		vsEntryDescription.push_back( ssprintf("High meter: %d", iHighMeter) );
	if( songSort != SongSort_Randomize )
		vsEntryDescription.push_back( "Sort: %d" + SongSortToThemedString(songSort) );
	if( songSort != SongSort_Randomize && iChooseIndex != 0 )
		vsEntryDescription.push_back( "Choose " + FormatNumberAndSuffix(iChooseIndex) + " match" );
	int iNumModChanges = GetNumModChanges();
	if( iNumModChanges != 0 )
		vsEntryDescription.push_back( ssprintf("%d mod changes", iNumModChanges) );
	if( fGainSeconds != 0 )
		vsEntryDescription.push_back( ssprintf("Low meter: %.0f", fGainSeconds) );

	CString s = join( ",", vsEntryDescription );
	return s;
}

int CourseEntry::GetNumModChanges() const
{
	int iNumModChanges = 0;
	if( !sModifiers.empty() )
		iNumModChanges++;
	iNumModChanges += attacks.size();
	return iNumModChanges;
}

// lua start
#include "LuaBinding.h"

class LunaCourseEntry: public Luna<CourseEntry>
{
public:
	LunaCourseEntry() { LUA->Register( Register ); }

	static int GetSong( T* p, lua_State *L )
	{
		if( p->pSong )
			p->pSong->PushSelf(L);
		else
			lua_pushnil(L);
		return 1;
	}

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetSong );

		Luna<T>::Register( L );
	}
};

LUA_REGISTER_CLASS( CourseEntry )
// lua end




Course::Course()
{
	Init();
}

CourseType Course::GetCourseType() const
{
	if( m_bRepeat )
		return COURSE_TYPE_ENDLESS;
	if( m_iLives > 0 ) 
		return COURSE_TYPE_ONI;
	if( !m_vEntries.empty()  &&  m_vEntries[0].fGainSeconds > 0 )
		return COURSE_TYPE_SURVIVAL;
	return COURSE_TYPE_NONSTOP;
}

void Course::SetCourseType( CourseType ct )
{
	if( GetCourseType() == ct )
		return;

	m_bRepeat = false;
	m_iLives = -1;
	if( !m_vEntries.empty() )
		m_vEntries[0].fGainSeconds = 0;
	
	switch( ct )
	{
	default:	ASSERT(0);
	case COURSE_TYPE_NONSTOP:
		break;		
	case COURSE_TYPE_ONI:
		m_iLives = 4;
		break;		
	case COURSE_TYPE_ENDLESS:
		m_bRepeat = true;
		break;		
	case COURSE_TYPE_SURVIVAL:
		if( !m_vEntries.empty() )
			m_vEntries[0].fGainSeconds = 120;
		break;		
	}		
}

PlayMode Course::GetPlayMode() const
{
	switch( GetCourseType() )
	{
	case COURSE_TYPE_ENDLESS:	return PLAY_MODE_ENDLESS;
	case COURSE_TYPE_ONI:		return PLAY_MODE_ONI;
	case COURSE_TYPE_SURVIVAL:	return PLAY_MODE_ONI;
	case COURSE_TYPE_NONSTOP:	return PLAY_MODE_NONSTOP;
	default: ASSERT(0);	return PLAY_MODE_INVALID;
	}
}

void Course::RevertFromDisk()
{
	// trying to catch invalid an Course
	ASSERT( !m_sPath.empty() );

	CourseLoaderCRS::LoadFromCRSFile( m_sPath, *this );
}

CString Course::GetCacheFilePath() const
{
	return SongCacheIndex::GetCacheFilePath( "Courses", m_sPath );
}

void Course::Init()
{
	m_bIsAutogen = false;
	m_bRepeat = false;
	m_bShuffle = false;
	m_iLives = -1;
	m_bSortByMeter = false;
	m_vEntries.clear();
	FOREACH_Difficulty(dc)
		m_iCustomMeter[dc] = -1;
	m_vEntries.clear();
	m_sPath = "";
	m_sGroupName = "";
	m_sMainTitle = "";
	m_sMainTitleTranslit = "";
	m_sSubTitle = "";
	m_sSubTitleTranslit = "";
	m_sBannerPath = "";
	m_sCDTitlePath = "";
	m_LoadedFromProfile = PROFILE_SLOT_INVALID;
	m_iTrailCacheSeed = 0;
}

void Course::AutogenEndlessFromGroup( CString sGroupName, Difficulty diff )
{
	m_bIsAutogen = true;
	m_bRepeat = true;
	m_bShuffle = true;
	m_iLives = -1;
	FOREACH_Difficulty(dc)
		m_iCustomMeter[dc] = -1;

	if( sGroupName == "" )
	{
		m_sMainTitle = "All Songs";
		// m_sBannerPath = ""; // XXX
	}
	else
	{
		m_sMainTitle = SONGMAN->ShortenGroupName( sGroupName );
		m_sBannerPath = SONGMAN->GetSongGroupBannerPath( sGroupName );
	}

	// We want multiple songs, so we can try to prevent repeats during
	// gameplay. (We might still get a repeat at the repeat boundary,
	// but that'd be rare.) -glenn
	CourseEntry e;
	e.sSongGroup = sGroupName;
	e.baseDifficulty = diff;
	e.bSecret = true;

	vector<Song*> vSongs;
	SONGMAN->GetSongs( vSongs, e.sSongGroup );
	for( unsigned i = 0; i < vSongs.size(); ++i)
		m_vEntries.push_back( e );
}

void Course::AutogenNonstopFromGroup( CString sGroupName, Difficulty diff )
{
	AutogenEndlessFromGroup( sGroupName, diff );

	m_bRepeat = false;

	m_sMainTitle += " Random";	

	// resize to 4
	while( m_vEntries.size() < 4 )
		m_vEntries.push_back( m_vEntries[0] );
	while( m_vEntries.size() > 4 )
		m_vEntries.pop_back();
}

void Course::AutogenOniFromArtist( CString sArtistName, CString sArtistNameTranslit, vector<Song*> aSongs, Difficulty dc )
{
	m_bIsAutogen = true;
	m_bRepeat = false;
	m_bShuffle = true;
	m_bSortByMeter = true;

	m_iLives = 4;
	FOREACH_Difficulty(cd)
		m_iCustomMeter[cd] = -1;

	ASSERT( sArtistName != "" );
	ASSERT( aSongs.size() > 0 );

	/* "Artist Oni" is a little repetitive; "by Artist" stands out less, and lowercasing
	 * "by" puts more emphasis on the artist's name.  It also sorts them together. */
	m_sMainTitle = "by " + sArtistName;
	if( sArtistNameTranslit != sArtistName )
		m_sMainTitleTranslit = "by " + sArtistNameTranslit;


	// m_sBannerPath = ""; // XXX

	/* Shuffle the list to determine which songs we'll use.  Shuffle it deterministically,
	 * so we always get the same set of songs unless the song set changes. */
	{
		RandomGen rng( GetHashForString( sArtistName ) + aSongs.size() );
		random_shuffle( aSongs.begin(), aSongs.end(), rng );
	}

	/* Only use up to four songs. */
	if( aSongs.size() > 4 )
		aSongs.erase( aSongs.begin()+4, aSongs.end() );

	CourseEntry e;
	e.baseDifficulty = dc;

	for( unsigned i = 0; i < aSongs.size(); ++i )
	{
		e.pSong = aSongs[i];
		m_vEntries.push_back( e );
	}
}

bool Course::IsPlayableIn( StepsType st ) const
{
	return GetTrail( st ) != NULL;
}


struct SortTrailEntry
{
	TrailEntry entry;
	int SortMeter;
	bool operator< ( const SortTrailEntry &rhs ) const { return SortMeter < rhs.SortMeter; }
};

CString Course::GetDisplayMainTitle() const
{
	if( !PREFSMAN->m_bShowNativeLanguage )
		return GetTranslitMainTitle();
	return m_sMainTitle;
}

CString Course::GetDisplaySubTitle() const
{
	if( !PREFSMAN->m_bShowNativeLanguage )
		return GetTranslitSubTitle();
	return m_sSubTitle;
}

CString Course::GetDisplayFullTitle() const
{
	CString Title = GetDisplayMainTitle();
	CString SubTitle = GetDisplaySubTitle();

	if(!SubTitle.empty()) Title += " " + SubTitle;
	return Title;
}

CString Course::GetTranslitFullTitle() const
{
	CString Title = GetTranslitMainTitle();
	CString SubTitle = GetTranslitSubTitle();

	if(!SubTitle.empty()) Title += " " + SubTitle;
	return Title;
}

/* This is called by many simple functions, like Course::GetTotalSeconds, and may
 * be called on all songs to sort.  It can take time to execute, so we cache the
 * results.  Returned pointers remain valid for the lifetime of the Course.  If the
 * course difficulty doesn't exist, NULL is returned. */
Trail* Course::GetTrail( StepsType st, CourseDifficulty cd ) const
{
	ASSERT( cd != DIFFICULTY_INVALID );

	//
	// Check to see if the Trail cache is out of date
	//
	if( m_iTrailCacheSeed != GAMESTATE->m_iStageSeed )
	{
		/* If we have any random entries (so that the seed matters), invalidate the cache. */
		bool bHaveRandom = false;
		FOREACH_CONST( CourseEntry, m_vEntries, e )
		{
			if( e->IsRandomSong() )
			{
				bHaveRandom = true;
				break;
			}
		}
		
		if( bHaveRandom )
			m_TrailCache.clear();

		m_iTrailCacheSeed = GAMESTATE->m_iStageSeed;
	}

	//
	// Look in the Trail cache
	//
	{
		TrailCache_t::iterator it = m_TrailCache.find( CacheEntry(st, cd) );
		if( it != m_TrailCache.end() )
		{
			CacheData &cache = it->second;
			if( cache.null )
				return NULL;
			return &cache.trail;
		}
	}

	return GetTrailForceRegenCache( st, cd );
}

Trail* Course::GetTrailForceRegenCache( StepsType st, CourseDifficulty cd ) const
{
	//
	// Construct a new Trail, add it to the cache, then return it.
	//
	CacheData &cache = m_TrailCache[ CacheEntry(st, cd) ];
	Trail &trail = cache.trail;
	trail = Trail();
	if( !GetTrailSorted( st, cd, trail ) || trail.m_vEntries.empty() )
	{
		/* This course difficulty doesn't exist. */
		cache.null = true;
		return NULL;
	}

	//
	// If we have cached RadarValues for this trail, insert them.
	//
	{
		RadarCache_t::const_iterator it = m_RadarCache.find( CacheEntry( st, cd ) );
		if( it != m_RadarCache.end() )
		{
			const RadarValues &rv = it->second;
			trail.SetRadarValues(rv);
		}
	}

	cache.null = false;
	return &cache.trail;
}

bool Course::GetTrailSorted( StepsType st, CourseDifficulty cd, Trail &trail ) const
{
	if( !GetTrailUnsorted( st, cd, trail ) )
		return false;

	if( this->m_bSortByMeter )
	{
		/* Sort according to DIFFICULTY_MEDIUM, since the order of songs
		 * must not change across difficulties. */
		Trail SortTrail;
		if( cd == DIFFICULTY_MEDIUM )
			SortTrail = trail;
		else
		{
			bool bOK = GetTrailUnsorted( st, DIFFICULTY_MEDIUM, SortTrail );

			/* If we have any other difficulty, we must have DIFFICULTY_MEDIUM. */
			ASSERT( bOK );
		}
		ASSERT_M( trail.m_vEntries.size() == SortTrail.m_vEntries.size(),
                  ssprintf("%i %i", int(trail.m_vEntries.size()), int(SortTrail.m_vEntries.size())) );

		vector<SortTrailEntry> entries;
		for( unsigned i = 0; i < trail.m_vEntries.size(); ++i )
		{
			SortTrailEntry ste;
			ste.entry = trail.m_vEntries[i];
			ste.SortMeter = SortTrail.m_vEntries[i].pSteps->GetMeter();
			entries.push_back( ste );
		}

		stable_sort( entries.begin(), entries.end() );
		for( unsigned i = 0; i < trail.m_vEntries.size(); ++i )
			trail.m_vEntries[i] = entries[i].entry;
	}

	return true;
}

bool Course::GetTrailUnsorted( StepsType st, CourseDifficulty cd, Trail &trail ) const
{
	trail.Init();

	switch( cd )
	{
	case DIFFICULTY_BEGINNER:
	case DIFFICULTY_CHALLENGE:
		return false;
	}

	//
	// Construct a new Trail, add it to the cache, then return it.
	//
	/* Different seed for each course, but the same for the whole round: */
	RandomGen rnd( GAMESTATE->m_iStageSeed + GetHashForString(m_sMainTitle) );

	vector<CourseEntry> tmp_entries;
	if( m_bShuffle )
	{
		/* Always randomize the same way per round.  Otherwise, the displayed course
		 * will change every time it's viewed, and the displayed order will have no
		 * bearing on what you'll actually play. */
		tmp_entries = m_vEntries;
		random_shuffle( tmp_entries.begin(), tmp_entries.end(), rnd );
	}

	const vector<CourseEntry> &entries = m_bShuffle? tmp_entries:m_vEntries;

	/* This can take some time, so don't fill it out unless we need it. */
	vector<Song*> vSongsByMostPlayed;
	vector<Song*> AllSongsShuffled;

	trail.m_StepsType = st;
	trail.m_CourseDifficulty = cd;

	/* Set to true if CourseDifficulty is able to change something. */
	bool bCourseDifficultyIsSignificant = (cd == DIFFICULTY_MEDIUM);

	// get a list of all songs that are 1 stage long
	vector<Song*> vpAllPossibleSongs;
	SONGMAN->GetSongs(vpAllPossibleSongs, 1);

	// remove locked and tutorial songs from the list
	FOREACH( Song*, vpAllPossibleSongs, song )
	{
		// Ignore locked songs when choosing randomly
		// TODO: Move Course initialization after UNLOCKMAN is created
		if( UNLOCKMAN  &&  UNLOCKMAN->SongIsLocked(*song) )
		{
			vector<Song*>::iterator eraseme = song;
			song--;
			vpAllPossibleSongs.erase( eraseme );
			continue;
		}

		// Ignore boring tutorial songs
		if( (*song)->IsTutorial() )
		{
			vector<Song*>::iterator eraseme = song;
			song--;
			vpAllPossibleSongs.erase( eraseme );
			continue;
		}
	}

	// Resolve each entry to a Song and Steps.
	FOREACH_CONST( CourseEntry, entries, e )
	{
		Song* pResolvedSong = NULL;	// fill this in
		Steps* pResolvedSteps = NULL;	// fill this in

		//
		// Create a list of matching songs.
		//

		// Start with all songs
		vector<Song*> vpPossibleSongs;
		vector<Steps*> vpPossibleSteps;

		if( e->pSong )
		{
			// Choose an exact song, if we have matching steps and all
			e->pSong->GetSteps( vpPossibleSteps, st, e->baseDifficulty, e->iLowMeter, e->iHighMeter );
			if( ( !e->sSongGroup.empty() && (*song)->m_sGroupName == e->sSongGroup ) && !vpPossibleSteps.empty() )
			{
				pResolvedSong = e->pSong;
			}
			else
			{
				continue;
			}
		}
		else
		{
			// copy over any songs that match our group filter, if one is set, and match our
			// steps filter.
			FOREACH( Song*, vpAllPossibleSongs, song )
			{
				if( !e->sSongGroup.empty() && (*song)->m_sGroupName != e->sSongGroup )
					continue;

				vector<Steps*> vpMatchingSteps;
				(*song)->GetSteps( vpMatchingSteps, st, e->baseDifficulty, e->iLowMeter, e->iHighMeter );
				if( vpMatchingSteps.empty() )
					continue;

				vpPossibleSongs.push_back( *song );
			}

			// if there are no songs to choose from, abort now
			if( vpPossibleSongs.empty() )
				continue;

			// TODO: Move Course initialization after PROFILEMAN is created
			switch( e->songSort )
			{
			default:
				ASSERT(0);
			case SongSort_Randomize:
				random_shuffle( vpPossibleSongs.begin(), vpPossibleSongs.end(), rnd );
				break;
			case SongSort_MostPlays:
				if( PROFILEMAN )
					SongUtil::SortSongPointerArrayByNumPlays( vpPossibleSongs, PROFILEMAN->GetMachineProfile(), true );	// descending
				break;
			case SongSort_FewestPlays:
				if( PROFILEMAN )
					SongUtil::SortSongPointerArrayByNumPlays( vpPossibleSongs, PROFILEMAN->GetMachineProfile(), false );	// ascending
				break;
			case SongSort_TopGrades:
				SongUtil::SortSongPointerArrayByGrades( vpPossibleSongs, true );	// descending
				break;
			case SongSort_LowestGrades:
				SongUtil::SortSongPointerArrayByGrades( vpPossibleSongs, false );	// ascending
				break;
			}

			if( e->iChooseIndex < int(vpPossibleSongs.size()) )
			{
				pResolvedSong = vpPossibleSongs[e->iChooseIndex];
				pResolvedSong->GetSteps( vpPossibleSteps, st, e->baseDifficulty, e->iLowMeter, e->iHighMeter );
			}
			else
			{
				continue;
			}
		}

		ASSERT( !vpPossibleSteps.empty() );	// if no steps are playable, this shouldn't be a possible song
		if( vpPossibleSteps.size() > 1 )	// no reason to randomize if there's only one set of possible steps
			random_shuffle( vpPossibleSteps.begin(), vpPossibleSteps.end(), rnd );
		pResolvedSteps = vpPossibleSteps[0];

		if( pResolvedSong == NULL || pResolvedSteps == NULL )
			continue;	// this song entry isn't playable.  Skip.


		/* If we're not COURSE_DIFFICULTY_REGULAR, then we should be choosing steps that are 
		 * either easier or harder than the base difficulty.  If no such steps exist, then 
		 * just use the one we already have. */
		Difficulty dc = pResolvedSteps->GetDifficulty();
		int iLowMeter = e->iLowMeter;
		int iHighMeter = e->iHighMeter;
		if( cd != DIFFICULTY_MEDIUM  &&  !e->bNoDifficult )
		{
			Difficulty new_dc = (Difficulty)(dc + cd - DIFFICULTY_MEDIUM);
			new_dc = clamp( new_dc, (Difficulty)0, (Difficulty)(DIFFICULTY_EDIT-1) );

			bool bChangedDifficulty = false;
			if( new_dc != dc )
			{
				Steps* pNewSteps = pResolvedSong->GetStepsByDifficulty( st, new_dc );
				if( pNewSteps )
				{
					dc = new_dc;
					pResolvedSteps = pNewSteps;
					bChangedDifficulty = true;
					bCourseDifficultyIsSignificant = true;
				}
			}

			/* Hack: We used to adjust low_meter/high_meter above while searching for
			 * songs.  However, that results in a different song being chosen for
			 * difficult courses, which is bad when LockCourseDifficulties is disabled;
			 * each player can end up with a different song.  Instead, choose based
			 * on the original range, bump the steps based on course difficulty, and
			 * then retroactively tweak the low_meter/high_meter so course displays
			 * line up. */
			if( e->baseDifficulty == DIFFICULTY_INVALID && bChangedDifficulty )
			{
				/* Minimum and maximum to add to make the meter range contain the actual
				 * meter: */
				int iMinDist = pResolvedSteps->GetMeter() - iHighMeter;
				int iMaxDist = pResolvedSteps->GetMeter() - iLowMeter;

				/* Clamp the possible adjustments to try to avoid going under 1 or over
				 * MAX_BOTTOM_RANGE. */
				iMinDist = min( max( iMinDist, -iLowMeter+1 ), iMaxDist );
				iMaxDist = max( min( iMaxDist, MAX_BOTTOM_RANGE-iHighMeter ), iMinDist );

				int iAdd;
				if( iMaxDist == iMinDist )
					iAdd = iMaxDist;
				else
					iAdd = rnd(iMaxDist-iMinDist) + iMinDist;
				iLowMeter += iAdd;
				iHighMeter += iAdd;
			}
		}

		TrailEntry te;
		te.pSong = pResolvedSong;
		te.pSteps = pResolvedSteps;
		te.Modifiers = e->sModifiers;
		te.Attacks = e->attacks;
		te.bSecret = e->bSecret;
		te.iLowMeter = iLowMeter;
		te.iHighMeter = iHighMeter;

		/* If we chose based on meter (not difficulty), then store DIFFICULTY_INVALID, so
		 * other classes can tell that we used meter. */
		if( e->baseDifficulty == DIFFICULTY_INVALID )
		{
			te.dc = DIFFICULTY_INVALID;
		}
		else
		{
			/* Otherwise, store the actual difficulty we got (post-course-difficulty).
			 * This may or may not be the same as e.difficulty. */
			te.dc = dc;
		}
		trail.m_vEntries.push_back( te ); 
	}

	/* Hack: If any entry was non-FIXED, or m_bShuffle is set, then radar values
	 * for this trail will be meaningless as they'll change every time.  Pre-cache
	 * empty data.  XXX: How can we do this cleanly, without propagating lots of
	 * otherwise unnecessary data (course entry types, m_bShuffle) to Trail, or
	 * storing a Course pointer in Trail (yuck)? */
	if( !AllSongsAreFixed() || m_bShuffle )
	{
		trail.m_bRadarValuesCached = true;
		trail.m_CachedRadarValues = RadarValues();
	}

	/* If we have a manually-entered meter for this difficulty, use it. */
	if( m_iCustomMeter[cd] != -1 )
		trail.m_iSpecifiedMeter = m_iCustomMeter[cd];

	/* If the course difficulty never actually changed anything, then this difficulty
	 * is equivalent to DIFFICULTY_MEDIUM; it doesn't exist. */
	return bCourseDifficultyIsSignificant;
}

void Course::GetTrails( vector<Trail*> &AddTo, StepsType st ) const
{
	FOREACH_ShownCourseDifficulty( cd )
	{
		Trail *pTrail = GetTrail( st, cd );
		if( pTrail == NULL )
			continue;
		AddTo.push_back( pTrail );
	}
}

void Course::GetAllTrails( vector<Trail*> &AddTo ) const
{
	vector<StepsType> vStepsTypesToShow;
	GAMEMAN->GetStepsTypesForGame( GAMESTATE->m_pCurGame, vStepsTypesToShow );
	FOREACH( StepsType, vStepsTypesToShow, st )
	{
		GetTrails( AddTo, *st );
	}
}

int Course::GetMeter( StepsType st, CourseDifficulty cd ) const
{
    if( m_iCustomMeter[cd] != -1 )
		return m_iCustomMeter[cd];
	const Trail* pTrail = GetTrail( st );
	if( pTrail != NULL )
		return pTrail->GetMeter();
	return 0;
}

bool Course::HasMods() const
{
	FOREACH_CONST( CourseEntry, m_vEntries, e )
	{
		if( !e->sModifiers.empty() || !e->attacks.empty() )
			return true;
	}

	return false;
}

bool Course::AllSongsAreFixed() const
{
	FOREACH_CONST( CourseEntry, m_vEntries, e )
	{
		if( e->pSong == NULL )
			return false;
	}
	return true;
}

void Course::Invalidate( Song *pStaleSong )
{
	FOREACH_CONST( CourseEntry, m_vEntries, e )
	{
		if( e->pSong == pStaleSong )	// a fixed entry that references the stale Song
		{
			RevertFromDisk();
			return;
		}
	}

	// Invalidate any Trails that contain this song.
	// If we find a Trail that contains this song, then it's part of a 
	// non-fixed entry.  So, regenerating the Trail will force different
	// songs to be chosen.
	FOREACH_StepsType( st )
	{
		FOREACH_ShownCourseDifficulty( cd )
		{
			TrailCache_t::iterator it = m_TrailCache.find( CacheEntry(st, cd) );
			if( it == m_TrailCache.end() )
				continue;
			CacheData &cache = it->second;
			if( !cache.null )
				if( GetTrail( st, cd )->ContainsSong( pStaleSong ) )
					m_TrailCache.erase( it );
		}
	}
}

void Course::RegenerateNonFixedTrails()
{
	// Only need to regen Trails if the Course has a random entry.
	// We can create these Trails on demand because we don't 
	// calculate RadarValues for Trails with one or more non-fixed 
	// entry.
	if( !IsFixed() )
		m_TrailCache.clear();
}

RageColor Course::GetColor() const
{
	// FIXME: Calculate the meter.
	int iMeter = 5;
	
	switch( PREFSMAN->m_CourseSortOrder )
	{
	case PrefsManager::COURSE_SORT_SONGS:	
		if( m_vEntries.size() >= 7 )				return SORT_LEVEL2_COLOR;
		else if( m_vEntries.size() >= 4 )		return SORT_LEVEL4_COLOR;
		else									return SORT_LEVEL5_COLOR;

	case PrefsManager::COURSE_SORT_METER:
		if( !IsFixed() )						return SORT_LEVEL1_COLOR;
		else if( iMeter > 9 )					return SORT_LEVEL2_COLOR;
		else if( iMeter >= 7 )					return SORT_LEVEL3_COLOR;
		else if( iMeter >= 5 )					return SORT_LEVEL4_COLOR;
		else 									return SORT_LEVEL5_COLOR;

	case PrefsManager::COURSE_SORT_METER_SUM:
		if( !IsFixed() )						return SORT_LEVEL1_COLOR;
		if( m_SortOrder_TotalDifficulty >= 40 )	return SORT_LEVEL2_COLOR;
		if( m_SortOrder_TotalDifficulty >= 30 )	return SORT_LEVEL3_COLOR;
		if( m_SortOrder_TotalDifficulty >= 20 )	return SORT_LEVEL4_COLOR;
		else									return SORT_LEVEL5_COLOR;

	case PrefsManager::COURSE_SORT_RANK:
		if( m_SortOrder_Ranking == 3 )			return SORT_LEVEL1_COLOR;
		else if( m_SortOrder_Ranking == 2 )		return SORT_LEVEL3_COLOR;
		else if( m_SortOrder_Ranking == 1 )		return SORT_LEVEL5_COLOR;
		else									return SORT_LEVEL4_COLOR;
	default:
		ASSERT(0);
		return RageColor(1,1,1,1);  // white, never should reach here
	}
}

bool Course::IsFixed() const
{
	for(unsigned i = 0; i < m_vEntries.size(); i++)
	{
		if( m_vEntries[i].pSong == NULL )
			return false;
	}

	return true;
}


bool Course::GetTotalSeconds( StepsType st, float& fSecondsOut ) const
{
	if( !IsFixed() )
		return false;

	Trail* pTrail = GetTrail( st, DIFFICULTY_MEDIUM );

	fSecondsOut = pTrail->GetLengthSeconds();
	return true;
}

bool Course::CourseHasBestOrWorst() const
{
	FOREACH_CONST( CourseEntry, m_vEntries, e )
	{
		if( e->iChooseIndex == SongSort_MostPlays  &&  e->iChooseIndex != -1 )
			return true;
		if( e->iChooseIndex == SongSort_FewestPlays  &&  e->iChooseIndex != -1 )
			return true;
	}

	return false;
}

bool Course::HasBanner() const
{
	return m_sBannerPath != ""  &&  IsAFile(m_sBannerPath);
}

void Course::UpdateCourseStats( StepsType st )
{
	m_SortOrder_TotalDifficulty = 0;

	// courses with random/players best-worst songs should go at the end
	for(unsigned i = 0; i < m_vEntries.size(); i++)
	{
		if ( m_vEntries[i].pSong != NULL )
			continue;

		if ( m_SortOrder_Ranking == 2 )
			m_SortOrder_Ranking = 3;
		m_SortOrder_TotalDifficulty = INT_MAX;
		return;
	}

	const Trail* pTrail = GetTrail( st, DIFFICULTY_MEDIUM );

	m_SortOrder_TotalDifficulty += pTrail != NULL? pTrail->GetTotalMeter():0;

	// OPTIMIZATION: Ranking info isn't dependant on style, so
	// call it sparingly.  Its handled on startup and when
	// themes change..
	
	LOG->Trace("%s: Total feet: %d",
		this->m_sMainTitle.c_str(),
		m_SortOrder_TotalDifficulty );
}

bool Course::IsRanking() const
{
	CStringArray rankingsongs;
	
	split(THEME->GetMetric("ScreenRanking", "CoursesToShow"), ",", rankingsongs);

	for(unsigned i=0; i < rankingsongs.size(); i++)
		if (rankingsongs[i].CompareNoCase(m_sPath))
			return true;

	return false;
}

const CourseEntry *Course::FindFixedSong( const Song *pSong ) const
{
	FOREACH_CONST( CourseEntry, m_vEntries, e )
	{
		const CourseEntry &entry = *e;
		if( entry.pSong == pSong )
			return &entry;
	}

	return NULL;
}

void Course::GetAllCachedTrails( vector<Trail *> &out )
{
	TrailCache_t::iterator it;
	for( it = m_TrailCache.begin(); it != m_TrailCache.end(); ++it )
	{
		CacheData &cd = it->second;
		if( !cd.null )
			out.push_back( &cd.trail );
	}
}

bool Course::ShowInDemonstrationAndRanking() const
{
	// Don't show endless courses in Ranking.
	if( IsEndless() )
		return false;
	return true;
}

void Course::CalculateRadarValues()
{
	FOREACH_StepsType( st )
	{
		FOREACH_CourseDifficulty( cd )
		{
			// For courses that aren't fixed, the radar values are meaningless.
			// Makes non-fixed courses have unknown radar values.
			if( IsFixed() )
			{
				Trail *pTrail = GetTrail( st, cd );
				if( pTrail == NULL )
					continue;
				RadarValues rv = pTrail->GetRadarValues();
				m_RadarCache[CacheEntry(st, cd)] = rv;
			}
			else
			{
				m_RadarCache[CacheEntry(st, cd)] = RadarValues();
			}
		}
	}
}

// lua start
#include "LuaBinding.h"

class LunaCourse: public Luna<Course>
{
public:
	LunaCourse() { LUA->Register( Register ); }

	static int GetPlayMode( T* p, lua_State *L )			{ lua_pushnumber(L, p->GetPlayMode() ); return 1; }
	static int GetDisplayFullTitle( T* p, lua_State *L )	{ lua_pushstring(L, p->GetDisplayFullTitle() ); return 1; }
	static int GetTranslitFullTitle( T* p, lua_State *L )	{ lua_pushstring(L, p->GetTranslitFullTitle() ); return 1; }
	static int HasMods( T* p, lua_State *L )				{ lua_pushboolean(L, p->HasMods() ); return 1; }
	static int GetCourseType( T* p, lua_State *L )			{ lua_pushnumber(L, p->GetCourseType() ); return 1; }
	static int GetCourseEntry( T* p, lua_State *L )			{ CourseEntry &ce = p->m_vEntries[IArg(1)]; ce.PushSelf(L); return 1; }
	static int GetAllTrails( T* p, lua_State *L )
	{
		vector<Trail*> v;
		p->GetAllTrails( v );
		LuaHelpers::CreateTableFromArray<Trail*>( v, L );
		return 1;
	}

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetPlayMode );
		ADD_METHOD( GetDisplayFullTitle );
		ADD_METHOD( GetTranslitFullTitle );
		ADD_METHOD( HasMods );
		ADD_METHOD( GetCourseType );
		ADD_METHOD( GetCourseEntry );
		ADD_METHOD( GetAllTrails );

		Luna<T>::Register( L );
	}
};

LUA_REGISTER_CLASS( Course )
// lua end


/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
