/* ModLevel -  */

#ifndef ModLevel_H
#define ModLevel_H

#include "EnumHelper.h"

enum ModsLevel
{
	ModsLevel_Preferred,	// user-chosen player options.  Does not include any forced mods.
	ModsLevel_Stage,	// Preferred + forced stage mods
	ModsLevel_Song,		// Stage + forced attack mods
	NUM_ModsLevel,
};

#define MODS_GROUP_ASSIGN( group, level, member, val ) \
	(group).Assign( (level), &(group).GroupType::member, (val) )
#define MODS_GROUP_CALL( group, level, fun ) (group).Call( (level), &(group).GroupType::fun )
#define MODS_GROUP_TOGGLE( group, level, member ) (group).Call( (level), &(group).GroupType::member )

template<class T>
struct ModsGroup
{
	typedef T GroupType;
	T m_[NUM_ModsLevel];
	RageTimer		m_timer;
	T m_current;		// approaches ModsLevel_Song

	void Init()
	{
		Call( ModsLevel_Preferred, &T::Init );
	}

	void Update( float fDelta )
	{
		// Don't let the mod approach speed be affected by Tab.
		// TODO: Find a more elegant way of handling this.
		fDelta = m_timer.GetDeltaTime();
		m_current.Approach( m_[ModsLevel_Song], fDelta );
	}
	
	template<typename U>
	void Assign( ModsLevel level, U T::*member, const U &val )
	{
		if( level != ModsLevel_Song )
			m_current.*member = val;
		for( ; level < NUM_ModsLevel; enum_add(level, 1) )
			m_[level].*member = val;
	}
	
	void Assign( ModsLevel level, const T &val )
	{
		if( level != ModsLevel_Song )
			m_current = val;
		for( ; level < NUM_ModsLevel; enum_add(level, 1) )
			m_[level] = val;
	}
	
	void Call( ModsLevel level, void (T::*fun)() )
	{
		if( level != ModsLevel_Song )
			(m_current.*fun)();
		for( ; level < NUM_ModsLevel; enum_add(level, 1) )
			(m_[level].*fun)();
	}
	
	void FromString( ModsLevel level, const RString &str )
	{
		if( level != ModsLevel_Song )
			m_current.FromString( str );
		for( ; level < NUM_ModsLevel; enum_add(level, 1) )
			m_[level].FromString( str );
	}
	
	// This seems like a very strange thing to do.
	void Toggle( ModsLevel level, bool T::*member )
	{
		if( level != ModsLevel_Song )
			m_current.*member ^= 1;
		for( ; level < NUM_ModsLevel; enum_add(level, 1) )
			m_[level].*member ^= 1;
	}

	const T &GetPreferred() const	{ return m_[ModsLevel_Preferred]; }
	const T &GetStage() const	{ return m_[ModsLevel_Stage]; }
	const T &GetSong() const	{ return m_[ModsLevel_Song]; }
	const T &GetCurrent() const	{ return m_current; }
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
