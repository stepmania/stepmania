#ifndef SCREENMESSAGE_H
#define SCREENMESSAGE_H
/*
-----------------------------------------------------------------------------
 File: ScreenMessage

 Desc: Definition of common ScreenMessages

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


// common ScreenMessages
enum ScreenMessage {
	SM_None = 0,
	SM_DoneClosingWipingLeft,
	SM_DoneClosingWipingRight,
	SM_DoneOpeningWipingLeft,
	SM_DoneOpeningWipingRight,
	SM_MenuTimer,
	SM_BeginFadingOut,
	SM_GoToNextScreen,
	SM_GoToPrevScreen,
	SM_RegainingFocus,
	SM_User	= 100
};

#endif
