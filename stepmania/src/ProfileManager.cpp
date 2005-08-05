#include "global.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageFile.h"
#include "RageFileManager.h"
#include "IniFile.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "GameState.h"
#include "song.h"
#include "Steps.h"
#include "Course.h"
#include "GameManager.h"
#include "ProductInfo.h"
#include "RageUtil.h"
#include "ThemeManager.h"
#include "MemoryCardManager.h"
#include "XmlFile.h"
#include "StepsUtil.h"
#include "Style.h"
#include "HighScore.h"


ProfileManager*	PROFILEMAN = NULL;	// global and accessable from anywhere in our program

#define NEW_MEM_CARD_NAME		""
#define USER_PROFILES_DIR		"Data/LocalProfiles/"
#define MACHINE_PROFILE_DIR		"Data/MachineProfile/"
const CString LAST_GOOD_DIR	=	"LastGood/";


// Directories to search for a profile if m_sMemoryCardProfileSubdir doesn't
// exist, separated by ";":
static Preference<CString> g_sMemoryCardProfileImportSubdirs( "MemoryCardProfileImportSubdirs", "" );

static CString LocalProfileIdToDir( const CString &sProfileID ) { return USER_PROFILES_DIR + sProfileID + "/"; }
static CString LocalProfileDirToId( const CString &sDir ) { return Basename( sDir ); }

static map<CString,Profile*> g_mapLocalProfileDirToProfile;


ProfileManager::ProfileManager()
{
	m_pMachineProfile = new Profile;
	FOREACH_PlayerNumber(pn)
		m_pMemoryCardProfile[pn] = new Profile;
}

ProfileManager::~ProfileManager()
{
	SAFE_DELETE( m_pMachineProfile );
	FOREACH_PlayerNumber(pn)
		SAFE_DELETE( m_pMemoryCardProfile[pn] );
}

void ProfileManager::Init()
{
	FOREACH_PlayerNumber( p )
	{
		m_bWasLoadedFromMemoryCard[p] = false;
		m_bLastLoadWasTamperedOrCorrupt[p] = false;
		m_bLastLoadWasFromLastGood[p] = false;
	}

	LoadMachineProfile();

	RefreshLocalProfilesFromDisk();
}

int ProfileManager::LoadProfile( PlayerNumber pn, CString sProfileDir, bool bIsMemCard )
{
	LOG->Trace( "LoadingProfile P%d, %s, %d", pn+1, sProfileDir.c_str(), bIsMemCard );

	ASSERT( !sProfileDir.empty() );
	ASSERT( sProfileDir.Right(1) == "/" );


	m_sProfileDir[pn] = sProfileDir;
	m_bWasLoadedFromMemoryCard[pn] = bIsMemCard;
	m_bLastLoadWasFromLastGood[pn] = false;

	// Try to load the original, non-backup data.
	Profile::LoadResult lr = GetProfile(pn)->LoadAllFromDir( m_sProfileDir[pn], PREFSMAN->m_bSignProfileData );
	
	CString sBackupDir = m_sProfileDir[pn] + LAST_GOOD_DIR;

	// Save a backup of the non-backup profile now that we've loaded it and know 
	// it's good. This should be reasonably fast because we're only saving Stats.xml 
	// and signatures - not all of the files in the Profile.
	if( lr == Profile::success )
	{
		Profile::BackupToDir( m_sProfileDir[pn], sBackupDir );
	}

	m_bLastLoadWasTamperedOrCorrupt[pn] = lr == Profile::failed_tampered;

	//
	// Try to load from the backup if the original data fails to load
	//
	if( lr == Profile::failed_tampered )
	{
		lr = GetProfile(pn)->LoadAllFromDir( sBackupDir, PREFSMAN->m_bSignProfileData );
		m_bLastLoadWasFromLastGood[pn] = lr == Profile::success;

		/* If the LastGood profile doesn't exist at all, and the actual profile was failed_tampered,
		 * then the error should be failed_tampered and not failed_no_profile. */
		if( lr == Profile::failed_no_profile )
		{
			LOG->Trace( "Profile was corrupt and LastGood for %s doesn't exist; error is Profile::failed_tampered",
					sProfileDir.c_str() );
			lr = Profile::failed_tampered;
		}
	}

	LOG->Trace( "Done loading profile - result %d", lr );

	return lr;
}

