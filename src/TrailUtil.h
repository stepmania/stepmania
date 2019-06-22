#ifndef TRAIL_UTIL_H
#define TRAIL_UTIL_H

#include "GameConstantsAndTypes.h"
#include "Difficulty.h"
#include "RageUtil_CachedObject.h"

class Song;
class Trail;
class Course;
class XNode;

/** @brief Utility functions for dealing with the Trail. */
namespace TrailUtil
{
	/**
	 * @brief Retrieve the number of 
	 * <a class="el" href="class_song.html">Songs</a> in the Trail.
	 * @param pTrail the Trail itself.
	 * @return the number of <a class="el" href="class_song.html">Songs</a>. */
	int GetNumSongs( const Trail *pTrail );
	/**
	 * @brief Retrieve how long the Trail will last in seconds.
	 * @param pTrail the Trail itself.
	 * @return the total run time of the Trail. */
	float GetTotalSeconds( const Trail *pTrail );
};

class TrailID
{
	StepsType st;
	CourseDifficulty cd;
	mutable CachedObjectPointer<Trail> m_Cache;

public:
	TrailID(): st(StepsType_Invalid), cd(Difficulty_Invalid),
		m_Cache() { m_Cache.Unset(); }
	void Unset() { FromTrail(nullptr); }
	void FromTrail( const Trail *p );
	Trail *ToTrail( const Course *p, bool bAllowNull ) const;
	bool operator<( const TrailID &rhs ) const;
	bool MatchesStepsType( StepsType s ) const { return st == s; }

	XNode* CreateNode() const;
	void LoadFromNode( const XNode* pNode );
	RString ToString() const;
	bool IsValid() const;
	static void Invalidate( Song* pStaleSong );
};

#endif

/*
 * (c) 2004 Chris Danford
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
