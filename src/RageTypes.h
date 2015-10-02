/* RageTypes - vector and matrix types. */

#ifndef RAGETYPES_H
#define RAGETYPES_H

#include "EnumHelper.h"
#include "RageColor.hpp"
#include "RageVector2.hpp"
#include "RageVector3.hpp"
#include "RageVector4.hpp"

enum BlendMode
{
	BLEND_NORMAL,
	BLEND_ADD,
	BLEND_SUBTRACT,
	BLEND_MODULATE,
	BLEND_COPY_SRC,
	BLEND_ALPHA_MASK,
	BLEND_ALPHA_KNOCK_OUT,
	BLEND_ALPHA_MULTIPLY,
	BLEND_WEIGHTED_MULTIPLY,
	BLEND_INVERT_DEST,
	BLEND_NO_EFFECT,
	NUM_BlendMode,
	BlendMode_Invalid
};
LuaDeclareType( BlendMode );

enum TextureMode
{
	// Affects one texture stage. Texture is modulated with the diffuse color.
	TextureMode_Modulate,

	/* Affects one texture stage. Color is replaced with white, leaving alpha.
	 * Used with BLEND_ADD to add glow. */
	TextureMode_Glow,

	// Affects one texture stage. Color is added to the previous texture stage.
	TextureMode_Add,

	NUM_TextureMode,
	TextureMode_Invalid
};
LuaDeclareType( TextureMode );

enum EffectMode
{
	EffectMode_Normal,
	EffectMode_Unpremultiply,
	EffectMode_ColorBurn,
	EffectMode_ColorDodge,
	EffectMode_VividLight,
	EffectMode_HardMix,
	EffectMode_Overlay,
	EffectMode_Screen,
	EffectMode_YUYV422,
	NUM_EffectMode,
	EffectMode_Invalid
};
LuaDeclareType( EffectMode );

enum CullMode
{
	CULL_BACK,
	CULL_FRONT,
	CULL_NONE,
	NUM_CullMode,
	CullMode_Invalid
};
LuaDeclareType( CullMode );

enum ZTestMode
{
	ZTEST_OFF,
	ZTEST_WRITE_ON_PASS,
	ZTEST_WRITE_ON_FAIL,
	NUM_ZTestMode,
	ZTestMode_Invalid
};
LuaDeclareType( ZTestMode );

enum PolygonMode
{
	POLYGON_FILL,
	POLYGON_LINE,
	NUM_PolygonMode,
	PolygonMode_Invalid
};
LuaDeclareType( PolygonMode );

enum TextGlowMode
{
	TextGlowMode_Inner,
	TextGlowMode_Stroke,
	TextGlowMode_Both,
	NUM_TextGlowMode,
	TextGlowMode_Invalid
};
LuaDeclareType( TextGlowMode );

struct lua_State;

void PushTable( Rage::Color const &color, lua_State *L );
void FromStack( Rage::Color &color, lua_State *L, int pos);
void FromStackCompat( Rage::Color &color, lua_State *L, int pos);

/* Convert floating-point 0..1 value to integer 0..255 value. *
 *
 * As a test case,
 *
 * int cnts[1000]; memset(cnts, 0, sizeof(cnts));
 * for( float n = 0; n <= 1.0; n += 0.0001 ) cnts[FTOC(n)]++;
 * for( int i = 0; i < 256; ++i ) printf("%i ", cnts[i]);
 *
 * should output the same value (+-1) 256 times.  If this function is
 * incorrect, the first and/or last values may be biased. */
inline unsigned char FTOC(float a)
{
        /* lfintf is much faster than C casts.  We don't care which way negative values
         * are rounded, since we'll clamp them to zero below.  Be sure to truncate (not
         * round) positive values.  The highest value that should be converted to 1 is
         * roughly (1/256 - 0.00001); if we don't truncate, values up to (1/256 + 0.5)
         * will be converted to 1, which is wrong. */
        int ret = lrintf(a*256.f - 0.5f);

        /* Benchmarking shows that clamping here, as integers, is much faster than clamping
         * before the conversion, as floats. */
        if( ret<0 ) return 0;
        else if( ret>255 ) return 255;
        else return (unsigned char) ret;
}

