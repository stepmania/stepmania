/* ActorFrame - A container for other actors. */

#ifndef ACTORFRAME_H
#define ACTORFRAME_H

#include "Actor.h"

template<class T>
class LunaActorFrame : public LunaActor<T>
{
public:
	LunaActorFrame() { LUA->Register( Register ); }

	static int propagate( T* p, lua_State *L )	{ p->SetPropagateCommands( BArg(1) ); return 0; }

	static void Register(lua_State *L) 
	{
		ADD_METHOD( propagate )
		LunaActor<T>::Register( L );
	}
};

class ActorFrame : public Actor
{
public:
	ActorFrame();
	virtual ~ActorFrame();

	void LoadFromNode( const CString& sDir, const XNode* pNode );

	virtual void AddChild( Actor* pActor );
	virtual void RemoveChild( Actor* pActor );
	void MoveToTail( Actor* pActor );
	void MoveToHead( Actor* pActor );
	void SortByDrawOrder();

	void DeleteChildrenWhenDone( bool bDelete=true ) { m_bDeleteChildren = bDelete; }
	void DeleteAllChildren();

	//
	// Commands
	//
	void PushSelf( lua_State *L );
	void RunCommandsOnChildren( const LuaReference& cmds ); /* but not on self */
	void RunCommandsOnChildren( const apActorCommands& cmds ) { RunCommandsOnChildren( *cmds ); }	// convenience

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void SetDiffuse( RageColor c );
	virtual void SetDiffuseAlpha( float f );
	
	virtual void SetZTestMode( ZTestMode mode );
	virtual void SetZWrite( bool b );
	virtual void FinishTweening();
	virtual void HurryTweening( float factor );
	
	void SetPropagateCommands( bool b );

	/* Amount of time until all tweens (and all children's tweens) have stopped: */
	virtual float GetTweenTimeLeft() const;

	virtual void GainFocus( float fRate, bool bRewindMovie, bool bLoop );
	virtual void LoseFocus();
	virtual void PlayCommand( const CString &sCommandName );
	virtual void RunCommands( const LuaReference& cmds );
	void RunCommands( const apActorCommands& cmds ) { ActorFrame::RunCommands( *cmds ); }	// convenience

protected:
	vector<Actor*>	m_SubActors;
	bool m_bPropagateCommands;
	bool m_bDeleteChildren;
};

#endif

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
