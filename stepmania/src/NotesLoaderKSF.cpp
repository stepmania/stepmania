#include "stdafx.h"

#include "NotesLoaderKSF.h"

#include "RageException.h"
#include "MsdFile.h"
#include "RageLog.h"

bool KSFLoader::LoadFromKSFFile( const CString &sPath, Notes &out )
{
	LOG->Trace( "Notes::LoadFromKSFFile( '%s' )", sPath );

	MsdFile msd;
	bool bResult = msd.ReadFile( sPath );
	if( !bResult )
		throw RageException( "Error opening file '%s'.", sPath );

	int iTickCount = -1;	// this is the value we read for TICKCOUNT
	CString iStep;			// this is the value we read for STEP

	for( int i=0; i<msd.m_iNumValues; i++ )
	{
		int iNumParams = msd.m_iNumParams[i];
		CString* sParams = msd.m_sValuesAndParams[i];
		CString sValueName = sParams[0];

		// handle the data
		if( 0==stricmp(sValueName,"TICKCOUNT") )
			iTickCount = atoi(sParams[1]);

		else if( 0==stricmp(sValueName,"STEP") )
			iStep = sParams[1];
		else if( 0==stricmp(sValueName,"DIFFICULTY") )
			out.m_iMeter = atoi(sParams[1]);
	}

	if( iTickCount == -1 )
	{
		iTickCount = 2;
		LOG->Warn( "%s:\nTICKCOUNT not found; defaulting to %i", sPath, iTickCount );
	}

	NoteData notedata;	// read it into here

	CStringArray asRows;
	iStep.TrimLeft();
	split( iStep, "\n", asRows, true );

	int iHoldStartRow[13];
	for( int t=0; t<13; t++ )
		iHoldStartRow[t] = -1;

	for( int r=0; r<asRows.GetSize(); r++ )
	{
		CString& sRowString = asRows[r];
		
		if( sRowString == "" )
			continue;	// skip

		/* All 2s indicates the end of the song. */
		if( sRowString == "2222222222222" )
			break;

		ASSERT( sRowString.GetLength() == 13 );		// why 13 notes per row.  Beats me!

		// the length of a note in a row depends on TICKCOUNT
		float fBeatThisRow = r/(float)iTickCount;
		int row = BeatToNoteRow(fBeatThisRow);
		for( int t=0; t<13; t++ )
		{
			if( sRowString[t] == '4' )
			{
				/* Remember when each hold starts; ignore the middle. */
				if( iHoldStartRow[t] == -1 )
					iHoldStartRow[t] = r;

				continue;
			}

			if( iHoldStartRow[t] != -1 )	// this ends the hold
			{
				HoldNote hn = {
					t, /* button */
					iHoldStartRow[t]/(float)iTickCount, /* start */
					(r-1)/(float)iTickCount /* end */
				};
				notedata.AddHoldNote( hn );
				iHoldStartRow[t] = -1;
			}

			notedata.m_TapNotes[t][row] = sRowString[t];
		}
	}

	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt );
	sFName.MakeLower();
	
	out.m_sDescription = sFName;
	if( sFName.Find("crazy")!=-1 )
	{
		out.m_DifficultyClass = CLASS_HARD;
		if(!out.m_iMeter) out.m_iMeter = 8;
	}
	else if( sFName.Find("hard")!=-1 )
	{
		out.m_DifficultyClass = CLASS_MEDIUM;
		if(!out.m_iMeter) out.m_iMeter = 5;
	}
	else if( sFName.Find("easy")!=-1 )
	{
		out.m_DifficultyClass = CLASS_EASY;
		if(!out.m_iMeter) out.m_iMeter = 2;
	}
	else
	{
		out.m_DifficultyClass = CLASS_MEDIUM;
		if(!out.m_iMeter) out.m_iMeter = 5;
	}

	if( sFName.Find("double") != -1 )
	{
		notedata.m_iNumTracks = 10;
		out.m_NotesType = NOTES_TYPE_PUMP_DOUBLE;
	}
	else
	{
		notedata.m_iNumTracks = 5;
		out.m_NotesType = NOTES_TYPE_PUMP_SINGLE;
	}

	out.m_sSMNoteData = notedata.GetSMNoteDataString();

	return true;
}


