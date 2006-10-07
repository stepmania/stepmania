#include "global.h"
#include "CourseWriterCRS.h"
#include "Course.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "song.h"
#include "RageFileDriverMemory.h"


bool CourseWriterCRS::Write( const Course &course, const RString &sPath, bool bSavingCache )
{
	RageFile f;
	if( !f.Open( sPath, RageFile::WRITE ) )
	{
		LOG->UserLog( "Course file", sPath, "couldn't be written: %s", f.GetError().c_str() );
		return false;
	}

	return CourseWriterCRS::Write( course, f, bSavingCache );
}

void CourseWriterCRS::GetEditFileContents( const Course *pCourse, RString &sOut )
{
	RageFileObjMem mem;
	CourseWriterCRS::Write( *pCourse, mem, true );
	sOut = mem.GetString();
}

bool CourseWriterCRS::Write( const Course &course, RageFileBasic &f, bool bSavingCache )
{
	ASSERT( !course.m_bIsAutogen );

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

			vector<RString> asRadarValues;
			const RadarValues &rv = it->second;
			for( int r=0; r < NUM_RadarCategory; r++ )
				asRadarValues.push_back( ssprintf("%.3f", rv[r]) );
			RString sLine = ssprintf( "#RADAR:%i:%i:", st, cd );
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
			const RString &sSong = Basename( entry.pSong->GetSongDir() );
			
			f.Write( "#SONG:" );
			if( !entry.songCriteria.m_sGroupName.empty() )
				f.Write( entry.songCriteria.m_sGroupName + '/' );
			f.Write( sSong );
		}
		else if( !entry.songCriteria.m_sGroupName.empty() )
		{
			f.Write( ssprintf( "#SONG:%s/*", entry.songCriteria.m_sGroupName.c_str() ) );
		}
		else
		{
			f.Write( "#SONG:*" );
		}

		f.Write( ":" );
		if( entry.stepsCriteria.m_difficulty != Difficulty_Invalid )
			f.Write( DifficultyToString(entry.stepsCriteria.m_difficulty) );
		else if( entry.stepsCriteria.m_iLowMeter != -1  &&  entry.stepsCriteria.m_iHighMeter != -1 )
			f.Write( ssprintf( "%d..%d", entry.stepsCriteria.m_iLowMeter, entry.stepsCriteria.m_iHighMeter ) );
		f.Write( ":" );

		RString sModifiers = entry.sModifiers;

		if( entry.bSecret )
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

		if( entry.iGainLives > -1 )
		{
			if( !sModifiers.empty() )
				sModifiers += ',';
			sModifiers += ssprintf( "award%d", entry.iGainLives );
		}
		
		f.Write( sModifiers );

		f.PutLine( ";" );
	}	
	
	return true;
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
