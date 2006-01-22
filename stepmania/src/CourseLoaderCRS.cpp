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
#include "BannerCache.h"
#include "RageFileManager.h"
#include "Profile.h"
#include "CourseWriterCRS.h"

const int MAX_EDIT_COURSE_SIZE_BYTES	= 30*1024;	// 30KB

bool CourseLoaderCRS::LoadFromBuffer( const RString &sPath, const RString &sBuffer, Course &out )
{
	MsdFile msd;
	msd.ReadFromString( sBuffer );
	return LoadFromMsd( sPath, msd, out, true );
}

bool CourseLoaderCRS::LoadFromMsd( const RString &sPath, const MsdFile &msd, Course &out, bool bFromCache )
{
	const RString sFName = SetExtension( out.m_sPath, "" );

	vector<RString> arrayPossibleBanners;
	GetDirListing( sFName + "*.png", arrayPossibleBanners, false, true );
	GetDirListing( sFName + "*.jpg", arrayPossibleBanners, false, true );
	GetDirListing( sFName + "*.bmp", arrayPossibleBanners, false, true );
	GetDirListing( sFName + "*.gif", arrayPossibleBanners, false, true );
	if( !arrayPossibleBanners.empty() )
		out.m_sBannerPath = arrayPossibleBanners[0];

	AttackArray attacks;
	float fGainSeconds = 0;
	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		RString sValueName = msd.GetParam(i, 0);
		const MsdFile::value_t &sParams = msd.GetValue(i);

		// handle the data
		if( 0 == stricmp(sValueName, "COURSE") )
			out.m_sMainTitle = sParams[1];
		else if( 0 == stricmp(sValueName, "COURSETRANSLIT") )
			out.m_sMainTitleTranslit = sParams[1];
		else if( 0 == stricmp(sValueName, "REPEAT") )
		{
			RString str = sParams[1];
			str.MakeLower();
			if( str.find("yes") != string::npos )
				out.m_bRepeat = true;
		}

		else if( 0 == stricmp(sValueName, "LIVES") )
			out.m_iLives = atoi( sParams[1] );

		else if( 0 == stricmp(sValueName, "GAINSECONDS") )
			fGainSeconds = strtof( sParams[1], NULL );

		else if( 0 == stricmp(sValueName, "METER") )
		{
			if( sParams.params.size() == 2 )
			{
				out.m_iCustomMeter[DIFFICULTY_MEDIUM] = atoi( sParams[1] ); /* compat */
			}
			else if( sParams.params.size() == 3 )
			{
				const CourseDifficulty cd = StringToCourseDifficulty( sParams[1] );
				if( cd == DIFFICULTY_INVALID )
				{
					LOG->Warn( "Course file '%s' contains an invalid #METER string: \"%s\"",
								sPath.c_str(), sParams[1].c_str() );
					continue;
				}
				out.m_iCustomMeter[cd] = atoi( sParams[2] );
			}
		}

		else if( 0 == stricmp(sValueName, "MODS") )
		{
			Attack attack;
			float end = -9999;
			for( unsigned j = 1; j < sParams.params.size(); ++j )
			{
				vector<RString> sBits;
				split( sParams[j], "=", sBits, false );
				if( sBits.size() < 2 )
					continue;

				TrimLeft( sBits[0] );
				TrimRight( sBits[0] );
				if( !sBits[0].CompareNoCase("TIME") )
					attack.fStartSecond = strtof( sBits[1], NULL );
				else if( !sBits[0].CompareNoCase("LEN") )
					attack.fSecsRemaining = strtof( sBits[1], NULL );
				else if( !sBits[0].CompareNoCase("END") )
					end = strtof( sBits[1], NULL );
				else if( !sBits[0].CompareNoCase("MODS") )
				{
					attack.sModifiers = sBits[1];
					if( end != -9999 )
					{
						ASSERT_M( end >= attack.fStartSecond, ssprintf("Attack ends before it starts.  end %f, start %f", end, attack.fStartSecond) );
						attack.fSecsRemaining = end - attack.fStartSecond;
						end = -9999;
					}
					
					// warn on invalid so we catch bogus mods on load
					PlayerOptions po;
					po.FromString( attack.sModifiers, true );

					attacks.push_back( attack );
				}
				else
				{
					LOG->Warn( "Unexpected value named '%s'", sBits[0].c_str() );
				}
			}

				
		}
		else if( 0 == stricmp(sValueName, "SONG") )
		{
			CourseEntry new_entry;

			// infer entry::Type from the first param
			if( sParams[1].Left(strlen("BEST")) == "BEST" )
			{
				new_entry.iChooseIndex = atoi( sParams[1].Right(sParams[1].size()-strlen("BEST")) ) - 1;
				CLAMP( new_entry.iChooseIndex, 0, 500 );
				new_entry.songSort = SongSort_MostPlays;
			}
			else if( sParams[1].Left(strlen("WORST")) == "WORST" )
			{
				new_entry.iChooseIndex = atoi( sParams[1].Right(sParams[1].size()-strlen("WORST")) ) - 1;
				CLAMP( new_entry.iChooseIndex, 0, 500 );
				new_entry.songSort = SongSort_FewestPlays;
			}
			else if( sParams[1] == "*" )
			{
				//new_entry.bSecret = true;
			}
			else if( sParams[1].Right(1) == "*" )
			{
				//new_entry.bSecret = true;
				RString sSong = sParams[1];
				sSong.Replace( "\\", "/" );
				vector<RString> bits;
				split( sSong, "/", bits );
				if( bits.size() == 2 )
				{
					new_entry.sSongGroup = bits[0];
				}
				else
				{
					LOG->Warn( "Course file '%s' contains a random_within_group entry '%s' that is invalid. "
								"Song should be in the format '<group>/*'.",
								sPath.c_str(), sSong.c_str());
				}

				if( !SONGMAN->DoesSongGroupExist(new_entry.sSongGroup) )
				{
					/* XXX: We need a place to put "user warnings".  This is too loud for info.txt--
				     * it obscures important warnings--and regular users never look there, anyway. */
					LOG->Trace( "Course file '%s' random_within_group entry '%s' specifies a group that doesn't exist. "
								"This entry will be ignored.",
								sPath.c_str(), sSong.c_str());
					continue;	// skip this #SONG
				}
			}
			else
			{
				RString sSong = sParams[1];
				new_entry.pSong = SONGMAN->FindSong( sSong );

				if( new_entry.pSong == NULL )
				{
					/* XXX: We need a place to put "user warnings".  This is too loud for info.txt--
				     * it obscures important warnings--and regular users never look there, anyway. */
					LOG->Trace( "Course file '%s' contains a fixed song entry '%s' that does not exist. "
								"This entry will be ignored.",
								sPath.c_str(), sSong.c_str());
					continue;	// skip this #SONG
				}
			}

			new_entry.baseDifficulty = StringToDifficulty( sParams[2] );
			if( new_entry.baseDifficulty == DIFFICULTY_INVALID )
			{
				int retval = sscanf( sParams[2], "%d..%d", &new_entry.iLowMeter, &new_entry.iHighMeter );
				if( retval == 1 )
					new_entry.iHighMeter = new_entry.iLowMeter;
				else if( retval != 2 )
				{
					LOG->Warn("Course file '%s' contains an invalid difficulty setting: \"%s\", 3..6 used instead",
						sPath.c_str(), sParams[2].c_str());
					new_entry.iLowMeter = 3;
					new_entry.iHighMeter = 6;
				}
			}

			{
				/* If "showcourse" or "noshowcourse" is in the list, force new_entry.secret 
				 * on or off. */
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
					else 
						continue;
					mods.erase(mods.begin() + j);
				}
				new_entry.sModifiers = join( ",", mods );
			}

			new_entry.attacks = attacks;
			new_entry.fGainSeconds = fGainSeconds;
			attacks.clear();
			
			out.m_vEntries.push_back( new_entry );
		}
		else if( bFromCache && !stricmp(sValueName, "RADAR") )
		{
			StepsType st = (StepsType) atoi(sParams[1]);
			CourseDifficulty cd = (CourseDifficulty) atoi(sParams[2]);

			RadarValues rv;
			rv.FromString( sParams[3] );
			out.m_RadarCache[Course::CacheEntry(st, cd)] = rv;
		}
		else
		{
			LOG->Warn( "Unexpected value named '%s'", sValueName.c_str() );
		}
	}
	static TitleSubst tsub("Courses");

	TitleFields title;
	title.Title = out.m_sMainTitle;
	title.TitleTranslit = out.m_sMainTitleTranslit;
	tsub.Subst( title );
	out.m_sMainTitle = title.Title;
	out.m_sMainTitleTranslit = title.TitleTranslit;

	/* Cache and load the course banner.  Only bother doing this if at least one
	 * song was found in the course. */
	if( out.m_sBannerPath != "" && !out.m_vEntries.empty() )
		BANNERCACHE->CacheBanner( out.m_sBannerPath );

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

	out.m_sPath = sPath;	// save path

	// save group name
	{
		vector<RString> parts;
		split( sPath, "/", parts, false );
		if( parts.size() >= 4 )		// e.g. "/Courses/blah/fun.cvs"
			out.m_sGroupName = parts[parts.size()-2];
	}


	bool bUseCache = true;
	{
		/* First look in the cache for this course.  Don't bother
		 * honoring FastLoad for checking the course hash, since
		 * courses are normally grouped into a few directories, not
		 * one directory per course. XXX: if !FastLoad, regen
		 * cache if the used songs have changed */
		unsigned uHash = SONGINDEX->GetCacheHash( out.m_sPath );
		if( !DoesFileExist(out.GetCacheFilePath()) )
			bUseCache = false;
		if( !PREFSMAN->m_bFastLoad && GetHashForDirectory(out.m_sPath) != uHash )
			bUseCache = false; // this cache is out of date 
	}

	if( bUseCache )
	{
		RString sCacheFile = out.GetCacheFilePath();
		LOG->Trace( "CourseLoaderCRS::LoadFromCRSFile(\"%s\") (\"%s\")", sPath.c_str(), sCacheFile.c_str() );
		sPath = sCacheFile.c_str();
	}
	else
	{
		LOG->Trace( "CourseLoaderCRS::LoadFromCRSFile(\"%s\")", sPath.c_str() );
	}

	MsdFile msd;
	if( !msd.ReadFile(sPath) )
		RageException::Throw( "Error opening CRS file '%s'.", sPath.c_str() );
	
	if( !LoadFromMsd(sPath, msd, out, bUseCache) )
		return false;

	if( !bUseCache )
	{
		/* If we have any cache data, write the cache file. */
		if( out.m_RadarCache.size() )
		{
			RString sCachePath = out.GetCacheFilePath();
			CourseWriterCRS::Write( out, sCachePath, true );

			SONGINDEX->AddCacheIndex( out.m_sPath, GetHashForFile(out.m_sPath) );
		}
	}

	return true;
}

bool CourseLoaderCRS::LoadEdit( const RString &sEditFilePath, ProfileSlot slot )
{
	LOG->Trace( "CourseLoaderCRS::LoadEdit(%s)", sEditFilePath.c_str() );

	int iBytes = FILEMAN->GetFileSizeInBytes( sEditFilePath );
	if( iBytes > MAX_EDIT_COURSE_SIZE_BYTES )
	{
		LOG->Warn( "The edit '%s' is unreasonably large.  It won't be loaded.", sEditFilePath.c_str() );
		return false;
	}

	MsdFile msd;
	if( !msd.ReadFile( sEditFilePath ) )
	{
		LOG->Warn( "Error opening edit file \"%s\": %s", sEditFilePath.c_str(), msd.GetError().c_str() );
		return false;
	}
	Course *pCourse = new Course;
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
