#ifndef APP_INSTANCE_H
#define APP_INSTANCE_H

/* Win32 only: get an HINSTANCE; used for starting dialog boxes. */
class AppInstance
{
	HINSTANCE h;

public:
	AppInstance();
	~AppInstance();
	HINSTANCE Get() const { return h; }
};

#endif

/*
 * Copyright (c) 2002 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
