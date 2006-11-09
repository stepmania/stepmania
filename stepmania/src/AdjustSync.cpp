/*
 * AdjustSync defines two methods for fixing the sync.  
 *
 * The first method adjusts either the song or the machine by the 
 * average offset of the user's steps.  In other words, if the user
 * averages to step early by 10 ms, either the song or the global
 * offset is adjusted by 10 ms to compensate for that.  These 
 * adjustments only require a small set of data, so this method
 * updates the offset while the song is playing.
 *
 * The second method adjusts both the offset and the tempo of an
 * individual song.  It records all of the steps during a play of
 * the song and uses linear least squares regression to minimize the
 * error of those steps.  It makes one adjustment for the tempo of 
 * the entire song, rather than adding many different tempo segments
 * to match the steps.  If there are already several tempo segments
 * in the stepfile, this method makes a proportional change to each
 * of them.  For example, if it changes 100 bpm to 101 bpm, it will
 * also change 200 bpm to 202 bpm.  This method also adjusts the stops.
 * It assumes that a given stop is measured in terms of beats and makes
 * the appropriate change.
 * 
 * If we use this method on a small set of data late in the song, it 
 * can have very chaotic effects on the early settings.  For example,
 * it may change the offset by several hundred milliseconds and make a
 * large change to the BPM to compensate if that would minimize the
 * error.  This problem occurs especially when the user makes a couple
 * steps that are significantly off beat.  The way to avoid this is to
 * perform the least squares regression once on all of the data
 * collected, rather than adjusting the sync every time we get another
 * 50 or so data points.  In fact, if we are playing in edit mode and
 * the user loops through the song more than once, we use all of the 
 * steps made.
 */

#include "global.h"
#include "song.h"
#include "AdjustSync.h"
#include "GameState.h"
#include "LocalizedString.h"
#include "PrefsManager.h"
#include "ScreenManager.h"

TimingData *AdjustSync::s_pTimingDataOriginal = NULL;
float AdjustSync::s_fGlobalOffsetSecondsOriginal = 0.0f;
int AdjustSync::s_iAutosyncOffsetSample = 0;
float AdjustSync::s_fAutosyncOffset[AdjustSync::OFFSET_SAMPLE_COUNT];
float AdjustSync::s_fStandardDeviation = 0.0f;
vector< pair<float, float> > AdjustSync::s_vAutosyncTempoData;
float AdjustSync::s_fAverageError = 0.0f;
const float AdjustSync::ERROR_TOO_HIGH = 0.025f;
int AdjustSync::s_iStepsFiltered = 0;

void AdjustSync::ResetOriginalSyncData()
{
	if( s_pTimingDataOriginal == NULL )
		s_pTimingDataOriginal = new TimingData;

	if( GAMESTATE->m_pCurSong )
		*s_pTimingDataOriginal = GAMESTATE->m_pCurSong->m_Timing;
	else
		*s_pTimingDataOriginal = TimingData();
	s_fGlobalOffsetSecondsOriginal = PREFSMAN->m_fGlobalOffsetSeconds;

	ResetAutosync();
}

void AdjustSync::ResetAutosync()
{
	s_iAutosyncOffsetSample = 0;
	s_vAutosyncTempoData.clear();
}

bool AdjustSync::IsSyncDataChanged()
{
	// Can't sync in course modes
	if( GAMESTATE->IsCourseMode() )
		return false;
	vector<RString> vs;
	AdjustSync::GetSyncChangeTextGlobal( vs );
	AdjustSync::GetSyncChangeTextSong( vs );
	return !vs.empty();
}

