#ifndef SMPackageUtil_H
#define SMPackageUtil_H

struct LanguageInfo;

namespace SMPackageUtil
{
	void WriteGameInstallDirs( const vector<RString>& asInstallDirsToWrite );
	void GetGameInstallDirs( vector<RString>& asInstallDirsOut );
	void AddGameInstallDir( const RString &sNewInstallDir );
	void SetDefaultInstallDir( int iInstallDirIndex );
	void SetDefaultInstallDir( const RString &sInstallDir );
	bool IsValidInstallDir( const RString &sInstallDir );

	bool GetPref( const RString &name, bool &val );
	bool SetPref( const RString &name, bool val );

	RString GetPackageDirectory( const RString &path );
	bool IsValidPackageDirectory( const RString &path );

	bool LaunchGame();

	RString GetLanguageDisplayString( const RString &sIsoCode );
	RString GetLanguageCodeFromDisplayString( const RString &sDisplayString );

	bool GetFileContentsOsAbsolute( const RString &sAbsoluteOsFile, RString &sOut );

	bool DoesOsAbsoluteFileExist( const RString &sOsAbsoluteFile ); 
}

#include "RageFile.h"

class RageFileOsAbsolute : public RageFile
{
public:
	bool Open( const RString& path, int mode = READ );
private:
	RString m_sOsDir;
};

#endif

/*
 * (c) 2002-2005 Chris Danford
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
