/* CourseLoaderCRS - Reads a Course from an .CRS file. */

#ifndef CourseLoaderCRS_H
#define CourseLoaderCRS_H

#include "GameConstantsAndTypes.h"
class Course;
class MsdFile;

class CourseLoaderCRS
{
public:
	static bool LoadFromCRSFile( const RString &sPath, Course &out );
	static bool LoadFromMsd( const RString &sPath, const MsdFile &msd, Course &out, bool bFromCache );
	static bool LoadFromBuffer( const RString &sPath, const RString &sBuffer, Course &out );
	static bool LoadEdit( const RString &sEditFilePath, ProfileSlot slot );
	static bool LoadEditFromBuffer( const RString &sBuffer, const RString &sPath, ProfileSlot slot );
};

#endif

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
