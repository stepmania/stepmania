#include "global.h"
#include "ModelManager.h"
#include "arch/MovieTexture/MovieTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageDisplay.h"

ModelManager*		MODELMAN		= NULL;

ModelManager::ModelManager()
{

}

ModelManager::~ModelManager()
{
	for( std::map<CString, RageModelGeometry*>::iterator i = m_mapFileToModel.begin();
		i != m_mapFileToModel.end(); 
		++i )
	{
		RageModelGeometry* pModel = i->second;
		if( pModel->m_iRefCount )
			LOG->Trace( "TEXTUREMAN LEAK: '%s', RefCount = %d.", i->first.c_str(), pModel->m_iRefCount );
		SAFE_DELETE( pModel );
	}
}

RageModelGeometry* ModelManager::LoadMilkshapeAscii( CString sFile )
{
	std::map<CString, RageModelGeometry*>::iterator p = m_mapFileToModel.find(sFile);
	if(p != m_mapFileToModel.end())
	{
		/* Found the texture.  Just increase the refcount and return it. */
		RageModelGeometry* pModel = p->second;
		pModel->m_iRefCount++;
		return pModel;
	}

	RageModelGeometry* pModel = new RageModelGeometry;
	pModel->LoadMilkshapeAscii( sFile );

	m_mapFileToModel[sFile] = pModel;
	return pModel;
}

void ModelManager::UnloadModel( RageModelGeometry *m )
{
	m->m_iRefCount--;
	ASSERT( m->m_iRefCount >= 0 );

	if( m->m_iRefCount )
		return; /* Can't unload textures that are still referenced. */

	for( std::map<CString, RageModelGeometry*>::iterator i = m_mapFileToModel.begin();
		i != m_mapFileToModel.end(); 
		i++ )
	{
		if( i->second == m )
		{
			if( m_Prefs.m_bDelayedUnload )
			{
				// leave this Model loaded
				return;
			}
			else
			{
				m_mapFileToModel.erase( i );	// remove map entry
				SAFE_DELETE( m );	// free the texture
				return;
			}
		}
	}

	ASSERT(0);	// we tried to delete a texture that wasn't loaded.
}

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
