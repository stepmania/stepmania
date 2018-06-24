#ifndef ERROR_STRINGS_H
#define ERROR_STRINGS_H

#include <string>
#include <windows.h>
#include "fmt/format.h"
#include "RageString.hpp"

template<typename... Args>
std::string werr_format(int err, std::string const &msg, Args const & ...args)
{
	char buf[1024] = "";
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		0, err, 0, buf, sizeof(buf), nullptr);

	// Why is FormatMessage returning text ending with \r\n? (who? -aj)
	// Perhaps it's because you're on Windows, where newlines are \r\n. -aj
	std::string text = buf;
	Rage::replace(text, "\n", "");
	Rage::replace(text, "\r", " "); // foo\r\nbar -> foo bar
	text = Rage::trim_right(text); // "foo\r\n" -> "foo"

	std::string s = fmt::format(msg, args...);
	return s += fmt::sprintf(" (%s)", text.c_str());
}
std::string ConvertWstringToCodepage( std::wstring s, int iCodePage );
std::string ConvertUTF8ToACP( const std::string &s );
std::wstring ConvertCodepageToWString( std::string s, int iCodePage );
std::string ConvertACPToUTF8( const std::string &s );

#endif

/*
 * Copyright (c) 2001-2005 Chris Danford, Glenn Maynard
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
