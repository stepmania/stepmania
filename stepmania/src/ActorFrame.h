/* ActorFrame - A container for other actors. */

#ifndef ACTORFRAME_H
#define ACTORFRAME_H

#include "Actor.h"

template<class T>
class LunaActorFrame : public LunaActor<T>
{
public:
	LunaActorFrame() { LUA->Register( Register ); }

	static int propagate( T* p, lua_State *L )			{ p->SetPropagateCommands( !!IArg(1) ); return 0; }
	static int fov( T* p, lua_State *L )				{ p->SetFOV( FArg(1) ); return 0; }
	static int SetUpdateRate( T* p, lua_State *L )		{ p->SetUpdateRate( FArg(1) ); return 0; }
	static int SetFOV( T* p, lua_State *L )				{ p->SetFOV( FArg(1) ); return 0; }
	static int GetChild( T* p, lua_State *L )
	{
		Actor *pChild = p->GetChild( SArg(1) );
		if( pChild )
			pChild->PushSelf( L );
		else
			lua_pushnil( L );
		return 1;
	}
	static int GetNumChildren( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetNumChildren() ); return 1; }
	static int SetDrawByZPosition( T* p, lua_State *L )	{ p->SetDrawByZPosition( BArg(1) ); return 1; }

	static void Register(lua_State *L) 
	{
		ADD_METHOD( propagate )
		ADD_METHOD( fov )
		ADD_METHOD( SetUpdateRate )
		ADD_METHOD( SetFOV )
		ADD_METHOD( GetChild )
		ADD_METHOD( GetNumChildren )
		ADD_METHOD( SetDrawByZPosition )
		LunaActor<T>::Register( L );
	}
};

class ActorFrame : public Actor
{
public:
	ActorFrame();
	virtual ~ActorFrame();

	void LoadFromNode( const CString& sDir, const XNode* pNode );
	void LoadChildrenFromNode( const CString& sDir, const XNode* pNode );

	virtual void AddChild( Actor* pActor );
	virtual void RemoveChild( Actor* pActor );
	Actor* GetChild( const CString &sName );
	int GetNumChildren() const { return m_SubActors.size(); }

	void RemoveAllChildren();
	void MoveToTail( Actor* pActor );
	void MoveToHead( Actor* pActor );
	void SortByDrawOrder();
	void SetDrawByZPosition( bool b );

	void DeleteChildrenWhenDone( bool bDelete=true ) { m_bDeleteChildren = bDelete; }
	void DeleteAllChildren();

	//
	// Commands
	//
	void PushSelf( lua_State *L );
	virtual void RunCommandsOnChildren( const LuaReference& cmds ); /* but not on self */
	virtual void RunCommandsOnChildren( const apActorCommands& cmds ) { RunCommandsOnChildren( *cmds ); }	// convenience
	virtual void RunCommandsOnLeaves( const LuaReference& cmds ); /* but not on self */

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	// propagated commands
	virtual void Reset();
	virtual void SetDiffuse( RageColor c );
	virtual void SetDiffuseAlpha( float f );
	virtual void SetBaseAlpha( float f );
	virtual void SetZTestMode( ZTestMode mode );
	virtual void SetZWrite( bool b );
	virtual void FinishTweening();
	virtual void HurryTweening( float factor );

	void SetUpdateRate( float fUpdateRate ) { m_fUpdateRate = fUpdateRate; }
	void SetFOV( float fFOV ) { m_fFOV = fFOV; }
	void SetVanishPoint( float fX, float fY) { m_fVanishX = fX; m_fVanishY = fY; }

	virtual void SetPropagateCommands( bool b );

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
	bool m_bDrawByZPosition;

	// state effects
	float m_fUpdateRate;
	float m_fFOV;	// -1 = no change
	float m_fVanishX;
	float m_fVanishY;
	bool m_bOverrideLighting;	// if true, set ligthing to m_bLighting
	bool m_bLighting;
};

class ActorFrameAutoDeleteChildren : public ActorFrame
{
public:
	ActorFrameAutoDeleteChildren() { DeleteChildrenWhenDone(true); }
	void LoadFromNode( const CString& sDir, const XNode* pNode )
	{
		ActorFrame::LoadFromNode( sDir, pNode );

		LoadChildrenFromNode( sDir, pNode );
	}
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
