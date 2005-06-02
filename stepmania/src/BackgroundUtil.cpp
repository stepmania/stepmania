#include "global.h"
#include "BackgroundUtil.h"
#include "RageUtil.h"
#include "song.h"
#include "Foreach.h"
#include "IniFile.h"
#include "RageLog.h"
#include <set>

const CString BACKGROUND_EFFECTS_DIR =		"BackgroundEffects/";
const CString BACKGROUND_TRANSITIONS_DIR =	"BackgroundTransitions/";
const CString BG_ANIMS_DIR			= "BGAnimations/";
const CString VISUALIZATIONS_DIR	= "Visualizations/";
const CString RANDOMMOVIES_DIR		= "RandomMovies/";

static void StripCvs( vector<CString> &vsPathsToStrip, vector<CString> &vsNamesToStrip )
{
	ASSERT( vsPathsToStrip.size() == vsNamesToStrip.size() );
	for( unsigned i=0; i<vsNamesToStrip.size(); i++ )
	{
		if( vsNamesToStrip[i].Right(3).CompareNoCase("CVS") == 0 )
		{
			vsPathsToStrip.erase( vsPathsToStrip.begin()+i );
			vsNamesToStrip.erase( vsNamesToStrip.begin()+i );
		}
	}
}

int CompareBackgroundChanges(const BackgroundChange &seg1, const BackgroundChange &seg2)
{
	return seg1.m_fStartBeat < seg2.m_fStartBeat;
}

void BackgroundUtil::SortBackgroundChangesArray( vector<BackgroundChange> &vBackgroundChanges )
{
	sort( vBackgroundChanges.begin(), vBackgroundChanges.end(), CompareBackgroundChanges );
}

