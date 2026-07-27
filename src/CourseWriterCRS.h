/** @brief CourseWriterCRS - Writes a Course to an .CRS file. */

#ifndef COURSE_WRITER_CRS_H
#define COURSE_WRITER_CRS_H

class Course;
class CourseEntry;
class RageFileBasic;

/** @brief The Course Writer handles writing the .crs files. */
namespace CourseWriterCRS
{
	/**
	 * @brief Write the course to a file.
	 * @param course the course contents.
	 * @param f the file being built.
	 * @param bSavingCache is true if cache information is being saved as well.
	 * @return its success or failure.
	 */
	bool Write( const Course &course, RageFileBasic &f, bool bSavingCache );
	/**
	 * @brief Write the course to a file.
	 * @param course the course contents.
	 * @param sPath the path to the file.
	 * @param bSavingCache is true if cache information is being saved as well.
	 * @return its success or failure.
	 */
	bool Write( const Course &course, const RString &sPath, bool bSavingCache );
	/**
	 * @brief Retrieve course information from a file for eventual writing.
	 * @param pCourse the course file.
	 * @param sOut the path to the file.
	 */
	void GetEditFileContents( const Course *pCourse, RString &sOut );
	/**
	 * @brief Write the custom course to the machine's hard drive.
	 * @param pCourse the course file.
	 */
	void WriteEditFileToMachine( const Course *pCourse );

	bool WriteCourseEntry( const CourseEntry &entry, RageFileBasic &f );

	bool WriteSongSelectCourseEntry( const CourseEntry &entry, RageFileBasic &f );
}

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2005
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
