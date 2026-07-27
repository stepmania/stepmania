#include "global.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageFile.h"
#include "RageFileManager.h"
#include "GameConstantsAndTypes.h"
#include "SongManager.h"
#include "GameState.h"
#include "Song.h"
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
#include "Character.h"
#include "CharacterManager.h"

#include <cstddef>
#include <vector>


ProfileManager*	PROFILEMAN = nullptr;	// global and accessible from anywhere in our program

#define ID_DIGITS 8
#define ID_DIGITS_STR "8"
#define MAX_ID 99999999

static void DefaultLocalProfileIDInit( std::size_t /*PlayerNumber*/ i, RString &sNameOut, RString &defaultValueOut )
{
	sNameOut = ssprintf( "DefaultLocalProfileIDP%d", int(i+1) );
	defaultValueOut = "";
}

Preference<bool> ProfileManager::m_bProfileStepEdits( "ProfileStepEdits", true );
Preference<bool> ProfileManager::m_bProfileCourseEdits( "ProfileCourseEdits", true );
Preference1D<RString> ProfileManager::m_sDefaultLocalProfileID( DefaultLocalProfileIDInit, NUM_PLAYERS );

const RString NEW_MEM_CARD_NAME	=	"";
const RString USER_PROFILES_DIR	=	"/Save/LocalProfiles/";
const RString MACHINE_PROFILE_DIR =	"/Save/MachineProfile/";
const RString LAST_GOOD_SUBDIR	=	"LastGood/";


// Directories to search for a profile if m_sMemoryCardProfileSubdir doesn't
// exist, separated by ";":
static Preference<RString> g_sMemoryCardProfileImportSubdirs( "MemoryCardProfileImportSubdirs", "StepMania 5.1;StepMania 5;In The Groove 2" );

static RString LocalProfileIDToDir( const RString &sProfileID ) { return USER_PROFILES_DIR + sProfileID + "/"; }
static RString LocalProfileDirToID( const RString &sDir ) { return Basename( sDir ); }

struct DirAndProfile
{
	RString sDir;
	Profile profile;
	void swap(DirAndProfile& other)
	{
		sDir.swap(other.sDir);
		profile.swap(other.profile);
	}
};
static std::vector<DirAndProfile> g_vLocalProfile;


static ThemeMetric<bool>	FIXED_PROFILES		( "ProfileManager", "FixedProfiles" );
static ThemeMetric<int>		NUM_FIXED_PROFILES	( "ProfileManager", "NumFixedProfiles" );
#define FIXED_PROFILE_CHARACTER_ID( i ) THEME->GetMetric( "ProfileManager", ssprintf("FixedProfileCharacterID%d",int(i+1)) )


ProfileManager::ProfileManager()
	:m_stats_prefix("")
{
	m_pMachineProfile = new Profile;
	FOREACH_PlayerNumber(pn)
		m_pMemoryCardProfile[pn] = new Profile;

	// Register with Lua.
	{
		Lua *L = LUA->Get();
		lua_pushstring( L, "PROFILEMAN" );
		this->PushSelf( L );
		lua_settable( L, LUA_GLOBALSINDEX );
		LUA->Release( L );
	}
}

ProfileManager::~ProfileManager()
{
	// Unregister with Lua.
	LUA->UnsetGlobal( "PROFILEMAN" );

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
		m_bNeedToBackUpLastLoad[p] = false;
		m_bNewProfile[p] = false;
	}

	LoadMachineProfile();

	RefreshLocalProfilesFromDisk();

	if( FIXED_PROFILES )
	{
		// resize to the fixed number
		if( (int)g_vLocalProfile.size() > NUM_FIXED_PROFILES )
			g_vLocalProfile.erase( g_vLocalProfile.begin()+NUM_FIXED_PROFILES, g_vLocalProfile.end() );

		for( int i=g_vLocalProfile.size(); i<NUM_FIXED_PROFILES; i++ )
		{
			RString sCharacterID = FIXED_PROFILE_CHARACTER_ID( i );
			Character *pCharacter = CHARMAN->GetCharacterFromID( sCharacterID );
			ASSERT_M( pCharacter != nullptr, sCharacterID );
			RString sProfileID;
			bool b = CreateLocalProfile( pCharacter->GetDisplayName(), sProfileID );
			ASSERT( b );
			Profile* pProfile = GetLocalProfile( sProfileID );
			ASSERT_M( pProfile != nullptr, sProfileID );
			pProfile->m_sCharacterID = sCharacterID;
			SaveLocalProfile( sProfileID );
		}

		ASSERT( (int)g_vLocalProfile.size() == NUM_FIXED_PROFILES );
	}
}

bool ProfileManager::FixedProfiles() const
{
	return FIXED_PROFILES;
}

ProfileLoadResult ProfileManager::LoadProfile( PlayerNumber pn, RString sProfileDir, bool bIsMemCard )
{
	LOG->Trace( "LoadingProfile P%d, %s, %d", pn+1, sProfileDir.c_str(), bIsMemCard );

	ASSERT( !sProfileDir.empty() );
	ASSERT( sProfileDir.Right(1) == "/" );


	m_sProfileDir[pn] = sProfileDir;
	m_bWasLoadedFromMemoryCard[pn] = bIsMemCard;
	m_bLastLoadWasFromLastGood[pn] = false;
	m_bNeedToBackUpLastLoad[pn] = false;

	// Try to load the original, non-backup data.
	ProfileLoadResult lr = GetProfile(pn)->LoadAllFromDir( m_sProfileDir[pn], PREFSMAN->m_bSignProfileData );

	RString sBackupDir = m_sProfileDir[pn] + LAST_GOOD_SUBDIR;

	if( lr == ProfileLoadResult_Success )
	{
		/* Next time the profile is written, move this good profile into LastGood. */
		m_bNeedToBackUpLastLoad[pn] = true;
	}

	m_bLastLoadWasTamperedOrCorrupt[pn] = lr == ProfileLoadResult_FailedTampered;

	//
	// Try to load from the backup if the original data fails to load
	//
	if( lr == ProfileLoadResult_FailedTampered )
	{
		lr = GetProfile(pn)->LoadAllFromDir( sBackupDir, PREFSMAN->m_bSignProfileData );
		m_bLastLoadWasFromLastGood[pn] = lr == ProfileLoadResult_Success;

		/* If the LastGood profile doesn't exist at all, and the actual profile was failed_tampered,
		 * then the error should be failed_tampered and not failed_no_profile. */
		if( lr == ProfileLoadResult_FailedNoProfile )
		{
			LOG->Trace( "Profile was corrupt and LastGood for %s doesn't exist; error is ProfileLoadResult_FailedTampered",
					sProfileDir.c_str() );
			lr = ProfileLoadResult_FailedTampered;
		}
	}

	if(lr == ProfileLoadResult_Success)
	{
		Profile* prof= GetProfile(pn);
		if(prof->m_sDisplayName.empty())
		{
			prof->m_sDisplayName= PlayerNumberToLocalizedString(pn);
		}
		prof->LoadCustomFunction(sProfileDir, pn);
		prof->LoadSongsFromDir(sProfileDir, ProfileSlot(pn));
	}

	LOG->Trace( "Done loading profile - result %d", lr );

	return lr;
}

