#include "global.h"
#include "NotesLoader.h"
#include "NoteTypes.h"
#include "GameManager.h"
#include "RageUtil.h"

#include "NotesLoaderSM.h"
#include "NotesLoaderDWI.h"
#include "NotesLoaderBMS.h"
#include "NotesLoaderKSF.h"

bool NotesLoader::Loadable( const RString &sPath )
{
	vector<RString> list;
	GetApplicableFiles( sPath, list );
	return !list.empty();
}

void NotesLoader::GetMainAndSubTitlesFromFullTitle( const RString &sFullTitle, RString &sMainTitleOut, RString &sSubTitleOut )
{
	const RString sLeftSeps[]  = { " -", " ~", " (", " [", "\t" };

	for( unsigned i=0; i<ARRAYLEN(sLeftSeps); i++ )
	{
		size_t iBeginIndex = sFullTitle.find( sLeftSeps[i] );
		if( iBeginIndex == string::npos )
			continue;
		sMainTitleOut = sFullTitle.Left( (int) iBeginIndex );
		sSubTitleOut = sFullTitle.substr( iBeginIndex+1, sFullTitle.size()-iBeginIndex+1 );
		return;
	}
	sMainTitleOut = sFullTitle; 
	sSubTitleOut = ""; 
};

NotesLoader *NotesLoader::MakeLoader( const RString &sDir )
{
	NotesLoader *ret;
	
	/* Actually, none of these have any persistant data, so we 
	* could optimize this, but since they don't have any data,
	* there's no real point ... */
	ret = new SMLoader;
	if(ret->Loadable( sDir )) return ret;
	delete ret;
	
	ret = new DWILoader;
	if(ret->Loadable( sDir )) return ret;
	delete ret;
	
	ret = new BMSLoader;
	if(ret->Loadable( sDir )) return ret;
	delete ret;
	
	ret = new KSFLoader;
	if(ret->Loadable( sDir )) return ret;
	delete ret;
	
	return NULL;
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
