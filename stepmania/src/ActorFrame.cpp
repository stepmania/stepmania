#include "stdafx.h"	// testing updates
/*
-----------------------------------------------------------------------------
 File: ActorFrame.h

 Desc: Base class for all objects that appear on the screen.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"


void ActorFrame::RenderPrimitives()
{
	// draw all sub-ActorFrames while we're in the ActorFrame's local coordinate space
	for( int i=0; i<m_SubActors.GetSize(); i++ ) {
		m_SubActors[i]->Draw();
	}
}


void ActorFrame::Update( float fDeltaTime )
{
//	RageLog( "ActorFrame::Update( %f )", fDeltaTime );

	Actor::Update( fDeltaTime );


	// update all sub-Actors
	for( int i=0; i<m_SubActors.GetSize(); i++ )
		m_SubActors[i]->Update(fDeltaTime);
}


void ActorFrame::SetDiffuseColor( D3DXCOLOR c )
{
	Actor::SetDiffuseColor( c );

	// set all sub-Actors
	for( int i=0; i<m_SubActors.GetSize(); i++ )
		m_SubActors[i]->SetDiffuseColor(c );
}
