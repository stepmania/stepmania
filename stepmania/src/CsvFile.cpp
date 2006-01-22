#include "global.h"
#include "CsvFile.h"
#include "RageUtil.h"
#include "RageFile.h"
#include "RageLog.h"

CsvFile::CsvFile()
{
}

bool CsvFile::ReadFile( const RString &sPath )
{
	m_sPath = sPath;
	CHECKPOINT_M( ssprintf("Reading '%s'",m_sPath.c_str()) );

	RageFile f;
	if( !f.Open( m_sPath ) )
	{
		LOG->Trace( "Reading '%s' failed: %s", m_sPath.c_str(), f.GetError().c_str() );
		m_sError = f.GetError();
		return 0;
	}

	return ReadFile( f );
}

bool CsvFile::ReadFile( RageFileBasic &f )
{
	m_vvs.clear();

	// hi,"hi2,","""hi3"""

	while( 1 )
	{
		RString line;
		switch( f.GetLine(line) )
		{
		case -1:
			m_sError = f.GetError();
			return false;
		case 0:
			return true; /* eof */
		}

		utf8_remove_bom( line );

		vector<RString> vs;

		while( !line.empty() )
		{
			if( line[0] == '\"' )	// quoted value
			{
				line.erase( line.begin() );	// eat open quote
				RString::size_type iEnd = 0;
				do
				{
					iEnd = line.find('\"', iEnd);
					if( iEnd == line.npos )
					{
						iEnd = line.size()-1;	// didn't find an end.  Take the whole line.
						break;
					}

					if( line.size() > iEnd+1 && line[iEnd+1] == '\"' )	// next char is also double quote
						iEnd = iEnd+2;
					else
						break;
				}
				while(true);

				RString sValue = line;
				sValue = sValue.Left( iEnd );
				vs.push_back( sValue );

				line.erase( line.begin(), line.begin()+iEnd );

				if( !line.empty() && line[0] == '\"' )
					line.erase( line.begin() );
			}
			else
			{
				RString::size_type iEnd = line.find(',');
				if( iEnd == line.npos )
					iEnd = line.size();	// didn't find an end.  Take the whole line

				RString sValue = line;
				sValue = sValue.Left( iEnd );
				vs.push_back( sValue );

				line.erase( line.begin(), line.begin()+iEnd );
			}

			if( !line.empty() && line[0] == ',' )
				line.erase( line.begin() );
		}

		m_vvs.push_back( vs );
	}
}

bool CsvFile::WriteFile( const RString &sPath ) const
{
	RageFile f;
	if( !f.Open( sPath, RageFile::WRITE ) )
	{
		LOG->Trace( "Writing '%s' failed: %s", sPath.c_str(), f.GetError().c_str() );
		m_sError = f.GetError();
		return false;
	}

	return CsvFile::WriteFile( f );
}

bool CsvFile::WriteFile( RageFileBasic &f ) const
{
	FOREACH_CONST( StringVector, m_vvs, line ) 
	{
		RString sLine;
		FOREACH_CONST( RString, *line, value ) 
		{
			RString sVal = *value;
			sVal.Replace( "\"", "\"\"" );	// escape quotes to double-quotes
			sLine += "\"" + sVal + "\"";
			if( value != line->end()-1 )
				sLine += ",";
		}
		if( f.PutLine(sLine) == -1 )
		{
			m_sError = f.GetError();
			return false;
		}
	}
	return true;
}


/*
 * (c) 2001-2004 Adam Clauss, Chris Danford
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
