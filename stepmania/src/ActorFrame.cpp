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

void ActorFrame::AddChild( Actor* pActor )
{
	ASSERT( pActor );
	ASSERT( (void*)pActor != (void*)0xC0000005 );
	m_SubActors.push_back( pActor );
}

void ActorFrame::MoveToBack( Actor* pActor )
{
	vector<Actor*>::iterator iter = find( m_SubActors.begin(), m_SubActors.end(), pActor );
	if( iter == m_SubActors.end() )	// didn't find
	{
		ASSERT(0);	// called with a pActor that doesn't exist
		return;
	}

	m_SubActors.erase( iter );
	m_SubActors.push_back( pActor );
}

void ActorFrame::MoveToFront( Actor* pActor )
{
	vector<Actor*>::iterator iter = find( m_SubActors.begin(), m_SubActors.end(), pActor );
	if( iter == m_SubActors.end() )	// didn't find
	{
		ASSERT(0);	// called with a pActor that doesn't exist
		return;
	}

	m_SubActors.erase( iter );
	m_SubActors.insert( m_SubActors.begin(), pActor );
}


void ActorFrame::DrawPrimitives()
{
	// draw all sub-ActorFrames while we're in the ActorFrame's local coordinate space
	for( unsigned i=0; i<m_SubActors.size(); i++ ) {
		m_SubActors[i]->Draw();
	}
}


void ActorFrame::Update( float fDeltaTime )
{
//	LOG->Trace( "ActorFrame::Update( %f )", fDeltaTime );

	Actor::Update( fDeltaTime );


	// update all sub-Actors
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->Update(fDeltaTime);
}


void ActorFrame::SetDiffuse( RageColor c )
{
	Actor::SetDiffuse( c );

	// set all sub-Actors
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->SetDiffuse(c );
}

float ActorFrame::TweenTime() const
{
	float m = Actor::TweenTime();

	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m = max(m, m_SubActors[i]->TweenTime());

	return m;

}