void BackgroundUtil::GetBackgroundEffects( const CString &_sName, vector<CString> &vsPathsOut, vector<CString> &vsNamesOut )
{
	CString sName = _sName;
	if( sName == "" )
		sName = "*";

	vsPathsOut.clear();
	GetDirListing( BACKGROUND_EFFECTS_DIR+sName+".xml", vsPathsOut, false, true );
	
	vsNamesOut.clear();
	FOREACH_CONST( CString, vsPathsOut, s )
		vsNamesOut.push_back( GetFileNameWithoutExtension(*s) );

	StripCvs( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetBackgroundTransitions( const CString &_sName, vector<CString> &vsPathsOut, vector<CString> &vsNamesOut )
{
	CString sName = _sName;
	if( sName == "" )
		sName = "*";

	vsPathsOut.clear();
	GetDirListing( BACKGROUND_TRANSITIONS_DIR+sName+".xml", vsPathsOut, false, true );
	
	vsNamesOut.clear();
	FOREACH_CONST( CString, vsPathsOut, s )
		vsNamesOut.push_back( GetFileNameWithoutExtension(*s) );

	StripCvs( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetSongBGAnimations( const Song *pSong, const CString &sMatch, vector<CString> &vsPathsOut, vector<CString> &vsNamesOut )
{
	vsPathsOut.clear();
	if( sMatch.empty() )
	{
		GetDirListing( pSong->GetSongDir()+sMatch+"*",	vsPathsOut, true, true );
	}
	else
	{
		GetDirListing( pSong->GetSongDir()+sMatch,		vsPathsOut, true, true );
	}
	
	vsNamesOut.clear();
	FOREACH_CONST( CString, vsPathsOut, s )
		vsNamesOut.push_back( Basename(*s) );

	StripCvs( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetSongMovies( const Song *pSong, const CString &sMatch, vector<CString> &vsPathsOut, vector<CString> &vsNamesOut )
{
	vsPathsOut.clear();
	if( sMatch.empty() )
	{
		GetDirListing( pSong->GetSongDir()+sMatch+"*.avi",	vsPathsOut, false, true );
		GetDirListing( pSong->GetSongDir()+sMatch+"*.mpg",	vsPathsOut, false, true );
		GetDirListing( pSong->GetSongDir()+sMatch+"*.mpeg", vsPathsOut, false, true );
	}
	else
	{
		GetDirListing( pSong->GetSongDir()+sMatch,			vsPathsOut, false, true );
	}

	vsNamesOut.clear();
	FOREACH_CONST( CString, vsPathsOut, s )
		vsNamesOut.push_back( Basename(*s) );

	StripCvs( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetSongBitmaps( const Song *pSong, const CString &sMatch, vector<CString> &vsPathsOut, vector<CString> &vsNamesOut )
{
	vsPathsOut.clear();
	if( sMatch.empty() )
	{
		GetDirListing( pSong->GetSongDir()+sMatch+"*.png",	vsPathsOut, false, true );
		GetDirListing( pSong->GetSongDir()+sMatch+"*.jpg",	vsPathsOut, false, true );
		GetDirListing( pSong->GetSongDir()+sMatch+"*.gif",	vsPathsOut, false, true );
		GetDirListing( pSong->GetSongDir()+sMatch+"*.bmp",	vsPathsOut, false, true );
	}
	else
	{
		GetDirListing( pSong->GetSongDir()+sMatch,			vsPathsOut, false, true );
	}

	vsNamesOut.clear();
	FOREACH_CONST( CString, vsPathsOut, s )
		vsNamesOut.push_back( Basename(*s) );

	StripCvs( vsPathsOut, vsNamesOut );
}

static void GetFilterToFileNames( const CString sBaseDir, const Song *pSong, set<CString> &vsPossibleFileNamesOut )
{
	vsPossibleFileNamesOut.clear();

	if( pSong->m_sGenre.empty() )
		return;

	ASSERT( !pSong->m_sGroupName.empty() )
	IniFile ini;
	CString sPath = sBaseDir+pSong->m_sGroupName+"/"+"BackgroundMapping.ini";
	ini.ReadFile( sPath );
	
	CString sSection;
	bool bSuccess = ini.GetValue( "GenreToSection", pSong->m_sGenre, sSection );
	if( !bSuccess )
	{
		LOG->Warn( "Genre '%s' isn't mapped", pSong->m_sGenre.c_str() );
		return;
	}

	XNode *pSection = ini.GetChild( sSection );
	if( pSection == NULL )
	{
		ASSERT_M( 0, ssprintf("File '%s' refers to a section '%s' that is missing.", sPath.c_str(), sSection.c_str()) );
		return;
	}

	FOREACH_CONST_Attr( pSection, p )
		vsPossibleFileNamesOut.insert( p->m_sName );
}

void BackgroundUtil::GetGlobalBGAnimations( const Song *pSong, const CString &sMatch, vector<CString> &vsPathsOut, vector<CString> &vsNamesOut )
{
	vsPathsOut.clear();
	GetDirListing( BG_ANIMS_DIR+sMatch+"*", vsPathsOut, true, true );

	vsNamesOut.clear();
	FOREACH_CONST( CString, vsPathsOut, s )
		vsNamesOut.push_back( Basename(*s) );

	StripCvs( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetGlobalRandomMovies( const Song *pSong, const CString &sMatch, vector<CString> &vsPathsOut, vector<CString> &vsNamesOut )
{
	vsPathsOut.clear();
	vsNamesOut.clear();

	CString sSongGroup;
	set<CString> ssFilterToFileNames;
	if( pSong  &&  !pSong->m_sGenre.empty() )
		GetFilterToFileNames( RANDOMMOVIES_DIR, pSong, ssFilterToFileNames );

	vector<CString> vsDirsToTry;
	if( !sSongGroup.empty() )
		vsDirsToTry.push_back( RANDOMMOVIES_DIR+sSongGroup+"/" );
	vsDirsToTry.push_back( RANDOMMOVIES_DIR );

	FOREACH_CONST( CString, vsDirsToTry, sDir )
	{
		if( sMatch.empty() )
		{
			GetDirListingRecursive( *sDir, "*.avi", vsPathsOut );
			GetDirListingRecursive( *sDir, "*.mpg", vsPathsOut );
			GetDirListingRecursive( *sDir, "*.mpeg", vsPathsOut );
		}
		else
		{
			GetDirListing( RANDOMMOVIES_DIR+sMatch, vsPathsOut, false, true );
		}

		if( !ssFilterToFileNames.empty() )
		{
			for( unsigned i=0; i<vsPathsOut.size(); i++ )
			{
				CString sBasename = Basename( vsPathsOut[i] );
				bool bFound = ssFilterToFileNames.find(sBasename) != ssFilterToFileNames.end();
				if( !bFound )
				{
					vsPathsOut.erase( vsPathsOut.begin()+i );
					i--;
				}
			}
		}

		FOREACH_CONST( CString, vsPathsOut, s )
		{
			CString sName = s->Right( s->size() - RANDOMMOVIES_DIR.size() - 1 );
			vsNamesOut.push_back( sName );
		}

		if( !vsPathsOut.empty() )
			break;
	}

	StripCvs( vsPathsOut, vsNamesOut );
}

/*
 * (c) 2001-2004 Chris Danford, Ben Nordstrom
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
