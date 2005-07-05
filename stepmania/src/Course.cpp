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
#include "MsdFile.h"
#include "PlayerOptions.h"
#include "SongOptions.h"
#include "SongCacheIndex.h"
#include "RageUtil.h"
#include "TitleSubstitution.h"
#include "Steps.h"
#include "BannerCache.h"
#include "RageFile.h"
#include "ThemeManager.h"
#include "ProfileManager.h"
#include "Foreach.h"
#include "UnlockManager.h"
#include <limits.h>


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
	FOREACH_CONST( CourseEntry, m_vEntries, e )
	{
		if( e->fGainSeconds > 0 )
			return COURSE_TYPE_SURVIVAL;
	}
	return COURSE_TYPE_NONSTOP;
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

void Course::LoadFromCRSFile( CString sPath )
{
	Init();

	m_sPath = sPath;	// save path

	// save group name
	{
		CStringArray parts;
		split( sPath, "/", parts, false );
		if( parts.size() >= 4 )		// e.g. "/Courses/blah/fun.cvs"
			m_sGroupName = parts[parts.size()-2];
	}


	bool bUseCache = true;
	{
		/* First look in the cache for this course.  Don't bother
		 * honoring FastLoad for checking the course hash, since
		 * courses are normally grouped into a few directories, not
		 * one directory per course. XXX: if !FastLoad, regen
		 * cache if the used songs have changed */
		unsigned uHash = SONGINDEX->GetCacheHash( m_sPath );
		if( !DoesFileExist(GetCacheFilePath()) )
			bUseCache = false;
		if( !PREFSMAN->m_bFastLoad && GetHashForDirectory(m_sPath) != uHash )
			bUseCache = false; // this cache is out of date 
	}

	if( bUseCache )
	{
		CString sCacheFile = GetCacheFilePath();
		LOG->Trace( "Course::LoadFromCRSFile(\"%s\") (\"%s\")", sPath.c_str(), sCacheFile.c_str() );
		sPath = sCacheFile.c_str();
	}
	else
	{
		LOG->Trace( "Course::LoadFromCRSFile(\"%s\")", sPath.c_str() );
	}

	MsdFile msd;
	if( !msd.ReadFile(sPath) )
		RageException::Throw( "Error opening CRS file '%s'.", sPath.c_str() );

	const CString sFName = SetExtension( m_sPath, "" );

	CStringArray arrayPossibleBanners;
	GetDirListing( sFName + "*.png", arrayPossibleBanners, false, true );
	GetDirListing( sFName + "*.jpg", arrayPossibleBanners, false, true );
	GetDirListing( sFName + "*.bmp", arrayPossibleBanners, false, true );
	GetDirListing( sFName + "*.gif", arrayPossibleBanners, false, true );
	if( !arrayPossibleBanners.empty() )
		m_sBannerPath = arrayPossibleBanners[0];

	AttackArray attacks;
	float fGainSeconds = 0;
	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		CString sValueName = msd.GetParam(i, 0);
		const MsdFile::value_t &sParams = msd.GetValue(i);

		// handle the data
		if( 0 == stricmp(sValueName, "COURSE") )
			m_sMainTitle = sParams[1];
		else if( 0 == stricmp(sValueName, "COURSETRANSLIT") )
			m_sMainTitleTranslit = sParams[1];
		else if( 0 == stricmp(sValueName, "REPEAT") )
		{
			CString str = sParams[1];
			str.MakeLower();
			if( str.Find("yes") != -1 )
				m_bRepeat = true;
		}

		else if( 0 == stricmp(sValueName, "LIVES") )
			m_iLives = atoi( sParams[1] );

		else if( 0 == stricmp(sValueName, "GAINSECONDS") )
			fGainSeconds = strtof( sParams[1], NULL );

		else if( 0 == stricmp(sValueName, "METER") )
		{
			if( sParams.params.size() == 2 )
				m_iCustomMeter[DIFFICULTY_MEDIUM] = atoi( sParams[1] ); /* compat */
			else if( sParams.params.size() == 3 )
			{
				const CourseDifficulty cd = StringToCourseDifficulty( sParams[1] );
				if( cd == DIFFICULTY_INVALID )
				{
					LOG->Warn( "Course file '%s' contains an invalid #METER string: \"%s\"",
								sPath.c_str(), sParams[1].c_str() );
					continue;
				}
				m_iCustomMeter[cd] = atoi( sParams[2] );
			}
		}

		else if( 0 == stricmp(sValueName, "MODS") )
		{
			Attack attack;
			float end = -9999;
			for( unsigned j = 1; j < sParams.params.size(); ++j )
			{
				CStringArray sBits;
				split( sParams[j], "=", sBits, false );
				if( sBits.size() < 2 )
					continue;

				TrimLeft( sBits[0] );
				TrimRight( sBits[0] );
				if( !sBits[0].CompareNoCase("TIME") )
					attack.fStartSecond = strtof( sBits[1], NULL );
				else if( !sBits[0].CompareNoCase("LEN") )
					attack.fSecsRemaining = strtof( sBits[1], NULL );
				else if( !sBits[0].CompareNoCase("END") )
					end = strtof( sBits[1], NULL );
				else if( !sBits[0].CompareNoCase("MODS") )
				{
					attack.sModifiers = sBits[1];
					if( end != -9999 )
					{
						ASSERT_M( end >= attack.fStartSecond, ssprintf("Attack ends before it starts.  end %f, start %f", end, attack.fStartSecond) );
						attack.fSecsRemaining = end - attack.fStartSecond;
						end = -9999;
					}
					
					// warn on invalid so we catch bogus mods on load
					PlayerOptions po;
					po.FromString( attack.sModifiers, true );

					attacks.push_back( attack );
				}
				else
				{
					LOG->Warn( "Unexpected value named '%s'", sBits[0].c_str() );
				}
			}

				
		}
		else if( 0 == stricmp(sValueName, "SONG") )
		{
			CourseEntry new_entry;

			// infer entry::Type from the first param
			if( sParams[1].Left(strlen("BEST")) == "BEST" )
			{
				new_entry.iChooseIndex = atoi( sParams[1].Right(sParams[1].size()-strlen("BEST")) ) - 1;
				CLAMP( new_entry.iChooseIndex, 0, 500 );
				new_entry.songSort = SongSort_MostPlays;
			}
			else if( sParams[1].Left(strlen("WORST")) == "WORST" )
			{
				new_entry.iChooseIndex = atoi( sParams[1].Right(sParams[1].size()-strlen("WORST")) ) - 1;
				CLAMP( new_entry.iChooseIndex, 0, 500 );
				new_entry.songSort = SongSort_FewestPlays;
			}
			else if( sParams[1] == "*" )
			{
				new_entry.bSecret = true;
			}
			else if( sParams[1].Right(1) == "*" )
			{
				new_entry.bSecret = true;
				CString sSong = sParams[1];
				sSong.Replace( "\\", "/" );
				CStringArray bits;
				split( sSong, "/", bits );
				if( bits.size() == 2 )
				{
					new_entry.sSongGroup = bits[0];
				}
				else
				{
					LOG->Warn( "Course file '%s' contains a random_within_group entry '%s' that is invalid. "
								"Song should be in the format '<group>/*'.",
								sPath.c_str(), sSong.c_str());
				}

				if( !SONGMAN->DoesSongGroupExist(new_entry.sSongGroup) )
				{
					/* XXX: We need a place to put "user warnings".  This is too loud for info.txt--
				     * it obscures important warnings--and regular users never look there, anyway. */
					LOG->Trace( "Course file '%s' random_within_group entry '%s' specifies a group that doesn't exist. "
								"This entry will be ignored.",
								sPath.c_str(), sSong.c_str());
					continue;	// skip this #SONG
				}
			}
			else
			{
				CString sSong = sParams[1];
				new_entry.pSong = SONGMAN->FindSong( sSong );

				if( new_entry.pSong == NULL )
				{
					/* XXX: We need a place to put "user warnings".  This is too loud for info.txt--
				     * it obscures important warnings--and regular users never look there, anyway. */
					LOG->Trace( "Course file '%s' contains a fixed song entry '%s' that does not exist. "
								"This entry will be ignored.",
								sPath.c_str(), sSong.c_str());
					continue;	// skip this #SONG
				}
			}

			new_entry.baseDifficulty = StringToDifficulty( sParams[2] );
			if( new_entry.baseDifficulty == DIFFICULTY_INVALID )
			{
				int retval = sscanf( sParams[2], "%d..%d", &new_entry.iLowMeter, &new_entry.iHighMeter );
				if( retval == 1 )
					new_entry.iHighMeter = new_entry.iLowMeter;
				else if( retval != 2 )
				{
					LOG->Warn("Course file '%s' contains an invalid difficulty setting: \"%s\", 3..6 used instead",
						sPath.c_str(), sParams[2].c_str());
					new_entry.iLowMeter = 3;
					new_entry.iHighMeter = 6;
				}
			}

			{
				/* If "showcourse" or "noshowcourse" is in the list, force new_entry.secret 
				 * on or off. */
				CStringArray mods;
				split( sParams[3], ",", mods, true );
				for( int j = (int) mods.size()-1; j >= 0 ; --j )
				{
					CString &sMod = mods[j];
					TrimLeft( sMod );
					TrimRight( sMod );
					if( !sMod.CompareNoCase("showcourse") )
						new_entry.bSecret = false;
					else if( !sMod.CompareNoCase("noshowcourse") )
						new_entry.bSecret = true;
					else if( !sMod.CompareNoCase("nodifficult") )
						new_entry.bNoDifficult = true;
					else 
						continue;
					mods.erase(mods.begin() + j);
				}
				new_entry.sModifiers = join( ",", mods );
			}

			new_entry.attacks = attacks;
			new_entry.fGainSeconds = fGainSeconds;
			attacks.clear();
			
			m_vEntries.push_back( new_entry );
		}
		else if( bUseCache && !stricmp(sValueName, "RADAR") )
		{
			StepsType st = (StepsType) atoi(sParams[1]);
			CourseDifficulty cd = (CourseDifficulty) atoi(sParams[2]);

			RadarValues rv;
			rv.FromString( sParams[3] );
			m_RadarCache[CacheEntry(st, cd)] = rv;
		}
		else
		{
			LOG->Warn( "Unexpected value named '%s'", sValueName.c_str() );
		}
	}
	static TitleSubst tsub("courses");

	TitleFields title;
	title.Title = m_sMainTitle;
	title.TitleTranslit = m_sMainTitleTranslit;
	tsub.Subst( title );
	m_sMainTitle = title.Title;
	m_sMainTitleTranslit = title.TitleTranslit;

	/* Cache and load the course banner.  Only bother doing this if at least one
	 * song was found in the course. */
	if( m_sBannerPath != "" && !m_vEntries.empty() )
		BANNERCACHE->CacheBanner( m_sBannerPath );

	/* Cache each trail RadarValues that's slow to load, so we
	 * don't have to do it at runtime. */
	if( !bUseCache )
	{
		CalculateRadarValues();

		/* If we have any cache data, write the cache file. */
		if( m_RadarCache.size() )
		{
			CString sCachePath = GetCacheFilePath();
			Save( sCachePath, true );

			SONGINDEX->AddCacheIndex( m_sPath, GetHashForFile(m_sPath) );
		}
	}
}

