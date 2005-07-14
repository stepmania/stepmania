/* ThemeMetric - Theme specific data. */

#ifndef THEME_METRIC_H
#define THEME_METRIC_H

#include "ThemeManager.h"
#include <map>
#include "Foreach.h"
#include "LuaManager.h"

class IThemeMetric
{
public:
	virtual ~IThemeMetric() { }
	virtual void Read() = 0;
};

template <class T>
class ThemeMetric : public IThemeMetric
{
private:
	CString		m_sGroup;
	CString		m_sName;
	T			m_currentValue;
	bool		m_bIsLoaded;

	/* HACK: If true, reload the metric each time it's read.  This is like DynamicThemeMetric,
	 * but works transparently for strings.  This is very slow, and should be removed once
	 * metrics are refactored to fix DynamicThemeMetric. */
	bool		m_bAlwaysReload;

public:
	/* Initializing with no group and name is allowed; if you do this, you must
	 * call Load() to set them.  This is done to allow initializing cached metrics
	 * in one place for classes that don't receive their m_sName in the constructor
	 * (everything except screens). */
	ThemeMetric( const CString& sGroup = "", const CString& sName = "", bool bAlwaysReload = false ):
		m_sGroup( sGroup ),
		m_sName( sName ),
		m_bIsLoaded( false ),
		m_bAlwaysReload( bAlwaysReload )
	{
		m_currentValue = T();
		ThemeManager::Subscribe( this );
	}

	ThemeMetric( const ThemeMetric<T> &cpy ):
		IThemeMetric( cpy ),
		m_sGroup( cpy.m_sGroup ),
		m_sName( cpy.m_sName ),
		m_currentValue( cpy.m_currentValue ),
		m_bIsLoaded( cpy.m_bIsLoaded ),
		m_bAlwaysReload( cpy.m_bAlwaysReload )
	{
		ThemeManager::Subscribe( this );
	}
	
	~ThemeMetric()
	{
		ThemeManager::Unsubscribe( this );
	}

	void Load( const CString &sGroup, const CString& sName )
	{
		m_sGroup = sGroup;
		m_sName = sName;
		Read();
	}

	void ChangeGroup( const CString &sGroup )
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

	const T& GetValue() const
	{
		ASSERT( m_sName != "" );
		ASSERT( m_bIsLoaded );

		if( m_bAlwaysReload )
		{
			ThemeMetric<T> *p = const_cast<ThemeMetric<T> *>(this);
			p->Read();
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

typedef CString (*MetricName1D)(size_t N);

template <class T>
class ThemeMetric1D : public IThemeMetric
{
	typedef ThemeMetric<T> ThemeMetricT;
	vector<ThemeMetricT> m_metric;

public:
	ThemeMetric1D( const CString& sGroup, MetricName1D pfn, size_t N )
	{
		Load( sGroup, pfn, N );
	}
	ThemeMetric1D()
	{
		Load( CString(), NULL, 0 );
	}
	void Load( const CString& sGroup, MetricName1D pfn, size_t N )
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
	const T& GetValue( size_t i ) const
	{
		return m_metric[i].GetValue();
	}
};

typedef CString (*MetricName2D)(size_t N, size_t M);

template <class T>
class ThemeMetric2D : public IThemeMetric
{
	typedef ThemeMetric<T> ThemeMetricT;
	typedef vector<ThemeMetricT> ThemeMetricTVector;
	vector<ThemeMetricTVector> m_metric;

public:
	ThemeMetric2D( const CString& sGroup = "", MetricName2D pfn = NULL, size_t N = 0, size_t M = 0 )
	{
		Load( sGroup, pfn, N, M );
	}
	void Load( const CString& sGroup, MetricName2D pfn, size_t N, size_t M )
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
	const T& GetValue( size_t i, size_t j ) const
	{
		return m_metric[i][j].GetValue();
	}
};

typedef CString (*MetricNameMap)(CString s);

template <class T>
class ThemeMetricMap : public IThemeMetric
{
	typedef ThemeMetric<T> ThemeMetricT;
	map<CString,ThemeMetricT> m_metric;

public:
	ThemeMetricMap( const CString& sGroup = "", MetricNameMap pfn = NULL, const vector<CString> vsValueNames = vector<CString>() )
	{
		Load( sGroup, pfn, vsValueNames );
	}
	void Load( const CString& sGroup, MetricNameMap pfn, const vector<CString> vsValueNames )
	{
		m_metric.clear();
		FOREACH_CONST( CString, vsValueNames, s )
			m_metric[*s].Load( sGroup, pfn(*s) );
	}
	void Read()
	{
		// HACK: GCC (3.4) takes this and pretty much nothing else.
		// I don't know why.
		for( typename map<CString,ThemeMetric<T> >::iterator m = m_metric.begin(); m != m_metric.end(); ++m )
			m->second.Read();
	}
	const T& GetValue( CString s ) const
	{
		// HACK: GCC (3.4) takes this and pretty much nothing else.
		// I don't know why.
		typename map<CString,ThemeMetric<T> >::const_iterator iter = m_metric.find(s);
		ASSERT( iter != m_metric.end() );
		return iter->second.GetValue();
	}
};

template <class T>
class ThemeMetricEnum : public ThemeMetric<int>
{
public:
	ThemeMetricEnum() : ThemeMetric<int>() {}
	ThemeMetricEnum( const CString& sGroup, const CString& sName ) : ThemeMetric<int>(sGroup,sName) {}
	T GetValue() const { return (T)ThemeMetric<int>::GetValue(); }
	bool operator ==( T other ) const { return GetValue() == other; }
};

/*
 * Like ThemeMetric, but allows evaluating Lua expressions each time.  This is
 * fast, since we only compile the expression once.  To evaluate every time,
 * return a function instead of a value.
 *
 * This is just as fast as ThemeMetric when the value is not a function.
 *
 * This differs from ThemeMetric: the value of the metric must be a valid Lua
 * expression; strings must be enclosed in quotes.  (This is probably how
 * all metrics will eventually behave.)
 */
template <class T>
class DynamicThemeMetric : public IThemeMetric
{
private:
	CString		m_sGroup;
	CString		m_sName;
	mutable T	m_currentValue;
	bool		m_bIsLoaded;
	LuaReference m_Value;

public:
	/* Initializing with no group and name is allowed; if you do this, you must
	 * call Load() to set them.  This is done to allow initializing cached metrics
	 * in one place for classes that don't receive their m_sName in the constructor
	 * (everything except screens). */
	DynamicThemeMetric( const CString& sGroup = "", const CString& sName = "" ):
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

	void Load( const CString &sGroup, const CString& sName )
	{
		m_sGroup = sGroup;
		m_sName = sName;
		Read();
	}

	void ChangeGroup( const CString &sGroup )
	{
		m_sGroup = sGroup;
		Read();
	}

	void Read()
	{
		if( m_sName != ""  &&  THEME  &&   THEME->IsThemeLoaded() )
		{
			CString sExpression;
			THEME->GetMetric( m_sGroup, m_sName, sExpression );

			Lua *L = LUA->Get();
			LuaHelpers::RunScript( L, "return " + sExpression, ssprintf("%s:%s", m_sGroup.c_str(), m_sName.c_str()), 1 );

			if( lua_type(L, -1) == LUA_TFUNCTION )
			{
				m_Value.SetFromStack( L );
			}
			else
			{
				m_Value.Unset();
				LuaHelpers::PopStack( m_currentValue, L );
			}

			LUA->Release(L);

			m_bIsLoaded = true;
		}
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
			LuaHelpers::PopStack( m_currentValue, L );
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
