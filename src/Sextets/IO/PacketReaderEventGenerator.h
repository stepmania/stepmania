#ifndef Sextets_IO_PacketReaderEventGenerator_h
#define Sextets_IO_PacketReaderEventGenerator_h

#include "Sextets/IO/PacketReader.h"

namespace Sextets
{
	namespace IO
	{
		// Class that spawns a thread to read from a PacketReader and relay
		// the results to a callback. The given PacketReader is destroyed when
		// this object is destroyed.
		class PacketReaderEventGenerator
		{
			public:
				typedef void PacketReaderEventCallback(void * context, const Packet& packet);

				// Creates a new PacketReaderEventGenerator with the given
				// parameters. packetReader and onChange must be specified;
				// otherwise, NULL is returned. If packetReader is specified
				// but onChange is NULL, packetReader is deleted before this
				// returns NULL.
				static PacketReaderEventGenerator * Create(
						PacketReader * packetReader,
						void * context,
						PacketReaderEventCallback * onChange);

				// Returns false if the thread has not started yet, or true
				// if it did start. If HasEnded() is true, then this must be
				// true also.
				virtual bool HasStarted() = 0;

				// Returns true if the thread has already started and ended.
				virtual bool HasEnded() = 0;

				// If the thread is currently running, calling this will
				// cause the read loop to stop after the current iteration.
				// If the thread is not yet started, having called this will
				// prevent the loop from starting its first iteration.
				virtual void RequestStop() = 0;
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
