#include "global.h"
#include "SongUtil.h"
#include "song.h"
#include "Steps.h"
#include "GameState.h"
#include "Style.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "XmlFile.h"
#include "Foreach.h"
#include "UnlockManager.h"
#include "ThemeMetric.h"
#include "LocalizedString.h"
#include "RageLog.h"

bool SongCriteria::Matches( const Song *pSong ) const
{
	if( !m_sGroupName.empty()  &&  m_sGroupName != pSong->m_sGroupName )
		return false;

	if( m_bUseSongGenreAllowedList )
	{
		if( find(m_vsSongGenreAllowedList.begin(),m_vsSongGenreAllowedList.end(),pSong->m_sGenre) == m_vsSongGenreAllowedList.end() )
			return false;
	}

	switch( m_Selectable )
	{
	DEFAULT_FAIL(m_Selectable);
	case Selectable_Yes:
		if( pSong->m_SelectionDisplay != Song::SHOW_ALWAYS )
			return false;
		break;
	case Selectable_No:
		if( pSong->m_SelectionDisplay != Song::SHOW_NEVER )
			return false;
		break;
	case Selectable_DontCare:
		break;
	}

	if( m_bUseSongAllowedList )
	{
		if( find(m_vpSongAllowedList.begin(),m_vpSongAllowedList.end(),pSong) == m_vpSongAllowedList.end() )
			return false;
	}

	if( m_iStagesForSong != -1  &&  SONGMAN->GetNumStagesForSong(pSong) != m_iStagesForSong )
		return false;

	switch( m_Tutorial )
	{
	DEFAULT_FAIL(m_Tutorial);
	case Tutorial_Yes:
		if( !pSong->IsTutorial() )
			return false;
		break;
	case Tutorial_No:
		if( pSong->IsTutorial() )
			return false;
		break;
	case Tutorial_DontCare:
		break;
	}

	switch( m_Locked )
	{
	DEFAULT_FAIL(m_Locked);
	case Locked_Locked:
		if( UNLOCKMAN  &&  !UNLOCKMAN->SongIsLocked(pSong) )
			return false;
		break;
	case Locked_Unlocked:
		if( UNLOCKMAN  &&  UNLOCKMAN->SongIsLocked(pSong) )
			return false;
		break;
	case Locked_DontCare:
		break;
	}

	return true;
}

void SongUtil::GetSteps( 
	const Song *pSong, 
	vector<Steps*>& arrayAddTo, 
	StepsType st, 
	Difficulty dc, 
	int iMeterLow, 
	int iMeterHigh, 
	const RString &sDescription, 
	bool bIncludeAutoGen, 
	unsigned uHash,
	int iMaxToGet 
	)
{
	if( !iMaxToGet )
		return;

	const vector<Steps*> &vpSteps = st == StepsType_Invalid ? pSong->GetAllSteps() : pSong->GetStepsByStepsType(st);
	for( unsigned i=0; i<vpSteps.size(); i++ )	// for each of the Song's Steps
	{
		Steps* pSteps = vpSteps[i];

		if( dc != DIFFICULTY_Invalid && dc != pSteps->GetDifficulty() )
			continue;
		if( iMeterLow != -1 && iMeterLow > pSteps->GetMeter() )
			continue;
		if( iMeterHigh != -1 && iMeterHigh < pSteps->GetMeter() )
			continue;
		if( sDescription.size() && sDescription != pSteps->GetDescription() )
			continue;
		if( uHash != 0 && uHash != pSteps->GetHash() )
			continue;
		if( !bIncludeAutoGen && pSteps->IsAutogen() )
			continue;

		arrayAddTo.push_back( pSteps );

		if( iMaxToGet != -1 )
		{
			--iMaxToGet;
			if( !iMaxToGet )
				break;
		}
	}
}

