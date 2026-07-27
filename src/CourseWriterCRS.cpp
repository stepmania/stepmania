#include "global.h"
#include "CourseWriterCRS.h"
#include "Course.h"
#include "RageFile.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "RageFileDriverMemory.h"

#include <vector>


/** @brief Load the difficulty names from CourseLoaderCRS. */
extern const char *g_CRSDifficultyNames[]; // in CourseLoaderCRS

/**
 * @brief Get the string of the course difficulty.
 * @param iVal the course difficulty.
 * @return the string.
 */
static RString DifficultyToCRSString( CourseDifficulty iVal )
{
	return g_CRSDifficultyNames[iVal];
}

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
	if( course.m_sScripter != "" )
		f.PutLine( ssprintf("#SCRIPTER:%s;", course.m_sScripter.c_str()) );
	if ( course.m_sDescription != "" )
	    f.PutLine( ssprintf("#DESCRIPTION:%s;", course.m_sDescription.c_str()) );
	if( course.m_bRepeat )
		f.PutLine( "#REPEAT:YES;" );
	if( course.m_iLives != -1 )
		f.PutLine( ssprintf("#LIVES:%i;", course.m_iLives) );
	if( !course.m_sBannerPath.empty() )
		f.PutLine( ssprintf("#BANNER:%s;", course.m_sBannerPath.c_str()) );

	if( !course.m_setStyles.empty() )
	{
		std::vector<RString> asStyles;
		asStyles.insert( asStyles.begin(), course.m_setStyles.begin(), course.m_setStyles.end() );
		f.PutLine( ssprintf("#STYLE:%s;", join( ",", asStyles ).c_str()) );
	}

	FOREACH_ENUM( CourseDifficulty,cd )
	{
		if( course.m_iCustomMeter[cd] == -1 )
			continue;
		f.PutLine( ssprintf("#METER:%s:%i;", DifficultyToCRSString(cd).c_str(), course.m_iCustomMeter[cd]) );
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

			std::vector<RString> asRadarValues;
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
		if( entry.bUseSongSelect )
		{
			WriteSongSelectCourseEntry(entry, f);
		}
		else
		{
			WriteCourseEntry(entry, f);
		}
	}

	return true;
}

bool CourseWriterCRS::WriteCourseEntry( const CourseEntry &entry, RageFileBasic &f )
{

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
	else if( entry.songSort == SongSort_TopGrades && entry.iChooseIndex != -1 )
	{
		f.Write( ssprintf( "#SONG:GRADEBEST%d", entry.iChooseIndex + 1 ) );
	}
	else if( entry.songSort == SongSort_LowestGrades && entry.iChooseIndex != -1 )
	{
		f.Write( ssprintf( "#SONG:GRADEWORST%d", entry.iChooseIndex + 1 ) );
	}
	else if( entry.songID.ToSong() )
	{
		Song *pSong = entry.songID.ToSong();
		const RString &sSong = Basename( pSong->GetSongDir() );

		f.Write( "#SONG:" );
		if( entry.songCriteria.m_vsGroupNames.size() > 0 )
			f.Write( entry.songCriteria.m_vsGroupNames[0] + '/' );
		f.Write( sSong );
	}
	else if( entry.songCriteria.m_vsGroupNames.size() > 0 )
	{
		f.Write( ssprintf( "#SONG:%s/*", entry.songCriteria.m_vsGroupNames[0].c_str() ) );
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
	return true;
}

bool CourseWriterCRS::WriteSongSelectCourseEntry( const CourseEntry &entry, RageFileBasic &f )
{
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

	std::vector<RString> songSelectParams;

	//TODO: Re-escape everything
	if( entry.songCriteria.m_vsSongNames.size() > 0 )
	{
		RString songNames = join(",", SmEscape( entry.songCriteria.m_vsSongNames, {'\\', ':', ';', '#', ',', '='} ));
		songSelectParams.push_back(ssprintf("TITLE=%s", songNames.c_str()));
	}
	if( entry.songCriteria.m_vsGroupNames.size() > 0)
	{
		RString groupNames = join(",", SmEscape( entry.songCriteria.m_vsGroupNames, {'\\', ':', ';', '#', ',', '='} ));
		songSelectParams.push_back(ssprintf("GROUP=%s", groupNames.c_str()));
	}
	if( entry.songCriteria.m_vsArtistNames.size() > 0)
	{
		RString artistNames = join(",", SmEscape( entry.songCriteria.m_vsArtistNames, {'\\', ':', ';', '#', ',', '='} ));
		songSelectParams.push_back(ssprintf("ARTIST=%s", artistNames.c_str()));
	}
	if( entry.songCriteria.m_bUseSongAllowedList &&
		entry.songCriteria.m_vsSongGenreAllowedList.size() > 0)
	{
		RString genreNames = join(",", SmEscape( entry.songCriteria.m_vsSongGenreAllowedList, {'\\', ':', ';', '#', ',', '='} ));
		songSelectParams.push_back(ssprintf("GENRE=%s", genreNames.c_str()));
	}
	if( entry.stepsCriteria.m_vDifficulties.size() > 0 )
	{
		std::vector<RString> difficulties;
		for (unsigned d = 0; d < entry.stepsCriteria.m_vDifficulties.size(); d++)
		{
			Difficulty diff = entry.stepsCriteria.m_vDifficulties[d];
			if( diff != Difficulty_Invalid)
			{
				difficulties.push_back(DifficultyToString(diff));
			}
		}
		songSelectParams.push_back("DIFFICULTY=" + join(",", difficulties));
	}
	if( entry.songSort != SongSort_Randomize && entry.iChooseIndex > -1)
	{
		RString songSort = SongSortToString(entry.songSort);
		songSelectParams.push_back(ssprintf("SORT=%s,%d", songSort.c_str(), entry.iChooseIndex+1));
	}
	if(entry.songCriteria.m_fMinDurationSeconds > 0 
		&& entry.songCriteria.m_fMaxDurationSeconds > 0 )
	{
		songSelectParams.push_back(ssprintf("DURATION=%d-%d", (int)entry.songCriteria.m_fMinDurationSeconds, (int)entry.songCriteria.m_fMaxDurationSeconds));
	}
	if(entry.songCriteria.m_fMinBPM > 0 && entry.songCriteria.m_fMaxBPM > 0 )
	{
		songSelectParams.push_back(ssprintf("BPMRANGE=%f-%f", entry.songCriteria.m_fMinBPM, entry.songCriteria.m_fMaxBPM));
	}
	if(entry.stepsCriteria.m_iLowMeter > 0 && entry.stepsCriteria.m_iHighMeter > 0 )
	{
		songSelectParams.push_back(ssprintf("METER=%d-%d", entry.stepsCriteria.m_iLowMeter, entry.stepsCriteria.m_iHighMeter));
	}
	if( entry.iGainLives > 0 )
	{
		songSelectParams.push_back(ssprintf("GAINLIVES=%d", entry.iGainLives));
	}
	if(entry.fGainSeconds > 0 )
	{
		songSelectParams.push_back(ssprintf("GAINSECONDS=%f", entry.fGainSeconds));
	}

	std::vector<RString> mods;
	split(entry.sModifiers, ",", mods);

	if (entry.bSecret)
	{
		mods.push_back("noshowcourse");
	}
	else
	{
		mods.push_back("showcourse");
	}
	if( entry.bNoDifficult )
	{
		mods.push_back("nodifficult");
	}
	songSelectParams.push_back("MODS=" + join(",", mods));

	RString songSelect = join(":", songSelectParams);
	f.PutLine(ssprintf("#SONGSELECT:%s;", songSelect.c_str()));


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
