#ifndef LRCFILE_H
#define LRCFILE_H
/*
-----------------------------------------------------------------------------
 Class: LRCFile

 Desc: Wrapper for reading an LRC Lyrics file.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
	Kevin Slaughter
-----------------------------------------------------------------------------
*/


class LRCFile  
{
public:
	/* #param:param:param:param; <- one whole value */
	struct value_t
	{
		vector<CString> params;

		CString operator[](unsigned i) const { if(i >= params.size()) return ""; return params[i]; }
	};

	virtual ~LRCFile() { }

	//returns true if successful, false otherwise
	bool ReadFile( CString sFilePath );

	unsigned GetNumValues() const { return values.size(); }
	unsigned GetNumParams(unsigned val) const { if(val >= GetNumValues()) return 0; return values[val].params.size(); }
	const value_t &GetValue(unsigned val) const { ASSERT(val < GetNumValues()); return values[val]; }
	CString GetParam(unsigned val, unsigned par) const;

private:
	void ReadBuf( char *buf, int len );
	void AddParam( const CString &buf );
	void AddValue();

	vector<value_t> values;
};

#endif
