#ifndef AdjustSync_H
#define AdjustSync_H

/*
 * This class defines two ways of adjusting the sync of a song.
 * The first adjusts only the offset, "on the fly".  It can adjust
 * either the song itself or the machine.  The other style adjusts
 * both the BPM and the offset of the song, but it needs more data.
 */

class TimingData;

class AdjustSync
{
public:
	static TimingData *s_pTimingDataOriginal;
	static float s_fGlobalOffsetSecondsOriginal;
	/* 
	 * We only want to call the Reset methods before a song, not immediately after a song.
	 * If we reset it at the end of a song, we have to carefully check the logic to make
	 * sure we never reset it before the user gets a chance to save or revert the change.
	 * Resetting at the start of the song is sufficient.
	 */
	static void ResetOriginalSyncData();
	static void ResetAutosync();
	static bool IsSyncDataChanged();
	
	static void SaveSyncChanges();
	static void RevertSyncChanges();
	static void HandleAutosync( float fNoteOffBySeconds, float fStepTime );
	static void HandleSongEnd();
	static void AutosyncOffset();
	static void AutosyncTempo();
	static void GetSyncChangeTextGlobal( vector<RString> &vsAddTo );
	static void GetSyncChangeTextSong( vector<RString> &vsAddTo );

	static const int OFFSET_SAMPLE_COUNT = 56;

	static float s_fAutosyncOffset[OFFSET_SAMPLE_COUNT];
	static int s_iAutosyncOffsetSample;
	static float s_fStandardDeviation;

	// Measured in seconds.  If the average error is too high, we 
	// reject the recorded data for the Least Squares Regression.
	static const float ERROR_TOO_HIGH;

	static vector< pair<float, float> > s_vAutosyncTempoData;
	static float s_fAverageError;
	static int s_iStepsFiltered;
};

#endif

/*
 * (c) 2003-2006 Chris Danford, John Bauer
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
