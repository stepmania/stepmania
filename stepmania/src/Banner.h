/*
-----------------------------------------------------------------------------
 File: Banner.h

 Desc: The song's banner displayed in SelectSong.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _Banner_H_
#define _Banner_H_


#include "Sprite.h"
#include "Song.h"


const float BANNER_WIDTH	= 264;
const float BANNER_HEIGHT	= 86;


class Banner : public Sprite
{
public:
	bool LoadFromSong( Song* pSong );	// NULL means no Song
	virtual bool Load( CString sFilePath, DWORD dwHints = 0, bool bForceReload = false );

protected:
	void CropToRightSize();
};




#endif