void AdjustSync::SaveSyncChanges()
{
	if( GAMESTATE->IsCourseMode() )
		return;
	if( GAMESTATE->m_pCurSong && *s_pTimingDataOriginal != GAMESTATE->m_pCurSong->m_Timing )
	{
		if( GAMESTATE->IsEditing() )
		{
			MESSAGEMAN->Broadcast( Message_SongModified );
		}
		else
		{
			GAMESTATE->m_pCurSong->Save();
		}
	}
	if( s_fGlobalOffsetSecondsOriginal != PREFSMAN->m_fGlobalOffsetSeconds )
		PREFSMAN->SavePrefsToDisk();
	ResetOriginalSyncData();
	s_fStandardDeviation = 0.0f;
	s_fAverageError = 0.0f;
}

void AdjustSync::RevertSyncChanges()
{
	if( GAMESTATE->IsCourseMode() )
		return;
	PREFSMAN->m_fGlobalOffsetSeconds.Set( s_fGlobalOffsetSecondsOriginal );
	GAMESTATE->m_pCurSong->m_Timing = *s_pTimingDataOriginal;
	ResetOriginalSyncData();
	s_fStandardDeviation = 0.0f;
	s_fAverageError = 0.0f;
}

static LocalizedString AUTOSYNC_CORRECTION_APPLIED	( "AdjustSync", "Autosync: Correction applied." );
static LocalizedString AUTOSYNC_CORRECTION_NOT_APPLIED	( "AdjustSync", "Autosync: Correction NOT applied. Deviation too high." );
static LocalizedString AUTOSYNC_SONG			( "AdjustSync", "Autosync Song" );
static LocalizedString AUTOSYNC_MACHINE			( "AdjustSync", "Autosync Machine" );
static LocalizedString AUTOSYNC_TEMPO			( "AdjustSync", "Autosync Tempo" );
void AdjustSync::HandleAutosync( float fNoteOffBySeconds, float fStepTime )
{
	if( GAMESTATE->IsCourseMode() )
		return;
	switch( GAMESTATE->m_SongOptions.GetCurrent().m_AutosyncType ) {
	case SongOptions::AUTOSYNC_OFF:
		return;
	case SongOptions::AUTOSYNC_TEMPO:
	{
		// We collect all of the data and process it at the end
		s_vAutosyncTempoData.push_back( make_pair(fStepTime, fNoteOffBySeconds) );
		break;
	}
	case SongOptions::AUTOSYNC_MACHINE:
	case SongOptions::AUTOSYNC_SONG:
	{
		s_fAutosyncOffset[s_iAutosyncOffsetSample] = fNoteOffBySeconds;
		++s_iAutosyncOffsetSample;

		if( s_iAutosyncOffsetSample < OFFSET_SAMPLE_COUNT ) 
			break; // need more

		AutosyncOffset();
		break;
 	}
	default:
		ASSERT(0);
	}
}

void AdjustSync::HandleSongEnd()
{
	if( GAMESTATE->IsCourseMode() )
		return;
	if( GAMESTATE->m_SongOptions.GetCurrent().m_AutosyncType == SongOptions::AUTOSYNC_TEMPO )
	{
		AutosyncTempo();
	}

	// all other states don't care
}

void AdjustSync::AutosyncOffset()
{
	const float mean = calc_mean( s_fAutosyncOffset, s_fAutosyncOffset+OFFSET_SAMPLE_COUNT );
	const float stddev = calc_stddev( s_fAutosyncOffset, s_fAutosyncOffset+OFFSET_SAMPLE_COUNT );

	RString sAutosyncType;
	switch( GAMESTATE->m_SongOptions.GetCurrent().m_AutosyncType )
	{
	case SongOptions::AUTOSYNC_SONG:
		sAutosyncType = AUTOSYNC_SONG;
		break;
	case SongOptions::AUTOSYNC_MACHINE:
		sAutosyncType = AUTOSYNC_MACHINE;
		break;
	default:
		ASSERT(0);
	}

	if( stddev < .03f )  // If they stepped with less than .03 error
	{
		switch( GAMESTATE->m_SongOptions.GetCurrent().m_AutosyncType )
		{
		case SongOptions::AUTOSYNC_SONG:
			GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds += mean;
			break;
		case SongOptions::AUTOSYNC_MACHINE:
			PREFSMAN->m_fGlobalOffsetSeconds.Set( PREFSMAN->m_fGlobalOffsetSeconds + mean );
			break;
		default:
			ASSERT(0);
		}

		SCREENMAN->SystemMessage( AUTOSYNC_CORRECTION_APPLIED.GetValue() );
	}
	else
	{
		SCREENMAN->SystemMessage( AUTOSYNC_CORRECTION_NOT_APPLIED.GetValue() );
	}

	s_iAutosyncOffsetSample = 0;
	s_fStandardDeviation = stddev;
}

