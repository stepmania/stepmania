#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: BGAnimation

 Desc: Particles used initially for background effects

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

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
    for( unsigned i=0; i<m_Layers.size(); i++ )
		delete m_Layers[i];
	m_Layers.clear();
}

void BGAnimation::LoadFromStaticGraphic( CString sPath )
{
	Unload();

	BGAnimationLayer* pLayer = new BGAnimationLayer( m_bGeneric );
	pLayer->LoadFromStaticGraphic( sPath );
	m_Layers.push_back( pLayer );
}

void AddLayersFromAniDir( CString sAniDir, vector<BGAnimationLayer*> &layersAddTo, bool Generic )
{
	if( sAniDir.empty() )
		 return;

	if( sAniDir.Right(1) != "/" )
		sAniDir += "/";

	RAGE_ASSERT_M( IsADirectory(sAniDir), sAniDir + " isn't a directory" );

	CString sPathToIni = sAniDir + "BGAnimation.ini";

	IniFile ini(sPathToIni);
	ini.ReadFile();

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

	RAGE_ASSERT_M( IsADirectory(sAniDir), sAniDir + " isn't a directory" );

	CString sPathToIni = sAniDir + "BGAnimation.ini";

	if( DoesFileExist(sPathToIni) )
	{
		// This is a new style BGAnimation (using .ini)
		AddLayersFromAniDir( sAniDir, m_Layers, m_bGeneric );	// TODO: Check for circular load

		IniFile ini(sPathToIni);
		ini.ReadFile();
		if( !ini.GetValue( "BGAnimation", "LengthSeconds", m_fLengthSeconds ) )
		{
			m_fLengthSeconds = 0;
			/* XXX: if m_bGeneric, simply constructing the BG layer won't run "On",
			 * so at this point GetMaxTweenTimeLeft is probably 0 */
			for( int i=0; (unsigned)i < m_Layers.size(); i++ )
				m_fLengthSeconds = max(m_fLengthSeconds, m_Layers[i]->GetMaxTweenTimeLeft());
		}

		bool bUseScroller;
		if( ini.GetValue( "BGAnimation", "UseScroller", bUseScroller ) && bUseScroller )
		{
#define REQUIRED_GET_VALUE( szName, valueOut ) \
	if( !ini.GetValue( "BGAnimation", szName, valueOut ) ) \
		RageException::Throw( "File '%s' is missing the value BGAnimation::%s", sPathToIni.c_str(), szName );

			float fScrollSecondsPerItem, fSpacingX, fSpacingY, fItemPaddingStart, fItemPaddingEnd;
			REQUIRED_GET_VALUE( "ScrollSecondsPerItem", fScrollSecondsPerItem );
			REQUIRED_GET_VALUE( "ScrollSpacingX", fSpacingX );
			REQUIRED_GET_VALUE( "ScrollSpacingY", fSpacingY );
			REQUIRED_GET_VALUE( "ItemPaddingStart", fItemPaddingStart );
			REQUIRED_GET_VALUE( "ItemPaddingEnd", fItemPaddingEnd );
#undef REQUIRED_GET_VALUE

			m_Scroller.Load( fScrollSecondsPerItem, fSpacingX, fSpacingY );
			for( unsigned i=0; i<m_Layers.size(); i++ )
				m_Scroller.AddChild( m_Layers[i] );
			m_Scroller.SetCurrentAndDestinationItem( int(-fItemPaddingStart) );
			m_Scroller.SetDestinationItem( int(m_Layers.size()-1+fItemPaddingEnd) );
			this->AddChild( &m_Scroller );
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
			m_Layers.push_back( pLayer );
		}
	}
}

void BGAnimation::LoadFromMovie( CString sMoviePath )
{
	Unload();

	BGAnimationLayer* pLayer = new BGAnimationLayer( m_bGeneric );
	pLayer->LoadFromMovie( sMoviePath );
	m_Layers.push_back( pLayer );
}

void BGAnimation::LoadFromVisualization( CString sVisPath )
{
	Unload();
	BGAnimationLayer* pLayer;
	
	const Song* pSong = GAMESTATE->m_pCurSong;
	CString sSongBGPath = pSong && pSong->HasBackground() ? pSong->GetBackgroundPath() : THEME->GetPathToG("Common fallback background");

	pLayer = new BGAnimationLayer( m_bGeneric );
	pLayer->LoadFromStaticGraphic( sSongBGPath );
	m_Layers.push_back( pLayer );

	pLayer = new BGAnimationLayer( m_bGeneric );
	pLayer->LoadFromVisualization( sVisPath );
	m_Layers.push_back( pLayer );	
}


void BGAnimation::Update( float fDeltaTime )
{
	for( unsigned i=0; i<m_Layers.size(); i++ )
		m_Layers[i]->Update( fDeltaTime );
	ActorFrame::Update( fDeltaTime );
}

void BGAnimation::DrawPrimitives()
{
	for( unsigned i=0; i<m_Layers.size(); i++ )
		m_Layers[i]->Draw();
}
	
void BGAnimation::GainingFocus( float fRate, bool bRewindMovie, bool bLoop )
{
	for( unsigned i=0; i<m_Layers.size(); i++ )
		m_Layers[i]->GainingFocus( fRate, bRewindMovie, bLoop );

	SetDiffuse( RageColor(1,1,1,1) );
}

void BGAnimation::LosingFocus()
{
	for( unsigned i=0; i<m_Layers.size(); i++ )
		m_Layers[i]->LosingFocus();
}

void BGAnimation::SetDiffuse( const RageColor &c )
{
	for( unsigned i=0; i<m_Layers.size(); i++ ) 
		m_Layers[i]->SetDiffuse(c);
	ActorFrame::SetDiffuse( c );
}

float BGAnimation::GetTweenTimeLeft() const
{
	float ret = 0;

	for( unsigned i=0; i<m_Layers.size(); ++i )
		ret = max( ret, m_Layers[i]->GetMaxTweenTimeLeft() );

	return max( ret, Actor::GetTweenTimeLeft() );
}

void BGAnimation::FinishTweening()
{
	for( unsigned i=0; i<m_Layers.size(); i++ )
		m_Layers[i]->FinishTweening();
	ActorFrame::FinishTweening();
}

void BGAnimation::PlayCommand( const CString &cmd )
{
	for( unsigned i=0; i<m_Layers.size(); i++ ) 
		m_Layers[i]->PlayCommand( cmd );
}

void BGAnimation::HandleCommand( const ParsedCommand &command )
{
	HandleParams;

	if( sParam(0)=="playcommand" )	PlayCommand( sParam(1) );
	else
	{
		Actor::HandleCommand( command );
		return;
	}

	CheckHandledParams;
}

