/*	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
	CAAudioHardwareSystem.cpp

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "CAAudioHardwareSystem.h"

//	PublicUtility Includes
#include "CAAutoDisposer.h"
#include "CADebugMacros.h"
#include "CAException.h"

//=============================================================================
//	CAAudioHardwareSystem
//=============================================================================

UInt32	CAAudioHardwareSystem::GetNumberDevices()
{
	UInt32 theAnswer = GetPropertyDataSize(kAudioHardwarePropertyDevices);
	theAnswer /= sizeof(AudioDeviceID);
	return theAnswer;
}

AudioDeviceID	CAAudioHardwareSystem::GetDeviceAtIndex(UInt32 inIndex)
{
	AudioDeviceID theAnswer = 0;
	UInt32 theNumberDevices = GetNumberDevices();
	if((theNumberDevices > 0) && (inIndex < theNumberDevices))
	{
		CAAutoArrayDelete<AudioDeviceID> theDeviceList(theNumberDevices);
		UInt32 theSize = theNumberDevices * sizeof(AudioDeviceID);
		GetPropertyData(kAudioHardwarePropertyDevices, theSize, theDeviceList);
		theAnswer = theDeviceList[inIndex];
	}
	return theAnswer;
}

UInt32	CAAudioHardwareSystem::GetIndexForDevice(const AudioDeviceID inDevice)
{
	UInt32 theAnswer = 0xFFFFFFFF;
	UInt32 theNumberDevices = GetNumberDevices();
	if(theNumberDevices > 0)
	{
		CAAutoArrayDelete<AudioDeviceID> theDeviceList(theNumberDevices);
		UInt32 theSize = theNumberDevices * sizeof(AudioDeviceID);
		GetPropertyData(kAudioHardwarePropertyDevices, theSize, theDeviceList);
		for(UInt32 theIndex = 0; theIndex < theNumberDevices; ++theIndex)
		{
			if(inDevice == theDeviceList[theIndex])
			{
				theAnswer = theIndex;
				break;
			}
		}
	}
	return theAnswer;
}

AudioDeviceID	CAAudioHardwareSystem::GetDeviceForUID(CFStringRef inUID)
{
	AudioDeviceID theAnswer = 0;
	AudioValueTranslation theValue = { &inUID, sizeof(CFStringRef), &theAnswer, sizeof(AudioDeviceID) };
	UInt32 theSize = sizeof(AudioValueTranslation);
	GetPropertyData(kAudioHardwarePropertyDeviceForUID, theSize, &theValue);
	return theAnswer;
}

AudioDeviceID	CAAudioHardwareSystem::GetDefaultDevice(bool inIsInput, bool inIsSystem)
{
	AudioDeviceID theAnswer = 0;
	AudioHardwarePropertyID thePropertyID = 0;
	if(inIsInput)
	{
		thePropertyID = kAudioHardwarePropertyDefaultInputDevice;
	}
	else if(inIsSystem)
	{
		thePropertyID = kAudioHardwarePropertyDefaultSystemOutputDevice;
	}
	else
	{
		thePropertyID = kAudioHardwarePropertyDefaultOutputDevice;
	}
	UInt32 theSize = sizeof(AudioDeviceID);
	GetPropertyData(thePropertyID, theSize, &theAnswer);
	return theAnswer;
}

void	CAAudioHardwareSystem::SetDefaultDevice(bool inIsInput, bool inIsSystem, AudioDeviceID inDevice)
{
	AudioHardwarePropertyID thePropertyID = 0;
	if(inIsInput)
	{
		thePropertyID = kAudioHardwarePropertyDefaultInputDevice;
	}
	else if(inIsSystem)
	{
		thePropertyID = kAudioHardwarePropertyDefaultSystemOutputDevice;
	}
	else
	{
		thePropertyID = kAudioHardwarePropertyDefaultOutputDevice;
	}
	UInt32 theSize = sizeof(AudioDeviceID);
	SetPropertyData(thePropertyID, theSize, &inDevice);
}

bool	CAAudioHardwareSystem::HasBootChimeVolumeControl()
{
	return HasProperty(kAudioHardwarePropertyBootChimeVolumeDecibels);
}

Float32	CAAudioHardwareSystem::GetBootChimeVolumeControlValueAsScalar()
{
	Float32 theAnswer = 0;
	UInt32 theSize = sizeof(Float32);
	GetPropertyData(kAudioHardwarePropertyBootChimeVolumeScalar, theSize, &theAnswer);
	return theAnswer;
}

Float32	CAAudioHardwareSystem::GetBootChimeVolumeControlValueAsDB()
{
	Float32 theAnswer = 0;
	UInt32 theSize = sizeof(Float32);
	GetPropertyData(kAudioHardwarePropertyBootChimeVolumeDecibels, theSize, &theAnswer);
	return theAnswer;
}

void	CAAudioHardwareSystem::GetBootChimeVolumeControlDBRange(Float32& outMinimum, Float32& outMaximum)
{
	AudioValueRange theRange = { 0, 0 };
	UInt32 theSize = sizeof(AudioValueRange);
	GetPropertyData(kAudioHardwarePropertyBootChimeVolumeRangeDecibels, theSize, &theRange);
	outMinimum = theRange.mMinimum;
	outMaximum = theRange.mMaximum;
}

void	CAAudioHardwareSystem::SetBootChimeVolumeControlValueAsScalar(Float32 inScalarValue)
{
	UInt32 theSize = sizeof(Float32);
	SetPropertyData(kAudioHardwarePropertyBootChimeVolumeScalar, theSize, &inScalarValue);
}

void	CAAudioHardwareSystem::SetBootChimeVolumeControlValueAsDB(Float32 inDBValue)
{
	UInt32 theSize = sizeof(Float32);
	SetPropertyData(kAudioHardwarePropertyBootChimeVolumeDecibels, theSize, &inDBValue);
}

Float32	CAAudioHardwareSystem::ConvertBootChimeVolumeControlScalarValueToDB(Float32 inScalarValue)
{
	UInt32 theSize = sizeof(Float32);
	GetPropertyData(kAudioHardwarePropertyBootChimeVolumeScalarToDecibels, theSize, &inScalarValue);
	return inScalarValue;
}

Float32	CAAudioHardwareSystem::ConvertBootChimeVolumeControlDBValueToScalar(Float32 inDBValue)
{
	UInt32 theSize = sizeof(Float32);
	GetPropertyData(kAudioHardwarePropertyBootChimeVolumeDecibelsToScalar, theSize, &inDBValue);
	return inDBValue;
}

bool	CAAudioHardwareSystem::AllowsIdleSleepDuringIO()
{
	UInt32 theValue = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(kAudioHardwarePropertySleepingIsAllowed, theSize, &theValue);
	return theValue != 0;
}

void	CAAudioHardwareSystem::SetAllowsIdleSleepDuringIO(bool inAllowIdleSleep)
{
	UInt32 theValue = inAllowIdleSleep ? 1 : 0;
	SetPropertyData(kAudioHardwarePropertySleepingIsAllowed, sizeof(UInt32), &theValue);
}

bool	CAAudioHardwareSystem::HasProperty(AudioHardwarePropertyID inPropertyID)
{
	OSStatus theError = AudioHardwareGetPropertyInfo(inPropertyID, NULL, NULL);
	return theError == 0;
}

bool	CAAudioHardwareSystem::PropertyIsSettable(AudioHardwarePropertyID inPropertyID)
{
	Boolean isWritable = false;
	OSStatus theError = AudioHardwareGetPropertyInfo(inPropertyID, NULL, &isWritable);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareSystem::PropertyIsSettable: got an error getting info about a property");
	return isWritable != 0;
}

UInt32	CAAudioHardwareSystem::GetPropertyDataSize(AudioHardwarePropertyID inPropertyID)
{
	UInt32 theSize = 0;
	OSStatus theError = AudioHardwareGetPropertyInfo(inPropertyID, &theSize, NULL);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareSystem::GetPropertyDataSize: got an error getting info about a property");
	return theSize;
}

void	CAAudioHardwareSystem::GetPropertyData(AudioHardwarePropertyID inPropertyID, UInt32& ioDataSize, void* outData)
{
	OSStatus theError = AudioHardwareGetProperty(inPropertyID, &ioDataSize, outData);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareSystem::GetPropertyData: got an error getting the value of a property");
}

void	CAAudioHardwareSystem::SetPropertyData(AudioHardwarePropertyID inPropertyID, UInt32 inDataSize, const void* inData)
{
	OSStatus theError = AudioHardwareSetProperty(inPropertyID, inDataSize, const_cast<void*>(inData));
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareSystem::SetPropertyData: got an error setting the value of a property");
}

void	CAAudioHardwareSystem::AddPropertyListener(AudioHardwarePropertyID inPropertyID, AudioHardwarePropertyListenerProc inListenerProc, void* inClientData)
{
	OSStatus theError = AudioHardwareAddPropertyListener(inPropertyID, inListenerProc, inClientData);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareSystem::AddPropertyListener: got an error adding a property listener");
}

void	CAAudioHardwareSystem::RemovePropertyListener(AudioHardwarePropertyID inPropertyID, AudioHardwarePropertyListenerProc inListenerProc)
{
	OSStatus theError = AudioHardwareRemovePropertyListener(inPropertyID, inListenerProc);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareSystem::RemovePropertyListener: got an error removing a property listener");
}
