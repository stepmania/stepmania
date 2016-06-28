#include "global.h"
#include "LightsDriver_SextetStream.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageUtil.h"

#include <cstring>
#include <string>

#include "Sextets/IO/PacketWriter.h"
#include "Sextets/IO/NoopPacketWriter.h"
#include "Sextets/IO/RageFilePacketWriter.h"
#include "Sextets/Data.h"
#include "Sextets/Packet.h"

using namespace std;
using namespace Sextets;
using namespace Sextets::IO;





// Implementation base class

class LightsDriver_SextetStream::Impl
{

private:
	Packet previousPacket;
	PacketWriter * writer;

public:
	Impl(PacketWriter * writer)
	{
		if(writer == NULL) {
			writer = new NoopPacketWriter();
		}
		this->writer = writer;
	}

	virtual ~Impl()
	{
		if(writer != NULL) {
			delete writer;
			writer = NULL;
		}
	}

	void Set(const LightsState * ls)
	{
		// Skip writing if the writer is not available.
		if(writer->IsReady()) {
			Packet packet;
			packet.SetToLightsState(ls);

			// Only write if the message has changed since the last write.
			if(!packet.Equals(previousPacket)) {
				writer->WritePacket(packet);
				LOG->Trace("Packet: %s", packet.GetLine().c_str());

				// Remember last message
				previousPacket.Copy(packet);
			}
		}
	}
};



// LightsDriver_SextetStream interface
// (Wrapper for Impl)

LightsDriver_SextetStream::LightsDriver_SextetStream()
{
	_impl = NULL;
}

LightsDriver_SextetStream::~LightsDriver_SextetStream()
{
	if(_impl != NULL) {
		delete _impl;
	}
}

void LightsDriver_SextetStream::Set(const LightsState *ls)
{
	_impl->Set(ls);
}


// LightsDriver_SextetStreamToFile implementation

REGISTER_LIGHTS_DRIVER_CLASS(SextetStreamToFile);

#if defined(_WINDOWS)
	#define DEFAULT_OUTPUT_FILENAME "\\\\.\\pipe\\StepMania-Lights-SextetStream"
#else
	#define DEFAULT_OUTPUT_FILENAME "Data/StepMania-Lights-SextetStream.out"
#endif
static Preference<RString> g_sSextetStreamOutputFilename("SextetStreamOutputFilename", DEFAULT_OUTPUT_FILENAME);

LightsDriver_SextetStreamToFile::LightsDriver_SextetStreamToFile()
{
	LOG->Info("Creating LightsDriver_SextetStreamToFile");
	LOG->Flush();

	PacketWriter * writer =
		RageFilePacketWriter::Create(g_sSextetStreamOutputFilename);

	if(writer == NULL) {
		LOG->Warn("Create of packet writer for LightsDriver_SextetStreamToFile failed.");
	} else {
		LOG->Info("Create of packet writer for LightsDriver_SextetStreamToFile OK.");
	}

	// Impl() accounts for the case where writer is NULL.
	_impl = new Impl(writer);
}

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
