#ifndef LOADING_WINDOW_NULL_H
#define LOADING_WINDOW_NULL_H

/* Loading window using a Windows dialog. */
#include "LoadingWindow.h"

class LoadingWindow_Null: public LoadingWindow {
public:
	LoadingWindow_Win32() { }
	~LoadingWindow_Win32() { }

	void SetText(CString str) { }
	void Paint() { }
};

#undef ARCH_LOADING_WINDOW
#define ARCH_LOADING_WINDOW LoadingWindow_Null

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
