/* ActorFrame - A container for other actors. */

#ifndef ACTORFRAME_H
#define ACTORFRAME_H

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

	virtual void GainFocus( float fRate, bool bRewindMovie, bool bLoop );
	virtual void LoseFocus();
	virtual void PlayCommand( const CString &sCommandName );

protected:
	vector<Actor*>	m_SubActors;
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
