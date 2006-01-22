#include "global.h"
#include "ScreenServiceAction.h"
#include "ThemeManager.h"
#include "Bookkeeper.h"
#include "Profile.h"
#include "ProfileManager.h"
#include "ScreenManager.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "song.h"
#include "MemoryCardManager.h"
#include "GameState.h"
#include "PlayerState.h"
#include "LocalizedString.h"

static LocalizedString BOOKKEEPING_DATA_CLEARED( "ScreenServiceAction", "Bookkeeping data cleared." );
static RString ClearBookkeepingData()
{
	BOOKKEEPER->ClearAll();
	BOOKKEEPER->WriteToDisk();
	return BOOKKEEPING_DATA_CLEARED.GetValue();
}

static LocalizedString MACHINE_STATS_CLEARED( "ScreenServiceAction", "Machine stats cleared." );
static RString ClearMachineStats()
{
	Profile* pProfile = PROFILEMAN->GetMachineProfile();
	// don't reset the Guid
	RString sGuid = pProfile->m_sGuid;
	pProfile->InitAll();
	pProfile->m_sGuid = sGuid;
	PROFILEMAN->SaveMachineProfile();
	return MACHINE_STATS_CLEARED.GetValue();
}

static LocalizedString MACHINE_EDITS_CLEARED( "ScreenServiceAction", "%d edits cleared, %d errors." );
static RString ClearMachineEdits()
{
	int iNumAttempted = 0;
	int iNumSuccessful = 0;
	
	vector<RString> vsEditFiles;
	GetDirListing( PROFILEMAN->GetProfileDir(ProfileSlot_Machine)+EDIT_STEPS_SUBDIR+"*.edit", vsEditFiles, false, true );
	GetDirListing( PROFILEMAN->GetProfileDir(ProfileSlot_Machine)+EDIT_COURSES_SUBDIR+"*.crs", vsEditFiles, false, true );
	FOREACH_CONST( RString, vsEditFiles, i )
	{
		iNumAttempted++;
		bool bSuccess = FILEMAN->Remove( *i );
		if( bSuccess )
			iNumSuccessful++;
	}

	// reload the machine profile
	PROFILEMAN->SaveMachineProfile();
	PROFILEMAN->LoadMachineProfile();
	
	int iNumErrors = iNumAttempted-iNumSuccessful;
	return ssprintf(MACHINE_EDITS_CLEARED.GetValue(),iNumSuccessful,iNumErrors);
}

static PlayerNumber GetFirstReadyMemoryCard()
{
	FOREACH_PlayerNumber( pn )
	{
		if( MEMCARDMAN->GetCardState(pn) != MemoryCardState_Ready )
			continue;	// skip

		if( !MEMCARDMAN->IsMounted(pn) )
			MEMCARDMAN->MountCard(pn);
		return pn;
	}

	return PLAYER_INVALID;
}

static LocalizedString MEMORY_CARD_EDITS_NOT_CLEARED( "ScreenServiceAction", "Edits not cleared - No memory cards ready." );
static LocalizedString EDITS_CLEARED				( "ScreenServiceAction", "%d edits cleared, %d errors." );
static RString ClearMemoryCardEdits()
{
	PlayerNumber pn = GetFirstReadyMemoryCard();
	if( pn == PLAYER_INVALID )
		return MEMORY_CARD_EDITS_NOT_CLEARED.GetValue();

	int iNumAttempted = 0;
	int iNumSuccessful = 0;
	
	if( !MEMCARDMAN->IsMounted(pn) )
		MEMCARDMAN->MountCard(pn);

	RString sDir = MEM_CARD_MOUNT_POINT[pn] + (RString)PREFSMAN->m_sMemoryCardProfileSubdir + "/";
	vector<RString> vsEditFiles;
	GetDirListing( sDir+EDIT_STEPS_SUBDIR+"*.edit", vsEditFiles, false, true );
	GetDirListing( sDir+EDIT_COURSES_SUBDIR+"*.crs", vsEditFiles, false, true );
	FOREACH_CONST( RString, vsEditFiles, i )
	{
		iNumAttempted++;
		bool bSuccess = FILEMAN->Remove( *i );
		if( bSuccess )
			iNumSuccessful++;
	}

	MEMCARDMAN->UnmountCard(pn);

	return ssprintf(EDITS_CLEARED.GetValue(),iNumSuccessful,iNumAttempted-iNumSuccessful);
}


