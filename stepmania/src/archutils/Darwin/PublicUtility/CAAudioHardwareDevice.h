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
	CAAudioHardwareDevice.h

=============================================================================*/
#if !defined(__CAAudioHardwareDevice_h__)
#define __CAAudioHardwareDevice_h__

//=============================================================================
//	Includes
//=============================================================================

//	System Includes
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/types.h>

//=============================================================================
//	Types
//=============================================================================

typedef	UInt8	CAAudioHardwareDeviceSectionID;
#define	kAudioDeviceSectionInput	((CAAudioHardwareDeviceSectionID)0x01)
#define	kAudioDeviceSectionOutput	((CAAudioHardwareDeviceSectionID)0x00)
#define	kAudioDeviceSectionGlobal	((CAAudioHardwareDeviceSectionID)0x00)
#define	kAudioDeviceSectionWildcard	((CAAudioHardwareDeviceSectionID)0xFF)

//=============================================================================
//	CAAudioHardwareDevice
//=============================================================================

class CAAudioHardwareDevice
{

//	Construction/Destruction
public:
					CAAudioHardwareDevice(AudioDeviceID inAudioDeviceID);
					~CAAudioHardwareDevice();

//	General Operations
public:
	AudioDeviceID	GetAudioDeviceID() const { return mAudioDeviceID; }
	CFStringRef		CopyName() const;
	CFStringRef		CopyManufacturer() const;
	CFStringRef		CopyUID() const;
	CFStringRef		CopyConfigurationApplicationBundleID() const;
	UInt32			GetTransportType() const;
	bool			CanBeDefaultDevice(CAAudioHardwareDeviceSectionID inSection, bool inIsSystem = false) const;
	
	bool			HasDevicePlugInStatus() const;
	OSStatus		GetDevicePlugInStatus() const;
	
	bool			IsAlive() const;
	bool			IsRunning() const;
	void			SetIsRunning(bool inIsRunning);
	bool			IsRunningSomewhere() const;
	
	pid_t			GetHogModeOwner() const;
	bool			TakeHogMode();
	void			ReleaseHogMode();
	
	bool			SupportsChangingMixability() const;
	bool			IsMixable() const;
	void			SetIsMixable(bool inIsMixable);
	
	bool			HasIsConnectedStatus(CAAudioHardwareDeviceSectionID inSection) const;
	bool			GetIsConnectedStatus(CAAudioHardwareDeviceSectionID inSection) const;
	
	bool			HasPreferredStereoChannels(CAAudioHardwareDeviceSectionID inSection) const;
	void			GetPreferredStereoChannels(CAAudioHardwareDeviceSectionID inSection, UInt32& outLeft, UInt32& outRight) const;
	void			SetPreferredStereoChannels(CAAudioHardwareDeviceSectionID inSection, UInt32 inLeft, UInt32 inRight);
	
//	IO Operations
public:
	UInt32			GetLatency(CAAudioHardwareDeviceSectionID inSection) const;
	UInt32			GetSafetyOffset(CAAudioHardwareDeviceSectionID inSection) const;
	
	bool			HasClockSourceControl() const;
	bool			ClockSourceControlIsSettable() const;
	UInt32			GetCurrentClockSourceID() const;
	void			SetCurrentClockSourceByID(UInt32 inID);
	UInt32			GetNumberAvailableClockSources() const;
	UInt32			GetAvailableClockSourceByIndex(UInt32 inIndex) const;
	void			GetAvailableClockSources(UInt32& ioNumberSources, UInt32* outSources) const;
	CFStringRef		CopyClockSourceNameForID(UInt32 inID) const;

	bool			HasDataSourceControl(CAAudioHardwareDeviceSectionID inSection) const;
	bool			DataSourceControlIsSettable(CAAudioHardwareDeviceSectionID inSection) const;
	UInt32			GetCurrentDataSourceID(CAAudioHardwareDeviceSectionID inSection) const;
	void			SetCurrentDataSourceByID(CAAudioHardwareDeviceSectionID inSection, UInt32 inID);
	UInt32			GetNumberAvailableDataSources(CAAudioHardwareDeviceSectionID inSection) const;
	UInt32			GetAvailableDataSourceByIndex(CAAudioHardwareDeviceSectionID inSection, UInt32 inIndex) const;
	void			GetAvailableDataSources(CAAudioHardwareDeviceSectionID inSection, UInt32& ioNumberSources, UInt32* outSources) const;
	CFStringRef		CopyDataSourceNameForID(CAAudioHardwareDeviceSectionID inSection, UInt32 inID) const;

	Float64			GetActualSampleRate() const;
	Float64			GetNominalSampleRate() const;
	void			SetNominalSampleRate(Float64 inSampleRate);
	bool			IsValidNominalSampleRate(Float64 inSampleRate) const;
	UInt32			GetNumberNominalSampleRateRanges() const;
	void			GetNominalSampleRateRanges(UInt32& ioNumberRanges, AudioValueRange* outRanges) const;
	void			GetNominalSampleRateRangeByIndex(UInt32 inIndex, Float64& outMinimum, Float64& outMaximum) const;
	
