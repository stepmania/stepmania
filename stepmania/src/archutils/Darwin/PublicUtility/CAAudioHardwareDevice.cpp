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
	CAAudioHardwareDevice.cpp

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "CAAudioHardwareDevice.h"

//	Internal Includes
#include "CAAudioHardwareStream.h"

//	PublicUtility Includes
#include "CAAutoDisposer.h"
#include "CADebugMacros.h"
#include "CAException.h"

//	System Includes
#include <IOKit/audio/IOAudioTypes.h>
#include <unistd.h>

//	Standard Library Includes
#include <algorithm>

//=============================================================================
//	CAAudioHardwareDevice
//=============================================================================

CAAudioHardwareDevice::CAAudioHardwareDevice(AudioDeviceID inAudioDeviceID)
:
	mAudioDeviceID(inAudioDeviceID)
{
}

CAAudioHardwareDevice::~CAAudioHardwareDevice()
{
}

CFStringRef	CAAudioHardwareDevice::CopyName() const
{
	CFStringRef theAnswer = NULL;
	try
	{
		UInt32 theSize = sizeof(CFStringRef);
		GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyDeviceNameCFString, theSize, &theAnswer);
	}
	catch(...)
	{
		//	Sometimes a device doesn't have a name. Rather than throw an exception here
		//	it is better for the rest of the code if it just return an empty string.
		//	At some point, this should probably go back to throwing an exception.
		theAnswer = CFSTR("");
	}
	return theAnswer;
}

CFStringRef	CAAudioHardwareDevice::CopyManufacturer() const
{
	CFStringRef theAnswer = NULL;
	UInt32 theSize = sizeof(CFStringRef);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyDeviceManufacturerCFString, theSize, &theAnswer);
	return theAnswer;
}

CFStringRef	CAAudioHardwareDevice::CopyUID() const
{
	CFStringRef theAnswer = NULL;
	UInt32 theSize = sizeof(CFStringRef);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyDeviceUID, theSize, &theAnswer);
	return theAnswer;
}

CFStringRef	CAAudioHardwareDevice::CopyConfigurationApplicationBundleID() const
{
	CFStringRef theAnswer = NULL;
	UInt32 theSize = sizeof(CFStringRef);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyConfigurationApplication, theSize, &theAnswer);
	return theAnswer;
}

UInt32	CAAudioHardwareDevice::GetTransportType() const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyTransportType, theSize, &theAnswer);
	return theAnswer;
}

bool	CAAudioHardwareDevice::CanBeDefaultDevice(CAAudioHardwareDeviceSectionID inSection, bool inIsSystem) const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	AudioHardwarePropertyID thePropertyID = inIsSystem ? kAudioDevicePropertyDeviceCanBeDefaultSystemDevice : kAudioDevicePropertyDeviceCanBeDefaultDevice;
	GetPropertyData(0, inSection, thePropertyID, theSize, &theAnswer);
	return theAnswer != 0;
}

bool	CAAudioHardwareDevice::HasDevicePlugInStatus() const
{
	return HasProperty(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyPlugIn);
}

OSStatus	CAAudioHardwareDevice::GetDevicePlugInStatus() const
{
	OSStatus theAnswer = 0;
	UInt32 theSize = sizeof(OSStatus);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyPlugIn, theSize, &theAnswer);
	return theAnswer;
}

bool	CAAudioHardwareDevice::IsAlive() const
{
	bool theAnswer = HasProperty(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyDeviceIsAlive);
	if(theAnswer)
	{
		UInt32 isAlive = 0;
		UInt32 theSize = sizeof(UInt32);
		GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyDeviceIsAlive, theSize, &isAlive);
		theAnswer = isAlive != 0;
	}
	return theAnswer;
}

bool	CAAudioHardwareDevice::IsRunning() const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyDeviceIsRunning, theSize, &theAnswer);
	return theAnswer != 0;
}

void	CAAudioHardwareDevice::SetIsRunning(bool inIsRunning)
{
	UInt32 theValue = inIsRunning ? 1 : 0;
	UInt32 theSize = sizeof(UInt32);
	SetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyDeviceIsRunning, theSize, &theValue);
}

bool	CAAudioHardwareDevice::IsRunningSomewhere() const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyDeviceIsRunningSomewhere, theSize, &theAnswer);
	return theAnswer != 0;
}

pid_t	CAAudioHardwareDevice::GetHogModeOwner() const
{
	pid_t theAnswer = 0;
	UInt32 theSize = sizeof(pid_t);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyHogMode, theSize, &theAnswer);
	return theAnswer;
}

bool	CAAudioHardwareDevice::TakeHogMode()
{
	pid_t thePID = 0;
	UInt32 theSize = sizeof(pid_t);
	SetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyHogMode, theSize, &thePID);
	return thePID == getpid();
}

