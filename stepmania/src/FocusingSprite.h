/*
-----------------------------------------------------------------------------
 File: FocusingSprite.h

 Desc: A graphic that appears to blur and come into focus.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _FocusingSprite_H_
#define _FocusingSprite_H_


#include "Sprite.h"
#include "ActorFrame.h"


class FocusingSprite : public ActorFrame
{
public:

	FocusingSprite();

	virtual bool Load( CString sFilePath )
	{
		for( int i=0; i<3; i++ )
			m_sprites[i].Load( sFilePath );
		return true;
	};

	virtual void Update( float fDeltaTime );
	virtual void RenderPrimitives();

	void StartFocusing();
	void StartBlurring();

protected:
	Sprite m_sprites[3];
	float m_fPercentBlurred;
	enum BlurState { focused, focusing, invisible, blurring };
	BlurState m_BlurState;
};




#endif