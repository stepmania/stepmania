#ifndef RAGETYPES_H
#define RAGETYPES_H
/*
-----------------------------------------------------------------------------
 File: RageTypes.h

 Desc: .

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

/* These are shared between Actor and RageDisplay.  I don't want to put this in
 * Actor (because Rage shouldn't touch Actor) and I don't want to have Actor.h
 * depend on RageDisplay (since that'd boost RDisplay.h a lot), so let's just
 * put this here. */
enum GlowMode { GLOW_BRIGHTEN, GLOW_WHITEN };
enum BlendMode { BLEND_NORMAL, BLEND_ADD, BLEND_NO_EFFECT, BLEND_INVALID };


struct RageVector2
{
public:
    RageVector2() {}
    RageVector2( const float * f )								{ x=f[0]; y=f[1]; }
	RageVector2( float x1, float y1 )							{ x=x1; y=y1; }

    // casting
	operator float* ()											{ return &x; };
    operator const float* () const								{ return &x; };

    // assignment operators
	RageVector2& operator += ( const RageVector2& other )		{ x+=other.x; y+=other.y; return *this; }
    RageVector2& operator -= ( const RageVector2& other )		{ x-=other.x; y-=other.y; return *this; }
    RageVector2& operator *= ( float f )						{ x*=f; y*=f; return *this; }
    RageVector2& operator /= ( float f )						{ x/=f; y/=f; return *this; }

    // binary operators
    RageVector2 operator + ( const RageVector2& other ) const	{ return RageVector2( x+other.x, y+other.y ); }
    RageVector2 operator - ( const RageVector2& other ) const	{ return RageVector2( x-other.x, y-other.y ); }
    RageVector2 operator * ( float f ) const					{ return RageVector2( x*f, y*f ); }
    RageVector2 operator / ( float f ) const					{ return RageVector2( x/f, y/f ); }

	friend RageVector2 operator * ( float f, const RageVector2& other )	{ return other*f; }
 
    float x, y;
};


struct RageVector3
{
public:
    RageVector3() {}
    RageVector3( const float * f )								{ x=f[0]; y=f[1]; z=f[2]; }
	RageVector3( float x1, float y1, float z1 )					{ x=x1; y=y1; z=z1; }

    // casting
	operator float* ()											{ return &x; };
    operator const float* () const								{ return &x; };

    // assignment operators
	RageVector3& operator += ( const RageVector3& other )		{ x+=other.x; y+=other.y; z+=other.z; return *this; }
    RageVector3& operator -= ( const RageVector3& other )		{ x-=other.x; y-=other.y; z-=other.z; return *this; }
    RageVector3& operator *= ( float f )						{ x*=f; y*=f; z*=f; return *this; }
    RageVector3& operator /= ( float f )						{ x/=f; y/=f; z/=f; return *this; }

    // binary operators
    RageVector3 operator + ( const RageVector3& other ) const	{ return RageVector3( x+other.x, y+other.y, z+other.z ); }
    RageVector3 operator - ( const RageVector3& other ) const	{ return RageVector3( x-other.x, y-other.y, z-other.z ); }
    RageVector3 operator * ( float f ) const					{ return RageVector3( x*f, y*f, z*f ); }
    RageVector3 operator / ( float f ) const					{ return RageVector3( x/f, y/f, z/f ); }

	friend RageVector3 operator * ( float f, const RageVector3& other )	{ return other*f; }

	float x, y, z;
};


struct RageVector4
{
public:
    RageVector4() {}
    RageVector4( const float * f )								{ x=f[0]; y=f[1]; z=f[2]; w=f[3]; }
	RageVector4( float x1, float y1, float z1, float w1 )		{ x=x1; y=y1; z=z1; w=w1; }

    // casting
	operator float* ()											{ return &x; };
    operator const float* () const								{ return &x; };

    // assignment operators
	RageVector4& operator += ( const RageVector4& other )		{ x+=other.x; y+=other.y; z+=other.z; w+=other.w; return *this; }
    RageVector4& operator -= ( const RageVector4& other )		{ x-=other.x; y-=other.y; z-=other.z; w-=other.w; return *this; }
    RageVector4& operator *= ( float f )						{ x*=f; y*=f; z*=f; w*=f; return *this; }
    RageVector4& operator /= ( float f )						{ x/=f; y/=f; z/=f; w/=f; return *this; }

    // binary operators
    RageVector4 operator + ( const RageVector4& other ) const	{ return RageVector4( x+other.x, y+other.y, z+other.z, w+other.w ); }
    RageVector4 operator - ( const RageVector4& other ) const	{ return RageVector4( x-other.x, y-other.y, z-other.z, w-other.w ); }
    RageVector4 operator * ( float f ) const					{ return RageVector4( x*f, y*f, z*f, w*f ); }
    RageVector4 operator / ( float f ) const					{ return RageVector4( x/f, y/f, z/f, w/f ); }

	friend RageVector4 operator * ( float f, const RageVector4& other )	{ return other*f; }

	float x, y, z, w;
};

struct RageColor
{
public:
    RageColor() {}
    RageColor( const float * f )							{ r=f[0]; g=f[1]; b=f[2]; a=f[3]; }
	RageColor( float r1, float g1, float b1, float a1 )		{ r=r1; g=g1; b=b1; a=a1; }

