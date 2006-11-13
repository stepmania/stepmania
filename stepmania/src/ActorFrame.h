/* ActorFrame - A container for other actors. */

#ifndef ACTORFRAME_H
#define ACTORFRAME_H

#include "Actor.h"

class ActorFrame : public Actor
{
public:
	ActorFrame();
	ActorFrame( const ActorFrame &cpy );
	virtual ~ActorFrame();

	virtual void InitState();
	void LoadFromNode( const XNode* pNode );
	virtual ActorFrame *Copy() const;

	virtual void AddChild( Actor* pActor );
	virtual void RemoveChild( Actor* pActor );
	Actor* GetChild( const RString &sName );
	int GetNumChildren() const { return m_SubActors.size(); }

	void RemoveAllChildren();
	void MoveToTail( Actor* pActor );
	void MoveToHead( Actor* pActor );
	void SortByDrawOrder();
	void SetDrawByZPosition( bool b );

	virtual bool AutoLoadChildren() const { return false; } // derived classes override to automatically LoadChildrenFromNode
	void DeleteChildrenWhenDone( bool bDelete=true ) { m_bDeleteChildren = bDelete; }
	void DeleteAllChildren();

	//
	// Commands
	//
	virtual void PushSelf( lua_State *L );
	void PlayCommandOnChildren( const RString &sCommandName, const LuaReference *pParamTable = NULL );
	void PlayCommandOnLeaves( const RString &sCommandName, const LuaReference *pParamTable = NULL );
	virtual void RunCommandsOnChildren( const LuaReference& cmds, const LuaReference *pParamTable = NULL ); /* but not on self */
	void RunCommandsOnChildren( const apActorCommands& cmds, const LuaReference *pParamTable = NULL ) { this->RunCommandsOnChildren( *cmds, pParamTable ); }	// convenience
	virtual void RunCommandsOnLeaves( const LuaReference& cmds, const LuaReference *pParamTable = NULL ); /* but not on self */

	virtual void UpdateInternal( float fDeltaTime );
	virtual void ProcessMessages( float fDeltaTime );
	virtual void BeginDraw();
	virtual void DrawPrimitives();
	virtual void EndDraw();

	// propagated commands
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

	virtual void PlayCommand( const RString &sCommandName, const LuaReference *pParamTable = NULL );
	virtual void RunCommands( const LuaReference& cmds, const LuaReference *pParamTable = NULL );
	void RunCommands( const apActorCommands& cmds, const LuaReference *pParamTable = NULL ) { this->RunCommands( *cmds, pParamTable ); }	// convenience

protected:
	void LoadChildrenFromNode( const XNode* pNode );

	vector<Actor*>	m_SubActors;
	bool m_bPropagateCommands;
	bool m_bDeleteChildren;
	bool m_bDrawByZPosition;

	// state effects
	float m_fUpdateRate;
	float m_fFOV;	// -1 = no change
	float m_fVanishX;
	float m_fVanishY;
	bool m_bOverrideLighting;	// if true, set lighting to m_bLighting
	bool m_bLighting;
};

class ActorFrameAutoDeleteChildren : public ActorFrame
{
public:
	ActorFrameAutoDeleteChildren() { DeleteChildrenWhenDone(true); }
	virtual bool AutoLoadChildren() const { return true; }
	virtual ActorFrameAutoDeleteChildren *Copy() const;
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