void	CAAudioHardwareDevice::ReleaseHogMode()
{
	pid_t thePID = -1;
	UInt32 theSize = sizeof(pid_t);
	SetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyHogMode, theSize, &thePID);
}

bool	CAAudioHardwareDevice::SupportsChangingMixability() const
{
	bool theAnswer = HasProperty(0, kAudioDeviceSectionGlobal, kAudioDevicePropertySupportsMixing);
	if(theAnswer)
	{
		theAnswer = PropertyIsSettable(0, kAudioDeviceSectionGlobal, kAudioDevicePropertySupportsMixing);
	}
	return theAnswer;
}

bool	CAAudioHardwareDevice::IsMixable() const
{
	UInt32 theAnswer = 1;
	if(HasProperty(0, kAudioDeviceSectionGlobal, kAudioDevicePropertySupportsMixing))
	{
		UInt32 theSize = sizeof(UInt32);
		GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertySupportsMixing, theSize, &theAnswer);
	}
	return theAnswer != 0;
}

void	CAAudioHardwareDevice::SetIsMixable(bool inIsMixable)
{
	UInt32 theValue = inIsMixable ? 1 : 0;
	UInt32 theSize = sizeof(UInt32);
	SetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertySupportsMixing, theSize, &theValue);
}

bool	CAAudioHardwareDevice::HasIsConnectedStatus(CAAudioHardwareDeviceSectionID inSection) const
{
	return HasProperty(0, inSection, kAudioDevicePropertyJackIsConnected);
}

bool	CAAudioHardwareDevice::GetIsConnectedStatus(CAAudioHardwareDeviceSectionID inSection) const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, inSection, kAudioDevicePropertyJackIsConnected, theSize, &theAnswer);
	return theAnswer != 0;
}

bool	CAAudioHardwareDevice::HasPreferredStereoChannels(CAAudioHardwareDeviceSectionID inSection) const
{
	return HasProperty(0, inSection, kAudioDevicePropertyPreferredChannelsForStereo);
}

void	CAAudioHardwareDevice::GetPreferredStereoChannels(CAAudioHardwareDeviceSectionID inSection, UInt32& outLeft, UInt32& outRight) const
{
	UInt32 theStereoPair[2] = { 0, 0 };
	UInt32 theSize = 2 * sizeof(UInt32);
	GetPropertyData(0, inSection, kAudioDevicePropertyPreferredChannelsForStereo, theSize, theStereoPair);
	outLeft = theStereoPair[0];
	outRight = theStereoPair[1];
}

void	CAAudioHardwareDevice::SetPreferredStereoChannels(CAAudioHardwareDeviceSectionID inSection, UInt32 inLeft, UInt32 inRight)
{
	UInt32 theStereoPair[2] = { inLeft, inRight };
	UInt32 theSize = 2 * sizeof(UInt32);
	SetPropertyData(0, inSection, kAudioDevicePropertyPreferredChannelsForStereo, theSize, theStereoPair);
}

UInt32	CAAudioHardwareDevice::GetLatency(CAAudioHardwareDeviceSectionID inSection) const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, inSection, kAudioDevicePropertyLatency, theSize, &theAnswer);
	return theAnswer;
}

UInt32	CAAudioHardwareDevice::GetSafetyOffset(CAAudioHardwareDeviceSectionID inSection) const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, inSection, kAudioDevicePropertySafetyOffset, theSize, &theAnswer);
	return theAnswer;
}

bool	CAAudioHardwareDevice::HasClockSourceControl() const
{
	return HasProperty(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyClockSource);
}

bool	CAAudioHardwareDevice::ClockSourceControlIsSettable() const
{
	return PropertyIsSettable(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyClockSource);
}

UInt32	CAAudioHardwareDevice::GetCurrentClockSourceID() const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyClockSource, theSize, &theAnswer);
	return theAnswer;
}

void	CAAudioHardwareDevice::SetCurrentClockSourceByID(UInt32 inID)
{
	UInt32 theSize = sizeof(UInt32);
	SetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyClockSource, theSize, &inID);
}

UInt32	CAAudioHardwareDevice::GetNumberAvailableClockSources() const
{
	UInt32 theAnswer = 0;
	if(HasClockSourceControl())
	{
		UInt32 theSize = GetPropertyDataSize(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyClockSources);
		theAnswer = theSize / sizeof(UInt32);
	}
	return theAnswer;
}

UInt32	CAAudioHardwareDevice::GetAvailableClockSourceByIndex(UInt32 inIndex) const
{
	AudioStreamID theAnswer = 0;
	UInt32 theNumberSources = GetNumberAvailableClockSources();
	if((theNumberSources > 0) && (inIndex < theNumberSources))
	{
		CAAutoArrayDelete<UInt32> theSourceList(theNumberSources);
		GetAvailableClockSources(theNumberSources, theSourceList);
		theAnswer = theSourceList[inIndex];
	}
	return theAnswer;
}

