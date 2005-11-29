#include "global.h"
#include "ScreenServiceAction.h"
#include "ThemeManager.h"
#include "Bookkeeper.h"
#include "Profile.h"
#include "ProfileManager.h"
#include "ScreenManager.h"
#include "RageFileManager.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "song.h"
#include "MemoryCardManager.h"
#include "GameState.h"
#include "PlayerState.h"


static CString ClearBookkeepingData()
{
	BOOKKEEPER->ClearAll();
	BOOKKEEPER->WriteToDisk();
	return "Bookkeeping data cleared.";
}

static CString ClearMachineStats()
{
	Profile* pProfile = PROFILEMAN->GetMachineProfile();
	// don't reset the Guid
	CString sGuid = pProfile->m_sGuid;
	pProfile->InitAll();
	pProfile->m_sGuid = sGuid;
	PROFILEMAN->SaveMachineProfile();
	return "Machine stats cleared.";
}

static CString ClearMachineEdits()
{
	int iNumAttempted = 0;
	int iNumSuccessful = 0;
	
	vector<CString> vsEditFiles;
	GetDirListing( PROFILEMAN->GetProfileDir(PROFILE_SLOT_MACHINE)+EDIT_STEPS_SUBDIR+"*.edit", vsEditFiles, false, true );
	GetDirListing( PROFILEMAN->GetProfileDir(PROFILE_SLOT_MACHINE)+EDIT_COURSES_SUBDIR+"*.crs", vsEditFiles, false, true );
	FOREACH_CONST( CString, vsEditFiles, i )
	{
		iNumAttempted++;
		bool bSuccess = FILEMAN->Remove( *i );
		if( bSuccess )
			iNumSuccessful++;
	}

	// reload the machine profile
	PROFILEMAN->SaveMachineProfile();
	PROFILEMAN->LoadMachineProfile();
	
	return ssprintf("%d edits cleared, %d errors.",iNumSuccessful,iNumAttempted-iNumSuccessful);
}

static CString ClearMemoryCardEdits()
{
	int iNumAttempted = 0;
	int iNumSuccessful = 0;
	
	bool bTriedToLoad = false;
	FOREACH_PlayerNumber( pn )
	{
		if( MEMCARDMAN->GetCardState(pn) != MEMORY_CARD_STATE_READY )
			continue;	// skip

		MEMCARDMAN->MountCard(pn);

		bTriedToLoad = true;

		CString sDir = MEM_CARD_MOUNT_POINT[pn];

		vector<CString> vsEditFiles;
		GetDirListing( sDir+EDIT_STEPS_SUBDIR+"*.edit", vsEditFiles, false, true );
		GetDirListing( sDir+EDIT_COURSES_SUBDIR+"*.crs", vsEditFiles, false, true );
		FOREACH_CONST( CString, vsEditFiles, i )
		{
			iNumAttempted++;
			bool bSuccess = FILEMAN->Remove( *i );
			if( bSuccess )
				iNumSuccessful++;
		}

		MEMCARDMAN->UnmountCard(pn);
		break;
	}

	if( !bTriedToLoad )
		return "Stats not loaded - No memory cards ready.";

	MEMCARDMAN->FlushAndReset();

	return ssprintf("%d edits cleared, %d errors.",iNumSuccessful,iNumAttempted-iNumSuccessful);
}

static HighScore MakeRandomHighScore( float fPercentDP )
{
	HighScore hs;
	hs.SetName( "FAKE" );
	hs.SetGrade( (Grade)SCALE( rand()%5, 0, 4, Grade_Tier01, Grade_Tier05 ) );
	hs.SetScore( rand()%100*1000 );
	hs.SetPercentDP( fPercentDP );
	hs.SetSurviveSeconds( randomf(30.0f, 100.0f) );
	PlayerOptions po;
	po.ChooseRandomModifiers();
	hs.SetModifiers( po.GetString() );
	hs.SetDateTime( DateTime::GetNowDateTime() );
	hs.SetPlayerGuid( Profile::MakeGuid() );
	hs.SetMachineGuid( Profile::MakeGuid() );
	hs.SetProductID( rand()%10 );
	FOREACH_TapNoteScore( tns )
		hs.SetTapNoteScore( tns, rand() % 100 );
	FOREACH_HoldNoteScore( hns )
		hs.SetHoldNoteScore( hns, rand() % 100 );
	RadarValues rv;
	FOREACH_RadarCategory( rc )
		rv.m_Values.f[rc] = randomf( 0, 1 );
	hs.SetRadarValues( rv );

	return hs;
}

