/* RageTypes - vector and matrix types. */

#ifndef RAGETYPES_H
#define RAGETYPES_H

#include "EnumHelper.h"

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
	EffectMode_DistanceField,
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

struct RageVector2
{
public:
	RageVector2(): x(0), y(0) {}
	RageVector2( const float * f ): x(f[0]), y(f[1]) {}
	RageVector2( float x1, float y1 ): x(x1), y(y1) {}
	
	// casting
	operator float* ()			{ return &x; };
	operator const float* () const		{ return &x; };
	
	// assignment operators
	RageVector2& operator += ( const RageVector2& other )	{ x+=other.x; y+=other.y; return *this; }
	RageVector2& operator -= ( const RageVector2& other )	{ x-=other.x; y-=other.y; return *this; }
	RageVector2& operator *= ( float f )			{ x*=f; y*=f; return *this; }
	RageVector2& operator /= ( float f )			{ x/=f; y/=f; return *this; }
	
	// binary operators
	RageVector2 operator + ( const RageVector2& other ) const	{ return RageVector2( x+other.x, y+other.y ); }
	RageVector2 operator - ( const RageVector2& other ) const	{ return RageVector2( x-other.x, y-other.y ); }
	RageVector2 operator * ( float f ) const			{ return RageVector2( x*f, y*f ); }
	RageVector2 operator / ( float f ) const			{ return RageVector2( x/f, y/f ); }
	
	friend RageVector2 operator * ( float f, const RageVector2& other )	{ return other*f; }
	
	float x, y;
};


struct RageVector3
{
public:
	RageVector3(): x(0), y(0), z(0) {}
	RageVector3( const float * f ):	x(f[0]), y(f[1]), z(f[2]) {}
	RageVector3( float x1, float y1, float z1 ): x(x1), y(y1), z(z1) {}
	
	// casting
	operator float* ()				{ return &x; };
	operator const float* () const			{ return &x; };
	
	// assignment operators
	RageVector3& operator += ( const RageVector3& other )	{ x+=other.x; y+=other.y; z+=other.z; return *this; }
	RageVector3& operator -= ( const RageVector3& other )	{ x-=other.x; y-=other.y; z-=other.z; return *this; }
	RageVector3& operator *= ( float f )			{ x*=f; y*=f; z*=f; return *this; }
	RageVector3& operator /= ( float f )			{ x/=f; y/=f; z/=f; return *this; }
	
	// binary operators
	RageVector3 operator + ( const RageVector3& other ) const	{ return RageVector3( x+other.x, y+other.y, z+other.z ); }
	RageVector3 operator - ( const RageVector3& other ) const	{ return RageVector3( x-other.x, y-other.y, z-other.z ); }
	RageVector3 operator * ( float f ) const			{ return RageVector3( x*f, y*f, z*f ); }
	RageVector3 operator / ( float f ) const			{ return RageVector3( x/f, y/f, z/f ); }
	
	friend RageVector3 operator * ( float f, const RageVector3& other )	{ return other*f; }
	
	float x, y, z;
};


struct RageVector4
{
public:
	RageVector4(): x(0), y(0), z(0), w(0) {}
	RageVector4( const float * f ): x(f[0]), y(f[1]), z(f[2]), w(f[3]) {}
	RageVector4( float x1, float y1, float z1, float w1 ): x(x1), y(y1), z(z1), w(w1) {}
	
	// casting
	operator float* ()					{ return &x; };
	operator const float* () const				{ return &x; };
	
	// assignment operators
	RageVector4& operator += ( const RageVector4& other )	{ x+=other.x; y+=other.y; z+=other.z; w+=other.w; return *this; }
	RageVector4& operator -= ( const RageVector4& other )	{ x-=other.x; y-=other.y; z-=other.z; w-=other.w; return *this; }
	RageVector4& operator *= ( float f )			{ x*=f; y*=f; z*=f; w*=f; return *this; }
	RageVector4& operator /= ( float f )			{ x/=f; y/=f; z/=f; w/=f; return *this; }
	
	// binary operators
	RageVector4 operator + ( const RageVector4& other ) const	{ return RageVector4( x+other.x, y+other.y, z+other.z, w+other.w ); }
	RageVector4 operator - ( const RageVector4& other ) const	{ return RageVector4( x-other.x, y-other.y, z-other.z, w-other.w ); }
	RageVector4 operator * ( float f ) const			{ return RageVector4( x*f, y*f, z*f, w*f ); }
	RageVector4 operator / ( float f ) const			{ return RageVector4( x/f, y/f, z/f, w/f ); }
	
	friend RageVector4 operator * ( float f, const RageVector4& other )	{ return other*f; }
	
	float x, y, z, w;
};

struct RageColor
{
public:
	RageColor(): r(0), g(0), b(0), a(0) {}
	explicit RageColor( const float * f ): r(f[0]), g(f[1]), b(f[2]), a(f[3]) {}
	RageColor( float r1, float g1, float b1, float a1 ): r(r1), g(g1), b(b1), a(a1) {}
	
	// casting
	operator float* ()					{ return &r; };
	operator const float* () const				{ return &r; };
	
