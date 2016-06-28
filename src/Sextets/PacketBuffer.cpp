
#include "Sextets/PacketBuffer.h"
#include "RageLog.h"
#include "RageUtil.h"

#include <queue>

typedef RString::value_type RChr;

namespace
{
	static const size_t LINE_MAX_LENGTH = 4096;

#define ASCII_NEWLINE_CHARS "\x0a\x0d"
#define ASCII_PRINTABLE_CHARS \
	"\x20\x21\x22\x23\x24\x25\x26\x27" \
	"\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f" \
	"\x30\x31\x32\x33\x34\x35\x36\x37" \
	"\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f" \
	"\x40\x41\x42\x43\x44\x45\x46\x47" \
	"\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f" \
	"\x50\x51\x52\x53\x54\x55\x56\x57" \
	"\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f" \
	"\x60\x61\x62\x63\x64\x65\x66\x67" \
	"\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f" \
	"\x70\x71\x72\x73\x74\x75\x76\x77" \
	"\x78\x79\x7a\x7b\x7c\x7d\x7e"
#define SIMPLE_CHARS ASCII_PRINTABLE_CHARS ASCII_NEWLINE_CHARS

	using namespace Sextets;

	inline bool EraseTo(RString& str, size_t index, size_t additional = 0)
	{
		if(index == RString::npos) {
			return false;
		}
		str.erase(0, index + additional);
		return true;
	}

	inline bool EraseToFirstOf(RString& str, const RChr * chars, size_t additional = 0)
	{
		size_t i = str.find_first_of(chars);
		return EraseTo(str, i, additional);
	}

	class ActualPacketBuffer : public PacketBuffer
	{
	private:
		std::queue<RString> lines;
		RString buffer;

		// If true, all data through the next newline will be discarded.
		// This is set where continuing a line after a mid-line error would
		// ruin the alignment of the data on the line.
		bool needsResync;

		void RequestResync()
		{
			needsResync = true;
		}

		// If needsResync, attempts to find and erase to the first newline
		// character in the buffer. If successful, needsResync is set to
		// false. Returns whether the buffer is now synced (which is
		// !needsResync).
		bool SyncBuffer()
		{
			if(needsResync) {
				if(EraseToFirstOf(buffer, ASCII_NEWLINE_CHARS)) {
					// Newline found; buffer erased up to newline to sync
					needsResync = false;
					return true;
				} else {
					// No newline found
					buffer.clear();
					return false;
				}
			}
			return true;
		}

		void PushLine(const RString& line)
		{
			if(!line.empty()) {
				lines.push(line);
			}
		}

		// Splits the buffer into lines.
		void BreakBuffer()
		{
			while(!buffer.empty() && SyncBuffer()) {

				// First newline
				size_t newlineIndex = buffer.find_first_of(ASCII_NEWLINE_CHARS);
				// First invalid char
				size_t invalidIndex = buffer.find_first_not_of(SIMPLE_CHARS);

				size_t len = (newlineIndex != RString::npos) ? newlineIndex : buffer.length();

				// An invalid character is seen within the current line's
				// portion of the buffer.
				if(invalidIndex != RString::npos && (invalidIndex < len))
				{
					// Discard line in progress up to next newline
					RequestResync();
					LOG->Warn(
						"Sextets PacketBuffer encountered a line "
						"that contains an invalid character. "
						"The current line will be discarded.");
					continue;
				}

				// The current line's portion of the buffer is too long.
				if(len > LINE_MAX_LENGTH) {
					// Take the beginning, then discard the rest of the line
					PushLine(buffer.substr(0, LINE_MAX_LENGTH));
							RequestResync();
							buffer.clear();
							LOG->Warn(
								"Sextets PacketBuffer encountered a line "
								"that is longer than the allowed maximum "
								"length (%d). The current line has been "
								"truncated.", (unsigned)LINE_MAX_LENGTH);
							continue;
				}

				// The buffer contains a newline and otherwise looks OK.
				if(newlineIndex != RString::npos) {
					PushLine(buffer.substr(0, newlineIndex));
					EraseTo(buffer, newlineIndex, 1);
					continue;
				}

				// The buffer doesn't contain a newline.
				return;
			}
		}

	public:
		ActualPacketBuffer() : needsResync(false)
		{
		}

		~ActualPacketBuffer()
		{
		}

		void Add(const RChr * data, size_t length)
		{
			buffer.append(data, length);
			BreakBuffer();
		}

		void Add(const RString& data)
		{
			buffer.append(data);
			BreakBuffer();
		}

		void Add(const uint8_t * data, size_t length)
		{
			size_t start = buffer.length();
			buffer.resize(start + length);
			for(size_t i = 0; i < length; ++i) {
				buffer[start + i] = (RChr)(data[i]);
			}
			BreakBuffer();
		}

		bool HasPacket()
		{
			return !lines.empty();
		}

		bool GetPacket(Packet& packet)
		{
			if(lines.empty()) {
				return false;
			}

			packet.SetToLine(lines.front());
			lines.pop();

			return true;
		}

		void DiscardUnfinished()
		{
			RequestResync();
		}
	};
}

namespace Sextets
{
	PacketBuffer* PacketBuffer::Create()
	{
		return new ActualPacketBuffer();
	}

	PacketBuffer::~PacketBuffer() {}
}

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
