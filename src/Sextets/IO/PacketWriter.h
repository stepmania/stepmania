#ifndef Sextets_IO_PacketWriter_h
#define Sextets_IO_PacketWriter_h

#include "global.h"
#include "Sextets/Packet.h"

// Sextets/IO/PacketWriter.h
namespace Sextets
{
	namespace IO
	{
		class PacketWriter
		{
		public:
			PacketWriter()
			{
			}
			virtual ~PacketWriter()
			{
			}

			// Returns whether this stream can currently write.
			virtual bool IsReady() = 0;

			// Writes the provided packet (and generally also an LF or CRLF,
			// depending on the intended receiver) to this packet writer.
			virtual bool WritePacket(const Packet& packet) = 0;
		};
	}
}

#endif

/*
 * Copyright © 2014-2016 Peter S. May
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
