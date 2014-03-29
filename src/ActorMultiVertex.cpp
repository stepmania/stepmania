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
	m_pTexture = TEXTUREMAN->LoadTexture( ID );
	ClearVertices();

	m_DrawMode = DrawMode_Invalid;
	m_EffectMode = EffectMode_Normal;
	m_TextureMode = TextureMode_Modulate;
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
	CPY( m_Vertices );
	CPY( m_DrawMode );
	CPY( m_EffectMode );
	CPY( m_TextureMode );
	CPY( m_fLineWidth );
#undef CPY

	if( cpy.m_pTexture != NULL )
		m_pTexture = TEXTUREMAN->CopyTexture( cpy.m_pTexture );
	else
		m_pTexture = NULL;

}

void ActorMultiVertex::LoadFromNode( const XNode* pNode )
{
	RString sPath;
	pNode->GetAttrValue( "Texture", sPath );
	if( !sPath.empty() && !TEXTUREMAN->IsTextureRegistered( RageTextureID(sPath) ) )
		ActorUtil::GetAttrPath( pNode, "Texture", sPath );

	if( !sPath.empty() )
		LoadFromTexture( sPath );

	Actor::LoadFromNode( pNode );
}

void ActorMultiVertex::SetTexture( RageTexture *pTexture )
{
	if( m_pTexture != pTexture )
	{
		UnloadTexture();
		m_pTexture = pTexture;
	}
}

void ActorMultiVertex::LoadFromTexture( RageTextureID ID )
{

	RageTexture *pTexture = NULL;
	if( m_pTexture && m_pTexture->GetID() == ID )
		return;
	else
		pTexture = TEXTUREMAN->LoadTexture( ID );

	SetTexture( pTexture );
		
}

void ActorMultiVertex::UnloadTexture()
{
	if( m_pTexture != NULL )
	{
		TEXTUREMAN->UnloadTexture( m_pTexture );
		m_pTexture = NULL;
	}
}

void ActorMultiVertex::ClearVertices()
{
	m_Vertices.clear();
}

void ActorMultiVertex::AddVertex()
{
	m_Vertices.push_back( RageSpriteVertex() );
}

void ActorMultiVertex::SetVertexPos( int iIndex , float fX , float fY , float fZ )
{
	if( iIndex >= (int) m_Vertices.size() )
	{
		LOG->Warn( "Can't set Vertex Position, index %d too high.", iIndex );
		return;
	}
	m_Vertices[iIndex].p = RageVector3( fX,  fY, fZ );
}

void ActorMultiVertex::SetVertexColor( int iIndex , RageColor c )
{
	if( iIndex >= (int) m_Vertices.size() )
	{
		LOG->Warn( "Can't set Vertex Color, index %d too high.", iIndex );
		return;
	}
	m_Vertices[iIndex].c = c;
}

void ActorMultiVertex::SetVertexCoords( int iIndex , float fTexCoordX , float fTexCoordY )
{
	if( iIndex >= (int) m_Vertices.size() )
	{
		LOG->Warn( "Can't set Vertex Texture Coordinates, index %d too high.", iIndex );
		return;
	}
	m_Vertices[iIndex].t = RageVector2( fTexCoordX,  fTexCoordY );
}

void ActorMultiVertex::DrawPrimitives()
{
	Actor::SetGlobalRenderStates();	// set Actor-specified render states

	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( TextureUnit_1, m_pTexture->GetTexHandle() );
   
	Actor::SetTextureRenderStates();
	DISPLAY->SetEffectMode( m_EffectMode );
	DISPLAY->SetTextureMode( TextureUnit_1, m_TextureMode );

	int iNumToDraw = m_Vertices.size();

	switch( m_DrawMode )
	{
		case DrawMode_Quads:
		{
			iNumToDraw -= iNumToDraw%4;
			if( iNumToDraw >= 4 )
				DISPLAY->DrawQuads( &m_Vertices[0] , iNumToDraw );
			break;
		}
		case DrawMode_QuadStrip:
		{
			iNumToDraw -= iNumToDraw%2;

			if( iNumToDraw >= 4 )
				DISPLAY->DrawQuadStrip( &m_Vertices[0] , iNumToDraw );
			break;
		}
		case DrawMode_Fan:
		{
			if( iNumToDraw >= 3 )
				DISPLAY->DrawFan( &m_Vertices[0] , iNumToDraw );
			break;
		}
		case DrawMode_Strip:
		{
			if( iNumToDraw >= 3 )
				DISPLAY->DrawStrip( &m_Vertices[0] , iNumToDraw );
			break;
		}
		case DrawMode_Triangles:
		{
			iNumToDraw -= iNumToDraw%3;
			if( iNumToDraw >= 3 )
					DISPLAY->DrawTriangles( &m_Vertices[0] , iNumToDraw );
			break;
		}
		case DrawMode_LineStrip:
		{
			if( iNumToDraw >= 2 )
				DISPLAY->DrawLineStrip( &m_Vertices[0] , iNumToDraw , m_fLineWidth );
			break;
		}
		case DrawMode_SymmetricQuadStrip:
		{
			iNumToDraw -= iNumToDraw%3;

			if( iNumToDraw >= 6 )
				DISPLAY->DrawSymmetricQuadStrip( &m_Vertices[0] , iNumToDraw );
			break;

		}
	}

	DISPLAY->SetEffectMode( EffectMode_Normal );
}

bool ActorMultiVertex::EarlyAbortDraw() const
{
	return m_Vertices.empty();
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ActorMultiVertex. */ 
class LunaActorMultiVertex: public Luna<ActorMultiVertex>
{
public:
	static int AddVertex( T* p, lua_State *L )			{ p->AddVertex( ); return 0; }
	static int ClearVertices( T* p, lua_State *L )		{ p->ClearVertices( ); return 0; }
	static int GetNumVertices( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetNumVertices() ); return 1; }

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

	LunaActorMultiVertex()
	{
		ADD_METHOD( ClearVertices );
		ADD_METHOD( AddVertex );
		ADD_METHOD( GetNumVertices );

		ADD_METHOD( SetDrawMode );
		ADD_METHOD( SetEffectMode );
		ADD_METHOD( SetTextureMode );

		// Set a specific vertex
		ADD_METHOD( SetVertexPos );
		ADD_METHOD( SetVertexColor );
		ADD_METHOD( SetVertexCoords );
		
		// Set the most recent vertex
		ADD_METHOD( SetPos );
		ADD_METHOD( SetColor );
		ADD_METHOD( SetCoords );
		
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
