
#include "Sextets/IO/SelectFilePacketReader.h"

// TODO: Determine which of these are actually necessary
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

#include "Sextets/PacketBuffer.h"
#include "RageLog.h"

namespace
{
	int OpenFd(const RString& filename)
	{
		int fd = open(filename.c_str(), O_RDONLY | O_NONBLOCK);

		if(fd < 0) {
			return -1;
		}

		return fd;
	}

	// Timeout in ms
	static const size_t timeout = 1000;
}

namespace
{
	using namespace Sextets;
	using namespace Sextets::IO;

	class Impl: public SelectFilePacketReader
	{
	private:
		// This buffer's size isn't critical since it is only used to
		// receive data from C APIs. The resulting data is copied
		// directly into a PacketBuffer; which will simply be extended
		// as needed.
		static const size_t BUFFER_SIZE = 64;
		char buffer[BUFFER_SIZE];
		PacketBuffer * const pb;

		int fd;
		bool seenEof;
		bool seenError;

		inline bool shouldRead()
		{
			if(seenEof) {
				// EOF
				return false;
			}
			else if(seenError) {
				LOG->Warn(
					"Problem with file: Stream experienced error");
				return false;
			}
			else if(fd < 0) {
				LOG->Warn(
					"Problem with file: Stream is invalid or disposed");
				return false;
			}
			return true;
		}

	public:
		Impl(const RString& filename)
			: pb(PacketBuffer::Create())
		{
			LOG->Info(
				"Sextets packet select()/file reader opening stream from file '%s' for input", filename.c_str());

			fd = OpenFd(filename);
			seenEof = false;
			seenError = false;

			if(!HasStream()) {
				LOG->Warn(
					"Sextets packet select()/file reader could not open stream from file '%s' for input", filename.c_str());
				return;
			}
		}

		bool HasStream()
		{
			return fd >= 0;
		}

		void Close()
		{
			pb->DiscardUnfinished();
			if(HasStream()) {
				LOG->Info("Sextets packet select()/file reader closing stream");
				close(fd); // This is close() with a lower-case c
				fd = -1;
			}
		}

		void CloseDueToStreamUnreadable()
		{
			LOG->Warn("SelectFilePacketReader: Closing because stream can no longer be read");
			Close();
		}

		~Impl()
		{
			Close();
			delete pb;
		}

		virtual bool IsReady()
		{
			return shouldRead() || pb->HasPacket();
		}
		
		virtual bool ReadPacket(Packet& packet)
		{
			// Ensure there isn't already a line in the packet buffer.
			// (Even if this object has been closed, there may still be
			// leftover unused data in the packet buffer.)
			if(pb->GetPacket(packet)) {
				// Got a waiting line from the packet buffer
				return true;
			}

			// No complete line from buffer yet. Can we read more?
			if(!shouldRead()) {
				// EOF or unrecoverable error.
				CloseDueToStreamUnreadable();
				return false;
			}

			// Try reading more.

			// select() and read() on an O_NONBLOCK stream combine
			// here as a "slightly blocking" read. If there is no
			// data, it will wait up to timeout ms for data to come
			// in. If any data has been received, it is then read.
			// If no data has been received, the read length is 0.
			// Therefore, this ReadPacket() implementation returns
			// in a finite (and small) amount of time.
			struct timeval timeoutval = { 0, timeout * 1000 };
			fd_set set;
			FD_ZERO(&set);
			FD_SET(fd, &set);
			int event_count = select(fd + 1, &set, NULL, NULL, &timeoutval);

			size_t readLen = 0;

			if(event_count == 0) {
				// Timed out
				readLen = 0;
			}
			else if(event_count < 0) {
				// Select call error
				LOG->Warn("Problem waiting for input on stream: %s", strerror(errno));
				seenError = true;
			}
			else if(FD_ISSET(fd, &set)) {
				// Change seen on our stream
				ssize_t bytes = read(fd, buffer, BUFFER_SIZE);

				if(bytes > 0) {
					readLen = (size_t) bytes;
				}
				else if(bytes == 0) {
					// EOF
					seenEof = true;
				}
				else if((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
					// Temporarily no more to read
					readLen = 0;
				}
				else {
					LOG->Warn("Problem reading input on stream: %s", strerror(errno));
					seenError = true;
				}
			}
			else {
				// Nothing to report
				readLen = 0;
			}

			if(seenEof || seenError) {
				CloseDueToStreamUnreadable();
				return false;
			}

			if(readLen == 0) {
				packet.Clear();
			}
			else {
				// Nonzero read; add to the packet buffer.
				pb->Add(buffer, readLen);

				// Now, see if a packet became available due to this read.
				if(!pb->GetPacket(packet)) {
					// No full line from buffer, but let the caller know
					// that there might still be one later.
					packet.Clear();
				}
				// Otherwise, a new packet is in packet.
			}

			return true;
		}
	};
}


namespace Sextets
{
	namespace IO
	{
		SelectFilePacketReader* SelectFilePacketReader::Create(const RString& filename)
		{
			return new Impl(filename);
		}
		
		SelectFilePacketReader::~SelectFilePacketReader() {}
	}
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
