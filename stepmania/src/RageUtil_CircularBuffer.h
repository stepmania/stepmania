#ifndef RAGE_UTIL_CIRCULAR_BUFFER
#define RAGE_UTIL_CIRCULAR_BUFFER

template<class T>
class CircBuf
{
	basic_string<T> buf;
	unsigned cnt, start;
	
public:
	CircBuf() { clear(); }
	unsigned size() const { return cnt; }
	unsigned capacity() const { return buf.size(); }

	void reserve( unsigned n )
	{
		clear();
		buf.erase();
		buf.insert( buf.end(), n, 0 );
	}

	void clear()
	{
		cnt = start = 0;
	}

	void write( const T *buffer, unsigned buffer_size )
	{
		ASSERT( size() + buffer_size <= capacity() ); /* overflow */

		while( buffer_size )
		{
			unsigned write_pos = start + size();
			if( write_pos >= buf.size() )
				write_pos -= buf.size();
			
			const int cpy = int(min(buffer_size, buf.size() - write_pos));
			buf.replace( write_pos, cpy, buffer, cpy );

			cnt += cpy;

			buffer += cpy;
			buffer_size -= cpy;
		}
	}

	void read( T *buffer, unsigned buffer_size )
	{
		ASSERT( size() >= buffer_size ); /* underflow */
		
		while( buffer_size )
		{
			const unsigned total = static_cast<unsigned>(min(buf.size() - start, size()));
			const unsigned cpy = min( buffer_size, total );
			buf.copy( buffer, cpy, start );

			start += cpy;
			if( start == buf.size() )
				start = 0;
			cnt -= cpy;

			buffer += cpy;
			buffer_size -= cpy;
		}
	}
};

#endif
