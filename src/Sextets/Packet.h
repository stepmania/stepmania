#ifndef Sextets_Packet_h
#define Sextets_Packet_h

#include "global.h"

// Needed for GetLightsStateAsPacket
struct LightsState;

namespace Sextets
{
	class Packet
	{
	public:
		Packet();
		Packet(const Packet& packet);
		~Packet();

		size_t SextetCount() const;

		void Clear();

		void Canonicalize();

		void Copy(const Packet& packet);
		Packet& operator=(const Packet& other);

		// Sets this packet to the first span of valid sextet characters
		// within the string. All characters before the first valid
		// character, and all characters starting with the first invalid
		// character after the first valid character, are discarded.
		void SetToLine(const std::string& line);

		// Sets this packet to the values corresponding to the given lights
		// state.
		void SetToLightsState(const LightsState * ls);

		// Calculates this XOR b, then assigns the result to this
		// packet.
		void SetToXor(const Packet& b);
		Packet& operator^=(const Packet& other);

		// Calculates a XOR b, then assigns the result to this packet.
		void SetToXor(const Packet& a, const Packet& b);

		typedef void (*ProcessEventCallback)(void * context,
											 size_t bitIndex, bool value);

		// Examines the low `bitCount` bits of `eventData` and, for each
		// `1` bit found, calls a callback with the bit index and value
		// of the bit at the same index of this packet.
		void ProcessEventData(const Packet& eventData, size_t bitCount,
							  void * context, ProcessEventCallback callback) const;

		// Iterates over the low `bitCount` bits of this packet, calling the
		// callback with the index and value of each.
		void ProcessEachBit(size_t bitCount, void * context,
				ProcessEventCallback callback) const;

		// Gets whether this packet is equal to another packet if all
		// `0` bits are trimmed off the right of both.
		bool Equals(const Packet& b) const;
		bool operator==(const Packet& other) const;

		// Retrieves a line reflecting the state of the current buffer
		// as-is.
		std::string GetUntrimmedLine() const;
		void GetUntrimmedLine(std::string& line) const;

		// Retrieves a line reflecting the state of the current buffer:
		//
		// If IsEmpty(), the result is an empty string ("").
		//
		// If IsZeroed() and not IsEmpty(), the result is a single armored
		// zero ("@").
		//
		// Otherwise, the result is the state of the current buffer with all
		// trailing zeroes removed.
		std::string GetLine() const;
		void GetLine(std::string& line) const;

		// Replace the buffer with the trimmed form returned by GetLine().
		void Trim();

		// true if the buffer contains no data.
		bool IsEmpty() const;

		// true if the buffer contains only (armored) zeroed sextets.
		bool IsZeroed() const;

	private:
		class Impl;
		Impl * _impl;
	};

	Packet operator^(const Packet& a, const Packet& b);
}

#endif

/*
 * Copyright © 2016-2017 Peter S. May
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