bool ProfileManager::LoadLocalProfileFromMachine( PlayerNumber pn )
{
	RString sProfileID = m_sDefaultLocalProfileID[pn];
	if( sProfileID.empty() )
	{
		m_sProfileDir[pn] = "";
		return false;
	}

	// Make sure the profile is currently not in use by another player
	FOREACH_HumanPlayer( pl )
	{
		if( PROFILEMAN->IsPersistentProfile(pl) && PROFILEMAN->GetProfile(pl) == PROFILEMAN->GetLocalProfile(sProfileID) )
		{
			m_sProfileDir[pn] = "";
			return false;
		}
	}

	m_sProfileDir[pn] = LocalProfileIDToDir( sProfileID );
	m_bWasLoadedFromMemoryCard[pn] = false;
	m_bLastLoadWasFromLastGood[pn] = false;

	if( GetLocalProfile(sProfileID) == nullptr )
	{
		m_sProfileDir[pn] = "";
		return false;
	}

	GetProfile(pn)->LoadCustomFunction(m_sProfileDir[pn], pn);

	return true;
}

void ProfileManager::GetMemoryCardProfileDirectoriesToTry( std::vector<RString> &asDirsToTry )
{
	/* Try to load the preferred profile. */
	asDirsToTry.push_back( PREFSMAN->m_sMemoryCardProfileSubdir.Get() );

	/* If that failed, try loading from all fallback directories. */
	split( g_sMemoryCardProfileImportSubdirs, ";", asDirsToTry, true );
}

bool ProfileManager::LoadProfileFromMemoryCard( PlayerNumber pn, bool bLoadEdits )
{
	UnloadProfile( pn );

	// mount slot
	if( MEMCARDMAN->GetCardState(pn) != MemoryCardState_Ready )
		return false;

	std::vector<RString> asDirsToTry;
	GetMemoryCardProfileDirectoriesToTry( asDirsToTry );
	m_bNewProfile[pn] = true;

	for( unsigned i = 0; i < asDirsToTry.size(); ++i )
	{
		const RString &sSubdir = asDirsToTry[i];
		RString sDir = MEM_CARD_MOUNT_POINT[pn] + sSubdir + "/";

		/* If the load fails with ProfileLoadResult_FailedNoProfile, keep searching.  However,
		 * if it fails with failed_tampered, data existed but couldn't be loaded;
		 * we don't want to mess with it, since it's confusing and may wipe out
		 * recoverable backup data.  The only time we really want to import data
		 * is on the very first use, when the new profile doesn't exist at all,
		 * but we also want to import scores in the case where the player created
		 * a directory for edits before playing, so keep searching if the directory
		 * exists with exists with no scores. */
		ProfileLoadResult res = LoadProfile( pn, sDir, true );
		if( res == ProfileLoadResult_Success )
		{
			m_bNewProfile[pn] = false;
			/* If importing, store the directory we imported from, for display purposes. */
			if( i > 0 )
				m_sProfileDirImportedFrom[pn] = asDirsToTry[i];
			break;
		}

		if( res == ProfileLoadResult_FailedTampered )
		{
			m_bNewProfile[pn] = false;
			break;
		}
	}

	/* If we imported a profile fallback directory, change the memory card
	 * directory back to the preferred directory: never write over imported
	 * scores. */
	m_sProfileDir[pn] = MEM_CARD_MOUNT_POINT[pn] + (RString)PREFSMAN->m_sMemoryCardProfileSubdir + "/";

	/* Load edits from all fallback directories, newest first. */
	if( bLoadEdits )
	{
		for( unsigned i = 0; i < asDirsToTry.size(); ++i )
		{
			const RString &sSubdir = asDirsToTry[i];
			RString sDir = MEM_CARD_MOUNT_POINT[pn] + sSubdir + "/";

			if( m_bProfileStepEdits )
				SONGMAN->LoadStepEditsFromProfileDir( sDir, (ProfileSlot) pn );
			if( m_bProfileCourseEdits )
				SONGMAN->LoadCourseEditsFromProfileDir( sDir, (ProfileSlot) pn );
		}
	}

	return true; // If a card is inserted, we want to use the memory card to save - even if the Profile load failed.
}

bool ProfileManager::LoadFirstAvailableProfile( PlayerNumber pn, bool bLoadEdits )
{
	if( LoadProfileFromMemoryCard(pn, bLoadEdits) )
		return true;

	if( LoadLocalProfileFromMachine(pn) )
		return true;

	return false;
}


bool ProfileManager::FastLoadProfileNameFromMemoryCard( RString sRootDir, RString &sName ) const
{
	std::vector<RString> asDirsToTry;
	GetMemoryCardProfileDirectoriesToTry( asDirsToTry );

	for( unsigned i = 0; i < asDirsToTry.size(); ++i )
	{
		const RString &sSubdir = asDirsToTry[i];
		RString sDir = sRootDir + sSubdir + "/";

		Profile profile;
		ProfileLoadResult res = profile.LoadEditableDataFromDir( sDir );
		if( res == ProfileLoadResult_Success )
		{
			sName = profile.GetDisplayNameOrHighScoreName();
			return true;
		}
		else if( res != ProfileLoadResult_FailedNoProfile )
			break;
	}

	return false;
}

bool ProfileManager::SaveProfile( PlayerNumber pn ) const
{
	if( m_sProfileDir[pn].empty() )
		return false;

	/*
	 * If the profile we're writing was loaded from the primary (non-backup)
	 * data, then we've validated it and know it's good.  Before writing our
	 * new data, move the old, good data to the backup.  (Only do this once;
	 * if we save the profile more than once, we haven't re-validated the
	 * newly written data.)
	 */
	if( m_bNeedToBackUpLastLoad[pn] )
	{
		m_bNeedToBackUpLastLoad[pn] = false;
		RString sBackupDir = m_sProfileDir[pn] + LAST_GOOD_SUBDIR;
		Profile::MoveBackupToDir( m_sProfileDir[pn], sBackupDir );
	}

	bool b = GetProfile(pn)->SaveAllToDir( m_sProfileDir[pn], PREFSMAN->m_bSignProfileData );

	return b;
}

