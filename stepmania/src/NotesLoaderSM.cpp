#include "stdafx.h"

#include "NotesLoaderSM.h"
#include "GameManager.h"
#include "RageException.h"
#include "MsdFile.h"
#include "RageLog.h"
#include "RageUtil.h"

void SMLoader::LoadFromSMTokens( 
	CString sNotesType, 
	CString sDescription,
	CString sDifficultyClass,
	CString sMeter,
	CString sRadarValues,
	CString sNoteData,
	Notes &out
)
{
	sNotesType.TrimLeft(); 
	sNotesType.TrimRight(); 
	sDescription.TrimLeft(); 
	sDescription.TrimRight(); 
	sDifficultyClass.TrimLeft(); 
	sDifficultyClass.TrimRight(); 


//	LOG->Trace( "Notes::LoadFromSMTokens()" );

	out.m_NotesType = GameManager::StringToNotesType(sNotesType);
	out.m_sDescription = sDescription;
	out.m_DifficultyClass = StringToDifficultyClass( sDifficultyClass );
	out.m_iMeter = atoi(sMeter);
	CStringArray saValues;
	split( sRadarValues, ",", saValues, true );
	if( saValues.GetSize() == NUM_RADAR_VALUES )
		for( int r=0; r<NUM_RADAR_VALUES; r++ )
			out.m_fRadarValues[r] = (float)atof(saValues[r]);
    
	out.m_sSMNoteData = sNoteData;

	out.TidyUpData();
}