Steps* SongUtil::GetOneSteps( 
	const Song *pSong, 
	StepsType st, 
	Difficulty dc, 
	int iMeterLow, 
	int iMeterHigh, 
	const RString &sDescription, 
	unsigned uHash,
	bool bIncludeAutoGen
	)
{
	vector<Steps*> vpSteps;
	GetSteps( pSong, vpSteps, st, dc, iMeterLow, iMeterHigh, sDescription, bIncludeAutoGen, uHash, 1 );	// get max 1
	if( vpSteps.empty() )
		return NULL;
	else
		return vpSteps[0];
}

Steps* SongUtil::GetStepsByDifficulty( const Song *pSong, StepsType st, Difficulty dc, bool bIncludeAutoGen )
{
	const vector<Steps*>& vpSteps = (st == StepsType_Invalid)? pSong->GetAllSteps() : pSong->GetStepsByStepsType(st);
	for( unsigned i=0; i<vpSteps.size(); i++ )	// for each of the Song's Steps
	{
		Steps* pSteps = vpSteps[i];

		if( dc != DIFFICULTY_Invalid && dc != pSteps->GetDifficulty() )
			continue;
		if( !bIncludeAutoGen && pSteps->IsAutogen() )
			continue;

		return pSteps;
	}

	return NULL;
}

Steps* SongUtil::GetStepsByMeter( const Song *pSong, StepsType st, int iMeterLow, int iMeterHigh )
{
	const vector<Steps*>& vpSteps = (st == StepsType_Invalid)? pSong->GetAllSteps() : pSong->GetStepsByStepsType(st);
	for( unsigned i=0; i<vpSteps.size(); i++ )	// for each of the Song's Steps
	{
		Steps* pSteps = vpSteps[i];

		if( iMeterLow > pSteps->GetMeter() )
			continue;
		if( iMeterHigh < pSteps->GetMeter() )
			continue;

		return pSteps;
	}

	return NULL;
}

Steps* SongUtil::GetStepsByDescription( const Song *pSong, StepsType st, RString sDescription )
{
	vector<Steps*> vNotes;
	GetSteps( pSong, vNotes, st, DIFFICULTY_Invalid, -1, -1, sDescription );
	if( vNotes.size() == 0 )
		return NULL;
	else 
		return vNotes[0];
}


Steps* SongUtil::GetClosestNotes( const Song *pSong, StepsType st, Difficulty dc, bool bIgnoreLocked )
{
	ASSERT( dc != DIFFICULTY_Invalid );

	const vector<Steps*>& vpSteps = (st == StepsType_Invalid)? pSong->GetAllSteps() : pSong->GetStepsByStepsType(st);
	Steps *pClosest = NULL;
	int iClosestDistance = 999;
	for( unsigned i=0; i<vpSteps.size(); i++ )	// for each of the Song's Steps
	{
		Steps* pSteps = vpSteps[i];

		if( pSteps->GetDifficulty() == DIFFICULTY_EDIT && dc != DIFFICULTY_EDIT )
			continue;
		if( bIgnoreLocked && UNLOCKMAN->StepsIsLocked(pSong,pSteps) )
			continue;

		int iDistance = abs(dc - pSteps->GetDifficulty());
		if( iDistance < iClosestDistance )
		{
			pClosest = pSteps;
			iClosestDistance = iDistance;
		}
	}

	return pClosest;
}


/* Make any duplicate difficulties edits.  (Note that BMS files do a first pass
 * on this; see BMSLoader::SlideDuplicateDifficulties.) */
