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
	RageVector2& operator += ( const RageVector2& other )		{ x+=other.x; y+=other.y; }
    RageVector2& operator -= ( const RageVector2& other )		{ x-=other.x; y-=other.y; }
    RageVector2& operator *= ( float f )						{ x*=f; y*=f; }
    RageVector2& operator /= ( float f )						{ x/=f; y/=f; }

    // binary operators
    RageVector2 operator + ( const RageVector2& other ) const	{ return RageVector2( x+other.x, y+other.y ); }
    RageVector2 operator - ( const RageVector2& other ) const	{ return RageVector2( x-other.x, y-other.y ); }
    RageVector2 operator * ( float f ) const					{ return RageVector2( x*f, y*f ); }
    RageVector2 operator / ( float f ) const					{ return RageVector2( x/f, y/f ); }

	friend RageVector2 operator * ( float f, const RageVector2& other )	{ return other*f; }

	/* unneeded; this is a POD type */
//    bool operator == ( const RageVector2& other ) const		{ return x==other.x && y==other.y; }
//    bool operator != ( const RageVector2& other ) const		{ return x!=other.x || y!=other.y; }
 
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

	/* unneeded; this is a POD type */
//    bool operator == ( const RageVector3& other ) const		{ return x==other.x && y==other.y && z==other.z; }
//    bool operator != ( const RageVector3& other ) const		{ return x!=other.x || y!=other.y || z!=other.z; }

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
	RageVector4& operator += ( const RageVector4& other )		{ x+=other.x; y+=other.y; z+=other.z; w+=other.w; }
    RageVector4& operator -= ( const RageVector4& other )		{ x-=other.x; y-=other.y; z-=other.z; w-=other.w; }
    RageVector4& operator *= ( float f )						{ x*=f; y*=f; z*=f; w*=f; }
    RageVector4& operator /= ( float f )						{ x/=f; y/=f; z/=f; w/=f; }

    // binary operators
    RageVector4 operator + ( const RageVector4& other ) const	{ return RageVector4( x+other.x, y+other.y, z+other.z, w+other.w ); }
    RageVector4 operator - ( const RageVector4& other ) const	{ return RageVector4( x-other.x, y-other.y, z-other.z, w-other.w ); }
    RageVector4 operator * ( float f ) const					{ return RageVector4( x*f, y*f, z*f, w*f ); }
    RageVector4 operator / ( float f ) const					{ return RageVector4( x/f, y/f, z/f, w/f ); }

	friend RageVector4 operator * ( float f, const RageVector4& other )	{ return other*f; }

	/* unneeded; this is a POD type */
//    bool operator == ( const RageVector4& other ) const		{ return x==other.x && y==other.y && z==other.z && w==other.w; }
//    bool operator != ( const RageVector4& other ) const		{ return x!=other.x || y!=other.y || z!=other.z || w!=other.w; }

	float x, y, z, w;
};

/* nuke this stuff once we're sure RageVColor is OK */
/* #define RAGECOLOR_ARGB(a,r,g,b) \
    ((DWORD)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

#define RAGECOLOR_RGBA(r,g,b,a) RAGECOLOR_ARGB(a,r,g,b)

#define RAGECOLOR_COLORVALUE(r,g,b,a) \
    RAGECOLOR_RGBA((DWORD)((r)*255.f),(DWORD)((g)*255.f),(DWORD)((b)*255.f),(DWORD)((a)*255.f))
*/

struct RageColor
{
public:
    RageColor() {}
    RageColor( const float * f )							{ r=f[0]; g=f[1]; b=f[2]; a=f[3]; }
	RageColor( float r1, float g1, float b1, float a1 )		{ r=r1; g=g1; b=b1; a=a1; }
/*	RageColor( unsigned long c )
	{
		a = ((c>>24)&0xff) / 255.f;
		r = ((c>>16)&0xff) / 255.f;
		g = ((c>>8)&0xff)  / 255.f;
		b = (c&0xff)       / 255.f; 
	}
*/
    // casting
//	operator unsigned long () const							{ return RAGECOLOR_COLORVALUE(min(1.f,max(0.f,r)),min(1.f,max(0.f,g)),min(1.f,max(0.f,b)),min(1.f,max(0.f,a)));	}
	operator float* ()										{ return &r; };
    operator const float* () const							{ return &r; };

    // assignment operators
	RageColor& operator += ( const RageColor& other )		{ r+=other.r; g+=other.g; b+=other.b; a+=other.a; }
    RageColor& operator -= ( const RageColor& other )		{ r-=other.r; g-=other.g; b-=other.b; a-=other.a; }
    RageColor& operator *= ( float f )						{ r*=f; g*=f; b*=f; a*=f; }
    RageColor& operator /= ( float f )						{ r/=f; g/=f; b/=f; a/=f; }

