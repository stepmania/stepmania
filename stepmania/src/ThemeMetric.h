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
		if( m_sName != "" )
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