void AdjustSync::AutosyncTempo()
{
	float fSlope = 0.0f;
	float fIntercept = 0.0f;
	if( !CalcLeastSquares( s_vAutosyncTempoData,
	                       &fSlope, &fIntercept, &s_fAverageError ) )
	{
		s_vAutosyncTempoData.clear();
		return;
	}

	if( s_fAverageError < ERROR_TOO_HIGH )
	{
		// Here we filter out any steps that are too far off.
		//
		// If it turns out that we want to be even more selective, we can
		// keep only a fraction of the data, such as the 80% with the lowest
		// error.  However, throwing away the ones with high error should
		// be enough in most cases.
		float fFilteredError = 0.0;
		s_iStepsFiltered = s_vAutosyncTempoData.size();
		FilterHighErrorPoints( &s_vAutosyncTempoData,
		                       fSlope, fIntercept, ERROR_TOO_HIGH );
		s_iStepsFiltered -= s_vAutosyncTempoData.size();

		if( !CalcLeastSquares( s_vAutosyncTempoData,
		                       &fSlope, &fIntercept, &fFilteredError ) )
			return;

		GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds += fIntercept;

		vector<BPMSegment>::iterator itBPM;
		for( itBPM = GAMESTATE->m_pCurSong->m_Timing.m_BPMSegments.begin();
		     itBPM != GAMESTATE->m_pCurSong->m_Timing.m_BPMSegments.end();
		     ++itBPM )
		{
			itBPM->SetBPM( 60.0f / ((60.0f / itBPM->GetBPM()) * (1.0f - fSlope)) );
		}

		// We assume that the stops were measured as a number of beats.
		// Therefore, if we change the bpms, we need to make a similar
		// change to the stops.
		vector<StopSegment>::iterator itStop;
		for( itStop = GAMESTATE->m_pCurSong->m_Timing.m_StopSegments.begin();
		     itStop != GAMESTATE->m_pCurSong->m_Timing.m_StopSegments.end();
		     ++itStop )
		{
			itStop->m_fStopSeconds = itStop->m_fStopSeconds * ( 1.0f - fSlope );
		}

		SCREENMAN->SystemMessage( AUTOSYNC_CORRECTION_APPLIED.GetValue() );
	}
	else
	{
		// deviation... error... close enough for an error message
		SCREENMAN->SystemMessage( AUTOSYNC_CORRECTION_NOT_APPLIED.GetValue() );
	}

	s_vAutosyncTempoData.clear();
}


static LocalizedString EARLIER			("AdjustSync","earlier");
static LocalizedString LATER			("AdjustSync","later");
static LocalizedString GLOBAL_OFFSET_FROM	( "AdjustSync", "Global Offset from %+.3f to %+.3f (notes %s)" );
// We need to limit the length of lines so each one fits on one line of the SM console.
// The tempo and stop change message can get very long in a complicated song, and at
// a low resolution, the keep/revert menu would be pushed off the bottom of the screen
// if we didn't limit the length of the message.  Keeping the lines short lets us fit 
// more information on the screen.
static LocalizedString SONG_OFFSET_FROM		( "AdjustSync", "Song offset from %+.3f to %+.3f (notes %s)" );
static LocalizedString TEMPO_SEGMENT_FROM	( "AdjustSync", "%s BPM from %.3f BPM to %.3f BPM." );
static LocalizedString CHANGED_STOP		("AdjustSync","The stop segment #%d changed from %+.3fs to %+.3fs (change of %+.3f).");
static LocalizedString ERROR			("AdjustSync", "Average Error %.5fs");
static LocalizedString ETC              ("AdjustSync", "Etc.");
static LocalizedString TAPS_IGNORED	("AdjustSync", "%d taps ignored.");