void	CAAudioHardwareDevice::GetAvailableClockSources(UInt32& ioNumberSources, UInt32* outSources) const
{
	UInt32 theNumberSources = std::min(GetNumberAvailableClockSources(), ioNumberSources);
	UInt32 theSize = theNumberSources * sizeof(UInt32);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyClockSources, theSize, outSources);
	ioNumberSources = theSize / sizeof(UInt32);
	UInt32* theFirstItem = &(outSources[0]);
	UInt32* theLastItem = theFirstItem + ioNumberSources;
	std::sort(theFirstItem, theLastItem);
}

CFStringRef	CAAudioHardwareDevice::CopyClockSourceNameForID(UInt32 inID) const
{
	CFStringRef theAnswer = NULL;
	AudioValueTranslation theTranslation = { &inID, sizeof(UInt32), &theAnswer, sizeof(CFStringRef) };
	UInt32 theSize = sizeof(AudioValueTranslation);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyClockSourceNameForIDCFString, theSize, &theTranslation);
	return theAnswer;
}

bool	CAAudioHardwareDevice::HasDataSourceControl(CAAudioHardwareDeviceSectionID inSection) const
{
	return HasProperty(0, inSection, kAudioDevicePropertyDataSource);
}

bool	CAAudioHardwareDevice::DataSourceControlIsSettable(CAAudioHardwareDeviceSectionID inSection) const
{
	return PropertyIsSettable(0, inSection, kAudioDevicePropertyDataSource);
}

UInt32	CAAudioHardwareDevice::GetCurrentDataSourceID(CAAudioHardwareDeviceSectionID inSection) const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, inSection, kAudioDevicePropertyDataSource, theSize, &theAnswer);
	return theAnswer;
}

void	CAAudioHardwareDevice::SetCurrentDataSourceByID(CAAudioHardwareDeviceSectionID inSection, UInt32 inID)
{
	UInt32 theSize = sizeof(UInt32);
	SetPropertyData(0, inSection, kAudioDevicePropertyDataSource, theSize, &inID);
}

UInt32	CAAudioHardwareDevice::GetNumberAvailableDataSources(CAAudioHardwareDeviceSectionID inSection) const
{
	UInt32 theAnswer = 0;
	if(HasDataSourceControl(inSection))
	{
		UInt32 theSize = GetPropertyDataSize(0, inSection, kAudioDevicePropertyDataSources);
		theAnswer = theSize / sizeof(UInt32);
	}
	return theAnswer;
}

UInt32	CAAudioHardwareDevice::GetAvailableDataSourceByIndex(CAAudioHardwareDeviceSectionID inSection, UInt32 inIndex) const
{
	AudioStreamID theAnswer = 0;
	UInt32 theNumberSources = GetNumberAvailableDataSources(inSection);
	if((theNumberSources > 0) && (inIndex < theNumberSources))
	{
		CAAutoArrayDelete<UInt32> theSourceList(theNumberSources);
		GetAvailableDataSources(inSection, theNumberSources, theSourceList);
		theAnswer = theSourceList[inIndex];
	}
	return theAnswer;
}

void	CAAudioHardwareDevice::GetAvailableDataSources(CAAudioHardwareDeviceSectionID inSection, UInt32& ioNumberSources, UInt32* outSources) const
{
	UInt32 theNumberSources = std::min(GetNumberAvailableDataSources(inSection), ioNumberSources);
	UInt32 theSize = theNumberSources * sizeof(UInt32);
	GetPropertyData(0, inSection, kAudioDevicePropertyDataSources, theSize, outSources);
	ioNumberSources = theSize / sizeof(UInt32);
	UInt32* theFirstItem = &(outSources[0]);
	UInt32* theLastItem = theFirstItem + ioNumberSources;
	std::sort(theFirstItem, theLastItem);
}

CFStringRef	CAAudioHardwareDevice::CopyDataSourceNameForID(CAAudioHardwareDeviceSectionID inSection, UInt32 inID) const
{
	CFStringRef theAnswer = NULL;
	AudioValueTranslation theTranslation = { &inID, sizeof(UInt32), &theAnswer, sizeof(CFStringRef) };
	UInt32 theSize = sizeof(AudioValueTranslation);
	GetPropertyData(0, inSection, kAudioDevicePropertyDataSourceNameForIDCFString, theSize, &theTranslation);
	return theAnswer;
}

Float64	CAAudioHardwareDevice::GetActualSampleRate() const
{
	Float64 theAnswer = 0;
	UInt32 theSize = sizeof(Float64);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyActualSampleRate, theSize, &theAnswer);
	return theAnswer;
}

Float64	CAAudioHardwareDevice::GetNominalSampleRate() const
{
	Float64 theAnswer = 0;
	UInt32 theSize = sizeof(Float64);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyNominalSampleRate, theSize, &theAnswer);
	return theAnswer;
}