    // casting
	operator float* ()										{ return &r; };
    operator const float* () const							{ return &r; };

    // assignment operators
	RageColor& operator += ( const RageColor& other )		{ r+=other.r; g+=other.g; b+=other.b; a+=other.a; return *this; }
    RageColor& operator -= ( const RageColor& other )		{ r-=other.r; g-=other.g; b-=other.b; a-=other.a; return *this; }
    RageColor& operator *= ( const RageColor& other )		{ r*=other.r; g*=other.g; b*=other.b; a*=other.a; return *this; }
    RageColor& operator *= ( float f )						{ r*=f; g*=f; b*=f; a*=f; return *this; }
	/* Divide is rarely useful: you can always use multiplication, and you don't have to
	 * worry about div/0. */
//    RageColor& operator /= ( float f )						{ r/=f; g/=f; b/=f; a/=f; return *this; }

    // binary operators
    RageColor operator + ( const RageColor& other ) const	{ return RageColor( r+other.r, g+other.g, b+other.b, a+other.a ); }
    RageColor operator - ( const RageColor& other ) const	{ return RageColor( r-other.r, g-other.g, b-other.b, a-other.a ); }
    RageColor operator * ( const RageColor& other ) const	{ return RageColor( r*other.r, g*other.g, b*other.b, a*other.a ); }
    RageColor operator * ( float f ) const					{ return RageColor( r*f, g*f, b*f, a*f ); }
//    RageColor operator / ( float f ) const					{ return RageColor( r/f, g/f, b/f, a/f ); }

	friend RageColor operator * ( float f, const RageColor& other )	{ return other*f; }		// What is this for?  Did I add this?  -Chris

	bool FromString( const CString &str )
	{
		int result = sscanf( str, "%f,%f,%f,%f", &r, &g, &b, &a );
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
	float r, g, b, a;
};

/* Convert floating-point 0..1 value to integer 0..255 value. */
inline unsigned char FTOC(float a) { if(a<0) a=0; if(a>1) a=1; return (unsigned char)(a*255.f); }	// need clamping to handle values <0.0 and > 1.0

/* Color type used only in vertex lists.  OpenGL expects colors in
 * r, g, b, a order, independent of endianness, so storing them this
 * way avoids endianness problems.  Don't try to manipulate this; only
 * manip RageColors. */
/* Perhaps the math in RageColor could be moved to RaveVColor.  We don't need the 
 * precision of a float for our calculations anyway.   -Chris */
class RageVColor
{
public:
	unsigned char b,g,r,a;	// specific ordering required by Direct3D

	RageVColor(): b(0), g(0), r(0), a(0) { }
	RageVColor(const RageColor &rc) { *this = rc; }
	RageVColor &operator= (const RageColor &rc) {
		r = FTOC(rc.r); g = FTOC(rc.g); b = FTOC(rc.b); a = FTOC(rc.a);
		return *this;
	}
};


template <class T>
class Rect
{
public:
	Rect()						{};
	Rect(T l, T t, T r, T b)	{ left = l, top = t, right = r, bottom = b; };

	T GetWidth() const		{ return right-left; };
	T GetHeight() const		{ return bottom-top;  };
	T GetCenterX() const	{ return (left+right)/2; };
	T GetCenterY() const	{ return (top+bottom)/2; };

    T    left, top, right, bottom;
};

typedef Rect<int> RectI;
typedef Rect<float> RectF;

/* Structure for our custom vertex type.  Note that these data structes 
 * have the same layout that D3D expects. */
struct RageSpriteVertex	// has color
{
	/* Zero out by default. */
	RageSpriteVertex():
		p(0,0,0),
		n(0,0,0),
		t(0,0)
		{ }
    RageVector3		p;	// position
    RageVector3		n;	// normal
    RageVColor		c;	// diffuse color
	RageVector2		t;	// texture coordinates
};

struct RageModelVertex	// doesn't have color.  Relies on material color
{
	/* Zero out by default. */
	RageModelVertex():
		p(0,0,0),
		n(0,0,0),
		t(0,0)
		{ }
    RageVector3		p;	// position
    RageVector3		n;	// normal
	RageVector2		t;	// texture coordinates
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
	RageMatrix( const float *f )			{ for(int i=0; i<4; i++) for(int j=0; j<4; j++) m[j][i]=f[j*4+i]; }
	RageMatrix( const RageMatrix& other )	{ for(int i=0; i<4; i++) for(int j=0; j<4; j++) m[j][i]=other.m[j][i]; }
    RageMatrix( float v00, float v01, float v02, float v03,
                float v10, float v11, float v12, float v13,
                float v20, float v21, float v22, float v23,
                float v30, float v31, float v32, float v33 );

    // access grants
	float& operator () ( int iRow, int iCol )		{ return m[iCol][iRow]; }
    float  operator () ( int iRow, int iCol ) const { return m[iCol][iRow]; }

    // casting operators
    operator float* ()								{ return m[0]; }
    operator const float* () const					{ return m[0]; }

    RageMatrix GetTranspose() const;

	float m[4][4];
};

#endif
