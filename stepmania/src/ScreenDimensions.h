/*
-----------------------------------------------------------------------------
 File: ScreenDimensions.h

 Desc: 

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _SCREENDIMENSIONS_H_
#define _SCREENDIMENSIONS_H_


const int SCREEN_WIDTH		=	640;
const int SCREEN_HEIGHT		=	480;

const float SCREEN_LEFT		=	0;
const float SCREEN_RIGHT	=	SCREEN_WIDTH;
const float SCREEN_TOP		=	0;
const float SCREEN_BOTTOM	=	SCREEN_HEIGHT;

const float CENTER_X		=	SCREEN_LEFT + (SCREEN_RIGHT - SCREEN_LEFT)/2.0f;
const float CENTER_Y		=	SCREEN_TOP + (SCREEN_BOTTOM - SCREEN_TOP)/2.0f;


#endif