void SongUtil::AdjustDuplicateSteps( Song *pSong )
{
	FOREACH_StepsType( st )
	{
		FOREACH_Difficulty( dc )
		{
			if( dc == DIFFICULTY_EDIT )
				continue;

			vector<Steps*> vSteps;
			SongUtil::GetSteps( pSong, vSteps, st, dc );

			/* Delete steps that are completely identical.  This happened due to a
			 * bug in an earlier version. */
			DeleteDuplicateSteps( pSong, vSteps );

			CHECKPOINT;
			StepsUtil::SortNotesArrayByDifficulty( vSteps );
			CHECKPOINT;
			for( unsigned k=1; k<vSteps.size(); k++ )
			{
				vSteps[k]->SetDifficulty( DIFFICULTY_EDIT );
				if( vSteps[k]->GetDescription() == "" )
				{
					/* "Hard Edit" */
					RString EditName = Capitalize( DifficultyToString(dc) ) + " Edit";
					vSteps[k]->SetDescription( EditName );
				}
			}
		}

		/* XXX: Don't allow edits to have descriptions that look like regular difficulties.
		 * These are confusing, and they're ambiguous when passed to GetStepsByID. */
	}
}

static RString RemoveInitialWhitespace( RString s )
{
	size_t i = s.find_first_not_of(" \t\r\n");
	if( i != s.npos )
		s.erase( 0, i );
	return s;
}

/* This is called within TidyUpData, before autogen notes are added. */
void SongUtil::DeleteDuplicateSteps( Song *pSong, vector<Steps*> &vSteps )
{
	/* vSteps have the same StepsType and Difficulty.  Delete them if they have the
	 * same m_sDescription, m_iMeter and SMNoteData. */
	CHECKPOINT;
	for( unsigned i=0; i<vSteps.size(); i++ )
	{
		CHECKPOINT;
		const Steps *s1 = vSteps[i];

		for( unsigned j=i+1; j<vSteps.size(); j++ )
		{
			CHECKPOINT;
			const Steps *s2 = vSteps[j];

			if( s1->GetDescription() != s2->GetDescription() )
				continue;
			if( s1->GetMeter() != s2->GetMeter() )
				continue;
			/* Compare, ignoring whitespace. */
			RString sSMNoteData1;
			s1->GetSMNoteData( sSMNoteData1 );
			RString sSMNoteData2;
			s2->GetSMNoteData( sSMNoteData2 );
			if( RemoveInitialWhitespace(sSMNoteData1) != RemoveInitialWhitespace(sSMNoteData2) )
				continue;

			LOG->Trace("Removed %p duplicate steps in song \"%s\" with description \"%s\" and meter \"%i\"",
				s2, pSong->GetSongDir().c_str(), s1->GetDescription().c_str(), s1->GetMeter() );
				
			pSong->DeleteSteps( s2, false );

			vSteps.erase(vSteps.begin()+j);
			--j;
		}
	}
}


/////////////////////////////////////
// Sorting
/////////////////////////////////////

static LocalizedString SORT_NOT_AVAILABLE( "Sort", "NotAvailable" );
static LocalizedString SORT_OTHER        ( "Sort", "Other" );

/* Just calculating GetNumTimesPlayed within the sort is pretty slow, so let's precompute
 * it.  (This could be generalized with a template.) */
static map<const Song*, RString> g_mapSongSortVal;

static bool CompareSongPointersBySortValueAscending( const Song *pSong1, const Song *pSong2 )
{
	return g_mapSongSortVal[pSong1] < g_mapSongSortVal[pSong2];
}

static bool CompareSongPointersBySortValueDescending( const Song *pSong1, const Song *pSong2 )
{
	return g_mapSongSortVal[pSong1] > g_mapSongSortVal[pSong2];
}


RString SongUtil::MakeSortString( RString s )
{
	s.MakeUpper();

	// Make sure that non-alphanumeric strings are placed at the very end.
	if( s.size() > 0 )
	{
		if( s[0] == '.' )	// ".59"
			s.erase(s.begin());
		if( (s[0] < 'A' || s[0] > 'Z') && (s[0] < '0' || s[0] > '9') )
			s = char(126) + s;
	}

	return s;
}

