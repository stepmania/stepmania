#include "global.h"
/*
	Loader for lyrics files
	By: Kevin Slaughter
*/
#include "RageLog.h"
#include "RageException.h"
#include "RageUtil.h"
#include "LRCFile.h"
#include "LyricsLoader.h"

#include <map>
using namespace std;


bool LyricsLoader::LoadFromLRCFile( CString sPath, Song &out )
{
	LOG->Trace( "Song::LoadFromLRCFile(%s)", sPath.GetString() );
	

	LRCFile lrc;
	bool bResult = lrc.ReadFile( sPath );
	if( !bResult )
		RageException::Throw( "Error opening file '%s' for reading.", sPath.GetString() );
	
	// unsigned iLineCount = 0; // hush "variable not referenced"
	for( unsigned i=0; i<lrc.GetNumValues(); i++ )
	{
		int iNumParams = lrc.GetNumParams(i);
		const LRCFile::value_t &sParams = lrc.GetValue(i);
		
		CString sValueName = sParams[0];
		CString sValueData = sParams[1];

		if(iNumParams < 1)
		{
			LOG->Warn("Got \"%s\" tag with no parameters", sValueName.GetString());
			continue;
		}

		// handle the data
		if( 0==stricmp(sValueName,"COLOUR") )
		{
			// set color var here
			LOG->Trace("\n\n\n Got color tag from lyric file \n\n\n");
			continue;
		}

		else
		{
			/* If we've gotten this far, and no other statement caught
			   this value before this does, assume it's a time value.
			   Add the lyric segment! */
			
			// For the sorting routine, we need a numerical version of the 'time'
				CString	sTempTime = sValueName.GetBuffer();
				sTempTime.Replace( ".", "" );
				sTempTime.Replace( ":", "." );

				// float	m_fStartTime = (float)atof(sTempTime);	// hush "variable not referenced"
				CString m_sLyric = sValueData;
				CString	m_sStartTime = sValueName;
			//--
			
			//LyricSegment	MOOZ;
			//MOOZ.m_fStartTime = (float)atof((LPCTSTR)sTempTime);
			//MOOZ.m_sStartTime = sValueName.GetBuffer();
			//MOOZ.m_sLyric = sValueData.GetBuffer();

			//out.AddLyricSegment( LyricSegment( m_fStartTime, m_sLyric, m_sStartTime ) );
		}
		
	}

	return true;
}

void LyricsLoader::GetApplicableFiles( CString sPath, CStringArray &out )
{
	GetDirListing( sPath + CString("*.dwi"), out );
}

bool LyricsLoader::LoadFromDir( CString sPath, Song &out )
{
	CStringArray aFileNames;
	GetApplicableFiles( sPath, aFileNames );

	if( aFileNames.size() > 1 )
		RageException::Throw( "There is more than one DWI file in '%s'.  There should be only one!", sPath.GetString() );

	/* We should have exactly one; if we had none, we shouldn't have been
	 * called to begin with. */
	ASSERT( aFileNames.size() == 1 );

	return LoadFromLRCFile( sPath + aFileNames[0], out );
}

