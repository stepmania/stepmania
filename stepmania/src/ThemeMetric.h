/* ThemeMetric - Theme specific data. */

#ifndef THEME_METRIC_H
#define THEME_METRIC_H

#include "ThemeManager.h"

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

public:
	/* Initializing with no group and name is allowed; if you do this, you must
	 * call Load() to set them.  This is done to allow initializing cached metrics
	 * in one place for classes that don't receive their m_sName in the constructor
	 * (everything except screens). */
	ThemeMetric( const CString& sGroup = "", const CString& sName = "" ):
		m_sGroup( sGroup ),
		m_sName( sName )
	{
		m_currentValue = T();
		ThemeManager::Subscribe( this );
	}

	ThemeMetric( const ThemeMetric<T> &cpy ):
		IThemeMetric( cpy ),
		m_sGroup( cpy.m_sGroup ),
		m_sName( cpy.m_sName ),
		m_currentValue( cpy.m_currentValue )
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
			THEME->GetMetric( m_sGroup, m_sName, m_currentValue );
	}

	const T& GetValue() const
	{
		ASSERT( m_sName != "" );

		return m_currentValue;
	}
	
	operator const T& () const
	{
		return m_currentValue;
	}

	//Hacks for VC6 for all boolean operators.
	bool operator ! () const { return !m_currentValue; }
	bool operator && ( const T& input ) const { return m_currentValue && input; }
	bool operator || ( const T& input ) const { return m_currentValue || input; }
	bool operator == ( const T& input ) const { return m_currentValue == input; }
};

typedef CString (*MetricName1D)(size_t N);

template <class T>
class ThemeMetric1D : public IThemeMetric
{
	typedef ThemeMetric<T> ThemeMetricT;
	vector<ThemeMetricT> m_metric;

public:
	ThemeMetric1D( const CString& sGroup = "", MetricName1D pfn = NULL, size_t N = 0 )
	{
		Load( sGroup, pfn, N );
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