bool ProfileManager::LoadLocalProfileFromMachine( PlayerNumber pn )
{
	CString sProfileID = PREFSMAN->GetDefaultLocalProfileID(pn);
	if( sProfileID.empty() )
	{
		m_sProfileDir[pn] = "";
		return false;
	}

	m_sProfileDir[pn] = LocalProfileIdToDir( sProfileID );
	m_bWasLoadedFromMemoryCard[pn] = false;
	m_bLastLoadWasFromLastGood[pn] = false;

	map<CString,Profile*>::iterator iter = g_mapLocalProfileDirToProfile.find( m_sProfileDir[pn] );
	if( iter == g_mapLocalProfileDirToProfile.end() )
	{
		m_sProfileDir[pn] = "";
		return false;
	}

	return true;
}

void ProfileManager::GetMemoryCardProfileDirectoriesToTry( vector<CString> &asDirsToTry ) const
{
	/* Try to load the preferred profile. */
	asDirsToTry.push_back( PREFSMAN->m_sMemoryCardProfileSubdir );

	/* If that failed, try loading from all fallback directories. */
	split( g_sMemoryCardProfileImportSubdirs, ";", asDirsToTry, true );
}

bool ProfileManager::LoadProfileFromMemoryCard( PlayerNumber pn )
{
	UnloadProfile( pn );

	// mount slot
	if( MEMCARDMAN->GetCardState(pn) != MEMORY_CARD_STATE_READY )
		return false;

	vector<CString> asDirsToTry;
	GetMemoryCardProfileDirectoriesToTry( asDirsToTry );

	int iLoadedFrom = -1;
	for( unsigned i = 0; i < asDirsToTry.size(); ++i )
	{
		const CString &sSubdir = asDirsToTry[i];
		CString sDir = MEM_CARD_MOUNT_POINT[pn] + sSubdir + "/";

		/* If the load fails with Profile::failed_no_profile, keep searching.  However,
		 * if it fails with failed_tampered, data existed but couldn't be loaded;
		 * we don't want to mess with it, since it's confusing and may wipe out
		 * recoverable backup data.  The only time we really want to import data
		 * is on the very first use, when the new profile doesn't exist at all,
		 * but we also want to import scores in the case where the player created
		 * a directory for edits before playing, so keep searching if the directory
		 * exists with exists with no scores. */
		Profile::LoadResult res = (Profile::LoadResult) LoadProfile( pn, sDir, true );
		if( res == Profile::success )
		{
			iLoadedFrom = i;
			break;
		}
		
		if( res == Profile::failed_tampered )
			break;
	}

	/* Store the directory we imported from, for display purposes. */
	if( iLoadedFrom > 0 )
	{
		m_sProfileDirImportedFrom[pn] = asDirsToTry[iLoadedFrom];
	}

	/* If we imported a profile fallback directory, change the memory card
	 * directory back to the preferred directory: never write over imported
	 * scores. */
	m_sProfileDir[pn] = MEM_CARD_MOUNT_POINT[pn] + (CString)PREFSMAN->m_sMemoryCardProfileSubdir + "/";

	/* Load edits from all fallback directories, newest first. */
	for( unsigned i = 0; i < asDirsToTry.size(); ++i )
	{
		const CString &sSubdir = asDirsToTry[i];
		CString sDir = MEM_CARD_MOUNT_POINT[pn] + sSubdir + "/";

		SONGMAN->LoadAllFromProfileDir( sDir, (ProfileSlot) pn );
	}

	return true; // If a card is inserted, we want to use the memory card to save - even if the Profile load failed.
}
			
bool ProfileManager::LoadFirstAvailableProfile( PlayerNumber pn )
{
	if( LoadProfileFromMemoryCard(pn) )
		return true;

	if( LoadLocalProfileFromMachine(pn) )
		return true;
	
	return false;
}


