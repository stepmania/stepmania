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

struct msRageModelGeometry;


class RageModelGeometry
{
public:
	RageModelGeometry ();
	virtual ~RageModelGeometry ();

public:
	void LoadMilkshapeAscii( CString sMilkshapeAsciiFile );

	int m_iRefCount;

    vector<msMesh> m_Meshes;

	RageVector3 m_vMins, m_vMaxs;
};



#endif
