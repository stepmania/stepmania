#ifndef LOADING_WINDOW_WIN32_H
#define LOADING_WINDOW_WIN32_H

/* Loading window using a Windows dialog. */
#include "LoadingWindow.h"
#include <windows.h>
#include "../../archutils/Win32/AppInstance.h"

class LoadingWindow_Win32: public LoadingWindow {
	AppInstance handle;
	HWND hwnd;
	CString text;

	static BOOL CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

public:

	LoadingWindow_Win32();
	~LoadingWindow_Win32();

	void SetText(CString str);
	void Paint();
};

#undef ARCH_LOADING_WINDOW
#define ARCH_LOADING_WINDOW LoadingWindow_Win32

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
