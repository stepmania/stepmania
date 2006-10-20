#ifndef ACTOR_FRAME_TEXTURE_H
#define ACTOR_FRAME_TEXTURE_H

#include "ActorFrame.h"
class RageTextureRenderTarget;

class ActorFrameTexture: public ActorFrame
{
public:
	ActorFrameTexture();
	ActorFrameTexture( const ActorFrameTexture &cpy );
	virtual ~ActorFrameTexture();
	virtual ActorFrameTexture *Copy() const;

	/* Set the texture name, which can be used with RageTextureManager (and users,
	 * eg. Sprite) to load the texture.  If no name is supplied, a unique one will
	 * be generated.  In that case, the only way to access the texture is via
	 * GetTextureName. */
	void SetTextureName( const RString &sName ) { m_sTextureName = sName; }
	RString GetTextureName() const { return m_sTextureName; }

	void EnableDepthBuffer( bool b ) { m_bDepthBuffer = b; }
	void EnableAlphaBuffer( bool b ) { m_bAlphaBuffer = b; }
	void EnablePreserveTexture( bool b ) { m_bPreserveTexture = b; }

	void Create();

	virtual void DrawPrimitives();

	//
	// Commands
	//
	virtual void PushSelf( lua_State *L );

private:
	RageTextureRenderTarget *m_pRenderTarget;

	bool m_bDepthBuffer;
	bool m_bAlphaBuffer;
	bool m_bPreserveTexture;
	RString m_sTextureName;
};

class ActorFrameTextureAutoDeleteChildren : public ActorFrameTexture
{
public:
	ActorFrameTextureAutoDeleteChildren() { DeleteChildrenWhenDone(true); }
	virtual bool AutoLoadChildren() const { return true; }
	virtual ActorFrameTextureAutoDeleteChildren *Copy() const;
};

#endif

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