bool SMLoader::LoadFromSMFile( CString sPath, Song &out )
{
	LOG->Trace( "Song::LoadFromSMDir(%s)", sPath );

	out.m_BPMSegments.RemoveAll();
	out.m_StopSegments.RemoveAll();

	int i;

	MsdFile msd;
	bool bResult = msd.ReadFile( sPath );
	if( !bResult )
		throw RageException( "Error opening file '%s'.", sPath );

	for( i=0; i<msd.m_iNumValues; i++ )
	{
		int iNumParams = msd.m_iNumParams[i];
		CString* sParams = msd.m_sValuesAndParams[i];
		CString sValueName = sParams[0];

		// handle the data
		if( 0==stricmp(sValueName,"TITLE") )
			out.GetMainAndSubTitlesFromFullTitle( sParams[1], out.m_sMainTitle, out.m_sSubTitle );

		else if( 0==stricmp(sValueName,"SUBTITLE") )
			out.m_sSubTitle = sParams[1];

		else if( 0==stricmp(sValueName,"ARTIST") )
			out.m_sArtist = sParams[1];

		else if( 0==stricmp(sValueName,"CREDIT") )
			out.m_sCredit = sParams[1];

		else if( 0==stricmp(sValueName,"BANNER") )
			out.m_sBannerFile = sParams[1];

		else if( 0==stricmp(sValueName,"BACKGROUND") )
			out.m_sBackgroundFile = sParams[1];

		else if( 0==stricmp(sValueName,"CDTITLE") )
			out.m_sCDTitleFile = sParams[1];

		else if( 0==stricmp(sValueName,"MOVIEBACKGROUND") )
			out.m_sMovieBackgroundFile = sParams[1];

		else if( 0==stricmp(sValueName,"MUSIC") )
			out.m_sMusicFile = sParams[1];

		else if( 0==stricmp(sValueName,"MUSICBYTES") )
			out.m_iMusicBytes = atoi( sParams[1] );

		else if( 0==stricmp(sValueName,"MUSICLENGTH") )
			out.m_fMusicLengthSeconds = (float)atof( sParams[1] );

		else if( 0==stricmp(sValueName,"FIRSTBEAT") )
			out.m_fFirstBeat = (float)atof( sParams[1] );

		else if( 0==stricmp(sValueName,"LASTBEAT") )
			out.m_fLastBeat = (float)atof( sParams[1] );

		else if( 0==stricmp(sValueName,"SAMPLESTART") )
			out.m_fMusicSampleStartSeconds = TimeToSeconds( sParams[1] );

		else if( 0==stricmp(sValueName,"SAMPLELENGTH") )
			out.m_fMusicSampleLengthSeconds = TimeToSeconds( sParams[1] );

		else if( 0==stricmp(sValueName,"OFFSET") )
			out.m_fBeat0OffsetInSeconds = (float)atof( sParams[1] );

		else if( 0==stricmp(sValueName,"SELECTABLE") )
		{
			if(!stricmp(sParams[1],"YES"))
				out.m_SelectionDisplay = out.SHOW_ALWAYS;
			else if(!stricmp(sParams[1],"NO"))
				out.m_SelectionDisplay = out.SHOW_NEVER;
			else if(!stricmp(sParams[1],"ROULETTE"))
				out.m_SelectionDisplay = out.SHOW_ROULETTE;
			else
				LOG->Warn( "The song file '%s' has an unknown #SELECTABLE value, \"%s\"; ignored.", sPath, sParams[1]);
		}

		else if( 0==stricmp(sValueName,"STOPS") || 0==stricmp(sValueName,"FREEZES") )
		{
			CStringArray arrayFreezeExpressions;
			split( sParams[1], ",", arrayFreezeExpressions );

			for( int f=0; f<arrayFreezeExpressions.GetSize(); f++ )
			{
				CStringArray arrayFreezeValues;
				split( arrayFreezeExpressions[f], "=", arrayFreezeValues );
				float fFreezeBeat = (float)atof( arrayFreezeValues[0] );
				float fFreezeSeconds = (float)atof( arrayFreezeValues[1] );
				
				StopSegment new_seg;
				new_seg.m_fStartBeat = fFreezeBeat;
				new_seg.m_fStopSeconds = fFreezeSeconds;

				LOG->Trace( "Adding a freeze segment: beat: %f, seconds = %f", new_seg.m_fStartBeat, new_seg.m_fStopSeconds );

				out.AddStopSegment( new_seg );
			}
		}

		else if( 0==stricmp(sValueName,"BPMS") )
		{
			CStringArray arrayBPMChangeExpressions;
			split( sParams[1], ",", arrayBPMChangeExpressions );

			for( int b=0; b<arrayBPMChangeExpressions.GetSize(); b++ )
			{
				CStringArray arrayBPMChangeValues;
				split( arrayBPMChangeExpressions[b], "=", arrayBPMChangeValues );
				float fBeat = (float)atof( arrayBPMChangeValues[0] );
				float fNewBPM = (float)atof( arrayBPMChangeValues[1] );
				
				BPMSegment new_seg;
				new_seg.m_fStartBeat = fBeat;
				new_seg.m_fBPM = fNewBPM;
				
				out.AddBPMSegment( new_seg );
			}
		}

		else if( 0==stricmp(sValueName,"BGCHANGES") )
		{
			CStringArray aBGChangeExpressions;
			split( sParams[1], ",", aBGChangeExpressions );

			for( int b=0; b<aBGChangeExpressions.GetSize(); b++ )
			{
				CStringArray aBGChangeValues;
				split( aBGChangeExpressions[b], "=", aBGChangeValues );
				float fBeat = (float)atof( aBGChangeValues[0] );
				CString sBGName = aBGChangeValues[1];
				sBGName.MakeLower();
				
				out.AddBackgroundChange( BackgroundChange(fBeat, sBGName) );
			}
		}

		else if( 0==stricmp(sValueName,"NOTES") )
		{
			Notes* pNewNotes = new Notes;
			ASSERT( pNewNotes );
			out.m_apNotes.Add( pNewNotes );

			if( iNumParams != 7 )
				throw RageException( "The song file '%s' is has %d fields in a #NOTES tag, but should have %d.", sPath, iNumParams, 7 );

			LoadFromSMTokens( 
				sParams[1], 
				sParams[2], 
				sParams[3], 
				sParams[4], 
				sParams[5], 
				sParams[6],
				*pNewNotes);
		}

		else
			LOG->Trace( "Unexpected value named '%s'", sValueName );
	}

	return true;
}