bool ProfileManager::SaveLocalProfile( RString sProfileID )
{
	const Profile *pProfile = GetLocalProfile( sProfileID );
	ASSERT( pProfile != nullptr );
	RString sDir = LocalProfileIDToDir( sProfileID );
	bool b = pProfile->SaveAllToDir( sDir, PREFSMAN->m_bSignProfileData );
	return b;
}

void ProfileManager::UnloadProfile( PlayerNumber pn )
{
	if( m_sProfileDir[pn].empty() )
	{
		// Don't bother unloading a profile that wasn't loaded in the first place.
		// Saves us an expensive and pointless trip through all the songs.
		return;
	}
	m_sProfileDir[pn] = "";
	m_sProfileDirImportedFrom[pn] = "";
	m_bWasLoadedFromMemoryCard[pn] = false;
	m_bLastLoadWasTamperedOrCorrupt[pn] = false;
	m_bLastLoadWasFromLastGood[pn] = false;
	m_bNeedToBackUpLastLoad[pn] = false;
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
		RString sProfileID = LocalProfileDirToID( m_sProfileDir[pn] );
		return GetLocalProfile( sProfileID );
	}
}

RString ProfileManager::GetPlayerName( PlayerNumber pn ) const
{
	const Profile *prof = GetProfile( pn );
	return prof ? prof->GetDisplayNameOrHighScoreName() : RString();
}


void ProfileManager::UnloadAllLocalProfiles()
{
	g_vLocalProfile.clear();
}

static void add_category_to_global_list(std::vector<DirAndProfile>& cat)
{
	g_vLocalProfile.insert(g_vLocalProfile.end(), cat.begin(), cat.end());
}

void ProfileManager::RefreshLocalProfilesFromDisk()
{
	UnloadAllLocalProfiles();

	switch(PREFSMAN->m_ProfileSortOrder)
	{
		case ProfileSortOrder_Alphabetical:
			LoadLocalProfilesByName();
			break;
		case ProfileSortOrder_Recent:
			LoadLocalProfilesByRecent();
			break;
		default:
			LoadLocalProfilesByPriority();
			break;
	}
}

void ProfileManager::LoadLocalProfilesByPriority()
{
	std::vector<RString> profile_ids;
	GetDirListing(USER_PROFILES_DIR + "*", profile_ids, true, true);
	// Profiles have 3 types:
	// 1.  Guest profiles:
	//   Meant for use by guests, always at the top of the list.
	// 2.  Normal profiles:
	//   Meant for normal use, listed after guests.
	// e.  Test profiles:
	//   Meant for use when testing things, listed last.
	// If the user renames a profile directory manually, that should not be a
	// problem. -Kyz
	std::map<ProfileType, std::vector<DirAndProfile>> categorized_profiles;
	// The type data for a profile is in its own file so that loading isn't
	// slowed down by copying temporary profiles around to make sure the list
	// is sorted.  The profiles are loaded at the end. -Kyz
	for (RString const &id : profile_ids)
	{
		DirAndProfile derp;
		derp.sDir= id + "/";
		derp.profile.LoadTypeFromDir(derp.sDir);
		std::map<ProfileType, std::vector<DirAndProfile>>::iterator category=
			categorized_profiles.find(derp.profile.m_Type);
		if(category == categorized_profiles.end())
		{
			categorized_profiles[derp.profile.m_Type].push_back(derp);
		}
		else
		{
			bool inserted= false;
			for (auto curr = g_vLocalProfile.begin(); curr != g_vLocalProfile.end(); ++curr)
			{
				if(curr->profile.m_ListPriority > derp.profile.m_ListPriority)
				{
					category->second.insert(curr, derp);
					inserted= true;
					break;
				}
			}
			if(!inserted)
			{
				category->second.push_back(derp);
			}
		}
	}
	add_category_to_global_list(categorized_profiles[ProfileType_Guest]);
	add_category_to_global_list(categorized_profiles[ProfileType_Normal]);
	add_category_to_global_list(categorized_profiles[ProfileType_Test]);
	for (DirAndProfile &curr : g_vLocalProfile)
	{
		curr.profile.LoadAllFromDir(curr.sDir, PREFSMAN->m_bSignProfileData);
	}
}

void ProfileManager::LoadLocalProfilesByName()
{
	std::vector<RString> profile_ids;
	GetDirListing(USER_PROFILES_DIR + "*", profile_ids, true, true);

	// Create separate vectors for each profile type
	std::vector<DirAndProfile> guestProfiles;
	std::vector<DirAndProfile> normalProfiles;
	std::vector<DirAndProfile> testProfiles;

	for (RString const &id : profile_ids)
	{
		DirAndProfile derp;
		derp.sDir= id + "/";
		derp.profile.LoadEditableDataFromDir(derp.sDir);
		derp.profile.LoadTypeFromDir(derp.sDir);
		// insert profile into the appropriate vector based on profile type
		switch(derp.profile.m_Type)
		{
			case ProfileType_Guest:
				guestProfiles.push_back(derp);
				break;
			case ProfileType_Normal:
				normalProfiles.push_back(derp);
				break;
			case ProfileType_Test:
				testProfiles.push_back(derp);
				break;
			default:
				break;
		}
	}

	// Sort each vector by display name
	if (PREFSMAN->m_bProfileSortOrderAscending) {
		auto displayNameAscending = [](const DirAndProfile &a, const DirAndProfile &b)
		{
			return a.profile.m_sDisplayName.CompareNoCase(b.profile.m_sDisplayName) < 0;
		};
		std::sort(guestProfiles.begin(), guestProfiles.end(), displayNameAscending);
		std::sort(normalProfiles.begin(), normalProfiles.end(), displayNameAscending);
		std::sort(testProfiles.begin(), testProfiles.end(), displayNameAscending);
	} else {
		auto displayNameDescending = [](const DirAndProfile &a, const DirAndProfile &b)
		{
			return a.profile.m_sDisplayName.CompareNoCase(b.profile.m_sDisplayName) > 0;
		};
		std::sort(guestProfiles.begin(), guestProfiles.end(), displayNameDescending);
		std::sort(normalProfiles.begin(), normalProfiles.end(), displayNameDescending);
		std::sort(testProfiles.begin(), testProfiles.end(), displayNameDescending);
	}

	add_category_to_global_list(guestProfiles);
	add_category_to_global_list(normalProfiles);
	add_category_to_global_list(testProfiles);

	for (DirAndProfile &curr : g_vLocalProfile)
	{
		curr.profile.LoadAllFromDir(curr.sDir, PREFSMAN->m_bSignProfileData);
	}

}