bool ProfileManager::FastLoadProfileNameFromMemoryCard( CString sRootDir, CString &sName ) const
{
	vector<CString> asDirsToTry;
	GetMemoryCardProfileDirectoriesToTry( asDirsToTry );

	for( unsigned i = 0; i < asDirsToTry.size(); ++i )
	{
		const CString &sSubdir = asDirsToTry[i];
		CString sDir = sRootDir + sSubdir + "/";

		Profile profile;
		Profile::LoadResult res = profile.LoadEditableDataFromDir( sDir );
		if( res == Profile::success )
		{
			sName = profile.GetDisplayNameOrHighScoreName();
			return true;
		}
		else if( res != Profile::failed_no_profile )
			break;
	}

	return false;
}

void ProfileManager::SaveAllProfiles() const
{
	this->SaveMachineProfile();

	FOREACH_HumanPlayer( pn )
	{
		if( !IsPersistentProfile(pn) )
			continue;

		this->SaveProfile( pn );
	}
}

bool ProfileManager::SaveProfile( PlayerNumber pn ) const
{
	if( m_sProfileDir[pn].empty() )
		return false;

	bool b = GetProfile(pn)->SaveAllToDir( m_sProfileDir[pn], PREFSMAN->m_bSignProfileData );

	return b;
}

bool ProfileManager::SaveLocalProfile( CString sProfileID )
{
	Profile &profile = GetLocalProfile( sProfileID );
	CString sDir = LocalProfileIdToDir( sProfileID );
	bool b = profile.SaveAllToDir( sDir, PREFSMAN->m_bSignProfileData );
	return b;
}

void ProfileManager::UnloadProfile( PlayerNumber pn )
{
	m_sProfileDir[pn] = "";
	m_sProfileDirImportedFrom[pn] = "";
	m_bWasLoadedFromMemoryCard[pn] = false;
	m_bLastLoadWasTamperedOrCorrupt[pn] = false;
	m_bLastLoadWasFromLastGood[pn] = false;
	m_pMemoryCardProfile[pn]->InitAll();
	SONGMAN->FreeAllLoadedFromProfile( (ProfileSlot) pn );
}

const Profile* ProfileManager::GetProfile( PlayerNumber pn ) const
{
	ASSERT( pn >= 0 && pn < NUM_PLAYERS );

	if( m_sProfileDir[pn].empty() )
	{
		// return an empty profile
		return m_pMemoryCardProfile[pn];
	}
	else if( ProfileWasLoadedFromMemoryCard(pn) )
	{
		return m_pMemoryCardProfile[pn];
	}
	else
	{
		map<CString,Profile*>::iterator iter = g_mapLocalProfileDirToProfile.find( m_sProfileDir[pn] );
		ASSERT( iter != g_mapLocalProfileDirToProfile.end() );
		return iter->second;
	}
}

CString ProfileManager::GetPlayerName( PlayerNumber pn ) const
{
	const Profile *prof = GetProfile( pn );
	return prof ? prof->GetDisplayNameOrHighScoreName() : CString();
}


void ProfileManager::RefreshLocalProfilesFromDisk()
{
	FOREACHM( CString, Profile*, g_mapLocalProfileDirToProfile, iter )
		SAFE_DELETE( iter->second );
	g_mapLocalProfileDirToProfile.clear();

	vector<CString> vsProfileID;
	GetDirListing( USER_PROFILES_DIR "*", vsProfileID, true, false );
	FOREACH_CONST( CString, vsProfileID, s )
	{
		Profile *&pProfile = g_mapLocalProfileDirToProfile[*s];
		pProfile = new Profile;
		CString sProfileDir = LocalProfileIdToDir( *s );
		LOG->Trace(" '%s'", pProfile->GetDisplayNameOrHighScoreName().c_str());
		pProfile->LoadAllFromDir( sProfileDir, PREFSMAN->m_bSignProfileData );
	}
}

