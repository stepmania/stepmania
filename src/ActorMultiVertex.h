/** @brief ActorMultiVertex - A texture created from multiple textures. */

#include "Actor.h"
#include "RageDisplay.h"
#include "RageTextureID.h"

enum DrawMode
{
	DrawMode_Quads = 0,
	DrawMode_QuadStrip,
	DrawMode_Fan,
	DrawMode_Strip,
	DrawMode_Triangles,
	DrawMode_LineStrip,
	DrawMode_SymmetricQuadStrip,
	NUM_DrawMode,
	DrawMode_Invalid
};

const RString& DrawModeToString( DrawMode cat );
const RString& DrawModeToLocalizedString( DrawMode cat );
LuaDeclareType( DrawMode );

class RageTexture;

class ActorMultiVertex: public Actor
{
public:
	ActorMultiVertex();
	ActorMultiVertex( const ActorMultiVertex &cpy );
	virtual ~ActorMultiVertex();

	void LoadFromNode( const XNode* pNode );
	virtual ActorMultiVertex *Copy() const;

	virtual bool EarlyAbortDraw() const;
	virtual void DrawPrimitives();

	void SetTexture( RageTexture *pTexture );
	void LoadFromTexture( RageTextureID ID );

	void UnloadTexture();
	void ClearVertices();
	void ReserveSpaceForMoreVertices(size_t n);
	void AddVertex();
	void AddVertex(float x, float y, float z);

	void SetDrawMode( DrawMode dm )				{ m_DrawMode = dm; }
	void SetEffectMode( EffectMode em)			{ m_EffectMode = em; }
	void SetTextureMode( TextureMode tm)		{ m_TextureMode = tm; }

	void SetVertexPos( int iIndex , float fX , float fY , float fZ );
	void SetVertexColor( int iIndex , RageColor c );
	void SetVertexCoords( int iIndex , float fTexCoordX , float fTexCoordY );

	// Set the last vertex without need to specify index.
	void SetPos( float fX , float fY , float fZ ) 			{ SetVertexPos( m_Vertices.size()-1 , fX , fY , fZ ); }
	void SetColor( RageColor c )							{ SetVertexColor( m_Vertices.size()-1 , c ); }
	void SetCoords( float fTexCoordX , float fTexCoordY )	{ SetVertexCoords( m_Vertices.size()-1 , fTexCoordX , fTexCoordY ); }

	int GetNumVertices() { return m_Vertices.size(); }
	virtual void PushSelf( lua_State *L );

private:
	RageTexture* m_pTexture;
	vector<RageSpriteVertex> m_Vertices;

	DrawMode m_DrawMode;
	EffectMode m_EffectMode;
	TextureMode m_TextureMode;

	// needed for DrawMode_LineStrip
	float m_fLineWidth;
};

/**
 * @file
 * @author Matthew Gardner (c) 2014
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
