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
	FormatConverterClient.h
	
=============================================================================*/

#ifndef __FormatConverterClient_h__
#define __FormatConverterClient_h__

#include <AudioToolbox/AudioConverter.h>
#include <vector>


// ____________________________________________________________________________
//
// FormatConverterClient
// C++ wrapper for an AudioConverter
class FormatConverterClient {
public:
	FormatConverterClient() :
		mConverter(NULL)
	{
	}
	
	virtual ~FormatConverterClient()
	{
		Destroy();
	}
	
	virtual OSStatus	Initialize(const AudioStreamBasicDescription &src, const AudioStreamBasicDescription &dest)
	{
		OSStatus err;
		Destroy();
		err = AudioConverterNew(&src, &dest, &mConverter);
		if (err) return err;
		mInputFormat = src;
		mOutputFormat = dest;
		return noErr;
	}
	
	void	Destroy()
	{
		if (mConverter) {
			AudioConverterDispose(mConverter);
			mConverter = NULL;
		}
	}
	
	OSStatus	Reset () const { return (mConverter ? AudioConverterReset (mConverter) : noErr); }

	OSStatus	SetProperty(AudioConverterPropertyID	inPropertyID,
							UInt32						inPropertyDataSize,
							const void *				inPropertyData)
	{
		return AudioConverterSetProperty(mConverter, inPropertyID, inPropertyDataSize, inPropertyData);
	}
	
	OSStatus	GetProperty(AudioConverterPropertyID	inPropertyID,
							UInt32 &					ioPropertyDataSize,
							void *						outPropertyData)
	{
		return AudioConverterGetProperty(mConverter, inPropertyID, &ioPropertyDataSize, outPropertyData);
	}

	OSStatus	GetPropertyInfo(AudioConverterPropertyID	inPropertyID,
								UInt32 &					outPropertyDataSize,
								Boolean &					outWritable)
	{
		return AudioConverterGetPropertyInfo(mConverter, inPropertyID, &outPropertyDataSize, &outWritable);
	}

	void	SetQuality(UInt32 inQuality)
	{
		SetProperty(kAudioConverterSampleRateConverterQuality, sizeof(UInt32), &inQuality );
	}

	OSStatus	FillComplexBuffer(	UInt32 &							ioOutputDataPacketSize,
									AudioBufferList &					outOutputData,
									AudioStreamPacketDescription*		outPacketDescription)
	{
		OSStatus err;
		err = AudioConverterFillComplexBuffer(mConverter, InputProc, this,
			&ioOutputDataPacketSize, &outOutputData, outPacketDescription);
		return err;
	}
	
	const AudioStreamBasicDescription &	GetInputFormat()	{ return mInputFormat; }

	const AudioStreamBasicDescription &	GetOutputFormat()	{ return mOutputFormat; }	
	
protected:
	virtual OSStatus	FormatConverterInputProc(	
								UInt32 &						ioNumberDataPackets,
								AudioBufferList &				ioData,
								AudioStreamPacketDescription**	outDataPacketDescription) = 0;
	
private:
	static OSStatus	InputProc(	AudioConverterRef				inAudioConverter,
								UInt32*							ioNumberDataPackets,
								AudioBufferList*				ioData,
								AudioStreamPacketDescription**	outDataPacketDescription,
								void*							inUserData)
	{
		OSStatus err;
		try {
			FormatConverterClient *This = (FormatConverterClient *)inUserData;
			AudioBuffer *buf = ioData->mBuffers;
			UInt32 nBytes = *ioNumberDataPackets * This->mInputFormat.mBytesPerPacket;
			for (UInt32 i = ioData->mNumberBuffers; i--; ++buf) {
				buf->mDataByteSize = nBytes;
			}
			err = This->FormatConverterInputProc(*ioNumberDataPackets, *ioData, outDataPacketDescription);
		} catch (OSStatus err) {
			return err;
		}
		catch (...) {
			return -1;
		}
		return err;
	}

protected:
	AudioConverterRef			mConverter;
	AudioStreamBasicDescription	mInputFormat, mOutputFormat;
};

#endif // __FormatConverterClient_h__
