#include "global.h"
#include "ActorFrameTexture.h"
#include "RageTextureRenderTarget.h"
#include "RageTextureManager.h"
#include "ActorUtil.h"

REGISTER_ACTOR_CLASS_WITH_NAME( ActorFrameTextureAutoDeleteChildren, ActorFrameTexture )
Actor *ActorFrameTexture::Copy() const { return new ActorFrameTexture(*this); }

ActorFrameTexture::ActorFrameTexture()
{
	m_bDepthBuffer = false;
	m_bAlphaBuffer = false;
	m_bPreserveTexture = false;

	m_pRenderTarget = NULL;
}

ActorFrameTexture::ActorFrameTexture( const ActorFrameTexture &cpy )
{
	FAIL_M( "ActorFrameTexture copy not implemented" );
}

ActorFrameTexture::~ActorFrameTexture()
{
	/* Release our reference to the texture. */
	TEXTUREMAN->UnloadTexture( m_pRenderTarget );
}

void ActorFrameTexture::Create()
{
	RageTextureID id( m_sTextureName );

	RenderTargetParam param;
	param.bWithDepthBuffer = m_bDepthBuffer;
	param.bWithAlpha = m_bAlphaBuffer;
	param.iWidth = (int) m_size.x;
	param.iHeight = (int) m_size.y;
	m_pRenderTarget = new RageTextureRenderTarget( id, param );

	/* This passes ownership of m_pRenderTarget to TEXTUREMAN, but we retain
	 * our reference to it until we call TEXTUREMAN->UnloadTexture. */
	TEXTUREMAN->RegisterTexture( id, m_pRenderTarget );
}

void ActorFrameTexture::DrawPrimitives()
{
	m_pRenderTarget->BeginRenderingTo( m_bPreserveTexture );

	ActorFrame::DrawPrimitives();

	m_pRenderTarget->FinishRenderingTo();
}

// lua start
#include "LuaBinding.h"

class LunaActorFrameTexture : public Luna<ActorFrameTexture>
{
public:
	static int create( T* p, lua_State *L )				{ p->Create(); return 0; }
	static int enabledepthbuffer( T* p, lua_State *L )		{ p->EnableDepthBuffer(BArg(1)); return 0; }
	static int enablealphabuffer( T* p, lua_State *L )		{ p->EnableAlphaBuffer(BArg(1)); return 0; }
	static int enablepreservetexture( T* p, lua_State *L )		{ p->EnablePreserveTexture(BArg(1)); return 0; }
	static int settexturename( T* p, lua_State *L )			{ p->SetTextureName(SArg(1)); return 0; }
	
	LunaActorFrameTexture()
	{
		ADD_METHOD( create );
		ADD_METHOD( enabledepthbuffer );
		ADD_METHOD( enablealphabuffer );
		ADD_METHOD( enablepreservetexture );
		ADD_METHOD( settexturename );
	}
};

LUA_REGISTER_DERIVED_CLASS( ActorFrameTexture, ActorFrame )
// lua end

/*
 * (c) 2006 Glenn Maynard
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