static LocalizedString STATS_NOT_SAVED				( "ScreenServiceAction", "Stats not saved - No memory cards ready." );
static LocalizedString MACHINE_STATS_SAVED			( "ScreenServiceAction", "Machine stats saved to P%d card." );
static LocalizedString ERROR_SAVING_MACHINE_STATS	( "ScreenServiceAction", "Error saving machine stats to P%d card." );
static RString TransferStatsMachineToMemoryCard()
{
	PlayerNumber pn = GetFirstReadyMemoryCard();
	if( pn == PLAYER_INVALID )
		return STATS_NOT_SAVED.GetValue();

	if( !MEMCARDMAN->IsMounted(pn) )
		MEMCARDMAN->MountCard(pn);

	RString sDir = MEM_CARD_MOUNT_POINT[pn];
	sDir += "MachineProfile/";

	bool bSaved = PROFILEMAN->GetMachineProfile()->SaveAllToDir( sDir, PREFSMAN->m_bSignProfileData );

	MEMCARDMAN->UnmountCard(pn);

	if( bSaved )
		return ssprintf(MACHINE_STATS_SAVED.GetValue(),pn+1);
	else
		return ssprintf(ERROR_SAVING_MACHINE_STATS.GetValue(),pn+1);
}

static LocalizedString STATS_NOT_LOADED		( "ScreenServiceAction", "Stats not loaded - No memory cards ready." );
static LocalizedString MACHINE_STATS_LOADED	( "ScreenServiceAction", "Machine stats loaded from P%d card." );
static LocalizedString THERE_IS_NO_PROFILE	( "ScreenServiceAction", "There is no machine profile on P%d card." );
static LocalizedString PROFILE_CORRUPT		( "ScreenServiceAction", "The profile on P%d card contains corrupt or tampered data." );
static RString TransferStatsMemoryCardToMachine()
{
	PlayerNumber pn = GetFirstReadyMemoryCard();
	if( pn == PLAYER_INVALID )
		return STATS_NOT_LOADED.GetValue();

	if( !MEMCARDMAN->IsMounted(pn) )
		MEMCARDMAN->MountCard(pn);

	RString sDir = MEM_CARD_MOUNT_POINT[pn];
	sDir += "MachineProfile/";

	Profile backup = *PROFILEMAN->GetMachineProfile();

	ProfileLoadResult lr = PROFILEMAN->GetMachineProfile()->LoadAllFromDir( sDir, PREFSMAN->m_bSignProfileData );
	RString s;
	switch( lr )
	{
	case ProfileLoadResult_Success:
		s = ssprintf(MACHINE_STATS_LOADED.GetValue(),pn+1);
		break;
	case ProfileLoadResult_FailedNoProfile:
		*PROFILEMAN->GetMachineProfile() = backup;
		s = ssprintf(THERE_IS_NO_PROFILE.GetValue(),pn+1);
		break;
	case ProfileLoadResult_FailedTampered:
		*PROFILEMAN->GetMachineProfile() = backup;
		s = ssprintf(PROFILE_CORRUPT.GetValue(),pn+1);
		break;
	default:
		ASSERT(0);
	}

	MEMCARDMAN->UnmountCard(pn);

	return s;
}

static void CopyEdits( const RString &sFrom, const RString &sTo, int &iNumAttempted, int &iNumSuccessful, int &iNumOverwritten )
{
	iNumAttempted = 0;
	iNumSuccessful = 0;
	iNumOverwritten = 0;

	{
		RString sFromDir = sFrom + EDIT_STEPS_SUBDIR;
		RString sToDir = sTo + EDIT_STEPS_SUBDIR;

		vector<RString> vsFiles;
		GetDirListing( sFromDir+"*.edit", vsFiles, false, false );
		FOREACH_CONST( RString, vsFiles, i )
		{
			iNumAttempted++;
			if( DoesFileExist(sToDir+*i) )
				iNumOverwritten++;
			bool bSuccess = FileCopy( sFromDir+*i, sToDir+*i );
			if( bSuccess )
				iNumSuccessful++;
		}
	}

	{
		RString sFromDir = sFrom + EDIT_COURSES_SUBDIR;
		RString sToDir = sTo + EDIT_COURSES_SUBDIR;

		vector<RString> vsFiles;
		GetDirListing( sFromDir+"*.crs", vsFiles, false, false );
		FOREACH_CONST( RString, vsFiles, i )
		{
			iNumAttempted++;
			if( DoesFileExist(sToDir+*i) )
				iNumOverwritten++;
			bool bSuccess = FileCopy( sFromDir+*i, sToDir+*i );
			if( bSuccess )
				iNumSuccessful++;
		}
	}
}

