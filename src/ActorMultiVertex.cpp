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

XToString( DrawMode );
XToLocalizedString( DrawMode );
LuaXType( DrawMode );

REGISTER_ACTOR_CLASS( ActorMultiVertex );

ActorMultiVertex::ActorMultiVertex()
{
	// Use blank texture by default.
	RageTextureID ID = TEXTUREMAN->GetDefaultTextureID();
	_Texture = TEXTUREMAN->LoadTexture( ID );
	ClearVertices();

	_DrawMode = DrawMode_Invalid;
	_EffectMode = EffectMode_Normal;
	_TextureMode = TextureMode_Modulate;
}

ActorMultiVertex::~ActorMultiVertex()
{
	ClearVertices();
	UnloadTexture();
}

ActorMultiVertex::ActorMultiVertex( const ActorMultiVertex &cpy ):
	Actor( cpy )
{
#define CPY(a) a = cpy.a
	CPY( _Vertices );
	CPY( _DrawMode );
	CPY( _EffectMode );
	CPY( _TextureMode );
	CPY( _LineWidth );
#undef CPY

	if( cpy._Texture != NULL )
		_Texture = TEXTUREMAN->CopyTexture( cpy._Texture );
	else
		_Texture = NULL;

}

void ActorMultiVertex::LoadFromNode( const XNode* Node )
{
	RString path;
	Node->GetAttrValue( "Texture", path );
	if( !path.empty() && !TEXTUREMAN->IsTextureRegistered( RageTextureID(path) ) )
		ActorUtil::GetAttrPath( Node, "Texture", path );

	if( !path.empty() )
		LoadFromTexture( path );

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
		return;
	else
		Texture = TEXTUREMAN->LoadTexture( ID );

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

void ActorMultiVertex::ClearVertices()
{
	_Vertices.clear();
}

void ActorMultiVertex::ReserveSpaceForMoreVertices(size_t n)
{
	// Repeatedly adding to a vector is more efficient when you know ahead how
	// many elements are going to be added and reserve space for them.
	_Vertices.reserve(_Vertices.size() + n);
}

void ActorMultiVertex::AddVertex( lua_State *L, int Pos )
{
	RageSpriteVertex tmp;

	if( lua_type(L, Pos) != LUA_TTABLE )
	{
		return;
	}

	size_t NumTables = lua_objlen(L, Pos);
	int TableIndex = lua_gettop(L);
	for( size_t i = 0; i < NumTables; ++i )
	{
		lua_pushnumber(L, i+1);
		lua_gettable(L, TableIndex );
		size_t NumParams = lua_objlen(L, -1);
		int ParamIndex = lua_gettop(L);
		if( NumParams == 2)
		{
			lua_rawgeti( L ,ParamIndex , 1);
			tmp.t.x = lua_tonumber(L, -1);
			lua_rawgeti( L ,ParamIndex , 2);
			tmp.t.y = lua_tonumber(L, -1);
		}
		else if( NumParams == 3)
		{
			lua_rawgeti( L ,ParamIndex , 1);
			tmp.p.x = lua_tonumber(L, -1);
			lua_rawgeti( L ,ParamIndex , 2);
			tmp.p.y = lua_tonumber(L, -1);
			lua_rawgeti( L ,ParamIndex , 3);
			tmp.p.z = lua_tonumber(L, -1);
		}
		else if( NumParams == 4)
		{
			RageColor c;
			lua_rawgeti( L ,ParamIndex , 1);
			c.r = lua_tonumber(L, -1);
			lua_rawgeti( L ,ParamIndex , 2);
			c.g = lua_tonumber(L, -1);
			lua_rawgeti( L ,ParamIndex , 3);
			c.b = lua_tonumber(L, -1);
			lua_rawgeti( L ,ParamIndex , 4);
			c.a = lua_tonumber(L, -1);
			tmp.c = c;
		}
		else
		{
			LOG->Warn( "ActorMultiVertex::AddVertex: %i Parameters supplied. 2, 3, or 4 expected." , NumParams );
		}
		lua_pop( L, int(NumParams) + 1 );
	}
	_Vertices.push_back( tmp );
}

void ActorMultiVertex::AddVertex()
{
	_Vertices.push_back( RageSpriteVertex() );
}

void ActorMultiVertex::SetVertexPos( int index , float x , float y , float z )
{
	if( index >= (int) _Vertices.size() )
	{
		LOG->Warn( "ActorMultiVertex::SetVertexPos: index %d too out of range.", index );
		return;
	}
	_Vertices[index].p = RageVector3( x, y, z );
}

void ActorMultiVertex::SetVertexColor( int index , RageColor c )
{
	if( index >= (int) _Vertices.size() )
	{
		LOG->Warn( "ActorMultiVertex::SetVertexColor: index %d too out of range.", index );
		return;
	}
	_Vertices[index].c = c;
}

void ActorMultiVertex::SetVertexCoords( int index , float TexCoordX , float TexCoordY )
{
	if( index >= (int) _Vertices.size() )
	{
		LOG->Warn( "ActorMultiVertex::SetVertexCoords: index %d too out of range.", index );
		return;
	}
	_Vertices[index].t = RageVector2( TexCoordX,  TexCoordY );
}

void ActorMultiVertex::DrawPrimitives()
{
	Actor::SetGlobalRenderStates();	// set Actor-specified render states

	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( TextureUnit_1, _Texture->GetTexHandle() );
   
	Actor::SetTextureRenderStates();
	DISPLAY->SetEffectMode( _EffectMode );
	DISPLAY->SetTextureMode( TextureUnit_1, _TextureMode );

	int NumToDraw = _Vertices.size();

	switch( _DrawMode )
	{
		case DrawMode_Quads:
		{
			NumToDraw -= NumToDraw%4;
			if( NumToDraw >= 4 )
				DISPLAY->DrawQuads( &_Vertices[0] , NumToDraw );
			break;
		}
		case DrawMode_QuadStrip:
		{
			NumToDraw -= NumToDraw%2;

			if( NumToDraw >= 4 )
				DISPLAY->DrawQuadStrip( &_Vertices[0] , NumToDraw );
			break;
		}
		case DrawMode_Fan:
		{
			if( NumToDraw >= 3 )
				DISPLAY->DrawFan( &_Vertices[0] , NumToDraw );
			break;
		}
		case DrawMode_Strip:
		{
			if( NumToDraw >= 3 )
				DISPLAY->DrawStrip( &_Vertices[0] , NumToDraw );
			break;
		}
		case DrawMode_Triangles:
		{
			NumToDraw -= NumToDraw%3;
			if( NumToDraw >= 3 )
					DISPLAY->DrawTriangles( &_Vertices[0] , NumToDraw );
			break;
		}
		case DrawMode_LineStrip:
		{
			if( NumToDraw >= 2 )
				DISPLAY->DrawLineStrip( &_Vertices[0] , NumToDraw , _LineWidth );
			break;
		}
		case DrawMode_SymmetricQuadStrip:
		{
			NumToDraw -= NumToDraw%3;

			if( NumToDraw >= 6 )
				DISPLAY->DrawSymmetricQuadStrip( &_Vertices[0] , NumToDraw );
			break;

		}
	}

	DISPLAY->SetEffectMode( EffectMode_Normal );
}

bool ActorMultiVertex::EarlyAbortDraw() const
{
	return _Vertices.empty();
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ActorMultiVertex. */ 
class LunaActorMultiVertex: public Luna<ActorMultiVertex>
{
public:
	static int ClearVertices( T* p, lua_State *L )		{ p->ClearVertices( ); return 0; }
	static int GetNumVertices( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetNumVertices() ); return 1; }

	static int AddVertex( T* p, lua_State *L )
	{ 
		if( lua_type(L, 1) == LUA_TTABLE )
		{
			p->AddVertex( L, 1 );
		}
		else
		{
			p->AddVertex( ); 
		}
		return 0; 
	}

	static int AddVertices( T* p, lua_State *L )
	{
		size_t NumVerts = lua_objlen(L, -1);
		p->ReserveSpaceForMoreVertices( NumVerts );
		int TableIndex = lua_gettop(L);
		for(size_t n= 0; n < NumVerts; ++n)
		{
			lua_pushnumber(L, n+1);
			lua_gettable(L, TableIndex);
			p->AddVertex( L , -1 );
			lua_pop(L, 1);
		}
		return 0;
	}

	static int SetDrawMode( T* p, lua_State *L )
	{
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

	static int SetVertexPos( T* p, lua_State *L )		{ p->SetVertexPos(IArg(1),FArg(2),FArg(3),FArg(4)); return 0; }
	static int SetVertexColor( T* p, lua_State *L )		{ RageColor c; c.FromStackCompat( L, 2 ); p->SetVertexColor( IArg(1), c ); return 0; }
	static int SetVertexCoords( T* p, lua_State *L )	{ p->SetVertexCoords(IArg(1),FArg(2),FArg(3)); return 0; }

	static int SetVertexPosMulti( T* p, lua_State *L )
	{
		size_t start= 0;
		int table_index= 1;
		// Allow the user to just pass a table without specifying a starting point.
		if(lua_type(L, 1) == LUA_TNUMBER)
		{
			table_index= 2;
			start= IArg(1);
		}
		size_t end= start + lua_objlen(L, table_index);
		if(start > p->GetNumVertices())
		{
			LOG->Warn("ActorMultiVertex:SetVertexPosMulti: Out of range starting point.");
			return 0;
		}
		if(end > p->GetNumVertices())
		{
			LOG->Warn("ActorMultiVertex:SetVertexPosMulti: Too many positions passed.");
			return 0;
		}
		for(size_t n= start; n < end; ++n)
		{
			lua_pushnumber(L, n-start+1);
			lua_gettable(L, table_index);
			int vindex= lua_gettop(L);
			lua_rawgeti(L, vindex, 1);
			float x= lua_tonumber(L, -1);
			lua_rawgeti(L, vindex, 2);
			float y= lua_tonumber(L, -1);
			lua_rawgeti(L, vindex, 3);
			float z= lua_tonumber(L, -1);
			p->SetVertexPos(n, x, y, z);
			lua_pop(L, 4);
		}
		return 0;
	}
	static int SetVertexColorMulti( T* p, lua_State *L )
	{
		size_t start= 0;
		int table_index= 1;
		// Allow the user to just pass a table without specifying a starting point.
		if(lua_type(L, 1) == LUA_TNUMBER)
		{
			table_index= 2;
			start= IArg(1);
		}
		size_t end= start + lua_objlen(L, table_index);
		if(start > p->GetNumVertices())
		{
			LOG->Warn("ActorMultiVertex:SetVertexColorMulti: Out of range starting point.");
			return 0;
		}
		if(end > p->GetNumVertices())
		{
			LOG->Warn("ActorMultiVertex:SetVertexColorMulti: Too many colors passed.");
			return 0;
		}
		for(size_t n= start; n < end; ++n)
		{
			lua_pushnumber(L, n-start+1);
			lua_gettable(L, table_index);
			RageColor c;
			c.FromStackCompat(L, -1);
			p->SetVertexColor(n, c);
			lua_pop(L, 1);
		}
		return 0;
	}
	static int SetVertexCoordsMulti( T* p, lua_State *L )
	{
		size_t start= 0;
		int table_index= 1;
		// Allow the user to just pass a table without specifying a starting point.
		if(lua_type(L, 1) == LUA_TNUMBER)
		{
			table_index= 2;
			start= IArg(1);
		}
		size_t end= start + lua_objlen(L, table_index);
		if(start > p->GetNumVertices())
		{
			LOG->Warn("ActorMultiVertex:SetVertexCoordsMulti: Out of range starting point.");
			return 0;
		}
		if(end > p->GetNumVertices())
		{
			LOG->Warn("ActorMultiVertex:SetVertexCoordsMulti: Too many coords passed.");
			return 0;
		}
		for(size_t n= start; n < end; ++n)
		{
			lua_pushnumber(L, n-start+1);
			lua_gettable(L, table_index);
			int vindex= lua_gettop(L);
			lua_rawgeti(L, vindex, 1);
			float x= lua_tonumber(L, -1);
			lua_rawgeti(L, vindex, 2);
			float y= lua_tonumber(L, -1);
			p->SetVertexCoords(n, x, y);
			lua_pop(L, 3);
		}
		return 0;
	}

	static int SetPos( T* p, lua_State *L )				{ p->SetPos(FArg(1),FArg(2),FArg(3)); return 0; }
	static int SetColor( T* p, lua_State *L )			{ RageColor c; c.FromStackCompat( L, 1 ); p->SetColor( c ); return 0; }
	static int SetCoords( T* p, lua_State *L )			{ p->SetCoords(FArg(1),FArg(2)); return 0; }

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
		ADD_METHOD( ClearVertices );
		ADD_METHOD( AddVertex );
		ADD_METHOD( GetNumVertices );
		ADD_METHOD( AddVertices );

		ADD_METHOD( SetDrawMode );
		ADD_METHOD( SetEffectMode );
		ADD_METHOD( SetTextureMode );

		// Set a specific vertex
		ADD_METHOD( SetVertexPos );
		ADD_METHOD( SetVertexColor );
		ADD_METHOD( SetVertexCoords );

		// Set multiple vertices from a table
		ADD_METHOD( SetVertexPosMulti );
		ADD_METHOD( SetVertexColorMulti );
		ADD_METHOD( SetVertexCoordsMulti );
		
		// Set the most recent vertex
		ADD_METHOD( SetPos );
		ADD_METHOD( SetColor );
		ADD_METHOD( SetCoords );
		
		// Copy from RageTexture
		ADD_METHOD( SetTexture );
		// Load from file path
		ADD_METHOD( LoadTexture );
	}
};

LUA_REGISTER_DERIVED_CLASS( ActorMultiVertex, Actor )

/*
 * (c) 2014 Matthew Gardner
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
