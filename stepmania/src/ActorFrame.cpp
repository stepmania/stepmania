#include "stdafx.h"	// testing updates
/*
-----------------------------------------------------------------------------
 Class: ActorFrame

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"

void ActorFrame::AddChild( Actor* pActor)
{
	ASSERT( pActor );
	ASSERT( (void*)pActor != (void*)0xC0000005 );
	m_SubActors.Add( pActor );
}

void ActorFrame::DrawPrimitives()
{
	// draw all sub-ActorFrames while we're in the ActorFrame's local coordinate space
	for( int i=0; i<m_SubActors.GetSize(); i++ ) {
		m_SubActors[i]->Draw();
	}
}


void ActorFrame::Update( float fDeltaTime )
{
//	LOG->Trace( "ActorFrame::Update( %f )", fDeltaTime );

	Actor::Update( fDeltaTime );


	// update all sub-Actors
	for( int i=0; i<m_SubActors.GetSize(); i++ )
		m_SubActors[i]->Update(fDeltaTime);
}


void ActorFrame::SetDiffuse( RageColor c )
{
	Actor::SetDiffuse( c );

	// set all sub-Actors
	for( int i=0; i<m_SubActors.GetSize(); i++ )
		m_SubActors[i]->SetDiffuse(c );
}
