#ifndef ERROR_DIALOG_H
#define ERROR_DIALOG_H

class ErrorDialog {
public:
	/* This is used for showing errors from exceptions, so it might be used
	 * before SDL has been initialized (or SDL may have failed to init); don't
	 * use SDL.  If the window has been opened, it'll be hidden or closed. */
	virtual void ShowError( const CString &error ) = 0;
	virtual ~ErrorDialog() { }
};

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
