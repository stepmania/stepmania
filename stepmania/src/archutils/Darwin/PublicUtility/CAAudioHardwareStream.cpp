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
	CAAudioHardwareStream.cpp

=============================================================================*/

//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "CAAudioHardwareStream.h"

//	PublicUtility Includes
#include "CAAutoDisposer.h"
#include "CADebugMacros.h"
#include "CAException.h"

//	System Includes
#include <IOKit/audio/IOAudioTypes.h>
#include <algorithm>

//=============================================================================
//	CAAudioHardwareStream
//=============================================================================

CAAudioHardwareStream::CAAudioHardwareStream(AudioStreamID inAudioStreamID)
:
	mAudioStreamID(inAudioStreamID)
{
}

CAAudioHardwareStream::~CAAudioHardwareStream()
{
}

CFStringRef	CAAudioHardwareStream::CopyName() const
{
	CFStringRef theAnswer = NULL;
	UInt32 theSize = sizeof(CFStringRef);
	GetPropertyData(0, kAudioDevicePropertyDeviceNameCFString, theSize, &theAnswer);
	return theAnswer;
}

AudioDeviceID	CAAudioHardwareStream::GetOwningDevice() const
{
	AudioDeviceID theAnswer = 0;
	UInt32 theSize = sizeof(AudioDeviceID);
	GetPropertyData(0, kAudioStreamPropertyOwningDevice, theSize, &theAnswer);
	return theAnswer;
}

CAAudioHardwareDeviceSectionID	CAAudioHardwareStream::GetSection() const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, kAudioStreamPropertyDirection, theSize, &theAnswer);
	return (theAnswer == 0) ? kAudioDeviceSectionOutput : kAudioDeviceSectionInput;
}

UInt32	CAAudioHardwareStream::GetTerminalType() const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, kAudioStreamPropertyTerminalType, theSize, &theAnswer);
	return theAnswer;
}

UInt32	CAAudioHardwareStream::GetStartingDeviceChannel() const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, kAudioStreamPropertyStartingChannel, theSize, &theAnswer);
	return theAnswer;
}

bool	CAAudioHardwareStream::HasIsConnectedStatus() const
{
	return HasProperty(0, kAudioDevicePropertyJackIsConnected);
}

bool	CAAudioHardwareStream::GetIsConnectedStatus() const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, kAudioDevicePropertyJackIsConnected, theSize, &theAnswer);
	return theAnswer != 0;
}

bool	CAAudioHardwareStream::HasDataSourceControl() const
{
	return HasProperty(0, kAudioDevicePropertyDataSource);
}

bool	CAAudioHardwareStream::DataSourceControlIsSettable() const
{
	return PropertyIsSettable(0, kAudioDevicePropertyDataSource);
}

UInt32	CAAudioHardwareStream::GetCurrentDataSourceID() const
{
	UInt32 theAnswer = 0;
	UInt32 theSize = sizeof(UInt32);
	GetPropertyData(0, kAudioDevicePropertyDataSource, theSize, &theAnswer);
	return theAnswer;
}

void	CAAudioHardwareStream::SetCurrentDataSourceByID(UInt32 inID)
{
	UInt32 theSize = sizeof(UInt32);
	SetPropertyData(0, kAudioDevicePropertyDataSource, theSize, &inID);
}

UInt32	CAAudioHardwareStream::GetNumberAvailableDataSources() const
{
	UInt32 theAnswer = 0;
	if(HasDataSourceControl())
	{
		UInt32 theSize = GetPropertyDataSize(0, kAudioDevicePropertyDataSources);
		theAnswer = theSize / sizeof(UInt32);
	}
	return theAnswer;
}

UInt32	CAAudioHardwareStream::GetAvailableDataSourceByIndex(UInt32 inIndex) const
{
	AudioStreamID theAnswer = 0;
	UInt32 theNumberSources = GetNumberAvailableDataSources();
	if((theNumberSources > 0) && (inIndex < theNumberSources))
	{
		CAAutoArrayDelete<UInt32> theSourceList(theNumberSources);
		GetAvailableDataSources(theNumberSources, theSourceList);
		theAnswer = theSourceList[inIndex];
	}
	return theAnswer;
}

