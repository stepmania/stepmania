#pragma once
/*
-----------------------------------------------------------------------------
 Class: MsdFile

 Desc: Wrapper for reading and writing an .sm, .dwi, .or msd file.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


const int MAX_VALUES = 50;
const int MAX_PARAMS_PER_VALUE = 10;

class MsdFile  
{
public:
	MsdFile();

	//default destructor
	virtual ~MsdFile();

	//reads ini file specified using MsdFile::SetPath()
	//returns true if successful, false otherwise
	bool ReadFile( CString sFilePath );

	CString m_sParams[MAX_VALUES][MAX_PARAMS_PER_VALUE];

	int m_iNumValues;	// tells how many values are valid
	int m_iNumParams[MAX_VALUES];	// tells how many params this value has
};
