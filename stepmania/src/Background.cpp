#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Background.h

 Desc: A graphic displayed in the background during Dancing.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Background.h"
#include "RageUtil.h"


const CString sVisDir = "Visualizations\\";


void Background::LoadFromSong( Song& song )
{
	CString sBGTexturePath = song.GetBackgroundPath();
	sBGTexturePath.MakeLower();

	bool bIsAMovieBackground = ( sBGTexturePath.Right(3) == "avi" || 
								 sBGTexturePath.Right(3) == "mpg" ||
								 sBGTexturePath.Right(3) == "mpeg" );

	if( bIsAMovieBackground )
	{
		Sprite::LoadFromTexture( sBGTexturePath );
		Sprite::StretchTo( CRect(0,480,640,0) );	// flip
		Sprite::SetDiffuseColor( D3DXCOLOR(0.8f,0.8f,0.8f,1) );

		// don't load m_sprVis
	}
	else	// !bIsAMovieBackground
	{
		Sprite::LoadFromTexture( sBGTexturePath );
		Sprite::StretchTo( CRect(0,0,640,480) );
		Sprite::SetDiffuseColor( D3DXCOLOR(0.8f,0.8f,0.8f,1) );

		if( GAMEINFO->m_GameOptions.m_bRandomVis )
		{
			// load a random visualization
			CStringArray sVisualizationPaths;
			GetDirListing( sVisDir + "*.*", sVisualizationPaths );
			if( sVisualizationPaths.GetSize() > 0 )	// there is at least one visualization
			{
				int iIndexRandom = rand() % sVisualizationPaths.GetSize();

				m_sprVis.LoadFromTexture( sVisDir + sVisualizationPaths[iIndexRandom] );
				m_sprVis.StretchTo( CRect(0,480,640,0) );	// flip
				m_sprVis.SetBlendModeAdd();
				//m_sprVis.SetColor( D3DXCOLOR(1,1,1,0.5f) );
			}
		}		
	}

}

void Background::Update( float fDeltaTime)
{
	Sprite::Update( fDeltaTime );
	m_sprVis.Update( fDeltaTime );
}

void Background::Draw()
{
	Sprite::Draw();
	m_sprVis.Draw();
}