Profile &ProfileManager::GetLocalProfile( const CString &sProfileID )
{
	map<CString,Profile*>::iterator iter = g_mapLocalProfileDirToProfile.find( sProfileID );
	if( iter == g_mapLocalProfileDirToProfile.end() )
	{
		LOG->Warn( "ProfileID '%s' doesn't exist", sProfileID.c_str() );
		static Profile s_pro;
		s_pro.InitAll();
		return s_pro;
	}

	return *iter->second;
}

bool ProfileManager::CreateLocalProfile( CString sName, CString &sProfileIDOut )
{
	ASSERT( !sName.empty() );

	//
	// Find a directory directory name that's a number greater than all 
	// existing numbers.  This preserves the "order by create date".
	//
	int iMaxProfileNumber = -1;
	vector<CString> vs;
	GetLocalProfileIDs( vs );
	FOREACH_CONST( CString, vs, s )
		iMaxProfileNumber = atoi( *s );

	int iProfileNumber = iMaxProfileNumber + 1;
	CString sProfileID = ssprintf( "%08d", iProfileNumber );
	CString sProfileDir = LocalProfileIdToDir( sProfileID );

	bool bResult = Profile::CreateNewProfile( sProfileDir, sName, true );
	if( bResult )
		sProfileIDOut = sProfileID;
	else
		sProfileIDOut = "";
	RefreshLocalProfilesFromDisk();
	return bResult;
}

bool ProfileManager::RenameLocalProfile( CString sProfileID, CString sNewName )
{
	ASSERT( !sProfileID.empty() );

	CString sProfileDir = LocalProfileIdToDir( sProfileID );

	Profile pro;
	Profile::LoadResult lr;
	lr = pro.LoadAllFromDir( sProfileDir, PREFSMAN->m_bSignProfileData );
	if( lr != Profile::success )
		return false;
	pro.m_sDisplayName = sNewName;

	bool bResult = pro.SaveAllToDir( sProfileDir, PREFSMAN->m_bSignProfileData );
	RefreshLocalProfilesFromDisk();
	return bResult;
}

bool ProfileManager::DeleteLocalProfile( CString sProfileID )
{
	CString sProfileDir = LocalProfileIdToDir( sProfileID );
	bool bResult = DeleteRecursive( sProfileDir );

	RefreshLocalProfilesFromDisk();
	return bResult;
}

void ProfileManager::SaveMachineProfile() const
{
	// If the machine name has changed, make sure we use the new name.
	// It's important that this name be applied before the Player profiles 
	// are saved, so that the Player's profiles show the right machine name.
	const_cast<ProfileManager *> (this)->m_pMachineProfile->m_sDisplayName = PREFSMAN->m_sMachineName;

	m_pMachineProfile->SaveAllToDir( MACHINE_PROFILE_DIR, false ); /* don't sign machine profiles */
}

void ProfileManager::LoadMachineProfile()
{
	Profile::LoadResult lr = m_pMachineProfile->LoadAllFromDir(MACHINE_PROFILE_DIR, false);
	if( lr == Profile::failed_no_profile )
	{
		Profile::CreateNewProfile( MACHINE_PROFILE_DIR, "Machine", false );
		m_pMachineProfile->LoadAllFromDir( MACHINE_PROFILE_DIR, false );
	}

	// If the machine name has changed, make sure we use the new name
	m_pMachineProfile->m_sDisplayName = PREFSMAN->m_sMachineName;

	SONGMAN->FreeAllLoadedFromProfile( PROFILE_SLOT_MACHINE );
	SONGMAN->LoadAllFromProfileDir( MACHINE_PROFILE_DIR, PROFILE_SLOT_MACHINE );
}

bool ProfileManager::ProfileWasLoadedFromMemoryCard( PlayerNumber pn ) const
{
	return !m_sProfileDir[pn].empty() && m_bWasLoadedFromMemoryCard[pn];
}

bool ProfileManager::LastLoadWasTamperedOrCorrupt( PlayerNumber pn ) const
{
	return !m_sProfileDir[pn].empty() && m_bLastLoadWasTamperedOrCorrupt[pn];
}

bool ProfileManager::LastLoadWasFromLastGood( PlayerNumber pn ) const
{
	return !m_sProfileDir[pn].empty() && m_bLastLoadWasFromLastGood[pn];
}

