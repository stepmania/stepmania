#include "global.h"
#include "BackgroundUtil.h"
#include "RageUtil.h"
#include "song.h"
#include "Foreach.h"
#include "IniFile.h"
#include "RageLog.h"
#include <set>
#include "Background.h"


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


RString BackgroundChange::GetTextDescription() const
{
	vector<RString> vsParts;
	if( !m_def.m_sFile1.empty() )	vsParts.push_back( m_def.m_sFile1 );
	if( !m_def.m_sFile2.empty() )	vsParts.push_back( m_def.m_sFile2 );
	if( m_fRate!=1.0f )				vsParts.push_back( ssprintf("%.2f%%",m_fRate*100) );
	if( !m_sTransition.empty() )	vsParts.push_back( m_sTransition );
	if( !m_def.m_sEffect.empty() )	vsParts.push_back( m_def.m_sEffect );
	if( !m_def.m_sColor1.empty() )	vsParts.push_back( m_def.m_sColor1 );
	if( !m_def.m_sColor2.empty() )	vsParts.push_back( m_def.m_sColor2 );
	
	if( vsParts.empty() )
		vsParts.push_back( "(empty)" );

	RString s = join( "\n", vsParts );
	return s;
}


const RString BACKGROUND_EFFECTS_DIR =		"BackgroundEffects/";
const RString BACKGROUND_TRANSITIONS_DIR =	"BackgroundTransitions/";
const RString BG_ANIMS_DIR			= "BGAnimations/";
const RString VISUALIZATIONS_DIR	= "Visualizations/";
const RString RANDOMMOVIES_DIR		= "RandomMovies/";

const RString RANDOM_BACKGROUND_FILE    = "-random-";
const RString NO_SONG_BG_FILE           = "-nosongbg-";
const RString SONG_BACKGROUND_FILE      = "songbackground";

const RString SBE_UpperLeft             = "UpperLeft";
const RString SBE_Centered              = "Centered";
const RString SBE_StretchNormal         = "StretchNormal";
const RString SBE_StretchNoLoop         = "StretchNoLoop";
const RString SBE_StretchRewind         = "StretchRewind";
const RString SBT_CrossFade             = "CrossFade";

static void StripCvs( vector<RString> &vsPathsToStrip, vector<RString> &vsNamesToStrip )
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