// This function is used within RefreshLocalProfilesFromDisk() to sort the profiles by date.
void ProfileManager::LoadLocalProfilesByRecent()
{

	std::vector<RString> profile_ids;
	GetDirListing(USER_PROFILES_DIR + "*", profile_ids, true, true);

	// Create separate vectors for each profile type
	std::vector<DirAndProfile> guestProfiles;
	std::vector<DirAndProfile> normalProfiles;
	std::vector<DirAndProfile> testProfiles;

	// The type data for a profile is in its own file so that loading isn't
	// slowed down by copying temporary profiles around to make sure the list
	// is sorted.	The profiles are loaded at the end. -Kyz
	for (RString const &id : profile_ids)
	{
		DirAndProfile derp;
		derp.sDir= id + "/";
		derp.profile.LoadTypeFromDir(derp.sDir);
		// insert profile into the appropriate vector based on profile type
		switch(derp.profile.m_Type)
		{
			case ProfileType_Guest:
				guestProfiles.push_back(derp);
				break;
			case ProfileType_Normal:
				normalProfiles.push_back(derp);
				break;
			case ProfileType_Test:
				testProfiles.push_back(derp);
				break;
			default:
				break;
		}
	}

	// Sort each vector by date
	if (PREFSMAN->m_bProfileSortOrderAscending) {
		auto lastPlayedAscending = [](const DirAndProfile &a, const DirAndProfile &b)
		{
			return a.profile.m_LastPlayedDate > b.profile.m_LastPlayedDate;
		};
		std::sort(guestProfiles.begin(), guestProfiles.end(), lastPlayedAscending);
		std::sort(normalProfiles.begin(), normalProfiles.end(), lastPlayedAscending);
		std::sort(testProfiles.begin(), testProfiles.end(), lastPlayedAscending);
	} else {
		auto lastPlayedDescending = [](const DirAndProfile &a, const DirAndProfile &b)
		{
			return a.profile.m_LastPlayedDate < b.profile.m_LastPlayedDate;
		};
		std::sort(guestProfiles.begin(), guestProfiles.end(), lastPlayedDescending);
		std::sort(normalProfiles.begin(), normalProfiles.end(), lastPlayedDescending);
		std::sort(testProfiles.begin(), testProfiles.end(), lastPlayedDescending);
	}

	add_category_to_global_list(guestProfiles);
	add_category_to_global_list(normalProfiles);
	add_category_to_global_list(testProfiles);

	for (DirAndProfile &curr : g_vLocalProfile)
	{
		curr.profile.LoadAllFromDir(curr.sDir, PREFSMAN->m_bSignProfileData);
	}
}

const Profile *ProfileManager::GetLocalProfile( const RString &sProfileID ) const
{
	RString sDir = LocalProfileIDToDir( sProfileID );
	for (DirAndProfile const &dap : g_vLocalProfile)
	{
		const RString &sOther = dap.sDir;
		if( sOther == sDir )
			return &dap.profile;
	}

	return nullptr;
}

bool ProfileManager::CreateLocalProfile( RString sName, RString &sProfileIDOut )
{
	ASSERT( !sName.empty() );

	// Find a directory directory name that's a number greater than all
	// existing numbers.  This preserves the "order by create date".
	// Profile IDs are actually the directory names, so they can be any string,
	// and we have to handle the case where the user renames one.
	// Since the user can rename them, they might have any number, wrapping our
	// counter or setting it to a ridiculous value.  That case must also be
	// handled. -Kyz
	int max_profile_number= -1;
	int first_free_number= 0;
	std::vector<RString> profile_ids;
	GetLocalProfileIDs(profile_ids);
	for (std::vector<RString>::const_iterator id = profile_ids.begin(); id != profile_ids.end(); ++id)
	{
		int tmp= 0;
		if((*id) >> tmp)
		{
			// The profile ids are already in order, so we don't have to handle the
			// case where 5 is encountered before 3.
			if(tmp == first_free_number)
			{
				++first_free_number;
			}
			max_profile_number= std::max(tmp, max_profile_number);
		}
	}

	int profile_number = max_profile_number + 1;
	// Prevent profiles from going over the 8 digit limit.
	if(profile_number > MAX_ID || profile_number < 0)
	{
		profile_number= first_free_number;
	}
	ASSERT_M(profile_number >= 0 && profile_number <= MAX_ID,
		"Too many profiles, cannot assign ID to new profile.");
	RString profile_id = ssprintf( "%0" ID_DIGITS_STR "d", profile_number );

	// make sure this id doesn't already exist
	ASSERT_M(GetLocalProfile(profile_id) == nullptr,
		ssprintf("creating profile with ID \"%s\" that already exists",
		profile_id.c_str()));

	// Create the new profile.
	Profile *pProfile = new Profile;
	pProfile->m_sDisplayName = sName;
	pProfile->m_sCharacterID = CHARMAN->GetRandomCharacter()->m_sCharacterID;

	// Save it to disk.
	RString sProfileDir = LocalProfileIDToDir(profile_id);
	if( !pProfile->SaveAllToDir(sProfileDir, PREFSMAN->m_bSignProfileData) )
	{
		delete pProfile;
		sProfileIDOut = "";
		return false;
	}

	AddLocalProfileByID(pProfile, profile_id);

	sProfileIDOut = profile_id;
	return true;
}

static void InsertProfileIntoList(DirAndProfile& derp)
{
	bool inserted = false;
	derp.profile.m_ListPriority = 0;
	int index = -1;
	for (auto curr = g_vLocalProfile.begin(); curr != g_vLocalProfile.end(); ++curr)
	{
		++index;
		if(curr->profile.m_Type > derp.profile.m_Type)
		{
			derp.profile.SaveTypeToDir(derp.sDir);
			g_vLocalProfile.insert(curr, derp);
			inserted = true;
			break;
		}
		else if(curr->profile.m_Type == derp.profile.m_Type)
		{
			++derp.profile.m_ListPriority;
		}
	}

	if(!inserted)
	{
		derp.profile.SaveTypeToDir(derp.sDir);
		g_vLocalProfile.push_back(derp);
	}

	// The above should run regardless of profile sort in case the user decides to change
	// back to priority sorting. If we're using Recent sorting, move the new profile to
	// the top, if we're using alphabetical find where it belongs.
	switch (PREFSMAN->m_ProfileSortOrder) {
		case ProfileSortOrder_Recent:
			PROFILEMAN->MoveProfileTopBottom(index, PREFSMAN->m_bProfileSortOrderAscending);
			break;
		case ProfileSortOrder_Alphabetical:
			PROFILEMAN->MoveProfileSorted(index, PREFSMAN->m_bProfileSortOrderAscending);
			break;
		default:
			break;
	}
}

