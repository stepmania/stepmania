#include "stdafx.h"
#include "NotesWriterSM.h"
#include "Notes.h"
#include "RageUtil.h"
#include "GameManager.h"

void NotesWriterSM::WriteGlobalTags(FILE *fp, const Song &out)
{
	fprintf( fp, "#TITLE:%s;\n", out.m_sMainTitle.GetString() );
	fprintf( fp, "#SUBTITLE:%s;\n", out.m_sSubTitle.GetString() );
	fprintf( fp, "#ARTIST:%s;\n", out.m_sArtist.GetString() );
	fprintf( fp, "#TITLETRANSLIT:%s;\n", out.m_sMainTitleTranslit.GetString() );
	fprintf( fp, "#SUBTITLETRANSLIT:%s;\n", out.m_sSubTitleTranslit.GetString() );
	fprintf( fp, "#ARTISTTRANSLIT:%s;\n", out.m_sArtistTranslit.GetString() );
	fprintf( fp, "#CREDIT:%s;\n", out.m_sCredit.GetString() );
	fprintf( fp, "#BANNER:%s;\n", out.m_sBannerFile.GetString() );
	fprintf( fp, "#BACKGROUND:%s;\n", out.m_sBackgroundFile.GetString() );
	fprintf( fp, "#CDTITLE:%s;\n", out.m_sCDTitleFile.GetString() );
	fprintf( fp, "#MUSIC:%s;\n", out.m_sMusicFile.GetString() );
	fprintf( fp, "#MUSICBYTES:%u;\n", out.m_iMusicBytes );
	fprintf( fp, "#MUSICLENGTH:%.3f;\n", out.m_fMusicLengthSeconds );
	fprintf( fp, "#OFFSET:%.3f;\n", out.m_fBeat0OffsetInSeconds );
	fprintf( fp, "#SAMPLESTART:%.3f;\n", out.m_fMusicSampleStartSeconds );
	fprintf( fp, "#SAMPLELENGTH:%.3f;\n", out.m_fMusicSampleLengthSeconds );
	fprintf( fp, "#SELECTABLE:" );

	switch(out.m_SelectionDisplay) {
	default: ASSERT(0);  /* fallthrough */
	case Song::SHOW_ALWAYS:
		fprintf( fp, "YES" ); break;
	case Song::SHOW_NEVER:
		fprintf( fp, "NO" ); break;
	case Song::SHOW_ROULETTE:
		fprintf( fp, "ROULETTE" ); break;
	}
	fprintf( fp, ";\n" );

	fprintf( fp, "#BPMS:" );
	unsigned i;
	for( i=0; i<out.m_BPMSegments.size(); i++ )
	{
		const BPMSegment &bs = out.m_BPMSegments[i];

		fprintf( fp, "%.3f=%.3f", bs.m_fStartBeat, bs.m_fBPM );
		if( i != out.m_BPMSegments.size()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );

	fprintf( fp, "#STOPS:" );
	for( i=0; i<out.m_StopSegments.size(); i++ )
	{
		const StopSegment &fs = out.m_StopSegments[i];

		fprintf( fp, "%.3f=%.3f", fs.m_fStartBeat, fs.m_fStopSeconds );
		if( i != out.m_StopSegments.size()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );
	
	fprintf( fp, "#BGCHANGES:" );
	for( i=0; i<out.m_BackgroundChanges.size(); i++ )
	{
		const BackgroundChange &seg = out.m_BackgroundChanges[i];

		fprintf( fp, "%.3f=%s", seg.m_fStartBeat, seg.m_sBGName.GetString() );
		if( i != out.m_BackgroundChanges.size()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );

}

void NotesWriterSM::WriteSMNotesTag( const Notes &in, FILE* fp )
{
	fprintf( fp, "\n//---------------%s - %s----------------\n",
		GameManager::NotesTypeToString(in.m_NotesType).GetString(), in.GetDescription().GetString() );
	fprintf( fp, "#NOTES:\n" );
	fprintf( fp, "     %s:\n", GameManager::NotesTypeToString(in.m_NotesType).GetString() );
	fprintf( fp, "     %s:\n", in.GetDescription().GetString() );
	fprintf( fp, "     %s:\n", DifficultyToString(in.GetDifficulty()).GetString() );
	fprintf( fp, "     %d:\n", in.GetMeter() );
	
	CStringArray asRadarValues;
	for( int r=0; r < NUM_RADAR_VALUES; r++ )
		asRadarValues.push_back( ssprintf("%.3f", in.GetRadarValues()[r]) );
	fprintf( fp, "     %s:\n", join(",",asRadarValues).GetString() );

	fprintf( fp, "%s;\n", in.GetSMNoteData().GetString() );
}

bool NotesWriterSM::Write(CString sPath, const Song &out, bool bSavingCache)
{
	unsigned i;

	FILE* fp = fopen( sPath, "w" );	
	if( fp == NULL )
		RageException::Throw( "Error opening song file '%s' for writing.", sPath.GetString() );

	WriteGlobalTags(fp, out);
	if(bSavingCache) {
		fprintf( fp, "#FIRSTBEAT:%.3f;\n", out.m_fFirstBeat );
		fprintf( fp, "#LASTBEAT:%.3f;\n", out.m_fLastBeat );
	}

	//
	// Save all Notes for this file
	//
	for( i=0; i<out.m_apNotes.size(); i++ ) 
	{
		Notes* pNotes = out.m_apNotes[i];
		if( pNotes->IsAutogen() )
			continue; /* don't write autogen notes */

		WriteSMNotesTag( *out.m_apNotes[i], fp );
	}

	fclose( fp );

	return true;
}