bool KSFLoader::LoadFromKSFDir( CString sDir, Song &out )
{
	LOG->Trace( "Song::LoadFromKSFDir(%s)", sDir );

	CStringArray arrayKSFFileNames;
	GetDirListing( sDir + CString("*.ksf"), arrayKSFFileNames );

	if( arrayKSFFileNames.GetSize() == 0 )
		throw RageException( "Couldn't find any KSF files in '%s'", sDir );

	// load the Notes from the rest of the KSF files
	for( int i=0; i<arrayKSFFileNames.GetSize(); i++ ) 
	{
		if( arrayKSFFileNames[i].Find("_2") != -1 )
			continue;	// any "right-side" files should be loaded by Notes
		Notes* pNewNotes = new Notes;
		LoadFromKSFFile( out.m_sSongDir + arrayKSFFileNames[i], *pNewNotes );
		out.m_apNotes.Add( pNewNotes );
	}

	CString sPath = out.m_sSongDir + arrayKSFFileNames[0];

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
		{
			//title is usually in format "artist - songtitle"
			CStringArray asBits;
			split( sParams[1], " - ", asBits, false );

			/* It's often "artist - songtitle - difficulty".  Ignore
			 * the difficulty, since we get that from the filename. */
			if( asBits.GetSize() == 3 &&
				(!stricmp(asBits[2], "double") ||
				 !stricmp(asBits[2], "easy") ||
				 !stricmp(asBits[2], "normal") ||
				 !stricmp(asBits[2], "hard") ||
				 !stricmp(asBits[2], "crazy")) )
			{
				asBits.RemoveAt(2);
			}

			if( asBits.GetSize() == 2 )
			{
				out.m_sArtist = asBits[0];
				out.m_sMainTitle = asBits[1];
			}
			else
			{
				out.m_sMainTitle = asBits[0];
			}

			for( int j=0; j<out.m_sMainTitle.GetLength(); j++ )
			{
				char c = out.m_sMainTitle[j];
				if( c < 0 )	// this title has a foreign char
				{
					CStringArray asBits;
					split( sDir, "\\", asBits, true);
					CString sSongFolderName = asBits[ asBits.GetSize()-1 ];
					asBits.RemoveAll();

					split( sSongFolderName, " - ", asBits, false );
					if( asBits.GetSize() == 2 )
					{
						out.m_sArtist = asBits[0];
						out.m_sMainTitle = asBits[1];
					}
					else
					{
						out.m_sMainTitle = asBits[0];
					}
					break;
				}
			}
		}

		else if( 0==stricmp(sValueName,"BPM") )
			out.AddBPMSegment( BPMSegment(0, (float)atof(sParams[1])) );		

		else if( 0==stricmp(sValueName,"STARTTIME") )
			out.m_fBeat0OffsetInSeconds = -(float)atof(sParams[1])/100;		
		else if( 0==stricmp(sValueName,"TICKCOUNT") ||
				 0==stricmp(sValueName,"STEP") ||
				 0==stricmp(sValueName,"DIFFICULTY"))
			; /* Handled in Notes::LoadFromKSFFile; don't warn. */
		else
			LOG->Trace( "Unexpected value named '%s'", sValueName );
	}

	// search for music with song in the file name
	CStringArray arrayPossibleMusic;
	GetDirListing( out.m_sSongDir + CString("song.mp3"), arrayPossibleMusic );
	GetDirListing( out.m_sSongDir + CString("song.ogg"), arrayPossibleMusic );
	GetDirListing( out.m_sSongDir + CString("song.wav"), arrayPossibleMusic );

	if( arrayPossibleMusic.GetSize() > 0 )		// we found a match
		out.m_sMusicFile = arrayPossibleMusic[0];

	return TRUE;
}


