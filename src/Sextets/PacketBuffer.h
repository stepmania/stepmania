#ifndef Sextets_PacketBuffer_h
#define Sextets_PacketBuffer_h

#include "Sextets/Packet.h"
#include "global.h"

namespace Sextets
{
	class PacketBuffer
	{
	public:
		virtual ~PacketBuffer();

		static PacketBuffer * Create();

		// Adds data to this buffer. Returns true iff the buffer, after
		// adding the new data, contains at least one newline.
		virtual void Add(const RString& data) = 0;
		virtual void Add(const RString::value_type * data, size_t length) = 0;
		virtual void Add(const uint8_t * data, size_t length) = 0;

		// If the input buffer contains an unfinished packet, clear it and
		// wait for the next newline to resume buffering.
		virtual void DiscardUnfinished() = 0;

		// Returns whether this buffer may contain data that can be
		// retrieved from GetPacket(). False positives may be possible in a
		// future implementation, so be sure to check the value returned by
		// GetPacket() after it is called.
		virtual bool HasPacket() = 0;

		// Attempts to retrieve a line from the buffer. Returns true iff a
		// line is successfully retrieved and converted to a non-empty
		// packet. Note the returned value from this method; even if the
		// most recent Add() returned true, the data in the buffer may
		// contain an empty or invalid line that cannot be converted.
		virtual bool GetPacket(Packet& packet) = 0;
	};
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