static void SyncFiles( const RString &sFromDir, const RString &sToDir, const RString &sMask, int &iNumAdded, int &iNumDeleted, int &iNumOverwritten, int &iNumFailed )
{
	vector<RString> vsFilesSource;
	GetDirListing( sFromDir+sMask, vsFilesSource, false, false );

	vector<RString> vsFilesDest;
	GetDirListing( sToDir+sMask, vsFilesDest, false, false );

	vector<RString> vsToDelete;
	GetAsNotInBs( vsFilesDest, vsFilesSource, vsToDelete );

	for( unsigned i = 0; i < vsToDelete.size(); ++i )
	{
		RString sFile = sToDir + vsToDelete[i];
		LOG->Trace( "Delete \"%s\"", sFile.c_str() );

		if( FILEMAN->Remove(sFile) )
			++iNumDeleted;
		else
			++iNumFailed;
	}

	for( unsigned i = 0; i < vsFilesSource.size(); ++i )
	{
		RString sFileFrom = sFromDir + vsFilesSource[i];
		RString sFileTo = sToDir + vsFilesSource[i];
		LOG->Trace( "Copy \"%s\"", sFileFrom.c_str() );
		bool bOverwrite = DoesFileExist( sFileTo );
		bool bSuccess = FileCopy( sFileFrom, sFileTo );
		if( bSuccess )
		{
			if( bOverwrite )
				++iNumOverwritten;
			else
				++iNumAdded;
		}
		else
			++iNumFailed;
	}
}

static void SyncEdits( const RString &sFromDir, const RString &sToDir, int &iNumAdded, int &iNumDeleted, int &iNumOverwritten, int &iNumFailed )
{
	iNumAdded = 0;
	iNumDeleted = 0;
	iNumOverwritten = 0;
	iNumFailed = 0;

	SyncFiles( sFromDir + EDIT_STEPS_SUBDIR, sToDir + EDIT_STEPS_SUBDIR, "*.edit", iNumAdded, iNumDeleted, iNumOverwritten, iNumFailed );
	SyncFiles( sFromDir + EDIT_COURSES_SUBDIR, sToDir + EDIT_COURSES_SUBDIR, "*.crs", iNumAdded, iNumDeleted, iNumOverwritten, iNumFailed );
}

static LocalizedString EDITS_NOT_COPIED( "ScreenServiceAction", "Edits not copied - No memory cards ready." );
static LocalizedString COPIED_TO_CARD( "ScreenServiceAction", "Copied to P%d card:" );
static LocalizedString COPIED_AND_OVERWRITTEN( "ScreenServiceAction", "%d copied, %d overwritten" );
static LocalizedString ADDED_AND_OVERWRITTEN( "ScreenServiceAction", "%d added, %d overwritten" );
static LocalizedString FAILED( "ScreenServiceAction", "%d failed" );
static LocalizedString DELETED( "ScreenServiceAction", "%d deleted" );
static RString CopyEditsMachineToMemoryCard()
{
	PlayerNumber pn = GetFirstReadyMemoryCard();
	if( pn == PLAYER_INVALID )
		return EDITS_NOT_COPIED.GetValue();

	if( !MEMCARDMAN->IsMounted(pn) )
		MEMCARDMAN->MountCard(pn);

	int iNumAttempted = 0;
	int iNumSuccessful = 0;
	int iNumOverwritten = 0;
	RString sFromDir = PROFILEMAN->GetProfileDir(ProfileSlot_Machine);
	RString sToDir = MEM_CARD_MOUNT_POINT[pn] + (RString)PREFSMAN->m_sMemoryCardProfileSubdir + "/";

	CopyEdits( sFromDir, sToDir, iNumAttempted, iNumSuccessful, iNumOverwritten );
	
	MEMCARDMAN->UnmountCard(pn);

	RString sRet = ssprintf( COPIED_TO_CARD.GetValue(), pn+1 ) + " ";
	sRet += ssprintf( COPIED_AND_OVERWRITTEN.GetValue(), iNumSuccessful, iNumOverwritten );
	if( iNumSuccessful < iNumAttempted )
		sRet += RString("; ") + ssprintf( FAILED.GetValue(), iNumAttempted-iNumSuccessful );
	return sRet;
}