void ProfileManager::AddLocalProfileByID( Profile *pProfile, RString sProfileID )
{
	// make sure this id doesn't already exist
	ASSERT_M( GetLocalProfile(sProfileID) == nullptr,
		ssprintf("creating \"%s\" \"%s\" that already exists",
		pProfile->m_sDisplayName.c_str(), sProfileID.c_str()) );

	DirAndProfile derp;
	derp.sDir= LocalProfileIDToDir(sProfileID);
	derp.profile= *pProfile;
	InsertProfileIntoList(derp);
}

bool ProfileManager::RenameLocalProfile( RString sProfileID, RString sNewName )
{
	ASSERT( !sProfileID.empty() );

	Profile *pProfile = ProfileManager::GetLocalProfile( sProfileID );
	ASSERT( pProfile != nullptr );
	pProfile->m_sDisplayName = sNewName;

	RString sProfileDir = LocalProfileIDToDir( sProfileID );
	return pProfile->SaveAllToDir( sProfileDir, PREFSMAN->m_bSignProfileData );
}

bool ProfileManager::DeleteLocalProfile( RString sProfileID )
{
	Profile *pProfile = ProfileManager::GetLocalProfile( sProfileID );
	ASSERT( pProfile != nullptr );
	RString sProfileDir = LocalProfileIDToDir( sProfileID );

	// flush directory cache in an attempt to get this working
	FILEMAN->FlushDirCache( sProfileDir );

	for (std::vector<DirAndProfile>::iterator i = g_vLocalProfile.begin(); i != g_vLocalProfile.end(); ++i)
	{
		if( i->sDir == sProfileDir )
		{
			if( DeleteRecursive(sProfileDir) )
			{
				g_vLocalProfile.erase( i );

				// Delete all references to this profileID
				for (Preference<RString> *j : m_sDefaultLocalProfileID.m_v)
				{
					if( j->Get() == sProfileID )
						j->Set( "" );
				}
				return true;
			}
			else
			{
				LOG->Warn("[ProfileManager::DeleteLocalProfile] DeleteRecursive(%s) failed",
					sProfileID.c_str() );
				return false;
			}
		}
	}

	LOG->Warn( "DeleteLocalProfile: ProfileID '%s' doesn't exist", sProfileID.c_str() );
	return false;
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
	ProfileLoadResult lr = m_pMachineProfile->LoadAllFromDir(MACHINE_PROFILE_DIR, false);
	if( lr == ProfileLoadResult_FailedNoProfile )
	{
		m_pMachineProfile->InitAll();
		m_pMachineProfile->SaveAllToDir( MACHINE_PROFILE_DIR, false ); /* don't sign machine profiles */
	}

	// If the machine name has changed, make sure we use the new name
	m_pMachineProfile->m_sDisplayName = PREFSMAN->m_sMachineName;

	LoadMachineProfileEdits();
}

void ProfileManager::LoadMachineProfileEdits()
{
	SONGMAN->FreeAllLoadedFromProfile( ProfileSlot_Machine );
	SONGMAN->LoadStepEditsFromProfileDir( MACHINE_PROFILE_DIR, ProfileSlot_Machine );
	SONGMAN->LoadCourseEditsFromProfileDir( MACHINE_PROFILE_DIR, ProfileSlot_Machine );
}

bool ProfileManager::ProfileWasLoadedFromMemoryCard( PlayerNumber pn ) const
{
	return !m_sProfileDir[pn].empty() && m_bWasLoadedFromMemoryCard[pn];
}

bool ProfileManager::ProfileFromMemoryCardIsNew( PlayerNumber pn ) const
{
	return GetProfile(pn) && m_bWasLoadedFromMemoryCard[pn] && m_bNewProfile[pn];
}

bool ProfileManager::LastLoadWasTamperedOrCorrupt( PlayerNumber pn ) const
{
	return !m_sProfileDir[pn].empty() && m_bLastLoadWasTamperedOrCorrupt[pn];
}

bool ProfileManager::LastLoadWasFromLastGood( PlayerNumber pn ) const
{
	return !m_sProfileDir[pn].empty() && m_bLastLoadWasFromLastGood[pn];
}

const RString& ProfileManager::GetProfileDir( ProfileSlot slot ) const
{
	switch( slot )
	{
	case ProfileSlot_Player1:
	case ProfileSlot_Player2:
		return m_sProfileDir[slot];
	case ProfileSlot_Machine:
		return MACHINE_PROFILE_DIR;
	default:
		FAIL_M("Invalid profile slot chosen: unable to get the directory!");
	}
}

RString ProfileManager::GetProfileDirImportedFrom( ProfileSlot slot ) const
{
	switch( slot )
	{
	case ProfileSlot_Player1:
	case ProfileSlot_Player2:
		return m_sProfileDirImportedFrom[slot];
	case ProfileSlot_Machine:
		return RString();
	default:
		FAIL_M("Invalid profile slot chosen: unable to get the directory!");
	}
}

const Profile* ProfileManager::GetProfile( ProfileSlot slot ) const
{
	switch( slot )
	{
	case ProfileSlot_Player1:
	case ProfileSlot_Player2:
		return GetProfile( (PlayerNumber)slot );
	case ProfileSlot_Machine:
		return m_pMachineProfile;
	default:
		FAIL_M("Invalid profile slot chosen: unable to get the profile!");
	}
}

void ProfileManager::MergeLocalProfiles(RString const& from_id, RString const& to_id)
{
	Profile* from= GetLocalProfile(from_id);
	Profile* to= GetLocalProfile(to_id);
	if(from == nullptr || to == nullptr)
	{
		return;
	}
	to->MergeScoresFromOtherProfile(from, false,
		LocalProfileIDToDir(from_id), LocalProfileIDToDir(to_id));
}

void ProfileManager::MergeLocalProfileIntoMachine(RString const& from_id, bool skip_totals)
{
	Profile* from= GetLocalProfile(from_id);
	if(from == nullptr)
	{
		return;
	}
	GetMachineProfile()->MergeScoresFromOtherProfile(from, skip_totals,
		LocalProfileIDToDir(from_id), MACHINE_PROFILE_DIR);
}