static bool CompareSongPointersByTitle( const Song *pSong1, const Song *pSong2 )
{
	// Prefer transliterations to full titles
	RString s1 = pSong1->GetTranslitMainTitle();
	RString s2 = pSong2->GetTranslitMainTitle();
	if( s1 == s2 )
	{
		s1 = pSong1->GetTranslitSubTitle();
		s2 = pSong2->GetTranslitSubTitle();
	}

	s1 = SongUtil::MakeSortString(s1);
	s2 = SongUtil::MakeSortString(s2);

	int ret = strcmp( s1, s2 );
	if(ret < 0) return true;
	if(ret > 0) return false;

	/* The titles are the same.  Ensure we get a consistent ordering
	 * by comparing the unique SongFilePaths. */
	return pSong1->GetSongFilePath().CompareNoCase(pSong2->GetSongFilePath()) < 0;
}

void SongUtil::SortSongPointerArrayByTitle( vector<Song*> &vpSongsInOut )
{
	sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersByTitle );
}

static bool CompareSongPointersByBPM( const Song *pSong1, const Song *pSong2 )
{
	DisplayBpms bpms1, bpms2;
	pSong1->GetDisplayBpms( bpms1 );
	pSong2->GetDisplayBpms( bpms2 );

	if( bpms1.GetMax() < bpms2.GetMax() )
		return true;
	if( bpms1.GetMax() > bpms2.GetMax() )
		return false;
	
	return CompareRStringsAsc( pSong1->GetSongFilePath(), pSong2->GetSongFilePath() );
}

void SongUtil::SortSongPointerArrayByBPM( vector<Song*> &vpSongsInOut )
{
	sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersByBPM );
}

void AppendOctal( int n, int digits, RString &out )
{
	for( int p = digits-1; p >= 0; --p )
	{
		const int shift = p*3;
		int n2 = (n >> shift) & 0x7;
		out.insert( out.end(), (char) (n2+'0') );
	}
}

static bool CompDescending( const pair<Song *, RString> &a, const pair<Song *, RString> &b )
{
	return a.second > b.second;
}
static bool CompAscending( const pair<Song *, RString> &a, const pair<Song *, RString> &b )
{
	return a.second < b.second;
}

void SongUtil::SortSongPointerArrayByGrades( vector<Song*> &vpSongsInOut, bool bDescending )
{
	/* Optimize by pre-writing a string to compare, since doing GetNumNotesWithGrade
	 * inside the sort is too slow. */
	typedef pair< Song *, RString > val;
	vector<val> vals;
	vals.reserve( vpSongsInOut.size() );

	for( unsigned i = 0; i < vpSongsInOut.size(); ++i )
	{
		Song *pSong = vpSongsInOut[i];

		int iCounts[NUM_Grade];
		const Profile *pProfile = PROFILEMAN->GetMachineProfile();
		ASSERT( pProfile ); // XXX: Debugging.
		const Style *pStyle = GAMESTATE->GetCurrentStyle();
		ASSERT( pStyle );   // XXX: Debugging.
		StepsType st = pStyle->m_StepsType;
		pProfile->GetGrades( pSong, st, iCounts );

		RString foo;
		foo.reserve(256);
		for( int g=Grade_Tier01; g<=Grade_NoData; ++g )
			AppendOctal( iCounts[g], 3, foo );
		vals.push_back( val(pSong, foo) );
	}

	sort( vals.begin(), vals.end(), bDescending ? CompDescending : CompAscending );

	for( unsigned i = 0; i < vpSongsInOut.size(); ++i )
		vpSongsInOut[i] = vals[i].first;
}


void SongUtil::SortSongPointerArrayByArtist( vector<Song*> &vpSongsInOut )
{
	for( unsigned i = 0; i < vpSongsInOut.size(); ++i )
		g_mapSongSortVal[vpSongsInOut[i]] = MakeSortString( vpSongsInOut[i]->GetTranslitArtist() );
	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersBySortValueAscending );
}

/* This is for internal use, not display; sorting by Unicode codepoints isn't very
 * interesting for display. */
