/* RageSoundReader_SpeedChange - change the speed of an audio stream without changing its pitch. */

#ifndef RAGE_SOUND_READER_SPEED_CHANGE_H
#define RAGE_SOUND_READER_SPEED_CHANGE_H

#include "RageSoundReader_Filter.h"

class RageSoundReader_SpeedChange: public RageSoundReader_Filter
{
public:
	RageSoundReader_SpeedChange( RageSoundReader *pSource );

	virtual int SetPosition( int iFrame );
	virtual int Read( int16_t *pBuf, int iFrames );
	virtual RageSoundReader_SpeedChange *Copy() const { return new RageSoundReader_SpeedChange(*this); }
	virtual bool SetProperty( const RString &sProperty, float fValue );
	virtual int GetNextSourceFrame() const;
	virtual float GetStreamToSourceRatio() const;

	void SetSpeedRatio( float fRatio );

	/* Return true if the next Read() will start a new block, allowing GetRatio() to
	 * be updated to a new value.  Used by RageSoundReader_PitchChange. */
	bool NextReadWillStep() const { return GetCursorAvail() == 0; }

	/* Get the ratio last set by SetSpeedRatio. */
	float GetRatio() const { return m_fSpeedRatio; }

protected:
	int FillData( int iMax );
	void EraseData( int iToDelete );
	int Step();
	void Reset();

	int GetCursorAvail() const;

	int GetWindowSizeFrames() const;
	int GetToleranceFrames() const { return GetWindowSizeFrames() / 4; }

	int m_iDataBufferAvailFrames;
	struct ChannelInfo
	{
		vector<int16_t> m_DataBuffer;
		int m_iCorrelatedPos;
		int m_iLastCorrelatedPos;
	};
	vector<ChannelInfo> m_Channels;

	int m_iUncorrelatedPos;
	int m_iPos;

	float m_fSpeedRatio;
	float m_fTrailingSpeedRatio;
	float m_fErrorFrames;
};

#endif

/*
 * Copyright (c) 2006 Glenn Maynard
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
