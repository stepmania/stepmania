/** @brief CourseLoaderCRS - Reads a Course from an .CRS file. */

#ifndef COURSE_LOADER_CRS_H
#define COURSE_LOADER_CRS_H

#include "GameConstantsAndTypes.h"
class Course;
class MsdFile;

/** @brief The Course Loader handles parsing the .crs files. */
namespace CourseLoaderCRS
{
	/**
	 * @brief Attempt to load a course file from a particular path.
	 * @param sPath the path to the file.
	 * @param out the course file.
	 * @return its success or failure.
	 */
	bool LoadFromCRSFile( const RString &sPath, Course &out );
	/**
	 * @brief Attempt to load the course information from the msd context.
	 * @param sPath the path to the file.
	 * @param msd the MSD context.
	 * @param out the course file.
	 * @param bFromCache true if loading from the cache area.
	 * @return its success or failure.
	 */
	bool LoadFromMsd( const RString &sPath, const MsdFile &msd, Course &out, bool bFromCache );
	/**
	 * @brief Attempt to load the course file from the buffer.
	 * @param sPath the path to the file.
	 * @param sBuffer the path to the buffer.
	 * @param out the course file.
	 * @return its success or failure.
	 */
	bool LoadFromBuffer( const RString &sPath, const RString &sBuffer, Course &out );
	/**
	 * @brief Attempt to load an edit course from the hard drive.
	 * @param sEditFilePath a path on the hard drive to check.
	 * @param slot the Profile of the user with the edit course.
	 * @return its success or failure.
	 */
	bool LoadEditFromFile( const RString &sEditFilePath, ProfileSlot slot );
	/**
	 * @brief Attempt to load an edit course from the buffer.
	 * @param sBuffer the path to the buffer.
	 * @param sPath the path to the file.
	 * @param slot the individual's profile.
	 * @return its success or failure.
	 */
	bool LoadEditFromBuffer( const RString &sBuffer, const RString &sPath, ProfileSlot slot );
}

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
