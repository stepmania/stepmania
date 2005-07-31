#include "global.h"
#include "CourseWriterCRS.h"
#include "Course.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"


bool CourseWriterCRS::Write( const Course &course, const CString &sPath, bool bSavingCache )
{
	ASSERT( !course.m_bIsAutogen );

	RageFile f;
	if( !f.Open( sPath, RageFile::WRITE ) )
	{
		LOG->Warn( "Could not write course file '%s': %s", sPath.c_str(), f.GetError().c_str() );
		return false;
	}

	f.PutLine( ssprintf("#COURSE:%s;", course.m_sMainTitle.c_str()) );
	if( course.m_sMainTitleTranslit != "" )
		f.PutLine( ssprintf("#COURSETRANSLIT:%s;", course.m_sMainTitleTranslit.c_str()) );
	if( course.m_bRepeat )
		f.PutLine( "#REPEAT:YES;" );
	if( course.m_iLives != -1 )
		f.PutLine( ssprintf("#LIVES:%i;", course.m_iLives) );

	FOREACH_CourseDifficulty( cd )
	{
		if( course.m_iCustomMeter[cd] == -1 )
			continue;
		f.PutLine( ssprintf("#METER:%s:%i;", CourseDifficultyToString(cd).c_str(), course.m_iCustomMeter[cd]) );
	}

	if( bSavingCache )
	{
		f.PutLine( "// cache tags:" );

		Course::RadarCache_t::const_iterator it;
		for( it = course.m_RadarCache.begin(); it != course.m_RadarCache.end(); ++it )
		{
			// #RADAR:type:difficulty:value,value,value...;
			const Course::CacheEntry &entry = it->first;
			StepsType st = entry.first;
			CourseDifficulty cd = entry.second;

			CStringArray asRadarValues;
			const RadarValues &rv = it->second;
			for( int r=0; r < NUM_RADAR_CATEGORIES; r++ )
				asRadarValues.push_back( ssprintf("%.3f", rv[r]) );
			CString sLine = ssprintf( "#RADAR:%i:%i:", st, cd );
			sLine += join( ",", asRadarValues ) + ";";
			f.PutLine( sLine );
		}
		f.PutLine( "// end cache tags" );
	}

	for( unsigned i=0; i<course.m_vEntries.size(); i++ )
	{
		const CourseEntry& entry = course.m_vEntries[i];

		for( unsigned j = 0; j < entry.attacks.size(); ++j )
		{
			if( j == 0 )
				f.PutLine( "#MODS:" );

			const Attack &a = entry.attacks[j];
			f.Write( ssprintf( "  TIME=%.2f:LEN=%.2f:MODS=%s",
				a.fStartSecond, a.fSecsRemaining, a.sModifiers.c_str() ) );

			if( j+1 < entry.attacks.size() )
				f.Write( ":" );
			else
				f.Write( ";" );
			f.PutLine( "" );
		}

		if( entry.fGainSeconds > 0 )
			f.PutLine( ssprintf("#GAINSECONDS:%f;", entry.fGainSeconds) );

		if( entry.songSort == SongSort_MostPlays  &&  entry.iChooseIndex != -1 )
		{
			f.Write( ssprintf( "#SONG:BEST%d", entry.iChooseIndex+1 ) );
		}
		else if( entry.songSort == SongSort_FewestPlays  &&  entry.iChooseIndex != -1 )
		{
			f.Write( ssprintf( "#SONG:WORST%d", entry.iChooseIndex+1 ) );
		}
		else if( entry.pSong )
		{
			// strip off everything but the group name and song dir
			CStringArray as;
			ASSERT( entry.pSong != NULL );
			split( entry.pSong->GetSongDir(), "/", as );
			ASSERT( as.size() >= 2 );
			CString sGroup = as[ as.size()-2 ];
			CString sSong = as[ as.size()-1 ];
			f.Write( "#SONG:" + sGroup + '/' + sSong );
		}
		else if( !entry.sSongGroup.empty() )
		{
			f.Write( ssprintf( "#SONG:%s/*", entry.sSongGroup.c_str() ) );
		}
		else 
		{
			f.Write( "#SONG:*" );
		}

		f.Write( ":" );
		if( entry.baseDifficulty != DIFFICULTY_INVALID )
			f.Write( DifficultyToString(entry.baseDifficulty) );
		else if( entry.iLowMeter != -1  &&  entry.iHighMeter != -1 )
			f.Write( ssprintf( "%d..%d", entry.iLowMeter, entry.iHighMeter ) );
		f.Write( ":" );

		CString sModifiers = entry.sModifiers;
		bool bDefaultSecret = entry.IsRandomSong();
		if( bDefaultSecret != entry.bSecret )
		{
			if( sModifiers != "" )
				sModifiers += ",";
			sModifiers += entry.bSecret? "noshowcourse":"showcourse";
		}

		if( entry.bNoDifficult )
		{
			if( sModifiers != "" )
				sModifiers += ",";
			sModifiers += "nodifficult";
		}
		f.Write( sModifiers );

		f.PutLine( ";" );
	}	
	
	return true;
}

void CourseWriterCRS::GetEditFile( const Course *pCourse, CString &sOut )
{

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
