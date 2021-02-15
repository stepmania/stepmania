#include "global.h"
#include "CourseLoaderCRS.h"
#include "RageLog.h"
#include "Course.h"
#include "RageUtil.h"
#include "SongCacheIndex.h"
#include "PrefsManager.h"
#include "MsdFile.h"
#include "PlayerOptions.h"
#include "SongManager.h"
#include "TitleSubstitution.h"
#include "ImageCache.h"
#include "RageFileManager.h"
#include "CourseWriterCRS.h"
#include "RageUtil.h"
#include "CourseUtil.h"
#include <float.h>

/** @brief Edit courses can only be so big before they are rejected. */
const int MAX_EDIT_COURSE_SIZE_BYTES	= 32*1024;	// 32KB

/** @brief The list of difficulty names for courses. */
const char *g_CRSDifficultyNames[] =
{
	"Beginner",
	"Easy",
	"Regular",
	"Difficult",
	"Challenge",
	"Edit",
};

/**
 * @brief Retrieve the course difficulty based on the string name.
 * @param s the name of the difficulty.
 * @return the course difficulty.
 */
static CourseDifficulty CRSStringToDifficulty( const RString& s )
{
	FOREACH_ENUM( Difficulty,i)
		if( !s.CompareNoCase(g_CRSDifficultyNames[i]) )
			return i;
	return Difficulty_Invalid;
}


bool CourseLoaderCRS::LoadFromBuffer( const RString &sPath, const RString &sBuffer, Course &out )
{
	MsdFile msd;
	msd.ReadFromString( sBuffer, false );  // don't unescape
	return LoadFromMsd( sPath, msd, out, true );
}

