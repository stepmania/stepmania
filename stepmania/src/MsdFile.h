#ifndef MSDFILE_H
#define MSDFILE_H
/*
-----------------------------------------------------------------------------
 Class: MsdFile

 Desc: Wrapper for reading and writing an .sm, .dwi, .or msd file.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/


class MsdFile  
{
public:
	/* #param:param:param:param; <- one whole value */
	struct value_t
	{
		vector<CString> params;

		CString operator[](unsigned i) const { if(i >= params.size()) return ""; return params[i]; }
	};

	virtual ~MsdFile() { }

	//returns true if successful, false otherwise
	bool ReadFile( CString sFilePath );

	unsigned GetNumValues() const { return values.size(); }
	unsigned GetNumParams(unsigned val) const { if(val >= GetNumValues()) return 0; return values[val].params.size(); }
	const value_t &GetValue(unsigned val) const { ASSERT(val < GetNumValues()); return values[val]; }
	CString GetParam(unsigned val, unsigned par) const;

private:
	void ReadBuf( char *buf, int len );
	void AddParam( char *buf, int len );
	void AddValue();

	vector<value_t> values;
};

#endif