void ProfileManager::ChangeProfileType(int index, ProfileType new_type)
{
	if(index < 0 || static_cast<std::size_t>(index) >= g_vLocalProfile.size())
	{ return; }
	if(new_type == g_vLocalProfile[index].profile.m_Type)
	{ return; }
	DirAndProfile derp= g_vLocalProfile[index];
	g_vLocalProfile.erase(g_vLocalProfile.begin() + index);
	derp.profile.m_Type= new_type;
	InsertProfileIntoList(derp);
}

void ProfileManager::MoveProfileTopBottom(int index, bool top)
{
	if (index < 0 || static_cast<std::size_t>(index) >= g_vLocalProfile.size())
	{
		return;
	}

	int swindex = 0;
	// There may be guest profiles at the top of the list, so we need to skip over them if moving to the top.
	// If we're moving the profile to the bottom we should stop once we find the first test profile.
	for (std::size_t i= 0; i < g_vLocalProfile.size(); ++i)
	{
		ProfileType type= g_vLocalProfile[i].profile.m_Type;
		if (!top)
		{
			if (type == ProfileType_Test)
			{
				break;
			}
		}
		else
		{
			if (type != ProfileType_Guest)
			{
				break;
			}
		}
		swindex++;
	}
	if ((top && index < swindex) || (!top && index > swindex))
	{
		return;
	}

	// Save the profile to move
	DirAndProfile profile = g_vLocalProfile[index];
	// Remove the profile from its current position
	g_vLocalProfile.erase(g_vLocalProfile.begin() + index);
	// Insert the profile at the beginning of the list
	g_vLocalProfile.insert(g_vLocalProfile.begin() + swindex, profile);
}

void ProfileManager::MoveProfileSorted(int index, bool bAscending) {

	if (index < 0 || static_cast<std::size_t>(index) >= g_vLocalProfile.size())
	{
		return;
	}

	int swindex = 0;
	// There may be guest profiles at the top of the list, so we need to skip over them.
	for (std::size_t i= 0; i < g_vLocalProfile.size(); ++i)
	{
		ProfileType type= g_vLocalProfile[i].profile.m_Type;
		if (type != ProfileType_Guest)
		{
			break;
		}
		swindex++;
	}

	if (index < swindex)
	{
		return;
	}

	// Copy the profile to be moved
	DirAndProfile temp = g_vLocalProfile[index];

	// Remove the profile from the list
	g_vLocalProfile.erase(g_vLocalProfile.begin()+index);

	// Find the correct location to insert the profile
	auto it = std::lower_bound(g_vLocalProfile.begin()+swindex, g_vLocalProfile.end(), temp,
		[bAscending](const DirAndProfile &a, const DirAndProfile &b)
		{
			if (bAscending)
				return a.profile.m_sDisplayName < b.profile.m_sDisplayName;
			else
				return a.profile.m_sDisplayName > b.profile.m_sDisplayName;
		});
	g_vLocalProfile.insert(it, temp);
}

void ProfileManager::MoveProfilePriority(int index, bool up)
{
	if(index < 0 || static_cast<std::size_t>(index) >= g_vLocalProfile.size())
	{ return; }
	// Changing the priority is complicated a bit because the profiles might
	// all have the same priority.  So this function has to assign priorities
	// to all the profiles of the same type.
	// bools are numbers, true evaluatues to 1.
	int swindex= index + ((up * -2) + 1);
	ProfileType type= g_vLocalProfile[index].profile.m_Type;
	int priority= 0;
	for(std::size_t i= 0; i < g_vLocalProfile.size(); ++i)
	{
		DirAndProfile* curr= &g_vLocalProfile[i];
		if(curr->profile.m_Type == type)
		{
			if(curr->profile.m_ListPriority != priority)
			{
				curr->profile.m_ListPriority= priority;
				if(i != static_cast<std::size_t>(index) && i != static_cast<std::size_t>(swindex))
				{
					curr->profile.SaveTypeToDir(curr->sDir);
				}
			}
			++priority;
		}
		else if(curr->profile.m_Type > type)
		{
			break;
		}
	}
	// Only swap if both indices are valid and the types match.
	if(swindex >= 0 && static_cast<std::size_t>(swindex) < g_vLocalProfile.size() &&
		g_vLocalProfile[swindex].profile.m_Type ==
		g_vLocalProfile[index].profile.m_Type)
	{
		g_vLocalProfile[index].swap(g_vLocalProfile[swindex]);
	}
}

//
// General
//
void ProfileManager::IncrementToastiesCount( PlayerNumber pn )
{
	if( IsPersistentProfile(pn) )
		++GetProfile(pn)->m_iNumToasties;
	++GetMachineProfile()->m_iNumToasties;
}

void ProfileManager::AddStepTotals( PlayerNumber pn, int iNumTapsAndHolds, int iNumJumps, int iNumHolds, int iNumRolls, int iNumMines, int iNumHands, int iNumLifts, float fCaloriesBurned )
{
	if( IsPersistentProfile(pn) )
		GetProfile(pn)->AddStepTotals( iNumTapsAndHolds, iNumJumps, iNumHolds, iNumRolls, iNumMines, iNumHands, iNumLifts, fCaloriesBurned );
	GetMachineProfile()->AddStepTotals( iNumTapsAndHolds, iNumJumps, iNumHolds, iNumRolls, iNumMines, iNumHands, iNumLifts, fCaloriesBurned );
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
	hs.SetPercentDP( std::max(0.0f, hs.GetPercentDP()) ); // bump up negative scores

	iPersonalIndexOut = -1;
	iMachineIndexOut = -1;

	// In event mode, set the score's name immediately to the Profile's last
	// used name.  If no profile last used name exists, use "EVNT".
	if( GAMESTATE->IsEventMode() )
	{
		Profile* pProfile = GetProfile(pn);
		if( pProfile && !pProfile->m_sLastUsedHighScoreName.empty() )
			hs.SetName( pProfile->m_sLastUsedHighScoreName );
		else
			hs.SetName( "EVNT" );
	}
	else
	{
		hs.SetName( RANKING_TO_FILL_IN_MARKER[pn] );
	}

	//
	// save high score
	//
	if( IsPersistentProfile(pn) )
		GetProfile(pn)->AddStepsHighScore( pSong, pSteps, hs, iPersonalIndexOut );

	// don't save machine scores for a failed song
	if( hs.GetPercentDP() >= PREFSMAN->m_fMinPercentageForMachineSongHighScore &&
		hs.GetGrade() != Grade_Failed )
	{
		// don't leave machine high scores for edits loaded from the player's card
		if( !pSteps->IsAPlayerEdit() )
			GetMachineProfile()->AddStepsHighScore( pSong, pSteps, hs, iMachineIndexOut );
	}

	/*
	// save recent score
	if( IsPersistentProfile(pn) )
		GetProfile(pn)->SaveStepsRecentScore( pSong, pSteps, hs );
	GetMachineProfile()->SaveStepsRecentScore( pSong, pSteps, hs );
	*/
}

