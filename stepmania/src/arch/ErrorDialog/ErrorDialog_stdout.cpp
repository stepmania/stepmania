#include "../../global.h"
#include "ErrorDialog_stdout.h"

void ErrorDialog_stdout::ShowError()
{
	/* Simplest "dialog" ever. */
	printf("Error: %s\n", GetErrorText().GetString());
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
