#pragma once
/*
-----------------------------------------------------------------------------
 Class: CroppedSprite

 Desc: The a bitmap that is cropped and zoomed to fill a box.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "Song.h"


class CroppedSprite : public Sprite
{
public:
	CroppedSprite();

	bool Load( CString sFilePath, bool bForceReload = false, int iMipMaps = 4, int iAlphaBits = 4, bool bDither = false, bool bStretch = false );
	void SetCroppedSize( float fWidth, float fHeight );

protected:
	void CropToSize( float fWidth, float fHeight );

	float m_fCropWidth, m_fCropHeight;
};

