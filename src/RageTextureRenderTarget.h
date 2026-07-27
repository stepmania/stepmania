/* RageTextureRenderTarget - RageTexture interface for creating render targets. */

#ifndef RAGE_TEXTURE_RENDER_TARGET_H
#define RAGE_TEXTURE_RENDER_TARGET_H

#include "RageTexture.h"
#include "RageTextureID.h"
#include "RageDisplay.h" // for RenderTargetParam

#include <cstdint>

class RageTextureRenderTarget: public RageTexture
{
public:
	RageTextureRenderTarget( RageTextureID name, const RenderTargetParam &param );
	virtual ~RageTextureRenderTarget();
	virtual void Invalidate() { m_iTexHandle = 0; /* don't Destroy() */ }
	virtual void Reload();
	virtual std::uintptr_t GetTexHandle() const { return m_iTexHandle; }

	void BeginRenderingTo( bool bPreserveTexture = true );
	void FinishRenderingTo();

	virtual void PushSelf( lua_State *L );

private:
	const RenderTargetParam m_Param;

	void Create();
	void Destroy();
	std::uintptr_t m_iTexHandle;
	std::uintptr_t m_iPreviousRenderTarget;
};

#endif

/*
 * Copyright (c) 2001-2006 Glenn Maynard, Chris Danford
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
