#ifndef LOADING_WINDOW_H
#define LOADING_WINDOW_H

/* Driver abstract class for opening and displaying the loading banner. */
class LoadingWindow {
public:
	virtual ~LoadingWindow() { }

	virtual void Paint() = 0;
	virtual void SetText(CString str) { }
};

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
