#include "global.h"
#include "BackgroundUtil.h"
#include "RageUtil.h"
#include "Song.h"
#include "IniFile.h"
#include "RageLog.h"
#include <set>
#include "Background.h"
#include "RageFileManager.h"
#include "ActorUtil.h"

using std::vector;

bool BackgroundDef::operator<( const BackgroundDef &other ) const
{
#define COMPARE(x) if( x < other.x ) return true; else if( x > other.x ) return false;
	COMPARE( m_sEffect );
	COMPARE( m_sFile1 );
	COMPARE( m_sFile2 );
	COMPARE( m_sColor1 );
	COMPARE( m_sColor2 );
#undef COMPARE
	return false;
}

bool BackgroundDef::operator==( const BackgroundDef &other ) const
{
	return
		m_sEffect == other.m_sEffect &&
		m_sFile1 == other.m_sFile1 &&
		m_sFile2 == other.m_sFile2 &&
		m_sColor1 == other.m_sColor1 &&
		m_sColor2 == other.m_sColor2;
}

XNode *BackgroundDef::CreateNode() const
{
	XNode* pNode = new XNode( "BackgroundDef" );

	if( !m_sEffect.empty() )
		pNode->AppendAttr( "Effect", m_sEffect );
	if( !m_sFile1.empty() )
		pNode->AppendAttr( "File1", m_sFile1 );
	if( !m_sFile2.empty() )
		pNode->AppendAttr( "File2", m_sFile2 );
	if( !m_sColor1.empty() )
		pNode->AppendAttr( "Color1", m_sColor1 );
	if( !m_sColor2.empty() )
		pNode->AppendAttr( "Color2", m_sColor2 );

	return pNode;
}


std::string BackgroundChange::GetTextDescription() const
{
	vector<std::string> vsParts;
	if( !m_def.m_sFile1.empty() )
	{
		vsParts.push_back( m_def.m_sFile1 );
	}
	if( !m_def.m_sFile2.empty() )
	{
		vsParts.push_back( m_def.m_sFile2 );
	}
	if( m_fRate!=1.0f )
	{
		vsParts.push_back( fmt::sprintf("%.2f%%",m_fRate*100) );
	}
	if( !m_sTransition.empty() )
	{
		vsParts.push_back( m_sTransition );
	}
	if( !m_def.m_sEffect.empty() )
	{
		vsParts.push_back( m_def.m_sEffect );
	}
	if( !m_def.m_sColor1.empty() )
	{
		vsParts.push_back( m_def.m_sColor1 );
	}
	if( !m_def.m_sColor2.empty() )
	{
		vsParts.push_back( m_def.m_sColor2 );
	}
	if( vsParts.empty() )
	{
		vsParts.push_back( "(empty)" );
	}
	return Rage::join("\n", vsParts);
}

std::string BackgroundChange::ToString() const
{
	/* TODO:  Technically we need to double-escape the filename
	 * (because it might contain '=') and then unescape the value
	 * returned by the MsdFile. */
	return fmt::sprintf("%.3f=%s=%.3f=%d=%d=%d=%s=%s=%s=%s=%s",
			this->m_fStartBeat,
			SmEscape(this->m_def.m_sFile1).c_str(),
			this->m_fRate,
			this->m_sTransition == SBT_CrossFade,		// backward compat
			this->m_def.m_sEffect == SBE_StretchRewind, 	// backward compat
			this->m_def.m_sEffect != SBE_StretchNoLoop, 	// backward compat
			this->m_def.m_sEffect.c_str(),
			this->m_def.m_sFile2.c_str(),
			this->m_sTransition.c_str(),
			SmEscape(Rage::Color::NormalizeColorString(this->m_def.m_sColor1)).c_str(),
			SmEscape(Rage::Color::NormalizeColorString(this->m_def.m_sColor2)).c_str());
}


const std::string BACKGROUND_EFFECTS_DIR =		"BackgroundEffects/";
const std::string BACKGROUND_TRANSITIONS_DIR =	"BackgroundTransitions/";
const std::string BG_ANIMS_DIR			= "BGAnimations/";
const std::string VISUALIZATIONS_DIR	= "Visualizations/";
const std::string RANDOMMOVIES_DIR		= "RandomMovies/";
const std::string SONG_MOVIES_DIR		= "SongMovies/";

const std::string RANDOM_BACKGROUND_FILE    = "-random-";
const std::string NO_SONG_BG_FILE           = "-nosongbg-";
const std::string SONG_BACKGROUND_FILE      = "songbackground";

const std::string SBE_UpperLeft             = "UpperLeft";
const std::string SBE_Centered              = "Centered";
const std::string SBE_StretchNormal         = "StretchNormal";
const std::string SBE_StretchNoLoop         = "StretchNoLoop";
const std::string SBE_StretchRewind         = "StretchRewind";
const std::string SBT_CrossFade             = "CrossFade";