    // binary operators
    RageColor operator + ( const RageColor& other ) const	{ return RageColor( r+other.r, g+other.g, b+other.b, a+other.a ); }
    RageColor operator - ( const RageColor& other ) const	{ return RageColor( r-other.r, g-other.g, b-other.b, a-other.a ); }
    RageColor operator * ( float f ) const					{ return RageColor( r*f, g*f, b*f, a*f ); }
    RageColor operator / ( float f ) const					{ return RageColor( r/f, g/f, b/f, a/f ); }

	friend RageVector4 operator * ( float f, const RageVector4& other )	{ return other*f; }		// What is this for?  Did I add this?  -Chris

	/* unneeded; this is a POD type */
//    bool operator == ( const RageColor& other ) const		{ return r==other.r && g==other.g && b==other.b && a==other.a; }
//    bool operator != ( const RageColor& other ) const		{ return r!=other.r || g!=other.g || b!=other.b || a!=other.a; }

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
	unsigned char r, g, b, a;
public:

	RageVColor() { }
	RageVColor(const RageColor &rc) { 
		r = FTOC(rc.r); g = FTOC(rc.g); b = FTOC(rc.b); a = FTOC(rc.a);
	}
};


template <class T>
class Rect
{
public:
	Rect()						{};
	Rect(T l, T t, T r, T b)	{ left = l, top = t, right = r, bottom = b; };

	int GetWidth() const		{ return right-left; };
	int GetHeight() const		{ return bottom-top;  };
	int GetCenterX() const		{ return (left+right)/2; };
	int GetCenterY() const		{ return (top+bottom)/2; };

    T    left, top, right, bottom;
};

typedef Rect<int> RectI;
typedef Rect<float> RectF;


// A structure for our custom vertex type.  Note that these data structes have the same layout that D3D expects.
struct RageVertex
{
	// This is the format expected by OpenGL.  D3D expects the reverse!
	RageVector2		t;	// texture coordinates
    RageVColor		c;	// diffuse color
    RageVector3		p;	// position
};

#define D3DFVF_RAGEVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)	// D3D FVF flags which describe our vertex structure


/* nonstandard extension used : nameless struct/union
 * It is, in fact, nonstandard.  G++ 3.x can handle it. 2.95.x can not. XXX */
#pragma warning (push)
#pragma warning (disable : 4201)

struct RageMatrix
{
public:
    RageMatrix() {};
	RageMatrix( const float *f )			{ for(int i=0; i<4; i++) for(int j=0; j<4; j++) m[j][i]=f[j*4+i]; }
	RageMatrix( const RageMatrix& other )	{ for(int i=0; i<4; i++) for(int j=0; j<4; j++) m[j][i]=other.m[j][i]; }
    RageMatrix( float v00, float v01, float v02, float v03,
                float v10, float v11, float v12, float v13,
                float v20, float v21, float v22, float v23,
                float v30, float v31, float v32, float v33 )
	{
		m00=v00; m01=v01; m02=v02; m03=v03;
		m10=v00; m11=v01; m12=v02; m13=v03;
		m20=v00; m21=v01; m22=v02; m23=v03;
		m30=v00; m31=v01; m32=v02; m33=v03;
	}

    // access grants
	float& operator () ( int iRow, int iCol )		{ return m[iCol][iRow]; }
    float  operator () ( int iRow, int iCol ) const { return m[iCol][iRow]; }

    // casting operators
    operator float* ()								{ return m[0]; }
    operator const float* () const					{ return m[0]; }

	RageMatrix GetTranspose() const { return RageMatrix(m00,m10,m20,m30,m01,m11,m21,m31,m02,m12,m22,m32,m03,m13,m23,m33); }

//	---These are not used.  Maybe I'll implement them later...---
//  // assignment operators
//  RageMatrix& operator *= ( const RageMatrix& );
//  RageMatrix& operator += ( const RageMatrix& );
//  RageMatrix& operator -= ( const RageMatrix& );
//  RageMatrix& operator *= ( float );
//  RageMatrix& operator /= ( float );
//
//  // unary operators
//  RageMatrix operator + () const;
//  RageMatrix operator - () const;
//
//  // binary operators
//  RageMatrix operator * ( const RageMatrix& ) const;
//  RageMatrix operator + ( const RageMatrix& ) const;
//  RageMatrix operator - ( const RageMatrix& ) const;
//  RageMatrix operator * ( float ) const;
//  RageMatrix operator / ( float ) const;
//
//  friend RageMatrix operator * ( float, const RageMatrix& );
//
//  bool operator == ( const RageMatrix& ) const;
//  bool operator != ( const RageMatrix& ) const;

    union
    {
        float m[4][4];
        struct
        {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };
    };
};

#pragma warning (pop)

#endif