CString ProfileManager::GetProfileDir( ProfileSlot slot ) const
{
	switch( slot )
	{
	case PROFILE_SLOT_PLAYER_1:
	case PROFILE_SLOT_PLAYER_2:
		return m_sProfileDir[slot];
	case PROFILE_SLOT_MACHINE:
		return MACHINE_PROFILE_DIR;
	default:
		ASSERT(0);
	}
}

CString ProfileManager::GetProfileDirImportedFrom( ProfileSlot slot ) const
{
	switch( slot )
	{
	case PROFILE_SLOT_PLAYER_1:
	case PROFILE_SLOT_PLAYER_2:
		return m_sProfileDirImportedFrom[slot];
	case PROFILE_SLOT_MACHINE:
		return "";
	default:
		ASSERT(0);
	}
}

const Profile* ProfileManager::GetProfile( ProfileSlot slot ) const
{
	switch( slot )
	{
	case PROFILE_SLOT_PLAYER_1:
	case PROFILE_SLOT_PLAYER_2:
		return GetProfile( (PlayerNumber)slot );
	case PROFILE_SLOT_MACHINE:
		return m_pMachineProfile;
	default:
		ASSERT(0);
	}
}

//
// General
//
void ProfileManager::IncrementToastiesCount( PlayerNumber pn )
{
	if( PROFILEMAN->IsPersistentProfile(pn) )
		++PROFILEMAN->GetProfile(pn)->m_iNumToasties;
	++PROFILEMAN->GetMachineProfile()->m_iNumToasties;
}

void ProfileManager::AddStepTotals( PlayerNumber pn, int iNumTapsAndHolds, int iNumJumps, int iNumHolds, int iNumRolls, int iNumMines, int iNumHands, float fCaloriesBurned )
{
	if( PROFILEMAN->IsPersistentProfile(pn) )
		PROFILEMAN->GetProfile(pn)->AddStepTotals( iNumTapsAndHolds, iNumJumps, iNumHolds, iNumRolls, iNumMines, iNumHands, fCaloriesBurned );
	PROFILEMAN->GetMachineProfile()->AddStepTotals( iNumTapsAndHolds, iNumJumps, iNumHolds, iNumRolls, iNumMines, iNumHands, fCaloriesBurned );
}

//
// Song stats
//
int ProfileManager::GetSongNumTimesPlayed( const Song* pSong, ProfileSlot slot ) const
{
	return GetProfile(slot)->GetSongNumTimesPlayed( pSong );
}

void ProfileManager::AddStepsScore( const Song* pSong, const Steps* pSteps, PlayerNumber pn, const HighScore &hs_, int &iPersonalIndexOut, int &iMachineIndexOut )
{
	HighScore hs = hs_;
	hs.fPercentDP = max( 0, hs.fPercentDP );	// bump up negative scores

	iPersonalIndexOut = -1;
	iMachineIndexOut = -1;

	// In event mode, set the score's name immediately to the Profile's last
	// used name.  If no profile last used name exists, use "EVNT".
	if( GAMESTATE->IsEventMode() )
	{
		Profile* pProfile = PROFILEMAN->GetProfile(pn);
		if( pProfile && !pProfile->m_sLastUsedHighScoreName.empty() )
			hs.sName = pProfile->m_sLastUsedHighScoreName;
		else
			hs.sName = "EVNT";
	}
	else
	{
		hs.sName = RANKING_TO_FILL_IN_MARKER[pn];
	}

	//
	// save high score	
	//
	if( PROFILEMAN->IsPersistentProfile(pn) )
		PROFILEMAN->GetProfile(pn)->AddStepsHighScore( pSong, pSteps, hs, iPersonalIndexOut );

	if( hs.fPercentDP >= PREFSMAN->m_fMinPercentageForMachineSongHighScore )
	{
		// don't leave machine high scores for edits loaded from the player's card
		if( !pSteps->IsAPlayerEdit() )
			PROFILEMAN->GetMachineProfile()->AddStepsHighScore( pSong, pSteps, hs, iMachineIndexOut );
	}

	//
	// save recent score	
	//
	if( PROFILEMAN->IsPersistentProfile(pn) )
		PROFILEMAN->GetProfile(pn)->AddStepsRecentScore( pSong, pSteps, hs );
	PROFILEMAN->GetMachineProfile()->AddStepsRecentScore( pSong, pSteps, hs );
}

