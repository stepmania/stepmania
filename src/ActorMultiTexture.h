/** @brief ActorMultiTexture - A texture created from multiple textures. */

#ifndef ACTOR_MULTI_TEXTURE_H
#define ACTOR_MULTI_TEXTURE_H

#include "Actor.h"
#include "RageDisplay.h"

#include <vector>

class RageTexture;

class ActorMultiTexture: public Actor
{
public:
	ActorMultiTexture();
	ActorMultiTexture( const ActorMultiTexture &cpy );
	virtual ~ActorMultiTexture();

	void LoadFromNode( const XNode* pNode );
	virtual ActorMultiTexture *Copy() const;

	virtual bool EarlyAbortDraw() const;
	virtual void DrawPrimitives();

	void ClearTextures();
	int AddTexture( RageTexture *pTexture );
	void SetTextureMode( int iIndex, TextureMode tm );

	void SetSizeFromTexture( RageTexture *pTexture );
	void SetTextureCoords( const RectF &r );
	void SetEffectMode( EffectMode em ) { m_EffectMode = em; }

	virtual void PushSelf( lua_State *L );

private:
	EffectMode m_EffectMode;
	struct TextureUnitState
	{
		TextureUnitState(): m_pTexture(nullptr), m_TextureMode(TextureMode_Modulate) {}
		RageTexture *m_pTexture;
		TextureMode m_TextureMode;
	};
	std::vector<TextureUnitState> m_aTextureUnits;
	RectF m_Rect;
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
