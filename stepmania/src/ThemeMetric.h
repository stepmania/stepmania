/* ThemeMetric - Theme specific data. */

#ifndef THEME_METRIC_H
#define THEME_METRIC_H

#include "ThemeManager.h"
#include <map>
#include "Foreach.h"
#include "LuaManager.h"
#include "RageUtil.h"

class IThemeMetric
{
public:
	virtual ~IThemeMetric() { }
	virtual void Read() = 0;
	virtual void Clear() = 0;
};

template <class T>
class ThemeMetric : public IThemeMetric
{
protected:
	RString		m_sGroup;
	RString		m_sName;
	T		m_currentValue;
	bool		m_bIsLoaded;

public:
	/* Initializing with no group and name is allowed; if you do this, you must
	 * call Load() to set them.  This is done to allow initializing cached metrics
	 * in one place for classes that don't receive their m_sName in the constructor
	 * (everything except screens). */
	ThemeMetric( const RString& sGroup = "", const RString& sName = "" ):
		m_sGroup( sGroup ),
		m_sName( sName ),
		m_bIsLoaded( false )
	{
		m_currentValue = T();
		ThemeManager::Subscribe( this );
	}

	ThemeMetric( const ThemeMetric<T> &cpy ):
		IThemeMetric( cpy ),
		m_sGroup( cpy.m_sGroup ),
		m_sName( cpy.m_sName ),
		m_currentValue( cpy.m_currentValue ),
		m_bIsLoaded( cpy.m_bIsLoaded )
	{
		ThemeManager::Subscribe( this );
	}
	
	~ThemeMetric()
	{
		ThemeManager::Unsubscribe( this );
	}

	void Load( const RString &sGroup, const RString& sName )
	{
		m_sGroup = sGroup;
		m_sName = sName;
		Read();
	}

	void ChangeGroup( const RString &sGroup )
	{
		m_sGroup = sGroup;
		Read();
	}

	void Read()
	{
		if( m_sName != ""  &&  THEME  &&   THEME->IsThemeLoaded() )
		{
			THEME->GetMetric( m_sGroup, m_sName, m_currentValue );
			m_bIsLoaded = true;
		}
	}

	void Clear()
	{
		m_currentValue = T();
		m_bIsLoaded = false;
	}


	const T& GetName() const { return m_sName; }
	const T& GetGroup() const { return m_sGroup; }
	const T& GetValue() const
	{
		ASSERT( m_sName != "" );
		ASSERT_M( m_bIsLoaded, m_sGroup + " " + m_sName );
		return m_currentValue;
	}
	
	operator const T& () const
	{
		return GetValue();
	}

	bool IsLoaded() const	{ return m_bIsLoaded; }

	//Hacks for VC6 for all boolean operators.
	bool operator ! () const { return !GetValue(); }
	bool operator && ( const T& input ) const { return GetValue() && input; }
	bool operator || ( const T& input ) const { return GetValue() || input; }
	bool operator == ( const T& input ) const { return GetValue() == input; }
};

typedef RString (*MetricName1D)(size_t N);

template <class T>
class ThemeMetric1D : public IThemeMetric
{
	typedef ThemeMetric<T> ThemeMetricT;
	vector<ThemeMetricT> m_metric;

public:
	ThemeMetric1D( const RString& sGroup, MetricName1D pfn, size_t N )
	{
		Load( sGroup, pfn, N );
	}
	ThemeMetric1D()
	{
		Load( RString(), NULL, 0 );
	}
	void Load( const RString& sGroup, MetricName1D pfn, size_t N )
	{
		m_metric.resize( N );
		for( unsigned i=0; i<N; i++ )
			m_metric[i].Load( sGroup, pfn(i) );
	}
	void Read()
	{
		for( unsigned i=0; i<m_metric.size(); i++ )
			m_metric[i].Read();
	}
	void Clear()
	{
		for( unsigned i=0; i<m_metric.size(); i++ )
			m_metric[i].Clear();
	}
	const T& GetValue( size_t i ) const
	{
		return m_metric[i].GetValue();
	}
};

typedef RString (*MetricName2D)(size_t N, size_t M);

template <class T>
class ThemeMetric2D : public IThemeMetric
{
	typedef ThemeMetric<T> ThemeMetricT;
	typedef vector<ThemeMetricT> ThemeMetricTVector;
	vector<ThemeMetricTVector> m_metric;

public:
	ThemeMetric2D( const RString& sGroup = "", MetricName2D pfn = NULL, size_t N = 0, size_t M = 0 )
	{
		Load( sGroup, pfn, N, M );
	}
	void Load( const RString& sGroup, MetricName2D pfn, size_t N, size_t M )
	{
		m_metric.resize( N );
		for( unsigned i=0; i<N; i++ )
		{
			m_metric[i].resize( M );
			for( unsigned j=0; j<M; j++ )
				m_metric[i][j].Load( sGroup, pfn(i,j) );
		}
	}
	void Read()
	{
		for( unsigned i=0; i<m_metric.size(); i++ )
			for( unsigned j=0; j<m_metric[i].size(); j++ )
				m_metric[i][j].Read();
	}
	void Clear()
	{
		for( unsigned i=0; i<m_metric.size(); i++ )
			for( unsigned j=0; j<m_metric[i].size(); j++ )
				m_metric[i][j].Clear();
	}
	const T& GetValue( size_t i, size_t j ) const
	{
		return m_metric[i][j].GetValue();
	}
};

typedef RString (*MetricNameMap)(RString s);

