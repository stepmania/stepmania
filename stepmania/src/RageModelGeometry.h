#ifndef RageModelGeometry_H
#define RageModelGeometry_H
/*
-----------------------------------------------------------------------------
 Class: RageModelGeometry

 Desc: A 3D RageModelGeometry.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"
#include "RageTypes.h"
#include "ModelTypes.h"
#include <vector>
#include <map>

class RageCompiledGeometry;

class RageModelGeometry
{
public:
	RageModelGeometry ();
	virtual ~RageModelGeometry ();

public:
	void LoadMilkshapeAscii( CString sMilkshapeAsciiFile );
	void OptimizeBones();

	int m_iRefCount;

	vector<msMesh> m_Meshes;
    RageCompiledGeometry* m_pGeometry;	// video memory copy of geometry shared by all meshes

	RageVector3 m_vMins, m_vMaxs;
};



#endif
