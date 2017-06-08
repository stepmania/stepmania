
#include "Sextets/IO/PacketReaderEventGenerator.h"
#include "RageThreads.h"
#include "RageLog.h"

namespace
{
	using namespace Sextets;
	using namespace Sextets::IO;

	class PacketReaderEventGeneratorImpl : public PacketReaderEventGenerator
	{
		private:
			PacketReader * packetReader;
			void * context;
			PacketReaderEventGenerator::PacketReaderEventCallback * onReadPacket;
			bool continueThread;
			volatile bool threadStarted;
			volatile bool threadEnded;
			RageThread thread;

			inline void CallOnReadPacket(const Packet& packet)
			{
				onReadPacket(context, packet);
			}

			inline void CreateThread()
			{
				continueThread = true;
				thread.SetName("Sextets PacketReaderEventGenerator thread");
				thread.Create(StartThread, this);
			}

			static int StartThread(void * p)
			{
				((PacketReaderEventGeneratorImpl *) p)->RunThread();
				return 0;
			}

			void Invalidate()
			{
				LOG->Trace("Disposing Sextets PacketReaderEventGenerator");

				if(packetReader != NULL) {
					LOG->Trace("Deleting packet reader");
					delete packetReader;
					packetReader = NULL;
				}

				context = NULL;

				onReadPacket = NULL;

			}

			void RunThread()
			{
				threadStarted = true;

				LOG->Info("Sextets PacketReaderEventGenerator thread started");
				
				while(continueThread) {
					Packet packet;

					LOG->Trace("Reading packet");

					if(packetReader->ReadPacket(packet)) {
						LOG->Trace("Got packet: '%s'", packet.GetLine().c_str());
						if(packet.IsEmpty()) {
							LOG->Trace("Packet was blank; not calling callback");
							LOG->Trace("Read packet length: %u", (unsigned) packet.SextetCount());
						}
						else {
							LOG->Trace("Calling callback");
							CallOnReadPacket(packet);
						}
					}
					else {
						// Error or EOF
						LOG->Info("Sextets PacketReader input ended");
						continueThread = false;
					}
				}

				LOG->Info("Sextets PacketReaderEventGenerator thread ending");
				Invalidate();

				threadEnded = true;
			}


		public:
			PacketReaderEventGeneratorImpl(
				PacketReader * packetReader,
				void * context,
				PacketReaderEventGenerator::PacketReaderEventCallback *	onReadPacket)
			{
				// Checks already performed in Create().
				this->packetReader = packetReader;
				this->context = context;
				this->onReadPacket = onReadPacket;
				continueThread = false;
				threadStarted = false;
				threadEnded = false;
				CreateThread();
			}

			~PacketReaderEventGeneratorImpl()
			{
				if(thread.IsCreated()) {
					continueThread = false;
					LOG->Trace("Event generator waiting for thread to end");
					thread.Wait();
				}
				Invalidate();
			}

			bool HasStarted()
			{
				return threadStarted;
			}

			bool HasEnded()
			{
				return threadEnded;
			}

			void RequestStop()
			{
				continueThread = false;
			}
	};
}

namespace Sextets
{
	namespace IO
	{
		PacketReaderEventGenerator * PacketReaderEventGenerator::Create(
				PacketReader * packetReader,
				void * context,
				PacketReaderEventGenerator::PacketReaderEventCallback * onReadPacket)
		{

			if(packetReader == NULL) {
				LOG->Warn("Cannot create event generator: packetReader cannot be NULL.");
				return NULL;
			}
			else if(onReadPacket == NULL) {
				LOG->Warn("Cannot create event generator: onReadPacket cannot be NULL. Destroying packet reader.");
				delete packetReader;
				return NULL;
			}
			else {
				return new PacketReaderEventGeneratorImpl(packetReader, context, onReadPacket);
			}
		}
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
