#include "global.h"

#include "RageSoundPosMap.h"
#include "RageLog.h"
#include "RageUtil.h"

/* The number of frames we should keep pos_map data for.  This being too high
 * is mostly harmless; the data is small. */
const int pos_map_backlog_frames = 100000;

pos_map_queue::pos_map_queue(): m_Mutex("pos_map_queue")
{
}


pos_map_queue::pos_map_queue( const pos_map_queue &cpy ): m_Mutex("pos_map_queue")
{
	*this = cpy;
}

pos_map_queue &pos_map_queue::operator=( const pos_map_queue &cpy )
{
	/* Hack: to prevent deadlock, always lock the lesser pointer first. */
	if( this < &cpy )
	{
		m_Mutex.Lock();
		cpy.m_Mutex.Lock();
	} else {
		cpy.m_Mutex.Lock();
		m_Mutex.Lock();
	}

	m_Queue = cpy.m_Queue;

	/* Unlock order doesn't matter. */
	m_Mutex.Unlock();
	cpy.m_Mutex.Unlock();
	return *this;
}

void pos_map_queue::Insert( int64_t frameno, int pos, int got_frames )
{
	LockMut(m_Mutex);

	if( m_Queue.size() )
	{
		/* Optimization: If the last entry lines up with this new entry, just merge them. */
		pos_map_t &last = m_Queue.back();
		if( last.frameno+last.frames == frameno &&
		    last.position+last.frames == pos )
		{
			last.frames += got_frames;
			return;
		}
	}

	m_Queue.push_back( pos_map_t( frameno, pos, got_frames ) );
	
	Cleanup();
}

void pos_map_queue::Cleanup()
{
	LockMut(m_Mutex);

	/* Determine the number of frames of data we have. */
	int64_t total_frames = 0;
	for( unsigned i = 0; i < m_Queue.size(); ++i )
		total_frames += m_Queue[i].frames;

	/* Remove the oldest entry so long we'll stil have enough data.  Don't delete every
	 * frame, so we'll always have some data to extrapolate from. */
	while( m_Queue.size() > 1 && total_frames - m_Queue.front().frames > pos_map_backlog_frames )
	{
		total_frames -= m_Queue.front().frames;
		m_Queue.pop_front();
	}
}

int64_t pos_map_queue::Search( int64_t frame, bool *approximate ) const
{
	LockMut(m_Mutex);

	if( IsEmpty() )
	{
		if( approximate )
			*approximate = true;
		return 0;
	}

	/* frame is probably in pos_map.  Search to figure out what position
	 * it maps to. */
	int64_t closest_position = 0, closest_position_dist = INT_MAX;
	int closest_block = 0; /* print only */
	for( unsigned i = 0; i < m_Queue.size(); ++i )
	{
		if( frame >= m_Queue[i].frameno &&
			frame < m_Queue[i].frameno+m_Queue[i].frames )
		{
			/* frame lies in this block; it's an exact match.  Figure
			 * out the exact position. */
			int64_t diff = m_Queue[i].position - m_Queue[i].frameno;
			return frame + diff;
		}

		/* See if the current position is close to the beginning of this block. */
		int64_t dist = llabs( m_Queue[i].frameno - frame );
		if( dist < closest_position_dist )
		{
			closest_position_dist = dist;
			closest_block = i;
			closest_position = m_Queue[i].position - dist;
		}

		/* See if the current position is close to the end of this block. */
		dist = llabs( m_Queue[i].frameno + m_Queue[i].frames - frame );
		if( dist < closest_position_dist )
		{
			closest_position_dist = dist;
			closest_position = m_Queue[i].position + m_Queue[i].frames + dist;
		}
	}

	/* The frame is out of the range of data we've actually sent.
	 * Return the closest position.
	 *
	 * There are three cases when this happens: 
	 * 1. After the first GetPCM call, but before it actually gets heard.
	 * 2. After GetPCM returns EOF and the sound has flushed, but before
	 *    SoundStopped has been called.
	 * 3. Underflow; we'll be given a larger frame number than we know about.
	 */
	/* XXX: %lli normally, %I64i in Windows */
	LOG->Trace( "Approximate sound time: driver frame %lli, m_Queue frame %lli (dist %lli), closest position is %lli",
		frame, m_Queue[closest_block].frameno, closest_position_dist, closest_position );

	if( approximate )
		*approximate = true;
	return closest_position;
}

void pos_map_queue::Clear()
{
	LockMut(m_Mutex);
	m_Queue.clear();
}

bool pos_map_queue::IsEmpty() const
{
	LockMut(m_Mutex);
	return m_Queue.empty();
}

