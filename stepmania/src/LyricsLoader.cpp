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
	LOG->Trace( "LyricsLoader::LoadFromLRCFile(%s)", sPath.GetString() );
	

	LRCFile lrc;
	bool bResult = lrc.ReadFile( sPath );
	RageColor	m_LastFoundColor(0,1,0,1);

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
			float r=1,b=1,g=1;	// initialize in case sscanf fails
			int result = sscanf( sValueData.GetString(), "0x%2x%2x%2x", &r, &g, &b );
			if(result != 3)
			{
				LOG->Trace( "The color value '%s' in '%s' is invalid.",
				sValueData.GetString(), sPath.GetString() );
				continue;
			}

			m_LastFoundColor = RageColor(r,g,b,1);
			continue;
		}

		else
		{
			/* If we've gotten this far, and no other statement caught
			   this value before this does, assume it's a time value. */		
			
			LyricSegment seg;
			seg.m_Color = m_LastFoundColor;
			seg.m_fStartTime = TimeToSeconds(sValueName);
			seg.m_sLyric = sValueData;
			
			/* Er, huh?  This won't do anything (replace \ with \). What's wrong
			 * with using \ in lyrics? */
			seg.m_sLyric.Replace( "\\", "\\" ); // to avoid possible screw-ups 
							    						// if someone uses a \ for whatever
												        // reason in their lyrics -- Miryokuteki
			
			seg.m_sLyric.Replace( "|","\n" ); // Pipe symbols denote a new line in LRC files
			out.AddLyricSegment( seg );
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