void ProfileManager::IncrementStepsPlayCount( const Song* pSong, const Steps* pSteps, PlayerNumber pn )
{
	if( PROFILEMAN->IsPersistentProfile(pn) )
		PROFILEMAN->GetProfile(pn)->IncrementStepsPlayCount( pSong, pSteps );
	PROFILEMAN->GetMachineProfile()->IncrementStepsPlayCount( pSong, pSteps );
}

void ProfileManager::GetHighScoreForDifficulty( const Song *s, const Style *st, ProfileSlot slot, Difficulty dc, HighScore &hsOut ) const
{
	// return max grade of notes in difficulty class
	vector<Steps*> aNotes;
	s->GetSteps( aNotes, st->m_StepsType );
	StepsUtil::SortNotesArrayByDifficulty( aNotes );

	const Steps* pSteps = s->GetStepsByDifficulty( st->m_StepsType, dc );

	if( pSteps && PROFILEMAN->IsPersistentProfile(slot) )
		hsOut = PROFILEMAN->GetProfile(slot)->GetStepsHighScoreList(s,pSteps).GetTopScore();
	else
		hsOut = HighScore();
}


//
// Course stats
//
void ProfileManager::AddCourseScore( const Course* pCourse, const Trail* pTrail, PlayerNumber pn, const HighScore &hs_, int &iPersonalIndexOut, int &iMachineIndexOut )
{
	HighScore hs = hs_;
	hs.fPercentDP = max( 0, hs.fPercentDP );	// bump up negative scores

	iPersonalIndexOut = -1;
	iMachineIndexOut = -1;

	// In event mode, set the score's name immediately to the Profile's last
	// used name.  If no profile last used name exists, use "EVNT".
	if( GAMESTATE->IsEventMode() )
	{
		Profile* pProfile = PROFILEMAN->GetProfile(pn);
		if( pProfile && !pProfile->m_sLastUsedHighScoreName.empty() )
			hs.sName = pProfile->m_sLastUsedHighScoreName;
		else
			hs.sName = "EVNT";
	}
	else
	{
		hs.sName = RANKING_TO_FILL_IN_MARKER[pn];
	}


	//
	// save high score	
	//
	if( PROFILEMAN->IsPersistentProfile(pn) )
		PROFILEMAN->GetProfile(pn)->AddCourseHighScore( pCourse, pTrail, hs, iPersonalIndexOut );
	if( hs.fPercentDP >= PREFSMAN->m_fMinPercentageForMachineCourseHighScore )
		PROFILEMAN->GetMachineProfile()->AddCourseHighScore( pCourse, pTrail, hs, iMachineIndexOut );

	//
	// save recent score	
	//
	if( PROFILEMAN->IsPersistentProfile(pn) )
		PROFILEMAN->GetProfile(pn)->AddCourseRecentScore( pCourse, pTrail, hs );
	PROFILEMAN->GetMachineProfile()->AddCourseRecentScore( pCourse, pTrail, hs );
}

void ProfileManager::IncrementCoursePlayCount( const Course* pCourse, const Trail* pTrail, PlayerNumber pn )
{
	if( PROFILEMAN->IsPersistentProfile(pn) )
		PROFILEMAN->GetProfile(pn)->IncrementCoursePlayCount( pCourse, pTrail );
	PROFILEMAN->GetMachineProfile()->IncrementCoursePlayCount( pCourse, pTrail );
}


