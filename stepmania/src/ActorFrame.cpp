#include "stdafx.h"	// testing updates
/*
-----------------------------------------------------------------------------
 File: ActorFrame.h

 Desc: Base class for all objects that appear on the screen.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include <math.h>
#include "RageScreen.h"


ActorFrame::ActorFrame()
{
	m_pos			= m_start_pos			= m_end_pos			= D3DXVECTOR2( 0, 0 );
	m_rotation		= m_start_rotation		= m_end_rotation	= D3DXVECTOR3( 0, 0, 0 );
	m_scale			= m_start_scale			= m_end_scale		= D3DXVECTOR2( 1, 1 );

	m_TweenType	= no_tween;
	m_fTweenTime = 0.0f;
	m_fTimeIntoTween = 0.0f;
}


void ActorFrame::Draw()
{
	D3DXVECTOR2 pos				= m_pos;
	D3DXVECTOR3 rotation		= m_rotation;
	D3DXVECTOR2 scale			= m_scale;

	
	LPDIRECT3DDEVICE8 pd3dDevice = SCREEN->GetDevice();

	// calculate and apply world transform
	D3DXMATRIX matOriginalWorld, matNewWorld, matTemp;
    pd3dDevice->GetTransform( D3DTS_WORLD, &matOriginalWorld );	// save the original world matrix

	matNewWorld = matOriginalWorld;		// initialize the matrix we're about to build to transform into this Frame's coord space

	D3DXMatrixTranslation( &matTemp, pos.x, pos.y, 0 );	// add in the translation
	matNewWorld = matTemp * matNewWorld;
	D3DXMatrixScaling( &matTemp, scale.x, scale.y, 1 );	// add in the zoom
	matNewWorld = matTemp * matNewWorld;
	D3DXMatrixRotationYawPitchRoll( &matTemp, rotation.y, rotation.x, rotation.z );	// add in the rotation
	matNewWorld = matTemp * matNewWorld;

    pd3dDevice->SetTransform( D3DTS_WORLD, &matNewWorld );	// apply the translation so we're in this ActorFrame's local coords

	// draw all sub-actors while we're in the frame's local coordinate space
	for( int i=0; i<m_SubActors.GetSize(); i++ ) {
	//    pd3dDevice->SetTransform( D3DTS_WORLD, &matNewWorld );	// apply the translation so we're in this ActorFrame's local coords
		m_SubActors[i]->Draw();
	}

    
	pd3dDevice->SetTransform( D3DTS_WORLD, &matOriginalWorld );	// restore the original world matrix

}


void ActorFrame::Update( float fDeltaTime )
{
//	RageLog( "ActorFrame::Update( %f )", fDeltaTime )

	// update tweening
	if( m_TweenType != no_tween )		// we are performing some type of tweening
	{
		m_fTimeIntoTween += fDeltaTime;

		if( m_fTimeIntoTween > m_fTweenTime )	// The tweening is over.  Stop the tweening
		{
			m_pos = m_end_pos;
			m_scale = m_end_scale;
			m_rotation = m_end_rotation;
			m_TweenType = no_tween;
		}
		else		// Tweening.  Recalcute the curent position.
		{
			float fPercentThroughTween = m_fTimeIntoTween / m_fTweenTime;

			// distort the percentage if appropriate
			if( m_TweenType == tween_bias_begin )
				fPercentThroughTween = (float) sqrt( fPercentThroughTween );
			else if( m_TweenType == tweening_bias_end )
				fPercentThroughTween = fPercentThroughTween * fPercentThroughTween;


			m_pos			= m_start_pos	  + (m_end_pos		- m_start_pos	  )*fPercentThroughTween;
			m_scale			= m_start_scale	  + (m_end_scale	- m_start_scale	  )*fPercentThroughTween;
			m_rotation		= m_start_rotation+ (m_end_rotation - m_start_rotation)*fPercentThroughTween;
		}
	
	}	// end if m_TweenType != no_tween


	// update all sub-actors
	for( int i=0; i<m_SubActors.GetSize(); i++ )
		m_SubActors[i]->Update(fDeltaTime);
}


void ActorFrame::BeginTweening( float time, TweenType tt )
{
	// set our tweeen starting and ending values to the current position
	m_start_pos				= m_end_pos				= m_pos;
	m_start_scale			= m_end_scale			= m_scale;
	m_start_rotation		= m_end_rotation		= m_rotation;

	m_TweenType = tt;
	m_fTweenTime = time;
	m_fTimeIntoTween = 0;
}

void ActorFrame::SetTweenX( float x )			{ m_end_pos.x = x; } 
void ActorFrame::SetTweenY( float y )			{ m_end_pos.y = y; }
void ActorFrame::SetTweenXY( float x, float y )	{ SetTweenX(x); SetTweenY(y); }
void ActorFrame::SetTweenZoom( float zoom )		{ m_end_scale.x = zoom;  m_end_scale.y = zoom; }
void ActorFrame::SetTweenRotationX( float r )	{ m_end_rotation.x = r; }
void ActorFrame::SetTweenRotationY( float r )	{ m_end_rotation.y = r; }
void ActorFrame::SetTweenRotationZ( float r )	{ m_end_rotation.z = r; }


