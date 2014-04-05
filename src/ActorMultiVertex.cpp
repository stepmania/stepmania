#include "global.h"
#include <cassert>

#include "ActorMultiVertex.h"
#include "RageTextureManager.h"
#include "XmlFile.h"
#include "RageLog.h"
#include "RageDisplay.h"
#include "RageTexture.h"
#include "RageUtil.h"
#include "ActorUtil.h"
#include "Foreach.h"
#include "LuaBinding.h"
#include "LuaManager.h"
#include "LocalizedString.h"

static const char *DrawModeNames[] = {
	"Quads",
	"QuadStrip",
	"Fan",
	"Strip",
	"Triangles",
	"LineStrip",
	"SymmetricQuadStrip",
};

static const int DrawModeVertexGroupSizes[] = {
	4, // Quads
	2, // QuadStrip
	1, // Fan
	1, // Strip
	3, // Triangles
	1, // LineStrip
	3, // SymmetricQuadStrip
	1, // NUM_DrawMode
	1, // DrawMode_Invalid
};

static const int DrawModeMinimumToDraw[] = {
	4, // Quads
	4, // QuadStrip
	3, // Fan
	3, // Strip
	3, // Triangles
	2, // LineStrip
	6, // SymmetricQuadStrip
	1, // NUM_DrawMode
	1, // DrawMode_Invalid
};

XToString( DrawMode );
XToLocalizedString( DrawMode );
LuaXType( DrawMode );

REGISTER_ACTOR_CLASS( ActorMultiVertex );

ActorMultiVertex::ActorMultiVertex()
{
	// Use blank texture by default.
	RageTextureID ID = TEXTUREMAN->GetDefaultTextureID();
	_Texture = TEXTUREMAN->LoadTexture( ID );

	_DrawMode = DrawMode_Invalid;
	_EffectMode = EffectMode_Normal;
	_TextureMode = TextureMode_Modulate;
}

ActorMultiVertex::~ActorMultiVertex()
{
	UnloadTexture();
}

ActorMultiVertex::ActorMultiVertex( const ActorMultiVertex &cpy ):
	Actor( cpy )
{
#define CPY(a) a = cpy.a
	CPY( AMV_Tweens );
	CPY( AMV_current );
	CPY( AMV_start );
	CPY( _DrawMode );
	CPY( _EffectMode );
	CPY( _TextureMode );
#undef CPY

	if( cpy._Texture != NULL )
	{
		_Texture = TEXTUREMAN->CopyTexture( cpy._Texture );
	}
	else
	{
		_Texture = NULL;
	}
}

void ActorMultiVertex::LoadFromNode( const XNode* Node )
{
	RString path;
	Node->GetAttrValue( "Texture", path );
	if( !path.empty() && !TEXTUREMAN->IsTextureRegistered( RageTextureID(path) ) )
	{
		ActorUtil::GetAttrPath( Node, "Texture", path );
	}
	if( !path.empty() )
	{
		LoadFromTexture( path );
	}

	Actor::LoadFromNode( Node );
}

void ActorMultiVertex::SetTexture( RageTexture *Texture )
{
	if( _Texture != Texture )
	{
		UnloadTexture();
		_Texture = Texture;
	}
}

void ActorMultiVertex::LoadFromTexture( RageTextureID ID )
{
	RageTexture *Texture = NULL;
	if( _Texture && _Texture->GetID() == ID )
	{
		return;
	}
	else
	{
		Texture = TEXTUREMAN->LoadTexture( ID );
	}
	SetTexture( Texture );
}

void ActorMultiVertex::UnloadTexture()
{
	if( _Texture != NULL )
	{
		TEXTUREMAN->UnloadTexture( _Texture );
		_Texture = NULL;
	}
}

