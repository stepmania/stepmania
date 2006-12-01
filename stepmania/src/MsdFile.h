/* MsdFile - Read .SM, .DWI, and .MSD files. */

#ifndef MSDFILE_H
#define MSDFILE_H

class MsdFile  
{
public:
	/* #param:param:param:param; <- one whole value */
	struct value_t
	{
		vector<RString> params;

		RString operator[]( unsigned i ) const { if( i >= params.size() ) return RString(); return params[i]; }
	};

	virtual ~MsdFile() { }

	// Returns true if successful, false otherwise.
	bool ReadFile( RString sFilePath, bool bUnescape );
	void ReadFromString( const RString &sString, bool bUnescape );

	RString GetError() const { return error; }

	unsigned GetNumValues() const { return values.size(); }
	unsigned GetNumParams( unsigned val ) const { if( val >= GetNumValues() ) return 0; return values[val].params.size(); }
	const value_t &GetValue( unsigned val ) const { ASSERT(val < GetNumValues()); return values[val]; }
	RString GetParam( unsigned val, unsigned par ) const;


private:
	void ReadBuf( const char *buf, int len, bool bUnescape );
	void AddParam( const char *buf, int len );
	void AddValue();

	vector<value_t> values;
	RString error;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
 *
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