void	CAAudioHardwareStream::GetAvailableDataSources(UInt32& ioNumberSources, UInt32* outSources) const
{
	UInt32 theNumberSources = std::min(GetNumberAvailableDataSources(), ioNumberSources);
	UInt32 theSize = theNumberSources * sizeof(UInt32);
	GetPropertyData(0, kAudioDevicePropertyDataSources, theSize, outSources);
	ioNumberSources = theSize / sizeof(UInt32);
	UInt32* theFirstItem = &(outSources[0]);
	UInt32* theLastItem = theFirstItem + ioNumberSources;
	std::sort(theFirstItem, theLastItem);
}

CFStringRef	CAAudioHardwareStream::CopyDataSourceNameForID(UInt32 inID) const
{
	CFStringRef theAnswer = NULL;
	AudioValueTranslation theTranslation = { &inID, sizeof(UInt32), &theAnswer, sizeof(CFStringRef) };
	UInt32 theSize = sizeof(AudioValueTranslation);
	GetPropertyData(0, kAudioDevicePropertyDataSourceNameForIDCFString, theSize, &theTranslation);
	return theAnswer;
}

void	CAAudioHardwareStream::GetCurrentIOProcFormat(AudioStreamBasicDescription& outFormat) const
{
	memset(&outFormat, 0, sizeof(AudioStreamBasicDescription));
	UInt32 theSize = sizeof(AudioStreamBasicDescription);
	GetPropertyData(0, kAudioDevicePropertyStreamFormat, theSize, &outFormat);
}

void	CAAudioHardwareStream::SetCurrentIOProcFormat(const AudioStreamBasicDescription& inFormat)
{
	UInt32 theSize = sizeof(AudioStreamBasicDescription);
	SetPropertyData(0, kAudioDevicePropertyStreamFormat, theSize, &inFormat);
}

UInt32	CAAudioHardwareStream::GetNumberAvailableIOProcFormats() const
{
	UInt32 theAnswer = GetPropertyDataSize(0, kAudioDevicePropertyStreamFormats);
	theAnswer /= sizeof(AudioStreamBasicDescription);
	return theAnswer;
}

void	CAAudioHardwareStream::GetAvailableIOProcFormats(UInt32& ioNumberFormats, AudioStreamBasicDescription* outFormats) const
{
	UInt32 theSize = ioNumberFormats * sizeof(AudioStreamBasicDescription);
	GetPropertyData(0, kAudioDevicePropertyStreamFormats, theSize, outFormats);
	ioNumberFormats = theSize / sizeof(AudioStreamBasicDescription);
}

void	CAAudioHardwareStream::GetAvailableIOProcFormatByIndex(UInt32 inIndex, AudioStreamBasicDescription& outFormat) const
{
	UInt32 theNumberFormats = GetNumberAvailableIOProcFormats();
	ThrowIf(inIndex >= theNumberFormats, CAException(kAudioHardwareIllegalOperationError), "CAAudioHardwareStream::GetAvailableIOProcFormatByIndex: index out of range");
	CAAutoArrayDelete<AudioStreamBasicDescription> theFormats(theNumberFormats);
	GetAvailableIOProcFormats(theNumberFormats, theFormats);
	outFormat = theFormats[inIndex];
}

void	CAAudioHardwareStream::GetCurrentPhysicalFormat(AudioStreamBasicDescription& outFormat) const
{
	memset(&outFormat, 0, sizeof(AudioStreamBasicDescription));
	UInt32 theSize = sizeof(AudioStreamBasicDescription);
	GetPropertyData(0, kAudioStreamPropertyPhysicalFormat, theSize, &outFormat);
}

