#ifndef ERROR_DIALOG_H
#define ERROR_DIALOG_H

class ErrorDialog {
	CString ErrorText;

public:
	void SetErrorText(const CString &error) { ErrorText = error; }
	CString GetErrorText() const { return ErrorText; }

	/* This is used for showing errors from exceptions, so it might be used
	 * before SDL has been initialized (or SDL may have failed to init); don't
	 * use SDL.  If the window has been opened, it'll be hidden or closed. */
	virtual void ShowError() = 0;
	virtual ~ErrorDialog() { }
};

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
