/* ThemeMetric - Theme specific data. */

#ifndef THEME_METRIC_H
#define THEME_METRIC_H

#include "ThemeManager.h"
#include <unordered_map>
#include "LuaManager.h"
#include "RageUtil.h"

/** @brief The general interface for reading ThemeMetrics. */
class IThemeMetric
{
public:
	virtual ~IThemeMetric() { }
	virtual void Read() = 0;
	virtual void Clear() = 0;
};

template<class T>
struct ThemeMetricTypeTraits
{
	enum { Callable = 1 };
};

/* LuaReference and apActorCommands return the function directly without calling it. */
template<> struct ThemeMetricTypeTraits<LuaReference> { enum { Callable = 0 }; };
template<> struct ThemeMetricTypeTraits<apActorCommands> { enum { Callable = 0 }; };

/** @brief The theme specific data.
 *
 * Each piece of data is to correspond to a type. */
template <class T>
class ThemeMetric : public IThemeMetric
{
protected:
	/** @brief the metric's group.
	 *
	 * In metrics.ini, it is usually done as such: [GroupName] */
	std::string m_sGroup;
	/** @brief the metric's name. */
	std::string m_sName;
	/** @brief the metric's value. */
	LuaReference m_Value;
	mutable T m_currentValue;
	bool m_bCallEachTime;

public:
	/* Initializing with no group and name is allowed; if you do this, you must
	 * call Load() to set them.  This is done to allow initializing cached metrics
	 * in one place for classes that don't receive their m_sName in the constructor
	 * (everything except screens). */
	ThemeMetric( std::string const & sGroup = "", std::string const & sName = "" ):
		m_sGroup( sGroup ),
		m_sName( sName ),
		m_Value(), m_currentValue(T()), m_bCallEachTime(false)
	{
		ThemeManager::Subscribe( this );
	}

	ThemeMetric( const ThemeMetric<T> &cpy ):
		IThemeMetric( cpy ),
		m_sGroup( cpy.m_sGroup ),
		m_sName( cpy.m_sName ),
		m_Value( cpy.m_Value )
		// do we transfer the current value or bCallEachTime?
	{
		ThemeManager::Subscribe( this );
	}

	~ThemeMetric()
	{
		ThemeManager::Unsubscribe( this );
	}
	/**
	 * @brief Load the chosen metric from the .ini file.
	 * @param sGroup the group the metric is in.
	 * @param sName the name of the metric. */
	void Load( std::string const &sGroup, std::string const & sName )
	{
		m_sGroup = sGroup;
		m_sName = sName;
		Read();
	}

	void ChangeGroup( std::string const &sGroup )
	{
		m_sGroup = sGroup;
		Read();
	}
	/**
	 * @brief Actually read the metric and get its data. */
	void Read()
	{
		if( m_sName != ""  &&  THEME  &&   THEME->IsThemeLoaded() )
		{
			Lua *L = LUA->Get();
			THEME->GetMetric( m_sGroup, m_sName, m_Value );
			m_Value.PushSelf(L);
			LuaHelpers::FromStack(L, m_currentValue, -1);
			lua_pop( L, 1 );
			LUA->Release(L);

			/* If the value is a function, evaluate it every time. */
			m_bCallEachTime = ThemeMetricTypeTraits<T>::Callable && m_Value.GetLuaType() == LUA_TFUNCTION;
		}
		else
		{
			m_Value.Unset();
			m_bCallEachTime = false;
		}
	}

	void PushSelf( Lua *L )
	{
		ASSERT( m_Value.IsSet() );
		m_Value.PushSelf(L);
	}

	void Clear()
	{
		m_Value.Unset();
	}

	/**
	 * @brief Retrieve the metric's name.
	 * @return the metric's name. */
	std::string const &GetName() const { return m_sName; }
	/**
	 * @brief Retrieve the metric's group.
	 * @return the metric's group. */
	std::string const &GetGroup() const { return m_sGroup; }

	/**
	 * @brief Retrieve the metric's value.
	 * @return the metric's value. */
	T const & GetValue() const
	{
		ASSERT( m_sName != "" );
		ASSERT_M( m_Value.IsSet(), m_sGroup + " " + m_sName );

		if( m_bCallEachTime )
		{
			Lua *L = LUA->Get();

			// call function with 0 arguments and 1 result
			m_Value.PushSelf( L );
			std::string error = m_sGroup + ": " + m_sName + ": ";
			LuaHelpers::RunScriptOnStack(L, error, 0, 1, true);
			if(!lua_isnil(L, -1))
			{
				LuaHelpers::Pop( L, m_currentValue );
			}
			else
			{
				lua_pop(L, 1);
			}
			LUA->Release(L);
		}

		return m_currentValue;
	}

