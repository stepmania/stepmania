/*	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
			copyrights in this original Apple software (the "Apple Software"), to use,
			reproduce, modify and redistribute the Apple Software, with or without
			modifications, in source and/or binary forms; provided that if you redistribute
			the Apple Software in its entirety and without modifications, you must retain
			this notice and the following text and disclaimers in all such redistributions of
			the Apple Software.  Neither the name, trademarks, service marks or logos of
			Apple Computer, Inc. may be used to endorse or promote products derived from the
			Apple Software without specific prior written permission from Apple.  Except as
			expressly stated in this notice, no other rights or licenses, express or implied,
			are granted by Apple herein, including but not limited to any patent rights that
			may be infringed by your derivative works or by other works in which the Apple
			Software may be incorporated.

			The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
			WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
			WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
			PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
			COMBINATION WITH YOUR PRODUCTS.

			IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
			CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
			GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
			ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
			OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
			(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
			ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*=============================================================================
	CAAudioHardwareSystem.h

=============================================================================*/
#if !defined(__CAAudioHardwareSystem_h__)
#define __CAAudioHardwareSystem_h__

//=============================================================================
//	Includes
//=============================================================================

//	System Includes
#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/CoreAudio.h>

//=============================================================================
//	CAAudioHardwareSystem
//=============================================================================

class CAAudioHardwareSystem
{

//	Operations
public:
	static UInt32			GetNumberDevices();
	static AudioDeviceID	GetDeviceAtIndex(UInt32 inIndex);
	static UInt32			GetIndexForDevice(const AudioDeviceID inDevice);
	static AudioDeviceID	GetDeviceForUID(CFStringRef inUID);
	
	static AudioDeviceID	GetDefaultDevice(bool inIsInput, bool inIsSystem);
	static void				SetDefaultDevice(bool inIsInput, bool inIsSystem, AudioDeviceID inDevice);

	static bool				HasBootChimeVolumeControl();
	static Float32			GetBootChimeVolumeControlValueAsScalar();
	static Float32			GetBootChimeVolumeControlValueAsDB();
	static void				GetBootChimeVolumeControlDBRange(Float32& outMinimum, Float32& outMaximum);
	static void				SetBootChimeVolumeControlValueAsScalar(Float32 inScalarValue);
	static void				SetBootChimeVolumeControlValueAsDB(Float32 inDBValue);
	static Float32			ConvertBootChimeVolumeControlScalarValueToDB(Float32 inScalarValue);
	static Float32			ConvertBootChimeVolumeControlDBValueToScalar(Float32 inDBValue);
	
	static bool				AllowsIdleSleepDuringIO();
	static void				SetAllowsIdleSleepDuringIO(bool inAllowIdleSleep);
	
//	Property Operations
public:
	static bool				HasProperty(AudioHardwarePropertyID inPropertyID);
	static bool				PropertyIsSettable(AudioHardwarePropertyID inPropertyID);
	
	static UInt32			GetPropertyDataSize(AudioHardwarePropertyID inPropertyID);
	static void				GetPropertyData(AudioHardwarePropertyID inPropertyID, UInt32& ioDataSize, void* outData);
	static void				SetPropertyData(AudioHardwarePropertyID inPropertyID, UInt32 inDataSize, const void* inData);
	
	static void				AddPropertyListener(AudioHardwarePropertyID inPropertyID, AudioHardwarePropertyListenerProc inListenerProc, void* inClientData);
	static void				RemovePropertyListener(AudioHardwarePropertyID inPropertyID, AudioHardwarePropertyListenerProc inListenerProc);
	
};

#endif
