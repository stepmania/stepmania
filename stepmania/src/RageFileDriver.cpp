#include "global.h"
#include "RageFileDriver.h"

void RageFileObj::SetError( const CString &err )
{
	parent.SetError( err );
}

int RageFileObj::Seek( int offset )
{
	const int OldPos = parent.Tell();
	if( offset < OldPos )
		parent.Rewind();
	else
		offset -= OldPos;

	char buf[256];
	while( offset )
	{
		/* Must call parent.Read: */
		int got = parent.Read( buf, min( (int) sizeof(buf), offset ) );
		if( got == -1 )
			return -1;

		if( got == 0 )
			break;
		offset -= got;
	}

	return parent.Tell();
}

int RageFileObj::SeekCur( int offset )
{
	return Seek( parent.Tell() + offset );
}

int RageFileObj::GetFileSize()
{
	int OldPos = parent.Tell();
	parent.Rewind();
	char buf[256];
	int ret = 0;
	while( 1 )
	{
		/* Must call parent.Read: */
		int got = parent.Read( buf, sizeof(buf) );
		if( got == -1 )
		{
			ret = -1;
			break;
		}
		if( got == 0 )
			break;
		ret += got;
	}
	Seek( OldPos );
	return ret;
}

/*
 * Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
 *   Glenn Maynard
 */

