/*
-----------------------------------------------------------------------------
 File: BitmapText.h

 Desc: A font class that draws characters from a bitmap.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _BITMAPTEXT_H_
#define _BITMAPTEXT_H_


//#include "Actor.h"
#include "Sprite.h"
#include "BitmapText.h"




class BitmapText : public Sprite
{
public:
	BitmapText();
//	virtual ~BitmapText();

	BOOL LoadFromFontFile( CString sFontFilePath );
	void SetText( CString sText );
	CString GetText() { return m_sText; };

	virtual void Draw();

protected:
	void ResetWidthAndHeight();

	CString m_sFontFilePath;
	CString	m_sText;	// the string that the font is displaying
	//Sprite	m_Sprite;	// holder for the graphic
	CMap<TCHAR, TCHAR&, UINT, UINT&> m_mapCharToFrameNo;
};


#endif