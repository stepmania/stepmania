#include "global.h"
#include "SpecialFiles.h"

const std::string SpecialFiles::USER_PACKAGES_DIR = "UserPackages/";
const std::string SpecialFiles::PACKAGES_DIR = "Packages/";
const std::string SpecialFiles::KEYMAPS_PATH = "Save/Keymaps.ini";
const std::string SpecialFiles::EDIT_MODE_KEYMAPS_PATH = "Save/EditMode_Keymaps.ini";
const std::string SpecialFiles::PREFERENCES_INI_PATH	= "Save/Preferences.ini";
const std::string SpecialFiles::THEMES_DIR  = "Themes/";
const std::string SpecialFiles::LANGUAGES_SUBDIR = "Languages/";
// TODO: A theme should be able to specify a base language.
const std::string SpecialFiles::BASE_LANGUAGE = "en";
const std::string SpecialFiles::METRICS_FILE = "metrics.ini";
const std::string SpecialFiles::CACHE_DIR = "Cache/";
const std::string SpecialFiles::BASE_THEME_NAME = "_fallback";
const std::string SpecialFiles::DEFAULTS_INI_PATH	= "Data/Defaults.ini";
const std::string SpecialFiles::STATIC_INI_PATH = "Data/Static.ini";
const std::string SpecialFiles::TYPE_TXT_FILE = "Data/Type.txt";
const std::string SpecialFiles::SONGS_DIR = "Songs/";
const std::string SpecialFiles::COURSES_DIR	= "Courses/";
const std::string SpecialFiles::NOTESKINS_DIR = "NoteSkins/";
const std::vector<std::string> SpecialFiles::USER_CONTENT_DIRS= {
	"/Announcers",
	"/BGAnimations",
	"/BackgroundEffects",
	"/BackgroundTransitions",
	"/CDTitles",
	"/Characters",
	"/Courses",
	"/NoteSkins",
	"/Packages",
	"/Songs",
	"/RandomMovies",
	"/Themes",
};
const std::vector<std::string> SpecialFiles::USER_DATA_DIRS= {
	// When adding entries to USER_DATA_DIRS, ArchHooks::MountUserFilesystems
	// in arch/ArchHooks_MacOSX.cpp must be edited manually because these are
	// all stored in different places on OS X. -Kyz
	"/Cache",
	"/Logs",
	"/Save",
	"/Screenshots",
};


/*
 * (c) 2003-2005 Chris Danford
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

