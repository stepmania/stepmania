#include "global.h"
#include "PitchDetectionTest.h"
#include "PitchDetectionTestUtil.h"
#include "srpd.h"
#include "EST_filter.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "NoteTypes.h"
#include <MMSystem.h>

MicrophoneStatus PitchDetectionTest::s_ms;
Microphone *PitchDetectionTest::s_pMicrophone = NULL;

#define MAX_BUFFERS	3

// http://tomscarff.tripod.com/midi_analyser/midi_note_frequency.htm
// http://musicdsp.org/showone.php?id=125
const float MIDI_A4_FREQUENCY = 440.0;
const int MIDI_A4_NUMBER = 69;
const int MIDI_C4_NUMBER = 60;	// middle C
const RString NOTE_NAME[MIDI_NOTES_PER_OCTAVE] = {"A ","A#","B ","C ","C#","D ","D#","E ","F ","F#","G ","G#"};

float MidiNoteToFreq( int iMidiNote )
{
	return MIDI_A4_FREQUENCY * powf( 2.0, ((float)iMidiNote - MIDI_A4_NUMBER) / MIDI_NOTES_PER_OCTAVE );
}

float log2f( float n )
{
	// logb (x) = (loga(x)) / (loga(B))
	return logf(n) / logf(2);
}

bool FreqToMidiNote( float fFreq, float &fMidiNoteOut )
{
	if( fFreq < 20 )	// well below A0 (MidiNote = 1)
		return false;
	fMidiNoteOut = MIDI_NOTES_PER_OCTAVE * log2f( fFreq / MIDI_A4_FREQUENCY) + MIDI_A4_NUMBER;
	return fMidiNoteOut >= 1;
}

#undef bool

bool PitchDetectionTest::MidiNoteToString( int iMidiNote, RString &sMidiNoteOut )
{
	if( iMidiNote < 1 )
		return false;
	int iDiff = iMidiNote - MIDI_A4_NUMBER;
	wrap( iDiff, MIDI_NOTES_PER_OCTAVE );
	sMidiNoteOut = NOTE_NAME[iDiff];
	return true;
}

bool MidiNoteToOctave( int iMidiNote, int &iOctaveOut )
{
	if( iMidiNote < 1 )
		return false;
	// middle C is start of octave 4.
	int iOctave = (int)floorf( ((float)iMidiNote - MIDI_C4_NUMBER) / MIDI_NOTES_PER_OCTAVE );
	iOctaveOut = iOctave + 4;
	return true;
}

bool MidiNoteToStringAndOctave( int iMidiNote, RString &sMidiNoteOut )
{
	if( !PitchDetectionTest::MidiNoteToString(iMidiNote, sMidiNoteOut) )
		return false;
	int iOctave;
	if( !MidiNoteToOctave(iMidiNote, iOctave) )
		return false;
	sMidiNoteOut += ssprintf("%d",iOctave);
	return true;
}

float PitchDetectionTest::WrapToNearestOctave( float fMidiNote, float fTargetMidiNote )
{
	float fDiff = fMidiNote - fTargetMidiNote;
	fDiff += MIDI_NOTES_PER_OCTAVE / 2;
	wrap( fDiff, (float)MIDI_NOTES_PER_OCTAVE );
	fDiff -= MIDI_NOTES_PER_OCTAVE / 2;
	float fRet = fTargetMidiNote + fDiff;
	return fRet;
}


void CALLBACK waveInProc(HWAVEIN hwi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);

class MicrophoneImpl
{
public:
	HWAVEIN m_hWaveIn;
	WAVEFORMATEX m_stWFEX;
	WAVEHDR m_stWHDR[MAX_BUFFERS];
	DetectPitch m_dp;
	HMMIO m_hOPFile;
	MMIOINFO m_stmmIF;
	MMCKINFO m_stckOut,m_stckOutRIFF; 

	static MicrophoneImpl *s_pMicrophoneImpl;	// one and only for the callback function to use


	void Init()
	{
		m_hWaveIn=NULL;
		ZeroMemory(&m_stWFEX,sizeof(WAVEFORMATEX));
		ZeroMemory(m_stWHDR,MAX_BUFFERS*sizeof(WAVEHDR));
		s_pMicrophoneImpl = this;
	}

