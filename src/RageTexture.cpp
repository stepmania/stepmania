#include "global.h"

#include "RageTexture.h"
#include "RageUtil.h"
#include "RageTextureManager.h"

#include <cstring>
#include <vector>


RageTexture::RageTexture( RageTextureID name ):
	m_iRefCount(1), m_bWasUsed(false), m_ID(name),
	m_iSourceWidth(0), m_iSourceHeight(0),
	m_iTextureWidth(0), m_iTextureHeight(0),
	m_iImageWidth(0), m_iImageHeight(0),
	m_iFramesWide(1), m_iFramesHigh(1) {}


RageTexture::~RageTexture()
{

}


void RageTexture::CreateFrameRects()
{
	GetFrameDimensionsFromFileName( GetID().filename, &m_iFramesWide, &m_iFramesHigh, m_iSourceWidth, m_iSourceHeight );

	// Fill in the m_FrameRects with the bounds of each frame in the animation.
	m_TextureCoordRects.clear();

	for( int j=0; j<m_iFramesHigh; j++ )		// traverse along Y
	{
		for( int i=0; i<m_iFramesWide; i++ )	// traverse along X (important that this is the inner loop)
		{
			RectF frect( (i+0)/(float)m_iFramesWide*m_iImageWidth /(float)m_iTextureWidth,	// these will all be between 0.0 and 1.0
						 (j+0)/(float)m_iFramesHigh*m_iImageHeight/(float)m_iTextureHeight,
						 (i+1)/(float)m_iFramesWide*m_iImageWidth /(float)m_iTextureWidth,
						 (j+1)/(float)m_iFramesHigh*m_iImageHeight/(float)m_iTextureHeight );
			m_TextureCoordRects.push_back( frect );	// the index of this array element will be (i + j*m_iFramesWide)

			//LOG->Trace( "Adding frect%d %f %f %f %f", (i + j*m_iFramesWide), frect.left, frect.top, frect.right, frect.bottom );
		}
	}
}

void RageTexture::GetFrameDimensionsFromFileName( RString sPath, int* piFramesWide, int* piFramesHigh, int source_width, int source_height )
{
	static Regex match( " ([0-9]+)x([0-9]+)([\\. ]|$)" );
	std::vector<RString> asMatch;
	if( !match.Compare(sPath, asMatch) )
	{
		*piFramesWide = *piFramesHigh = 1;
		return;
	}
	// Check for nonsense values.  Some people might not intend the hint. -Kyz
	int maybe_width= StringToInt(asMatch[0]);
	int maybe_height= StringToInt(asMatch[1]);
	if(maybe_width <= 0 || maybe_height <= 0)
	{
		*piFramesWide = *piFramesHigh = 1;
		return;
	}
	// Font.cpp uses this function, but can't pass in a texture size.  Other
	// textures can pass in a size though, and having more frames than pixels
	// makes no sense. -Kyz
	if(source_width > 0 && source_height > 0)
	{
		if(maybe_width > source_width || maybe_height > source_height)
		{
			*piFramesWide = *piFramesHigh = 1;
			return;
		}
	}
	*piFramesWide = maybe_width;
	*piFramesHigh = maybe_height;
}

const RectF *RageTexture::GetTextureCoordRect( int iFrameNo ) const
{
	return &m_TextureCoordRects[iFrameNo % GetNumFrames()];
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the RageTexture. */
class LunaRageTexture: public Luna<RageTexture>
{
public:
	static int position( T* p, lua_State *L )		{ p->SetPosition( FArg(1) ); COMMON_RETURN_SELF; }
	static int loop( T* p, lua_State *L )			{ p->SetLooping( BIArg(1) ); COMMON_RETURN_SELF; }
	static int rate( T* p, lua_State *L )			{ p->SetPlaybackRate( FArg(1) ); COMMON_RETURN_SELF; }
	static int GetTextureCoordRect( T* p, lua_State *L )
	{
		const RectF *pRect = p->GetTextureCoordRect( IArg(1) );
		lua_pushnumber( L, pRect->left );
		lua_pushnumber( L, pRect->top );
		lua_pushnumber( L, pRect->right );
		lua_pushnumber( L, pRect->bottom );
		return 4;
	}
	static int GetNumFrames(T* p, lua_State* L)
	{
		lua_pushnumber(L, p->GetNumFrames());
		return 1;
	}
	static int Reload(T* p, lua_State* L)
	{
		p->Reload();
		COMMON_RETURN_SELF;
	}
	DEFINE_METHOD(GetSourceWidth, GetSourceWidth());
	DEFINE_METHOD(GetSourceHeight, GetSourceHeight());
	DEFINE_METHOD(GetTextureWidth, GetTextureWidth());
	DEFINE_METHOD(GetTextureHeight, GetTextureHeight());
	DEFINE_METHOD(GetImageWidth, GetImageWidth());
	DEFINE_METHOD(GetImageHeight, GetImageHeight());
	DEFINE_METHOD(GetPath, GetID().filename);

	LunaRageTexture()
	{
		ADD_METHOD( position );
		ADD_METHOD( loop );
		ADD_METHOD( rate );
		ADD_METHOD( GetTextureCoordRect );
		ADD_METHOD( GetNumFrames );
		ADD_METHOD( Reload );
		ADD_METHOD(GetSourceWidth);
		ADD_METHOD(GetSourceHeight);
		ADD_METHOD(GetTextureWidth);
		ADD_METHOD(GetTextureHeight);
		ADD_METHOD(GetImageWidth);
		ADD_METHOD(GetImageHeight);
		ADD_METHOD(GetPath);
	}
};

LUA_REGISTER_CLASS( RageTexture )
// lua end

/*
 * Copyright (c) 2001-2004 Chris Danford
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
