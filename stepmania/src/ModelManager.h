#ifndef ModelManager_H
#define ModelManager_H

/*
-----------------------------------------------------------------------------
 Class: ModelManager

 Desc: Interface for loading and releasing textures.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "RageModelGeometry.h"

#include <map>

class ModelManager
{
public:
	ModelManager();
	~ModelManager();

	RageModelGeometry* LoadMilkshapeAscii( CString sFile );
	void UnloadModel( RageModelGeometry *m );
//	void ReloadAll();

protected:

	std::map<CString, RageModelGeometry*> m_mapFileToModel;
};

extern ModelManager*	MODELMAN;	// global and accessable from anywhere in our program

#endif
