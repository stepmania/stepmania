#include "global.h"
#include "RageTypes.h"
#include "RageMath.hpp"
#include "LuaManager.h"

void PushTable( Rage::Color const &color, lua_State *L )
{
	lua_newtable(L);
	int table = lua_gettop(L);
	
	lua_pushnumber(L, color.r);
	lua_rawseti(L, table, 1);
	lua_pushnumber(L, color.g);
	lua_rawseti(L, table, 2);
	lua_pushnumber(L, color.b);
	lua_rawseti(L, table, 3);
	lua_pushnumber(L, color.a);
	lua_rawseti(L, table, 4);
}

void FromStack( Rage::Color &color, lua_State *L, int pos)
{
	if (lua_type(L, pos) != LUA_TTABLE)
	{
		return;
	}
	
	lua_pushvalue(L, pos);
	int from = lua_gettop(L);
	
	lua_rawgeti(L, from, 1);
	color.r = static_cast<float>(lua_tonumber(L, -1));
	lua_rawgeti(L, from, 2);
	color.g = static_cast<float>(lua_tonumber(L, -1));
	lua_rawgeti(L, from, 3);
	color.b = static_cast<float>(lua_tonumber(L, -1));
	lua_rawgeti(L, from, 4);
	color.a = static_cast<float>(lua_tonumber(L, -1));
	
	lua_pop(L, 5);
}

void FromStackCompat( Rage::Color &color, lua_State *L, int pos)
{
	if (lua_type(L, pos) == LUA_TTABLE)
	{
		// Utilize the shortcut variant.
		FromStack(color, L, pos);
	}
	else
	{
		color.r = FArg(pos + 0);
		color.g = FArg(pos + 1);
		color.b = FArg(pos + 2);
		color.a = FArg(pos + 3);
	}
}

void lerp_rage_color(Rage::Color& out, Rage::Color const& a, Rage::Color const& b, float t)
{
	out.b= Rage::lerp(t, a.b, b.b);
	out.g= Rage::lerp(t, a.g, b.g);
	out.r= Rage::lerp(t, a.r, b.r);
	out.a= Rage::lerp(t, a.a, b.a);
}

void WeightedAvergeOfRSVs(Rage::SpriteVertex& average_out, Rage::SpriteVertex const& rsv1, Rage::SpriteVertex const& rsv2, float percent_between)
{
	average_out.p= Rage::lerp(percent_between, rsv1.p, rsv2.p);
	average_out.n= Rage::lerp(percent_between, rsv1.n, rsv2.n);
	average_out.c.b= Rage::lerp(percent_between, rsv1.c.b, rsv2.c.b);
	average_out.c.g= Rage::lerp(percent_between, rsv1.c.g, rsv2.c.g);
	average_out.c.r= Rage::lerp(percent_between, rsv1.c.r, rsv2.c.r);
	average_out.c.a= Rage::lerp(percent_between, rsv1.c.a, rsv2.c.a);
	average_out.t= Rage::lerp(percent_between, rsv1.t, rsv2.t);
}

/** @brief Utilities for working with Lua. */
namespace LuaHelpers
{
	template<> bool FromStack<Rage::Color>( lua_State *L, Rage::Color &Object, int iOffset )
	{
		FromStack( Object, L, iOffset );
		return true;
	}
	template<> void Push<Rage::Color>( lua_State *L, const Rage::Color &Object )
	{
		PushTable( Object, L );
	}
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
	std::string sColor = SArg(1);
	Rage::Color c;
	c.FromString( sColor );
	PushTable( c, L );
	return 1;
}
LUAFUNC_REGISTER_COMMON(color);

int LuaFunc_lerp_color(lua_State *L)
{
	// Args:  percent, color, color
	// Returns:  color
	float percent= FArg(1);
	Rage::Color a, b, c;
	FromStack(a, L, 2);
	FromStack(b, L, 3);
	lerp_rage_color(c, a, b, percent);
	PushTable(c, L);
	return 1;
}
LUAFUNC_REGISTER_COMMON(lerp_color);

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
