#ifndef SONGPOSITION_H
#define SONGPOSITION_H

#include "GameConstantsAndTypes.h"
#include "RageTimer.h"
#include "LightsManager.h"
#include "MessageManager.h"
#include "TimingData.h"
struct lua_State;

class SongPosition
{
public:
	// Arcade - the current stage (one song).
	// Oni/Endless - a single song in a course.
	// Let a lot of classes access this info here so they don't have to keep their own copies.
	float m_fMusicSeconds;		// time into the current song, not scaled by music rate
	float m_fSongBeat;
	float m_fSongBeatNoOffset;
	float m_fCurBPS;
	float m_fLightSongBeat;		// g_fLightsFalloffSeconds ahead
	//bool m_bStop;			// in the middle of a stop (freeze or delay)
	bool m_bFreeze;			// A flag to determine if we're in the middle of a freeze/stop.
	bool m_bDelay;			// A flag to determine if we're in the middle of a delay (Pump style stop).
	int m_iWarpBeginRow;		// The row used to start a warp.
	float m_fWarpDestination;	// The beat to warp to afterwards.
	RageTimer m_LastBeatUpdate;	// time of last m_fSongBeat, etc. update
	float m_fMusicSecondsVisible;
	float m_fSongBeatVisible;

	SongPosition()
		: m_fMusicSeconds(0.0f),
		  m_fSongBeat(0.0f),
		  m_fSongBeatNoOffset(0.0f),
		  m_fCurBPS(0.0f),
		  m_fLightSongBeat(0.0f),
		  //m_bStop(false),
		  m_bFreeze(false),
		  m_bDelay(false),
		  m_iWarpBeginRow(0),
		  m_fWarpDestination(0.0f),
		  m_LastBeatUpdate(RageZeroTimer),
		  m_fMusicSecondsVisible(0.0f),
		  m_fSongBeatVisible(0.0f)
	{
	}

	void Reset();
	
	void UpdateSongPosition(
		float fPositionSeconds,
		const TimingData &timing,
		const RageTimer &timestamp = RageZeroTimer,
		float fAdditionalVisualDelay = 0.0f
	);

	// Lua
	void PushSelf( lua_State *L );
};

#endif // SONGPOSITION_H

/**
 * @file
 * @author Thai Pangsakulyanont (c) 2011
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
