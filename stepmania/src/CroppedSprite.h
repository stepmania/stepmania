/*
-----------------------------------------------------------------------------
 File: CroppedSprite.h

 Desc: The song's CroppedSprite displayed in SelectSong.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _CroppedSprite_H_
#define _CroppedSprite_H_


#include "Sprite.h"
#include "Song.h"


class CroppedSprite : public Sprite
{
public:
	CroppedSprite()
	{
		m_fCropWidth = m_fCropHeight = 100;
	};

	bool Load( CString sFilePath, bool bForceReload = false, int iMipMaps = 4, int iAlphaBits = 4, bool bDither = false, bool bStretch = false )
	{
		Sprite::Load( sFilePath, bForceReload, iMipMaps, iAlphaBits, bDither, bStretch );
		CropToSize( m_fCropWidth, m_fCropHeight );
		
		return true;
	}

	void SetCroppedSize( float fWidth, float fHeight )
	{
		m_fCropWidth = fWidth;
		m_fCropHeight = fHeight;
	}

protected:
	void CropToSize( float fWidth, float fHeight );

	float m_fCropWidth, m_fCropHeight;
};




#endif