void ProfileManager::IncrementStepsPlayCount( const Song* pSong, const Steps* pSteps, PlayerNumber pn )
{
	if( IsPersistentProfile(pn) )
		GetProfile(pn)->IncrementStepsPlayCount( pSong, pSteps );
	GetMachineProfile()->IncrementStepsPlayCount( pSong, pSteps );
}

// Course stats
void ProfileManager::AddCourseScore( const Course* pCourse, const Trail* pTrail, PlayerNumber pn, const HighScore &hs_, int &iPersonalIndexOut, int &iMachineIndexOut )
{
	HighScore hs = hs_;
	hs.SetPercentDP(std::max( 0.0f, hs.GetPercentDP()) ); // bump up negative scores

	iPersonalIndexOut = -1;
	iMachineIndexOut = -1;

	// In event mode, set the score's name immediately to the Profile's last
	// used name. If no profile last used name exists, use "EVNT".
	if( GAMESTATE->IsEventMode() )
	{
		Profile* pProfile = GetProfile(pn);
		if( pProfile && !pProfile->m_sLastUsedHighScoreName.empty() )
			hs.SetName(  pProfile->m_sLastUsedHighScoreName );
		else
			hs.SetName( "EVNT" );
	}
	else
	{
		hs.SetName( RANKING_TO_FILL_IN_MARKER[pn] );
	}

	// save high score
	if( IsPersistentProfile(pn) )
		GetProfile(pn)->AddCourseHighScore( pCourse, pTrail, hs, iPersonalIndexOut );
	if( hs.GetPercentDP() >= PREFSMAN->m_fMinPercentageForMachineCourseHighScore )
		GetMachineProfile()->AddCourseHighScore( pCourse, pTrail, hs, iMachineIndexOut );

	/*
	// save recent score
	if( IsPersistentProfile(pn) )
		GetProfile(pn)->SaveCourseRecentScore( pCourse, pTrail, hs );
	GetMachineProfile()->SaveCourseRecentScore( pCourse, pTrail, hs );
	*/
}

void ProfileManager::IncrementCoursePlayCount( const Course* pCourse, const Trail* pTrail, PlayerNumber pn )
{
	if( IsPersistentProfile(pn) )
		GetProfile(pn)->IncrementCoursePlayCount( pCourse, pTrail );
	GetMachineProfile()->IncrementCoursePlayCount( pCourse, pTrail );
}

// Category stats
void ProfileManager::AddCategoryScore( StepsType st, RankingCategory rc, PlayerNumber pn, const HighScore &hs_, int &iPersonalIndexOut, int &iMachineIndexOut )
{
	HighScore hs = hs_;
	hs.SetName( RANKING_TO_FILL_IN_MARKER[pn] );
	if( IsPersistentProfile(pn) )
		GetProfile(pn)->AddCategoryHighScore( st, rc, hs, iPersonalIndexOut );
	if( hs.GetPercentDP() > PREFSMAN->m_fMinPercentageForMachineSongHighScore )
		GetMachineProfile()->AddCategoryHighScore( st, rc, hs, iMachineIndexOut );
}

void ProfileManager::IncrementCategoryPlayCount( StepsType st, RankingCategory rc, PlayerNumber pn )
{
	if( IsPersistentProfile(pn) )
		GetProfile(pn)->IncrementCategoryPlayCount( st, rc );
	GetMachineProfile()->IncrementCategoryPlayCount( st, rc );
}

bool ProfileManager::IsPersistentProfile( ProfileSlot slot ) const
{
	switch( slot )
	{
	case ProfileSlot_Player1:
	case ProfileSlot_Player2:
		return GAMESTATE->IsHumanPlayer((PlayerNumber)slot) && !m_sProfileDir[slot].empty();
	case ProfileSlot_Machine:
		return true;
	default:
		FAIL_M("Invalid profile slot chosen: unable to get profile info!");
	}
}

void ProfileManager::GetLocalProfileIDs( std::vector<RString> &vsProfileIDsOut ) const
{
	vsProfileIDsOut.clear();
	for (DirAndProfile const &i : g_vLocalProfile)
	{
		RString sID = LocalProfileDirToID( i.sDir );
		vsProfileIDsOut.push_back( sID );
	}
}

void ProfileManager::GetLocalProfileDisplayNames( std::vector<RString> &vsProfileDisplayNamesOut ) const
{
	vsProfileDisplayNamesOut.clear();
	for (DirAndProfile const &i : g_vLocalProfile)
		vsProfileDisplayNamesOut.push_back( i.profile.m_sDisplayName );
}

int ProfileManager::GetLocalProfileIndexFromID( RString sProfileID ) const
{
	RString sDir = LocalProfileIDToDir( sProfileID );
	int j = 0;
	for (DirAndProfile const &i : g_vLocalProfile)
	{
		if( i.sDir == sDir )
			return j;
		++j;
	}
	return -1;
}

RString ProfileManager::GetLocalProfileIDFromIndex( int iIndex )
{
	RString sID = LocalProfileDirToID( g_vLocalProfile[iIndex].sDir );
	return sID;
}

Profile *ProfileManager::GetLocalProfileFromIndex( int iIndex )
{
	return &g_vLocalProfile[iIndex].profile;
}

int ProfileManager::GetNumLocalProfiles() const
{
	return g_vLocalProfile.size();
}

