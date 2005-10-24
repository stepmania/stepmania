/* ModelManager - Interface for loading and releasing textures. */
#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

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

	RageModelGeometry* LoadMilkshapeAscii( const CString& sFile, bool bNeedNormals );
	void UnloadModel( RageModelGeometry *m );
//	void ReloadAll();

	bool SetPrefs( const ModelManagerPrefs& prefs );		// returns true if needs display to be reset
	const ModelManagerPrefs& GetPrefs() { return m_Prefs; }

protected:

	std::map<CString, RageModelGeometry*> m_mapFileToGeometry;

	ModelManagerPrefs m_Prefs;
};

extern ModelManager*	MODELMAN;	// global and accessable from anywhere in our program

#endif

/*
 * (c) 2003-2004 Chris Danford
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