void ActorMultiVertex::SetNumVertices( size_t n )
{
	if( n == 0 )
	{
		for( size_t i = 0; i < AMV_Tweens.size(); ++i )
		{
			AMV_Tweens[i].vertices.clear();
		}
		AMV_current.vertices.clear();
		AMV_start.vertices.clear();
	}
	else
	{
		for( size_t i = 0; i < AMV_Tweens.size(); ++i )
		{
			AMV_Tweens[i].vertices.resize( n );
		}
		AMV_current.vertices.resize( n );
		AMV_start.vertices.resize( n );
	}
}

void ActorMultiVertex::AddVertex()
{
	for( size_t i = 0; i < AMV_Tweens.size(); ++i )
	{
		AMV_Tweens[i].vertices.push_back( RageSpriteVertex() );
	}
	AMV_current.vertices.push_back( RageSpriteVertex() );
	AMV_start.vertices.push_back( RageSpriteVertex() );
}

void ActorMultiVertex::AddVertices( int Add )
{
	int size = AMV_DestTweenState().vertices.size();
	size += Add;
	for( size_t i = 0; i < AMV_Tweens.size(); ++i )
	{
		AMV_Tweens[i].vertices.resize( size );
	}
	AMV_current.vertices.resize( size );
	AMV_start.vertices.resize( size );
}

void ActorMultiVertex::SetVertexPos( int index, float x, float y, float z )
{
	AMV_DestTweenState().vertices[index].p = RageVector3( x, y, z );
}

void ActorMultiVertex::SetVertexColor( int index, RageColor c )
{
	AMV_DestTweenState().vertices[index].c = c;
}

void ActorMultiVertex::SetVertexCoords( int index, float TexCoordX, float TexCoordY )
{
	AMV_DestTweenState().vertices[index].t = RageVector2( TexCoordX, TexCoordY );
}

void ActorMultiVertex::DrawPrimitives()
{
	Actor::SetGlobalRenderStates();	// set Actor-specified render states

	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( TextureUnit_1, _Texture->GetTexHandle() );

	Actor::SetTextureRenderStates();
	DISPLAY->SetEffectMode( _EffectMode );
	DISPLAY->SetTextureMode( TextureUnit_1, _TextureMode );

	int FirstToDraw = AMV_current.FirstToDraw;
	int NumToDraw = AMV_current.GetSafeNumToDraw(_DrawMode);

	if( NumToDraw == 0 )
	{
		// Nothing to draw.
		return;
	}
	
	switch( _DrawMode )
	{
		case DrawMode_Quads:
		{
			DISPLAY->DrawQuads( &AMV_current.vertices[FirstToDraw], NumToDraw );
			break;
		}
		case DrawMode_QuadStrip:
		{
			DISPLAY->DrawQuadStrip( &AMV_current.vertices[FirstToDraw], NumToDraw );
			break;
		}
		case DrawMode_Fan:
		{
			DISPLAY->DrawFan( &AMV_current.vertices[FirstToDraw], NumToDraw );
			break;
		}
		case DrawMode_Strip:
		{
			DISPLAY->DrawStrip( &AMV_current.vertices[FirstToDraw], NumToDraw );
			break;
		}
		case DrawMode_Triangles:
		{
			DISPLAY->DrawTriangles( &AMV_current.vertices[FirstToDraw], NumToDraw );
			break;
		}
		case DrawMode_LineStrip:
		{
			DISPLAY->DrawLineStrip( &AMV_current.vertices[FirstToDraw], NumToDraw, AMV_current.line_width );
			break;
		}
		case DrawMode_SymmetricQuadStrip:
		{
			DISPLAY->DrawSymmetricQuadStrip( &AMV_current.vertices[FirstToDraw], NumToDraw );
			break;
		}
	}

	DISPLAY->SetEffectMode( EffectMode_Normal );
}

bool ActorMultiVertex::EarlyAbortDraw() const
{
	if( AMV_current.FirstToDraw >= (int) AMV_current.vertices.size() || _DrawMode >= NUM_DrawMode )
	{
		return true;
	}
	return false;
}

