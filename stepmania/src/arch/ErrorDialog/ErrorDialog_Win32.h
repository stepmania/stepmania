#ifndef ERROR_DIALOG_WIN32_H
#define ERROR_DIALOG_WIN32_H

#include "ErrorDialog.h"
#include "../../archutils/Win32/AppInstance.h"

class ErrorDialog_Win32: public ErrorDialog
{
	AppInstance handle;

public:
	void ShowError();
};

#undef ARCH_ERROR_DIALOG
#define ARCH_ERROR_DIALOG ErrorDialog_Win32

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
