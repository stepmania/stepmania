/*
-----------------------------------------------------------------------------
 File: Background.h

 Desc: Background behind arrows while dancing

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _Background_H_
#define _Background_H_


#include "Sprite.h"
#include "ActorFrame.h"
#include "Song.h"


class Background : public ActorFrame
{
public:

	Background();
	virtual bool LoadFromSong( Song &song );

	virtual void Update( float fDeltaTime );
	virtual void RenderPrimitives();

	virtual void TurnDangerOn()		{ m_bShowDanger = true; };
	virtual void TurnDangerOff()	{ m_bShowDanger = false; };

protected:
	Sprite m_sprVisualizationOverlay;
	Sprite m_sprSongBackground;
	
	Sprite m_sprDanger;
	Sprite m_sprDangerBackground;

	bool m_bShowDanger;
};




#endif