/*
-----------------------------------------------------------------------------
 File: ScreenMessage.h

 Desc: 

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _ScreenMessage_H_
#define _ScreenMessage_H_


// common ScreenMessages
enum ScreenMessage {
	SM_None = 0,
	SM_DoneClosingWipingLeft,
	SM_DoneClosingWipingRight,
	SM_DoneOpeningWipingLeft,
	SM_DoneOpeningWipingRight,
	SM_LosingInputFocus,
	SM_RegainingInputFocus,
	SM_User	= 100
};

#endif