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
#include "arch/ArchHooks/ArchHooks.h"
#include "RageUtil.h"

void ActorFrame::AddChild( Actor* pActor )
{
#if _DEBUG
	// check that this Actor isn't already added.
	vector<Actor*>::iterator iter = find( m_SubActors.begin(), m_SubActors.end(), pActor );
	if( iter != m_SubActors.end() )
		HOOKS->MessageBoxOK( ssprintf("Actor \"%s\" adds child \"%s\" more than once", m_sName.c_str(), pActor->m_sName.c_str()) );
#endif

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
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->Draw();
}

void ActorFrame::RunCommandOnChildren( const CString &cmd )
{
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->Command( cmd );
}

void ActorFrame::Update( float fDeltaTime )
{
//	LOG->Trace( "ActorFrame::Update( %f )", fDeltaTime );
	Actor::Update( fDeltaTime );

	// update all sub-Actors
	for( vector<Actor*>::iterator it=m_SubActors.begin(); it!=m_SubActors.end(); it++ )
		(*it)->Update(fDeltaTime);
}

#define PropagateActorFrameCommand( cmd, type ) \
	void ActorFrame::cmd( type f )		\
	{									\
		Actor::cmd( f );				\
										\
		/* set all sub-Actors */		\
		for( unsigned i=0; i<m_SubActors.size(); i++ ) \
			m_SubActors[i]->cmd( f );	\
	}

PropagateActorFrameCommand( SetDiffuse,			RageColor )
PropagateActorFrameCommand( SetZTest,			bool )
PropagateActorFrameCommand( SetZWrite,			bool )
PropagateActorFrameCommand( HurryTweening,		float )

void ActorFrame::SetDiffuseAlpha( float f )	
{
	Actor::SetDiffuseAlpha( f );
								
	/* set all sub-Actors */
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->SetDiffuseAlpha( f );
}


void ActorFrame::FinishTweening()
{
	Actor::FinishTweening();

	// set all sub-Actors
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->FinishTweening();
}

float ActorFrame::GetTweenTimeLeft() const
{
	float m = Actor::GetTweenTimeLeft();

	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m = max(m, m_SubActors[i]->GetTweenTimeLeft());

	return m;

}

bool CompareActorsByZDesc(const Actor *p1, const Actor *p2)
{
	return p1->GetZ() > p2->GetZ();
}

void ActorFrame::SortByZ()
{
	// Preserve ordering of Actors with equal Z values.
	stable_sort( m_SubActors.begin(), m_SubActors.end(), CompareActorsByZDesc );
}

void ActorFrame::DeleteAllChildren()
{
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		delete m_SubActors[i];
	m_SubActors.clear();
}

void ActorFrame::HandleCommand( const ParsedCommand &command )
{
	Actor::HandleCommand( command );
}

void ActorFrame::GainingFocus( float fRate, bool bRewindMovie, bool bLoop )
{
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->GainingFocus( fRate, bRewindMovie, bLoop );

	SetDiffuse( RageColor(1,1,1,1) );
}

void ActorFrame::LosingFocus()
{
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->LosingFocus();
}

void ActorFrame::PlayCommand( const CString &sCommandName )
{
	for( unsigned i=0; i<m_SubActors.size(); i++ ) 
		m_SubActors[i]->PlayCommand( sCommandName );
}
