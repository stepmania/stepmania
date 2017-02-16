#ifndef SpecialFiles_H
#define SpecialFiles_H

/** @brief The listing of the special files and directories in use. */
namespace SpecialFiles
{
	/**
	 * @brief The user packages directory.
	 *
	 * This should be separate from system packages so that
	 * we can write to it (installing a package).
	 */
	extern const RString USER_PACKAGES_DIR;
	/** @brief The system packages directory.
	 *
	 * This is not the user packages directory. */
	extern const RString PACKAGES_DIR;
	extern const RString KEYMAPS_PATH;
	/** @brief Edit Mode keymaps are separate from standard keymaps because
	 * it should not change with the gametype, and to avoid possible
	 * interference with the normal keymaps system. -Kyz */
	extern const RString EDIT_MODE_KEYMAPS_PATH;
	extern const RString PREFERENCES_INI_PATH;
	/** @brief The directory that contains the themes. */
	extern const RString THEMES_DIR;
	/** @brief The directory that contains the different languages. */
	extern const RString LANGUAGES_SUBDIR;
	/** @brief The base language for most users of this program. */
	extern const RString BASE_LANGUAGE;
	extern const RString METRICS_FILE;
	extern const RString CACHE_DIR;
	extern const RString BASE_THEME_NAME;
	extern const RString DEFAULTS_INI_PATH;
	extern const RString STATIC_INI_PATH;
	extern const RString TYPE_TXT_FILE;
	/** @brief The default Songs directory. */
	extern const RString SONGS_DIR;
	/** @brief The default courses directory. */
	extern const RString COURSES_DIR;
	/** @brief The default noteskins directory. */
	extern const RString NOTESKINS_DIR;

	extern const RString COINS_INI;

}

#endif

/**
 * @file
 * @author Chris Danford (c) 2003-2005
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

