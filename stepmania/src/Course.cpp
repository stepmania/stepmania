#include "global.h"
#include "Course.h"
#include "PrefsManager.h"
#include "song.h"
#include "GameManager.h"
#include "SongManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageLog.h"
#include "MsdFile.h"
#include "PlayerOptions.h"
#include "SongOptions.h"
#include "RageUtil.h"
#include "TitleSubstitution.h"
#include "Steps.h"
#include "BannerCache.h"
#include "RageFile.h"
#include "arch/arch.h"
#include "ThemeManager.h"
#include "ProfileManager.h"
#include "Foreach.h"

/* Amount to increase meter ranges to make them difficult: */
const int COURSE_DIFFICULTY_CLASS_CHANGE[NUM_DIFFICULTIES] = { -1, -1, 0, 1, 1 };

/* Maximum lower value of ranges when difficult: */
const int MAX_BOTTOM_RANGE = 10;


Course::Course()
{
	Init();
}

PlayMode Course::GetPlayMode() const
{
	if( m_bRepeat )
		return PLAY_MODE_ENDLESS;
	return m_iLives > 0? PLAY_MODE_ONI:PLAY_MODE_NONSTOP;
}

void Course::LoadFromCRSFile( CString sPath )
{
	LOG->Trace( "Course::LoadFromCRSFile( '%s' )", sPath.c_str() );

	Init();

	m_sPath = sPath;	// save path

	MsdFile msd;
	if( !msd.ReadFile(sPath) )
		RageException::Throw( "Error opening CRS file '%s'.", sPath.c_str() );

	const CString sFName = SetExtension( sPath, "" );

	CStringArray arrayPossibleBanners;
	GetDirListing( sFName + ".png", arrayPossibleBanners, false, true );
	GetDirListing( sFName + ".jpg", arrayPossibleBanners, false, true );
	GetDirListing( sFName + ".bmp", arrayPossibleBanners, false, true );
	GetDirListing( sFName + ".gif", arrayPossibleBanners, false, true );
	if( !arrayPossibleBanners.empty() )
	{
		m_sBannerPath = arrayPossibleBanners[0];

		/* Cache and load the course banner. */
		BANNERCACHE->CacheBanner( m_sBannerPath );
	}

	AttackArray attacks;
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
								m_sPath.c_str(), sParams[1].c_str() );
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
					if( end != -9999 )
					{
						attack.fSecsRemaining = end - attack.fStartSecond;
						end = -9999;
					}
					attack.sModifier = sBits[1];
					attacks.push_back( attack );
				}
			}

				
		}
		else if( 0 == stricmp(sValueName, "SONG") )
		{
			CourseEntry new_entry;

			// infer entry::Type from the first param
			if( sParams[1].Left(strlen("BEST")) == "BEST" )
			{
				new_entry.type = COURSE_ENTRY_BEST;
				new_entry.players_index = atoi( sParams[1].Right(sParams[1].size()-strlen("BEST")) ) - 1;
				CLAMP( new_entry.players_index, 0, 500 );
			}
			else if( sParams[1].Left(strlen("WORST")) == "WORST" )
			{
				new_entry.type = COURSE_ENTRY_WORST;
				new_entry.players_index = atoi( sParams[1].Right(sParams[1].size()-strlen("WORST")) ) - 1;
				CLAMP( new_entry.players_index, 0, 500 );
			}
			else if( sParams[1] == "*" )
			{
				new_entry.mystery = true;
				new_entry.type = COURSE_ENTRY_RANDOM;
			}
			else if( sParams[1].Right(1) == "*" )
			{
				new_entry.mystery = true;
				new_entry.type = COURSE_ENTRY_RANDOM_WITHIN_GROUP;
				CString sSong = sParams[1];
				sSong.Replace( "\\", "/" );
				CStringArray bits;
				split( sSong, "/", bits );
				if( bits.size() == 2 )
					new_entry.group_name = bits[0];
				else
					LOG->Warn( "Course file '%s' contains a random_within_group entry '%s' that is invalid. "
								"Song should be in the format '<group>/*'.",
								m_sPath.c_str(), sSong.c_str());
				if( !SONGMAN->DoesGroupExist(new_entry.group_name) )
				{
					/* XXX: We need a place to put "user warnings".  This is too loud for info.txt--
				     * it obscures important warnings--and regular users never look there, anyway. */
					LOG->Trace( "Course file '%s' random_within_group entry '%s' specifies a group that doesn't exist. "
								"This entry will be ignored.",
								m_sPath.c_str(), sSong.c_str());
					continue;	// skip this #SONG
				}
			}
			else
			{
				new_entry.type = COURSE_ENTRY_FIXED;

				CString sSong = sParams[1];
				new_entry.pSong = SONGMAN->FindSong( sSong );

				if( new_entry.pSong == NULL )
				{
					/* XXX: We need a place to put "user warnings".  This is too loud for info.txt--
				     * it obscures important warnings--and regular users never look there, anyway. */
					LOG->Trace( "Course file '%s' contains a fixed song entry '%s' that does not exist. "
								"This entry will be ignored.",
								m_sPath.c_str(), sSong.c_str());
					continue;	// skip this #SONG
				}
			}

			new_entry.difficulty = StringToDifficulty( sParams[2] );
			if( new_entry.difficulty == DIFFICULTY_INVALID )
			{
				int retval = sscanf( sParams[2], "%d..%d", &new_entry.low_meter, &new_entry.high_meter );
				if( retval == 1 )
					new_entry.high_meter = new_entry.low_meter;
				else if( retval != 2 )
				{
					LOG->Warn("Course file '%s' contains an invalid difficulty setting: \"%s\", 3..6 used instead",
						m_sPath.c_str(), sParams[2].c_str());
					new_entry.low_meter = 3;
					new_entry.high_meter = 6;
				}
			}

			{
				/* If "showcourse" or "noshowcourse" is in the list, force new_entry.mystery 
				 * on or off. */
				CStringArray mods;
				split( sParams[3], ",", mods, true );
				for( int j = (int) mods.size()-1; j >= 0 ; --j )
				{
					if( !mods[j].CompareNoCase("showcourse") )
						new_entry.mystery = false;
					else if( !mods[j].CompareNoCase("noshowcourse") )
						new_entry.mystery = true;
					else if( !mods[j].CompareNoCase("nodifficult") )
						new_entry.no_difficult = true;
					else 
						continue;
					mods.erase(mods.begin() + j);
				}
				new_entry.modifiers = join( ",", mods );
			}

			new_entry.attacks = attacks;
			attacks.clear();
			
			m_entries.push_back( new_entry );
		}

		else
			LOG->Trace( "Unexpected value named '%s'", sValueName.c_str() );
	}
	static TitleSubst tsub("courses");

	TitleFields title;
	title.Title = m_sMainTitle;
	title.TitleTranslit = m_sMainTitleTranslit;
	tsub.Subst( title );
	m_sMainTitle = title.Title;
	m_sMainTitleTranslit = title.TitleTranslit;
}

