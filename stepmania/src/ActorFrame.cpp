#include "global.h"	// testing updates
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

void ActorFrame::MoveToTail( Actor* pActor )
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

void ActorFrame::MoveToHead( Actor* pActor )
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
	// Don't set Actor-defined render states because we won't be drawing 
	// any geometry that belongs to this object.
	// Actor::DrawPrimitives();

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

void ActorFrame::SetUseZBuffer( bool b )
{
	Actor::SetUseZBuffer( b );

	// set all sub-Actors
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->SetUseZBuffer( b );
}

float ActorFrame::GetTweenTimeLeft() const
{
	float m = Actor::GetTweenTimeLeft();

	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m = max(m, m_SubActors[i]->GetTweenTimeLeft());

	return m;

}

void ActorFrame::SortByZ()
{
	// Sort to have descending Z values so we draw back to front.
	// STL sort()s typically use qsort, which won't preserve 
	// ordering of Actors with equal Z values.
	// Sort it ourselves.  -Chris
	vector<Actor*> v;
	for( unsigned i=0; i<m_SubActors.size(); i++ )
	{
		Actor* pToInsert = m_SubActors[i];

		for( unsigned j=0; j<v.size(); j++ )
			if( pToInsert->GetZ() > v[j]->GetZ() )
			{
				v.insert( v.begin()+j, pToInsert );
				break;
			}

		if( j == v.size() )
			v.push_back( pToInsert );
	}

	for( unsigned j=0; j<v.size(); j++ )
		printf( "%.0f ", v[j]->GetZ() );

	m_SubActors = v;
}