	UInt32			GetIOBufferSize() const;
	void			SetIOBufferSize(UInt32 inBufferSize);
	bool			UsesVariableIOBufferSizes() const;
	UInt32			GetMaximumVariableIOBufferSize() const;
	void			GetIOBufferSizeRange(UInt32& outMinimum, UInt32& outMaximum) const;

	void			AddIOProc(AudioDeviceIOProc inIOProc, void* inClientData);
	void			RemoveIOProc(AudioDeviceIOProc inIOProc);
	
	void			StartIOProc(AudioDeviceIOProc inIOProc);
	void			StartIOProcAtTime(AudioDeviceIOProc inIOProc, AudioTimeStamp& ioStartTime, bool inIsInput, bool inIgnoreHardware);
	void			StopIOProc(AudioDeviceIOProc inIOProc);
	
	void			GetIOProcStreamUsage(AudioDeviceIOProc inIOProc, CAAudioHardwareDeviceSectionID inSection, bool* outStreamUsage) const;
	void			SetIOProcStreamUsage(AudioDeviceIOProc inIOProc, CAAudioHardwareDeviceSectionID inSection, const bool* inStreamUsage);

//	Time Operations
public:
	void			GetCurrentTime(AudioTimeStamp& outTime);
	void			TranslateTime(const AudioTimeStamp& inTime, AudioTimeStamp& outTime);
	void			GetNearestStartTime(AudioTimeStamp& ioTime, bool inIsInput, bool inIgnoreHardware);

//	Stream Operations
public:
	UInt32			GetNumberStreams(CAAudioHardwareDeviceSectionID inSection) const;
	void			GetStreams(CAAudioHardwareDeviceSectionID inSection, UInt32& ioNumberStreams, AudioStreamID* outStreamList) const;
	AudioStreamID	GetStreamByIndex(CAAudioHardwareDeviceSectionID inSection, UInt32 inIndex) const;
	
	bool			HasSection(CAAudioHardwareDeviceSectionID inSection) const { return GetNumberStreams(inSection) > 0; }
	UInt32			GetTotalNumberChannels(CAAudioHardwareDeviceSectionID inSection) const;

//	Format Operations
public:
	void			GetCurrentIOProcFormats(CAAudioHardwareDeviceSectionID inSection, UInt32& ioNumberStreams, AudioStreamBasicDescription* outFormats) const;
	void			GetCurrentPhysicalFormats(CAAudioHardwareDeviceSectionID inSection, UInt32& ioNumberStreams, AudioStreamBasicDescription* outFormats) const;

//	Control Operations
public:
	bool			HasVolumeControl(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	bool			VolumeControlIsSettable(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	Float32			GetVolumeControlScalarValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	Float32			GetVolumeControlDecibelValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	void			SetVolumeControlScalarValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue);
	void			SetVolumeControlDecibelValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue);
	Float32			GetVolumeControlScalarForDecibelValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue) const;
	Float32			GetVolumeControlDecibelForScalarValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue) const;
	
	bool			HasMuteControl(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	bool			MuteControlIsSettable(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	bool			GetMuteControlValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	void			SetMuteControlValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, bool inValue);
	
	bool			HasPlayThruControl(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	bool			PlayThruControlIsSettable(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	bool			GetPlayThruControlValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	void			SetPlayThruControlValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, bool inValue);
	
	bool			HasISubOwnershipControl() const;
	bool			ISubOwnershipControlIsSettable() const;
	bool			GetISubOwnershipControlValue() const;
	void			SetISubOwnershipControlValue(bool inValue);
	
	bool			HasSubMuteControl(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	bool			SubMuteControlIsSettable(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	bool			GetSubMuteControlValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	void			SetSubMuteControlValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, bool inValue);
	
	bool			HasSubVolumeControl(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	bool			SubVolumeControlIsSettable(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	Float32			GetSubVolumeControlScalarValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	Float32			GetSubVolumeControlDecibelValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection) const;
	void			SetSubVolumeControlScalarValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue);
	void			SetSubVolumeControlDecibelValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue);
	Float32			GetSubVolumeControlScalarForDecibelValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue) const;
	Float32			GetSubVolumeControlDecibelForScalarValue(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, Float32 inValue) const;
	
//	Property Operations
public:
	bool			HasProperty(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID) const;
	bool			PropertyIsSettable(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID) const;
	
	UInt32			GetPropertyDataSize(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID) const;
	void			GetPropertyData(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID, UInt32& ioDataSize, void* outData) const;
	void			SetPropertyData(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID, UInt32 inDataSize, const void* inData, const AudioTimeStamp* inWhen = NULL);
	
	void			AddPropertyListener(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID, AudioDevicePropertyListenerProc inListenerProc, void* inClientData);
	void			RemovePropertyListener(UInt32 inChannel, CAAudioHardwareDeviceSectionID inSection, AudioHardwarePropertyID inPropertyID, AudioDevicePropertyListenerProc inListenerProc);

//	Utility Operations
public:
	static void		GetNameForTransportType(UInt32 inTransportType, char* outName);

//	Implementation
private:
	AudioDeviceID	mAudioDeviceID;

};

#endif