static RString SyncEditsMachineToMemoryCard()
{
	PlayerNumber pn = GetFirstReadyMemoryCard();
	if( pn == PLAYER_INVALID )
		return EDITS_NOT_COPIED.GetValue();

	if( !MEMCARDMAN->IsMounted(pn) )
		MEMCARDMAN->MountCard(pn);

	int iNumAdded = 0;
	int iNumDeleted = 0;
	int iNumOverwritten = 0;
	int iNumFailed = 0;

	RString sFromDir = PROFILEMAN->GetProfileDir(ProfileSlot_Machine);
	RString sToDir = MEM_CARD_MOUNT_POINT[pn] + (RString)PREFSMAN->m_sMemoryCardProfileSubdir + "/";
	SyncEdits( sFromDir, sToDir, iNumAdded, iNumDeleted, iNumOverwritten, iNumFailed );
	
	MEMCARDMAN->UnmountCard(pn);

	RString sRet = ssprintf( COPIED_TO_CARD.GetValue(), pn+1 ) + " ";
	sRet += ssprintf( ADDED_AND_OVERWRITTEN.GetValue(), iNumAdded, iNumOverwritten );
	if( iNumDeleted )
		sRet += RString(" ") + ssprintf( DELETED.GetValue(), iNumDeleted );
	if( iNumFailed )
		sRet += RString("; ") + ssprintf( FAILED.GetValue(), iNumFailed );
	return sRet;
}

static LocalizedString COPIED_FROM_CARD( "ScreenServiceAction", "Copied from P%d card:" );
static RString CopyEditsMemoryCardToMachine()
{
	PlayerNumber pn = GetFirstReadyMemoryCard();
	if( pn == PLAYER_INVALID )
		return EDITS_NOT_COPIED.GetValue();

	if( !MEMCARDMAN->IsMounted(pn) )
		MEMCARDMAN->MountCard(pn);

	int iNumAttempted = 0;
	int iNumSuccessful = 0;
	int iNumOverwritten = 0;
	RString sFromDir = MEM_CARD_MOUNT_POINT[pn] + (RString)PREFSMAN->m_sMemoryCardProfileSubdir + "/";
	RString sToDir = PROFILEMAN->GetProfileDir(ProfileSlot_Machine);
	
	CopyEdits( sFromDir, sToDir, iNumAttempted, iNumSuccessful, iNumOverwritten );
	
	MEMCARDMAN->UnmountCard(pn);

	// reload the machine profile
	PROFILEMAN->SaveMachineProfile();
	PROFILEMAN->LoadMachineProfile();

	RString sRet = ssprintf( COPIED_FROM_CARD.GetValue(), pn+1 ) + " ";
	sRet += ssprintf( COPIED_AND_OVERWRITTEN.GetValue(), iNumSuccessful, iNumOverwritten );
	if( iNumSuccessful < iNumAttempted )
		sRet += RString("; ") + ssprintf( FAILED.GetValue(), iNumAttempted-iNumSuccessful );
	return sRet;
}

static LocalizedString PREFERENCES_RESET( "ScreenServiceAction", "Preferences reset." );
static RString ResetPreferences()
{
	PREFSMAN->ResetToFactoryDefaults();
	return PREFERENCES_RESET.GetValue();
}



REGISTER_SCREEN_CLASS( ScreenServiceAction );
void ScreenServiceAction::Init()
{
	ScreenPrompt::Init();

	RString sAction = THEME->GetMetric(m_sName,"Action");

	RString (*pfn)() = NULL;

	if(	 sAction == "ClearBookkeepingData" )		pfn = ClearBookkeepingData;
	else if( sAction == "ClearMachineStats" )			pfn = ClearMachineStats;
	else if( sAction == "ClearMachineEdits" )			pfn = ClearMachineEdits;
	else if( sAction == "ClearMemoryCardEdits" )			pfn = ClearMemoryCardEdits;
	else if( sAction == "TransferStatsMachineToMemoryCard" )	pfn = TransferStatsMachineToMemoryCard;
	else if( sAction == "TransferStatsMemoryCardToMachine" )	pfn = TransferStatsMemoryCardToMachine;
	else if( sAction == "CopyEditsMachineToMemoryCard" )		pfn = CopyEditsMachineToMemoryCard;
	else if( sAction == "CopyEditsMemoryCardToMachine" )		pfn = CopyEditsMemoryCardToMachine;
	else if( sAction == "SyncEditsMachineToMemoryCard" )		pfn = SyncEditsMachineToMemoryCard;
	else if( sAction == "ResetPreferences" )			pfn = ResetPreferences;
	
	ASSERT_M( pfn, sAction );
	
	RString s = pfn();

	ScreenPrompt::SetPromptSettings( s, PROMPT_OK );
}

/*
 * (c) 2001-2005 Chris Danford
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
