#ifndef MSDFILE_H
#define MSDFILE_H
/*
-----------------------------------------------------------------------------
 Class: MsdFile

 Desc: Wrapper for reading and writing an .sm, .dwi, .or msd file.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


const unsigned MAX_VALUES = 200;
const unsigned MAX_PARAMS_PER_VALUE = 10;

class MsdFile  
{
	void ReadBuf( char *buf, int len );
	void AddParam( char *buf, int len );
	void AddValue();

	unsigned m_iNumValues;	// tells how many values are valid

public:
	MsdFile();

	//default destructor
	virtual ~MsdFile();

	//reads ini file specified using MsdFile::SetPath()
	//returns true if successful, false otherwise
	bool ReadFile( CString sFilePath );

	CString m_sParams[MAX_VALUES][MAX_PARAMS_PER_VALUE];

	/* #param:param:param:param; <- one whole value */
	unsigned m_iNumParams[MAX_VALUES];	// tells how many params this value has

	unsigned GetNumValues() const { return m_iNumValues; }
};

#endif
