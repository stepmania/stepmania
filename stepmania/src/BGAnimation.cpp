#include "global.h"
#include "BGAnimation.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "IniFile.h"
#include "BGAnimationLayer.h"
#include "RageUtil.h"
#include "song.h"
#include "ThemeManager.h"
#include "RageFile.h"
#include "ActorUtil.h"
#include "LuaHelpers.h"
#include "arch/Dialog/Dialog.h"

const int MAX_LAYERS = 1000;

BGAnimation::BGAnimation( bool Generic )
{
	/* See BGAnimationLayer::BGAnimationLayer for explanation. */
	m_bGeneric = Generic;
	m_fLengthSeconds = 10;
}

BGAnimation::~BGAnimation()
{
	Unload();
}

void BGAnimation::Unload()
{
    DeleteAllChildren();
}

void BGAnimation::LoadFromStaticGraphic( CString sPath )
{
	Unload();

	BGAnimationLayer* pLayer = new BGAnimationLayer( m_bGeneric );
	pLayer->LoadFromStaticGraphic( sPath );
	AddChild( pLayer );
}

void AddLayersFromAniDir( CString sAniDir, vector<Actor*> &layersAddTo, bool Generic )
{
	if( sAniDir.empty() )
		 return;

	if( sAniDir.Right(1) != "/" )
		sAniDir += "/";

	ASSERT_M( IsADirectory(sAniDir), sAniDir + " isn't a directory" );

	CString sPathToIni = sAniDir + "BGAnimation.ini";

	IniFile ini;
	ini.ReadFile( sPathToIni );

	{
		CString expr;
		if( ini.GetValue( "BGAnimation", "Condition", expr ) || ini.GetValue( "BGAnimation", "Cond", expr ) )
		{
			if( !Lua::RunExpression( expr ) )
				return;
		}
	}

	int i;
	for( i=0; i<MAX_LAYERS; i++ )
	{
		CString sLayer = ssprintf("Layer%d",i+1);
		const IniFile::key* pKey = ini.GetKey( sLayer );
		if( pKey == NULL )
			continue;	// skip

		CString sImportDir;
		if( ini.GetValue(sLayer, "Import", sImportDir) )
		{
			CString expr;
			if( ini.GetValue(sLayer,"Condition",expr) )
			{
				if( !Lua::RunExpression( expr ) )
					continue;
			}

			// import a whole BGAnimation
			sImportDir = sAniDir + sImportDir;
			CollapsePath( sImportDir );
			AddLayersFromAniDir( sImportDir, layersAddTo, Generic );
		}
		else
		{
			// import as a single layer
			BGAnimationLayer* pLayer = new BGAnimationLayer( Generic );
			pLayer->LoadFromIni( sAniDir, sLayer );
			layersAddTo.push_back( pLayer );
		}
	}

}

