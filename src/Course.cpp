#include <limits.h>
#include "global.h"
#include "Course.h"
#include "CourseLoaderCRS.h"

#include "GameManager.h"
#include "GameState.h"
#include "LocalizedString.h"
#include "LuaManager.h"
#include "Preference.h"
#include "PrefsManager.h"
#include "ProfileManager.h"
#include "RageLog.h"
#include "Song.h"
#include "SongCacheIndex.h"
#include "Steps.h"
#include "ThemeManager.h"
#include "UnlockManager.h"
#include "Game.h"
#include "Style.h"

static Preference<int> MAX_SONGS_IN_EDIT_COURSE( "MaxSongsInEditCourse", -1 );

static const char *SongSortNames[] = {
	"Randomize",
	"MostPlays",
	"FewestPlays",
	"TopGrades",
	"LowestGrades",
};
XToString( SongSort );
XToLocalizedString( SongSort );


/* Maximum lower value of ranges when difficult: */
const int MAX_BOTTOM_RANGE = 10;

#define SORT_PREFERRED_COLOR	THEME->GetMetricC("Course","SortPreferredColor")
#define SORT_LEVEL1_COLOR	THEME->GetMetricC("Course","SortLevel1Color")
#define SORT_LEVEL2_COLOR	THEME->GetMetricC("Course","SortLevel2Color")
#define SORT_LEVEL3_COLOR	THEME->GetMetricC("Course","SortLevel3Color")
#define SORT_LEVEL4_COLOR	THEME->GetMetricC("Course","SortLevel4Color")
#define SORT_LEVEL5_COLOR	THEME->GetMetricC("Course","SortLevel5Color")

//#define INCLUDE_BEGINNER_STEPS	THEME->GetMetricB( "Course","IncludeBeginnerSteps" );