static void FillProfile( Profile *pProfile )
{
	// Choose a percent for all scores.  This is useful for testing unlocks
	// where some elements are unlocked at a certain percent complete
	float fPercentDP = randomf( 0.6f, 1.2f );
	CLAMP( fPercentDP, 0.0f, 1.0f );

	int iCount = pProfile->IsMachine()? 
		PREFSMAN->m_iMaxHighScoresPerListForMachine.Get():
		PREFSMAN->m_iMaxHighScoresPerListForPlayer.Get();

	vector<Song*> vpAllSongs = SONGMAN->GetAllSongs();
	FOREACH( Song*, vpAllSongs, pSong )
	{
		vector<Steps*> vpAllSteps = (*pSong)->GetAllSteps();
		FOREACH( Steps*, vpAllSteps, pSteps )
		{
			pProfile->IncrementStepsPlayCount( *pSong, *pSteps );
			for( int i=0; i<iCount; i++ )
			{
				int iIndex = 0;
				pProfile->AddStepsHighScore( *pSong, *pSteps, MakeRandomHighScore(fPercentDP), iIndex );
			}
		}
	}
	
	vector<Course*> vpAllCourses;
	SONGMAN->GetAllCourses( vpAllCourses, true );
	FOREACH( Course*, vpAllCourses, pCourse )
	{
		vector<Trail*> vpAllTrails;
		(*pCourse)->GetAllTrails( vpAllTrails );
		FOREACH( Trail*, vpAllTrails, pTrail )
		{
			pProfile->IncrementCoursePlayCount( *pCourse, *pTrail );
			for( int i=0; i<iCount; i++ )
			{
				int iIndex = 0;
				pProfile->AddCourseHighScore( *pCourse, *pTrail, MakeRandomHighScore(fPercentDP), iIndex );
			}
		}
	}
}

static CString FillMachineStats()
{
	Profile* pProfile = PROFILEMAN->GetMachineProfile();
	FillProfile( pProfile );

	PROFILEMAN->SaveMachineProfile();
	return "Machine stats filled.";
}

static CString TransferStatsMachineToMemoryCard()
{
	bool bTriedToSave = false;
	FOREACH_PlayerNumber( pn )
	{
		if( MEMCARDMAN->GetCardState(pn) != MEMORY_CARD_STATE_READY )
			continue;	// skip

		MEMCARDMAN->MountCard(pn);

		bTriedToSave = true;

		CString sDir = MEM_CARD_MOUNT_POINT[pn];
		sDir += "MachineProfile/";

		bool bSaved = PROFILEMAN->GetMachineProfile()->SaveAllToDir( sDir, PREFSMAN->m_bSignProfileData );

		MEMCARDMAN->UnmountCard(pn);

		if( bSaved )
			return ssprintf("Machine stats saved to P%d card.",pn+1);
		else
			return ssprintf("Error saving machine stats to P%d card.",pn+1);
		break;
	}

	if( !bTriedToSave )
		return "Stats not saved - No memory cards ready.";

	MEMCARDMAN->FlushAndReset();

	return "Stats transferred to Memory Card";
}

static CString TransferStatsMemoryCardToMachine()
{
	bool bTriedToLoad = false;
	FOREACH_PlayerNumber( pn )
	{
		if( MEMCARDMAN->GetCardState(pn) != MEMORY_CARD_STATE_READY )
			continue;	// skip

		MEMCARDMAN->MountCard(pn);

		bTriedToLoad = true;

		CString sDir = MEM_CARD_MOUNT_POINT[pn];
		sDir += "MachineProfile/";

		Profile backup = *PROFILEMAN->GetMachineProfile();

		ProfileLoadResult lr = PROFILEMAN->GetMachineProfile()->LoadAllFromDir( sDir, PREFSMAN->m_bSignProfileData );
		switch( lr )
		{
		case ProfileLoadResult_Success:
			return ssprintf("Machine stats loaded from P%d card.",pn+1);
			break;
		case ProfileLoadResult_FailedNoProfile:
			return ssprintf("There is no machine profile on P%d card.",pn+1);
			*PROFILEMAN->GetMachineProfile() = backup;
			break;
		case ProfileLoadResult_FailedTampered:
			return ssprintf("The profile on P%d card contains corrupt or tampered data.",pn+1);
			*PROFILEMAN->GetMachineProfile() = backup;
			break;
		default:
			ASSERT(0);
		}

		MEMCARDMAN->UnmountCard(pn);
		break;
	}

	if( !bTriedToLoad )
		return "Stats not loaded - No memory cards ready.";

	MEMCARDMAN->FlushAndReset();

	return "Stats transferred to machine.";
}

