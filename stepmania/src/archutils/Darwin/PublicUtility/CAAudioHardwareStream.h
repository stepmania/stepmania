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
	CAAudioHardwareStream.h

=============================================================================*/
#if !defined(__CAAudioHardwareStream_h__)
#define __CAAudioHardwareStream_h__

//=============================================================================
//	Includes
//=============================================================================

//	Local Includes
#include "CAAudioHardwareDevice.h"

//=============================================================================
//	CAAudioHardwareStream
//=============================================================================

class CAAudioHardwareStream
{

//	Construction/Destruction
public:
									CAAudioHardwareStream(AudioStreamID inAudioStreamID);
									~CAAudioHardwareStream();

//	General Operations
public:
	AudioStreamID					GetAudioStreamID() const { return mAudioStreamID; }
	CFStringRef						CopyName() const;
	AudioDeviceID					GetOwningDevice() const;
	CAAudioHardwareDeviceSectionID	GetSection() const;
	UInt32							GetTerminalType() const;
	UInt32							GetStartingDeviceChannel() const;

	bool							HasIsConnectedStatus() const;
	bool							GetIsConnectedStatus() const;
	
	bool							HasDataSourceControl() const;
	bool							DataSourceControlIsSettable() const;
	UInt32							GetCurrentDataSourceID() const;
	void							SetCurrentDataSourceByID(UInt32 inID);
	UInt32							GetNumberAvailableDataSources() const;
	UInt32							GetAvailableDataSourceByIndex(UInt32 inIndex) const;
	void							GetAvailableDataSources(UInt32& ioNumberSources, UInt32* outSources) const;
	CFStringRef						CopyDataSourceNameForID(UInt32 inID) const;

//	Format Operations
public:
	void							GetCurrentIOProcFormat(AudioStreamBasicDescription& outFormat) const;
	void							SetCurrentIOProcFormat(const AudioStreamBasicDescription& inFormat);
	UInt32							GetNumberAvailableIOProcFormats() const;
	void							GetAvailableIOProcFormats(UInt32& ioNumberFormats, AudioStreamBasicDescription* outFormats) const;
	void							GetAvailableIOProcFormatByIndex(UInt32 inIndex, AudioStreamBasicDescription& outFormat) const;
	
	void							GetCurrentPhysicalFormat(AudioStreamBasicDescription& outFormat) const;
	void							SetCurrentPhysicalFormat(const AudioStreamBasicDescription& inFormat);
	UInt32							GetNumberAvailablePhysicalFormats() const;
	void							GetAvailablePhysicalFormats(UInt32& ioNumberFormats, AudioStreamBasicDescription* outFormats) const;
	void							GetAvailablePhysicalFormatByIndex(UInt32 inIndex, AudioStreamBasicDescription& outFormat) const;
	
//	Property Operations
public:
	bool							HasProperty(UInt32 inChannel, AudioHardwarePropertyID inPropertyID) const;
	bool							PropertyIsSettable(UInt32 inChannel, AudioHardwarePropertyID inPropertyID) const;
	
	UInt32							GetPropertyDataSize(UInt32 inChannel, AudioHardwarePropertyID inPropertyID) const;
	void							GetPropertyData(UInt32 inChannel, AudioHardwarePropertyID inPropertyID, UInt32& ioDataSize, void* outData) const;
	void							SetPropertyData(UInt32 inChannel, AudioHardwarePropertyID inPropertyID, UInt32 inDataSize, const void* inData, const AudioTimeStamp* inWhen = NULL);
	
	void							AddPropertyListener(UInt32 inChannel, AudioHardwarePropertyID inPropertyID, AudioStreamPropertyListenerProc inListenerProc, void* inClientData);
	void							RemovePropertyListener(UInt32 inChannel, AudioHardwarePropertyID inPropertyID, AudioStreamPropertyListenerProc inListenerProc);
	
//	Utility Operations
public:
	static void						GetNameForTerminalType(UInt32 inTerminalType, char* outName);

//	Implementation
private:
	AudioStreamID					mAudioStreamID;

};

#endif
