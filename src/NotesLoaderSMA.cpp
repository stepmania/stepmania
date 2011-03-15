#include "global.h"
#include "NotesLoaderSMA.h"
#include "BackgroundUtil.h"
#include "GameManager.h"
#include "MsdFile.h"
#include "NoteTypes.h"
#include "NotesLoaderSM.h" // may need this.
#include "PrefsManager.h"
#include "RageFileManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "Song.h"
#include "SongManager.h"
#include "Steps.h"

/**
 * @brief A custom .edit file can only be so big before we have to reject it.
 */
const int MAX_EDIT_STEPS_SIZE_BYTES = 60*1024; // 60 KB

void SMALoader::LoadFromSMATokens(
				  RString sStepsType,
				  RString sDescription,
				  RString sDifficulty,
				  RString sMeter,
				  RString sMeterType,
				  vector<RString> attackData,
				  RString sRadarValues,
				  RString sNoteData,
				  RString sAttackData,
				  Steps &out
)
{
	// we're loading from disk, so this is by definition already saved:
	out.SetSavedToDisk( true );
	
	Trim( sStepsType );
	Trim( sDescription );
	Trim( sDifficulty );
	Trim( sNoteData );
	
	//	LOG->Trace( "Steps::LoadFromSMTokens()" );
	
	// insert stepstype hacks from GameManager.cpp here? -aj
	out.m_StepsType = GAMEMAN->StringToStepsType( sStepsType );
	out.SetDescription( sDescription );
	out.SetCredit( sDescription ); // this is often used for both.
	out.SetDifficulty( DwiCompatibleStringToDifficulty(sDifficulty) );
	
	sDescription.MakeLower();
	
	// Handle hacks that originated back when StepMania didn't have
	// Difficulty_Challenge. (At least v1.64, possibly v3.0 final...)
	if( out.GetDifficulty() == Difficulty_Hard )
	{
		// HACK: SMANIAC used to be Difficulty_Hard with a special description.
		if( sDescription == "smaniac" ) 
			out.SetDifficulty( Difficulty_Challenge );
		
		// HACK: CHALLENGE used to be Difficulty_Hard with a special description.
		if( sDescription == "challenge" ) 
			out.SetDifficulty( Difficulty_Challenge );
	}
	
	out.SetMeter( atoi(sMeter) );
	vector<RString> saValues;
	split( sRadarValues, ",", saValues, true );
	int categories = NUM_RadarCategory - 1; // Fakes aren't counted in the radar values.
	if( saValues.size() == (unsigned)categories * NUM_PLAYERS )
	{
		RadarValues v[NUM_PLAYERS];
		FOREACH_PlayerNumber( pn )
		{
			// Can't use the foreach anymore due to flexible radar lines.
			for( RadarCategory rc = (RadarCategory)0; rc < categories; 
			    enum_add<RadarCategory>( rc, 1 ) )
			{
				v[pn][rc] = StringToFloat( saValues[pn*categories + rc] );
			}
		}
		out.SetCachedRadarValues( v );
	}
	
	out.SetSMNoteData( sNoteData );
	
	out.TidyUpData();
}

void SMALoader::GetApplicableFiles( const RString &sPath, vector<RString> &out )
{
	GetDirListing( sPath + RString("*.sma"), out );
}

bool SMALoader::LoadTimingFromFile( const RString &fn, TimingData &out )
{
	MsdFile msd;
	if( !msd.ReadFile( fn, true ) )  // unescape
	{
		LOG->UserLog( "Song file", fn, "couldn't be loaded: %s", msd.GetError().c_str() );
		return false;
	}
	
	out.m_sFile = fn;
	LoadTimingFromSMAFile( msd, out );
	return true;
}