template <class T>
class ThemeMetricMap : public IThemeMetric
{
	typedef ThemeMetric<T> ThemeMetricT;
	map<RString,ThemeMetricT> m_metric;

public:
	ThemeMetricMap( const RString& sGroup = "", MetricNameMap pfn = NULL, const vector<RString> vsValueNames = vector<RString>() )
	{
		Load( sGroup, pfn, vsValueNames );
	}
	void Load( const RString& sGroup, MetricNameMap pfn, const vector<RString> vsValueNames )
	{
		m_metric.clear();
		FOREACH_CONST( RString, vsValueNames, s )
			m_metric[*s].Load( sGroup, pfn(*s) );
	}
	void Read()
	{
		// HACK: GCC (3.4) takes this and pretty much nothing else.
		// I don't know why.
		for( typename map<RString,ThemeMetric<T> >::iterator m = m_metric.begin(); m != m_metric.end(); ++m )
			m->second.Read();
	}
	void Clear()
	{
		for( typename map<RString,ThemeMetric<T> >::iterator m = m_metric.begin(); m != m_metric.end(); ++m )
			m->second.Clear();
	}
	const T& GetValue( RString s ) const
	{
		// HACK: GCC (3.4) takes this and pretty much nothing else.
		// I don't know why.
		typename map<RString,ThemeMetric<T> >::const_iterator iter = m_metric.find(s);
		ASSERT( iter != m_metric.end() );
		return iter->second.GetValue();
	}
};

// VC 6 hack: even templated functions can't differ by only return value.  Move return value to a reference parameter.
template<class T> void StringTo( const RString& s, T &out );
template<class T>
class ThemeMetricEnum : public ThemeMetric<RString>
{
	T m_Value;
public:
	ThemeMetricEnum() : ThemeMetric<RString>() {}
	ThemeMetricEnum( const RString& sGroup, const RString& sName ) :
	ThemeMetric<RString>( sGroup, sName ) {}
	void Read() { ThemeMetric<RString>::Read(); StringTo<T>(m_currentValue,m_Value); }
	const T &GetValue() const { ASSERT( !m_sName.empty() && m_bIsLoaded ); return m_Value; }
	operator const T& () const { return GetValue(); }
};


/*
 * Like ThemeMetric, but allows evaluating Lua expressions each time.  This is
 * fast, since we only compile the expression once.  To evaluate every time,
 * return a function instead of a value.
 *
 * This is just as fast as ThemeMetric when the value is not a function.
 */
template <class T>
class DynamicThemeMetric : public IThemeMetric
{
protected:
	RString		m_sGroup;
	RString		m_sName;
	mutable T	m_currentValue;
	bool		m_bIsLoaded;
	LuaReference m_Value;

public:
	/* Initializing with no group and name is allowed; if you do this, you must
	 * call Load() to set them.  This is done to allow initializing cached metrics
	 * in one place for classes that don't receive their m_sName in the constructor
	 * (everything except screens). */
	DynamicThemeMetric( const RString& sGroup = "", const RString& sName = "" ):
		m_sGroup( sGroup ),
		m_sName( sName ),
		m_bIsLoaded( false )
	{
		m_currentValue = T();
		ThemeManager::Subscribe( this );
	}

	DynamicThemeMetric( const ThemeMetric<T> &cpy ):
		IThemeMetric( cpy ),
		m_sGroup( cpy.m_sGroup ),
		m_sName( cpy.m_sName ),
		m_currentValue( cpy.m_currentValue ),
		m_bIsLoaded( cpy.m_bIsLoaded ),
		m_Value( cpy.m_Value )
	{
		ThemeManager::Subscribe( this );
	}
	
	~DynamicThemeMetric()
	{
		ThemeManager::Unsubscribe( this );
	}

	void Load( const RString &sGroup, const RString& sName )
	{
		m_sGroup = sGroup;
		m_sName = sName;
		Read();
	}

	void ChangeGroup( const RString &sGroup )
	{
		m_sGroup = sGroup;
		Read();
	}

	void Read()
	{
		if( m_sName != ""  &&  THEME  &&   THEME->IsThemeLoaded() )
		{
			Lua *L = LUA->Get();

			THEME->GetMetric( m_sGroup, m_sName, m_Value );

			m_Value.PushSelf( L );
			if( lua_type(L, -1) == LUA_TFUNCTION )
			{
				lua_pop( L, 1 );
			}
			else
			{
				m_Value.Unset();
				LuaHelpers::Pop( L, m_currentValue );
			}

			LUA->Release(L);

			m_bIsLoaded = true;
		}
	}
	void Clear()
	{
		m_Value.Unset();
		m_bIsLoaded = false;
	}

	const T& GetValue() const
	{
		ASSERT( m_sName != "" );
		ASSERT( m_bIsLoaded );

		/* If the value is a function, evaluate it every time. */
		if( m_Value.IsSet() )
		{
			Lua *L = LUA->Get();

			// call function with 0 arguments and 1 result
			m_Value.PushSelf( L );
			lua_call(L, 0, 1); 
			ASSERT( !lua_isnil(L, -1) );
			LuaHelpers::Pop( L, m_currentValue );
			LUA->Release(L);
		}

		return m_currentValue;
	}
	
	operator const T& () const
	{
		return GetValue();
	}

	bool IsLoaded() const	{ return m_bIsLoaded; }

	//Hacks for VC6 for all boolean operators.
	bool operator ! () const { return !GetValue(); }
	bool operator && ( const T& input ) const { return GetValue() && input; }
	bool operator || ( const T& input ) const { return GetValue() || input; }
	bool operator == ( const T& input ) const { return GetValue() == input; }
};


#endif

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez
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