	void FillDevices()
	{
		WAVEINCAPS stWIC={0};

		UINT nDevices=waveInGetNumDevs();

		for(UINT nC1=0;nC1<nDevices;++nC1)
		{
			ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
			MMRESULT mRes=waveInGetDevCaps(nC1,&stWIC,sizeof(WAVEINCAPS));
			if(mRes==0)
				LOG->Trace( stWIC.szPname );
			else
				FAIL_M("bad");
		}
	}

	void StartRecording()
	{
		OpenDevice();
		
		m_dp.Init( SAMPLES_PER_SEC );
		
		PrepareBuffers();
		MMRESULT mRes=waveInStart(m_hWaveIn);
		if(mRes!=0)
			FAIL_M("bad");
	}

	void StopRecording()
	{
		CloseDevice();
	}

	void OpenDevice()
	{
		MMRESULT mRes=0;

		m_stWFEX.nSamplesPerSec=SAMPLES_PER_SEC;
		m_stWFEX.nChannels=1;
		m_stWFEX.wBitsPerSample = 16;
		m_stWFEX.wFormatTag=WAVE_FORMAT_PCM;
		m_stWFEX.nBlockAlign=m_stWFEX.nChannels*m_stWFEX.wBitsPerSample/8;
		m_stWFEX.nAvgBytesPerSec=m_stWFEX.nSamplesPerSec*m_stWFEX.nBlockAlign;
		m_stWFEX.cbSize=sizeof(WAVEFORMATEX);
		int device = 0;

		WAVEINCAPS stWIC={0};
		ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
		mRes=waveInGetDevCaps(device,&stWIC,sizeof(WAVEINCAPS));
		if(mRes==0)
			LOG->Trace( stWIC.szPname );
		else
			FAIL_M("bad");


		mRes=waveInOpen(&m_hWaveIn,device,&m_stWFEX,(DWORD_PTR)waveInProc,(DWORD_PTR)this,CALLBACK_FUNCTION);
		if(mRes!=MMSYSERR_NOERROR)
			FAIL_M("bad");



		const char *csT1 = "C:\\cvs\\stepmania\\speech-test.wav";
		ZeroMemory(&m_stmmIF,sizeof(MMIOINFO));
		DeleteFile((PCHAR)(LPCTSTR)csT1);
		m_hOPFile=mmioOpen((PCHAR)(LPCTSTR)csT1,&m_stmmIF,MMIO_WRITE | MMIO_CREATE);
		if(m_hOPFile==NULL)
			FAIL_M("Can not open file...");

		ZeroMemory(&m_stckOutRIFF,sizeof(MMCKINFO));
		m_stckOutRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E'); 
		mRes=mmioCreateChunk(m_hOPFile, &m_stckOutRIFF, MMIO_CREATERIFF);
		if(mRes!=MMSYSERR_NOERROR)
		{
			FAIL_M("bad");
		}
		ZeroMemory(&m_stckOut,sizeof(MMCKINFO));
		m_stckOut.ckid = mmioFOURCC('f', 'm', 't', ' ');
		m_stckOut.cksize = sizeof(m_stWFEX);
		mRes=mmioCreateChunk(m_hOPFile, &m_stckOut, 0);
		if(mRes!=MMSYSERR_NOERROR)
		{
			FAIL_M("bad");
		}
		int nT1=mmioWrite(m_hOPFile, (HPSTR) &m_stWFEX, sizeof(m_stWFEX));
		if(nT1!=sizeof(m_stWFEX))
		{
			FAIL_M("bad");
		}
		mRes=mmioAscend(m_hOPFile, &m_stckOut, 0);
		if(mRes!=MMSYSERR_NOERROR)
		{
			FAIL_M("bad");
		}
		m_stckOut.ckid = mmioFOURCC('d', 'a', 't', 'a');
		mRes=mmioCreateChunk(m_hOPFile, &m_stckOut, 0);
		if(mRes!=MMSYSERR_NOERROR)
		{
			FAIL_M("bad");
		}
	}

