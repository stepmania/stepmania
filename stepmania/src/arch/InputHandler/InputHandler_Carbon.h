#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <vector>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include "InputHandler.h"
#include "RageThreads.h"

class HIDDevice;

class InputHandler_Carbon : public InputHandler
{
private:
	vector<HIDDevice *> m_vDevices;
	RageThread m_InputThread;
	RageSemaphore m_Sem;
	CFRunLoopRef m_LoopRef;
	CFRunLoopSourceRef m_SourceRef;
	vector<io_iterator_t> m_vIters; // We don't really care about these but they need to stick around
	IONotificationPortRef m_NotifyPort;
	RageMutex m_ChangeLock;
	bool m_bChanged;
	
	static int Run( void *data );
	static void DeviceAdded( void *refCon, io_iterator_t iter );
	static void DeviceChanged( void *refCon, io_service_t service, natural_t messageType, void *arg );
	void StartDevices();
	void AddDevices( int usagePage, int usage, InputDevice &id );

public:
	InputHandler_Carbon();
	~InputHandler_Carbon();

	bool DevicesChanged() { LockMut( m_ChangeLock ); return m_bChanged; }
	void GetDevicesAndDescriptions( vector<InputDeviceInfo>& vDevicesOut );
	RString GetDeviceSpecificInputString( const DeviceInput &di );
	wchar_t DeviceButtonToChar( DeviceButton button, bool bUseCurrentKeyModifiers );
	static void QueueCallBack( void *target, int result, void *refcon, void *sender );
	
};

#endif

/*
 * (c) 2005-2006 Steve Checkoway
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