bool CourseLoaderCRS::LoadFromMsd( const RString &sPath, const MsdFile &msd, Course &out, bool bFromCache )
{
	AttackArray attacks;
	float fGainSeconds = 0;
	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		RString sValueName = msd.GetParam(i, 0);
		const MsdFile::value_t &sParams = msd.GetValue(i);

		// handle the data
		if( sValueName.EqualsNoCase("COURSE") )
			out.m_sMainTitle = sParams[1];
		else if( sValueName.EqualsNoCase("COURSETRANSLIT") )
			out.m_sMainTitleTranslit = sParams[1];
		else if( sValueName.EqualsNoCase("SCRIPTER") )
			out.m_sScripter = sParams[1];
		else if( sValueName.EqualsNoCase("DESCRIPTION") )
			out.m_sDescription = sParams[1];
		else if( sValueName.EqualsNoCase("REPEAT") )
		{
			RString str = sParams[1];
			str.MakeLower();
			if( str.find("yes") != string::npos )
				out.m_bRepeat = true;
		}

		else if( sValueName.EqualsNoCase("BANNER") )
		{
			out.m_sBannerPath = sParams[1];
		}
		else if( sValueName.EqualsNoCase("BACKGROUND") )
		{
			out.m_sBackgroundPath = sParams[1];
		}
		else if( sValueName.EqualsNoCase("LIVES") )
		{
			out.m_iLives = max( StringToInt(sParams[1]), 0 );
		}
		else if( sValueName.EqualsNoCase("GAINSECONDS") )
		{
			fGainSeconds = StringToFloat( sParams[1] );
		}
		else if( sValueName.EqualsNoCase("METER") )
		{
			if( sParams.params.size() == 2 )
			{
				out.m_iCustomMeter[Difficulty_Medium] = max( StringToInt(sParams[1]), 0 ); /* compat */
			}
			else if( sParams.params.size() == 3 )
			{
				const CourseDifficulty cd = CRSStringToDifficulty( sParams[1] );
				if( cd == Difficulty_Invalid )
				{
					LOG->UserLog( "Course file", sPath, "contains an invalid #METER string: \"%s\"", sParams[1].c_str() );
					continue;
				}
				out.m_iCustomMeter[cd] = max( StringToInt(sParams[2]), 0 );
			}
		}

		else if( sValueName.EqualsNoCase("MODS") )
		{
			Attack attack;
			float end = -9999;
			for( unsigned j = 1; j < sParams.params.size(); ++j )
			{
				vector<RString> sBits;
				split( sParams[j], "=", sBits, false );
				if( sBits.size() < 2 )
					continue;

				Trim( sBits[0] );
				if( !sBits[0].CompareNoCase("TIME") )
					attack.fStartSecond = max( StringToFloat(sBits[1]), 0.0f );
				else if( !sBits[0].CompareNoCase("LEN") )
					attack.fSecsRemaining = StringToFloat( sBits[1] );
				else if( !sBits[0].CompareNoCase("END") )
					end = StringToFloat( sBits[1] );
				else if( !sBits[0].CompareNoCase("MODS") )
				{
					attack.sModifiers = sBits[1];

					if( end != -9999 )
					{
						attack.fSecsRemaining = end - attack.fStartSecond;
						end = -9999;
					}

					if( attack.fSecsRemaining <= 0.0f)
					{
						LOG->UserLog( "Course file", sPath, "has an attack with a nonpositive length: %s", sBits[1].c_str() );
						attack.fSecsRemaining = 0.0f;
					}

					// warn on invalid so we catch typos on load
					CourseUtil::WarnOnInvalidMods( attack.sModifiers );

					attacks.push_back( attack );
				}
				else
				{
					LOG->UserLog( "Course file", sPath, "has an unexpected value named '%s'", sBits[0].c_str() );
				}
			}

		}
		else if( sValueName.EqualsNoCase("SONG") )
		{
			CourseEntry new_entry;

			// infer entry::Type from the first param
			// todo: make sure these aren't generating bogus entries due
			// to a lack of songs. -aj
			int iNumSongs = SONGMAN->GetNumSongs();
			// most played
			if( sParams[1].Left(strlen("BEST")) == "BEST" )
			{
				int iChooseIndex = StringToInt( sParams[1].Right(sParams[1].size()-strlen("BEST")) ) - 1;
				if( iChooseIndex > iNumSongs )
				{
					// looking up a song that doesn't exist.
					LOG->UserLog( "Course file", sPath, "is trying to load BEST%i with only %i songs installed. "
						      "This entry will be ignored.", iChooseIndex, iNumSongs);
					out.m_bIncomplete = true;
					continue; // skip this #SONG
				}

				new_entry.iChooseIndex = iChooseIndex;
				CLAMP( new_entry.iChooseIndex, 0, 500 );
				new_entry.songSort = SongSort_MostPlays;
			}
			// least played
			else if( sParams[1].Left(strlen("WORST")) == "WORST" )
			{
				int iChooseIndex = StringToInt( sParams[1].Right(sParams[1].size()-strlen("WORST")) ) - 1;
				if( iChooseIndex > iNumSongs )
				{
					// looking up a song that doesn't exist.
					LOG->UserLog( "Course file", sPath, "is trying to load WORST%i with only %i songs installed. "
						      "This entry will be ignored.", iChooseIndex, iNumSongs);
					out.m_bIncomplete = true;
					continue; // skip this #SONG
				}

				new_entry.iChooseIndex = iChooseIndex;
				CLAMP( new_entry.iChooseIndex, 0, 500 );
				new_entry.songSort = SongSort_FewestPlays;
			}
			// best grades
			else if( sParams[1].Left(strlen("GRADEBEST")) == "GRADEBEST" )
			{
				new_entry.iChooseIndex = StringToInt( sParams[1].Right(sParams[1].size()-strlen("GRADEBEST")) ) - 1;
				CLAMP( new_entry.iChooseIndex, 0, 500 );
				new_entry.songSort = SongSort_TopGrades;
			}
			// worst grades
			else if( sParams[1].Left(strlen("GRADEWORST")) == "GRADEWORST" )
			{
				new_entry.iChooseIndex = StringToInt( sParams[1].Right(sParams[1].size()-strlen("GRADEWORST")) ) - 1;
				CLAMP( new_entry.iChooseIndex, 0, 500 );
				new_entry.songSort = SongSort_LowestGrades;
			}
			else if( sParams[1] == "*" )
			{
				//new_entry.bSecret = true;
			}
			// group random
			else if( sParams[1].Right(1) == "*" )
			{
				//new_entry.bSecret = true;
				RString sSong = sParams[1];
				sSong.Replace( "\\", "/" );
				vector<RString> bits;
				split( sSong, "/", bits );
				if( bits.size() == 2 )
				{
					new_entry.songCriteria.m_sGroupName = bits[0];
				}
				else
				{
					LOG->UserLog( "Course file", sPath, "contains a random_within_group entry \"%s\" that is invalid. "
						      "Song should be in the format \"<group>/*\".", sSong.c_str() );
				}

				if( !SONGMAN->DoesSongGroupExist(new_entry.songCriteria.m_sGroupName) )
				{
					LOG->UserLog( "Course file", sPath, "random_within_group entry \"%s\" specifies a group that doesn't exist. "
						      "This entry will be ignored.", sSong.c_str() );
					out.m_bIncomplete = true;
					continue; // skip this #SONG
				}
			}
			else
			{
				RString sSong = sParams[1];
				sSong.Replace( "\\", "/" );
				vector<RString> bits;
				split( sSong, "/", bits );

				Song *pSong = nullptr;
				if( bits.size() == 2 )
				{
					new_entry.songCriteria.m_sGroupName = bits[0];
					pSong = SONGMAN->FindSong( bits[0], bits[1] );
				}
				else if( bits.size() == 1 )
				{
					pSong = SONGMAN->FindSong( "", sSong );
				}
				new_entry.songID.FromSong( pSong );

				if( pSong == nullptr )
				{
					LOG->UserLog( "Course file", sPath, "contains a fixed song entry \"%s\" that does not exist. "
						      "This entry will be ignored.", sSong.c_str());
					out.m_bIncomplete = true;
					continue; // skip this #SONG
				}
			}

			new_entry.stepsCriteria.m_difficulty = OldStyleStringToDifficulty( sParams[2] );
      //most CRS files use old-style difficulties, but Difficulty enum values can be used in SM5. Test for those too.
      if( new_entry.stepsCriteria.m_difficulty == Difficulty_Invalid )
        new_entry.stepsCriteria.m_difficulty = StringToDifficulty( sParams[2] );
			if( new_entry.stepsCriteria.m_difficulty == Difficulty_Invalid )
			{
				int retval = sscanf( sParams[2], "%d..%d", &new_entry.stepsCriteria.m_iLowMeter, &new_entry.stepsCriteria.m_iHighMeter );
				if( retval == 1 )
					new_entry.stepsCriteria.m_iHighMeter = new_entry.stepsCriteria.m_iLowMeter;
				else if( retval != 2 )
				{
					LOG->UserLog( "Course file", sPath, "contains an invalid difficulty setting: \"%s\", 3..6 used instead",
						      sParams[2].c_str() );
					new_entry.stepsCriteria.m_iLowMeter = 3;
					new_entry.stepsCriteria.m_iHighMeter = 6;
				}
				new_entry.stepsCriteria.m_iLowMeter = max( new_entry.stepsCriteria.m_iLowMeter, 1 );
				new_entry.stepsCriteria.m_iHighMeter = max( new_entry.stepsCriteria.m_iHighMeter, new_entry.stepsCriteria.m_iLowMeter );
			}

			{
				// If "showcourse" or "noshowcourse" is in the list, force
				// new_entry.secret on or off.
				vector<RString> mods;
				split( sParams[3], ",", mods, true );
				for( int j = (int) mods.size()-1; j >= 0 ; --j )
				{
					RString &sMod = mods[j];
					TrimLeft( sMod );
					TrimRight( sMod );
					if( !sMod.CompareNoCase("showcourse") )
						new_entry.bSecret = false;
					else if( !sMod.CompareNoCase("noshowcourse") )
						new_entry.bSecret = true;
					else if( !sMod.CompareNoCase("nodifficult") )
						new_entry.bNoDifficult = true;
					else if( sMod.length() > 5 && !sMod.Left(5).CompareNoCase("award") )
						new_entry.iGainLives = StringToInt( sMod.substr(5) );
					else
						continue;
					mods.erase( mods.begin() + j );
				}
				new_entry.sModifiers = join( ",", mods );
			}

			new_entry.attacks = attacks;
			new_entry.fGainSeconds = fGainSeconds;
			attacks.clear();

			out.m_vEntries.push_back( new_entry );
		}
		else if( !sValueName.EqualsNoCase("DISPLAYCOURSE") || !sValueName.EqualsNoCase("COMBO") ||
			 !sValueName.EqualsNoCase("COMBOMODE") )
		{
			// Ignore
		}

		else if( bFromCache && !sValueName.EqualsNoCase("RADAR") )
		{
			StepsType st = (StepsType) StringToInt(sParams[1]);
			CourseDifficulty cd = (CourseDifficulty) StringToInt( sParams[2] );

			RadarValues rv;
			rv.FromString( sParams[3] );
			out.m_RadarCache[Course::CacheEntry(st, cd)] = rv;
		}
		else if( sValueName.EqualsNoCase("STYLE") )
		{
			RString sStyles = sParams[1];
			vector<RString> asStyles;
			split( sStyles, ",", asStyles );
			for (RString const &s : asStyles)
				out.m_setStyles.insert( s );

		}
		else
		{
			LOG->UserLog( "Course file", sPath, "contains an unexpected value named \"%s\"", sValueName.c_str() );
		}
	}

	if( out.m_sBannerPath.empty() )
	{
		const RString sFName = SetExtension( out.m_sPath, "" );

		vector<RString> arrayPossibleBanners;
		GetDirListing( sFName + "*.png", arrayPossibleBanners, false, false );
		GetDirListing( sFName + "*.jpg", arrayPossibleBanners, false, false );
		GetDirListing( sFName + "*.jpeg", arrayPossibleBanners, false, false );
		GetDirListing( sFName + "*.bmp", arrayPossibleBanners, false, false );
		GetDirListing( sFName + "*.gif", arrayPossibleBanners, false, false );
		if( !arrayPossibleBanners.empty() )
			out.m_sBannerPath = arrayPossibleBanners[0];
	}

	static TitleSubst tsub("Courses");

	TitleFields title;
	title.Title = out.m_sMainTitle;
	title.TitleTranslit = out.m_sMainTitleTranslit;
	tsub.Subst( title );
	out.m_sMainTitle = title.Title;
	out.m_sMainTitleTranslit = title.TitleTranslit;

	/* Cache and load the course banner. Only bother doing this if at least one
	 * song was found in the course. */
	if( out.m_vEntries.empty() )
		return true;
	if( out.m_sBannerPath != "" )
		IMAGECACHE->CacheImage( "Banner", out.GetBannerPath() );

	/* Cache each trail RadarValues that's slow to load, so we
	 * don't have to do it at runtime. */
	if( !bFromCache )
		out.CalculateRadarValues();

	return true;
}