void SongUtil::SortSongPointerArrayByDisplayArtist( vector<Song*> &vpSongsInOut )
{
	for( unsigned i = 0; i < vpSongsInOut.size(); ++i )
		g_mapSongSortVal[vpSongsInOut[i]] = MakeSortString( vpSongsInOut[i]->GetDisplayArtist() );
	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersBySortValueAscending );
}

static int CompareSongPointersByGenre(const Song *pSong1, const Song *pSong2)
{
	return pSong1->m_sGenre < pSong2->m_sGenre;
}

void SongUtil::SortSongPointerArrayByGenre( vector<Song*> &vpSongsInOut )
{
	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersByGenre );
}

int SongUtil::CompareSongPointersByGroup(const Song *pSong1, const Song *pSong2)
{
	return pSong1->m_sGroupName < pSong2->m_sGroupName;
}

static int CompareSongPointersByGroupAndTitle( const Song *pSong1, const Song *pSong2 )
{
	const RString &sGroup1 = pSong1->m_sGroupName;
	const RString &sGroup2 = pSong2->m_sGroupName;

	if( sGroup1 < sGroup2 )
		return true;
	if( sGroup1 > sGroup2 )
		return false;

	/* Same group; compare by name. */
	return CompareSongPointersByTitle( pSong1, pSong2 );
}

void SongUtil::SortSongPointerArrayByGroupAndTitle( vector<Song*> &vpSongsInOut )
{
	sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersByGroupAndTitle );
}

void SongUtil::SortSongPointerArrayByNumPlays( vector<Song*> &vpSongsInOut, ProfileSlot slot, bool bDescending )
{
	if( !PROFILEMAN->IsPersistentProfile(slot) )
		return;	// nothing to do since we don't have data
	Profile* pProfile = PROFILEMAN->GetProfile(slot);
	SortSongPointerArrayByNumPlays( vpSongsInOut, pProfile, bDescending );
}

void SongUtil::SortSongPointerArrayByNumPlays( vector<Song*> &vpSongsInOut, const Profile* pProfile, bool bDescending )
{
	ASSERT( pProfile );
	for(unsigned i = 0; i < vpSongsInOut.size(); ++i)
		g_mapSongSortVal[vpSongsInOut[i]] = ssprintf("%9i", pProfile->GetSongNumTimesPlayed(vpSongsInOut[i]));
	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), bDescending ? CompareSongPointersBySortValueDescending : CompareSongPointersBySortValueAscending );
	g_mapSongSortVal.clear();
}

