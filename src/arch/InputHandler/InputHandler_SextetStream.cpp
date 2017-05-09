#include "global.h"
#include "InputHandler_SextetStream.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageThreads.h"
#include "RageUtil.h"

#include "Sextets/IO/PacketReader.h"
#include "Sextets/IO/StdCFilePacketReader.h"
#include "Sextets/IO/SelectFilePacketReader.h"
#include "Sextets/Data.h"

#include <cerrno>
#include <cstdio>
#include <cstring>

using namespace std;
using namespace Sextets;
using namespace Sextets::IO;

// In so many words, ceil(n/6).
#define NUMBER_OF_SEXTETS_FOR_BIT_COUNT(n) (((n) + 5) / 6)

#define FIRST_DEVICE DEVICE_JOY1

#define FIRST_JOY_BUTTON JOY_BUTTON_1
#define LAST_JOY_BUTTON JOY_BUTTON_32
#define COUNT_JOY_BUTTON ((LAST_JOY_BUTTON) - (FIRST_JOY_BUTTON) + 1)

#define FIRST_KEY KEY_OTHER_0
#define LAST_KEY KEY_LAST_OTHER
#define COUNT_KEY ((LAST_KEY) - (FIRST_KEY) + 1)

#define BUTTON_COUNT (COUNT_JOY_BUTTON + COUNT_KEY)

#define DEFAULT_TIMEOUT_MS 1000
#define STATE_BUFFER_SIZE NUMBER_OF_SEXTETS_FOR_BIT_COUNT(BUTTON_COUNT)


namespace
{
	// Gets the DeviceButton that corresponds with the given index.
	//
	// The first COUNT_JOY_BUTTON indices are mapped to the range starting
	// with FIRST_JOY_BUTTON.
	//
	// The next COUNT_KEY indices are mapped to the range starting with
	// FIRST_KEY.
	//
	// Any indices after these are mapped to DeviceButton_Invalid.
	inline DeviceButton ButtonAtIndex(size_t index)
	{
		if(index < COUNT_JOY_BUTTON) {
			return enum_add2(FIRST_JOY_BUTTON, index);
		} else if(index < COUNT_JOY_BUTTON + COUNT_KEY) {
			return enum_add2(FIRST_KEY, index - COUNT_JOY_BUTTON);
		} else {
			return DeviceButton_Invalid;
		}
	}
}

class InputHandler_SextetStream::Impl
{
private:
	Packet currentStatePacket, nextStatePacket;
	InputHandler_SextetStream * handler;
	PacketReaderEventGenerator * eventGenerator;
	InputDevice id;
	RageMutex statePacketsLock;

	static void TriggerSetButtonState(void * p, size_t index, bool value)
	{
		((Impl*)p)->SetButtonState(index, value);
	}

	static void TriggerOnReadPacket(void * p, const Packet& packet)
	{
		((Impl*)p)->OnReadPacket(packet);
	}

	void SetButtonState(size_t index, bool value)
	{
		DeviceInput di = DeviceInput(id, ButtonAtIndex(index), value ? 1 : 0);
		handler->ButtonPressed(di);
	}

	void OnReadPacket(const Packet& newStatePacket)
	{
		statePacketsLock.Lock();
		nextStatePacket = newStatePacket;
		statePacketsLock.Unlock();
	}

	inline void Update0()
	{
		Packet changesPacket;

		changesPacket.SetToXor(currentStatePacket, nextStatePacket);

		if(changesPacket.IsEmpty()) {
			// No updates needed
			return;
		}

		// Trigger button updates
		nextStatePacket.ProcessEventData(changesPacket, BUTTON_COUNT, this, TriggerSetButtonState);

		// Must be called at end of Update (cargo cult style).
		handler->InputHandler::UpdateTimer();

		currentStatePacket = nextStatePacket;
	}

public:
	Impl(InputHandler_SextetStream * handler, PacketReader * packetReader) :
		statePacketsLock("InputHandler_SextetStream")
	{
		LOG->Info("Number of button states supported by current InputHandler_SextetStream: %u",
				  (unsigned)BUTTON_COUNT);

		this->handler = handler;

		eventGenerator = PacketReaderEventGenerator::Create(packetReader, (void*) this, TriggerOnReadPacket);

		if(eventGenerator == NULL) {
			LOG->Warn("Failed to get PacketReader event generator; this input handler is disabled.");
		}

		id = InputDevice(FIRST_DEVICE);
	}