void SMALoader::LoadTimingFromSMAFile( const MsdFile &msd, TimingData &out )
{
	out.m_fBeat0OffsetInSeconds = 0;
	out.m_BPMSegments.clear();
	out.m_StopSegments.clear();
	out.m_WarpSegments.clear();
	out.m_vTimeSignatureSegments.clear();
	
	vector<WarpSegment> arrayWarpsFromNegativeBPMs;
	//vector<WarpSegment> arrayWarpsFromNegativeStops;
	int rowsPerMeasure = 0;
	
	for( unsigned i=0; i<msd.GetNumValues(); i++ )
	{
		const MsdFile::value_t &sParams = msd.GetValue(i);
		RString sValueName = sParams[0];
		sValueName.MakeUpper();
		
		
		if( sValueName=="OFFSET" )
		{
			out.m_fBeat0OffsetInSeconds = StringToFloat( sParams[1] );
		}
		else if( sValueName=="STOPS" || sValueName=="FREEZES" )
		{
			vector<RString> arrayFreezeExpressions;
			split( sParams[1], ",", arrayFreezeExpressions );
			
			for( unsigned f=0; f<arrayFreezeExpressions.size(); f++ )
			{
				vector<RString> arrayFreezeValues;
				split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
				if( arrayFreezeValues.size() != 2 )
				{
					// XXX: Hard to tell which file caused this.
					LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
						     sValueName.c_str(), arrayFreezeExpressions[f].c_str() );
					continue;
				}
				
				const float fFreezeBeat = StringToFloat( arrayFreezeValues[0] );
				const float fFreezeSeconds = StringToFloat( arrayFreezeValues[1] );
				StopSegment new_seg( BeatToNoteRow(fFreezeBeat), fFreezeSeconds );
				// XXX: Remove Negatives Bug?
				new_seg.m_iStartRow = BeatToNoteRow(fFreezeBeat);
				new_seg.m_fStopSeconds = fFreezeSeconds;
				
				if(fFreezeSeconds > 0.0f)
				{
					// LOG->Trace( "Adding a freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );
					out.AddStopSegment( new_seg );
				}
				else
				{
					// negative stops (hi JS!) -aj
					if( PREFSMAN->m_bQuirksMode )
					{
						// LOG->Trace( "Adding a negative freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );
						out.AddStopSegment( new_seg );
					}
					else
						LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid stop at beat %f, length %f.", fFreezeBeat, fFreezeSeconds );
				}
			}
		}
		else if( sValueName=="DELAYS" )
		{
			vector<RString> arrayDelayExpressions;
			split( sParams[1], ",", arrayDelayExpressions );
			
			for( unsigned f=0; f<arrayDelayExpressions.size(); f++ )
			{
				vector<RString> arrayDelayValues;
				split( arrayDelayExpressions[f], "=", arrayDelayValues );
				if( arrayDelayValues.size() != 2 )
				{
					// XXX: Hard to tell which file caused this.
					LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
						     sValueName.c_str(), arrayDelayExpressions[f].c_str() );
					continue;
				}
				
				const float fFreezeBeat = StringToFloat( arrayDelayValues[0] );
				const float fFreezeSeconds = StringToFloat( arrayDelayValues[1] );
				
				StopSegment new_seg( BeatToNoteRow(fFreezeBeat), fFreezeSeconds, true );
				// XXX: Remove Negatives Bug?
				new_seg.m_iStartRow = BeatToNoteRow(fFreezeBeat);
				new_seg.m_fStopSeconds = fFreezeSeconds;
				
				// LOG->Trace( "Adding a delay segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );
				
				if(fFreezeSeconds > 0.0f)
					out.AddStopSegment( new_seg );
				else
					LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid delay at beat %f, length %f.", fFreezeBeat, fFreezeSeconds );
			}
		}
		
		else if( sValueName=="BPMS" )
		{
			vector<RString> arrayBPMChangeExpressions;
			split( sParams[1], ",", arrayBPMChangeExpressions );
			
			for( unsigned b=0; b<arrayBPMChangeExpressions.size(); b++ )
			{
				vector<RString> arrayBPMChangeValues;
				split( arrayBPMChangeExpressions[b], "=", arrayBPMChangeValues );
				// XXX: Hard to tell which file caused this.
				if( arrayBPMChangeValues.size() != 2 )
				{
					LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
						     sValueName.c_str(), arrayBPMChangeExpressions[b].c_str() );
					continue;
				}
				
				const float fBeat = StringToFloat( arrayBPMChangeValues[0] );
				const float fNewBPM = StringToFloat( arrayBPMChangeValues[1] );
				// XXX: Remove Negatives Bug?
				BPMSegment new_seg;
				new_seg.m_iStartRow = BeatToNoteRow(fBeat);
				new_seg.SetBPM( fNewBPM );
				
				// convert negative BPMs into Warp segments
				if( fNewBPM < 0.0f )
				{
					vector<RString> arrayNextBPMChangeValues;
					// get next bpm in sequence
					if((b+1) < arrayBPMChangeExpressions.size())
					{
						split( arrayBPMChangeExpressions[b+1], "=", arrayNextBPMChangeValues );
						const float fNextPositiveBeat = StringToFloat( arrayNextBPMChangeValues[0] );
						const float fNextPositiveBPM  = StringToFloat( arrayNextBPMChangeValues[1] );
						
						// tJumpPos = (tPosBPS-abs(negBPS)) + (gPosBPMPosition - fNegPosition)
						float fDeltaBeat = ((fNextPositiveBPM/60.0f)-abs(fNewBPM/60.0f)) + (fNextPositiveBeat-fBeat);
						//float fWarpLengthBeats = fNextPositiveBeat + fDeltaBeat;
						WarpSegment wsTemp(BeatToNoteRow(fBeat),fDeltaBeat);
						arrayWarpsFromNegativeBPMs.push_back(wsTemp);
						
						/*
						 LOG->Trace( ssprintf("==NotesLoSM negbpm==\nfnextposbeat = %f, fnextposbpm = %f,\nfdelta = %f, fwarpto = %f",
						 fNextPositiveBeat,
						 fNextPositiveBPM,
						 fDeltaBeat,
						 fWarpToBeat
						 ) );
						 */
						/*
						 LOG->Trace( ssprintf("==Negative/Subtractive BPM in NotesLoader==\nNegBPM has noterow = %i, BPM = %f\nNextBPM @ noterow %i\nDelta value = %i noterows\nThis warp will have us end up at noterow %i",
						 BeatToNoteRow(fBeat), fNewBPM,
						 BeatToNoteRow(fNextPositiveBeat),
						 BeatToNoteRow(fDeltaBeat),
						 BeatToNoteRow(fWarpToBeat))
						 );
						 */
						//float fDeltaBeat = ((fNextPositiveBPM/60.0f)-abs(fNewBPM/60.0f)) + (fNextPositiveBeat-fBeat);
						/*
						 LOG->Trace( ssprintf("==NotesLoader Delta as NoteRows==\nfDeltaBeat = %f (beat)\nfDeltaBeat = (NextBPMSeg %f - abs(fBPS %f)) + (nextStartRow %i - thisRow %i)",
						 fDeltaBeat,(fNextPositiveBPM/60.0f),abs(fNewBPM/60.0f),BeatToNoteRow(fNextPositiveBeat),BeatToNoteRow(fBeat))
						 );
						 */
						
						out.AddBPMSegment( new_seg );
						
						continue;
					}
					else
					{
						// last BPM is a negative one? ugh. -aj (MAX_NOTE_ROW exists btw)
						out.AddBPMSegment( new_seg );
					}
				}
				
				if(fNewBPM > 0.0f)
					out.AddBPMSegment( new_seg );
				else
				{
					out.m_bHasNegativeBpms = true;
					// only add Negative BPMs in quirks mode -aj
					if( PREFSMAN->m_bQuirksMode )
						out.AddBPMSegment( new_seg );
					else
						LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid BPM change at beat %f, BPM %f.", fBeat, fNewBPM );
				}
			}
		}
		
		else if( sValueName=="TIMESIGNATURES" )
		{
			vector<RString> vs1;
			split( sParams[1], ",", vs1 );
			
			FOREACH_CONST( RString, vs1, s1 )
			{
				vector<RString> vs2;
				split( *s1, "=", vs2 );
				
				if( vs2.size() < 3 )
				{
					LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid time signature change with %i values.", (int)vs2.size() );
					continue;
				}
				
				const float fBeat = StringToFloat( vs2[0] );
				
				TimeSignatureSegment seg;
				seg.m_iStartRow = BeatToNoteRow(fBeat);
				seg.m_iNumerator = atoi( vs2[1] ); 
				seg.m_iDenominator = atoi( vs2[2] ); 
				
				if( fBeat < 0 )
				{
					LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid time signature change with beat %f.", fBeat );
					continue;
				}
				
				if( seg.m_iNumerator < 1 )
				{
					LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid time signature change with beat %f, iNumerator %i.", fBeat, seg.m_iNumerator );
					continue;
				}
				
				if( seg.m_iDenominator < 1 )
				{
					LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid time signature change with beat %f, iDenominator %i.", fBeat, seg.m_iDenominator );
					continue;
				}
				
				out.AddTimeSignatureSegment( seg );
			}
		}
		
		else if( sValueName=="TICKCOUNTS" )
		{
			vector<RString> arrayTickcountExpressions;
			split( sParams[1], ",", arrayTickcountExpressions );
			
			for( unsigned f=0; f<arrayTickcountExpressions.size(); f++ )
			{
				vector<RString> arrayTickcountValues;
				split( arrayTickcountExpressions[f], "=", arrayTickcountValues );
				if( arrayTickcountValues.size() != 2 )
				{
					// XXX: Hard to tell which file caused this.
					LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
						     sValueName.c_str(), arrayTickcountExpressions[f].c_str() );
					continue;
				}
				
				const float fTickcountBeat = StringToFloat( arrayTickcountValues[0] );
				int iTicks = atoi( arrayTickcountValues[1] );
				// you're lazy, let SM do the work for you... -DaisuMaster
				if( iTicks < 1) iTicks = 1;
				if( iTicks > ROWS_PER_BEAT ) iTicks = ROWS_PER_BEAT;
				
				TickcountSegment new_seg( BeatToNoteRow(fTickcountBeat), iTicks );
				out.AddTickcountSegment( new_seg );
				
				if(iTicks >= 1 && iTicks <= ROWS_PER_BEAT ) // Constants
				{
					// LOG->Trace( "Adding a tickcount segment: beat: %f, ticks = %d", fTickcountBeat, iTicks );
					//out.AddTickcountSegment( new_seg );
				}
				else
				{
					//LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid tickcount at beat %f, ticks %d.", fTickcountBeat, iTicks );
					//LOG->UserLog( "Song file", "(UNKNOWN)", "Clamping tickcount value to %d at beat %f.", iTicks, fTickcountBeat);
					//etc
				}
			}
		}
		
		// warps (replacement for Negative BPM and Negative Stops)
		/*
		 else if( sValueName=="WARPS" )
		 {
		 vector<RString> arrayWarpExpressions;
		 split( sParams[1], ",", arrayWarpExpressions );
		 
		 for( unsigned f=0; f<arrayWarpExpressions.size(); f++ )
		 {
		 vector<RString> arrayWarpValues;
		 split( arrayWarpExpressions[f], "=", arrayWarpValues );
		 if( arrayWarpValues.size() != 2 )
		 {
		 // XXX: Hard to tell which file caused this.
		 LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid #%s value \"%s\" (must have exactly one '='), ignored.",
		 sValueName.c_str(), arrayWarpExpressions[f].c_str() );
		 continue;
		 }
		 
		 const float fWarpStart = StringToFloat( arrayWarpValues[0] );
		 const float fWarpBeats = StringToFloat( arrayWarpValues[1] );
		 
		 if( fWarpStart > 0.0f && fWarpBeats > 0.0f )
		 {
		 WarpSegment new_seg( BeatToNoteRow(fWarpStart), fWarpBeats );
		 out.AddWarpSegment( new_seg );
		 }
		 else
		 {
		 // Currently disallow negative warps, to prevent the same
		 // kind of problem that happened when Negative/Subtractive
		 // BPMs arrived on the StepMania scene. -aj
		 LOG->UserLog( "Song file", "(UNKNOWN)", "has an invalid warp at beat %f lasting %f beats.", fWarpStart, fWarpBeats );
		 }
		 }
		 }
		 */
		
		// Note: Even though it is possible to have Negative BPMs and Stops in
		// a song along with Warps, we should not support files that contain
		// both styles of warp tricks (Negatives vs. #WARPS).
		// If Warps have been populated from Negative BPMs, then go through that
		// instead of using the data in the Warps tag. This should be above,
		// but it breaks compiling so...
		if(arrayWarpsFromNegativeBPMs.size() > 0)
		{
			// zomg we already have some warps...
			for( unsigned j=0; j<arrayWarpsFromNegativeBPMs.size(); j++ )
			{
				out.AddWarpSegment( arrayWarpsFromNegativeBPMs[j] );
			}
		}
		// warp sorting will need to take place.
		//sort(out.m_WarpSegments.begin(), out.m_WarpSegments.end());
	}
}

bool SMALoader::LoadFromBGChangesString( BackgroundChange &change, 
			     const RString &sBGChangeExpression )
{
	return SMLoader::LoadFromBGChangesString(change, sBGChangeExpression);
}

bool SMALoader::LoadFromDir( const RString &sPath, Song &out )
{
	vector<RString> aFileNames;
	GetApplicableFiles( sPath, aFileNames );
	
	if( aFileNames.size() > 1 )
	{
		LOG->UserLog( "Song", sPath, "has more than one SMA file. There can be only one!" );
		return false;
	}
	ASSERT( aFileNames.size() == 1 );
	return LoadFromSMAFile( sPath + aFileNames[0], out );
}

/**
 * @file
 * @author Aldo Fregoso, Jason Felds (c) 2009-2011
 * @section LICENSE
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
