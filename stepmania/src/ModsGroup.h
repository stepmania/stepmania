/* ModLevel -  */

#ifndef ModLevel_H
#define ModLevel_H

enum ModsLevel
{
	ModsLevel_Preferred,	// user-chosen player options.  Does not include any forced mods.
	ModsLevel_Stage,	// Preferred + forced stage mods
	ModsLevel_Song,		// Stage + forced attack mods
	NUM_ModsLevel,
};

#define MODS_GROUP_ASSIGN( modsGroup, modsLevel, assignment ) \
	{ \
	for( int i=(ModsLevel)(modsLevel); i<NUM_ModsLevel; i++ ) \
		(modsGroup).m_[i] assignment; \
	if( modsLevel != ModsLevel_Song ) \
		(modsGroup).m_current assignment; \
	}

template<class T>
struct ModsGroup
{
	T m_[NUM_ModsLevel];
	RageTimer		m_timer;
	T m_current;		// approaches ModsLevel_Song

	void Init()
	{
		MODS_GROUP_ASSIGN( *this, (ModsLevel)0, .Init() );
		m_current.Init();
	}

	void Update( float fDelta )
	{
		// Don't let the mod approach speed be affected by Tab.
		// TODO: Find a more elegant way of handling this.
		fDelta = m_timer.GetDeltaTime();
		m_current.Approach( m_[ModsLevel_Song], fDelta );
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
