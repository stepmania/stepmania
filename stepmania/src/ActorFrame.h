#ifndef ACTORFRAME_H
#define ACTORFRAME_H
/*
-----------------------------------------------------------------------------
 Class: ActorFrame

 Desc: A container for other actors.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"

class ActorFrame : public Actor
{
public:
	virtual void AddChild( Actor* pActor );
	virtual void RemoveChild( Actor* pActor );
	virtual void MoveToTail( Actor* pActor );
	virtual void MoveToHead( Actor* pActor );
	virtual void SortByDrawOrder();

	virtual ~ActorFrame() { }
	
	void DeleteAllChildren();

	virtual void RunCommandOnChildren( const CString &cmd ); /* but not on self */
	virtual void HandleCommand( const ParsedCommand &command );	// derivable

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();

	virtual void SetDiffuse( RageColor c );
	virtual void SetDiffuseAlpha( float f );
	
	virtual void SetZTestMode( ZTestMode mode );
	virtual void SetZWrite( bool b );
	virtual void FinishTweening();
	virtual void HurryTweening( float factor );
	
	/* Amount of time until all tweens (and all children's tweens) have stopped: */
	virtual float GetTweenTimeLeft() const;

	virtual void GainingFocus( float fRate, bool bRewindMovie, bool bLoop );
	virtual void LosingFocus();
	virtual void PlayCommand( const CString &sCommandName );

protected:
	vector<Actor*>	m_SubActors;
};

#endif
