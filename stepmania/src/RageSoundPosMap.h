#ifndef RAGE_SOUND_POS_MAP_H
#define RAGE_SOUND_POS_MAP_H

#include "RageThreads.h"
#include <deque>

struct pos_map_t
{
	/* Frame number from the POV of the sound driver: */
	int64_t frameno;

	/* Actual sound position within the sample: */
	int64_t position;

	/* The number of frames in this block: */
	int64_t frames;

	pos_map_t() { frameno=0; position=0; frames=0; }
	pos_map_t( int64_t frame, int pos, int cnt ) { frameno=frame; position=pos; frames=cnt; }
};

/* This class maps one range of frames to another. */
class pos_map_queue
{
	deque<pos_map_t> m_Queue;
	mutable RageMutex m_Mutex;

	void Cleanup();

public:
	pos_map_queue();
	pos_map_queue( const pos_map_queue &cpy );
	pos_map_queue &operator=( const pos_map_queue &cpy );

	/* Insert a mapping from frameno to position, containing pos got_frames. */
	void Insert( int64_t frameno, int position, int got_frames );

	/* Return the position for the given frameno. */
	int64_t Search( int64_t frameno, bool *approximate ) const;

	/* Erase all mappings. */
	void Clear();

	bool IsEmpty() const;
};


#endif
