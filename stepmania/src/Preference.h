/* Preference - Holds user-chosen preferences that are saved between sessions. */

#ifndef PREFERENCE_H
#define PREFERENCE_H

#include "IniFile.h"
#include "PrefsManager.h"

enum PrefsGroup
{
	Debug,
	Editor,
	Options,
};

inline CString PrefsGroupToString( PrefsGroup pg )
{
	switch( pg )
	{
	case Debug:		return "Debug";
	case Editor:	return "Editor";
	case Options:	return "Options";
	default:	ASSERT(0); return "";
	}
}

class IPreference
{
public:
	virtual ~IPreference() { }
	virtual void LoadDefault() = 0;
	virtual void ReadFrom( const IniFile &ini ) = 0;
	virtual void WriteTo( IniFile &ini ) const = 0;
};

template <class T>
class Preference : public IPreference
{
private:
	PrefsGroup	m_PrefsGroup;
	CString		m_sName;
	T			m_defaultValue;
	T			m_currentValue;

public:
	Preference( PrefsGroup PrefsGroup, const CString& sName, const T& defaultValue ):
		m_PrefsGroup( PrefsGroup ),
		m_sName( sName ),
		m_defaultValue( defaultValue ),
		m_currentValue( defaultValue )
	{
		Subscribe( this );
	}

	void LoadDefault()
	{
		m_currentValue = m_defaultValue;
	}

	void ReadFrom( const IniFile &ini )
	{
		ini.GetValue( PrefsGroupToString(m_PrefsGroup), m_sName, m_currentValue );
	}

	void WriteTo( IniFile &ini ) const
	{
		ini.SetValue( PrefsGroupToString(m_PrefsGroup), m_sName, m_currentValue );
	}

	const T& GetValue() const
	{
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