void	CAAudioHardwareDevice::SetNominalSampleRate(Float64 inSampleRate)
{
	UInt32 theSize = sizeof(Float64);
	SetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyNominalSampleRate, theSize, &inSampleRate);
}

bool	CAAudioHardwareDevice::IsValidNominalSampleRate(Float64 inSampleRate) const
{
	bool theAnswer = false;
	UInt32 theNumberRanges = GetNumberNominalSampleRateRanges();
	CAAutoArrayDelete<AudioValueRange> theRanges(theNumberRanges);
	GetNominalSampleRateRanges(theNumberRanges, theRanges);
	for(UInt32 theIndex = 0; !theAnswer && (theIndex < theNumberRanges); ++theIndex)
	{
		theAnswer = (inSampleRate >= theRanges[theIndex].mMinimum) && (inSampleRate <= theRanges[theIndex].mMinimum);
	}
	return theAnswer;
}

UInt32	CAAudioHardwareDevice::GetNumberNominalSampleRateRanges() const
{
	UInt32 theSize = GetPropertyDataSize(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyAvailableNominalSampleRates);
	return theSize / sizeof(AudioValueRange);
}

void	CAAudioHardwareDevice::GetNominalSampleRateRanges(UInt32& ioNumberRanges, AudioValueRange* outRanges) const
{
	UInt32 theSize = ioNumberRanges * sizeof(AudioValueRange);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyAvailableNominalSampleRates, theSize, outRanges);
	ioNumberRanges = theSize / sizeof(AudioValueRange);
}

void	CAAudioHardwareDevice::GetNominalSampleRateRangeByIndex(UInt32 inIndex, Float64& outMinimum, Float64& outMaximum) const
{
	UInt32 theNumberRanges = GetNumberNominalSampleRateRanges();
	ThrowIf(inIndex >= theNumberRanges, CAException(kAudioHardwareIllegalOperationError), "CAAudioHardwareDevice::GetNominalSampleRateRangeByIndex: index out of range");
	CAAutoArrayDelete<AudioValueRange> theRanges(theNumberRanges);
	GetNominalSampleRateRanges(theNumberRanges, theRanges);
	outMinimum = theRanges[inIndex].mMinimum;
	outMaximum = theRanges[inIndex].mMaximum;
}

UInt32	CAAudioHardwareDevice::GetIOBufferSize() const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyBufferFrameSize, theSize, &theAnswer);
	return theAnswer;
}

void	CAAudioHardwareDevice::SetIOBufferSize(UInt32 inBufferSize)
{
	UInt32 theSize = sizeof(UInt32);
	SetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyBufferFrameSize, theSize, &inBufferSize);
}

bool	CAAudioHardwareDevice::UsesVariableIOBufferSizes() const
{
	return HasProperty(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyUsesVariableBufferFrameSizes);
}

UInt32	CAAudioHardwareDevice::GetMaximumVariableIOBufferSize() const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyUsesVariableBufferFrameSizes, theSize, &theAnswer);
	return theAnswer;
}

void	CAAudioHardwareDevice::GetIOBufferSizeRange(UInt32& outMinimum, UInt32& outMaximum) const
{
	AudioValueRange theAnswer = { 0, 0 };
	UInt32 theSize = sizeof(AudioValueRange);
	GetPropertyData(0, kAudioDeviceSectionGlobal, kAudioDevicePropertyBufferFrameSizeRange, theSize, &theAnswer);
	outMinimum = static_cast<UInt32>(theAnswer.mMinimum);
	outMaximum = static_cast<UInt32>(theAnswer.mMaximum);
}

void	CAAudioHardwareDevice::AddIOProc(AudioDeviceIOProc inIOProc, void* inClientData)
{
	OSStatus theError = AudioDeviceAddIOProc(mAudioDeviceID, inIOProc, inClientData);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::AddIOProc: got an error adding an IOProc");
}

void	CAAudioHardwareDevice::RemoveIOProc(AudioDeviceIOProc inIOProc)
{
	OSStatus theError = AudioDeviceRemoveIOProc(mAudioDeviceID, inIOProc);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::RemoveIOProc: got an error removing an IOProc");
}

void	CAAudioHardwareDevice::StartIOProc(AudioDeviceIOProc inIOProc)
{
	OSStatus theError = AudioDeviceStart(mAudioDeviceID, inIOProc);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::StartIOProc: got an error starting an IOProc");
}