void	CAAudioHardwareStream::SetCurrentPhysicalFormat(const AudioStreamBasicDescription& inFormat)
{
	UInt32 theSize = sizeof(AudioStreamBasicDescription);
	SetPropertyData(0, kAudioStreamPropertyPhysicalFormat, theSize, &inFormat);
}

UInt32	CAAudioHardwareStream::GetNumberAvailablePhysicalFormats() const
{
	UInt32 theAnswer = GetPropertyDataSize(0, kAudioStreamPropertyPhysicalFormats);
	theAnswer /= sizeof(AudioStreamBasicDescription);
	return theAnswer;
}

void	CAAudioHardwareStream::GetAvailablePhysicalFormats(UInt32& ioNumberFormats, AudioStreamBasicDescription* outFormats) const
{
	UInt32 theSize = ioNumberFormats * sizeof(AudioStreamBasicDescription);
	GetPropertyData(0, kAudioStreamPropertyPhysicalFormats, theSize, outFormats);
	ioNumberFormats = theSize / sizeof(AudioStreamBasicDescription);
}

void	CAAudioHardwareStream::GetAvailablePhysicalFormatByIndex(UInt32 inIndex, AudioStreamBasicDescription& outFormat) const
{
	UInt32 theNumberFormats = GetNumberAvailablePhysicalFormats();
	ThrowIf(inIndex >= theNumberFormats, CAException(kAudioHardwareIllegalOperationError), "CAAudioHardwareStream::GetAvailablePhysicalFormatByIndex: index out of range");
	CAAutoArrayDelete<AudioStreamBasicDescription> theFormats(theNumberFormats);
	GetAvailablePhysicalFormats(theNumberFormats, theFormats);
	outFormat = theFormats[inIndex];
}

bool	CAAudioHardwareStream::HasProperty(UInt32 inChannel, AudioHardwarePropertyID inPropertyID) const
{
	OSStatus theError = AudioStreamGetPropertyInfo(GetAudioStreamID(), inChannel, inPropertyID, NULL, NULL);
	return theError == 0;
}

bool	CAAudioHardwareStream::PropertyIsSettable(UInt32 inChannel, AudioHardwarePropertyID inPropertyID) const
{
	Boolean isWritable = false;
	OSStatus theError = AudioStreamGetPropertyInfo(GetAudioStreamID(), inChannel, inPropertyID, NULL, &isWritable);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareStream::PropertyIsSettable: got an error getting info about a property");
	return isWritable != 0;
}

UInt32	CAAudioHardwareStream::GetPropertyDataSize(UInt32 inChannel, AudioHardwarePropertyID inPropertyID) const
{
	UInt32 theSize = 0;
	OSStatus theError = AudioStreamGetPropertyInfo(GetAudioStreamID(), inChannel, inPropertyID, &theSize, NULL);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareStream::GetPropertyDataSize: got an error getting info about a property");
	return theSize;
}

void	CAAudioHardwareStream::GetPropertyData(UInt32 inChannel, AudioHardwarePropertyID inPropertyID, UInt32& ioDataSize, void* outData) const
{
	OSStatus theError = AudioStreamGetProperty(GetAudioStreamID(), inChannel, inPropertyID, &ioDataSize, outData);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareStream::GetPropertyData: got an error getting the value of a property");
}

void	CAAudioHardwareStream::SetPropertyData(UInt32 inChannel, AudioHardwarePropertyID inPropertyID, UInt32 inDataSize, const void* inData, const AudioTimeStamp* inWhen)
{
	OSStatus theError = AudioStreamSetProperty(GetAudioStreamID(), inWhen, inChannel, inPropertyID, inDataSize, inData);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareStream::SetPropertyData: got an error setting the value of a property");
}

void	CAAudioHardwareStream::AddPropertyListener(UInt32 inChannel, AudioHardwarePropertyID inPropertyID, AudioStreamPropertyListenerProc inListenerProc, void* inClientData)
{
	OSStatus theError = AudioStreamAddPropertyListener(GetAudioStreamID(), inChannel, inPropertyID, inListenerProc, inClientData);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareStream::AddPropertyListener: got an error adding a property listener");
}

