#ifndef ERROR_DIALOG_STDOUT_H
#define ERROR_DIALOG_STDOUT_H

#include "ErrorDialog.h"

class ErrorDialog_stdout: public ErrorDialog
{
public:
	void ShowError();
};

#undef ARCH_ERROR_DIALOG
#define ARCH_ERROR_DIALOG ErrorDialog_stdout

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
