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


BGAnimation::BGAnimation()
{
	m_fFadeSeconds = 0;
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

	BGAnimationLayer* pLayer = new BGAnimationLayer;
	pLayer->LoadFromStaticGraphic( sPath );
	m_Layers.push_back( pLayer );

	m_fFadeSeconds = 0.5f;
}

void BGAnimation::LoadFromAniDir( CString sAniDir, CString sSongBGPath )
{
	Unload();

	if( sAniDir.Right(1) != "/" )
		sAniDir += "/";

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
		CString sDir, sFName, sExt;
		splitrelpath( sPath, sDir, sFName, sExt );
		if( sFName.Left(1) == "_" )
			continue;	// don't directly load files starting with an underscore
		BGAnimationLayer* pLayer = new BGAnimationLayer;
		pLayer->LoadFromAniLayerFile( asImagePaths[i], sSongBGPath );
		m_Layers.push_back( pLayer );
	}

	m_fFadeSeconds = 0;
}

void BGAnimation::LoadFromMovie( CString sMoviePath, bool bLoop, bool bRewind )
{
	Unload();

	BGAnimationLayer* pLayer = new BGAnimationLayer;
	pLayer->LoadFromMovie( sMoviePath, bLoop, bRewind );
	m_Layers.push_back( pLayer );

	m_fFadeSeconds = 0.5f;
}

void BGAnimation::LoadFromVisualization( CString sVisPath, CString sSongBGPath )
{
	Unload();
	BGAnimationLayer* pLayer;
	
	pLayer = new BGAnimationLayer;
	pLayer->LoadFromStaticGraphic( sSongBGPath );
	m_Layers.push_back( pLayer );

	pLayer = new BGAnimationLayer;
	pLayer->LoadFromVisualization( sVisPath );
	m_Layers.push_back( pLayer );	

	m_fFadeSeconds = 0.5f;
}


void BGAnimation::Update( float fDeltaTime )
{
	for( unsigned i=0; i<m_Layers.size(); i++ )
		m_Layers[i]->Update( fDeltaTime );
}

void BGAnimation::DrawPrimitives()
{
	for( unsigned i=0; i<m_Layers.size(); i++ )
		m_Layers[i]->Draw();
}
	
void BGAnimation::GainingFocus()
{
	for( unsigned i=0; i<m_Layers.size(); i++ )
		m_Layers[i]->GainingFocus();

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
 }