void	CAAudioHardwareDevice::StartIOProcAtTime(AudioDeviceIOProc inIOProc, AudioTimeStamp& ioStartTime, bool inIsInput, bool inIgnoreHardware)
{
	UInt32 theFlags = 0;
	if(inIsInput)
	{
		theFlags |= kAudioDeviceStartTimeIsInputFlag;
	}
	if(inIgnoreHardware)
	{
		theFlags |= kAudioDeviceStartTimeDontConsultDeviceFlag;
	}
	
	OSStatus theError = AudioDeviceStartAtTime(mAudioDeviceID, inIOProc, &ioStartTime, theFlags);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::StartIOProcAtTime: got an error starting an IOProc");
}

void	CAAudioHardwareDevice::StopIOProc(AudioDeviceIOProc inIOProc)
{
	OSStatus theError = AudioDeviceStop(mAudioDeviceID, inIOProc);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::StopIOProc: got an error stopping an IOProc");
}

void	CAAudioHardwareDevice::GetIOProcStreamUsage(AudioDeviceIOProc inIOProc, CAAudioHardwareDeviceSectionID inSection, bool* outStreamUsage) const
{
	//	make an AudioHardwareIOProcStreamUsage the right size
	UInt32 theNumberStreams = GetNumberStreams(inSection);
	UInt32 theSize = sizeof(void*) + sizeof(UInt32) + (theNumberStreams * sizeof(UInt32));
	CAAutoFree<AudioHardwareIOProcStreamUsage> theStreamUsage(theSize);
	
	//	set it up
	theStreamUsage->mIOProc = (void*)inIOProc;
	theStreamUsage->mNumberStreams = theNumberStreams;
	
	//	get the property
	GetPropertyData(0, inSection, kAudioDevicePropertyIOProcStreamUsage, theSize, theStreamUsage);
	
	//	fill out the return value
	for(UInt32 theIndex = 0; theIndex < theNumberStreams; ++theIndex)
	{
		outStreamUsage[theIndex] = (theStreamUsage->mStreamIsOn[theIndex] != 0);
	}
}

void	CAAudioHardwareDevice::SetIOProcStreamUsage(AudioDeviceIOProc inIOProc, CAAudioHardwareDeviceSectionID inSection, const bool* inStreamUsage)
{
	//	make an AudioHardwareIOProcStreamUsage the right size
	UInt32 theNumberStreams = GetNumberStreams(inSection);
	UInt32 theSize = sizeof(void*) + sizeof(UInt32) + (theNumberStreams * sizeof(UInt32));
	CAAutoFree<AudioHardwareIOProcStreamUsage> theStreamUsage(theSize);
	
	//	set it up
	theStreamUsage->mIOProc = (void*)inIOProc;
	theStreamUsage->mNumberStreams = theNumberStreams;
	for(UInt32 theIndex = 0; theIndex < theNumberStreams; ++theIndex)
	{
		theStreamUsage->mStreamIsOn[theIndex] = (inStreamUsage[theIndex] ? 1 : 0);
	}
	
	//	set the property
	SetPropertyData(0, inSection, kAudioDevicePropertyIOProcStreamUsage, theSize, theStreamUsage);
}

void	CAAudioHardwareDevice::GetCurrentTime(AudioTimeStamp& outTime)
{
	OSStatus theError = AudioDeviceGetCurrentTime(mAudioDeviceID, &outTime);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::GetCurrentTime: got an error retrieving the current time");
}

void	CAAudioHardwareDevice::TranslateTime(const AudioTimeStamp& inTime, AudioTimeStamp& outTime)
{
	OSStatus theError = AudioDeviceTranslateTime(mAudioDeviceID, &inTime, &outTime);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::TranslateTime: got an error translating time");
}

void	CAAudioHardwareDevice::GetNearestStartTime(AudioTimeStamp& ioTime, bool inIsInput, bool inIgnoreHardware)
{
	UInt32 theFlags = 0;
	if(inIsInput)
	{
		theFlags |= kAudioDeviceStartTimeIsInputFlag;
	}
	if(inIgnoreHardware)
	{
		theFlags |= kAudioDeviceStartTimeDontConsultDeviceFlag;
	}
	
	OSStatus theError = AudioDeviceGetNearestStartTime(mAudioDeviceID, &ioTime, theFlags);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::GetNearestStartTime: got an error getting the nearest start time");
}

UInt32	CAAudioHardwareDevice::GetNumberStreams(CAAudioHardwareDeviceSectionID inSection) const
{
	UInt32 theSize = GetPropertyDataSize(0, inSection, kAudioDevicePropertyStreams);
	return theSize / sizeof(AudioStreamID);
}

void	CAAudioHardwareDevice::GetStreams(CAAudioHardwareDeviceSectionID inSection, UInt32& ioNumberStreams, AudioStreamID* outStreamList) const
{
	ioNumberStreams = std::min(GetNumberStreams(inSection), ioNumberStreams);
	UInt32 theSize = ioNumberStreams * sizeof(AudioStreamID);
	GetPropertyData(0, inSection, kAudioDevicePropertyStreams, theSize, outStreamList);
	ioNumberStreams = theSize / sizeof(AudioStreamID);
	AudioStreamID* theFirstItem = &(outStreamList[0]);
	AudioStreamID* theLastItem = theFirstItem + ioNumberStreams;
	std::sort(theFirstItem, theLastItem);
}

