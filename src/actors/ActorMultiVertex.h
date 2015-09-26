/** @brief ActorMultiVertex - An actor with mutiple vertices. Can be used to create shapes that quads can't. */

#include "Actor.h"
#include "CubicSpline.h"
#include "RageDisplay.h"
#include "RageMath.h"
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
	static const size_t num_vert_splines= 4;
	ActorMultiVertex();
	ActorMultiVertex( const ActorMultiVertex &cpy );
	virtual ~ActorMultiVertex();

	void LoadFromNode( const XNode* Node );
	virtual ActorMultiVertex *Copy() const;

	struct AMV_TweenState
	{
	AMV_TweenState(): _DrawMode(DrawMode_Invalid), FirstToDraw(0),
			NumToDraw(-1), line_width(1.0f)
		{}
		static void MakeWeightedAverage(AMV_TweenState& average_out, const AMV_TweenState& ts1, const AMV_TweenState& ts2, float percent_between);
		bool operator==(const AMV_TweenState& other) const;
		bool operator!=(const AMV_TweenState& other) const { return !operator==(other); }

		void SetDrawState( DrawMode dm, int first, int num );
		int GetSafeNumToDraw( DrawMode dm, int num ) const;

		vector<RageSpriteVertex> vertices;
		vector<size_t> quad_states;

		DrawMode _DrawMode;
		int FirstToDraw;
		int NumToDraw;

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

	virtual void EnableAnimation(bool bEnable);
	virtual void Update(float fDelta);
	virtual bool EarlyAbortDraw() const;
	virtual void DrawPrimitives();
	virtual void DrawInternal( const AMV_TweenState *TS );
	
	void SetCurrentTweenStart();
	void EraseHeadTween();
	void UpdatePercentThroughTween( float PercentThroughTween );
	void BeginTweening( float time, ITween *pInterp );

	void StopTweening();
	void FinishTweening();
	
	void SetTexture( RageTexture *Texture );
	RageTexture* GetTexture() { return _Texture; };
	void LoadFromTexture( RageTextureID ID );

	void UnloadTexture();
	void SetNumVertices( size_t n );

	void AddVertex();
	void AddVertices( int Add );

	void SetEffectMode( EffectMode em)			{ _EffectMode = em; }
	void SetTextureMode( TextureMode tm)		{ _TextureMode = tm; }
	void SetLineWidth( float width)				{ AMV_DestTweenState().line_width = width; }

	void SetDrawState( DrawMode dm, int first, int num ) { AMV_DestTweenState().SetDrawState(dm, first, num); }

	DrawMode GetDestDrawMode() const { return AMV_DestTweenState()._DrawMode; }
	int GetDestFirstToDraw() const					{ return AMV_DestTweenState().FirstToDraw; }
	int GetDestNumToDraw() const					{ return AMV_DestTweenState().NumToDraw; }
	DrawMode GetCurrDrawMode() const { return AMV_current._DrawMode; }
	int GetCurrFirstToDraw() const					{ return AMV_current.FirstToDraw; }
	int GetCurrNumToDraw() const					{ return AMV_current.NumToDraw; }
	size_t GetNumVertices() 					{ return AMV_DestTweenState().vertices.size(); }
	
	void SetVertexPos( int index , float x , float y , float z );
	void SetVertexColor( int index , RageColor c );
	void SetVertexCoords( int index , float TexCoordX , float TexCoordY );

	inline void SetVertsFromSplinesInternal(size_t num_splines, size_t start_vert);
	void SetVertsFromSplines();
	CubicSplineN* GetSpline(size_t i);

	struct State
	{
		RectF rect;
		float delay;
	};
	int GetNumStates() const { return _states.size(); }
	void AddState(const State& new_state) { _states.push_back(new_state); }
	void RemoveState(size_t i)
	{ ASSERT(i < _states.size()); _states.erase(_states.begin()+i); }
	size_t GetState() { return _cur_state; }
	State& GetStateData(size_t i)
	{ ASSERT(i < _states.size()); return _states[i]; }
	void SetStateData(size_t i, const State& s)
	{ ASSERT(i < _states.size()); _states[i]= s; }
	void SetStateProperties(const vector<State>& new_states)
	{ _states= new_states; SetState(0); }
	void SetState(size_t i);
	void SetAllStateDelays(float delay);
	float GetAnimationLengthSeconds() const;
	void SetSecondsIntoAnimation(float seconds);
	void UpdateAnimationState(bool force_update= false);
	size_t GetNumQuadStates() const
	{ return AMV_DestTweenState().quad_states.size(); }
	void AddQuadState(size_t s)
	{ AMV_DestTweenState().quad_states.push_back(s); }
	void RemoveQuadState(size_t i)
	{ AMV_DestTweenState().quad_states.erase(AMV_DestTweenState().quad_states.begin()+i); }
	size_t GetQuadState(size_t i)
	{ return AMV_DestTweenState().quad_states[i]; }
	void SetQuadState(size_t i, size_t s)
	{ AMV_DestTweenState().quad_states[i]= s; }
	bool _use_animation_state;
	bool _decode_movie;

	virtual void PushSelf( lua_State *L );

private:
	RageTexture* _Texture;

	vector<RageSpriteVertex> _Vertices;
	vector<AMV_TweenState> AMV_Tweens;
	AMV_TweenState AMV_current;
	AMV_TweenState AMV_start;

	// required to handle diffuse and glow
	AMV_TweenState *AMV_TempState;
	
	EffectMode _EffectMode;
	TextureMode _TextureMode;

	// Four splines for controlling vert positions, because quads drawmode
	// requires four. -Kyz
	vector<CubicSplineN> _splines;

	bool _skip_next_update;
	float _secs_into_state;
	size_t _cur_state;
	vector<State> _states;
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