bool CourseLoaderCRS::LoadFromCRSFile( const RString &_sPath, Course &out )
{
	RString sPath = _sPath;

	out.Init();

	out.m_sPath = sPath; // save path

	// save group name
	{
		vector<RString> parts;
		split( sPath, "/", parts, false );
		if( parts.size() >= 4 ) // e.g. "/Courses/blah/fun.crs"
			out.m_sGroupName = parts[parts.size()-2];
	}


	bool bUseCache = true;
	{
		/* First look in the cache for this course. Don't bother honoring
		 * FastLoad for checking the course hash, since courses are normally
		 * grouped into a few directories, not one directory per course. */
		unsigned uHash = SONGINDEX->GetCacheHash( out.m_sPath );
		if( !DoesFileExist(out.GetCacheFilePath()) )
			bUseCache = false;
		// XXX: if !FastLoad, regen cache if the used songs have changed
		if( !PREFSMAN->m_bFastLoad && GetHashForFile(out.m_sPath) != uHash )
			bUseCache = false; // this cache is out of date 
	}

	if( bUseCache )
	{
		RString sCacheFile = out.GetCacheFilePath();
		LOG->Trace( "CourseLoaderCRS::LoadFromCRSFile(\"%s\") (\"%s\")", sPath.c_str(), sCacheFile.c_str() );
		sPath = sCacheFile;
	}
	else
	{
		LOG->Trace( "CourseLoaderCRS::LoadFromCRSFile(\"%s\")", sPath.c_str() );
	}

	MsdFile msd;
	if( !msd.ReadFile( sPath, false ) ) // don't unescape
	{
		LOG->UserLog( "Course file", sPath, "couldn't be opened: %s.", msd.GetError().c_str() );
		return false;
	}
	
	if( !LoadFromMsd(sPath, msd, out, bUseCache) )
		return false;

	if( !bUseCache )
	{
		// If we have any cache data, write the cache file.
		if( out.m_RadarCache.size() )
		{
			RString sCachePath = out.GetCacheFilePath();
			if( CourseWriterCRS::Write(out, sCachePath, true) )
				SONGINDEX->AddCacheIndex( out.m_sPath, GetHashForFile(out.m_sPath) );
		}
	}

	return true;
}