void ActorMultiVertex::UpdateInternal( float fDeltaTime )
{
	if( CheckValidity )
	{
		if( AMV_Tweens.empty() )
		{
			AMV_current.CheckValidity( _DrawMode );
		}
		else
		{
			for( size_t i = 0; i < AMV_Tweens.size(); ++i )
			{
				AMV_Tweens[i].CheckValidity( _DrawMode );
			}
		}
		CheckValidity = false;
	}
	Actor::UpdateInternal( fDeltaTime );
}

void ActorMultiVertex::UpdateTweening( float fDeltaTime )
{
	// Preserve time to send to Actor::UpdateTweening()
	float Time = fDeltaTime;
	// Walk through TweenStates without changing Actor's TweenInfo
	size_t TweenIndex = 0;
	float TimeIntoTween = 0;

	while( AMV_Tweens.size() > TweenIndex && Time != 0 )
	{
		AMV_TweenState &TS = AMV_Tweens[TweenIndex];
		TweenInfo  &TI = m_Tweens[TweenIndex]->info;

		float TimeLeftInTween = TI.m_fTimeLeftInTween - TimeIntoTween;
		bool Beginning = TimeLeftInTween == TI.m_fTweenTime;

		float SecsToSubtract = min( TimeLeftInTween, Time );
		TimeLeftInTween -= SecsToSubtract;

		Time -= SecsToSubtract;

		if( Beginning )			// we are just beginning this tween
			AMV_start = AMV_current;	// set the start position

		if( TimeLeftInTween == 0 )	// Current tween is over. Stop.
		{
			AMV_current = TS;
			TimeIntoTween = 0;
			TweenIndex++;
		}
		else	// in the middle of tweening. Recalcute the current position.
		{
			const float PercentThroughTween = 1 - ( TimeLeftInTween / TI.m_fTweenTime );

			// distort the percentage if appropriate
			float PercentAlongPath = TI.m_pTween->Tween( PercentThroughTween );
			AMV_TweenState::MakeWeightedAverage( AMV_current, AMV_start, TS, PercentAlongPath );
		}
	}
	// Take out any TweenStates that have passed.
	for( size_t i = 0; i < TweenIndex; ++i )
	{
		AMV_Tweens.erase( AMV_Tweens.begin() );
	}
	// update Actor
	Actor::UpdateTweening( fDeltaTime );
}

void ActorMultiVertex::BeginTweening( float time, ITween *pTween )
{
	Actor::BeginTweening( time, pTween );

	if( AMV_Tweens.size() >= 1 )		// if there was already a TS on the stack
	{
		AMV_Tweens.push_back( AMV_Tweens.back() );
	}
	else
	{
		AMV_Tweens.push_back( AMV_current );
	}
}

void ActorMultiVertex::StopTweening()
{
	AMV_Tweens.clear();
	Actor::StopTweening();
}

void ActorMultiVertex::FinishTweening()
{
	if( !AMV_Tweens.empty() )
	{
		AMV_current = AMV_DestTweenState();
	}
	Actor::FinishTweening();
}

void ActorMultiVertex::AMV_TweenState::MakeWeightedAverage(AMV_TweenState& average_out, const AMV_TweenState& ts1, const AMV_TweenState& ts2, float percent_between)
{
	average_out.line_width= lerp(percent_between, ts1.line_width, ts2.line_width);
	for(size_t v= 0; v < average_out.vertices.size(); ++v)
	{
		WeightedAvergeOfRSVs(average_out.vertices[v], ts1.vertices[v], ts2.vertices[v], percent_between);
	}
}

int ActorMultiVertex::AMV_TweenState::GetSafeNumToDraw( DrawMode dm ) const
{
	int num = NumToDraw;
	int max = vertices.size() - FirstToDraw;
	if( num == -1 || num > max )
	{
		num = max;
	}
	num -= ( num % DrawModeVertexGroupSizes[dm]);
	if( num < DrawModeMinimumToDraw[dm])
	{
		num = 0;
	}
	return num;
}

