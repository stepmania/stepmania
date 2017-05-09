#ifndef Sextets_Data_h
#define Sextets_Data_h

#include "global.h"

// Needed for GetLightsStateAsPacket
struct LightsState;

namespace Sextets
{
	namespace Data
	{
		// Retrieves the first span of valid sextet characters
		// (`IsValidSextetByte()`) within the string. All characters before
		// the first valid character, and all characters starting with the
		// first invalid character after the first valid character, are
		// discarded.
		std::string CleanPacketCopy(const std::string& str);

		// Performs XOR on a pair of packets in std::string form, re-armoring
		// the result.
		// If either packet is shorter than the other, the shorter packet is
		// extended.
		std::string XorPacketsCopy(const std::string& a, const std::string& b);

		// Examines the bits that have changed in a state and calls a
		// callback for each change.
		void ProcessPacketChanges(const std::string& statePacket, const std::string& changedPacket, size_t numberOfStateBits, void * context, void updateButton(void * context, size_t index, bool value));

		// Compares two strings and determines whether their contents would
		// be equal as sextet packets. The top two bits of each character
		// are discarded before comparing, but no excess characters are
		// trimmed. If one string is longer than the other, they are
		// compared as if both had infinite trailing zeros.
		bool StdStringSextetsEqual(const std::string& a, const std::string& b);

		// Retrieves a sextet packet containing the information from a
		// LightsState.
		std::string GetLightsStateAsPacket(const LightsState* ls);
	}
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
