#ifndef NOTES_LOADER_H
#define NOTES_LOADER_H

#include <set>

class Song;

/** @brief Base class for step file loaders. */
namespace NotesLoader
{
	/**
	 * @brief Identify the main and sub titles from a full title.
	 * @param sFullTitle the full title.
	 * @param sMainTitleOut the eventual main title.
	 * @param sSubTitleOut the ventual sub title. */
	void GetMainAndSubTitlesFromFullTitle( const RString &sFullTitle, 
					      RString &sMainTitleOut, RString &sSubTitleOut );
	/**
	 * @brief Attempt to load a Song from the given directory.
	 * @param sPath the path to the file.
	 * @param out the Song in question.
	 * @param BlacklistedImages images to exclude (DWI files only for some reason).
	 * @return its success or failure. */
	bool LoadFromDir( const RString &sPath, Song &out, set<RString> &BlacklistedImages, bool load_autosave= false );
}

#endif

/**
 * @file
 * @author Chris Danford, Glenn Maynard, Steve Checkoway (c) 2001-2003,2007
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