void BackgroundUtil::GetBackgroundEffects( const RString &_sName, vector<RString> &vsPathsOut, vector<RString> &vsNamesOut )
{
	RString sName = _sName;
	if( sName == "" )
		sName = "*";

	vsPathsOut.clear();
	GetDirListing( BACKGROUND_EFFECTS_DIR+sName+".xml", vsPathsOut, false, true );
	
	vsNamesOut.clear();
	FOREACH_CONST( RString, vsPathsOut, s )
		vsNamesOut.push_back( GetFileNameWithoutExtension(*s) );

	StripCvs( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetBackgroundTransitions( const RString &_sName, vector<RString> &vsPathsOut, vector<RString> &vsNamesOut )
{
	RString sName = _sName;
	if( sName == "" )
		sName = "*";

	vsPathsOut.clear();
	GetDirListing( BACKGROUND_TRANSITIONS_DIR+sName+".xml", vsPathsOut, false, true );
	
	vsNamesOut.clear();
	FOREACH_CONST( RString, vsPathsOut, s )
		vsNamesOut.push_back( GetFileNameWithoutExtension(*s) );

	StripCvs( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetSongBGAnimations( const Song *pSong, const RString &sMatch, vector<RString> &vsPathsOut, vector<RString> &vsNamesOut )
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
	FOREACH_CONST( RString, vsPathsOut, s )
		vsNamesOut.push_back( Basename(*s) );

	StripCvs( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetSongMovies( const Song *pSong, const RString &sMatch, vector<RString> &vsPathsOut, vector<RString> &vsNamesOut )
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
	FOREACH_CONST( RString, vsPathsOut, s )
		vsNamesOut.push_back( Basename(*s) );

	StripCvs( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetSongBitmaps( const Song *pSong, const RString &sMatch, vector<RString> &vsPathsOut, vector<RString> &vsNamesOut )
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
	FOREACH_CONST( RString, vsPathsOut, s )
		vsNamesOut.push_back( Basename(*s) );

	StripCvs( vsPathsOut, vsNamesOut );
}

static void GetFilterToFileNames( const RString sBaseDir, const Song *pSong, set<RString> &vsPossibleFileNamesOut )
{
	vsPossibleFileNamesOut.clear();

	if( pSong->m_sGenre.empty() )
		return;

	ASSERT( !pSong->m_sGroupName.empty() );
	IniFile ini;
	RString sPath = sBaseDir+pSong->m_sGroupName+"/"+"BackgroundMapping.ini";
	ini.ReadFile( sPath );
	
	RString sSection;
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
		vsPossibleFileNamesOut.insert( p->first );
}

void BackgroundUtil::GetGlobalBGAnimations( const Song *pSong, const RString &sMatch, vector<RString> &vsPathsOut, vector<RString> &vsNamesOut )
{
	vsPathsOut.clear();
	GetDirListing( BG_ANIMS_DIR+sMatch+"*", vsPathsOut, true, true );
	GetDirListing( BG_ANIMS_DIR+sMatch+"*.xml", vsPathsOut, false, true );

	vsNamesOut.clear();
	FOREACH_CONST( RString, vsPathsOut, s )
		vsNamesOut.push_back( Basename(*s) );

	StripCvs( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::GetGlobalRandomMovies( 
	const Song *pSong, 
	const RString &sMatch, 
	vector<RString> &vsPathsOut, 
	vector<RString> &vsNamesOut,
	bool bTryInsideOfSongGroupAndGenreFirst,
	bool bTryInsideOfSongGroupFirst )
{
	vsPathsOut.clear();
	vsNamesOut.clear();

	// Check for an exact match
	if( !sMatch.empty() )
	{
		GetDirListing( RANDOMMOVIES_DIR+sMatch, vsPathsOut, false, true );
		if( !vsPathsOut.empty() )
			goto found_files;
		
		if( sMatch != NO_SONG_BG_FILE )
			LOG->Warn( "Background missing: %s", sMatch.c_str() );
		return;
	}

	//
	// Search for the most appropriate background
	//
	{
		set<RString> ssFileNameWhitelist;
		if( bTryInsideOfSongGroupAndGenreFirst  &&  pSong  &&  !pSong->m_sGenre.empty() )
			GetFilterToFileNames( RANDOMMOVIES_DIR, pSong, ssFileNameWhitelist );

		vector<RString> vsDirsToTry;
		if( bTryInsideOfSongGroupFirst && pSong )
		{
			ASSERT( !pSong->m_sGroupName.empty() );
			vsDirsToTry.push_back( RANDOMMOVIES_DIR+pSong->m_sGroupName+"/" );
		}
		vsDirsToTry.push_back( RANDOMMOVIES_DIR );

		FOREACH_CONST( RString, vsDirsToTry, sDir )
		{
			GetDirListing( *sDir+"*.avi", vsPathsOut, false, true );
			GetDirListing( *sDir+"*.mpg", vsPathsOut, false, true );
			GetDirListing( *sDir+"*.mpeg", vsPathsOut, false, true );

			if( !ssFileNameWhitelist.empty() )
			{
				vector<RString> vsFiltered = vsPathsOut;
				for( unsigned i=0; i<vsPathsOut.size(); i++ )
				{
					RString sBasename = Basename( vsPathsOut[i] );
					bool bFound = ssFileNameWhitelist.find(sBasename) != ssFileNameWhitelist.end();
					if( !bFound )
					{
						vsPathsOut.erase( vsPathsOut.begin()+i );
						i--;
					}
				}
				// If we filtered every movie out then this was a bad whitelist, so ignore the whitelist.
				if( !vsFiltered.empty() )
					vsPathsOut = vsFiltered;
			}

			if( !vsPathsOut.empty() )
				goto found_files;
		}
	}

found_files:

	FOREACH_CONST( RString, vsPathsOut, s )
	{
		RString sName = s->Right( s->size() - RANDOMMOVIES_DIR.size() - 1 );
		vsNamesOut.push_back( sName );
	}
	StripCvs( vsPathsOut, vsNamesOut );
}

void BackgroundUtil::BakeAllBackgroundChanges( Song *pSong )
{
	Background bg;
	bg.LoadFromSong( pSong );
	vector<BackgroundChange> *vBGChanges[NUM_BackgroundLayer];
	FOREACH_BackgroundLayer( i )
		vBGChanges[i] = &pSong->GetBackgroundChanges(i);
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