void Course::RevertFromDisk()
{
	// trying to catch invalid an Course
	ASSERT( !m_sPath.empty() );

	LoadFromCRSFile( m_sPath );
}

CString Course::GetCacheFilePath() const
{
	return SongCacheIndex::GetCacheFilePath( "Courses", m_sPath );
}

void Course::Init()
{
	m_bIsAutogen = false;
	m_bRepeat = false;
	m_bRandomize = false;
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
	m_iTrailCacheSeed = 0;
}

void Course::Save( CString sPath, bool bSavingCache )
{
	ASSERT( !m_bIsAutogen );

	/* By default, save to the file we loaded from. */
	if( sPath == "" )
		sPath = m_sPath;

	RageFile f;
	if( !f.Open( sPath, RageFile::WRITE ) )
	{
		LOG->Warn( "Could not write course file '%s': %s", sPath.c_str(), f.GetError().c_str() );
		return;
	}

	f.PutLine( ssprintf("#COURSE:%s;", m_sMainTitle.c_str()) );
	if( m_sMainTitleTranslit != "" )
		f.PutLine( ssprintf("#COURSETRANSLIT:%s;", m_sMainTitleTranslit.c_str()) );
	if( m_bRepeat )
		f.PutLine( "#REPEAT:YES;" );
	if( m_iLives != -1 )
		f.PutLine( ssprintf("#LIVES:%i;", m_iLives) );

	FOREACH_CourseDifficulty( cd )
	{
		if( m_iCustomMeter[cd] == -1 )
			continue;
		f.PutLine( ssprintf("#METER:%s:%i;", CourseDifficultyToString(cd).c_str(), m_iCustomMeter[cd]) );
	}

	if( bSavingCache )
	{
		f.PutLine( "// cache tags:" );

		RadarCache_t::const_iterator it;
		for( it = m_RadarCache.begin(); it != m_RadarCache.end(); ++it )
		{
			// #RADAR:type:difficulty:value,value,value...;
			const CacheEntry &entry = it->first;
			StepsType st = entry.first;
			CourseDifficulty cd = entry.second;

			CStringArray asRadarValues;
			const RadarValues &rv = it->second;
			for( int r=0; r < NUM_RADAR_CATEGORIES; r++ )
				asRadarValues.push_back( ssprintf("%.3f", rv[r]) );
			CString sLine = ssprintf( "#RADAR:%i:%i:", st, cd );
			sLine += join( ",", asRadarValues ) + ";";
			f.PutLine( sLine );
		}
		f.PutLine( "// end cache tags" );
	}

	for( unsigned i=0; i<m_vEntries.size(); i++ )
	{
		const CourseEntry& entry = m_vEntries[i];

		for( unsigned j = 0; j < entry.attacks.size(); ++j )
		{
			if( j == 0 )
				f.PutLine( "#MODS:" );

			const Attack &a = entry.attacks[j];
			f.Write( ssprintf( "  TIME=%.2f:LEN=%.2f:MODS=%s",
				a.fStartSecond, a.fSecsRemaining, a.sModifiers.c_str() ) );

			if( j+1 < entry.attacks.size() )
				f.Write( ":" );
			else
				f.Write( ";" );
			f.PutLine( "" );
		}

		if( entry.fGainSeconds > 0 )
			f.PutLine( ssprintf("#GAINSECONDS:%f;", entry.fGainSeconds) );

		if( entry.songSort == SongSort_MostPlays  &&  entry.iChooseIndex != -1 )
		{
			f.Write( ssprintf( "#SONG:BEST%d", entry.iChooseIndex+1 ) );
		}
		else if( entry.songSort == SongSort_FewestPlays  &&  entry.iChooseIndex != -1 )
		{
			f.Write( ssprintf( "#SONG:WORST%d", entry.iChooseIndex+1 ) );
		}
		else if( entry.pSong )
		{
			// strip off everything but the group name and song dir
			CStringArray as;
			ASSERT( entry.pSong != NULL );
			split( entry.pSong->GetSongDir(), "/", as );
			ASSERT( as.size() >= 2 );
			CString sGroup = as[ as.size()-2 ];
			CString sSong = as[ as.size()-1 ];
			f.Write( "#SONG:" + sGroup + '/' + sSong );
		}
		else if( !entry.sSongGroup.empty() )
		{
			f.Write( ssprintf( "#SONG:%s/*", entry.sSongGroup.c_str() ) );
		}
		else 
		{
			f.Write( "#SONG:*" );
		}

		f.Write( ":" );
		if( entry.baseDifficulty != DIFFICULTY_INVALID )
			f.Write( DifficultyToString(entry.baseDifficulty) );
		else if( entry.iLowMeter != -1  &&  entry.iHighMeter != -1 )
			f.Write( ssprintf( "%d..%d", entry.iLowMeter, entry.iHighMeter ) );
		f.Write( ":" );

		CString sModifiers = entry.sModifiers;
		bool bDefaultSecret = entry.IsRandomSong();
		if( bDefaultSecret != entry.bSecret )
		{
			if( sModifiers != "" )
				sModifiers += ",";
			sModifiers += entry.bSecret? "noshowcourse":"showcourse";
		}

		if( entry.bNoDifficult )
		{
			if( sModifiers != "" )
				sModifiers += ",";
			sModifiers += "nodifficult";
		}
		f.Write( sModifiers );

		f.PutLine( ";" );
	}
}