	// assignment operators
	RageColor& operator += ( const RageColor& other )	{ r+=other.r; g+=other.g; b+=other.b; a+=other.a; return *this; }
	RageColor& operator -= ( const RageColor& other )	{ r-=other.r; g-=other.g; b-=other.b; a-=other.a; return *this; }
	RageColor& operator *= ( const RageColor& other )	{ r*=other.r; g*=other.g; b*=other.b; a*=other.a; return *this; }
	RageColor& operator *= ( float f )			{ r*=f; g*=f; b*=f; a*=f; return *this; }
	/* Divide is rarely useful: you can always use multiplication, and you don't have to
		* worry about div/0. */
	//    RageColor& operator /= ( float f )		{ r/=f; g/=f; b/=f; a/=f; return *this; }
	
	// binary operators
	RageColor operator + ( const RageColor& other ) const	{ return RageColor( r+other.r, g+other.g, b+other.b, a+other.a ); }
	RageColor operator - ( const RageColor& other ) const	{ return RageColor( r-other.r, g-other.g, b-other.b, a-other.a ); }
	RageColor operator * ( const RageColor& other ) const	{ return RageColor( r*other.r, g*other.g, b*other.b, a*other.a ); }
	RageColor operator * ( float f ) const			{ return RageColor( r*f, g*f, b*f, a*f ); }
	// Divide is useful for using with the SCALE macro
	RageColor operator / ( float f ) const			{ return RageColor( r/f, g/f, b/f, a/f ); }
	
	friend RageColor operator * ( float f, const RageColor& other )	{ return other*f; } // What is this for?  Did I add this?  -Chris
	
	bool operator == ( const RageColor& other ) const	{ return r==other.r && g==other.g && b==other.b && a==other.a; }
	bool operator != ( const RageColor& other ) const	{ return !operator==(other); }
	
	bool FromString( const RString &str )
	{
		int result = sscanf( str, "%f,%f,%f,%f", &r, &g, &b, &a );
		if( result == 3 )
		{
			a = 1;
			return true;
		}
		if( result == 4 )
			return true;
		
		int ir=255, ib=255, ig=255, ia=255;
		result = sscanf( str, "#%2x%2x%2x%2x", &ir, &ig, &ib, &ia );
		if( result >= 3 )
		{
			r = ir / 255.0f; g = ig / 255.0f; b = ib / 255.0f;
			if( result == 4 )
				a = ia / 255.0f;
			else
				a = 1;
			return true;
		}
		
		r=1; b=1; g=1; a=1;
		return false;
	}

	RString ToString() const;
	static RString NormalizeColorString( RString sColor );

	void PushTable( lua_State *L ) const;
	void FromStack( lua_State *L, int iPos );
	void FromStackCompat( lua_State *L, int iPos );

	float r, g, b, a;
};

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
#ifdef CPU_AARCH64
inline unsigned char FTOC(float a)
{
	const float v = a < 0.0f ? 0.0f : (a > 1.0f ? 1.0f : a);
	return static_cast<unsigned char>(v * 255.0f);
}
#else
inline unsigned char FTOC(float a)
{
	//This value is 2^52 * 1.5.
	const double INT_MANTISSA = 6755399441055744.0;

	/* Be sure to truncate (not round) positive values. The highest value that
	* should be converted to 1 is roughly(1 / 256 - 0.00001); if we don't
	* truncate, values up to (1/256 + 0.5) will be converted to 1, which is
	* wrong. */
	double base = double(a * 256.f - 0.5f);

	/* INT_MANTISSA is chosen such that, when added to a sufficiently small
	* double, the mantissa bits of that double can be reinterpreted as that
	* number rounded to an integer. This is done to improve performance. */
	base += INT_MANTISSA;
	int ret = reinterpret_cast<int&>(base);

	/* Benchmarking shows that clamping here, as integers, is much faster than clamping
	* before the conversion, as floats. */
	if (ret < 0) {
		return 0;
	}
	if (ret > 255) {
		return 255;
	}

	return static_cast<unsigned char>(ret);
}
#endif

/* Color type used only in vertex lists.  OpenGL expects colors in
 * r, g, b, a order, independent of endianness, so storing them this
 * way avoids endianness problems.  Don't try to manipulate this; only
 * manip RageColors. */
/* Perhaps the math in RageColor could be moved to RageVColor.  We don't need the 
 * precision of a float for our calculations anyway.   -Chris */
class RageVColor
{
public:
	uint8_t b,g,r,a;	// specific ordering required by Direct3D

	RageVColor(): b(0), g(0), r(0), a(0) { }
	RageVColor(const RageColor &rc): b(0), g(0), r(0), a(0) { *this = rc; }
	RageVColor &operator= (const RageColor &rc)
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
	RageVector3 p; // position
	RageVector3 n; // normal
	RageVColor  c; // diffuse color
	RageVector2 t; // texture coordinates
};

void lerp_rage_color(RageColor& out, RageColor const& a, RageColor const& b, float t);
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
	RageVector3 p;	// position
	RageVector3 n;	// normal
	RageVector2 t;	// texture coordinates
	int8_t      bone;
	RageVector2 TextureMatrixScale; // usually 1,1
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
