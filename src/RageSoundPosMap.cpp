// NOTE:  RageSoundPosMap (as well as a 44100/48000hz sample rate inconsistency) are two of the biggest sources of timing drift that aren't addressed in this commit as is. ITGmania has fixed these issues, but that code is GPL3 licensed so I'll have to make an alternate implementation for StepMania. --sukibaby
#include "global.h"
#include "RageSoundPosMap.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageTimer.h"

#include <climits>
#include <cmath>
#include <cstdint>
#include <list>

// The number of frames we should keep pos_map data for.
// This comes out to about ~800kb in audio frames, assuming 44.1khz.
// File bitrate, metadata, etc will factor in here. If the queue is
// TOO big it will make things slow, but 200k frames is no problem.
// Making the queue larger than 200k hasn't been tested extensively.
const int pos_map_backlog_frames = 200000;

struct pos_map_t
{
	std::int64_t m_iSourceFrame;
	std::int64_t m_iDestFrame;
	std::int64_t m_iFrames;
	double m_fSourceToDestRatio;

	pos_map_t() { m_iSourceFrame = 0; m_iDestFrame = 0; m_iFrames = 0; m_fSourceToDestRatio = 1.0; }
};

struct pos_map_impl
{
	std::list<pos_map_t> m_Queue;
	void Cleanup();
};

pos_map_queue::pos_map_queue()
{
	m_pImpl = new pos_map_impl;
}

pos_map_queue::~pos_map_queue()
{
	delete m_pImpl;
}

pos_map_queue::pos_map_queue( const pos_map_queue &cpy )
{
	*this = cpy;
	m_pImpl = new pos_map_impl( *cpy.m_pImpl );
}

pos_map_queue &pos_map_queue::operator=( const pos_map_queue &rhs )
{
	if (this != &rhs)
	{
		pos_map_impl* tempImpl = new pos_map_impl(*rhs.m_pImpl);
		std::swap(m_pImpl, tempImpl);
		delete tempImpl;
	}
	return *this;
}

void pos_map_queue::Insert(std::int64_t iSourceFrame, std::int64_t iFrames, std::int64_t iDestFrame, double fSourceToDestRatio)
{
	bool merged = false;
	if (!m_pImpl->m_Queue.empty())
	{
		// Check if the last entry can be merged with the new entry
		pos_map_t& last = m_pImpl->m_Queue.back();
		if (last.m_iSourceFrame + last.m_iFrames == iSourceFrame &&
			last.m_fSourceToDestRatio == fSourceToDestRatio &&

			// llabs() is used instead of abs() because abs() would be susceptible to an integer overflow.
			llabs(last.m_iDestFrame + static_cast<int64_t>((last.m_iFrames * last.m_fSourceToDestRatio) + 0.5) - iDestFrame) <= 1)

		{
			// Merge the frames and set the merged flag to true.
			last.m_iFrames += iFrames;
			merged = true;
		}
	}

	if (!merged)
	{
		m_pImpl->m_Queue.push_back(pos_map_t());
		pos_map_t& m = m_pImpl->m_Queue.back();
		m.m_iSourceFrame = iSourceFrame;
		m.m_iDestFrame = iDestFrame;
		m.m_iFrames = iFrames;
		m.m_fSourceToDestRatio = fSourceToDestRatio;
	}

	m_pImpl->Cleanup();
}

void pos_map_impl::Cleanup()
{
	std::list<pos_map_t>::iterator it = m_Queue.end();
	std::int64_t iTotalFrames = 0;
	// Scan backwards until we have at least pos_map_backlog_frames.
	while (iTotalFrames < pos_map_backlog_frames)
	{
		if (it == m_Queue.begin())
			break;
		--it;
		iTotalFrames += it->m_iFrames;
	}

	m_Queue.erase(m_Queue.begin(), it);
}

std::int64_t pos_map_queue::Search( std::int64_t iSourceFrame, bool *bApproximate ) const
{
	if( IsEmpty() )
	{
		return 0;
	}

	// iSourceFrame is probably in pos_map.  Search to figure out what position it maps to.
	std::int64_t iClosestPosition = 0, iClosestPositionDist = std::numeric_limits<int64_t>::max();
	for (pos_map_t const &pm : m_pImpl->m_Queue)
	{
		// Loop over the queue until we know generally where iSourceFrame is
		if( iSourceFrame >= pm.m_iSourceFrame &&
			iSourceFrame < pm.m_iSourceFrame+pm.m_iFrames )
		{
			// If we are in the correct block, calculate its current position
			std::int64_t iDiff = static_cast<std::int64_t>(iSourceFrame - pm.m_iSourceFrame);
			iDiff = static_cast<int64_t>(( iDiff * pm.m_fSourceToDestRatio) + 0.5 ); 
			return pm.m_iDestFrame + iDiff;
		}

		// See if the current position is close to the beginning of this block.
		std::int64_t dist = llabs( pm.m_iSourceFrame - iSourceFrame );
		if( dist < iClosestPositionDist )
		{
			iClosestPositionDist = dist;
			iClosestPosition = pm.m_iDestFrame;
		}

		// See if the current position is close to the end of this block.
		dist = llabs( pm.m_iSourceFrame + pm.m_iFrames - iSourceFrame );
		if( dist < iClosestPositionDist )
		{
			iClosestPositionDist = dist;
			iClosestPosition = pm.m_iDestFrame + static_cast<int64_t>((pm.m_iFrames * pm.m_fSourceToDestRatio) + 0.5 );
		}
	}

	/*
	 * The frame is out of the range of data we've actually sent.
	 * Return the closest position.
	 *
	 * There are three cases when this happens:
	 * 1. Before the first CommitPlayingPosition call.
	 * 2. After GetDataToPlay returns EOF and the sound has flushed, but before
	 *    SoundStopped has been called.
	 * 3. Underflow; we'll be given a larger frame number than we know about.
	 */
	static RageTimer last;
	if( last.PeekDeltaTime() >= 1.0f )
	{
		last.Touch();
		LOG->Trace("Audio frame was out of range of the data sent - possible buffer underflow? This is not always an error, however if you see it frequently there could be sound buffer problems.");
	}

	return iClosestPosition;
}

void pos_map_queue::Clear()
{
	m_pImpl->m_Queue.clear();
}

bool pos_map_queue::IsEmpty() const
{
	return m_pImpl->m_Queue.empty();
}

/*
 * Copyright (c) 2002-2004 Glenn Maynard
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
