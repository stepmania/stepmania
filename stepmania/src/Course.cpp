#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: Course

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Course.h"
#include "PrefsManager.h"
#include "Song.h"
#include "GameManager.h"
#include "SongManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageLog.h"
#include "MsdFile.h"


void Course::LoadFromCRSFile( CString sPath, CArray<Song*,Song*> &apSongs )
{
	MsdFile msd;
	if( !msd.ReadFile(sPath) )
		throw RageException( "Error opening CRS file '%s'.", sPath );

	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt );

	CStringArray arrayPossibleBanners;
	GetDirListing( "Courses\\" + sFName + ".png", arrayPossibleBanners, false, true );
	GetDirListing( "Courses\\" + sFName + ".jpg", arrayPossibleBanners, false, true );
	GetDirListing( "Courses\\" + sFName + ".bmp", arrayPossibleBanners, false, true );
	GetDirListing( "Courses\\" + sFName + ".gif", arrayPossibleBanners, false, true );
	if( arrayPossibleBanners.GetSize() > 0 )
		m_sBannerPath = arrayPossibleBanners[0];

	for( int i=0; i<msd.m_iNumValues; i++ )
	{
		CString sValueName = msd.m_szValuesAndParams[i][0];
		CString* sParams = msd.m_sValuesAndParams[i];

		// handle the data
		if( 0 == stricmp(sValueName, "COURSE") )
			m_sName = sParams[1];

		else if( 0 == stricmp(sValueName, "REPEAT") )
		{
			sParams[1].MakeLower();
			if( sParams[1].Find("yes") != -1 )
				m_bRepeat = true;
		}

		else if( 0 == stricmp(sValueName, "LIVES") )
		{
			m_iLives = atoi( sParams[1] );
		}

		else if( 0 == stricmp(sValueName, "SONG") )
		{
			CString sSongDir = "Songs\\" + sParams[1] + "\\";
			CString sNotesDescription = sParams[2];

			int i;

			Song* pSong = NULL;
			for( i=0; i<apSongs.GetSize(); i++ )	// foreach song
			{
				CString sThisSongDir = apSongs[i]->m_sSongDir;
				if( 0 == stricmp(sThisSongDir, sSongDir) )
					pSong = apSongs[i];
			}
			if( pSong == NULL )	// we didn't find the Song
				continue;	// skip this song
			
			AddStage( pSong, sNotesDescription );
		}

		else
			LOG->WriteLine( "Unexpected value named '%s'", sValueName );
	}
}


void Course::CreateFromGroupAndDifficultyClass( CString sGroupName, DifficultyClass dc, CArray<Song*,Song*> &apSongsInGroup )
{
	CString sShortGroupName = SONGMAN->ShortenGroupName( sGroupName );	

	m_sName = sShortGroupName + " ";
	switch( dc )
	{
	case CLASS_EASY:	m_sName += "Easy";		break;
	case CLASS_MEDIUM:	m_sName += "Medium";	break;
	case CLASS_HARD:	m_sName += "Hard";		break;
	}

	for( int s=0; s<apSongsInGroup.GetSize(); s++ )
	{
		Song* pSong = apSongsInGroup[s];
		AddStage( pSong, DifficultyClassToString(dc) );
	}
}


Notes* Course::GetNotesForStage( int iStage )
{
	Song* pSong = m_apSongs[iStage];
	CString sDescription = m_asDescriptions[iStage];

	for( int i=0; i<pSong->m_apNotes.GetSize(); i++ )
	{
		Notes* pNotes = pSong->m_apNotes[i];
		if( 0==stricmp(pNotes->m_sDescription, sDescription)  &&
			pNotes->m_NotesType == GAMESTATE->GetCurrentStyleDef()->m_NotesType )
			return pNotes;
	}


	// Didn't find a matching description.  Try to match the DifficultyClass instead.
	DifficultyClass dc = Notes::DifficultyClassFromDescriptionAndMeter( sDescription, 5 );

	for( i=0; i<pSong->m_apNotes.GetSize(); i++ )
	{
		Notes* pNotes = pSong->m_apNotes[i];
		if( pNotes->m_DifficultyClass == dc )
			return pNotes;
	}


	return NULL;
}


void Course::GetSongAndNotesForCurrentStyle( CArray<Song*,Song*>& apSongsOut, CArray<Notes*,Notes*> apNotesOut[NUM_PLAYERS] )
{
	for( int i=0; i<m_iStages; i++ )
	{
		Song* pSong = m_apSongs[i];
		Notes* pNotes = GetNotesForStage( i );

		if( pNotes == NULL )
			continue;	// skip

		apSongsOut.Add( pSong );
		for( int p=0; p<NUM_PLAYERS; p++ )
			apNotesOut[p].Add( pNotes );
	}
}

D3DXCOLOR Course::GetColor()
{
	// This could be made smarter
	if( m_iStages >= 7 )
		return D3DXCOLOR(1,0,0,1);	// red
	else if( m_iStages >= 4 )
		return D3DXCOLOR(1,0.5f,0,1);	// orange
	else
		return D3DXCOLOR(0,1,0,1);	// green
}

//
// Sorting stuff
//

int CompareCoursePointersByDifficulty(const void *arg1, const void *arg2)
{
	Course* pCourse1 = *(Course**)arg1;
	Course* pCourse2 = *(Course**)arg2;
	
	float fScore1 = (float)pCourse1->m_iStages;
	float fScore2 = (float)pCourse2->m_iStages;

	if( fScore1 < fScore2 )
		return -1;
	else if( fScore1 == fScore2 )
		return 0;
	else
		return 1;
}

void SortCoursePointerArrayByDifficulty( CArray<Course*,Course*> &apCourses )
{
	qsort( apCourses.GetData(), apCourses.GetSize(), sizeof(Course*), CompareCoursePointersByDifficulty );
}