//
// Category stats
//
void ProfileManager::AddCategoryScore( StepsType st, RankingCategory rc, PlayerNumber pn, const HighScore &hs_, int &iPersonalIndexOut, int &iMachineIndexOut )
{
	HighScore hs = hs_;
	hs.sName = RANKING_TO_FILL_IN_MARKER[pn];
	if( PROFILEMAN->IsPersistentProfile(pn) )
		PROFILEMAN->GetProfile(pn)->AddCategoryHighScore( st, rc, hs, iPersonalIndexOut );
	if( hs.fPercentDP > PREFSMAN->m_fMinPercentageForMachineSongHighScore )
		PROFILEMAN->GetMachineProfile()->AddCategoryHighScore( st, rc, hs, iMachineIndexOut );
}

void ProfileManager::IncrementCategoryPlayCount( StepsType st, RankingCategory rc, PlayerNumber pn )
{
	if( PROFILEMAN->IsPersistentProfile(pn) )
		PROFILEMAN->GetProfile(pn)->IncrementCategoryPlayCount( st, rc );
	PROFILEMAN->GetMachineProfile()->IncrementCategoryPlayCount( st, rc );
}

bool ProfileManager::IsPersistentProfile( ProfileSlot slot ) const
{
	switch( slot )
	{
	case PROFILE_SLOT_PLAYER_1:
	case PROFILE_SLOT_PLAYER_2:
		return GAMESTATE->IsHumanPlayer((PlayerNumber)slot) && !m_sProfileDir[slot].empty(); 
	case PROFILE_SLOT_MACHINE:
		return true;
	default:
		ASSERT(0);
		return false;
	}
}

void ProfileManager::GetLocalProfileIDs( vector<CString> &vsProfileIDsOut ) const
{
	vsProfileIDsOut.clear();
	FOREACHM_CONST( CString, Profile*, g_mapLocalProfileDirToProfile, iter )
	{
		CString sID = LocalProfileDirToId( iter->first );
		vsProfileIDsOut.push_back( sID );
	}
}

void ProfileManager::GetLocalProfileDisplayNames( vector<CString> &vsProfileDisplayNamesOut ) const
{
	vsProfileDisplayNamesOut.clear();
	FOREACHM_CONST( CString, Profile*, g_mapLocalProfileDirToProfile, iter )
		vsProfileDisplayNamesOut.push_back( iter->second->m_sDisplayName );
}

int ProfileManager::GetNumLocalProfiles() const
{
	return g_mapLocalProfileDirToProfile.size();
}


// lua start
#include "LuaBinding.h"

class LunaProfileManager: public Luna<ProfileManager>
{
public:
	LunaProfileManager() { LUA->Register( Register ); }

	static int IsPersistentProfile( T* p, lua_State *L )	{ lua_pushboolean(L, p->IsPersistentProfile((PlayerNumber)IArg(1)) ); return 1; }
	static int GetProfile( T* p, lua_State *L )				{ PlayerNumber pn = (PlayerNumber)IArg(1); Profile* pP = p->GetProfile(pn); ASSERT(pP); pP->PushSelf(L); return 1; }
	static int GetMachineProfile( T* p, lua_State *L )		{ p->GetMachineProfile()->PushSelf(L); return 1; }
	static int SaveMachineProfile( T* p, lua_State *L )		{ p->SaveMachineProfile(); return 1; }
	static int GetLocalProfile( T* p, lua_State *L )		{ Profile &pro = p->GetLocalProfile(SArg(1)); pro.PushSelf(L); return 1; }

	static void Register(lua_State *L)
	{
		ADD_METHOD( IsPersistentProfile )
		ADD_METHOD( GetProfile )
		ADD_METHOD( GetMachineProfile )
		ADD_METHOD( SaveMachineProfile )
		ADD_METHOD( GetLocalProfile )
		Luna<T>::Register( L );

		// Add global singleton if constructed already.  If it's not constructed yet,
		// then we'll register it later when we reinit Lua just before 
		// initializing the display.
		if( PROFILEMAN )
		{
			lua_pushstring(L, "PROFILEMAN");
			PROFILEMAN->PushSelf( L );
			lua_settable(L, LUA_GLOBALSINDEX);
		}
	}
};

LUA_REGISTER_CLASS( ProfileManager )
// lua end

/*
 * (c) 2003-2004 Chris Danford
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