AudioStreamID	CAAudioHardwareDevice::GetStreamByIndex(CAAudioHardwareDeviceSectionID inSection, UInt32 inIndex) const
{
	AudioStreamID theAnswer = 0;
	UInt32 theNumberStreams = GetNumberStreams(inSection);
	if((theNumberStreams > 0) && (inIndex < theNumberStreams))
	{
		CAAutoArrayDelete<AudioStreamID> theStreamList(theNumberStreams);
		GetStreams(inSection, theNumberStreams, theStreamList);
		theAnswer = theStreamList[inIndex];
	}
	return theAnswer;
}

UInt32	CAAudioHardwareDevice::GetTotalNumberChannels(CAAudioHardwareDeviceSectionID inSection) const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = GetPropertyDataSize(0, inSection, kAudioDevicePropertyStreamConfiguration);
	CAAutoFree<AudioBufferList> theBufferList(theSize);
	GetPropertyData(0, inSection, kAudioDevicePropertyStreamConfiguration, theSize, theBufferList);
	for(UInt32 theIndex = 0; theIndex < theBufferList->mNumberBuffers; ++theIndex)
	{
		theAnswer += theBufferList->mBuffers[theIndex].mNumberChannels;
	}
	return theAnswer;
}

void	CAAudioHardwareDevice::GetCurrentIOProcFormats(CAAudioHardwareDeviceSectionID inSection, UInt32& ioNumberStreams, AudioStreamBasicDescription* outFormats) const
{
	ioNumberStreams = std::min(ioNumberStreams, GetNumberStreams(inSection));
	for(UInt32 theIndex = 0; theIndex < ioNumberStreams; ++theIndex)
	{
		CAAudioHardwareStream theStream(GetStreamByIndex(inSection, theIndex));
		theStream.GetCurrentIOProcFormat(outFormats[theIndex]);
	}
}

void	CAAudioHardwareDevice::GetCurrentPhysicalFormats(CAAudioHardwareDeviceSectionID inSection, UInt32& ioNumberStreams, AudioStreamBasicDescription* outFormats) const
{
	ioNumberStreams = std::min(ioNumberStreams, GetNumberStreams(inSection));
	for(UInt32 theIndex = 0; theIndex < ioNumberStreams; ++theIndex)
	{
		CAAudioHardwareStream theStream(GetStreamByIndex(inSection, theIndex));
		theStream.GetCurrentPhysicalFormat(outFormats[theIndex]);
	}
}

bool	CAAudioHardwareDevice::HasVolumeControl(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	return HasProperty(inChannel, inSection, kAudioDevicePropertyVolumeScalar);
}

bool	CAAudioHardwareDevice::VolumeControlIsSettable(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	return PropertyIsSettable(inChannel, inSection, kAudioDevicePropertyVolumeScalar);
}

Float32	CAAudioHardwareDevice::GetVolumeControlScalarValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	Float32 theValue = 0.0;
	UInt32 theSize = sizeof(Float32);
	GetPropertyData(inChannel, inSection, kAudioDevicePropertyVolumeScalar, theSize, &theValue);
	return theValue;
}

Float32	CAAudioHardwareDevice::GetVolumeControlDecibelValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	Float32 theValue = 0.0;
	UInt32 theSize = sizeof(Float32);
	GetPropertyData(inChannel, inSection, kAudioDevicePropertyVolumeDecibels, theSize, &theValue);
	return theValue;
}

void	CAAudioHardwareDevice::SetVolumeControlScalarValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue)
{
	UInt32 theSize = sizeof(Float32);
	SetPropertyData(inChannel, inSection, kAudioDevicePropertyVolumeScalar, theSize, &inValue);
}

void	CAAudioHardwareDevice::SetVolumeControlDecibelValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue)
{
	UInt32 theSize = sizeof(Float32);
	SetPropertyData(inChannel, inSection, kAudioDevicePropertyVolumeDecibels, theSize, &inValue);
}

Float32	CAAudioHardwareDevice::GetVolumeControlScalarForDecibelValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue) const
{
	Float32 theValue = inValue;
	UInt32 theSize = sizeof(Float32);
	GetPropertyData(inChannel, inSection, kAudioDevicePropertyVolumeDecibelsToScalar, theSize, &theValue);
	return theValue;
}

Float32	CAAudioHardwareDevice::GetVolumeControlDecibelForScalarValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue) const
{
	Float32 theValue = inValue;
	UInt32 theSize = sizeof(Float32);
	GetPropertyData(inChannel, inSection, kAudioDevicePropertyVolumeScalarToDecibels, theSize, &theValue);
	return theValue;
}