void BGAnimation::LoadFromAniDir( CString sAniDir )
{
	Unload();

	if( sAniDir.empty() )
		 return;

	if( sAniDir.Right(1) != "/" )
		sAniDir += "/";

	ASSERT_M( IsADirectory(sAniDir), sAniDir + " isn't a directory" );

	CString sPathToIni = sAniDir + "BGAnimation.ini";

	if( DoesFileExist(sPathToIni) )
	{
		// This is a new style BGAnimation (using .ini)
		AddLayersFromAniDir( sAniDir, m_SubActors, m_bGeneric );	// TODO: Check for circular load

		IniFile ini;
		ini.ReadFile( sPathToIni );
		if( !ini.GetValue( "BGAnimation", "LengthSeconds", m_fLengthSeconds ) )
		{
			/* XXX: if m_bGeneric, simply constructing the BG layer won't run "On",
			 * so at this point GetMaxTweenTimeLeft is probably 0 */
			m_fLengthSeconds = this->GetTweenTimeLeft();
		}

		bool bUseScroller;
		if( ini.GetValue( "BGAnimation", "UseScroller", bUseScroller ) && bUseScroller )
		{
			// TODO: Move this into ActorScroller
			
#define REQUIRED_GET_VALUE( szName, valueOut ) \
	if( !ini.GetValue( "Scroller", szName, valueOut ) ) \
		Dialog::OK( ssprintf("File '%s' is missing the value Scroller::%s", sPathToIni.c_str(), szName) );

			float fSecondsPerItem = 1;
			int iNumItemsToDraw = 7;
			RageVector3	vRotationDegrees = RageVector3(0,0,0);
			RageVector3	vTranslateTerm0 = RageVector3(0,0,0);
			RageVector3	vTranslateTerm1 = RageVector3(0,0,0);
			RageVector3	vTranslateTerm2 = RageVector3(0,0,0);
			float fItemPaddingStart = 0;
			float fItemPaddingEnd = 0;

			REQUIRED_GET_VALUE( "SecondsPerItem", fSecondsPerItem );
			REQUIRED_GET_VALUE( "NumItemsToDraw", iNumItemsToDraw );
			REQUIRED_GET_VALUE( "RotationDegreesX", vRotationDegrees[0] );
			REQUIRED_GET_VALUE( "RotationDegreesY", vRotationDegrees[1] );
			REQUIRED_GET_VALUE( "RotationDegreesZ", vRotationDegrees[2] );
			REQUIRED_GET_VALUE( "TranslateTerm0X", vTranslateTerm0[0] );
			REQUIRED_GET_VALUE( "TranslateTerm0Y", vTranslateTerm0[1] );
			REQUIRED_GET_VALUE( "TranslateTerm0Z", vTranslateTerm0[2] );
			REQUIRED_GET_VALUE( "TranslateTerm1X", vTranslateTerm1[0] );
			REQUIRED_GET_VALUE( "TranslateTerm1Y", vTranslateTerm1[1] );
			REQUIRED_GET_VALUE( "TranslateTerm1Z", vTranslateTerm1[2] );
			REQUIRED_GET_VALUE( "TranslateTerm2X", vTranslateTerm2[0] );
			REQUIRED_GET_VALUE( "TranslateTerm2Y", vTranslateTerm2[1] );
			REQUIRED_GET_VALUE( "TranslateTerm2Z", vTranslateTerm2[2] );
			REQUIRED_GET_VALUE( "ItemPaddingStart", fItemPaddingStart );
			REQUIRED_GET_VALUE( "ItemPaddingEnd", fItemPaddingEnd );
#undef REQUIRED_GET_VALUE

			ActorScroller::Load( 
				fSecondsPerItem,
				iNumItemsToDraw,
				vRotationDegrees,
				vTranslateTerm0,
				vTranslateTerm1,
				vTranslateTerm2 );
			ActorScroller::SetCurrentAndDestinationItem( int(-fItemPaddingStart) );
			ActorScroller::SetDestinationItem( int(m_SubActors.size()-1+fItemPaddingEnd) );
		}

		CString InitCommand;
		if( ini.GetValue( "BGAnimation", "InitCommand", InitCommand ) )
		{
			/* There's an InitCommand.  Run it now.  This can be used to eg. change Z to
			 * modify draw order between BGAs in a Foreground.  Most things should be done
			 * in metrics.ini commands, not here. */
			this->Command( InitCommand );
		}
	}
	else
	{
		// This is an old style BGAnimation (not using .ini)

		// loading a directory of layers
		CStringArray asImagePaths;
		ASSERT( sAniDir != "" );

		GetDirListing( sAniDir+"*.png", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.jpg", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.gif", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.avi", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.mpg", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.mpeg", asImagePaths, false, true );
		GetDirListing( sAniDir+"*.sprite", asImagePaths, false, true );

		SortCStringArray( asImagePaths );

		for( unsigned i=0; i<asImagePaths.size(); i++ )
		{
			const CString sPath = asImagePaths[i];
			if( Basename(sPath).Left(1) == "_" )
				continue;	// don't directly load files starting with an underscore
			BGAnimationLayer* pLayer = new BGAnimationLayer( m_bGeneric );
			pLayer->LoadFromAniLayerFile( asImagePaths[i] );
			AddChild( pLayer );
		}
	}
}

void BGAnimation::LoadFromMovie( CString sMoviePath )
{
	Unload();

	BGAnimationLayer* pLayer = new BGAnimationLayer( m_bGeneric );
	pLayer->LoadFromMovie( sMoviePath );
	AddChild( pLayer );
}

void BGAnimation::LoadFromVisualization( CString sVisPath )
{
	Unload();
	BGAnimationLayer* pLayer;
	
	const Song* pSong = GAMESTATE->m_pCurSong;
	CString sSongBGPath = pSong && pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathToG("Common fallback background");

	pLayer = new BGAnimationLayer( m_bGeneric );
	pLayer->LoadFromStaticGraphic( sSongBGPath );
	AddChild( pLayer );

	pLayer = new BGAnimationLayer( m_bGeneric );
	pLayer->LoadFromVisualization( sVisPath );
	AddChild( pLayer );
}

/*
 * (c) 2001-2004 Ben Nordstrom, Chris Danford
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
