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
#include "ThemeManager.h"
#include "Song.h"


void Course::LoadFromCRSFile( CString sPath, CArray<Song*,Song*> &apSongs )
{
	CStdioFile file;	
	if( !file.Open( sPath, CFile::modeRead|CFile::shareDenyNone ) )
		throw RageException( "Error opening CRS file '%s'.", sPath );


	// read the whole file into a sFileText
	CString sFileText;
	CString buffer;
	while( file.ReadString(buffer) )
		sFileText += buffer + "\n";
	file.Close();

	// strip comments out of sFileText
	while( sFileText.Find("//") != -1 )
	{
		int iIndexCommentStart = sFileText.Find("//");
		int iIndexCommentEnd = sFileText.Find("\n", iIndexCommentStart);
		if( iIndexCommentEnd == -1 )	// comment doesn't have an end?
			sFileText.Delete( iIndexCommentStart, 2 );
		else
			sFileText.Delete( iIndexCommentStart, iIndexCommentEnd-iIndexCommentStart );
	}

	// split sFileText into strings containing each value expression
	CStringArray arrayValueStrings;
	split( sFileText, ";", arrayValueStrings, false );


	// for each value expression string, parse it into a value name and data
	for( int i=0; i < arrayValueStrings.GetSize(); i++ )
	{
		CString sValueString = arrayValueStrings[i];

		// split the value string into tokens
		CStringArray arrayValueTokens;
		split( sValueString, ":", arrayValueTokens, false );
		for( int j=0; j<arrayValueTokens.GetSize(); j++ )
		{
			arrayValueTokens[j].TrimLeft();
			arrayValueTokens[j].TrimRight();
		}
		if( arrayValueTokens.GetSize() == 0 )
			continue;

		CString sValueName = arrayValueTokens.GetAt( 0 );

		// handle the data
		if( sValueName == "#COURSE" )
			m_sName = arrayValueTokens[1];

		else if( sValueName == "#REPEAT" )
		{
			arrayValueTokens[1].MakeLower();
			if( arrayValueTokens[1].Find("yes") != -1 )
				m_bRepeat = true;
		}

		else if( sValueName == "#SONG" )
		{
			CString sSongDir = arrayValueTokens[1];
			CString sNotesDescription = arrayValueTokens[2];

			int i;

			Song* pSong = NULL;
			for( i=0; i<apSongs.GetSize(); i++ )
				if( 0 == stricmp(apSongs[i]->m_sSongDir, sSongDir) )
					pSong = apSongs[i];

			if( pSong == NULL )	// we didn't find the Song
				continue;	// skip this song

			DifficultyClass dc = Notes::DifficultyClassFromDescriptionAndMeter( sNotesDescription, 6 );
			
			AddStage( pSong, dc );
		}

		else
			LOG->WriteLine( "Unexpected value named '%s'", sValueName );
	}
}
