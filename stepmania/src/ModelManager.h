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

struct ModelManagerPrefs
{
	bool m_bDelayedUnload;

	ModelManagerPrefs()
	{
		m_bDelayedUnload = false;
	}
	ModelManagerPrefs( bool bDelayedUnload )
	{
		m_bDelayedUnload = bDelayedUnload;
	}

	bool operator!=( const ModelManagerPrefs& rhs )
	{
		return 
			m_bDelayedUnload != rhs.m_bDelayedUnload;
	}
};

class ModelManager
{
public:
	ModelManager();
	~ModelManager();

	RageModelGeometry* LoadMilkshapeAscii( CString sFile );
	void UnloadModel( RageModelGeometry *m );
//	void ReloadAll();

	bool SetPrefs( ModelManagerPrefs prefs ) { m_Prefs = prefs; return true; }
	ModelManagerPrefs GetPrefs() { return m_Prefs; }

protected:

	std::map<CString, RageModelGeometry*> m_mapFileToModel;

	ModelManagerPrefs m_Prefs;
};

extern ModelManager*	MODELMAN;	// global and accessable from anywhere in our program

#endif
