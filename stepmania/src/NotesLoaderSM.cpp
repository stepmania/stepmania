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
	CString sDifficulty,
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
	sDifficulty.TrimLeft(); 
	sDifficulty.TrimRight(); 


//	LOG->Trace( "Notes::LoadFromSMTokens()" );

	out.m_NotesType = GameManager::StringToNotesType(sNotesType);
	out.m_sDescription = sDescription;
	out.m_Difficulty = StringToDifficulty( sDifficulty );
	out.m_iMeter = atoi(sMeter);
	CStringArray saValues;
	split( sRadarValues, ",", saValues, true );
	if( saValues.size() == NUM_RADAR_VALUES )
		for( int r=0; r<NUM_RADAR_VALUES; r++ )
			out.m_fRadarValues[r] = (float)atof(saValues[r]);
    
	out.m_sSMNoteData = sNoteData;

	out.TidyUpData();
}

void SMLoader::GetApplicableFiles( CString sPath, CStringArray &out )
{
	GetDirListing( sPath + CString("*.sm"), out );
}

bool SMLoader::LoadFromSMFile( CString sPath, Song &out )
{
	LOG->Trace( "Song::LoadFromSMDir(%s)", sPath.GetString() );

	out.m_BPMSegments.clear();
	out.m_StopSegments.clear();

	MsdFile msd;
	bool bResult = msd.ReadFile( sPath );
	if( !bResult )
		throw RageException( "Error opening file '%s'.", sPath.GetString() );

	for( unsigned i=0; i<msd.m_iNumValues; i++ )
	{
		int iNumParams = msd.m_iNumParams[i];
		CString* sParams = msd.m_sParams[i];
		CString sValueName = sParams[0];

		// handle the data
		if( 0==stricmp(sValueName,"TITLE") )
			GetMainAndSubTitlesFromFullTitle( sParams[1], out.m_sMainTitle, out.m_sSubTitle );

		else if( 0==stricmp(sValueName,"SUBTITLE") )
			out.m_sSubTitle = sParams[1];

		else if( 0==stricmp(sValueName,"ARTIST") )
			out.m_sArtist = sParams[1];

		else if( 0==stricmp(sValueName,"TITLETRANSLIT") )
			out.m_sMainTitleTranslit = sParams[1];

		else if( 0==stricmp(sValueName,"SUBTITLETRANSLIT") )
			out.m_sSubTitleTranslit = sParams[1];

		else if( 0==stricmp(sValueName,"ARTISTTRANSLIT") )
			out.m_sArtistTranslit = sParams[1];

		else if( 0==stricmp(sValueName,"CREDIT") )
			out.m_sCredit = sParams[1];

		else if( 0==stricmp(sValueName,"BANNER") )
			out.m_sBannerFile = sParams[1];

		else if( 0==stricmp(sValueName,"BACKGROUND") )
			out.m_sBackgroundFile = sParams[1];

		else if( 0==stricmp(sValueName,"CDTITLE") )
			out.m_sCDTitleFile = sParams[1];

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
				LOG->Warn( "The song file '%s' has an unknown #SELECTABLE value, '%s'; ignored.", sPath.GetString(), sParams[1].GetString());
		}

		else if( 0==stricmp(sValueName,"STOPS") || 0==stricmp(sValueName,"FREEZES") )
		{
			CStringArray arrayFreezeExpressions;
			split( sParams[1], ",", arrayFreezeExpressions );

			for( unsigned f=0; f<arrayFreezeExpressions.size(); f++ )
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

			for( unsigned b=0; b<arrayBPMChangeExpressions.size(); b++ )
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

		else if( 0==stricmp(sValueName,"BGCHANGES") || 0==stricmp(sValueName,"ANIMATIONS") )
		{
			CStringArray aBGChangeExpressions;
			split( sParams[1], ",", aBGChangeExpressions );

			for( unsigned b=0; b<aBGChangeExpressions.size(); b++ )
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
			out.m_apNotes.push_back( pNewNotes );

			if( iNumParams != 7 )
			{
				LOG->Trace( "The song file '%s' is has %d fields in a #NOTES tag, but should have %d.", sPath.GetString(), iNumParams, 7 );
			}
			else
			{
				LoadFromSMTokens( 
					sParams[1], 
					sParams[2], 
					sParams[3], 
					sParams[4], 
					sParams[5], 
					sParams[6],
					*pNewNotes);
			}
		}

		else
			LOG->Trace( "Unexpected value named '%s'", sValueName.GetString() );
	}

	return true;
}


bool SMLoader::LoadFromDir( CString sPath, Song &out )
{
	CStringArray aFileNames;
	GetApplicableFiles( sPath, aFileNames );

	if( aFileNames.size() > 1 )
		throw RageException( "There is more than one SM file in '%s'.  There should be only one!", sPath.GetString() );

	/* We should have exactly one; if we had none, we shouldn't have been
	 * called to begin with. */
	ASSERT( aFileNames.size() == 1 );

	return LoadFromSMFile( sPath + aFileNames[0], out );
}