void ProfileManager::SetStatsPrefix(RString const& prefix)
{
	m_stats_prefix= prefix;
	for(std::size_t i= 0; i < g_vLocalProfile.size(); ++i)
	{
		g_vLocalProfile[i].profile.HandleStatsPrefixChange(g_vLocalProfile[i].sDir, PREFSMAN->m_bSignProfileData);
	}
	FOREACH_PlayerNumber(pn)
	{
		if(ProfileWasLoadedFromMemoryCard(pn))
		{
			// This probably runs into a problem if the memory card has been removed. -Kyz
			GetProfile(pn)->HandleStatsPrefixChange(m_sProfileDir[pn], PREFSMAN->m_bSignProfileData);
		}
	}
	m_pMachineProfile->HandleStatsPrefixChange(MACHINE_PROFILE_DIR, false);
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ProfileManager. */
class LunaProfileManager: public Luna<ProfileManager>
{
public:
	static int GetStatsPrefix(T* p, lua_State* L)
	{
		lua_pushstring(L, p->GetStatsPrefix().c_str());
		return 1;
	}
	static int SetStatsPrefix(T* p, lua_State* L)
	{
		RString prefix= SArg(1);
		p->SetStatsPrefix(prefix);
		COMMON_RETURN_SELF;
	}
	static int IsPersistentProfile( T* p, lua_State *L )	{ lua_pushboolean(L, p->IsPersistentProfile(Enum::Check<PlayerNumber>(L, 1)) ); return 1; }
	static int GetProfile( T* p, lua_State *L )				{ PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1); Profile* pP = p->GetProfile(pn); ASSERT(pP != nullptr); pP->PushSelf(L); return 1; }
	static int GetMachineProfile( T* p, lua_State *L )		{ p->GetMachineProfile()->PushSelf(L); return 1; }
	static int SaveMachineProfile( T* p, lua_State * )		{ p->SaveMachineProfile(); return 0; }
	static int GetLocalProfile( T* p, lua_State *L )
	{
		Profile *pProfile = p->GetLocalProfile(SArg(1));
		if( pProfile )
			pProfile->PushSelf(L);
		else
			lua_pushnil(L);
		return 1;
	}
	static int GetLocalProfileFromIndex( T* p, lua_State *L )
	{
		int index= IArg(1);
		if(index >= p->GetNumLocalProfiles())
		{
			luaL_error(L, "Profile index %d out of range.", index);
		}
		Profile *pProfile = p->GetLocalProfileFromIndex(index);
		if(pProfile == nullptr)
		{
			luaL_error(L, "No profile at index %d.", index);
		}
		pProfile->PushSelf(L);
		return 1;
	}
	static int GetLocalProfileIDFromIndex( T* p, lua_State *L )
	{
		int index= IArg(1);
		if(index >= p->GetNumLocalProfiles())
		{
			luaL_error(L, "Profile index %d out of range.", index);
		}
		lua_pushstring(L, p->GetLocalProfileIDFromIndex(index) );
		return 1;
	}
	static int GetLocalProfileIndexFromID( T* p, lua_State *L )	{ lua_pushnumber(L, p->GetLocalProfileIndexFromID(SArg(1)) ); return 1; }
	static int GetNumLocalProfiles( T* p, lua_State *L )	{ lua_pushnumber(L, p->GetNumLocalProfiles() ); return 1; }
	static int GetProfileDir( T* p, lua_State *L ) { lua_pushstring(L, p->GetProfileDir(Enum::Check<ProfileSlot>(L, 1)) ); return 1; }
	static int IsSongNew( T* p, lua_State *L )	{ lua_pushboolean(L, p->IsSongNew(Luna<Song>::check(L,1)) ); return 1; }
	static int ProfileWasLoadedFromMemoryCard( T* p, lua_State *L )	{ lua_pushboolean(L, p->ProfileWasLoadedFromMemoryCard(Enum::Check<PlayerNumber>(L, 1)) ); return 1; }
	static int LastLoadWasTamperedOrCorrupt( T* p, lua_State *L ) { lua_pushboolean(L, p->LastLoadWasTamperedOrCorrupt(Enum::Check<PlayerNumber>(L, 1)) ); return 1; }
	static int GetPlayerName( T* p, lua_State *L )				{ PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1); lua_pushstring(L, p->GetPlayerName(pn)); return 1; }

	static int LocalProfileIDToDir( T* , lua_State *L )
	{
		RString dir = USER_PROFILES_DIR + SArg(1) + "/";
		lua_pushstring( L, dir );
		return 1;
	}
	static int SaveProfile( T* p, lua_State *L ) { lua_pushboolean( L, p->SaveProfile(Enum::Check<PlayerNumber>(L, 1)) ); return 1; }
	static int SaveLocalProfile( T* p, lua_State *L ) { lua_pushboolean( L, p->SaveLocalProfile(SArg(1)) ); return 1; }
	static int ProfileFromMemoryCardIsNew( T* p, lua_State *L ) { lua_pushboolean( L, p->ProfileFromMemoryCardIsNew(Enum::Check<PlayerNumber>(L, 1)) ); return 1; }
	static int GetSongNumTimesPlayed( T* p, lua_State *L )
	{
		lua_pushnumber(L, p->GetSongNumTimesPlayed(Luna<Song>::check(L,1),Enum::Check<ProfileSlot>(L, 2)) );
		return 1;
	}
	static int GetLocalProfileIDs( T* p, lua_State *L )
	{
		std::vector<RString> vsProfileIDs;
		p->GetLocalProfileIDs(vsProfileIDs);
		LuaHelpers::CreateTableFromArray<RString>( vsProfileIDs, L );
		return 1;
	}
	static int GetLocalProfileDisplayNames( T* p, lua_State *L )
	{
		std::vector<RString> vsProfileNames;
		p->GetLocalProfileDisplayNames(vsProfileNames);
		LuaHelpers::CreateTableFromArray<RString>( vsProfileNames, L );
		return 1;
	}

	LunaProfileManager()
	{
		ADD_METHOD(GetStatsPrefix);
		ADD_METHOD(SetStatsPrefix);
		ADD_METHOD( IsPersistentProfile );
		ADD_METHOD( GetProfile );
		ADD_METHOD( GetMachineProfile );
		ADD_METHOD( SaveMachineProfile );
		ADD_METHOD( GetLocalProfile );
		ADD_METHOD( GetLocalProfileFromIndex );
		ADD_METHOD( GetLocalProfileIDFromIndex );
		ADD_METHOD( GetLocalProfileIndexFromID );
		ADD_METHOD( GetNumLocalProfiles );
		ADD_METHOD( GetProfileDir );
		ADD_METHOD( IsSongNew );
		ADD_METHOD( ProfileWasLoadedFromMemoryCard );
		ADD_METHOD( LastLoadWasTamperedOrCorrupt );
		ADD_METHOD( GetPlayerName );
		//
		ADD_METHOD( SaveProfile );
		ADD_METHOD( SaveLocalProfile );
		ADD_METHOD( ProfileFromMemoryCardIsNew );
		ADD_METHOD( GetSongNumTimesPlayed );
		ADD_METHOD( GetLocalProfileIDs );
		ADD_METHOD( GetLocalProfileDisplayNames );
		ADD_METHOD( LocalProfileIDToDir );
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
