#ifndef StatsManager_H
#define StatsManager_H

#include "StageStats.h"

#include <vector>


/** @brief Managed non-persisted statistics. */
class StatsManager
{
public:
    StatsManager();
    ~StatsManager();

    void Reset();

    /**
     * @brief The current Stage stats.
     *
     * This is not necessarily passed stage stats if this is an Extra Stage. */
    StageStats		m_CurStageStats;
    std::vector<StageStats>	m_vPlayedStageStats;

    // Only the latest 3 normal songs + passed extra stages.
    void GetFinalEvalStageStats( StageStats& statsOut ) const;

    // All stages played.  Returns a ref to the private member so that
    // the object will remain alive while Lua is operating on it.
    void CalcAccumPlayedStageStats();
    StageStats& GetAccumPlayedStageStats() { return m_AccumPlayedStageStats; }

    static void CommitStatsToProfiles( const StageStats *pSS );

    void UnjoinPlayer( PlayerNumber pn );
    void GetStepsInUse( std::set<Steps*> &apInUseOut ) const;

    // Lua
    void PushSelf( lua_State *L );

protected:
    static void SaveUploadFile( const StageStats *pSS );
    static void SavePadmissScore( const StageStats *pSS, PlayerNumber pn );

private:
    StageStats m_AccumPlayedStageStats;
};

extern StatsManager*	STATSMAN;	// global and accessible from anywhere in our program

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
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
