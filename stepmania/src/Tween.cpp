#include "global.h"
#include "Tween.h"
#include "RageUtil.h"
#include "RageMath.h"
#include "LuaManager.h"
#include "EnumHelper.h"

static const char *TweenTypeNames[] = {
	"Linear",
	"Accelerate",
	"Decelerate",
	"Smooth",
	"Spring",
	"Bezier"
};
XToString( TweenType, NUM_TweenType );

static void LuaTweenType(lua_State* L)
{
	FOREACH_TweenType( tt )
	{
		RString s = TweenTypeToString( tt );
		LUA->SetGlobal( "Tween"+s, tt );
	}
}
REGISTER_WITH_LUA_FUNCTION( LuaTweenType );


struct TweenLinear: public ITween
{
	float Tween( float f ) const { return f; }
	ITween *Copy() const { return new TweenLinear(*this); }
};
struct TweenAccelerate: public ITween
{
	float Tween( float f ) const { return f*f; }
	ITween *Copy() const { return new TweenAccelerate(*this); }
};
struct TweenDecelerate: public ITween
{
	float Tween( float f ) const { return 1 - (1-f) * (1-f); }
	ITween *Copy() const { return new TweenDecelerate(*this); }
};
struct TweenSmooth: public ITween
{
	float Tween( float f ) const
	{
		/* Accelerate, reaching full speed at fShift, then decelerate the rest of
		 * the way.  fShift = 1 degrades to TWEEN_ACCELERATE.  fShift = 0 degrades
		 * to TWEEN_DECELERATE.  (This is a rough approximation of a sigmoid.) */
		const float fShift = 0.5f;
		if( f < fShift )
		{
			f = SCALE( f, 0.0f, fShift, 0.0f, 1.0f );
			f = f * f;
			f = SCALE( f, 0.0f, 1.0f, 0.0f, fShift );
		}
		else
		{
			f = SCALE( f, fShift, 1.0f, 0.0f, 1.0f );
			f = 1 - (1-f) * (1-f);
			f = SCALE( f, 0.0f, 1.0f, fShift, 1.0f );
		}

		return f;
	}

	ITween *Copy() const { return new TweenSmooth(*this); }
};

struct TweenSpring: public ITween
{
	float Tween( float f ) const { return 1 - RageFastCos( f*PI*2.5f )/(1+f*3); }
	ITween *Copy() const { return new TweenSpring(*this); }
};


/*
 * Interpolation with 1-dimensional cubic Bezier curves.
 */
struct InterpolateBezier1D: public ITween
{
	float Tween( float f ) const;
	ITween *Copy() const { return new InterpolateBezier1D(*this); }

	RageQuadradtic m_Bezier;
};

float InterpolateBezier1D::Tween( float f ) const
{
	return m_Bezier.Evaluate( f );
}

/*
 * Interpolation with 2-dimensional cubic Bezier curves.
 */
struct InterpolateBezier2D: public ITween
{
	float Tween( float f ) const;
	ITween *Copy() const { return new InterpolateBezier2D(*this); }

	RageBezier2D m_Bezier;
};

float InterpolateBezier2D::Tween( float f ) const
{
	return m_Bezier.EvaluateYFromX( f );
}

/* This interpolator combines multiple other interpolators, to allow
 * generating more complex tweens.  For example, "compound,10,linear,0.25,accelerate,0.75"
 * means 25% of the tween is linear, and the rest is accelerate.  This can be
 * used with Bezier to create spline tweens. */
// InterpolateCompound

ITween *ITween::CreateFromType( TweenType tt )
{
	switch( tt )
	{
	case TWEEN_LINEAR: return new TweenLinear;
	case TWEEN_ACCELERATE: return new TweenAccelerate;
	case TWEEN_DECELERATE: return new TweenDecelerate;
	case TWEEN_SMOOTH: return new TweenSmooth;

	/*
	 * These may actually be faster than InterpolateBounceBegin/InterpolateBounceEnd, since
	 * they don't use RageFastSin/RageFastCos.
	 */
	case TWEEN_SPRING: return new TweenSpring;
	default: ASSERT(0);
	}
}

ITween *ITween::CreateFromStack( Lua *L, int iStackPos )
{
	TweenType iType = (TweenType) IArg(iStackPos);
	if( iType == TWEEN_BEZIER )
	{
		luaL_checktype( L, iStackPos+1, LUA_TTABLE );
		int iArgs = luaL_getn( L, iStackPos+1 );
		if( iArgs != 4 && iArgs != 8 )
			RageException::Throw( "CreateFromStack: table argument must have 4 or 8 entries" );

		float fC[8];
		for( int i = 0; i < iArgs; ++i )
		{
			lua_rawgeti( L, iStackPos+1, i+1 );
			fC[i] = (float) lua_tonumber( L, -1 );
		}

		lua_pop( L, iArgs );
		if( iArgs == 4 )
		{
			InterpolateBezier1D *pBezier = new InterpolateBezier1D;
			pBezier->m_Bezier.SetFromBezier( fC[0], fC[1], fC[2], fC[3] );
			return pBezier;
		}
		else if( iArgs == 8 )
		{
			InterpolateBezier2D *pBezier = new InterpolateBezier2D;
			pBezier->m_Bezier.SetFromBezier( fC[0], fC[1], fC[2], fC[3], fC[4], fC[5], fC[6], fC[7] );
			return pBezier;
		}
	}
	
	return CreateFromType( iType );
}

/*
 * (c) 2001-2006 Glenn Maynard, Chris Danford
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