RString SongUtil::GetSectionNameFromSongAndSort( const Song* pSong, SortOrder so )
{
	if( pSong == NULL )
		return RString();

	switch( so )
	{
	case SORT_PREFERRED:
		return RString();
	case SORT_GROUP:
		// guaranteed not empty	
		return pSong->m_sGroupName;
	case SORT_TITLE:
	case SORT_ARTIST:	
		{
			RString s;
			switch( so )
			{
			case SORT_TITLE:	s = pSong->GetTranslitMainTitle();	break;
			case SORT_ARTIST:	s = pSong->GetTranslitArtist();		break;
			default:	ASSERT(0);
			}
			s = MakeSortString(s);	// resulting string will be uppercase
			
			if( s.empty() )
				return RString();
			else if( s[0] >= '0' && s[0] <= '9' )
				return "0-9";
			else if( s[0] < 'A' || s[0] > 'Z')
				return SORT_OTHER.GetValue();
			else
				return s.Left(1);
		}
	case SORT_GENRE:
		if( !pSong->m_sGenre.empty() )
			return pSong->m_sGenre;
		return SORT_NOT_AVAILABLE.GetValue();
	case SORT_BPM:
		{
			const int iBPMGroupSize = 20;
			DisplayBpms bpms;
			pSong->GetDisplayBpms( bpms );
			int iMaxBPM = (int)bpms.GetMax();
			iMaxBPM += iBPMGroupSize - (iMaxBPM%iBPMGroupSize) - 1;
			return ssprintf("%03d-%03d",iMaxBPM-(iBPMGroupSize-1), iMaxBPM);
		}
	case SORT_POPULARITY:
		return RString();
	case SORT_TOP_GRADES:
		{
			int iCounts[NUM_Grade];
			PROFILEMAN->GetMachineProfile()->GetGrades( pSong, GAMESTATE->GetCurrentStyle()->m_StepsType, iCounts );

			for( int i=Grade_Tier01; i<NUM_Grade; ++i )
			{
				Grade g = (Grade)i;
				if( iCounts[i] > 0 )
					return ssprintf( "%4s x %d", GradeToLocalizedString(g).c_str(), iCounts[i] );
			}
			return GradeToLocalizedString( Grade_NoData );
		}
	case SORT_EASY_METER:
		{
			Steps* pSteps = GetStepsByDifficulty(pSong, GAMESTATE->GetCurrentStyle()->m_StepsType,DIFFICULTY_EASY);
			if( pSteps && !UNLOCKMAN->StepsIsLocked(pSong,pSteps) )	
				return ssprintf("%02d", pSteps->GetMeter() );
			return SORT_NOT_AVAILABLE.GetValue();
		}
	case SORT_MEDIUM_METER:
		{
			Steps* pSteps = GetStepsByDifficulty(pSong, GAMESTATE->GetCurrentStyle()->m_StepsType,DIFFICULTY_MEDIUM);
			if( pSteps && !UNLOCKMAN->StepsIsLocked(pSong,pSteps) )	
				return ssprintf("%02d", pSteps->GetMeter() );
			return SORT_NOT_AVAILABLE.GetValue();
		}
	case SORT_HARD_METER:
		{
			Steps* pSteps = GetStepsByDifficulty(pSong, GAMESTATE->GetCurrentStyle()->m_StepsType,DIFFICULTY_HARD);
			if( pSteps && !UNLOCKMAN->StepsIsLocked(pSong,pSteps) )	
				return ssprintf("%02d", pSteps->GetMeter() );
			return SORT_NOT_AVAILABLE.GetValue();
		}
	case SORT_CHALLENGE_METER:
		{
			Steps* pSteps = GetStepsByDifficulty(pSong, GAMESTATE->GetCurrentStyle()->m_StepsType,DIFFICULTY_CHALLENGE);
			if( pSteps && !UNLOCKMAN->StepsIsLocked(pSong,pSteps) )	
				return ssprintf("%02d", pSteps->GetMeter() );
			return SORT_NOT_AVAILABLE.GetValue();
		}
	case SORT_MODE_MENU:
		return RString();
	case SORT_ALL_COURSES:
	case SORT_NONSTOP_COURSES:
	case SORT_ONI_COURSES:
	case SORT_ENDLESS_COURSES:
	default:
		ASSERT(0);
		return RString();
	}
}

void SongUtil::SortSongPointerArrayBySectionName( vector<Song*> &vpSongsInOut, SortOrder so )
{
	RString sOther = SORT_OTHER.GetValue();
	for(unsigned i = 0; i < vpSongsInOut.size(); ++i)
	{
		RString val = GetSectionNameFromSongAndSort( vpSongsInOut[i], so );

		/* Make sure 0-9 comes first and OTHER comes last. */
		if( val == "0-9" )			val = "0";
		else if( val == sOther )    val = "2";
		else						val = "1" + MakeSortString(val);

		g_mapSongSortVal[vpSongsInOut[i]] = val;
	}

	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersBySortValueAscending );
	g_mapSongSortVal.clear();
}