bool	CAAudioHardwareDevice::HasMuteControl(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	return HasProperty(inChannel, inSection, kAudioDevicePropertyMute);
}

bool	CAAudioHardwareDevice::MuteControlIsSettable(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	return PropertyIsSettable(inChannel, inSection, kAudioDevicePropertyMute);
}

bool	CAAudioHardwareDevice::GetMuteControlValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	UInt32 theValue = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(inChannel, inSection, kAudioDevicePropertyMute, theSize, &theValue);
	return theValue != 0;
}

void	CAAudioHardwareDevice::SetMuteControlValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, bool inValue)
{
	UInt32 theValue = (inValue ? 1 : 0);
	UInt32 theSize = sizeof(UInt32);
	SetPropertyData(inChannel, inSection, kAudioDevicePropertyMute, theSize, &theValue);
}

bool	CAAudioHardwareDevice::HasPlayThruControl(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	return HasProperty(inChannel, inSection, kAudioDevicePropertyPlayThru);
}

bool	CAAudioHardwareDevice::PlayThruControlIsSettable(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	return PropertyIsSettable(inChannel, inSection, kAudioDevicePropertyPlayThru);
}

bool	CAAudioHardwareDevice::GetPlayThruControlValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	UInt32 theValue = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(inChannel, inSection, kAudioDevicePropertyPlayThru, theSize, &theValue);
	return theValue != 0;
}

void	CAAudioHardwareDevice::SetPlayThruControlValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, bool inValue)
{
	UInt32 theValue = (inValue ? 1 : 0);
	UInt32 theSize = sizeof(UInt32);
	SetPropertyData(inChannel, inSection, kAudioDevicePropertyPlayThru, theSize, &theValue);
}

bool	CAAudioHardwareDevice::HasISubOwnershipControl() const
{
	return HasProperty(0, kAudioDeviceSectionOutput, kAudioDevicePropertyDriverShouldOwniSub);
}

bool	CAAudioHardwareDevice::ISubOwnershipControlIsSettable() const
{
	return PropertyIsSettable(0, kAudioDeviceSectionOutput, kAudioDevicePropertyDriverShouldOwniSub);
}

bool	CAAudioHardwareDevice::GetISubOwnershipControlValue() const
{
	UInt32 theValue = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, kAudioDeviceSectionOutput, kAudioDevicePropertyDriverShouldOwniSub, theSize, &theValue);
	return theValue != 0;
}

void	CAAudioHardwareDevice::SetISubOwnershipControlValue(bool inValue)
{
	UInt32 theValue = (inValue ? 1 : 0);
	UInt32 theSize = sizeof(UInt32);
	SetPropertyData(0, kAudioDeviceSectionOutput, kAudioDevicePropertyDriverShouldOwniSub, theSize, &theValue);
}

bool	CAAudioHardwareDevice::HasSubMuteControl(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	return HasProperty(inChannel, inSection, kAudioDevicePropertySubMute);
}

bool	CAAudioHardwareDevice::SubMuteControlIsSettable(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	return PropertyIsSettable(inChannel, inSection, kAudioDevicePropertySubMute);
}

bool	CAAudioHardwareDevice::GetSubMuteControlValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	UInt32 theValue = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(inChannel, inSection, kAudioDevicePropertySubMute, theSize, &theValue);
	return theValue != 0;
}

void	CAAudioHardwareDevice::SetSubMuteControlValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, bool inValue)
{
	UInt32 theValue = (inValue ? 1 : 0);
	UInt32 theSize = sizeof(UInt32);
	SetPropertyData(inChannel, inSection, kAudioDevicePropertySubMute, theSize, &theValue);
}

bool	CAAudioHardwareDevice::HasSubVolumeControl(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	return HasProperty(inChannel, inSection, kAudioDevicePropertySubVolumeScalar);
}

bool	CAAudioHardwareDevice::SubVolumeControlIsSettable(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	return PropertyIsSettable(inChannel, inSection, kAudioDevicePropertySubVolumeScalar);
}

Float32	CAAudioHardwareDevice::GetSubVolumeControlScalarValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	Float32 theValue = 0.0;
	UInt32 theSize = sizeof(Float32);
	GetPropertyData(inChannel, inSection, kAudioDevicePropertySubVolumeScalar, theSize, &theValue);
	return theValue;
}

Float32	CAAudioHardwareDevice::GetSubVolumeControlDecibelValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const
{
	Float32 theValue = 0.0;
	UInt32 theSize = sizeof(Float32);
	GetPropertyData(inChannel, inSection, kAudioDevicePropertySubVolumeDecibels, theSize, &theValue);
	return theValue;
}