static void StripCvsAndSvn( vector<std::string> &vsPathsToStrip, vector<std::string> &vsNamesToStrip )
{
	ASSERT( vsPathsToStrip.size() == vsNamesToStrip.size() );
	Rage::ci_ascii_string cvs{ "CVS" };
	Rage::ci_ascii_string svn{ ".svn" };
	for( unsigned i=0; i<vsNamesToStrip.size(); i++ )
	{
		std::string ext{ Rage::tail(vsNamesToStrip[i], 3) };
		if (cvs == ext || svn == vsNamesToStrip[i])
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

void BackgroundUtil::AddBackgroundChange( vector<BackgroundChange> &vBackgroundChanges, BackgroundChange seg )
{
	vector<BackgroundChange>::iterator it;
	it = upper_bound( vBackgroundChanges.begin(), vBackgroundChanges.end(), seg, CompareBackgroundChanges );
	vBackgroundChanges.insert( it, seg );
}

void BackgroundUtil::GetBackgroundEffects( std::string const &_sName, vector<std::string> &vsPathsOut, vector<std::string> &vsNamesOut )
{
	std::string sName = _sName;
	if( sName == "" )
	{
		sName = "*";
	}
	vsPathsOut.clear();
	GetDirListing( BACKGROUND_EFFECTS_DIR+sName+".lua", vsPathsOut, false, true );

	vsNamesOut.clear();
	for (auto const &s: vsPathsOut)
	{
		vsNamesOut.push_back( GetFileNameWithoutExtension(s) );
	}

	StripCvsAndSvn( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetBackgroundTransitions( std::string const &_sName, vector<std::string> &vsPathsOut, vector<std::string> &vsNamesOut )
{
	std::string sName = _sName;
	if( sName == "" )
	{
		sName = "*";
	}
	vsPathsOut.clear();
	if( true )
	{
		GetDirListing( BACKGROUND_TRANSITIONS_DIR+sName+".xml", vsPathsOut, false, true );
	}
	GetDirListing( BACKGROUND_TRANSITIONS_DIR+sName+".lua", vsPathsOut, false, true );

	vsNamesOut.clear();
	for (auto const &s: vsPathsOut)
	{
		vsNamesOut.push_back( GetFileNameWithoutExtension(s) );
	}

	StripCvsAndSvn( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetSongBGAnimations( const Song *pSong, std::string const &sMatch, vector<std::string> &vsPathsOut, vector<std::string> &vsNamesOut )
{
	vsPathsOut.clear();
	if( sMatch.empty() )
	{
		GetDirListing( pSong->GetSongDir()+"*",    vsPathsOut, true, true );
	}
	else
	{
		GetDirListing( pSong->GetSongDir()+sMatch, vsPathsOut, true, true );
	}

	vsNamesOut.clear();
	for (auto const &s: vsPathsOut)
	{
		vsNamesOut.push_back( Rage::base_name(s) );
	}

	StripCvsAndSvn( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetSongMovies( const Song *pSong, std::string const &sMatch, vector<std::string> &vsPathsOut, vector<std::string> &vsNamesOut )
{
	vsPathsOut.clear();
	if( sMatch.empty() )
	{
		FILEMAN->GetDirListingWithMultipleExtensions(pSong->GetSongDir()+sMatch,
			ActorUtil::GetTypeExtensionList(FT_Movie), vsPathsOut, false, true);
	}
	else
	{
		GetDirListing( pSong->GetSongDir()+sMatch,			vsPathsOut, false, true );
	}

	vsNamesOut.clear();
	for (auto const &s: vsPathsOut)
	{
		vsNamesOut.push_back( Rage::base_name(s) );
	}

	StripCvsAndSvn( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetSongBitmaps( const Song *pSong, std::string const &sMatch, vector<std::string> &vsPathsOut, vector<std::string> &vsNamesOut )
{
	vsPathsOut.clear();
	if( sMatch.empty() )
	{
		FILEMAN->GetDirListingWithMultipleExtensions(pSong->GetSongDir()+sMatch,
			ActorUtil::GetTypeExtensionList(FT_Bitmap), vsPathsOut, false, true);
	}
	else
	{
		GetDirListing( pSong->GetSongDir()+sMatch,			vsPathsOut, false, true );
	}

	vsNamesOut.clear();
	for (auto const &s: vsPathsOut)
	{
		vsNamesOut.push_back( Rage::base_name(s) );
	}

	StripCvsAndSvn( vsPathsOut, vsNamesOut );
}

static void GetFilterToFileNames( const std::string sBaseDir, const Song *pSong, std::set<std::string> &vsPossibleFileNamesOut )
{
	vsPossibleFileNamesOut.clear();

	if( pSong->m_sGenre.empty() )
		return;

	ASSERT( !pSong->m_sGroupName.empty() );
	IniFile ini;
	std::string sPath = sBaseDir+pSong->m_sGroupName+"/"+"BackgroundMapping.ini";
	ini.ReadFile( sPath );

	std::string sSection;
	bool bSuccess = ini.GetValue( "GenreToSection", pSong->m_sGenre, sSection );
	if( !bSuccess )
	{
		//LOG->Warn( "Genre '%s' isn't mapped", pSong->m_sGenre.c_str() );
		return;
	}

	XNode *pSection = ini.GetChild( sSection );
	if( pSection == nullptr )
	{
		ASSERT_M( 0, fmt::sprintf("File '%s' refers to a section '%s' that is missing.", sPath.c_str(), sSection.c_str()) );
		return;
	}

	for (auto const &p: pSection->m_attrs)
	{
		vsPossibleFileNamesOut.insert( p.first );
	}
}

void BackgroundUtil::GetGlobalBGAnimations( const Song *pSong, std::string const &sMatch, vector<std::string> &vsPathsOut, vector<std::string> &vsNamesOut )
{
	vsPathsOut.clear();
	GetDirListing( BG_ANIMS_DIR+sMatch+"*", vsPathsOut, true, true );
	if( true )
		GetDirListing( BG_ANIMS_DIR+sMatch+"*.xml", vsPathsOut, false, true );

	vsNamesOut.clear();
	for (auto const &s: vsPathsOut)
	{
		vsNamesOut.push_back( Rage::base_name(s) );
	}

	StripCvsAndSvn( vsPathsOut, vsNamesOut );
}

namespace {
	/**
	 * @brief Fetches the appropriate path(s) for global random movies.
	 */
	void GetGlobalRandomMoviePaths(
		const Song *pSong,
		const std::string &sMatch,
		vector<std::string> &vsPathsOut,
		bool bTryInsideOfSongGroupAndGenreFirst,
		bool bTryInsideOfSongGroupFirst )
	{
		// Check for an exact match
		if( !sMatch.empty() )
		{
			GetDirListing( SONG_MOVIES_DIR+pSong->m_sGroupName+"/"+sMatch, vsPathsOut, false, true );	// search in SongMovies/SongGroupName/ first
			GetDirListing( SONG_MOVIES_DIR+sMatch, vsPathsOut, false, true );
			GetDirListing( RANDOMMOVIES_DIR+sMatch, vsPathsOut, false, true );
			if( vsPathsOut.empty() && sMatch != NO_SONG_BG_FILE )
			{
				LOG->Warn( "Background missing: %s", sMatch.c_str() );
			}
			return;
		}

		// Search for the most appropriate background
		std::set<std::string> ssFileNameWhitelist;
		if( bTryInsideOfSongGroupAndGenreFirst  &&  pSong  &&  !pSong->m_sGenre.empty() )
			GetFilterToFileNames( RANDOMMOVIES_DIR, pSong, ssFileNameWhitelist );

		vector<std::string> vsDirsToTry;
		if( bTryInsideOfSongGroupFirst && pSong )
		{
			ASSERT( !pSong->m_sGroupName.empty() );
			vsDirsToTry.push_back( RANDOMMOVIES_DIR+pSong->m_sGroupName+"/" );
		}
		vsDirsToTry.push_back( RANDOMMOVIES_DIR );

		for (auto const &sDir: vsDirsToTry)
		{
			GetDirListing( sDir+"*.ogv", vsPathsOut, false, true );
			GetDirListing( sDir+"*.avi", vsPathsOut, false, true );
			GetDirListing( sDir+"*.mpg", vsPathsOut, false, true );
			GetDirListing( sDir+"*.mpeg", vsPathsOut, false, true );

			if( !ssFileNameWhitelist.empty() )
			{
				vector<std::string> vsMatches;
				for (auto const &s: vsPathsOut)
				{
					auto sBasename = Rage::base_name( s );
					bool bFound = ssFileNameWhitelist.find(sBasename) != ssFileNameWhitelist.end();
					if( bFound )
					{
						vsMatches.push_back(s);
					}
				}
				// If we found any that match the whitelist, use only them.
				// If none match the whitelist, ignore the whitelist..
				if( !vsMatches.empty() )
					vsPathsOut = vsMatches;
			}

			if( !vsPathsOut.empty() )
			{
				// Return only the first directory found
				return;
			}
		}
	}

}

void BackgroundUtil::GetGlobalRandomMovies(
	const Song *pSong,
	std::string const &sMatch,
	vector<std::string> &vsPathsOut,
	vector<std::string> &vsNamesOut,
	bool bTryInsideOfSongGroupAndGenreFirst,
	bool bTryInsideOfSongGroupFirst )
{
	vsPathsOut.clear();
	vsNamesOut.clear();

	GetGlobalRandomMoviePaths( pSong, sMatch, vsPathsOut, bTryInsideOfSongGroupAndGenreFirst, bTryInsideOfSongGroupFirst );

	for (auto const &s: vsPathsOut)
	{
		std::string name{ Rage::tail(s, s.size() - RANDOMMOVIES_DIR.size() - 1) };
		vsNamesOut.push_back( name );
	}
	StripCvsAndSvn( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::BakeAllBackgroundChanges( Song *pSong )
{
	Background bg;
	bg.LoadFromSong( pSong );
	vector<BackgroundChange> *vBGChanges[NUM_BackgroundLayer];
	FOREACH_BackgroundLayer( i )
	{
		vBGChanges[i] = &pSong->GetBackgroundChanges(i);
	}
	bg.GetLoadedBackgroundChanges( vBGChanges );
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
