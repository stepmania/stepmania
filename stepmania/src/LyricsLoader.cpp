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
	CString	m_sLastFoundColor = "0x00ff00";

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
		if( 0==stricmp(sValueName,"COLOUR") || 0==stricmp(sValueName,"COLOR") )
		{
			// set color var here for this segment
			m_sLastFoundColor = sValueData;
			continue;
		}

		else
		{
			/* If we've gotten this far, and no other statement caught
			   this value before this does, assume it's a time value. */		
			
			LyricSegment	LYRICSTRING;
			LYRICSTRING.m_sColor = m_sLastFoundColor;
			LYRICSTRING.m_fStartTime = TimeToSeconds(sValueName);
			LYRICSTRING.m_sLyric = sValueData;
			
			LYRICSTRING.m_sLyric.Replace( "\\", "\\" ); // to avoid possible screw-ups 
							    						// if someone uses a \ for whatever
												        // reason in their lyrics -- Miryokuteki
			
			LYRICSTRING.m_sLyric.Replace( "|","\n" ); // Pipe symbols denote a new line in LRC files
			out.AddLyricSegment( LYRICSTRING );
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

