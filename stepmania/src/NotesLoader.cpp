#include "stdafx.h"
#include "NotesLoader.h"
#include "NoteTypes.h"
#include "NoteData.h"
#include "GameManager.h"
#include "RageUtil.h"

bool NotesLoader::Loadable( CString sPath )
{
	CStringArray list;
	GetApplicableFiles( sPath, list );
	return !list.empty();
}

void NotesLoader::GetMainAndSubTitlesFromFullTitle( const CString sFullTitle, CString &sMainTitleOut, CString &sSubTitleOut )
{
	const CString sLeftSeps[]  = { " -", " ~", " (", " [" };
	int iNumSeps = sizeof(sLeftSeps)/sizeof(CString);
	for( int i=0; i<iNumSeps; i++ )
	{
		int iBeginIndex = sFullTitle.Find( sLeftSeps[i] );
		if( iBeginIndex == -1 )
			continue;
		sMainTitleOut = sFullTitle.Left( iBeginIndex );
		sSubTitleOut = sFullTitle.Mid( iBeginIndex+1, sFullTitle.GetLength()-iBeginIndex+1 );
		return;
	}
	sMainTitleOut = sFullTitle; 
	sSubTitleOut = ""; 
};	

