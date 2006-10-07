/* Preference - Holds user-chosen preferences that are saved between sessions. */

#ifndef PREFERENCE_H
#define PREFERENCE_H

#include "EnumHelper.h"
#include "RageUtil.h"
#include "LuaManager.h"
class XNode;

struct lua_State;
class IPreference
{
public:
	IPreference( const RString& sName );
	virtual ~IPreference();
	void ReadFrom( const XNode* pNode, bool bIsStatic );
	void WriteTo( XNode* pNode ) const;
	void ReadDefaultFrom( const XNode* pNode );

	virtual void LoadDefault() = 0;
	virtual void SetDefaultFromString( const RString &s ) = 0;

	virtual RString ToString() const = 0;
	virtual void FromString( const RString &s ) = 0;

	virtual void SetFromStack( lua_State *L );
	virtual void PushValue( lua_State *L ) const;

	const RString &GetName() const { return m_sName; }

	static IPreference *GetPreferenceByName( const RString &sName );
	static void LoadAllDefaults();
	static void ReadAllPrefsFromNode( const XNode* pNode, bool bIsStatic );
	static void SavePrefsToNode( XNode* pNode );
	static void ReadAllDefaultsFromNode( const XNode* pNode );

	RString GetName() { return m_sName; }
	void SetStatic( bool b ) { m_bIsStatic = b; }
private:
	RString	m_sName;
	bool m_bIsStatic;	// loaded from Static.ini?  If so, don't write to Preferences.ini
};

void BroadcastPreferenceChanged( const RString& sPreferenceName );

template <class T>
class Preference : public IPreference
{
public:
	Preference( const RString& sName, const T& defaultValue, void (pfnValidate)(T& val) = NULL ):
		IPreference( sName ),
		m_currentValue( defaultValue ),
		m_defaultValue( defaultValue ),
		m_pfnValidate( pfnValidate )
	{
		LoadDefault();
	}

	RString ToString() const { return ::ToString<T>( m_currentValue ); }
	void FromString( const RString &s )
	{
		if( !::FromString<T>(s, m_currentValue) )
			m_currentValue = m_defaultValue;
		if( m_pfnValidate ) 
			m_pfnValidate( m_currentValue );
	}
	void SetFromStack( lua_State *L )
	{
		LuaHelpers::Pop<T>( L, m_currentValue );
		if( m_pfnValidate )
			m_pfnValidate( m_currentValue );
	}
	void PushValue( lua_State *L ) const
	{
		LuaHelpers::Push<T>( L, m_currentValue );
	}

	void LoadDefault()
	{
		m_currentValue = m_defaultValue;
	}
	void SetDefaultFromString( const RString &s )
	{
		T def = m_defaultValue;
		if( !::FromString<T>(s, m_defaultValue) )
			m_defaultValue = def;
	}

	const T &Get() const
	{
		return m_currentValue;
	}
	
	const T &GetDefault() const
	{
		return m_defaultValue;
	}

	operator const T () const
	{
		return Get();
	}
	
	void Set( const T& other )
	{
		m_currentValue = other;
		BroadcastPreferenceChanged( GetName() );
	}

private:
	T m_currentValue;
	T m_defaultValue;
	void (*m_pfnValidate)(T& val);
};

namespace LuaHelpers { template<typename T> void Push( lua_State *L, const Preference<T> &Object ) { LuaHelpers::Push<T>( L, Object.Get() ); } }

template <class T>
class Preference1D
{
public:
	typedef Preference<T> PreferenceT;
	vector<PreferenceT*> m_v;
	
	Preference1D( void pfn(size_t i, RString &sNameOut, T &defaultValueOut ), size_t N )
	{
		for( size_t i=0; i<N; ++i )
		{
			RString sName;
			T defaultValue;
			pfn( i, sName, defaultValue );
			m_v.push_back( new Preference<T>(sName, defaultValue) );
		}
	}

	~Preference1D()
	{
		for( size_t i=0; i<m_v.size(); ++i )
			SAFE_DELETE( m_v[i] );
	}
	const Preference<T>& operator[]( size_t i ) const
	{
		return *m_v[i];
	}
	Preference<T>& operator[]( size_t i )
	{
		return *m_v[i];
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