/* Color type used only in vertex lists.  OpenGL expects colors in
 * r, g, b, a order, independent of endianness, so storing them this
 * way avoids endianness problems.  Don't try to manipulate this; only
 * manip Rage::Colors. */
/* Perhaps the math in Rage::Color could be moved to RageVColor.  We don't need the 
 * precision of a float for our calculations anyway.   -Chris */
class RageVColor
{
public:
	uint8_t b,g,r,a;	// specific ordering required by Direct3D

	RageVColor(): b(0), g(0), r(0), a(0) { }
	RageVColor(const Rage::Color &rc): b(0), g(0), r(0), a(0) { *this = rc; }
	RageVColor &operator= (const Rage::Color &rc)
	{
		r = FTOC(rc.r); g = FTOC(rc.g); b = FTOC(rc.b); a = FTOC(rc.a);
		return *this;
	}
};

namespace StepMania
{
	template <class T>
	class Rect
	{
public:
		Rect(): left(0), top(0), right(0), bottom(0) {}
		Rect(T l, T t, T r, T b): left(l), top(t), right(r), bottom(b) {}
		
		T GetWidth() const	{ return right-left; };
		T GetHeight() const	{ return bottom-top;  };
		T GetCenterX() const	{ return (left+right)/2; };
		T GetCenterY() const	{ return (top+bottom)/2; };
		
		bool operator==( const Rect &other ) const
		{
#define COMPARE( x )	if( x != other.x ) return false
			COMPARE( left );
			COMPARE( top );
			COMPARE( right );
			COMPARE( bottom );
#undef COMPARE
			return true;
		}
		bool operator!=( const Rect &other ) const { return !operator==(other); }
		
		T left, top, right, bottom;
	};
}
typedef StepMania::Rect<int> RectI;
typedef StepMania::Rect<float> RectF;

/* Structure for our custom vertex type.  Note that these data structes 
 * have the same layout that D3D expects. */
struct RageSpriteVertex	// has color
{
	RageSpriteVertex(): p(), n(), c(), t() {}
	Rage::Vector3 p; // position
	Rage::Vector3 n; // normal
	RageVColor  c; // diffuse color
	Rage::Vector2 t; // texture coordinates
};

void lerp_rage_color(Rage::Color& out, Rage::Color const& a, Rage::Color const& b, float t);
void WeightedAvergeOfRSVs(RageSpriteVertex& average_out, RageSpriteVertex const& rsv1, RageSpriteVertex const& rsv2, float percent_between);

struct RageModelVertex	// doesn't have color.  Relies on material color
{
	/* Zero out by default. */
	RageModelVertex():
		p(0,0,0),
		n(0,0,0),
		t(0,0),
		bone(0),
		TextureMatrixScale(1,1)
		{ }
	Rage::Vector3 p;	// position
	Rage::Vector3 n;	// normal
	Rage::Vector2 t;	// texture coordinates
	int8_t      bone;
	Rage::Vector2 TextureMatrixScale; // usually 1,1
};


// RageMatrix elements are specified in row-major order.  This
// means that the translate terms are located in the fourth row and the
// projection terms in the fourth column.  This is consistent with the way
// MAX, Direct3D, and OpenGL all handle matrices.  Even though the OpenGL
// documentation is in column-major form, the OpenGL code is designed to
// handle matrix operations in row-major form.
struct RageMatrix
{
public:
	RageMatrix() {};
	RageMatrix( const float *f )	{ for(int i=0; i<4; i++) for(int j=0; j<4; j++) m[j][i]=f[j*4+i]; }
	RageMatrix( const RageMatrix& other )	{ for(int i=0; i<4; i++) for(int j=0; j<4; j++) m[j][i]=other.m[j][i]; }
	RageMatrix( float v00, float v01, float v02, float v03,
                float v10, float v11, float v12, float v13,
                float v20, float v21, float v22, float v23,
                float v30, float v31, float v32, float v33 );

	// access grants
	float& operator () ( int iRow, int iCol )	{ return m[iCol][iRow]; }
	float  operator () ( int iRow, int iCol ) const { return m[iCol][iRow]; }

	// casting operators
	operator float* ()				{ return m[0]; }
	operator const float* () const			{ return m[0]; }

	RageMatrix GetTranspose() const;

	float m[4][4];
};

#endif

/*
 * Copyright (c) 2001-2002 Chris Danford
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
