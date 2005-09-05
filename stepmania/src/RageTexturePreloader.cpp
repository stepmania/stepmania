/*
 * Preemptively load textures before use, by loading it and keeping
 * a reference to it.  By putting a RageTexturePreloader inside the
 * object doing the preloading, the preload will exist for the lifetime
 * of that object.
 */

#include "global.h"
#include "RageTexturePreloader.h"
#include "RageTextureManager.h"

RageTexturePreloader &RageTexturePreloader::operator=( const RageTexturePreloader &rhs )
{
	if( &rhs == this )
		return *this;

	UnloadAll();

	for( unsigned i = 0; i < rhs.m_apTextures.size(); ++i )
	{
		RageTexture *pTexture = TEXTUREMAN->CopyTexture( rhs.m_apTextures[i] );
		m_apTextures.push_back( pTexture );
	}

	return *this;
}

void RageTexturePreloader::Load( const RageTextureID &ID )
{
	ASSERT( TEXTUREMAN );

	RageTexture *pTexture = TEXTUREMAN->LoadTexture( ID );
	m_apTextures.push_back( pTexture );
}

void RageTexturePreloader::UnloadAll()
{
	if( TEXTUREMAN == NULL )
		return;

	for( unsigned i = 0; i < m_apTextures.size(); ++i )
		TEXTUREMAN->UnloadTexture( m_apTextures[i] );
	m_apTextures.clear();
}

RageTexturePreloader::~RageTexturePreloader()
{
	UnloadAll();
}

/*
 * (c) 2005 Glenn Maynard
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