void SongUtil::SortSongPointerArrayByMeter( vector<Song*> &vpSongsInOut, Difficulty dc )
{
	g_mapSongSortVal.clear();
	for(unsigned i = 0; i < vpSongsInOut.size(); ++i)
	{
		/* Ignore locked steps. */
		const Steps* pSteps = GetClosestNotes( vpSongsInOut[i], GAMESTATE->GetCurrentStyle()->m_StepsType, dc, true );
		RString &s = g_mapSongSortVal[vpSongsInOut[i]];
		s = ssprintf("%03d", pSteps ? pSteps->GetMeter() : 0);

		/* Hack: always put tutorial songs first. */
		s += ssprintf( "%c", vpSongsInOut[i]->IsTutorial()? '0':'1' );

		/* 
		 * pSteps may not be exactly the difficulty we want; for example, we may
		 * be sorting by Hard difficulty and a song may have no Hard steps.
		 *
		 * In this case, we can end up with unintuitive ties; for example, pSteps
		 * may be Medium with a meter of 5, which will sort it among the 5-meter
		 * Hard songs.  Break the tie, by adding the difficulty to the sort as
		 * well.  That way, we'll always put Medium 5s before Hard 5s.  If all
		 * songs are using the preferred difficulty (dc), this will be a no-op.
		 */
		s += ssprintf( "%c", (pSteps? pSteps->GetDifficulty():0) + '0' );

		if( PREFSMAN->m_bSubSortByNumSteps )
			s += ssprintf("%06.0f",pSteps ? pSteps->GetRadarValues(PLAYER_1)[RadarCategory_TapsAndHolds] : 0);
	}
	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersBySortValueAscending );
}

void SongUtil::SortByMostRecentlyPlayedForMachine( vector<Song*> &vpSongsInOut )
{
	Profile *pProfile = PROFILEMAN->GetMachineProfile();

	FOREACH_CONST( Song*, vpSongsInOut, s )
	{
		int iNumTimesPlayed = pProfile->GetSongNumTimesPlayed( *s );
		RString val = iNumTimesPlayed ? pProfile->GetSongLastPlayedDateTime(*s).GetString() : "0";
		g_mapSongSortVal[*s] = val;
	}

	stable_sort( vpSongsInOut.begin(), vpSongsInOut.end(), CompareSongPointersBySortValueDescending );
	g_mapSongSortVal.clear();
}

bool SongUtil::IsEditDescriptionUnique( const Song* pSong, StepsType st, const RString &sPreferredDescription, const Steps *pExclude )
{
	FOREACH_CONST( Steps*, pSong->GetAllSteps(), s )
	{
		Steps *pSteps = *s;

		if( pSteps->GetDifficulty() != DIFFICULTY_EDIT )
			continue;
		if( pSteps->m_StepsType != st )
			continue;
		if( pSteps == pExclude )
			continue;
		if( pSteps->GetDescription() == sPreferredDescription )
			return false;
	}
	return true;
}

RString SongUtil::MakeUniqueEditDescription( const Song *pSong, StepsType st, const RString &sPreferredDescription )
{
	if( IsEditDescriptionUnique( pSong, st, sPreferredDescription, NULL ) )
		return sPreferredDescription;

	RString sTemp;

	for( int i=0; i<1000; i++ )
	{
		// make name "My Edit" -> "My Edit2"
		RString sNum = ssprintf("%d", i+1);
		sTemp = sPreferredDescription.Left( MAX_EDIT_STEPS_DESCRIPTION_LENGTH - sNum.size() ) + sNum;

		if( IsEditDescriptionUnique(pSong, st, sTemp, NULL) )
			return sTemp;
	}
	
	// Edit limit guards should keep us from ever having more than 1000 edits per song.
	ASSERT(0);
	return RString();
}