void	CAAudioHardwareStream::RemovePropertyListener(UInt32 inChannel, AudioHardwarePropertyID inPropertyID, AudioStreamPropertyListenerProc inListenerProc)
{
	OSStatus theError = AudioStreamRemovePropertyListener(GetAudioStreamID(), inChannel, inPropertyID, inListenerProc);
	ThrowIfError(theError, CAException(theError), "CAAudioHardwareStream::RemovePropertyListener: got an error removing a property listener");
}

void	CAAudioHardwareStream::GetNameForTerminalType(UInt32 inTerminalType, char* outName)
{
	switch(inTerminalType)
	{
		//	types that go nowhere
		case OUTPUT_NULL:
			strcpy(outName, "Null Output");
			break;

		case INPUT_NULL:
			strcpy(outName, "Null Input");
			break;

		//	Input terminal types
		case INPUT_UNDEFINED:
			strcpy(outName, "Undefined Input");
			break;

		case INPUT_MICROPHONE:
			strcpy(outName, "Microphone");
			break;

		case INPUT_DESKTOP_MICROPHONE:
			strcpy(outName, "Desktop Microphone");
			break;

		case INPUT_PERSONAL_MICROPHONE:
			strcpy(outName, "Personal Microphone");
			break;

		case INPUT_OMNIDIRECTIONAL_MICROPHONE:
			strcpy(outName, "Omnidirectional Microphone");
			break;

		case INPUT_MICROPHONE_ARRAY:
			strcpy(outName, "Microphone Array");
			break;

		case INPUT_PROCESSING_MICROPHONE_ARRAY:
			strcpy(outName, "Processing Microphone Array");
			break;

		case INPUT_MODEM_AUDIO:
			strcpy(outName, "Modem");
			break;
		
		//	Output terminal types
		case OUTPUT_UNDEFINED:
			strcpy(outName, "Undefined Output");
			break;
		
		case OUTPUT_SPEAKER:
			strcpy(outName, "Speaker");
			break;
		
		case OUTPUT_HEADPHONES:
			strcpy(outName, "Headphones");
			break;
		
		case OUTPUT_HEAD_MOUNTED_DISPLAY_AUDIO:
			strcpy(outName, "Head-Monted Display");
			break;
		
		case OUTPUT_DESKTOP_SPEAKER:
			strcpy(outName, "Desktop Speaker");
			break;
		
		case OUTPUT_ROOM_SPEAKER:
			strcpy(outName, "Room Speaker");
			break;
		
		case OUTPUT_COMMUNICATION_SPEAKER:
			strcpy(outName, "Communication Speaker");
			break;
		
		case OUTPUT_LOW_FREQUENCY_EFFECTS_SPEAKER:
			strcpy(outName, "LFE Speaker");
			break;
		
		//	Bi-directional terminal types
		case BIDIRECTIONAL_UNDEFINED:
			strcpy(outName, "Undefined Bidirectional");
			break;
		
		case BIDIRECTIONAL_HANDSET:
			strcpy(outName, "Handset");
			break;
		
		case BIDIRECTIONAL_HEADSET:
			strcpy(outName, "Headset");
			break;
		
		case BIDIRECTIONAL_SPEAKERPHONE_NO_ECHO_REDX:
			strcpy(outName, "Speakerphone w/o Echo Reduction");
			break;
		
		case BIDIRECTIONAL_ECHO_SUPPRESSING_SPEAKERPHONE:
			strcpy(outName, "Speakerphone w/ Echo Suppression");
			break;
		
		case BIDIRECTIONAL_ECHO_CANCELING_SPEAKERPHONE:
			strcpy(outName, "Speakerphone w/ Echo Cacellation");
			break;
		
		//	Telephony terminal types
		case TELEPHONY_UNDEFINED:
			strcpy(outName, "Undefined telephony");
			break;
		
		case TELEPHONY_PHONE_LINE:
			strcpy(outName, "Telephone Line");
			break;
		
		case TELEPHONY_TELEPHONE:
			strcpy(outName, "Telephone");
			break;
		
		case TELEPHONY_DOWN_LINE_PHONE:
			strcpy(outName, "Down Line Telephone");
			break;
		
		//	External terminal types
		case EXTERNAL_UNDEFINED:
			strcpy(outName, "Undefined External");
			break;
		
		case EXTERNAL_ANALOG_CONNECTOR:
			strcpy(outName, "Analog");
			break;
		
		case EXTERNAL_DIGITAL_AUDIO_INTERFACE:
			strcpy(outName, "Digital Interface");
			break;
		
		case EXTERNAL_LINE_CONNECTOR:
			strcpy(outName, "Line");
			break;
		
		case EXTERNAL_LEGACY_AUDIO_CONNECTOR:
			strcpy(outName, "Legacy");
			break;
		
		case EXTERNAL_SPDIF_INTERFACE:
			strcpy(outName, "SPDIF");
			break;
		
		case EXTERNAL_1394_DA_STREAM:
			strcpy(outName, "1394 Digital Audio");
			break;
		
		case EXTERNAL_1394_DV_STREAM_SOUNDTRACK:
			strcpy(outName, "1394 Digital Video");
			break;
		
		//	Embedded terminal types
		case EMBEDDED_UNDEFINED:
			strcpy(outName, "Undefined Embedded");
			break;
		
		case EMBEDDED_LEVEL_CALIBRATION_NOISE_SOURCE:
			strcpy(outName, "Level Calibration Noise Source");
			break;
		
		case EMBEDDED_EQUALIZATION_NOISE:
			strcpy(outName, "Equalization Noise");
			break;
		
		case EMBEDDED_CD_PLAYER:
			strcpy(outName, "CD Player");
			break;
		
		case EMBEDDED_DAT:
			strcpy(outName, "DAT");
			break;
		
		case EMBEDDED_DCC:
			strcpy(outName, "DCC");
			break;
		
		case EMBEDDED_MINIDISK:
			strcpy(outName, "Minidisk");
			break;
		
		case EMBEDDED_ANALOG_TAPE:
			strcpy(outName, "Analog Tape");
			break;
		
		case EMBEDDED_PHONOGRAPH:
			strcpy(outName, "Phonograph");
			break;
		
		case EMBEDDED_VCR_AUDIO:
			strcpy(outName, "VCR");
			break;
		
		case EMBEDDED_VIDEO_DISC_AUDIO:
			strcpy(outName, "Video Disc");
			break;
		
		case EMBEDDED_DVD_AUDIO:
			strcpy(outName, "DVD");
			break;
		
		case EMBEDDED_TV_TUNER_AUDIO:
			strcpy(outName, "TV Tuner");
			break;
		
		case EMBEDDED_SATELLITE_RECEIVER_AUDIO:
			strcpy(outName, "Satellite Receiver");
			break;
		
		case EMBEDDED_CABLE_TUNER_AUDIO:
			strcpy(outName, "Cable Tuner");
			break;
		
		case EMBEDDED_DSS_AUDIO:
			strcpy(outName, "DSS");
			break;
		
		case EMBEDDED_RADIO_RECEIVER:
			strcpy(outName, "Radio Receiver");
			break;
		
		case EMBEDDED_RADIO_TRANSMITTER:
			strcpy(outName, "Radio Transmitter");
			break;
		
		case EMBEDDED_MULTITRACK_RECORDER:
			strcpy(outName, "Mutitrack Recorder");
			break;
		
		case EMBEDDED_SYNTHESIZER:
			strcpy(outName, "Synthesizer");
			break;
		
		//	Processing terminal types
		case PROCESSOR_UNDEFINED:
			strcpy(outName, "Undefined Processor");
			break;
		
		case PROCESSOR_GENERAL:
			strcpy(outName, "General Processor");
			break;
		
		default:
			sprintf(outName, "0x%X", (unsigned int)inTerminalType);
			break;
		
	};
}
