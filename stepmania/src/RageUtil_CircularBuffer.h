#ifndef RAGE_UTIL_CIRCULAR_BUFFER
#define RAGE_UTIL_CIRCULAR_BUFFER

/* Lock-free circular buffer.  This should be threadsafe if one thread is reading
 * and another is writing. */
template<class T>
class CircBuf
{
	T *buf;
	/* read_pos is the position data is read from; write_pos is the position
	 * data is written to.  If read_pos == write_pos, the buffer is empty.
	 *
	 * There will always be at least one position empty, as a completely full
	 * buffer (read_pos == write_pos) is indistinguishable from an empty buffer.
	 *
	 * Invariants: read_pos < size, write_pos < size. */
	unsigned size;

	/* These are volatile to prevent reads and writes to them from being optimized. */
	volatile unsigned read_pos, write_pos;

public:
	CircBuf()
	{
		buf = NULL;
		clear();
	}

	~CircBuf()
	{
		delete[] buf;
	}
		
	/* Return the number of elements available to read. */
	unsigned num_readable() const
	{
		const int rpos = read_pos;
		const int wpos = write_pos;
		if( rpos < wpos )
			/* The buffer looks like "eeeeDDDDeeee" (e = empty, D = data). */
			return wpos - rpos;
		else if( rpos > wpos )
			/* The buffer looks like "DDeeeeeeeeDD" (e = empty, D = data). */
			return size - (rpos - wpos);
		else // if( rpos == wpos )
			/* The buffer looks like "eeeeeeeeeeee" (e = empty, D = data). */
			return 0;
	}
	
	/* Return the number of elements writable.  Note that there must always
	 * be one */
	unsigned num_writable() const
	{
		const int rpos = read_pos;
		const int wpos = write_pos;

		int ret;
		if( rpos < wpos )
			/* The buffer looks like "eeeeDDDDeeee" (e = empty, D = data). */
			ret = size - (wpos - rpos);
		else if( rpos > wpos )
			/* The buffer looks like "DDeeeeeeeeDD" (e = empty, D = data). */
			ret = rpos - wpos;
		else // if( rpos == wpos )
			/* The buffer looks like "eeeeeeeeeeee" (e = empty, D = data). */
			ret = size;

		/* Subtract one, to account for the element that we never fill. */
		return ret - 1;
	}

	unsigned capacity() const { return size; }

	void reserve( unsigned n )
	{
		/* Reserve an extra byte.  We'll never fill more than n bytes; the extra
		 * byte is to guarantee that read_pos != write_pos when the buffer is full,
		 * since that would be ambiguous with an empty buffer. */
		clear();
		delete[] buf;
		buf = new T[n+1];
		size = n+1;
	}

	void clear()
	{
		read_pos = write_pos = 0;
	}

	/* Indicate that n elements have been written. */
	void advance_write_pointer( int n )
	{
		write_pos = (write_pos + n) % size;
	}
	
	/* Indicate that n elements have been read. */
	void advance_read_pointer( int n )
	{
		read_pos = (read_pos + n) % size;
	}
	
	void get_write_pointers( T *pPointers[2], unsigned pSizes[2] )
	{
		const int rpos = read_pos;
		const int wpos = write_pos;

		if( rpos <= wpos )
		{
			/* The buffer looks like "eeeeDDDDeeee" or "eeeeeeeeeeee" (e = empty, D = data). */
			pPointers[0] = buf+wpos;
			pPointers[1] = buf;

			pSizes[0] = size - wpos;
			pSizes[1] = rpos;
		}
		else if( rpos > wpos )
		{
			/* The buffer looks like "DDeeeeeeeeDD" (e = empty, D = data). */
			pPointers[0] = buf+wpos;
			pPointers[1] = NULL;

			pSizes[0] = rpos - wpos;
			pSizes[1] = 0;
		}

		/* Subtract one, to account for the element that we never fill. */
		if( pSizes[1] )
			--pSizes[1];
		else
			--pSizes[0];
	}

	void get_read_pointers( T *pPointers[2], unsigned pSizes[2] )
	{
		const int rpos = read_pos;
		const int wpos = write_pos;

		if( rpos < wpos )
		{
			/* The buffer looks like "eeeeDDDDeeee" (e = empty, D = data). */
			pPointers[0] = buf+rpos;
			pPointers[1] = NULL;

			pSizes[0] = wpos - rpos;
			pSizes[1] = 0;
		}
		else if( rpos > wpos )
		{
			/* The buffer looks like "DDeeeeeeeeDD" (e = empty, D = data). */
			pPointers[0] = buf+rpos;
			pPointers[1] = buf;

			pSizes[0] = size - rpos;
			pSizes[1] = wpos;
		}
		else
		{
			/* The buffer looks like "eeeeeeeeeeee" (e = empty, D = data). */
			pPointers[0] = NULL;
			pPointers[1] = NULL;

			pSizes[0] = 0;
			pSizes[1] = 0;
		}
	}
	
	/* Write buffer_size elements from buffer, and advance the write pointer.  If
	 * the data will not fit entirely, the write pointer will be unchanged
	 * and false will be returned. */
	bool write( const T *buffer, unsigned buffer_size )
	{
		T *p[2];
		unsigned sizes[2];
		get_write_pointers( p, sizes );

		if( buffer_size > sizes[0] + sizes[1] )
			return false;
		
		const int from_first = min( buffer_size, sizes[0] );
		memcpy( p[0], buffer, from_first*sizeof(T) );
		if( buffer_size > sizes[0] )
			memcpy( p[1], buffer+from_first, max(buffer_size-sizes[0], 0u)*sizeof(T) );

		advance_write_pointer( buffer_size );

		return true;
	}

	/* Read buffer_size elements from buffer, and advance the read pointer.  If
	 * the buffer can not be filled completely, the read pointer will be unchanged
	 * and false will be returned. */
	bool read( T *buffer, unsigned buffer_size )
	{
		T *p[2];
		unsigned sizes[2];
		get_read_pointers( p, sizes );

		if( buffer_size > sizes[0] + sizes[1] )
			return false;

		const int from_first = min( buffer_size, sizes[0] );
		memcpy( buffer, p[0], from_first*sizeof(T) );
		if( buffer_size > sizes[0] )
			memcpy( buffer+from_first, p[1], max(buffer_size-sizes[0], 0u)*sizeof(T) );

		/* Set the data that we just read to 0xFF.  This way, if we're passing pointesr
		 * through, we can tell if we accidentally get a stale pointer. */
		memset( p[0], 0xFF, from_first*sizeof(T) );
		if( buffer_size > sizes[0] )
			memset( p[1], 0xFF, max(buffer_size-sizes[0], 0u)*sizeof(T) );

		advance_read_pointer( buffer_size );
		return true;
	}
};

#endif
