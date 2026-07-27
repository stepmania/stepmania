#include "global.h"
#include "ActorFrameTexture.h"
#include "RageTextureRenderTarget.h"
#include "RageTextureManager.h"
#include "RageLog.h"
#include "ActorUtil.h"

#include <cstdint>

REGISTER_ACTOR_CLASS_WITH_NAME( ActorFrameTextureAutoDeleteChildren, ActorFrameTexture );
ActorFrameTexture *ActorFrameTexture::Copy() const { return new ActorFrameTexture(*this); }

ActorFrameTexture::ActorFrameTexture()
{
	m_bDepthBuffer = false;
	m_bAlphaBuffer = false;
	m_bFloat = false;
	m_bPreserveTexture = false;
	static std::uint64_t i = 0;
	++i;
	m_sTextureName = ssprintf( ConvertI64FormatString("ActorFrameTexture %lli"), i );

	m_pRenderTarget = nullptr;
}

ActorFrameTexture::ActorFrameTexture( const ActorFrameTexture &cpy ):
	ActorFrame(cpy)
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
	if( m_pRenderTarget != nullptr )
	{
		LuaHelpers::ReportScriptError( "Can't Create an already created ActorFrameTexture" );
		return;
	}

	if( TEXTUREMAN->IsTextureRegistered( m_sTextureName ) )
	{
		LuaHelpers::ReportScriptError( "ActorFrameTexture: Texture Name already in use." );
		return;
	}

	if( (int)m_size.x < 1 || (int)m_size.y < 1 )
	{
		LuaHelpers::ReportScriptError( "ActorFrameTexture: Cannot have width or height less than 1" );
		return;
	}
	RageTextureID id( m_sTextureName );
	id.Policy = RageTextureID::TEX_VOLATILE;

	RenderTargetParam param;
	param.bWithDepthBuffer = m_bDepthBuffer;
	param.bWithAlpha = m_bAlphaBuffer;
	param.bFloat = m_bFloat;
	param.iWidth = (int) m_size.x;
	param.iHeight = (int) m_size.y;
	m_pRenderTarget = new RageTextureRenderTarget( id, param );
	m_pRenderTarget->m_bWasUsed = true;

	/* This passes ownership of m_pRenderTarget to TEXTUREMAN, but we retain
	 * our reference to it until we call TEXTUREMAN->UnloadTexture. */
	TEXTUREMAN->RegisterTexture( id, m_pRenderTarget );
}

void ActorFrameTexture::DrawPrimitives()
{
	if( m_pRenderTarget == nullptr )
		return;

	m_pRenderTarget->BeginRenderingTo( m_bPreserveTexture );

	ActorFrame::DrawPrimitives();

	m_pRenderTarget->FinishRenderingTo();
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ActorFrameTexture. */
class LunaActorFrameTexture : public Luna<ActorFrameTexture>
{
public:
	static int Create( T* p, lua_State *L )				{ p->Create(); COMMON_RETURN_SELF; }
	static int EnableDepthBuffer( T* p, lua_State *L )		{ p->EnableDepthBuffer(BArg(1)); COMMON_RETURN_SELF; }
	static int EnableAlphaBuffer( T* p, lua_State *L )		{ p->EnableAlphaBuffer(BArg(1)); COMMON_RETURN_SELF; }
	static int EnableFloat( T* p, lua_State *L )			{ p->EnableFloat(BArg(1)); COMMON_RETURN_SELF; }
	static int EnablePreserveTexture( T* p, lua_State *L )		{ p->EnablePreserveTexture(BArg(1)); COMMON_RETURN_SELF; }
	static int SetTextureName( T* p, lua_State *L )			{ p->SetTextureName(SArg(1)); COMMON_RETURN_SELF; }
	static int GetTexture( T* p, lua_State *L )
	{
		RageTexture *pTexture = p->GetTexture();
		if( pTexture == nullptr )
		{
			lua_pushnil(L);
		}
		else
		{
			pTexture->PushSelf(L);
		}
		return 1;
	}

	LunaActorFrameTexture()
	{
		ADD_METHOD( Create );
		ADD_METHOD( EnableDepthBuffer );
		ADD_METHOD( EnableAlphaBuffer );
		ADD_METHOD( EnableFloat );
		ADD_METHOD( EnablePreserveTexture );
		ADD_METHOD( SetTextureName );
		ADD_METHOD( GetTexture );
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
