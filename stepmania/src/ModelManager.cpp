#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ModelManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

	
#include "ModelManager.h"
#include "RageBitmapTexture.h"
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
			m_mapFileToModel.erase( i );	// remove map entry
			SAFE_DELETE( m );	// free the texture
			return;
		}
	}

	ASSERT(0);	// we tried to delete a texture that wasn't loaded.
}