void ActorMultiVertex::AMV_TweenState::CheckValidity( DrawMode dm )
{
	if( FirstToDraw >= (int) vertices.size() )
	{	
		LOG->Warn("ActorMultiVertex: FirstToDraw >= vertices.size(), %i >= %i", FirstToDraw, (int) vertices.size() );
	}
	int num = GetSafeNumToDraw( dm );
	if( NumToDraw != num && NumToDraw != -1 )
	{
		LOG->Warn("ActorMultiVertex: NumToDraw %i is not valid for %i vertices with DrawMode %s", NumToDraw, (int) vertices.size(), DrawModeNames[dm] );
	}
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ActorMultiVertex. */ 
class LunaActorMultiVertex: public Luna<ActorMultiVertex>
{
public:
	static int SetNumVertices( T* p, lua_State *L )
	{
		p->CheckValidity = true;
		p->SetNumVertices( IArg(1) );
	}
	static int GetNumVertices( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetNumVertices() ); return 1; }

	static void SetVertexFromStack(T* p, lua_State* L, size_t VertexIndex, int DataStackIndex)
	{
		if(lua_type(L, DataStackIndex) != LUA_TTABLE)
		{
			LOG->Warn("ActorMultiVertex::SetVertex: non-table parameter supplied. Table of tables of vertex data expected.");
			return;
		}
		size_t NumDataParts = lua_objlen(L, DataStackIndex);
		for(size_t i = 0; i < NumDataParts; ++i)
		{
			lua_pushnumber(L, i+1);
			lua_gettable(L, DataStackIndex);
			int DataPieceIndex = lua_gettop(L);
			size_t DataPieceElements = lua_objlen(L, DataPieceIndex);
			if(lua_type(L, DataPieceIndex) != LUA_TTABLE)
			{
				LOG->Warn( "ActorMultiVertex::SetVertex: non-table parameter %d supplied inside table of parameters, table expected.", (int)i );
				return;
			}
			int pushes = 1;
			if(DataPieceElements == 2)
			{
				pushes += 2;
				lua_rawgeti(L, DataPieceIndex, 1);
				float x= FArg(-1);
				lua_rawgeti(L, DataPieceIndex, 2);
				float y= FArg(-1);
				p->SetVertexCoords(VertexIndex, x, y);
			}
			else if(DataPieceElements == 3)
			{
				pushes += 3;
				lua_rawgeti(L, DataPieceIndex, 1);
				float x= FArg(-1);
				lua_rawgeti(L, DataPieceIndex, 2);
				float y= FArg(-1);
				lua_rawgeti(L, DataPieceIndex, 3);
				float z= FArg(-1);
				p->SetVertexPos(VertexIndex, x, y, z);
			}
			else if(DataPieceElements == 4)
			{
				// RageColor pops the things it pushes onto the stack, so we don't need to.
				RageColor c;
				// Does not use FromStackCompat because we are not compatible with passing a color in non-table form.
				c.FromStack(L, DataPieceIndex);
				p->SetVertexColor(VertexIndex, c);
			}
			else
			{
				LOG->Warn( "ActorMultiVertex::SetVertex: Parameter %d has %i elements supplied. 2, 3, or 4 expected.", (int)i, (int)DataPieceElements );

			}
			// Avoid a stack underflow by only popping the amount we pushed.
			lua_pop(L, pushes);
		}
		return;
	}

	static int SetVertex(T* p, lua_State* L)
	{
		size_t Index = 0;
		if(lua_type(L, 1) == LUA_TNUMBER)
		{
			// Indices from Lua are one-indexed.  -1 to adjust.
			Index = IArg(1)-1;
		}
		if( Index == p->GetNumVertices() )
		{
			p->CheckValidity = true;
			p->AddVertices( 1 );
		}
		else if( Index > p->GetNumVertices() )
		{
			LOG->Warn( "ActorMultiVertex::SetVertex: index %d too out of range.", (int)Index );
			return 0;
		}
		SetVertexFromStack(p, L, Index, lua_gettop(L));
		return 0;
	}

	static int SetVertices(T* p, lua_State* L)
	{
		size_t First = 0;
		int StackIndex = lua_gettop(L);
		// Allow the user to just pass a table without specifying a starting point.
		if(lua_type(L, 1) == LUA_TNUMBER)
		{
			// Indices from Lua are one-indexed.  -1 to adjust.
			First = IArg(1)-1;
		}
		size_t Last = First + lua_objlen(L, StackIndex );
		if( Last > p->GetNumVertices())
		{
			p->CheckValidity = true;
			p->AddVertices( Last - p->GetNumVertices() );
		}
		for(size_t n = First; n < Last; ++n)
		{
			lua_pushnumber(L, n-First+1);
			lua_gettable(L, StackIndex);
			SetVertexFromStack(p, L, n, lua_gettop(L));
			lua_pop(L, 1);
		}
		return 0;
	}

	static int SetDrawMode( T* p, lua_State *L )
	{
		p->CheckValidity = true;
		DrawMode dm = Enum::Check<DrawMode>(L, 1);
		p->SetDrawMode( dm );
		return 0;
	}

	static int SetEffectMode( T* p, lua_State *L )
	{
		EffectMode em = Enum::Check<EffectMode>(L, 1);
		p->SetEffectMode( em );
		return 0;
	}

	static int SetTextureMode( T* p, lua_State *L )
	{
		TextureMode tm = Enum::Check<TextureMode>(L, 1);
		p->SetTextureMode( tm );
		return 0;
	}

	static int SetLineWidth( T* p, lua_State *L )
	{
		p->SetLineWidth(FArg(1));
		return 0;
	}

	static int SetFirstToDraw( T* p, lua_State *L )
	{
		// Indices from Lua are one-indexed.  -1 to adjust.
		p->CheckValidity = true;
		p->SetFirstToDraw(IArg(1) - 1);
		return 0;
	}

	static int SetNumToDraw( T* p, lua_State* L )
	{
		p->CheckValidity = true;
		p->SetNumToDraw(IArg(1));
		return 0;
	}

	static int GetFirstToDraw( T* p, lua_State* L )
	{
		// Indices in Lua are one-indexed.  +1 to adjust.
		lua_pushnumber(L, p->GetFirstToDraw()+1);
		return 1;
	}

	static int GetNumToDraw( T* p, lua_State* L )
	{
		lua_pushnumber(L, p->GetNumToDraw());
		return 1;
	}
	
	static int LoadTexture( T* p, lua_State *L )
	{
		if( lua_isnil(L, 1) )
		{
			p->UnloadTexture();
		}
		else
		{
			RageTextureID ID( SArg(1) );
			p->LoadFromTexture( ID );
		}
		return 0;
	}

	static int SetTexture( T* p, lua_State *L )
	{
		RageTexture *Texture = Luna<RageTexture>::check(L, 1);
		Texture = TEXTUREMAN->CopyTexture( Texture );
		p->SetTexture( Texture );
		return 0;
	}

	LunaActorMultiVertex()
	{
		ADD_METHOD( SetNumVertices );
		ADD_METHOD( GetNumVertices );

		ADD_METHOD( SetVertex );
		ADD_METHOD( SetVertices );

		ADD_METHOD( SetDrawMode );
		ADD_METHOD( SetEffectMode );
		ADD_METHOD( SetTextureMode );
		ADD_METHOD( SetLineWidth );

		ADD_METHOD( SetFirstToDraw );
		ADD_METHOD( SetNumToDraw );
		ADD_METHOD( GetFirstToDraw );
		ADD_METHOD( GetNumToDraw );

		// Copy from RageTexture
		ADD_METHOD( SetTexture );
		// Load from file path
		ADD_METHOD( LoadTexture );
	}
};

LUA_REGISTER_DERIVED_CLASS( ActorMultiVertex, Actor )

/*
 * (c) 2014 Matthew Gardner and Eric Reese
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
