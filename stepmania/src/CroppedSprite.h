#ifndef CROPPEDSPRITE_H
#define CROPPEDSPRITE_H
/*
-----------------------------------------------------------------------------
 Class: CroppedSprite

 Desc: The a bitmap that is cropped and zoomed to fill a box.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"


class CroppedSprite : public Sprite
{
public:
	CroppedSprite();
	virtual ~CroppedSprite() { }

	bool Load( RageTextureID ID );
	void SetCroppedSize( float fWidth, float fHeight );
	void SetWH(float fWidth, float fHeight);	

	static bool IsDiagonalBanner( int iWidth, int iHeight );

protected:
	void CropToSize( float fWidth, float fHeight );

	float m_fCropWidth, m_fCropHeight;
};


#endif
