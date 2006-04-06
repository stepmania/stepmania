#ifndef AdjustSync_H
#define AdjustSync_H

class TimingData;

const int SAMPLE_COUNT = 56;

class AdjustSync
{
public:
	static TimingData *s_pTimingDataOriginal;
	static float s_fGlobalOffsetSecondsOriginal;
	static void ResetOriginalSyncData();
	static bool IsSyncDataChanged();
	static void SaveSyncChanges();
	static void RevertSyncChanges();
	static void HandleAutosync( float fNoteOffBySeconds );
	static void GetSyncChangeTextGlobal( vector<RString> &vsAddTo );
	static void GetSyncChangeTextSong( vector<RString> &vsAddTo );

	static float s_fAutosyncOffset[SAMPLE_COUNT];
	static int s_iAutosyncOffsetSample;
	static float s_fStandardDeviation;
};

#endif

/*
 * (c) 2003-2004 Chris Danford
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
