#ifndef RageFile_H
#define RageFile_H

/*
-----------------------------------------------------------------------------
 Class: RageFile

 Desc: Encapsulates C and C++ file classes to deal with arch-specific oddities.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include <fstream>
using namespace std;	// using "std::ifstream" causes problems below in VC6.  Why?!?

void FixSlashesInPlace( CString &sPath );
CString FixSlashes( CString sPath );

void CollapsePath( CString &sPath );

FILE* Ragefopen( const char *szPath, const char *szMode );

// replacement for ifstream
class Rageifstream : public std::ifstream
{
public:
	Rageifstream() {};
	Rageifstream( const char *szPath ) : ifstream(FixSlashes(szPath)) {}
	void open( const char *szPath ) { ifstream::open(FixSlashes(szPath)); }
};


#endif
