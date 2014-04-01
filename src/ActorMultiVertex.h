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

	void LoadFromNode( const XNode* Node );
	virtual ActorMultiVertex *Copy() const;

	virtual bool EarlyAbortDraw() const;
	virtual void DrawPrimitives();

	void UpdateTweening( float fDeltaTime );
	void BeginTweening( float time, ITween *pInterp );

	void StopTweening();
	void FinishTweening();
	
	void SetTexture( RageTexture *Texture );
	void LoadFromTexture( RageTextureID ID );

	void UnloadTexture();
	void ClearVertices();
	void ReserveSpaceForMoreVertices(size_t n);
	void AddVertex();
	void AddVertices( int Add );

	void SetDrawMode( DrawMode dm )				{ _DrawMode = dm; }
	void SetEffectMode( EffectMode em)			{ _EffectMode = em; }
	void SetTextureMode( TextureMode tm)		{ _TextureMode = tm; }

	void SetVertexPos( int index , float x , float y , float z );
	void SetVertexColor( int index , RageColor c );
	void SetVertexCoords( int index , float TexCoordX , float TexCoordY );

	size_t GetNumVertices() { return AMV_current.vertices.size(); }
	virtual void PushSelf( lua_State *L );

	struct AMV_TweenState
	{
		AMV_TweenState(): line_width(1.0f) {}
		static void MakeWeightedAverage(AMV_TweenState& average_out, const AMV_TweenState& ts1, const AMV_TweenState& ts2, float percent_between);
		bool operator==(const AMV_TweenState& other) const;
		bool operator!=(const AMV_TweenState& other) const { return !operator==(other); }

		vector<RageSpriteVertex> vertices;
		
		// needed for DrawMode_LineStrip
		float line_width;
	};

	AMV_TweenState& AMV_DestTweenState()
	{
		if(AMV_Tweens.empty())
		{ return AMV_current; }
		else
		{ return AMV_Tweens.back(); }
	}
	const AMV_TweenState& AMV_DestTweenState() const { return const_cast<ActorMultiVertex*>(this)->AMV_DestTweenState(); }

private:
	RageTexture* _Texture;

	vector<RageSpriteVertex> _Vertices;
	vector<AMV_TweenState> AMV_Tweens;
	AMV_TweenState AMV_current;
	AMV_TweenState AMV_start;

	DrawMode _DrawMode;
	EffectMode _EffectMode;
	TextureMode _TextureMode;

	float _LineWidth;
};

/**
 * @file
 * @author Matthew Gardner and Eric Reese (c) 2014
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