static CString CopyEditsMachineToMemoryCard()
{
	bool bTriedToCopy = false;
	FOREACH_PlayerNumber( pn )
	{
		if( MEMCARDMAN->GetCardState(pn) != MEMORY_CARD_STATE_READY )
			continue;	// skip

		MEMCARDMAN->MountCard(pn);

		bTriedToCopy = true;

		int iNumAttempted = 0;
		int iNumSuccessful = 0;
		int iNumOverwritten = 0;
		
		{
			CString sFromDir = PROFILEMAN->GetProfileDir(PROFILE_SLOT_MACHINE) + EDIT_STEPS_SUBDIR;
			CString sToDir = MEM_CARD_MOUNT_POINT[pn] + (CString)PREFSMAN->m_sMemoryCardProfileSubdir + "/" + EDIT_STEPS_SUBDIR;

			vector<CString> vsFiles;
			GetDirListing( sFromDir+"*.edit", vsFiles, false, false );
			FOREACH_CONST( CString, vsFiles, i )
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
			CString sFromDir = PROFILEMAN->GetProfileDir(PROFILE_SLOT_MACHINE) + EDIT_COURSES_SUBDIR;
			CString sToDir = MEM_CARD_MOUNT_POINT[pn] + (CString)PREFSMAN->m_sMemoryCardProfileSubdir + "/" + EDIT_COURSES_SUBDIR;

			vector<CString> vsFiles;
			GetDirListing( sFromDir+"*.crs", vsFiles, false, false );
			FOREACH_CONST( CString, vsFiles, i )
			{
				iNumAttempted++;
				if( DoesFileExist(sToDir+*i) )
					iNumOverwritten++;
				bool bSuccess = FileCopy( sFromDir+*i, sToDir+*i );
				if( bSuccess )
					iNumSuccessful++;
			}
		}
		
		MEMCARDMAN->UnmountCard(pn);

		return ssprintf("Copied to P%d card: %d/%d copies OK (%d overwritten).",pn+1,iNumSuccessful,iNumAttempted,iNumOverwritten);
		break;
	}

	if( !bTriedToCopy )
		return "Edits not copied - No memory cards ready.";

	MEMCARDMAN->FlushAndReset();

	return "Edits copied to machine.";
}

static CString CopyEditsMemoryCardToMachine()
{
	bool bTriedToCopy = false;
	FOREACH_PlayerNumber( pn )
	{
		if( MEMCARDMAN->GetCardState(pn) != MEMORY_CARD_STATE_READY )
			continue;	// skip

		MEMCARDMAN->MountCard(pn);

		bTriedToCopy = true;

		int iNumAttempted = 0;
		int iNumSuccessful = 0;
		int iNumOverwritten = 0;
		
		{
			CString sFromDir = MEM_CARD_MOUNT_POINT[pn] + (CString)PREFSMAN->m_sMemoryCardProfileSubdir + "/" + EDIT_STEPS_SUBDIR;
			CString sToDir = PROFILEMAN->GetProfileDir(PROFILE_SLOT_MACHINE) + EDIT_STEPS_SUBDIR;

			vector<CString> vsFiles;
			GetDirListing( sFromDir+"*.edit", vsFiles, false, false );
			FOREACH_CONST( CString, vsFiles, i )
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
			CString sFromDir = MEM_CARD_MOUNT_POINT[pn] + (CString)PREFSMAN->m_sMemoryCardProfileSubdir + "/" + EDIT_COURSES_SUBDIR;
			CString sToDir = PROFILEMAN->GetProfileDir(PROFILE_SLOT_MACHINE) + EDIT_COURSES_SUBDIR;

			vector<CString> vsFiles;
			GetDirListing( sFromDir+"*.crs", vsFiles, false, false );
			FOREACH_CONST( CString, vsFiles, i )
			{
				iNumAttempted++;
				if( DoesFileExist(sToDir+*i) )
					iNumOverwritten++;
				bool bSuccess = FileCopy( sFromDir+*i, sToDir+*i );
				if( bSuccess )
					iNumSuccessful++;
			}
		}
		
		MEMCARDMAN->UnmountCard(pn);

		// reload the machine profile
		PROFILEMAN->SaveMachineProfile();
		PROFILEMAN->LoadMachineProfile();

		return ssprintf("Copied from P%d card: %d/%d copies OK (%d overwritten).",pn+1,iNumSuccessful,iNumAttempted,iNumOverwritten);
		break;
	}

	if( !bTriedToCopy )
		return "Edits not copied - No memory cards ready.";

	MEMCARDMAN->FlushAndReset();

	return "Edits copied to memory card.";
}

static CString ResetPreferences()
{
	PREFSMAN->ResetToFactoryDefaults();
	return "Preferences reset.";
}



REGISTER_SCREEN_CLASS( ScreenServiceAction );
ScreenServiceAction::ScreenServiceAction( CString sClassName ) : ScreenPrompt( sClassName )
{
	CString sAction = THEME->GetMetric(m_sName,"Action");

	CString (*pfn)() = NULL;

	if(		 sAction == "ClearBookkeepingData" )				pfn = ClearBookkeepingData;
	else if( sAction == "ClearMachineStats" )					pfn = ClearMachineStats;
	else if( sAction == "ClearMachineEdits" )					pfn = ClearMachineEdits;
	else if( sAction == "ClearMemoryCardEdits" )				pfn = ClearMemoryCardEdits;
	else if( sAction == "FillMachineStats" )					pfn = FillMachineStats;
	else if( sAction == "TransferStatsMachineToMemoryCard" )	pfn = TransferStatsMachineToMemoryCard;
	else if( sAction == "TransferStatsMemoryCardToMachine" )	pfn = TransferStatsMemoryCardToMachine;
	else if( sAction == "CopyEditsMachineToMemoryCard" )		pfn = CopyEditsMachineToMemoryCard;
	else if( sAction == "CopyEditsMemoryCardToMachine" )		pfn = CopyEditsMemoryCardToMachine;
	else if( sAction == "ResetPreferences" )					pfn = ResetPreferences;
	
	ASSERT_M( pfn, sAction );
	
	CString s = pfn();

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
