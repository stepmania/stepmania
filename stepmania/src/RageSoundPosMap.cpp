#include "global.h"
#include "RageSoundPosMap.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageTimer.h"
#include "Foreach.h"

#include <list>

/* The number of frames we should keep pos_map data for.  This being too high
 * is mostly harmless; the data is small. */
const int pos_map_backlog_frames = 100000;

struct pos_map_t
{
	int64_t m_iSourceFrame;
	int64_t m_iDestFrame;
	int64_t m_iFrames;

	pos_map_t() { m_iSourceFrame = 0; m_iDestFrame = 0; m_iFrames = 0; }
};

struct pos_map_impl
{
	list<pos_map_t> m_Queue;
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
	delete m_pImpl;
	m_pImpl = new pos_map_impl( *rhs.m_pImpl );
	return *this;
}

void pos_map_queue::Insert( int64_t iSourceFrame, int iFrames, int64_t iDestFrame )
{
	if( m_pImpl->m_Queue.size() )
	{
		/* Optimization: If the last entry lines up with this new entry, just merge them. */
		pos_map_t &last = m_pImpl->m_Queue.back();
		if( last.m_iSourceFrame+last.m_iFrames == iSourceFrame &&
		    last.m_iDestFrame+last.m_iFrames == iDestFrame )
		{
			last.m_iFrames += iFrames;
			return;
		}
	}

	m_pImpl->m_Queue.push_back( pos_map_t() );
	pos_map_t &m = m_pImpl->m_Queue.back();
	m.m_iSourceFrame = iSourceFrame;
	m.m_iDestFrame = iDestFrame;
	m.m_iFrames = iFrames;
	
	m_pImpl->Cleanup();
}

void pos_map_impl::Cleanup()
{
	/* Scan backwards until we have at least pos_map_backlog_frames. */
	list<pos_map_t>::iterator it = m_Queue.end();
	int iTotalFrames = 0;
	while( iTotalFrames < pos_map_backlog_frames )
	{
		if( it == m_Queue.begin() )
			break;
		--it;
		iTotalFrames += it->m_iFrames;
	}

	m_Queue.erase( m_Queue.begin(), it );
}

int64_t pos_map_queue::Search( int64_t iSourceFrame, bool *bApproximate ) const
{
	if( IsEmpty() )
	{
		if( bApproximate )
			*bApproximate = true;
		return 0;
	}

	/* iSourceFrame is probably in pos_map.  Search to figure out what position
	 * it maps to. */
	int64_t iClosestPosition = 0, iClosestPositionDist = INT_MAX;
	const pos_map_t *pClosestBlock = &*m_pImpl->m_Queue.begin(); /* print only */
	FOREACHL_CONST( pos_map_t, m_pImpl->m_Queue, it )
	{
		const pos_map_t &pm = *it;

		if( iSourceFrame >= pm.m_iSourceFrame &&
			iSourceFrame < pm.m_iSourceFrame+pm.m_iFrames )
		{
			/* iSourceFrame lies in this block; it's an exact match.  Figure
			 * out the exact position. */
			int64_t diff = pm.m_iDestFrame - pm.m_iSourceFrame;
			return iSourceFrame + diff;
		}

		/* See if the current position is close to the beginning of this block. */
		int64_t dist = llabs( pm.m_iSourceFrame - iSourceFrame );
		if( dist < iClosestPositionDist )
		{
			iClosestPositionDist = dist;
			pClosestBlock = &pm;
			iClosestPosition = pm.m_iDestFrame;
		}

		/* See if the current position is close to the end of this block. */
		dist = llabs( pm.m_iSourceFrame + pm.m_iFrames - iSourceFrame );
		if( dist < iClosestPositionDist )
		{
			iClosestPositionDist = dist;
			pClosestBlock = &pm;
			iClosestPosition = pm.m_iDestFrame + pm.m_iFrames;
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
		LOG->Trace( "Approximate sound time: driver frame " LI ", m_pImpl->m_Queue frame " LI ".." LI " (dist " LI "), closest position is " LI,
			iSourceFrame, pClosestBlock->m_iSourceFrame, pClosestBlock->m_iSourceFrame+pClosestBlock->m_iFrames,
			iClosestPositionDist, iClosestPosition );
	}

	if( bApproximate )
		*bApproximate = true;
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