	operator const T& () const
	{
		return GetValue();
	}

	bool IsLoaded() const	{ return m_Value.IsSet(); }

	// Hacks for VC6 for all boolean operators.
	// These three no longer appear to be required:
	//bool operator ! () const { return !GetValue(); }
	//bool operator && ( const T& input ) const { return GetValue() && input; }
	//bool operator || ( const T& input ) const { return GetValue() || input; }

	// This one is still required in at least Visual Studio 2008:
	bool operator == ( const T& input ) const { return GetValue() == input; }
};

typedef std::string (*MetricName1D)(size_t N);

template <class T>
class ThemeMetric1D : public IThemeMetric
{
	typedef ThemeMetric<T> ThemeMetricT;
	std::vector<ThemeMetricT> m_metric;

public:
	ThemeMetric1D( std::string const & sGroup, MetricName1D pfn, size_t N )
	{
		Load( sGroup, pfn, N );
	}
	ThemeMetric1D()
	{
		Load( std::string(), nullptr, 0 );
	}
	void Load( std::string const & sGroup, MetricName1D pfn, size_t N )
	{
		m_metric.resize( N );
		for( unsigned i=0; i<N; i++ )
		{
			m_metric[i].Load( sGroup, pfn(i) );
		}
	}
	void Read()
	{
		for (auto &metric: m_metric)
		{
			metric.Read();
		}
	}
	void Clear()
	{
		for (auto &metric: m_metric)
		{
			metric.Clear();
		}
	}
	const T& GetValue( size_t i ) const
	{
		return m_metric[i].GetValue();
	}
};

typedef std::string (*MetricName2D)(size_t N, size_t M);

template <class T>
class ThemeMetric2D : public IThemeMetric
{
	typedef ThemeMetric<T> ThemeMetricT;
	typedef std::vector<ThemeMetricT> ThemeMetricTVector;
	std::vector<ThemeMetricTVector> m_metric;

public:
	ThemeMetric2D( std::string const & sGroup = "", MetricName2D pfn = nullptr, size_t N = 0, size_t M = 0 )
	{
		Load( sGroup, pfn, N, M );
	}
	void Load( std::string const & sGroup, MetricName2D pfn, size_t N, size_t M )
	{
		m_metric.resize( N );
		for( unsigned i=0; i<N; i++ )
		{
			m_metric[i].resize( M );
			for( unsigned j=0; j<M; j++ )
			{
				m_metric[i][j].Load( sGroup, pfn(i,j) );
			}
		}
	}
	void Read()
	{
		for (auto &metricArray: m_metric)
		{
			for (auto &metric: metricArray)
			{
				metric.Read();
			}
		}
	}
	void Clear()
	{
		for (auto &metricArray: m_metric)
		{
			for (auto &metric: metricArray)
			{
				metric.Clear();
			}
		}
	}
	const T& GetValue( size_t i, size_t j ) const
	{
		return m_metric[i][j].GetValue();
	}
};

typedef std::string (*MetricNameMap)(std::string s);

template <class T>
class ThemeMetricMap : public IThemeMetric
{
	typedef ThemeMetric<T> ThemeMetricT;
	std::unordered_map<std::string,ThemeMetricT> m_metric;

public:
	ThemeMetricMap( std::string const & sGroup = "", MetricNameMap pfn = nullptr, const std::vector<std::string> vsValueNames = std::vector<std::string>() )
	{
		Load( sGroup, pfn, vsValueNames );
	}
	void Load( std::string const & sGroup, MetricNameMap pfn, const std::vector<std::string> vsValueNames )
	{
		m_metric.clear();
		for (auto const &s: vsValueNames)
		{
			m_metric[s].Load( sGroup, pfn(s) );
		}
	}
	void Read()
	{
		for (auto &m: m_metric)
		{
			m.second.Read();
		}
	}
	void Clear()
	{
		for (auto &m: m_metric)
		{
			m.second.Clear();
		}
	}
	const T& GetValue( std::string s ) const
	{
		auto iter = m_metric.find(s);
		ASSERT( iter != m_metric.end() );
		return iter->second.GetValue();
	}
};

#endif

/**
 * @file
 * @author Chris Danford, Chris Gomez (c) 2001-2004
 * @section LICENSE
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