void Course::RevertFromDisk()
{
	// trying to catch invalid an Course
	ASSERT( !m_sPath.empty() );

	LoadFromCRSFile( m_sPath );
}

void Course::Init()
{
	m_bIsAutogen = false;
	m_bRepeat = false;
	m_bRandomize = false;
	m_iLives = -1;
	m_bSortByMeter = false;
	m_entries.clear();
	FOREACH_Difficulty(dc)
		m_iCustomMeter[dc] = -1;
	m_entries.clear();
	m_sPath = "";
	m_sMainTitle = "";
	m_sMainTitleTranslit = "";
	m_sSubTitle = "";
	m_sSubTitleTranslit = "";
	m_sBannerPath = "";
	m_sCDTitlePath = "";
	ZERO( m_TrailCacheValid );
	m_iTrailCacheSeed = 0;
}

void Course::Save()
{
	ASSERT( !m_bIsAutogen );

	RageFile f;
	if( !f.Open( m_sPath, RageFile::WRITE ) )
	{
		LOG->Warn( "Could not write course file '%s': %s", m_sPath.c_str(), f.GetError().c_str() );
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

	for( unsigned i=0; i<m_entries.size(); i++ )
	{
		const CourseEntry& entry = m_entries[i];

		for( unsigned j = 0; j < entry.attacks.size(); ++j )
		{
			if( j == 0 )
				f.PutLine( "#MODS:" );

			CString line;
			const Attack &a = entry.attacks[j];
			line += ssprintf( "  TIME=%.2f:LEN=%.2f:MODS=%s",
				a.fStartSecond, a.fSecsRemaining, a.sModifier.c_str() );

			if( j+1 < entry.attacks.size() )
				line += ":";
			else
				line += ";";
			f.PutLine( line );
		}

		CString line;
		switch( entry.type )
		{
		case COURSE_ENTRY_FIXED:
			{
				// strip off everything but the group name and song dir
				CStringArray as;
				ASSERT( entry.pSong != NULL );
				split( entry.pSong->GetSongDir(), "/", as );
				ASSERT( as.size() >= 2 );
				CString sGroup = as[ as.size()-2 ];
				CString sSong = as[ as.size()-1 ];
				line += "#SONG:" + sGroup + '/' + sSong;
			}
			break;
		case COURSE_ENTRY_RANDOM:
			line += "#SONG:*";
			break;
		case COURSE_ENTRY_RANDOM_WITHIN_GROUP:
			line += ssprintf( "#SONG:%s/*", entry.group_name.c_str() );
			break;
		case COURSE_ENTRY_BEST:
			line += ssprintf( "#SONG:BEST%d", entry.players_index+1 );
			break;
		case COURSE_ENTRY_WORST:
			line += ssprintf( "#SONG:WORST%d", entry.players_index+1 );
			break;
		default:
			ASSERT(0);
		}

		line += ":";
		if( entry.difficulty != DIFFICULTY_INVALID )
			line += DifficultyToString(entry.difficulty);
		else if( entry.low_meter != -1  &&  entry.high_meter != -1 )
			line += ssprintf( "%d..%d", entry.low_meter, entry.high_meter );
		line += ":";

		CString modifiers = entry.modifiers;
		bool default_mystery = (entry.type == COURSE_ENTRY_RANDOM || entry.type == COURSE_ENTRY_RANDOM_WITHIN_GROUP);
		if( default_mystery != entry.mystery )
		{
			if( modifiers != "" )
				modifiers += ",";
			modifiers += entry.mystery? "noshowcourse":"showcourse";
		}

		if( entry.no_difficult )
		{
			if( modifiers != "" )
				modifiers += ",";
			modifiers += "nodifficult";
		}
		line += modifiers;

		line += ";";
		f.PutLine( line );
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
	} else {
		m_sMainTitle = SONGMAN->ShortenGroupName( sGroupName );
		m_sBannerPath = SONGMAN->GetGroupBannerPath( sGroupName );
	}

	// We want multiple songs, so we can try to prevent repeats during
	// gameplay. (We might still get a repeat at the repeat boundary,
	// but that'd be rare.) -glenn
	CourseEntry e;
	if( sGroupName != "" )
		e.type = COURSE_ENTRY_RANDOM_WITHIN_GROUP;
	else
		e.type = COURSE_ENTRY_RANDOM;

	e.group_name = sGroupName;
	e.difficulty = diff;
	e.mystery = true;

	vector<Song*> vSongs;
	SONGMAN->GetSongs( vSongs, e.group_name );
	for( unsigned i = 0; i < vSongs.size(); ++i)
		m_entries.push_back( e );
}

void Course::AutogenNonstopFromGroup( CString sGroupName, Difficulty diff )
{
	AutogenEndlessFromGroup( sGroupName, diff );

	m_bRepeat = false;

	m_sMainTitle += " Random";	

	// resize to 4
	while( m_entries.size() < 4 )
		m_entries.push_back( m_entries[0] );
	while( m_entries.size() > 4 )
		m_entries.pop_back();
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
	e.type = COURSE_ENTRY_FIXED;
	e.difficulty = dc;

	for( unsigned i = 0; i < aSongs.size(); ++i )
	{
		e.pSong = aSongs[i];
		m_entries.push_back( e );
	}
}

bool Course::IsPlayableIn( StepsType st ) const
{
	return GetTrail( st ) != NULL;
}

static vector<Song*> GetFilteredBestSongs( StepsType st )
{
	const vector<Song*> &vSongsByMostPlayed = SONGMAN->GetBestSongs();
	vector<Song*> ret;
	ret.reserve( vSongsByMostPlayed.size() );

	for( unsigned i=0; i < vSongsByMostPlayed.size(); ++i )
	{
		// filter out long songs and songs that don't have both medium and hard steps
		Song* pSong = vSongsByMostPlayed[i];
		if( SONGMAN->GetNumStagesForSong(pSong) > 1 )
			continue;

		const vector<Steps*>& vpSteps = pSong->GetAllSteps();

		bool FoundMedium = false, FoundHard = false;
		FOREACH_CONST( Steps*, vpSteps, pSteps )
		{
			if( (*pSteps)->m_StepsType != st )
				continue;
			if( !PREFSMAN->m_bAutogenSteps && (*pSteps)->IsAutogen() )
				continue;

			if( (*pSteps)->GetDifficulty() == DIFFICULTY_MEDIUM )
				FoundMedium = true;
			else if( (*pSteps)->GetDifficulty() == DIFFICULTY_HARD )
				FoundHard = true;

			if( FoundMedium && FoundHard )
				break;
		}
		if( !FoundMedium || !FoundHard )
			continue;

		ret.push_back( pSong );
	}

	return ret;
}

struct SortTrailEntry
{
	TrailEntry entry;
	int SortMeter;
	bool operator< ( const SortTrailEntry &rhs ) const { return SortMeter < rhs.SortMeter; }
};

CString Course::GetDisplayMainTitle() const
{
	if( !PREFSMAN->m_bShowNative )
		return GetTranslitMainTitle();
	return m_sMainTitle;
}

CString Course::GetDisplaySubTitle() const
{
	if( !PREFSMAN->m_bShowNative )
		return GetTranslitSubTitle();
	return m_sSubTitle;
}

CString Course::GetFullDisplayTitle() const
{
	CString Title = GetDisplayMainTitle();
	CString SubTitle = GetDisplaySubTitle();

	if(!SubTitle.empty()) Title += " " + SubTitle;
	return Title;
}

CString Course::GetFullTranslitTitle() const
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
	if( m_iTrailCacheSeed != GAMESTATE->m_iRoundSeed )
	{
		/* If we have any random entries (so that the seed matters), invalidate the cache. */
		bool bHaveRandom = false;
		for( unsigned i=0; !bHaveRandom && i<m_entries.size(); i++ )
			if( m_entries[i].type == COURSE_ENTRY_RANDOM ||
				m_entries[i].type == COURSE_ENTRY_RANDOM_WITHIN_GROUP )
				bHaveRandom = true;
		
		if( bHaveRandom )
			ZERO( m_TrailCacheValid );

		m_iTrailCacheSeed = GAMESTATE->m_iRoundSeed;
	}

	//
	// Look in the Trail cache
	//
	if( m_TrailCacheValid[st][cd] )
	{
		if( m_TrailCacheNull[st][cd] )
			return NULL;
		return &m_TrailCache[st][cd];
	}

	//
	// Construct a new Trail, add it to the cache, then return it.
	//
	Trail &trail = m_TrailCache[st][cd];
	trail = Trail();
	if( !GetTrailSorted( st, cd, trail ) || trail.m_vEntries.empty() )
	{
		/* This course difficulty doesn't exist. */
		m_TrailCacheNull[st][cd] = true;
		m_TrailCacheValid[st][cd] = true;
		return NULL;
	}

	m_TrailCacheValid[st][cd] = true;
	m_TrailCacheNull[st][cd] = false;
	return &m_TrailCache[st][cd];
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
	RandomGen rnd( GAMESTATE->m_iRoundSeed + GetHashForString(m_sMainTitle) );

	vector<CourseEntry> tmp_entries;
	if( m_bRandomize )
	{
		/* Always randomize the same way per round.  Otherwise, the displayed course
		 * will change every time it's viewed, and the displayed order will have no
		 * bearing on what you'll actually play. */
		tmp_entries = m_entries;
		random_shuffle( tmp_entries.begin(), tmp_entries.end(), rnd );
	}

	const vector<CourseEntry> &entries = m_bRandomize? tmp_entries:m_entries;

	/* This can take some time, so don't fill it out unless we need it. */
	bool bMostPlayedSet = false;
	vector<Song*> vSongsByMostPlayed;
	
	bool bShuffledSet = false;
	vector<Song*> AllSongsShuffled;

	int CurSong = 0; /* Current offset into AllSongsShuffled */
	
	trail.m_StepsType = st;
	trail.m_CourseDifficulty = cd;

	/* Set to true if CourseDifficulty is able to change something. */
	bool bCourseDifficultyIsSignificant = (cd == DIFFICULTY_MEDIUM);
	for( unsigned i=0; i<entries.size(); i++ )
	{
		const CourseEntry &e = entries[i];
		CourseDifficulty entry_difficulty = cd;
		if( e.no_difficult && entry_difficulty == DIFFICULTY_HARD )
			entry_difficulty = DIFFICULTY_MEDIUM;

		Song* pSong = NULL;	// fill this in
		Steps* pSteps = NULL;	// fill this in

		/* This applies difficult mode for meter ranges.  (If it's a difficulty
		 * class, we'll do it below.) */
		int low_meter = e.low_meter;
		int high_meter = e.high_meter;

		switch( e.type )
		{
		case COURSE_ENTRY_FIXED:
			pSong = e.pSong;
			if( pSong )
			{
				if( e.difficulty != DIFFICULTY_INVALID )
					pSteps = pSong->GetStepsByDifficulty( st, e.difficulty );
				else if( e.low_meter != -1  &&  e.high_meter != -1 )
					pSteps = pSong->GetStepsByMeter( st, low_meter, high_meter );
				else
					pSteps = pSong->GetStepsByDifficulty( st, DIFFICULTY_MEDIUM );
			}
			break;
		case COURSE_ENTRY_RANDOM:
		case COURSE_ENTRY_RANDOM_WITHIN_GROUP:
			{
				if( !bShuffledSet )
				{
					AllSongsShuffled = SONGMAN->GetAllSongs();
					random_shuffle( AllSongsShuffled.begin(), AllSongsShuffled.end(), rnd );
					bShuffledSet = true;
				}

				// find a song with the notes we want
				for( unsigned j=0; j<AllSongsShuffled.size(); j++ )
				{
					/* See if the first song matches what we want. */
					ASSERT( unsigned(CurSong) < AllSongsShuffled.size() );
					pSong = AllSongsShuffled[CurSong];
					ASSERT( pSong );
					CurSong = (CurSong+1) % AllSongsShuffled.size();

					if(e.type == COURSE_ENTRY_RANDOM_WITHIN_GROUP &&
					   pSong->m_sGroupName.CompareNoCase(e.group_name))
					   continue; /* wrong group */

					if( e.difficulty == DIFFICULTY_INVALID )
						pSteps = pSong->GetStepsByMeter( st, low_meter, high_meter );
					else
						pSteps = pSong->GetStepsByDifficulty( st, e.difficulty );

					if( pSteps )	// found a match
						break;		// stop searching

					pSong = NULL;
					pSteps = NULL;
				}
			}
			break;
		case COURSE_ENTRY_BEST:
		case COURSE_ENTRY_WORST:
			{
				if( !bMostPlayedSet )
				{
					bMostPlayedSet = true;
					vSongsByMostPlayed = GetFilteredBestSongs( st );
				}

				if( e.players_index >= (int)vSongsByMostPlayed.size() )
					break;

				switch( e.type )
				{
				case COURSE_ENTRY_BEST:
					pSong = vSongsByMostPlayed[e.players_index];
					break;
				case COURSE_ENTRY_WORST:
					pSong = vSongsByMostPlayed[vSongsByMostPlayed.size()-1-e.players_index];
					break;
				default:
					ASSERT(0);
				}

				if( e.difficulty == DIFFICULTY_INVALID )
					pSteps = pSong->GetStepsByMeter( st, low_meter, high_meter );
				else
					pSteps = pSong->GetStepsByDifficulty( st, e.difficulty );

				if( pSteps == NULL )
					pSteps = pSong->GetClosestNotes( st, DIFFICULTY_MEDIUM );
			}
			break;
		default:
			ASSERT(0);
		}

		if( !pSong || !pSteps )
			continue;	// this song entry isn't playable.  Skip.

		Difficulty dc = pSteps->GetDifficulty();
		if( entry_difficulty != DIFFICULTY_MEDIUM )
		{
			/* See if we can find a NoteData after adjusting the difficulty by COURSE_DIFFICULTY_CLASS_CHANGE.
			 * If we can't, just use the one we already have. */
			Difficulty new_dc = Difficulty( dc + COURSE_DIFFICULTY_CLASS_CHANGE[entry_difficulty] );
			new_dc = clamp( new_dc, DIFFICULTY_BEGINNER, DIFFICULTY_CHALLENGE );

			bool bChangedDifficulty = false;
			if( new_dc != dc )
			{
				Steps* pNewSteps = pSong->GetStepsByDifficulty( st, new_dc );
				if( pNewSteps )
				{
					dc = new_dc;
					pSteps = pNewSteps;
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
			if( e.difficulty == DIFFICULTY_INVALID && bChangedDifficulty )
			{
				/* Minimum and maximum to add to make the meter range contain the actual
				 * meter: */
				int iMinDist = pSteps->GetMeter() - high_meter;
				int iMaxDist = pSteps->GetMeter() - low_meter;

				/* Clamp the possible adjustments to try to avoid going under 1 or over
				 * MAX_BOTTOM_RANGE. */
				iMinDist = min( max( iMinDist, -low_meter+1 ), iMaxDist );
				iMaxDist = max( min( iMaxDist, MAX_BOTTOM_RANGE-high_meter ), iMinDist );

				int iAdd;
				if( iMaxDist == iMinDist )
					iAdd = iMaxDist;
				else
					iAdd = rnd(iMaxDist-iMinDist) + iMinDist;
				low_meter += iAdd;
				high_meter += iAdd;
			}
		}

		TrailEntry te;
		te.pSong = pSong;
		te.pSteps = pSteps;
		te.Modifiers = e.modifiers;
		te.Attacks = e.attacks;
		te.bMystery = e.mystery;
		te.iLowMeter = low_meter;
		te.iHighMeter = high_meter;
		/* If we chose based on meter (not difficulty), then store DIFFICULTY_INVALID, so
		 * other classes can tell that we used meter. */
		if( e.difficulty == DIFFICULTY_INVALID )
			te.dc = DIFFICULTY_INVALID;
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

bool Course::HasMods() const
{
	FOREACH_CONST( CourseEntry, m_entries, e )
	{
		if( !e->modifiers.empty() || !e->attacks.empty() )
			return true;
	}

	return false;
}

bool Course::AllSongsAreFixed() const
{
	FOREACH_CONST( CourseEntry, m_entries, e )
	{
		if( e->type != COURSE_ENTRY_FIXED )
			return false;
	}
	return true;
}

void Course::Invalidate( Song *pStaleSong )
{
	FOREACH_CONST( CourseEntry, m_entries, e )
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
			if( m_TrailCacheValid[st][cd] && !m_TrailCacheNull[st][cd] )
				if( GetTrail( st, cd )->ContainsSong( pStaleSong ) )
					m_TrailCacheValid[st][cd] = false;
}

void Course::RegenerateNonFixedTrails()
{
	// Only need to regen Trails if the Course has a random entry.
	// We can create these Trails on demand because we don't 
	// calculate RadarValues for Trails with one or more non-fixed 
	// entry.
	if( !IsFixed() )
		ZERO( m_TrailCacheValid );
}

RageColor Course::GetColor() const
{
	// FIXME: These colors shouldn't be hard-coded
	int iMeter = 5;
	
	switch (PREFSMAN->m_iCourseSortOrder)
	{
	case PrefsManager::COURSE_SORT_SONGS:	
		if( m_entries.size() >= 7 )
			return RageColor(1,0,0,1);	// red
		else if( m_entries.size() >= 4 )
			return RageColor(1,1,0,1);	// yellow
		else
			return RageColor(0,1,0,1);	// green
		// never should get here
		break;

	case PrefsManager::COURSE_SORT_METER:
		if ( !IsFixed() )
			return RageColor(0,0,1,1);  // blue
		if (iMeter > 8.5)
			return RageColor(1,0,0,1);  // red
		if (iMeter >= 7)
			return RageColor(1,0.5f,0,1); // orange
		if (iMeter >= 5)
			return RageColor(1,1,0,1);  // yellow
		return RageColor(0,1,0,1); // green

	case PrefsManager::COURSE_SORT_METER_SUM:
		if ( !IsFixed() )
			return RageColor(0,0,1,1);  // blue
		if (m_SortOrder_TotalDifficulty >= 40)
			return RageColor(1,0,0,1);  // red
		if (m_SortOrder_TotalDifficulty >= 30)
			return RageColor(1,0.5f,0,1); // orange
		if (m_SortOrder_TotalDifficulty >= 20)
			return RageColor(1,1,0,1);  // yellow
		return RageColor(0,1,0,1); // green

	case PrefsManager::COURSE_SORT_RANK:
		if (m_SortOrder_Ranking == 3)
			return RageColor(0,0,1,1);  // blue
		if (m_SortOrder_Ranking == 2)
			return RageColor(1,0.5f,0,1); // orange
		if (m_SortOrder_Ranking == 1)
			return RageColor(0,1,0,1); // green
		return RageColor(1,1,0,1); // yellow, never should get here
	default:
		ASSERT(0);
		return RageColor(1,1,1,1);  // white, never should reach here
	}
}

bool Course::IsFixed() const
{
	for(unsigned i = 0; i < m_entries.size(); i++)
	{
		if ( m_entries[i].type == COURSE_ENTRY_FIXED )
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
	for(unsigned i = 0; i < m_entries.size(); i++)
	{
		switch( m_entries[i].type )
		{
		case COURSE_ENTRY_BEST:
		case COURSE_ENTRY_WORST:
			return true;
		}
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

	unsigned i;

	// courses with random/players best-worst songs should go at the end
	for(i = 0; i < m_entries.size(); i++)
	{
		if ( m_entries[i].type == COURSE_ENTRY_FIXED )
			continue;

		if ( m_SortOrder_Ranking == 2 )
			m_SortOrder_Ranking = 3;
		m_SortOrder_TotalDifficulty = 999999;     // large number
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

void Course::GetAllCachedTrails( vector<Trail *> &out )
{
	FOREACH_StepsType( st )
		FOREACH_ShownCourseDifficulty( cd )
			if( m_TrailCacheValid[st][cd] && !m_TrailCacheNull[st][cd] )
				out.push_back( &m_TrailCache[st][cd] );
}

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