void AdjustSync::GetSyncChangeTextGlobal( vector<RString> &vsAddTo )
{
	{
		float fOld = Quantize( AdjustSync::s_fGlobalOffsetSecondsOriginal, 0.001f );
		float fNew = Quantize( PREFSMAN->m_fGlobalOffsetSeconds, 0.001f ) ;
		float fDelta = fNew - fOld;

		if( fabsf(fDelta) > 0.00001f )
		{
			vsAddTo.push_back( ssprintf( 
				GLOBAL_OFFSET_FROM.GetValue(),
				fOld, 
				fNew,
				(fDelta > 0 ? EARLIER:LATER).GetValue().c_str() ) );
		}
	}
}

void AdjustSync::GetSyncChangeTextSong( vector<RString> &vsAddTo )
{
	if( GAMESTATE->m_pCurSong.Get() )
	{
		unsigned int iOriginalSize = vsAddTo.size();

		{
			float fOld = Quantize( AdjustSync::s_pTimingDataOriginal->m_fBeat0OffsetInSeconds, 0.001f );
			float fNew = Quantize( GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds, 0.001f );
			float fDelta = fNew - fOld;

			if( fabsf(fDelta) > 0.00001f )
			{
				vsAddTo.push_back( ssprintf( 
					SONG_OFFSET_FROM.GetValue(),
					fOld, 
					fNew,
					(fDelta > 0 ? EARLIER:LATER).GetValue().c_str() ) );
			}
		}

		for( unsigned i=0; i<GAMESTATE->m_pCurSong->m_Timing.m_BPMSegments.size(); i++ )
		{
			float fOld = Quantize( AdjustSync::s_pTimingDataOriginal->m_BPMSegments[i].GetBPM(), 0.001f );
			float fNew = Quantize( GAMESTATE->m_pCurSong->m_Timing.m_BPMSegments[i].GetBPM(), 0.001f );
			float fDelta = fNew - fOld;

			if( fabsf(fDelta) > 0.00001f )
			{
				if ( i >= 4 ) 
				{
					vsAddTo.push_back(ETC.GetValue());
					break;
				}
				vsAddTo.push_back( ssprintf( 
					TEMPO_SEGMENT_FROM.GetValue(),
					FormatNumberAndSuffix(i+1).c_str(),
					fOld, 
					fNew ) );
			}
		}

		for( unsigned i=0; i<GAMESTATE->m_pCurSong->m_Timing.m_StopSegments.size(); i++ )
		{
			float fOld = Quantize( AdjustSync::s_pTimingDataOriginal->m_StopSegments[i].m_fStopSeconds, 0.001f );
			float fNew = Quantize( GAMESTATE->m_pCurSong->m_Timing.m_StopSegments[i].m_fStopSeconds, 0.001f );
			float fDelta = fNew - fOld;

			if( fabsf(fDelta) > 0.00001f )
			{
				if ( i >= 4 )
				{
					vsAddTo.push_back(ETC.GetValue());
					break;
				}
				vsAddTo.push_back( ssprintf(
					CHANGED_STOP.GetValue(),
					i+1,
					fOld, 
					fNew ) );
			}
		}

		if( vsAddTo.size() > iOriginalSize && s_fAverageError > 0.0f )
		{
			vsAddTo.push_back( ssprintf(ERROR.GetValue(), s_fAverageError) );
		}
		if( vsAddTo.size() > iOriginalSize && s_iStepsFiltered > 0 )
		{
			vsAddTo.push_back( ssprintf(TAPS_IGNORED.GetValue(), s_iStepsFiltered) );
		}
	}
}

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
