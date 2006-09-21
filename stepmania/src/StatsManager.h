/* ThemeManager - Managed non-persisted statistics. */

#ifndef StatsManager_H
#define StatsManager_H

#include "StageStats.h"

class StatsManager
{
public:
	StatsManager();
	~StatsManager();

	void Reset();

	StageStats		m_CurStageStats;	// current stage (not necessarily passed if Extra Stage)
	vector<StageStats>	m_vPlayedStageStats;

	// Only the latest 3 normal songs + passed extra stages.
	void GetFinalEvalStageStats( StageStats& statsOut ) const;

	// All stages played.  Returns a ref to the private member so that
	// the object will remain alive while Lua is operating on it.
	void CalcAccumPlayedStageStats();
	StageStats& GetAccumPlayedStageStats() { return m_AccumPlayedStageStats; }

	void CommitStatsToProfiles();

	// Lua
	void PushSelf( lua_State *L );

private:
	StageStats m_AccumPlayedStageStats;
};


extern StatsManager*	STATSMAN;	// global and accessable from anywhere in our program
	
#endif

/*
 * (c) 2001-2004 Chris Danford
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
