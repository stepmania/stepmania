#include "global.h"
#include "NotesLoader.h"
#include "NoteTypes.h"
#include "GameManager.h"

bool NotesLoader::Loadable( CString sPath )
{
	CStringArray list;
	GetApplicableFiles( sPath, list );
	return !list.empty();
}

void NotesLoader::GetMainAndSubTitlesFromFullTitle( const CString sFullTitle, CString &sMainTitleOut, CString &sSubTitleOut )
{
	const CString sLeftSeps[]  = { " -", " ~", " (", " [" };

	for( unsigned i=0; i<ARRAYSIZE(sLeftSeps); i++ )
	{
		int iBeginIndex = sFullTitle.Find( sLeftSeps[i] );
		if( iBeginIndex == -1 )
			continue;
		sMainTitleOut = sFullTitle.Left( iBeginIndex );
		sSubTitleOut = sFullTitle.substr( iBeginIndex+1, sFullTitle.GetLength()-iBeginIndex+1 );
		return;
	}
	sMainTitleOut = sFullTitle; 
	sSubTitleOut = ""; 
};	