static LocalizedString YOU_MUST_SUPPLY_NAME	( "SongUtil", "You must supply a name for your new edit." );
static LocalizedString EDIT_NAME_CONFLICTS	( "SongUtil", "The name you chose conflicts with another edit. Please use a different name." );
static LocalizedString EDIT_NAME_CANNOT_CONTAIN	( "SongUtil", "The edit name cannot contain any of the following characters: %s" );
bool SongUtil::ValidateCurrentEditStepsDescription( const RString &sAnswer, RString &sErrorOut )
{
	Steps *pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];
	Song *pSong = SONGMAN->GetSongFromSteps( pSteps );

	ASSERT( pSteps->IsAnEdit() );

	if( sAnswer.empty() )
	{
		sErrorOut = YOU_MUST_SUPPLY_NAME;
		return false;
	}

	static const RString sInvalidChars = "\\/:*?\"<>|";
	if( strpbrk(sAnswer, sInvalidChars) != NULL )
	{
		sErrorOut = ssprintf( EDIT_NAME_CANNOT_CONTAIN.GetValue(), sInvalidChars.c_str() );
		return false;
	}

	// Steps name must be unique for this song.
	vector<Steps*> v;
	GetSteps( pSong, v, StepsType_Invalid, DIFFICULTY_EDIT ); 
	FOREACH_CONST( Steps*, v, s )
	{
		if( pSteps == *s )
			continue;	// don't comepare name against ourself

		if( (*s)->GetDescription() == sAnswer )
		{
			sErrorOut = EDIT_NAME_CONFLICTS;
			return false;
		}
	}

	return true;
}

bool SongUtil::ValidateCurrentStepsDescription( const RString &sAnswer, RString &sErrorOut )
{
	if( sAnswer.empty() )
		return true;

	/* Don't allow duplicate edit names within the same StepsType; edit names uniquely
	 * identify the edit. */
	Steps *pSteps = GAMESTATE->m_pCurSteps[PLAYER_1];

	/* If unchanged: */
	if( pSteps->GetDescription() == sAnswer )
		return true;

	
	if( pSteps->IsAnEdit() )
	{
		return SongUtil::ValidateCurrentEditStepsDescription( sAnswer, sErrorOut );
	}

	return true;
}

void SongUtil::GetAllSongGenres( vector<RString> &vsOut )
{
	set<RString> genres;
	FOREACH_CONST( Song*, SONGMAN->GetAllSongs(), song )
	{
		if( !(*song)->m_sGenre.empty() )
			genres.insert( (*song)->m_sGenre );
	}

	FOREACHS_CONST( RString, genres, s )
	{
		vsOut.push_back( *s );
	}
}

void SongUtil::FilterSongs( const SongCriteria &sc, const vector<Song*> &in, vector<Song*> &out )
{
	FOREACH_CONST( Song*, in, s )
	{
		if( sc.Matches( *s ) )
		{
			out.push_back( *s );
		}
	}
}


//////////////////////////////////
// SongID
//////////////////////////////////

void SongID::FromSong( const Song *p )
{
	if( p )
		sDir = p->GetSongDir();
	else
		sDir = "";
	
	// HACK for backwards compatibility:
	// Strip off leading "/".  2005/05/21 file layer changes added a leading slash.
	if( sDir.Left(1) == "/" )
		sDir.erase( sDir.begin() );
}

Song *SongID::ToSong() const
{
	// HACK for backwards compatibility:
	// Re-add the leading "/".  2005/05/21 file layer changes added a leading slash.
	RString sDir2 = sDir;
	if( sDir2.Left(1) != "/" )
		sDir2 = "/" + sDir2;

	return SONGMAN->GetSongFromDir( sDir2 );
}

XNode* SongID::CreateNode() const
{
	XNode* pNode = new XNode( "Song" );

	pNode->AppendAttr( "Dir", sDir );

	return pNode;
}

void SongID::LoadFromNode( const XNode* pNode ) 
{
	ASSERT( pNode->GetName() == "Song" );
	pNode->GetAttrValue("Dir", sDir);
}

RString SongID::ToString() const
{
	return sDir;
}

bool SongID::IsValid() const
{
	return !sDir.empty();
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
