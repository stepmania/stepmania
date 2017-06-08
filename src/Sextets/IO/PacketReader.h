#ifndef Sextets_IO_PacketReader_h
#define Sextets_IO_PacketReader_h

#include "global.h"
#include "Sextets/Packet.h"

namespace Sextets
{
	namespace IO
	{
		// Interface for classes that implement the PacketReader protocol
		// for Sextets.
		class PacketReader
		{
			public:
				virtual ~PacketReader() {}

				// Returns true if this stream is currently in such a state
				// that ReadPacket() will return a valid response. A false
				// value may indicate that an underlying stream is closed or
				// has encountered an unrecoverable error condition. Do not
				// call ReadPacket() unless this returns true. Once false
				// has been returned, this method will not return true for
				// the same object.
				virtual bool IsReady() = 0;

				// Retrieves the next packet available from the stream.
				//
				// Returns true with packet set to a non-empty string if a
				// whole packet is immediately available for read.
				//
				// Returns false with packet undefined if the stream can no
				// longer be read due to EOF or an unrecoverable error
				// condition.
				//
				// Returns true with packet set to "" if the stream is open
				// but a non-blocking read has not read an entire packet
				// yet.
				//
				// Whenever possible, this should be implemented on a
				// non-blocking read. If this method blocks, the loop that
				// checks continueInputThread may be paused indefinitely,
				// which can cause the application to hang, especially while
				// closing. To mitigate this effect, make sure the stream is
				// never idle: have the far side of the connection repeat
				// its most recent message at a small interval (for example,
				// every second) in order to allow the check at least that
				// often. Closing the far side of the connection will also
				// end the blocking, but this is only really useful while
				// closing.
				virtual bool ReadPacket(Packet& packet) = 0;
		};
	}
}

#endif

/*
 * Copyright © 2016 Peter S. May
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
