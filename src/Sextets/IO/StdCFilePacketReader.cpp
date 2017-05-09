
#include "Sextets/IO/StdCFilePacketReader.h"
#include "RageLog.h"
#include <cerrno>
#include <cstring>

namespace
{
	using namespace Sextets;
	using namespace Sextets::IO;

	class Impl: public StdCFilePacketReader
	{
		private:
			// The buffer size isn't critical; the std::string will simply be
			// extended until the packet is done.
			static const size_t BUFFER_SIZE = 64;
			char buffer[BUFFER_SIZE];
		protected:
			std::FILE * file;

		public:
			Impl(std::FILE * file)
			{
				LOG->Info("Starting Sextets packet reader from open std::FILE");
				this->file = file;
			}

			Impl(const std::string& filename)
			{
				LOG->Info("Starting Sextets packet reader from std::FILE with filename '%s'",
					filename.c_str());
				file = std::fopen(filename.c_str(), "rb");

				if(file == NULL) {
					LOG->Warn("Error opening file '%s' for input (cstdio): %s", filename.c_str(),
						std::strerror(errno));
				}
				else {
					LOG->Info("File opened");
					// Disable buffering on the file
					std::setbuf(file, NULL);
				}
			}

			~Impl()
			{
				if(file != NULL) {
					std::fclose(file);
				}
			}

			virtual bool IsReady()
			{
				return file != NULL;
			}

			virtual bool ReadPacket(Packet& packet)
			{
				bool afterFirst = false;

				if(file != NULL) {
					std::string line;

					while(fgets(buffer, BUFFER_SIZE, file) != NULL) {
						afterFirst = true;
						line += buffer;
						size_t lineLength = line.length();
						if(lineLength > 0) {
							int lastChar = line[lineLength - 1];
							if(lastChar == 0xD || lastChar == 0xA) {
								break;
							}
						}
					}

					packet.SetToLine(line);
				}

				return afterFirst;
			}
	};
}

namespace Sextets
{
	namespace IO
	{
		StdCFilePacketReader* StdCFilePacketReader::Create(std::FILE * file)
		{
			return new Impl(file);
		}

		StdCFilePacketReader* StdCFilePacketReader::Create(const std::string& filename)
		{
			return new Impl(filename);
		}
		
		StdCFilePacketReader::~StdCFilePacketReader() {}
	}
}


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
