#ifndef LOADING_WINDOW_NULL_H
#define LOADING_WINDOW_NULL_H

/* Loading window using a Windows dialog. */
#include "LoadingWindow.h"

class LoadingWindow_Null: public LoadingWindow {
public:
	LoadingWindow_Null() { }
	~LoadingWindow_Null() { }

	void SetText(CString str) { }
	void Paint() { }
};

#define HAVE_LOADING_WINDOW_NULL

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