void Course::AutogenEndlessFromGroup( CString sGroupName, Difficulty diff )
{
	m_bIsAutogen = true;
	m_bRepeat = true;
	m_bRandomize = true;
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
	m_bRandomize = true;
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
	if( m_bRandomize )
	{
		/* Always randomize the same way per round.  Otherwise, the displayed course
		 * will change every time it's viewed, and the displayed order will have no
		 * bearing on what you'll actually play. */
		tmp_entries = m_vEntries;
		random_shuffle( tmp_entries.begin(), tmp_entries.end(), rnd );
	}

	const vector<CourseEntry> &entries = m_bRandomize? tmp_entries:m_vEntries;

	/* This can take some time, so don't fill it out unless we need it. */
	vector<Song*> vSongsByMostPlayed;
	vector<Song*> AllSongsShuffled;

	trail.m_StepsType = st;
	trail.m_CourseDifficulty = cd;

	/* Set to true if CourseDifficulty is able to change something. */
	bool bCourseDifficultyIsSignificant = (cd == DIFFICULTY_MEDIUM);

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
		
		if( e->pSong )
		{
			// Choose an exact song
			vpPossibleSongs.push_back( e->pSong );
		}
		else
		{
			vpPossibleSongs = SONGMAN->GetAllSongs();

			FOREACH( Song*, vpPossibleSongs, song )
			{
				// Ignore locked songs when choosing randomly
				// TODO: Move Course initialization after UNLOCKMAN is created
				if( UNLOCKMAN  &&  UNLOCKMAN->SongIsLocked(*song) )
				{
					vector<Song*>::iterator eraseme = song;
					song--;
					vpPossibleSongs.erase( eraseme );
					continue;
				}

				// Ignore boring tutorial songs
				if( (*song)->IsTutorial() )
				{
					vector<Song*>::iterator eraseme = song;
					song--;
					vpPossibleSongs.erase( eraseme );
					continue;
				}

				// Don't allow long songs
				if( SONGMAN->GetNumStagesForSong(*song) > 1 )
				{
					vector<Song*>::iterator eraseme = song;
					song--;
					vpPossibleSongs.erase( eraseme );
					continue;
				}
			}
		}


		// Filter out all songs that don't have matching steps
		// At the same time, create a list of matching song/steps.
		typedef vector<Steps*> StepsVector;
		map<Song*,StepsVector> mapSongToSteps;
		FOREACH( Song*, vpPossibleSongs, song )
		{
			// Apply song group filter
			if( !e->sSongGroup.empty()  &&  (*song)->m_sGroupName != e->sSongGroup )
			{
				vector<Song*>::iterator eraseme = song;
				song--;
				vpPossibleSongs.erase( eraseme );
				continue;
			}

			vector<Steps*> vpMatchingSteps;
			(*song)->GetSteps( vpMatchingSteps, st );
	
			FOREACH( Steps*, vpMatchingSteps, steps )
			{
				if( e->baseDifficulty != DIFFICULTY_INVALID )
				{
					Difficulty dc = e->baseDifficulty;
					if( (*steps)->GetDifficulty() != dc )
					{
						vector<Steps*>::iterator eraseme = steps;
						steps--;
						vpMatchingSteps.erase( eraseme );
						continue;
					}
				}

				if( e->iLowMeter != -1 )
				{
					if( (*steps)->GetMeter() < e->iLowMeter )
					{
						vector<Steps*>::iterator eraseme = steps;
						steps--;
						vpMatchingSteps.erase( eraseme );
						continue;
					}
				}

				if( e->iHighMeter != -1 )
				{
					if( (*steps)->GetMeter() > e->iHighMeter )
					{
						vector<Steps*>::iterator eraseme = steps;
						steps--;
						vpMatchingSteps.erase( eraseme );
						continue;
					}
				}
			}

			if( vpMatchingSteps.empty() )
			{
				vector<Song*>::iterator eraseme = song;
				song--;
				vpPossibleSongs.erase( eraseme );
				continue;
			}

			mapSongToSteps[*song] = vpMatchingSteps;
		}


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
			vector<Steps*> &vpPossibleSteps = mapSongToSteps[pResolvedSong];
			ASSERT( !vpPossibleSteps.empty() );	// if no steps are playable, this shouldn't be a possible song
			random_shuffle( vpPossibleSteps.begin(), vpPossibleSteps.end(), rnd );
			pResolvedSteps = vpPossibleSteps[0];
		}


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

	/* Hack: If any entry was non-FIXED, or m_bRandomize is set, then radar values
	 * for this trail will be meaningless as they'll change every time.  Pre-cache
	 * empty data.  XXX: How can we do this cleanly, without propagating lots of
	 * otherwise unnecessary data (course entry types, m_bRandomize) to Trail, or
	 * storing a Course pointer in Trail (yuck)? */
	if( !AllSongsAreFixed() || m_bRandomize )
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
		if ( m_vEntries[i].pSong == NULL )
			continue;

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

	static void Register(lua_State *L)
	{
		ADD_METHOD( GetPlayMode )
		ADD_METHOD( GetDisplayFullTitle )
		ADD_METHOD( GetTranslitFullTitle )
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
