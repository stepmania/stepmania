/** @brief DWILoader - reads a Song from a .DWI file. */

#ifndef NOTES_LOADER_DWI_H
#define NOTES_LOADER_DWI_H

#include <set>
#include <vector>


class Song;
class Steps;

/** @brief The DWILoader handles parsing the .dwi file. */
namespace DWILoader
{
	/**
	 * @brief Retrieve the list of .dwi files.
	 * @param sPath a const reference to the path on the hard drive to check.
	 * @param out a vector of files found in the path.
	 */
	void GetApplicableFiles( const RString &sPath, std::vector<RString> &out );
	/**
	 * @brief Attempt to load a song from a specified path.
	 * @param sPath a const reference to the path on the hard drive to check.
	 * @param out a reference to the Song that will retrieve the song information.
	 * @param BlacklistedImages a set of images that aren't used.
	 * @return its success or failure.
	 */
	bool LoadFromDir( const RString &sPath, Song &out, std::set<RString> &BlacklistedImages );

	bool LoadNoteDataFromSimfile( const RString &path, Steps &out );
}

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard (c) 2001-2004
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