	virtual ~Impl()
	{
		if(eventGenerator != NULL) {
			delete eventGenerator;
			eventGenerator = NULL;
		}
	}


	void Update()
	{
		statePacketsLock.Lock();
		Update0();
		statePacketsLock.Unlock();
	}

};

// ctor and dtor of InputHandler_SextetStream.
// If _impl is non-NULL at dtor time, it will be deleted.
//
InputHandler_SextetStream::InputHandler_SextetStream()
{
	_impl = NULL;
}

InputHandler_SextetStream::~InputHandler_SextetStream()
{
	if(_impl != NULL) {
		delete _impl;
	}
}

void InputHandler_SextetStream::GetDevicesAndDescriptions(vector<InputDeviceInfo>& vDevicesOut)
{
	vDevicesOut.push_back(InputDeviceInfo(FIRST_DEVICE, "SextetStream"));
}

void InputHandler_SextetStream::Update()
{
	_impl->Update();
}

// SextetStreamFromFile

REGISTER_INPUT_HANDLER_CLASS(SextetStreamFromFile);

#if defined(_WINDOWS)
	#define DEFAULT_INPUT_FILENAME "\\\\.\\pipe\\StepMania-Input-SextetStream"
#else
	#define DEFAULT_INPUT_FILENAME "Data/StepMania-Input-SextetStream.in"
#endif
static Preference<std::string> g_sSextetStreamInputFilename("SextetStreamInputFilename", DEFAULT_INPUT_FILENAME);

InputHandler_SextetStreamFromFile::InputHandler_SextetStreamFromFile()
{
	_impl = new InputHandler_SextetStream::Impl(this, StdCFilePacketReader::Create(g_sSextetStreamInputFilename));
}

InputHandler_SextetStreamFromFile::~InputHandler_SextetStreamFromFile()
{
}


// SextetStreamFromSelectFile

#if !defined(_WINDOWS)

REGISTER_INPUT_HANDLER_CLASS(SextetStreamFromSelectFile);

InputHandler_SextetStreamFromSelectFile::InputHandler_SextetStreamFromSelectFile()
{
	_impl = new InputHandler_SextetStream::Impl(this, SelectFilePacketReader::Create(g_sSextetStreamInputFilename));
}

InputHandler_SextetStreamFromSelectFile::~InputHandler_SextetStreamFromSelectFile()
{
}

#endif

#ifndef WITHOUT_NETWORKING

// SextetStreamFromSocket

#include "ezsockets.h"
#include "Sextets/IO/EzSocketsPacketReader.h"

REGISTER_INPUT_HANDLER_CLASS(SextetStreamFromSocket);

#define DEFAULT_SOCKET_HOST "localhost"
#define DEFAULT_SOCKET_PORT 6761

static Preference<std::string> g_sSextetStreamInputSocketHost("SextetStreamInputSocketHost", DEFAULT_SOCKET_HOST);
static Preference<int> g_iSextetStreamInputSocketPort("SextetStreamInputSocketPort", DEFAULT_SOCKET_PORT);

InputHandler_SextetStreamFromSocket::InputHandler_SextetStreamFromSocket()
{
	std::string host = g_sSextetStreamInputSocketHost;
	unsigned short port = (unsigned short) g_iSextetStreamInputSocketPort;

	_impl = new InputHandler_SextetStream::Impl(this, EzSocketsPacketReader::Create(host, port));
}

InputHandler_SextetStreamFromSocket::~InputHandler_SextetStreamFromSocket()
{
}

#endif // ndef WITHOUT_NETWORKING

/*
 * Copyright © 2014-2017 Peter S. May
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