void	CAAudioHardwareDevice::SetSubVolumeControlScalarValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue)
{
	UInt32 theSize = sizeof(Float32);
	SetPropertyData(inChannel, inSection, kAudioDevicePropertySubVolumeScalar, theSize, &inValue);
}

void	CAAudioHardwareDevice::SetSubVolumeControlDecibelValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue)
{
	UInt32 theSize = sizeof(Float32);
	SetPropertyData(inChannel, inSection, kAudioDevicePropertySubVolumeDecibels, theSize, &inValue);
}

Float32	CAAudioHardwareDevice::GetSubVolumeControlScalarForDecibelValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue) const
{
	Float32 theValue = inValue;
	UInt32 theSize = sizeof(Float32);
	GetPropertyData(inChannel, inSection, kAudioDevicePropertySubVolumeDecibelsToScalar, theSize, &theValue);
	return theValue;
}

Float32	CAAudioHardwareDevice::GetSubVolumeControlDecibelForScalarValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue) const
{
	Float32 theValue = inValue;
	UInt32 theSize = sizeof(Float32);
	GetPropertyData(inChannel, inSection, kAudioDevicePropertySubVolumeScalarToDecibels, theSize, &theValue);
	return theValue;
}

bool	CAAudioHardwareDevice::HasProperty(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID) const
{
	OSStatus theError = AudioDeviceGetPropertyInfo(mAudioDeviceID, inChannel, inSection, inPropertyID, NULL, NULL);
	return theError == 0;
}

bool	CAAudioHardwareDevice::PropertyIsSettable(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID) const
{
	Boolean isWritable = false;
	OSStatus theError = AudioDeviceGetPropertyInfo(mAudioDeviceID, inChannel, inSection, inPropertyID, NULL, &isWritable);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::PropertyIsSettable: got an error getting info about a property");
	return isWritable != 0;
}

UInt32	CAAudioHardwareDevice::GetPropertyDataSize(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID) const
{
	UInt32 theSize = 0;
	OSStatus theError = AudioDeviceGetPropertyInfo(mAudioDeviceID, inChannel, inSection, inPropertyID, &theSize, NULL);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::GetPropertyDataSize: got an error getting info about a property");
	return theSize;
}

void	CAAudioHardwareDevice::GetPropertyData(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID, UInt32& ioDataSize, void* outData) const
{
	OSStatus theError = AudioDeviceGetProperty(mAudioDeviceID, inChannel, inSection, inPropertyID, &ioDataSize, outData);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::GetPropertyData: got an error getting the value of a property");
}

void	CAAudioHardwareDevice::SetPropertyData(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID, UInt32 inDataSize, const void* inData, const AudioTimeStamp* inWhen)
{
	OSStatus theError = AudioDeviceSetProperty(mAudioDeviceID, inWhen, inChannel, inSection, inPropertyID, inDataSize, inData);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::SetPropertyData: got an error setting the value of a property");
}

void	CAAudioHardwareDevice::AddPropertyListener(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID, AudioDevicePropertyListenerProc inListenerProc, void* inClientData)
{
	OSStatus theError = AudioDeviceAddPropertyListener(mAudioDeviceID, inChannel, inSection, inPropertyID, inListenerProc, inClientData);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::AddPropertyListener: got an error adding a property listener");
}

void	CAAudioHardwareDevice::RemovePropertyListener(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID, AudioDevicePropertyListenerProc inListenerProc)
{
	OSStatus theError = AudioDeviceRemovePropertyListener(mAudioDeviceID, inChannel, inSection, inPropertyID, inListenerProc);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareDevice::RemovePropertyListener: got an error removing a property listener");
}

void	CAAudioHardwareDevice::GetNameForTransportType(UInt32 inTransportType, char* outName)
{
	switch(inTransportType)
	{
		case 0:
			strcpy(outName, "Unknown");
			break;
			
		case kIOAudioDeviceTransportTypeBuiltIn:
			strcpy(outName, "Built-In");
			break;
			
		case kIOAudioDeviceTransportTypePCI:
			strcpy(outName, "PCI");
			break;
			
		case kIOAudioDeviceTransportTypeUSB:
			strcpy(outName, "USB");
			break;
			
		case kIOAudioDeviceTransportTypeFireWire:
			strcpy(outName, "FireWire");
			break;
			
		case kIOAudioDeviceTransportTypeNetwork:
			strcpy(outName, "Network");
			break;
			
		case kIOAudioDeviceTransportTypeWireless:
			strcpy(outName, "Wireless");
			break;
			
		case kIOAudioDeviceTransportTypeOther:
			strcpy(outName, "Other");
			break;
		
		default:
			{
				char* the4CC = (char*)&inTransportType;
				outName[0] = the4CC[0];
				outName[1] = the4CC[1];
				outName[2] = the4CC[2];
				outName[3] = the4CC[3];
				outName[4] = 0;
			}
			break;
	};
}
