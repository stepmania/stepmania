/*
-----------------------------------------------------------------------------
 File: ScreenDimensions.h

 Desc: 

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _SCREENDIMENSIONS_H_
#define _SCREENDIMENSIONS_H_


#define		SCREEN_WIDTH	(640)
#define		SCREEN_HEIGHT	(480)

#define		SCREEN_LEFT		(0)
#define		SCREEN_RIGHT	(SCREEN_WIDTH)
#define		SCREEN_TOP		(0)
#define		SCREEN_BOTTOM	(SCREEN_HEIGHT)

#define		CENTER_X		(SCREEN_LEFT + (SCREEN_RIGHT - SCREEN_LEFT)/2.0f)
#define		CENTER_Y		(SCREEN_TOP + (SCREEN_BOTTOM - SCREEN_TOP)/2.0f)


#endif