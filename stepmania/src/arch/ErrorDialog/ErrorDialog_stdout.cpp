#include "../../global.h"
#include "ErrorDialog_stdout.h"

void ErrorDialog_stdout::ShowError( const CString &error )
{
	/* Simplest "dialog" ever. */
	printf("Error: %s\n", error.c_str());
}

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