RString CourseEntry::GetTextDescription() const
{
	vector<RString> vsEntryDescription;
	Song *pSong = songID.ToSong();
	if( pSong )
		vsEntryDescription.push_back( pSong->GetTranslitFullTitle() ); 
	else
		vsEntryDescription.push_back( "Random" );
	if( !songCriteria.m_sGroupName.empty() )
		vsEntryDescription.push_back( songCriteria.m_sGroupName );
	if( songCriteria.m_bUseSongGenreAllowedList )
		vsEntryDescription.push_back( join(",",songCriteria.m_vsSongGenreAllowedList) );
	if( stepsCriteria.m_difficulty != Difficulty_Invalid  &&  stepsCriteria.m_difficulty != Difficulty_Medium )
		vsEntryDescription.push_back( CourseDifficultyToLocalizedString(stepsCriteria.m_difficulty) );
	if( stepsCriteria.m_iLowMeter != -1 )
		vsEntryDescription.push_back( ssprintf("Low meter: %d", stepsCriteria.m_iLowMeter) );
	if( stepsCriteria.m_iHighMeter != -1 )
		vsEntryDescription.push_back( ssprintf("High meter: %d", stepsCriteria.m_iHighMeter) );
	if( songSort != SongSort_Randomize )
		vsEntryDescription.push_back( "Sort: %d" + SongSortToLocalizedString(songSort) );
	if( songSort != SongSort_Randomize && iChooseIndex != 0 )
		vsEntryDescription.push_back( "Choose " + FormatNumberAndSuffix(iChooseIndex) + " match" );
	int iNumModChanges = GetNumModChanges();
	if( iNumModChanges != 0 )
		vsEntryDescription.push_back( ssprintf("%d mod changes", iNumModChanges) );
	if( fGainSeconds != 0 )
		vsEntryDescription.push_back( ssprintf("Low meter: %.0f", fGainSeconds) );

	RString s = join( ",", vsEntryDescription );
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


Course::Course(): m_bIsAutogen(false), m_sPath(""), m_sMainTitle(""),
	m_sMainTitleTranslit(""), m_sSubTitle(""), m_sSubTitleTranslit(""),
	m_sScripter(""), m_sDescription(""), m_sBannerPath(""), m_sBackgroundPath(""),
	m_sCDTitlePath(""), m_sGroupName(""), m_bRepeat(false), m_fGoalSeconds(0), 
	m_bShuffle(false), m_iLives(-1), m_bSortByMeter(false),
	m_bIncomplete(false), m_vEntries(), m_SortOrder_TotalDifficulty(0),
	m_SortOrder_Ranking(0), m_LoadedFromProfile(ProfileSlot_Invalid),
	m_TrailCache(), m_iTrailCacheSeed(0), m_RadarCache(),
	m_setStyles(), m_CachedObject()
{
	FOREACH_ENUM( Difficulty,dc)
	m_iCustomMeter[dc] = -1;
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
	default:
		FAIL_M(ssprintf("Invalid course type: %i", ct));
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
	CourseType ct = GetCourseType();
	switch( ct )
	{
	case COURSE_TYPE_ENDLESS:	return PLAY_MODE_ENDLESS;
	case COURSE_TYPE_ONI:		return PLAY_MODE_ONI;
	case COURSE_TYPE_SURVIVAL:	return PLAY_MODE_ONI;
	case COURSE_TYPE_NONSTOP:	return PLAY_MODE_NONSTOP;
	default:
		FAIL_M(ssprintf("Invalid course type: %i", ct));
	}
}

void Course::RevertFromDisk()
{
	// trying to catch invalid an Course
	ASSERT( !m_sPath.empty() );

	CourseLoaderCRS::LoadFromCRSFile( m_sPath, *this );
}

RString Course::GetCacheFilePath() const
{
	return SongCacheIndex::GetCacheFilePath( "Courses", m_sPath );
}

void Course::Init()
{
	m_bIsAutogen = false;
	m_sPath = "";

	m_sMainTitle = "";
	m_sMainTitleTranslit = "";
	m_sSubTitle = "";
	m_sSubTitleTranslit = "";
	m_sScripter = "";
	m_sDescription = "";

	m_sBannerPath = "";
	m_sBackgroundPath = "";
	m_sCDTitlePath = "";
	m_sGroupName = "";

	m_bRepeat = false;
	m_fGoalSeconds = 0;
	m_bShuffle = false;

	m_iLives = -1;
	FOREACH_ENUM( Difficulty,dc)
		m_iCustomMeter[dc] = -1;
	m_bSortByMeter = false;

	m_vEntries.clear();

	m_SortOrder_TotalDifficulty = 0;
	m_SortOrder_Ranking = 0;

	m_LoadedFromProfile = ProfileSlot_Invalid;
	m_bIncomplete = false;
	m_TrailCache.clear();
	m_iTrailCacheSeed = 0;
	m_RadarCache.clear();
}

bool Course::IsPlayableIn( StepsType st ) const
{
	Trail t;
	FOREACH_ShownCourseDifficulty( cd )
	{
		if( GetTrailUnsorted( st, cd, t ) )
			return true;
	}

	// No valid trail for this StepsType.
	return false;
}


struct SortTrailEntry
{
	TrailEntry entry;
	int SortMeter;
	
	SortTrailEntry(): entry(), SortMeter(0) {}
	
	bool operator< ( const SortTrailEntry &rhs ) const { return SortMeter < rhs.SortMeter; }
};

RString Course::GetDisplayMainTitle() const
{
	if( !PREFSMAN->m_bShowNativeLanguage )
		return GetTranslitMainTitle();
	return m_sMainTitle;
}

RString Course::GetDisplaySubTitle() const
{
	if( !PREFSMAN->m_bShowNativeLanguage )
		return GetTranslitSubTitle();
	return m_sSubTitle;
}

RString Course::GetDisplayFullTitle() const
{
	RString sTitle = GetDisplayMainTitle();
	RString sSubTitle = GetDisplaySubTitle();

	if( !sSubTitle.empty() )
		sTitle += " " + sSubTitle;
	return sTitle;
}

RString Course::GetTranslitFullTitle() const
{
	RString sTitle = GetTranslitMainTitle();
	RString sSubTitle = GetTranslitSubTitle();

	if( !sSubTitle.empty() )
		sTitle += " " + sSubTitle;
	return sTitle;
}

/* This is called by many simple functions, like Course::GetTotalSeconds, and may
 * be called on all songs to sort.  It can take time to execute, so we cache the
 * results.  Returned pointers remain valid for the lifetime of the Course.  If the
 * course difficulty doesn't exist, nullptr is returned. */
Trail* Course::GetTrail( StepsType st, CourseDifficulty cd ) const
{
	ASSERT( cd != Difficulty_Invalid );

	// Check to see if the Trail cache is out of date
	if( m_iTrailCacheSeed != GAMESTATE->m_iStageSeed )
	{
		RegenerateNonFixedTrails();
		m_iTrailCacheSeed = GAMESTATE->m_iStageSeed;
	}

	// Look in the Trail cache
	{
		TrailCache_t::iterator it = m_TrailCache.find( CacheEntry(st, cd) );
		if( it != m_TrailCache.end() )
		{
			CacheData &cache = it->second;
			if( cache.null )
				return nullptr;
			return &cache.trail;
		}
	}

	return GetTrailForceRegenCache( st, cd );
}

Trail* Course::GetTrailForceRegenCache( StepsType st, CourseDifficulty cd ) const
{
	// Construct a new Trail, add it to the cache, then return it.
	CacheData &cache = m_TrailCache[ CacheEntry(st, cd) ];
	Trail &trail = cache.trail;
	trail.Init();
	if( !GetTrailSorted(st, cd, trail) )
	{
		// This course difficulty doesn't exist.
		cache.null = true;
		return nullptr;
	}

	// If we have cached RadarValues for this trail, insert them.
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
		/* Sort according to Difficulty_Medium, since the order of songs
		 * must not change across difficulties. */
		Trail SortTrail;
		if( cd == Difficulty_Medium )
			SortTrail = trail;
		else
		{
			bool bOK = GetTrailUnsorted( st, Difficulty_Medium, SortTrail );

			/* If we have any other difficulty, we must have Difficulty_Medium. */
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

	if( trail.m_vEntries.empty() )
		return false;
	return true;
}

// TODO: Move Course initialization after PROFILEMAN is created
static void CourseSortSongs( SongSort sort, vector<Song*> &vpPossibleSongs, RandomGen &rnd )
{
	switch( sort )
	{
	DEFAULT_FAIL(sort);
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
		if( PROFILEMAN )
			SongUtil::SortSongPointerArrayByGrades( vpPossibleSongs, true );	// descending
		break;
	case SongSort_LowestGrades:
		if( PROFILEMAN )
			SongUtil::SortSongPointerArrayByGrades( vpPossibleSongs, false );	// ascending
		break;
	}
}

namespace
{
	struct SongIsEqual
	{
		const Song *m_pSong;
		SongIsEqual( const Song *pSong ) : m_pSong(pSong) { }
		bool operator()( const SongAndSteps &sas ) const { return sas.pSong == m_pSong; }
	};
}

bool Course::GetTrailUnsorted( StepsType st, CourseDifficulty cd, Trail &trail ) const
{
	trail.Init();

	// XXX: Why are beginner and challenge excluded here? -Wolfman2000
	// No idea, probably an obsolete design decision from ITG, removing
	// exclusion here, but there's some other area that prevents it too. -Kyz
	/*
	switch( cd )
	{
		case Difficulty_Beginner:
			return false;
		case Difficulty_Challenge:
			return false;
		default: break;
	}
	*/

	// Construct a new Trail, add it to the cache, then return it.
	// Different seed for each course, but the same for the whole round:
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

	const vector<CourseEntry> &entries = m_bShuffle ? tmp_entries:m_vEntries;

	// This can take some time, so don't fill it out unless we need it.
	vector<Song*> vSongsByMostPlayed;
	vector<Song*> AllSongsShuffled;

	trail.m_StepsType = st;
	trail.m_CourseType = GetCourseType();
	trail.m_CourseDifficulty = cd;
	trail.m_vEntries.reserve(entries.size());

	// Set to true if CourseDifficulty is able to change something.
	bool bCourseDifficultyIsSignificant = (cd == Difficulty_Medium);

	

	// Resolve each entry to a Song and Steps.
	if( trail.m_CourseType == COURSE_TYPE_ENDLESS )
	{
		GetTrailUnsortedEndless(entries, trail, st, cd, rnd, bCourseDifficultyIsSignificant);
	}
	else
	{
		vector<SongAndSteps> vSongAndSteps;
		for (auto e = entries.begin(); e != entries.end(); ++e)
		{
			SongAndSteps resolved;	// fill this in
			SongCriteria soc = e->songCriteria;

			Song *pSong = e->songID.ToSong();
			if( pSong )
			{
				soc.m_bUseSongAllowedList = true;
				soc.m_vpSongAllowedList.push_back( pSong );
			}
			soc.m_Tutorial = SongCriteria::Tutorial_No;
			soc.m_Locked = SongCriteria::Locked_Unlocked;
			if( !soc.m_bUseSongAllowedList )
				soc.m_iMaxStagesForSong = 1;

			StepsCriteria stc = e->stepsCriteria;
			stc.m_st = st;
			stc.m_Locked = StepsCriteria::Locked_Unlocked;

			const bool bSameSongCriteria = e != entries.begin() && ( e - 1 )->songCriteria == soc;
			const bool bSameStepsCriteria = e != entries.begin() && ( e - 1 )->stepsCriteria == stc;

			if( pSong )
			{
				vSongAndSteps.clear();
				StepsUtil::GetAllMatching( pSong, stc, vSongAndSteps );
			}
			else if( vSongAndSteps.empty() || !( bSameSongCriteria && bSameStepsCriteria ) )
			{
				vSongAndSteps.clear();
				StepsUtil::GetAllMatching( soc, stc, vSongAndSteps );
			}

			// It looks bad to have the same song 2x in a row in a randomly generated course.
			// Don't allow the same song to be played 2x in a row, unless there's only
			// one song in vpPossibleSongs.
			if( trail.m_vEntries.size() > 0 && vSongAndSteps.size() > 1 )
			{
				const TrailEntry &teLast = trail.m_vEntries.back();
				RemoveIf( vSongAndSteps, SongIsEqual( teLast.pSong ) );
			}

			// if there are no songs to choose from, abort this CourseEntry
			if( vSongAndSteps.empty() )
				continue;

			vector<Song*> vpSongs;
			typedef vector<Steps*> StepsVector;
			map<Song*, StepsVector> mapSongToSteps;
			for (std::vector<SongAndSteps>::const_iterator sas = vSongAndSteps.begin(); sas != vSongAndSteps.end(); ++sas)
			{
				StepsVector &v = mapSongToSteps[ sas->pSong ];

				v.push_back( sas->pSteps );
				if( v.size() == 1 )
					vpSongs.push_back( sas->pSong );
			}

			CourseSortSongs( e->songSort, vpSongs, rnd );

			ASSERT( e->iChooseIndex >= 0 );
			if( e->iChooseIndex < int( vSongAndSteps.size() ) )
			{
				resolved.pSong = vpSongs[ e->iChooseIndex ];
				const vector<Steps*> &mappedSongs = mapSongToSteps[ resolved.pSong ];
				resolved.pSteps = mappedSongs[ RandomInt( mappedSongs.size() ) ];
			}
			else
			{
				continue;
			}

			/* If we're not COURSE_DIFFICULTY_REGULAR, then we should be choosing steps that are
			* either easier or harder than the base difficulty.  If no such steps exist, then
			* just use the one we already have. */
			Difficulty dc = resolved.pSteps->GetDifficulty();
			int iLowMeter = e->stepsCriteria.m_iLowMeter;
			int iHighMeter = e->stepsCriteria.m_iHighMeter;
			if( cd != Difficulty_Medium  &&  !e->bNoDifficult )
			{
				Difficulty new_dc = ( Difficulty )( dc + cd - Difficulty_Medium );
				new_dc = clamp( new_dc, ( Difficulty )0, ( Difficulty )( Difficulty_Edit - 1 ) );
				/*
				// re-edit this code to work using the metric.
				Difficulty new_dc;
				if( INCLUDE_BEGINNER_STEPS )
				{
				// don't factor in the course difficulty if we're including
				// beginner steps -aj
				new_dc = clamp( dc, Difficulty_Beginner, (Difficulty)(Difficulty_Edit-1) );
				}
				else
				{
				new_dc = (Difficulty)(dc + cd - Difficulty_Medium);
				new_dc = clamp( new_dc, (Difficulty)0, (Difficulty)(Difficulty_Edit-1) );
				}
				*/

				bool bChangedDifficulty = false;
				if( new_dc != dc )
				{
					Steps* pNewSteps = SongUtil::GetStepsByDifficulty( resolved.pSong, st, new_dc );
					if( pNewSteps )
					{
						dc = new_dc;
						resolved.pSteps = pNewSteps;
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
				if( e->stepsCriteria.m_difficulty == Difficulty_Invalid && bChangedDifficulty )
				{
					/* Minimum and maximum to add to make the meter range contain the actual
					* meter: */
					int iMinDist = resolved.pSteps->GetMeter() - iHighMeter;
					int iMaxDist = resolved.pSteps->GetMeter() - iLowMeter;

					/* Clamp the possible adjustments to try to avoid going under 1 or over
					* MAX_BOTTOM_RANGE. */
					iMinDist = min( max( iMinDist, -iLowMeter + 1 ), iMaxDist );
					iMaxDist = max( min( iMaxDist, MAX_BOTTOM_RANGE - iHighMeter ), iMinDist );

					int iAdd;
					if( iMaxDist == iMinDist )
						iAdd = iMaxDist;
					else
						iAdd = rnd( iMaxDist - iMinDist ) + iMinDist;
					iLowMeter += iAdd;
					iHighMeter += iAdd;
				}
			}

			TrailEntry te;
			te.pSong = resolved.pSong;
			te.pSteps = resolved.pSteps;
			te.Modifiers = e->sModifiers;
			te.Attacks = e->attacks;
			te.bSecret = e->bSecret;
			te.iLowMeter = iLowMeter;
			te.iHighMeter = iHighMeter;

			/* If we chose based on meter (not difficulty), then store Difficulty_Invalid, so
			* other classes can tell that we used meter. */
			if( e->stepsCriteria.m_difficulty == Difficulty_Invalid )
			{
				te.dc = Difficulty_Invalid;
			}
			else
			{
				/* Otherwise, store the actual difficulty we got (post-course-difficulty).
				* This may or may not be the same as e.difficulty. */
				te.dc = dc;
			}
			trail.m_vEntries.push_back( te );

			// LOG->Trace( "Chose: %s, %d", te.pSong->GetSongDir().c_str(), te.pSteps->GetMeter() );

			if( IsAnEdit() && MAX_SONGS_IN_EDIT_COURSE > 0 &&
				int( trail.m_vEntries.size() ) >= MAX_SONGS_IN_EDIT_COURSE )
			{
				break;
			}
		}
	}

	/* Hack: If any entry was non-FIXED, or m_bShuffle is set, then radar values
	 * for this trail will be meaningless as they'll change every time. Pre-cache
	 * empty data. XXX: How can we do this cleanly, without propagating lots of
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
	 * is equivalent to Difficulty_Medium; it doesn't exist. */
	return bCourseDifficultyIsSignificant && trail.m_vEntries.size() > 0;
}

void Course::GetTrailUnsortedEndless( const vector<CourseEntry> &entries, Trail &trail, StepsType &st, 
	CourseDifficulty &cd, RandomGen &rnd, bool &bCourseDifficultyIsSignificant ) const
{
	typedef vector<Steps*> StepsVector;

	std::set<Song*> alreadySelected;
	Song* lastSongSelected;
	vector<SongAndSteps> vSongAndSteps;
	for (auto e = entries.begin(); e != entries.end(); ++e)
	{

		SongAndSteps resolved;	// fill this in
		SongCriteria soc = e->songCriteria;


		Song *pSong = e->songID.ToSong();
		if( pSong )
		{
			soc.m_bUseSongAllowedList = true;
			soc.m_vpSongAllowedList.push_back( pSong );
		}
		soc.m_Tutorial = SongCriteria::Tutorial_No;
		soc.m_Locked = SongCriteria::Locked_Unlocked;
		if( !soc.m_bUseSongAllowedList )
			soc.m_iMaxStagesForSong = 1;

		StepsCriteria stc = e->stepsCriteria;
		stc.m_st = st;
		stc.m_Locked = StepsCriteria::Locked_Unlocked;

		const bool bSameSongCriteria = e != entries.begin() && ( e - 1 )->songCriteria == soc;
		const bool bSameStepsCriteria = e != entries.begin() && ( e - 1 )->stepsCriteria == stc;

		// If we're doing the same wildcard search as last entry,
		// we can just reuse the vSongAndSteps vector.
		if( pSong )
		{
			vSongAndSteps.clear();
			StepsUtil::GetAllMatchingEndless( pSong, stc, vSongAndSteps );
		}
		else if( vSongAndSteps.empty() || !( bSameSongCriteria && bSameStepsCriteria ) )
		{
			vSongAndSteps.clear();
			StepsUtil::GetAllMatching( soc, stc, vSongAndSteps );
		}

		// if there are no songs to choose from, abort this CourseEntry
		if( vSongAndSteps.empty() )
			continue;

		// if we're doing a RANDOM wildcard search, try to avoid repetition
		if (vSongAndSteps.size() > 1 && e->songSort == SongSort::SongSort_Randomize)
		{
			// Make a backup of the steplist so we can revert if we overfilter
			std::vector<SongAndSteps> revertList = vSongAndSteps;
			// Filter candidate list via blacklist
			RemoveIf(vSongAndSteps, [&](const SongAndSteps& ss) {
				return std::find(alreadySelected.begin(), alreadySelected.end(), ss.pSong) != alreadySelected.end();
			});
			// If every candidate is in the blacklist, pick random song that wasn't played last
			// (Repeat songs may still occur if song after this is fixed; this algorithm doesn't look ahead)
			if (vSongAndSteps.empty())
			{
				vSongAndSteps = revertList;
				RemoveIf(vSongAndSteps, SongIsEqual(lastSongSelected));

				// If the song that was played last was the only candidate, give up pick randomly
				if (vSongAndSteps.empty())
				{
					vSongAndSteps = revertList;
				}
			}
		}

		std::vector<Song*> vpSongs;
		std::map<Song*, StepsVector> songStepMap;
		// Build list of songs for sorting, and mapping of songs to steps simultaneously
		for (auto& ss : vSongAndSteps)
		{
			StepsVector& stepsForSong = songStepMap[ss.pSong];
			// If we haven't noted this song yet, add it to the song list
			if (stepsForSong.size() == 0)
			{
				vpSongs.push_back(ss.pSong);
			}

			stepsForSong.push_back(ss.pSteps);
		}

		ASSERT(e->iChooseIndex >= 0);
		// If we're trying to pick BEST100 when only 99 songs exist,
		// we have a problem, so bail out
		if (e->iChooseIndex >= vpSongs.size()) {
			continue;
		}


		// Otherwise, pick random steps corresponding to the selected song
		CourseSortSongs(e->songSort, vpSongs, rnd);
		resolved.pSong = vpSongs[e->iChooseIndex];
		const vector<Steps*>& songSteps = songStepMap[resolved.pSong];
		resolved.pSteps = songSteps[RandomInt(songSteps.size())];

		lastSongSelected = resolved.pSong;
		alreadySelected.emplace(resolved.pSong);

		/* If we're not COURSE_DIFFICULTY_REGULAR, then we should be choosing steps that are
		 * either easier or harder than the base difficulty.  If no such steps exist, then
		 * just use the one we already have. */
		Difficulty dc = resolved.pSteps->GetDifficulty();
		int iLowMeter = e->stepsCriteria.m_iLowMeter;
		int iHighMeter = e->stepsCriteria.m_iHighMeter;
		if( cd != Difficulty_Medium  &&  !e->bNoDifficult )
		{
			Difficulty new_dc = ( Difficulty )( dc + cd - Difficulty_Medium );
			if( dc != Difficulty_Medium )
			{
				new_dc = cd;
			}
			new_dc = clamp( new_dc, ( Difficulty )0, ( Difficulty )( Difficulty_Edit - 1 ) );
			/*
			// re-edit this code to work using the metric.
			Difficulty new_dc;
			if( INCLUDE_BEGINNER_STEPS )
			{
				// don't factor in the course difficulty if we're including
				// beginner steps -aj
				new_dc = clamp( dc, Difficulty_Beginner, (Difficulty)(Difficulty_Edit-1) );
			}
			else
			{
				new_dc = (Difficulty)(dc + cd - Difficulty_Medium);
				new_dc = clamp( new_dc, (Difficulty)0, (Difficulty)(Difficulty_Edit-1) );
			}
			*/

			bool bChangedDifficulty = false;
			if( new_dc != dc )
			{
				Steps* pNewSteps = SongUtil::GetStepsByDifficulty( resolved.pSong, st, new_dc );
				if( pNewSteps )
				{
					dc = new_dc;
					resolved.pSteps = pNewSteps;
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
			if( e->stepsCriteria.m_difficulty == Difficulty_Invalid && bChangedDifficulty )
			{
				/* Minimum and maximum to add to make the meter range contain the actual
				 * meter: */
				int iMinDist = resolved.pSteps->GetMeter() - iHighMeter;
				int iMaxDist = resolved.pSteps->GetMeter() - iLowMeter;

				/* Clamp the possible adjustments to try to avoid going under 1 or over
				 * MAX_BOTTOM_RANGE. */
				iMinDist = min( max( iMinDist, -iLowMeter + 1 ), iMaxDist );
				iMaxDist = max( min( iMaxDist, MAX_BOTTOM_RANGE - iHighMeter ), iMinDist );

				int iAdd;
				if( iMaxDist == iMinDist )
					iAdd = iMaxDist;
				else
					iAdd = rnd( iMaxDist - iMinDist ) + iMinDist;
				iLowMeter += iAdd;
				iHighMeter += iAdd;
			}
		}

		TrailEntry te;
		te.pSong = resolved.pSong;
		te.pSteps = resolved.pSteps;
		te.Modifiers = e->sModifiers;
		te.Attacks = e->attacks;
		te.bSecret = e->bSecret;
		te.iLowMeter = iLowMeter;
		te.iHighMeter = iHighMeter;

		/* If we chose based on meter (not difficulty), then store Difficulty_Invalid, so
		 * other classes can tell that we used meter. */
		if( e->stepsCriteria.m_difficulty == Difficulty_Invalid )
		{
			te.dc = Difficulty_Invalid;
		}
		else
		{
			/* Otherwise, store the actual difficulty we got (post-course-difficulty).
			 * This may or may not be the same as e.difficulty. */
			te.dc = dc;
		}

		trail.m_vEntries.push_back( te );
		// LOG->Trace( "Chose: %s, %d", te.pSong->GetSongDir().c_str(), te.pSteps->GetMeter() );

		if( IsAnEdit() && MAX_SONGS_IN_EDIT_COURSE > 0 &&
			int( trail.m_vEntries.size() ) >= MAX_SONGS_IN_EDIT_COURSE )
		{
			break;
		}
	}
}

void Course::GetTrails( vector<Trail*> &AddTo, StepsType st ) const
{
	FOREACH_ShownCourseDifficulty( cd )
	{
		Trail *pTrail = GetTrail( st, cd );
		if( pTrail == nullptr )
			continue;
		AddTo.push_back( pTrail );
	}
}

void Course::GetAllTrails( vector<Trail*> &AddTo ) const
{
	vector<StepsType> vStepsTypesToShow;
	GAMEMAN->GetStepsTypesForGame( GAMESTATE->m_pCurGame, vStepsTypesToShow );
	for (StepsType const &st : vStepsTypesToShow)
	{
		GetTrails( AddTo, st );
	}
}

int Course::GetMeter( StepsType st, CourseDifficulty cd ) const
{
	if( m_iCustomMeter[cd] != -1 )
		return m_iCustomMeter[cd];
	const Trail* pTrail = GetTrail( st );
	if( pTrail != nullptr )
		return pTrail->GetMeter();
	return 0;
}

bool Course::HasMods() const
{
	return std::any_of(m_vEntries.begin(), m_vEntries.end(), [](CourseEntry const &e) { return !e.attacks.empty(); });
}

bool Course::HasTimedMods() const
{
	// What makes this different from the SM4 implementation is that
	// HasTimedMods now searches for bGlobal in the attacks; if one of
	// them is false, it has timed mods. Also returning false will probably
	// take longer than expected. -aj
	for (CourseEntry const &e : m_vEntries)
	{
		if( e.attacks.empty() )
		{
			continue;
		}
		if (std::any_of(e.attacks.begin(), e.attacks.end(), [](Attack const &a) { return !a.bGlobal; }))
		{
			return true;
		}
	}
	return false;
}

bool Course::AllSongsAreFixed() const
{
	return std::all_of(m_vEntries.begin(), m_vEntries.end(), [](CourseEntry const &e) { return e.IsFixedSong(); });
}

const Style *Course::GetCourseStyle( const Game *pGame, int iNumPlayers ) const
{
	vector<const Style*> vpStyles;
	GAMEMAN->GetCompatibleStyles( pGame, iNumPlayers, vpStyles );

	for (Style const *pStyle : vpStyles)
	{
		for (RString const &style : m_setStyles)
		{
			if( !style.CompareNoCase(pStyle->m_szName) )
				return pStyle;
		}
	}
	return nullptr;
}

void Course::InvalidateTrailCache()
{
	m_TrailCache.clear();
}

void Course::Invalidate( const Song *pStaleSong )
{
	for (CourseEntry const &e : m_vEntries)
	{
		Song *pSong = e.songID.ToSong();
		if( pSong == pStaleSong )	// a fixed entry that references the stale Song
		{
			RevertFromDisk();
			return;
		}
	}

	// Invalidate any Trails that contain this song.
	// If we find a Trail that contains this song, then it's part of a 
	// non-fixed entry. So, regenerating the Trail will force different
	// songs to be chosen.
	FOREACH_ENUM( StepsType,st )
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

void Course::RegenerateNonFixedTrails() const
{
	// Only need to regen Trails if the Course has a random entry.
	// We can create these Trails on demand because we don't 
	// calculate RadarValues for Trails with one or more non-fixed 
	// entry.
	if( AllSongsAreFixed() )
		return;

	for (auto const & e: m_TrailCache)
	{
		const CacheEntry &ce = e.first;
		GetTrailForceRegenCache( ce.first, ce.second );
	}
}

RageColor Course::GetColor() const
{
	// FIXME: Calculate the meter.
	int iMeter = 5;

	switch( PREFSMAN->m_CourseSortOrder )
	{
	case COURSE_SORT_PREFERRED:
		return SORT_PREFERRED_COLOR;	//This will also be used for autogen'd courses in some cases.

	case COURSE_SORT_SONGS:	
		if( m_vEntries.size() >= 7 )		return SORT_LEVEL2_COLOR;
		else if( m_vEntries.size() >= 4 )	return SORT_LEVEL4_COLOR;
		else					return SORT_LEVEL5_COLOR;

	case COURSE_SORT_METER:
		if( !AllSongsAreFixed() )		return SORT_LEVEL1_COLOR;
		else if( iMeter > 9 )			return SORT_LEVEL2_COLOR;
		else if( iMeter >= 7 )			return SORT_LEVEL3_COLOR;
		else if( iMeter >= 5 )			return SORT_LEVEL4_COLOR;
		else 					return SORT_LEVEL5_COLOR;

	case COURSE_SORT_METER_SUM:
		if( !AllSongsAreFixed() )		return SORT_LEVEL1_COLOR;
		if( m_SortOrder_TotalDifficulty >= 40 )	return SORT_LEVEL2_COLOR;
		if( m_SortOrder_TotalDifficulty >= 30 )	return SORT_LEVEL3_COLOR;
		if( m_SortOrder_TotalDifficulty >= 20 )	return SORT_LEVEL4_COLOR;
		else					return SORT_LEVEL5_COLOR;

	case COURSE_SORT_RANK:
		if( m_SortOrder_Ranking == 3 )		return SORT_LEVEL1_COLOR;
		else if( m_SortOrder_Ranking == 2 )	return SORT_LEVEL3_COLOR;
		else if( m_SortOrder_Ranking == 1 )	return SORT_LEVEL5_COLOR;
		else					return SORT_LEVEL4_COLOR;
	default:
		FAIL_M( ssprintf("Invalid course sort %d.", int(PREFSMAN->m_CourseSortOrder)) );
		return RageColor(1,1,1,1);  // white; should never reach here
	}
}

bool Course::GetTotalSeconds( StepsType st, float& fSecondsOut ) const
{
	if( !AllSongsAreFixed() )
		return false;

	Trail* trail = GetTrail( st, Difficulty_Medium );
	if(!trail)
	{
		for(int cd= 0; cd < NUM_CourseDifficulty; ++cd)
		{
			trail= GetTrail(st, (CourseDifficulty)cd);
			if(trail)
			{
				break;
			}
		}
		if(!trail)
		{
			return false;
		}
	}

	fSecondsOut = trail->GetLengthSeconds();
	return true;
}

bool Course::CourseHasBestOrWorst() const
{
	for (CourseEntry const &e : m_vEntries)
	{
		if( e.songSort == SongSort_MostPlays  &&  e.iChooseIndex != -1 )
			return true;
		if( e.songSort == SongSort_FewestPlays  &&  e.iChooseIndex != -1 )
			return true;
	}

	return false;
}

RString Course::GetBannerPath() const
{
	if( m_sBannerPath.empty() )
		return RString();
	if( m_sBannerPath[0] == '/' )
		return m_sBannerPath;
	return Dirname(m_sPath) + m_sBannerPath;
}

RString Course::GetBackgroundPath() const
{
	if( m_sBackgroundPath.empty() )
		return RString();
	if( m_sBackgroundPath[0] == '/' )
		return m_sBackgroundPath;
	return Dirname(m_sPath) + m_sBackgroundPath;
}

bool Course::HasBanner() const
{
	return GetBannerPath() != ""  &&  IsAFile(GetBannerPath());
}

bool Course::HasBackground() const
{
	return GetBackgroundPath() != ""  &&  IsAFile(GetBackgroundPath());
}

void Course::UpdateCourseStats( StepsType st )
{
	m_SortOrder_TotalDifficulty = 0;

	// courses with random/players best-worst songs should go at the end
	for(unsigned i = 0; i < m_vEntries.size(); i++)
	{
		Song *pSong = m_vEntries[i].songID.ToSong();
		if( pSong != nullptr )
			continue;

		if ( m_SortOrder_Ranking == 2 )
			m_SortOrder_Ranking = 3;
		m_SortOrder_TotalDifficulty = INT_MAX;
		return;
	}

	const Trail* pTrail = GetTrail( st, Difficulty_Medium );

	m_SortOrder_TotalDifficulty += pTrail != nullptr? pTrail->GetTotalMeter():0;

	// OPTIMIZATION: Ranking info isn't dependent on style, so call it
	// sparingly. It's handled on startup and when themes change.

	LOG->Trace("%s: Total feet: %d",
		this->m_sMainTitle.c_str(),
		m_SortOrder_TotalDifficulty );
}

bool Course::IsRanking() const
{
	vector<RString> rankingsongs;

	split(THEME->GetMetric("ScreenRanking", "CoursesToShow"), ",", rankingsongs);

	for(unsigned i=0; i < rankingsongs.size(); i++)
		if (rankingsongs[i].EqualsNoCase(m_sPath))
			return true;

	return false;
}

const CourseEntry *Course::FindFixedSong( const Song *pSong ) const
{
	for (CourseEntry const &entry : m_vEntries)
	{
		Song *lSong = entry.songID.ToSong();
		if( pSong == lSong )
			return &entry;
	}

	return nullptr;
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
	// todo: make this a metric of course types not to show? -aj
	return !IsEndless();
}

void Course::CalculateRadarValues()
{
	FOREACH_ENUM( StepsType,st )
	{
		FOREACH_ENUM( CourseDifficulty,cd )
		{
			// For courses that aren't fixed, the radar values are meaningless.
			// Makes non-fixed courses have unknown radar values.
			if( AllSongsAreFixed() )
			{
				Trail *pTrail = GetTrail( st, cd );
				if( pTrail == nullptr )
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

bool Course::Matches( RString sGroup, RString sCourse ) const
{
	if( sGroup.size() && sGroup.CompareNoCase(this->m_sGroupName) != 0)
		return false;

	RString sFile = m_sPath;
	if( !sFile.empty() )
	{
		sFile.Replace("\\","/");
		vector<RString> bits;
		split( sFile, "/", bits );
		const RString &sLastBit = bits[bits.size()-1];
		if( sCourse.EqualsNoCase(sLastBit) )
			return true;
	}

	if( sCourse.EqualsNoCase(this->GetTranslitFullTitle()) )
		return true;

	return false;
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the CourseEntry. */ 
class LunaCourseEntry: public Luna<CourseEntry>
{
public:
	static int GetSong( T* p, lua_State *L )
	{
		if( p->songID.ToSong() )
			p->songID.ToSong()->PushSelf(L);
		else
			lua_pushnil(L);
		return 1;
	}
	DEFINE_METHOD( IsSecret, bSecret );
	DEFINE_METHOD( IsFixedSong, IsFixedSong() );
	DEFINE_METHOD( GetGainSeconds, fGainSeconds );
	DEFINE_METHOD( GetGainLives, iGainLives );
	DEFINE_METHOD( GetNormalModifiers, sModifiers );
	// GetTimedModifiers - table
	DEFINE_METHOD( GetNumModChanges, GetNumModChanges() );
	DEFINE_METHOD( GetTextDescription, GetTextDescription() );
	
	LunaCourseEntry()
	{
		ADD_METHOD( GetSong );
		// sm-ssc additions:
		ADD_METHOD( IsSecret );
		ADD_METHOD( IsFixedSong );
		ADD_METHOD( GetGainSeconds );
		ADD_METHOD( GetGainLives );
		ADD_METHOD( GetNormalModifiers );
		//ADD_METHOD( GetTimedModifiers );
		ADD_METHOD( GetNumModChanges );
		ADD_METHOD( GetTextDescription );
	}
};

LUA_REGISTER_CLASS( CourseEntry )

// Now for the Course bindings:
/** @brief Allow Lua to have access to the Course. */ 
class LunaCourse: public Luna<Course>
{
public:
	DEFINE_METHOD( GetPlayMode, GetPlayMode() )
	static int GetDisplayFullTitle( T* p, lua_State *L )	{ lua_pushstring(L, p->GetDisplayFullTitle() ); return 1; }
	static int GetTranslitFullTitle( T* p, lua_State *L )	{ lua_pushstring(L, p->GetTranslitFullTitle() ); return 1; }
	static int HasMods( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasMods() ); return 1; }
	static int HasTimedMods( T* p, lua_State *L )		{ lua_pushboolean( L, p->HasTimedMods() ); return 1; }
	DEFINE_METHOD( GetCourseType, GetCourseType() )
	static int GetCourseEntry(T* p, lua_State* L)
	{
		size_t id= static_cast<size_t>(IArg(1));
		if(id >= p->m_vEntries.size())
		{
			lua_pushnil(L);
		}
		else
		{
			p->m_vEntries[id].PushSelf(L);
		}
		return 1;
	}
	static int GetCourseEntries( T* p, lua_State *L )
	{
		vector<CourseEntry*> v;
		for( unsigned i = 0; i < p->m_vEntries.size(); ++i )
		{
			v.push_back(&p->m_vEntries[i]);
		}
		LuaHelpers::CreateTableFromArray<CourseEntry*>( v, L );
		return 1;
	}
	static int GetNumCourseEntries(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->m_vEntries.size());
		return 1;
	}
	static int GetAllTrails( T* p, lua_State *L )
	{
		vector<Trail*> v;
		p->GetAllTrails( v );
		LuaHelpers::CreateTableFromArray<Trail*>( v, L );
		return 1;
	}
	static int GetBannerPath( T* p, lua_State *L )		{ RString s = p->GetBannerPath(); if( s.empty() ) return 0; LuaHelpers::Push(L, s); return 1; }
	static int GetBackgroundPath( T* p, lua_State *L )		{ RString s = p->GetBackgroundPath(); if( s.empty() ) return 0; LuaHelpers::Push(L, s); return 1; }
	static int GetCourseDir( T* p, lua_State *L )			{ lua_pushstring(L, p->m_sPath ); return 1; }
	static int GetGroupName( T* p, lua_State *L )		{ lua_pushstring(L, p->m_sGroupName ); return 1; }
	static int IsAutogen( T* p, lua_State *L )		{ lua_pushboolean(L, p->m_bIsAutogen ); return 1; }
	static int GetEstimatedNumStages( T* p, lua_State *L )	{ lua_pushnumber(L, p->GetEstimatedNumStages() ); return 1; }
	static int GetScripter( T* p, lua_State *L )		{ lua_pushstring(L, p->m_sScripter ); return 1; }
	static int GetDescription( T* p, lua_State *L )		{ lua_pushstring(L, p->m_sDescription ); return 1; }
	static int GetTotalSeconds( T* p, lua_State *L )
	{
		StepsType st = Enum::Check<StepsType>(L, 1);
		float fTotalSeconds;
		if( !p->GetTotalSeconds(st, fTotalSeconds) )
			lua_pushnil( L );
		else
			lua_pushnumber( L, fTotalSeconds );
		return 1;
	}
	DEFINE_METHOD( IsEndless,		IsEndless() )
	DEFINE_METHOD( IsNonstop,		IsNonstop() )
	DEFINE_METHOD( IsOni,			IsOni() )
	DEFINE_METHOD( GetGoalSeconds,	m_fGoalSeconds )
	static int HasBanner( T* p, lua_State *L )		{ lua_pushboolean(L, p->HasBanner() ); return 1; }
	static int HasBackground( T* p, lua_State *L )	{ lua_pushboolean(L, p->HasBackground() ); return 1; }
	DEFINE_METHOD( IsAnEdit,		IsAnEdit() )
	static int IsPlayableIn( T* p, lua_State *L )
	{
		StepsType st = Enum::Check<StepsType>(L, 1);
		lua_pushboolean(L, p->IsPlayableIn( st ) );
		return 1;
	}
	DEFINE_METHOD( IsRanking, IsRanking() )
	DEFINE_METHOD( AllSongsAreFixed, AllSongsAreFixed() )

	LunaCourse()
	{
		ADD_METHOD( GetPlayMode );
		ADD_METHOD( GetDisplayFullTitle );
		ADD_METHOD( GetTranslitFullTitle );
		ADD_METHOD( HasMods );
		ADD_METHOD( HasTimedMods );
		ADD_METHOD( GetCourseType );
		ADD_METHOD( GetCourseEntry );
		ADD_METHOD( GetCourseEntries );
		ADD_METHOD(GetNumCourseEntries);
		ADD_METHOD( GetAllTrails );
		ADD_METHOD( GetBannerPath );
		ADD_METHOD( GetBackgroundPath );
		ADD_METHOD( GetCourseDir );
		ADD_METHOD( GetGroupName );
		ADD_METHOD( IsAutogen );
		ADD_METHOD( GetEstimatedNumStages );
		ADD_METHOD( GetScripter ); 
		ADD_METHOD( GetDescription ); 
		ADD_METHOD( GetTotalSeconds );
		ADD_METHOD( IsEndless );
		ADD_METHOD( IsNonstop );
		ADD_METHOD( IsOni );
		ADD_METHOD( GetGoalSeconds );
		ADD_METHOD( HasBanner );
		ADD_METHOD( HasBackground );
		ADD_METHOD( IsAnEdit );
		ADD_METHOD( IsPlayableIn );
		ADD_METHOD( IsRanking );
		ADD_METHOD( AllSongsAreFixed );
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