bool CourseLoaderCRS::LoadEditFromFile( const RString &sEditFilePath, ProfileSlot slot )
{
	LOG->Trace( "CourseLoaderCRS::LoadEdit(%s)", sEditFilePath.c_str() );

	int iBytes = FILEMAN->GetFileSizeInBytes( sEditFilePath );
	if( iBytes > MAX_EDIT_COURSE_SIZE_BYTES )
	{
		LOG->UserLog( "Edit file", sEditFilePath, "is unreasonably large. It won't be loaded." );
		return false;
	}

	MsdFile msd;
	if( !msd.ReadFile( sEditFilePath, false ) )  // don't unescape
	{
		LOG->UserLog( "Edit file", sEditFilePath, "couldn't be opened: %s", msd.GetError().c_str() );
		return false;
	}
	Course *pCourse = new Course;

	pCourse->m_sPath = sEditFilePath;
	LoadFromMsd( sEditFilePath, msd, *pCourse, true );

	pCourse->m_LoadedFromProfile = slot;

	SONGMAN->AddCourse( pCourse );
	return true;
}

bool CourseLoaderCRS::LoadEditFromBuffer( const RString &sBuffer, const RString &sPath, ProfileSlot slot )
{
	Course *pCourse = new Course;
	if( !LoadFromBuffer(sPath, sBuffer, *pCourse) )
	{
		delete pCourse;
		return false;
	}

	pCourse->m_LoadedFromProfile = slot;

	SONGMAN->AddCourse( pCourse );
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
