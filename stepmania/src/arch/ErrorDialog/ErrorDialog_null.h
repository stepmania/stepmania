#ifndef ERROR_DIALOG_NULL_H
#define ERROR_DIALOG_NULL_H

#include "ErrorDialog.h"

class ErrorDialog_null: public ErrorDialog
{
public:
	void ShowError() { }
};

#undef ARCH_ERROR_DIALOG
#define ARCH_ERROR_DIALOG ErrorDialog_null

#endif

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
