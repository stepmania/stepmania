#ifndef RAGE_DISPLAY_OGL_HELPERS_H
#define RAGE_DISPLAY_OGL_HELPERS_H

#if defined(WIN32)
#include <windows.h>
#endif

#include <GL/glew.h>

/* Import RageDisplay, for types.  Do not include RageDisplay_Legacy.h. */
#include "RageDisplay.h"

/* Windows defines GL_EXT_paletted_texture incompletely: */
#ifndef GL_TEXTURE_INDEX_SIZE_EXT
#define GL_TEXTURE_INDEX_SIZE_EXT         0x80ED
#endif

/** @brief Utilities for working with the RageDisplay. */
namespace RageDisplay_Legacy_Helpers
{
	void Init();
	RString GLToString( GLenum e );
};

class RenderTarget
{
public:
	virtual ~RenderTarget() { }
	virtual void Create( const RenderTargetParam &param, int &iTextureWidthOut, int &iTextureHeightOut ) = 0;

	virtual uintptr_t GetTexture() const = 0;

	/* Render to this RenderTarget. */
	virtual void StartRenderingTo() = 0;

	/* Stop rendering to this RenderTarget.  Update the texture, if necessary, and
	 * make it available. */
	virtual void FinishRenderingTo() = 0;

	virtual bool InvertY() const { return false; }

	const RenderTargetParam &GetParam() const { return m_Param; }

protected:
	RenderTargetParam m_Param;
};

#endif

/*
 * Copyright (c) 2001-2011 Chris Danford, Glenn Maynard, Colby Klein
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
