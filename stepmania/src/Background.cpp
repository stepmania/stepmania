#include "stdafx.h"
//-----------------------------------------------------------------------------
// File: Background.cpp
//
// Desc: Cropped version of the song background displayed in Song Select.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------


#include "Background.h"
#include "RageUtil.h"


const CString sVisDir = "Visualizations\\";


void Background::LoadFromSong( Song& song )
{
	Sprite::LoadFromTexture( song.GetBackgroundPath() );
	Sprite::StretchTo( CRect(0,0,640,480) );
	Sprite::SetColor( D3DXCOLOR(0.7f,0.7f,0.7f,1) );

	CStringArray sVisualizationPaths;
	GetDirListing( sVisDir + "*.*", sVisualizationPaths );
	if( sVisualizationPaths.GetSize() > 0 )	// there is at least one visualization
	{
		int iIndexRandom = rand() % sVisualizationPaths.GetSize();

		m_sprVis.LoadFromTexture( sVisDir + sVisualizationPaths[iIndexRandom] );
		m_sprVis.StretchTo( CRect(0,0,640,480) );
		m_sprVis.SetBlendMode( TRUE );
		//m_sprVis.SetColor( D3DXCOLOR(1,1,1,0.5f) );
	}
}

void Background::Update( const FLOAT& fDeltaTime)
{
	Sprite::Update( fDeltaTime );
	m_sprVis.Update( fDeltaTime );
}

void Background::Draw()
{
	Sprite::Draw();
	m_sprVis.Draw();
}