	void PrepareBuffers()
	{
		MMRESULT mRes=0;
		
		for(int nT1=0;nT1<MAX_BUFFERS;++nT1)
		{
			int iSampleCount = SAMPLES_PER_SEC / 60;	// 60 times per second
			int iBufferSizeBytes = iSampleCount*sizeof(short);
			m_stWHDR[nT1].lpData=(LPSTR)HeapAlloc(GetProcessHeap(),8,iBufferSizeBytes);
			m_stWHDR[nT1].dwBufferLength=iBufferSizeBytes;
			m_stWHDR[nT1].dwUser=nT1;
			mRes=waveInPrepareHeader(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
			if(mRes!=0)
				FAIL_M("bad");
			mRes=waveInAddBuffer(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
			if(mRes!=0)
				FAIL_M("bad");
		}
	}

	void ProcessHeader(WAVEHDR * pHdr)
	{
		//LOG->Trace("%d",pHdr->dwUser);
		if(WHDR_DONE==(WHDR_DONE &pHdr->dwFlags))
		{
			//LOG->Trace("Got %d bytes of data", pHdr->dwBytesRecorded );
			int iSampleCount = pHdr->dwBytesRecorded / sizeof(short);
			m_dp.ReadOne( (short*)pHdr->lpData, iSampleCount );

			mmioWrite(m_hOPFile,pHdr->lpData,pHdr->dwBytesRecorded);
			MMRESULT mRes = waveInAddBuffer(m_hWaveIn,pHdr,sizeof(WAVEHDR));
			if(mRes!=0)
				FAIL_M("bad");
		}
	}

	void CloseDevice()
	{
		MMRESULT mRes=0;
		
		if(m_hWaveIn)
		{
			UnPrepareBuffers();
			mRes=waveInClose(m_hWaveIn);
		}
		if(m_hOPFile)
		{
			mRes=mmioAscend(m_hOPFile, &m_stckOut, 0);
			if(mRes!=MMSYSERR_NOERROR)
			{
				FAIL_M("bad");
			}
			mRes=mmioAscend(m_hOPFile, &m_stckOutRIFF, 0);
			if(mRes!=MMSYSERR_NOERROR)
			{
				FAIL_M("bad");
			}
			mmioClose(m_hOPFile,0);
			m_hOPFile=NULL;
		}
		m_hWaveIn=NULL;
	}

	void UnPrepareBuffers()
	{
		MMRESULT mRes=0;

		if(m_hWaveIn)
		{
			mRes=waveInStop(m_hWaveIn);
			for(int nT1=0;nT1<3;++nT1)
			{
				if(m_stWHDR[nT1].lpData)
				{
					mRes=waveInUnprepareHeader(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
					HeapFree(GetProcessHeap(),0,m_stWHDR[nT1].lpData);
					ZeroMemory(&m_stWHDR[nT1],sizeof(WAVEHDR));
				}
			}
		}
	}
	
	void GetStatus( MicrophoneStatus &out )
	{
		return m_dp.GetStatus( out );
	}
};

MicrophoneImpl *MicrophoneImpl::s_pMicrophoneImpl = NULL;

void CALLBACK waveInProc(HWAVEIN hwi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	switch(uMsg)
	{
		case WIM_CLOSE:
			break;

		case WIM_DATA:
			{
				MicrophoneImpl::s_pMicrophoneImpl->ProcessHeader( (WAVEHDR *)dwParam1 );
			}
			break;

		case WIM_OPEN:
			break;

		default:
			break;
	}
}


void PitchDetectionTest::Update()
{
	s_pMicrophone->GetStatus( s_ms );
}

void DoPitchDetectionTest()
{
	MicrophoneImpl mr;
	mr.Init();
	mr.StartRecording();

	//srpd2();		// do f0 tracking
}



Microphone::Microphone()
{
	m_pImpl = new MicrophoneImpl;
	m_pImpl->Init();
	m_pImpl->StartRecording();
}
Microphone::~Microphone()
{
	m_pImpl->StopRecording();
	SAFE_DELETE( m_pImpl );
}

void Microphone::GetStatus( MicrophoneStatus &out )
{
	m_pImpl->GetStatus( out );
	if( !FreqToMidiNote(out.fFreq, out.fMidiNote) )
		out.fMidiNote = 0;
	int iMidiNote = (int)roundf( out.fMidiNote );
	if( !MidiNoteToStringAndOctave( iMidiNote, out.sMidiNote ) )
		out.sMidiNote = "";
}


/*
 * (c) 2007 Chris Danford
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
