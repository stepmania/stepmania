#include "global.h"	// testing updates
/*
-----------------------------------------------------------------------------
 Class: ActorUtil

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorUtil.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "Model.h"


Actor* MakeActor( CString sPath )
{
	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt );
	sExt.MakeLower();
	

	if( sExt=="png" ||
		sExt=="jpg" || 
		sExt=="gif" || 
		sExt=="bmp" || 
		sExt=="avi" || 
		sExt=="mpeg" || 
		sExt=="mpg" )
	{
		Sprite* pSprite = new Sprite;
		pSprite->Load( sPath );
		return pSprite;
	}
	if( sExt=="ini" )
	{
		BitmapText* pBitmapText = new BitmapText;
		pBitmapText->LoadFromFont( sPath );
		return pBitmapText;
	}
	if( sExt=="txt" )
	{
		Model* pModel = new Model;
		pModel->LoadMilkshapeAscii( sPath );
		return pModel;
	}

	ASSERT(0);
	return NULL;
}

