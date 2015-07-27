#include "global.h"
#include "WheelItemBase.h"
#include "LuaManager.h"

static const char *WheelItemDataTypeNames[] = {
	"Generic",
	"Section",
	"Song",
	"Roulette",
	"Random",
	"Portal",
	"Course",
	"Sort",
	"Custom",
};
XToString( WheelItemDataType );
StringToX( WheelItemDataType );
LuaXType( WheelItemDataType );

WheelItemBaseData::WheelItemBaseData( WheelItemDataType type, RString sText, RageColor color )
{
	m_Type = type;
	m_sText = sText;
	m_color = color;
}

// begin WheelItemBase
WheelItemBase::WheelItemBase( const WheelItemBase &cpy ):
	ActorFrame( cpy ),
	m_pData( cpy.m_pData ),
	m_bExpanded( cpy.m_bExpanded )
{
	// FIXME
	//if( cpy.m_pGrayBar == cpy.m_sprBar )
	//	m_pGrayBar = m_sprBar;
}

WheelItemBase::WheelItemBase(RString sType)
{
	SetName( sType );
	m_pData = nullptr;
	m_bExpanded = false;
	m_pGrayBar = nullptr;
	Load(sType);
}

void WheelItemBase::Load( RString sType )
{
	m_colorLocked = RageColor(0,0,0,0.25f);
}

void WheelItemBase::LoadFromWheelItemData( const WheelItemBaseData* pWID, int iIndex, bool bHasFocus, int iDrawIndex )
{
	ASSERT( pWID != nullptr );
	m_pData = pWID;
}

void WheelItemBase::DrawGrayBar( Actor& bar )
{
	if( m_colorLocked.a == 0 )
		return;

	RageColor glow = bar.GetGlow();
	RageColor diffuse = bar.GetDiffuse();

	bar.SetGlow( m_colorLocked );
	bar.SetDiffuse( RageColor(0,0,0,0) );

	bar.Draw();

	bar.SetGlow( glow );
	bar.SetDiffuse( diffuse );
}

void WheelItemBase::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();

	if( m_pGrayBar != nullptr )
		DrawGrayBar( *m_pGrayBar );
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the WheelItemBase. */ 
class LunaWheelItemBase: public Luna<WheelItemBase>
{
public:
#define IS_LOADED_CHECK \
	if(!p->IsLoaded()) \
	{ \
		luaL_error(L, "Wheel item is not loaded yet.  Use WheelItem:IsLoaded() to check."); \
	}

	static int GetColor(T* p, lua_State *L)
	{
		IS_LOADED_CHECK;
		LuaHelpers::Push(L, p->GetColor());
		return 1;
	}

	static int GetText(T* p, lua_State *L)
	{
		IS_LOADED_CHECK;
		LuaHelpers::Push(L, p->GetText());
		return 1;
	}
	
	static int GetType(T* p, lua_State *L)
	{
		IS_LOADED_CHECK;
		lua_pushnumber(L, p->GetType());
		return 1;
	}

	static int IsLoaded(T* p, lua_State *L)
	{
		lua_pushboolean(L, p->IsLoaded());
		return 1;
	}

	LunaWheelItemBase()
	{
		ADD_METHOD( GetColor );
		ADD_METHOD( GetText );
		ADD_METHOD( GetType );
		ADD_METHOD( IsLoaded );
	}
};
LUA_REGISTER_DERIVED_CLASS( WheelItemBase, ActorFrame )
// lua end

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez, Glenn Maynard, Josh Allen
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
