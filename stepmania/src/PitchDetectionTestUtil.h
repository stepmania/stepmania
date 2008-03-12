/* PitchDetectionTestUtil -  */

#ifndef PitchDetectionTestUtil_H
#define PitchDetectionTestUtil_H

enum EST_bo_t {bo_big, bo_little, bo_perq};

static int est_endian_loc = 1;
/* Sun, HP, SGI Mips, M68000 */
#define EST_BIG_ENDIAN (((char *)&est_endian_loc)[0] == 0)
/* Intel, Alpha, DEC Mips, Vax */
#define EST_LITTLE_ENDIAN (((char *)&est_endian_loc)[0] != 0)
#define EST_NATIVE_BO (EST_BIG_ENDIAN ? bo_big : bo_little)
#define EST_SWAPPED_BO (EST_BIG_ENDIAN ? bo_little : bo_big)


#define SAMPLES_PER_SEC 44100

struct Srpd_Op;
struct SEGMENT_;
struct CROSS_CORR_;
struct STATUS_;
class RageSoundReader_FileReader;
struct MicrophoneStatus;

class DetectPitch
{
	Srpd_Op *m_pSrpdOp;
	SEGMENT_ *m_pSegment;
	int m_iSamplesFilledInSegment;	// how many samples in m_pSegment are filled
	CROSS_CORR_ *m_pCC;
	STATUS_ *m_pPdaStatus;

public:
	DetectPitch();
	~DetectPitch();

	void Init(RageSoundReader_FileReader *sample);
	void Init(int iSampleFreq);
	int ReadOne(RageSoundReader_FileReader *sample);
	int ReadOne(short *pData, int iCount);
	void GetStatus( MicrophoneStatus &out );
	void End();
};





#endif

/*
 * (c) 2004 Chris Danford
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
