#include "global.h"
#include "ActorFrame.h"
#include "arch/Dialog/Dialog.h"
#include "RageUtil.h"
#include "XmlFile.h"
#include "ActorUtil.h"
#include "LuaBinding.h"
#include "ActorCommands.h"

// lua start
LUA_REGISTER_CLASS( ActorFrame )
// lua end

ActorFrame::ActorFrame()
{
	m_bPropagateCommands = false;
	m_bDeleteChildren = false;
}

ActorFrame::~ActorFrame()
{
	if( m_bDeleteChildren )
		DeleteAllChildren();
}

void ActorFrame::LoadFromNode( const CString& sDir, const XNode* pNode )
{
	Actor::LoadFromNode( sDir, pNode );

	//
	// Load children
	//
	const XNode* pChildren = pNode->GetChild("children");
	if( pChildren )
	{
		FOREACH_CONST_Child( pChildren, pChild )
		{
			Actor* pChildActor = ActorUtil::LoadFromActorFile( sDir, pChild );
			if( pChildActor )
				AddChild( pChildActor );
		}
		SortByDrawOrder();
	}
}

void ActorFrame::AddChild( Actor* pActor )
{
#if _DEBUG
	// check that this Actor isn't already added.
	vector<Actor*>::iterator iter = find( m_SubActors.begin(), m_SubActors.end(), pActor );
	if( iter != m_SubActors.end() )
		Dialog::OK( ssprintf("Actor \"%s\" adds child \"%s\" more than once", m_sName.c_str(), pActor->m_sName.c_str()) );
#endif

	ASSERT( pActor );
	ASSERT( (void*)pActor != (void*)0xC0000005 );
	m_SubActors.push_back( pActor );
}

void ActorFrame::RemoveChild( Actor* pActor )
{
	vector<Actor*>::iterator iter = find( m_SubActors.begin(), m_SubActors.end(), pActor );
	if( iter != m_SubActors.end() )
		m_SubActors.erase( iter );
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

void ActorFrame::RunCommandsOnChildren( const LuaReference& cmds )
{
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->RunCommands( cmds );
}

void ActorFrame::Update( float fDeltaTime )
{
//	LOG->Trace( "ActorFrame::Update( %f )", fDeltaTime );
	Actor::Update( fDeltaTime );

	if( m_fHibernateSecondsLeft > 0 )
		return;

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
PropagateActorFrameCommand( SetZTestMode,		ZTestMode )
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
	{
		const Actor* pActor = m_SubActors[i];
		m = max(m, m_fHibernateSecondsLeft + pActor->GetTweenTimeLeft());
	}

	return m;

}

bool CompareActorsByDrawOrder(const Actor *p1, const Actor *p2)
{
	return p1->GetDrawOrder() < p2->GetDrawOrder();
}

void ActorFrame::SortByDrawOrder()
{
	// Preserve ordering of Actors with equal DrawOrders.
	stable_sort( m_SubActors.begin(), m_SubActors.end(), CompareActorsByDrawOrder );
}

void ActorFrame::DeleteAllChildren()
{
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		delete m_SubActors[i];
	m_SubActors.clear();
}

void ActorFrame::RunCommands( const LuaReference& cmds )
{
	if( m_bPropagateCommands )
		RunCommandsOnChildren( cmds );
	else
		Actor::RunCommands( cmds );
}

void ActorFrame::SetPropagateCommands( bool b )
{
	m_bPropagateCommands = b;
}

/*
void ActorFrame::HandleCommand( const Command &command )
{
	BeginHandleArgs;

	const CString& sName = command.GetName();
	do
	{
		if( sName=="propagate" )
		{
			m_bPropagateCommands = bArg(1);
			RunCommandOnChildren( command );
		}
		else
		{
			Actor::HandleCommand( command );
			break;
		}
		EndHandleArgs;
	} while(0);

	// By default, don't propograte most commands to children; it makes no sense
	// to run "x,50" recursively.  If m_bPropagateCommands is set, propagate all
	// commands.
	if( m_bPropagateCommands && sName!="propagate" )
		RunCommandOnChildren( command );
}
*/

void ActorFrame::GainFocus( float fRate, bool bRewindMovie, bool bLoop )
{
	Actor::GainFocus( fRate, bRewindMovie, bLoop );

	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->GainFocus( fRate, bRewindMovie, bLoop );
}

void ActorFrame::LoseFocus()
{
	Actor::LoseFocus();

	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->LoseFocus();
}

void ActorFrame::PlayCommand( const CString &sCommandName )
{
	Actor::PlayCommand( sCommandName );

	for( unsigned i=0; i<m_SubActors.size(); i++ ) 
	{
		Actor* pActor = m_SubActors[i];
		pActor->PlayCommand( sCommandName );
	}
}

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
