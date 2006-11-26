#include "global.h"
#include "RageSoundPosMap.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageTimer.h"

/* The number of frames we should keep pos_map data for.  This being too high
 * is mostly harmless; the data is small. */
const int pos_map_backlog_frames = 100000;

pos_map_queue::pos_map_queue()
{
}


pos_map_queue::pos_map_queue( const pos_map_queue &cpy )
{
	*this = cpy;
}

void pos_map_queue::CommitPosition( int64_t iFrame, int iPosition, int iGotFrames )
{
	if( m_Queue.size() )
	{
		/* Optimization: If the last entry lines up with this new entry, just merge them. */
		pos_map_t &last = m_Queue.back();
		if( last.m_iFrameNo+last.m_iFrames == iFrame &&
		    last.m_iPosition+last.m_iFrames == iPosition )
		{
			last.m_iFrames += iGotFrames;
			return;
		}
	}

	m_Queue.push_back( pos_map_t(iFrame, iPosition, iGotFrames) );
	
	Cleanup();
}

void pos_map_queue::Cleanup()
{
	/* Determine the number of frames of data we have. */
	int64_t iTotalFrames = 0;
	for( unsigned i = 0; i < m_Queue.size(); ++i )
		iTotalFrames += m_Queue[i].m_iFrames;

	/* Remove the oldest entry so long we'll stil have enough data.  Don't delete every
	 * frame, so we'll always have some data to extrapolate from. */
	while( m_Queue.size() > 1 && iTotalFrames - m_Queue.front().m_iFrames > pos_map_backlog_frames )
	{
		iTotalFrames -= m_Queue.front().m_iFrames;
		m_Queue.pop_front();
	}
}

int64_t pos_map_queue::Search( int64_t iFrame, bool *bApproximate ) const
{
	if( IsEmpty() )
	{
		if( bApproximate )
			*bApproximate = true;
		return 0;
	}

	/* iFrame is probably in pos_map.  Search to figure out what position
	 * it maps to. */
	int64_t iClosestPosition = 0, iClosestPositionDist = INT_MAX;
	int iClosestBlock = 0; /* print only */
	for( unsigned i = 0; i < m_Queue.size(); ++i )
	{
		if( iFrame >= m_Queue[i].m_iFrameNo &&
			iFrame < m_Queue[i].m_iFrameNo+m_Queue[i].m_iFrames )
		{
			/* iFrame lies in this block; it's an exact match.  Figure
			 * out the exact position. */
			int64_t diff = m_Queue[i].m_iPosition - m_Queue[i].m_iFrameNo;
			return iFrame + diff;
		}

		/* See if the current position is close to the beginning of this block. */
		int64_t dist = llabs( m_Queue[i].m_iFrameNo - iFrame );
		if( dist < iClosestPositionDist )
		{
			iClosestPositionDist = dist;
			iClosestBlock = i;
			iClosestPosition = m_Queue[i].m_iPosition;
		}

		/* See if the current position is close to the end of this block. */
		dist = llabs( m_Queue[i].m_iFrameNo + m_Queue[i].m_iFrames - iFrame );
		if( dist < iClosestPositionDist )
		{
			iClosestPositionDist = dist;
			iClosestBlock = i;
			iClosestPosition = m_Queue[i].m_iPosition + m_Queue[i].m_iFrames;
		}
	}

	/*
	 * The frame is out of the range of data we've actually sent.
	 * Return the closest position.
	 *
	 * There are three cases when this happens: 
	 * 1. After the first GetPCM call, but before it actually gets heard.
	 * 2. After GetPCM returns EOF and the sound has flushed, but before
	 *    SoundStopped has been called.
	 * 3. Underflow; we'll be given a larger frame number than we know about.
	 */
#if defined(WIN32)
#define LI "%I64i"
#else
#define LI "%lli"
#endif
	static RageTimer last;
	if( last.PeekDeltaTime() >= 1.0f )
	{
		last.GetDeltaTime();
		LOG->Trace( "Approximate sound time: driver frame " LI ", m_Queue frame " LI ".." LI " (dist " LI "), closest position is " LI,
			iFrame, m_Queue[iClosestBlock].m_iFrameNo, m_Queue[iClosestBlock].m_iFrameNo+m_Queue[iClosestBlock].m_iFrames,
			iClosestPositionDist, iClosestPosition );
	}

	if( bApproximate )
		*bApproximate = true;
	return iClosestPosition;
}

void pos_map_queue::Clear()
{
	m_Queue.clear();
}

bool pos_map_queue::IsEmpty() const
{
	return m_Queue.empty();
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
