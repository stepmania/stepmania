#include "global.h"
#include "RageTypes.h"
#include "LuaManager.h"

void RageColor::PushTable( lua_State *L ) const
{
	lua_newtable( L );
	int iTable = lua_gettop(L);

	lua_pushnumber( L, r );
	lua_rawseti( L, iTable, 1 );
	lua_pushnumber( L, g );
	lua_rawseti( L, iTable, 2 );
	lua_pushnumber( L, b );
	lua_rawseti( L, iTable, 3 );
	lua_pushnumber( L, a );
	lua_rawseti( L, iTable, 4 );
}

void RageColor::FromStack( lua_State *L, int iPos )
{
	if( lua_type(L, iPos) != LUA_TTABLE )
		return;

	lua_pushvalue( L, iPos );
	int iFrom = lua_gettop( L );

	lua_rawgeti( L, iFrom, 1 );
	r = (float)lua_tonumber( L, -1 );
	lua_rawgeti( L, iFrom, 2 );
	g = (float)lua_tonumber( L, -1 );
	lua_rawgeti( L, iFrom, 3 );
	b = (float)lua_tonumber( L, -1 );
	lua_rawgeti( L, iFrom, 4 );
	a = (float)lua_tonumber( L, -1 );
	lua_pop( L, 5 );
}

void RageColor::FromStackCompat( lua_State *L, int iPos )
{
	if( lua_type(L, iPos) == LUA_TTABLE )
	{
		FromStack( L, iPos );
	}
	else
	{
		r = FArg(iPos+0);
		g = FArg(iPos+1);
		b = FArg(iPos+2);
		a = FArg(iPos+3);
	}
}

RString RageColor::ToString() const
{
	int iR = clamp( (int) lrintf(r * 255), 0, 255 );
	int iG = clamp( (int) lrintf(g * 255), 0, 255 );
	int iB = clamp( (int) lrintf(b * 255), 0, 255 );
	int iA = clamp( (int) lrintf(a * 255), 0, 255 );

	if( iA == 255 )
		return ssprintf( "#%02X%02X%02X", iR, iG, iB );
	else
		return ssprintf( "#%02X%02X%02X%02X", iR, iG, iB, iA );
}

RString RageColor::NormalizeColorString( RString sColor )
{
	if( sColor.empty() )
		return "";
	RageColor c;
	if( !c.FromString(sColor) )
		return "";
	return c.ToString();
}
/** @brief Utilities for working with Lua. */
namespace LuaHelpers
{
	template<> bool FromStack<RageColor>( lua_State *L, RageColor &Object, int iOffset )
	{
		Object.FromStack( L, iOffset );
		return true;
	}
	template<> void Push<RageColor>( lua_State *L, const RageColor &Object ) { Object.PushTable( L ); }
}

static const char *CullModeNames[] =
{
	"Back",
	"Front",
	"None"
};
XToString( CullMode );
LuaXType( CullMode );

static const char *BlendModeNames[] =
{
	"Normal",
	"Add",
	"Subtract",
	"Modulate",

	/*
	 * Copy the source directly to the destination.  Alpha is not premultiplied.
	 *
	 * Co = Cs
	 * Ao = As
	 */
	"CopySrc",

	/*
	 * Leave the color alone, and apply the source alpha on top of the existing alpha.
	 * Transparent areas in the source become transparent in the destination.
	 *
	 * Be sure to disable alpha test with this blend mode.
	 *
	 * Co = Cd
	 * Ao = Ad*As
	 */
	"AlphaMask",

	/*
	 * Leave the color alone, and apply the source alpha on top of the existing alpha.
	 * Transparent areas in the source become transparent in the destination.
	 * Co = Cd
	 * Ao = Ad*(1-As)
	 */
	"AlphaKnockOut",
	"AlphaMultiply",
	"WeightedMultiply",
	"InvertDest",
	"NoEffect"
};
XToString( BlendMode );
StringToX( BlendMode );
LuaXType( BlendMode );

static const char *TextureModeNames[] =
{
	"Modulate",
	"Glow",
	"Add",
};
XToString( TextureMode );
LuaXType( TextureMode );


static const char *EffectModeNames[] =
{
	/* Normal blending.  All supported texture modes have their standard effects. */
	"Normal",

	/* After rendering to a destination alpha render target, the color will be premultiplied
	 * with its alpha.  An Unpremultiply pass with CopySrc blending must be performed to
	 * correct this. */
	"Unpremultiply",

	/* Layered blending.    These shaders take two source textures. */
	"ColorBurn",
	"ColorDodge",
	"VividLight",
	"HardMix",
	"Overlay",
	"Screen",

	"YUYV422"
};
XToString( EffectMode );
LuaXType( EffectMode );


static const char *ZTestModeNames[] =
{
	"Off",
	"WriteOnPass",
	"WriteOnFail"
};
XToString( ZTestMode );
LuaXType( ZTestMode );

static const char *TextGlowModeNames[] =
{
	"Inner",
	"Stroke",
	"Both"
};
XToString( TextGlowMode );
LuaXType( TextGlowMode );

int LuaFunc_color( lua_State *L )
{
	RString sColor = SArg(1);
	RageColor c;
	c.FromString( sColor );
	c.PushTable( L );
	return 1;
}
void LuaFunc_Register_color( lua_State *L ) { lua_register( L, "color", LuaFunc_color ); }
REGISTER_WITH_LUA_FUNCTION( LuaFunc_Register_color );

/*
 * Copyright (c) 2006 Glenn Maynard
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
