#include "global.h"
#include "NotesWriterSM.h"
#include "Steps.h"
#include "RageUtil.h"
#include "GameManager.h"
#include "RageLog.h"
#include "RageFile.h"
#include <string.h>
#include <errno.h>

void NotesWriterSM::WriteGlobalTags(FILE *fp, const Song &out)
{
	fprintf( fp, "#TITLE:%s;\n", out.m_sMainTitle.c_str() );
	fprintf( fp, "#SUBTITLE:%s;\n", out.m_sSubTitle.c_str() );
	fprintf( fp, "#ARTIST:%s;\n", out.m_sArtist.c_str() );
	fprintf( fp, "#TITLETRANSLIT:%s;\n", out.m_sMainTitleTranslit.c_str() );
	fprintf( fp, "#SUBTITLETRANSLIT:%s;\n", out.m_sSubTitleTranslit.c_str() );
	fprintf( fp, "#ARTISTTRANSLIT:%s;\n", out.m_sArtistTranslit.c_str() );
	fprintf( fp, "#CREDIT:%s;\n", out.m_sCredit.c_str() );
	fprintf( fp, "#BANNER:%s;\n", out.m_sBannerFile.c_str() );
	fprintf( fp, "#BACKGROUND:%s;\n", out.m_sBackgroundFile.c_str() );
	fprintf( fp, "#LYRICSPATH:%s;\n", out.m_sLyricsFile.c_str() );
	fprintf( fp, "#CDTITLE:%s;\n", out.m_sCDTitleFile.c_str() );
	fprintf( fp, "#MUSIC:%s;\n", out.m_sMusicFile.c_str() );
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

	switch( out.m_DisplayBPMType )
	{
	case Song::DISPLAY_ACTUAL:
		// write nothing
		break;
	case Song::DISPLAY_SPECIFIED:
		if( out.m_fSpecifiedBPMMin == out.m_fSpecifiedBPMMax )
			fprintf( fp, "#DISPLAYBPM:%.3f;\n", out.m_fSpecifiedBPMMin );
		else
			fprintf( fp, "#DISPLAYBPM:%.3f:%.3f;\n", out.m_fSpecifiedBPMMin, out.m_fSpecifiedBPMMax );
		break;
	case Song::DISPLAY_RANDOM:
		fprintf( fp, "#DISPLAYBPM:*;\n" );
		break;
	}


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

		fprintf( fp, "%.3f=%s=%.3f=%d=%d=%d", seg.m_fStartBeat, seg.m_sBGName.c_str(), seg.m_fRate, seg.m_bFadeLast, seg.m_bRewindMovie, seg.m_bLoop );
		if( i != out.m_BackgroundChanges.size()-1 )
			fprintf( fp, "," );
	}
	fprintf( fp, ";\n" );

}

void NotesWriterSM::WriteSMNotesTag( const Steps &in, FILE* fp )
{
	fprintf( fp, "\n//---------------%s - %s----------------\n",
		GameManager::NotesTypeToString(in.m_NotesType).c_str(), in.GetDescription().c_str() );
	fprintf( fp, "#NOTES:\n" );
	fprintf( fp, "     %s:\n", GameManager::NotesTypeToString(in.m_NotesType).c_str() );
	fprintf( fp, "     %s:\n", in.GetDescription().c_str() );
	fprintf( fp, "     %s:\n", DifficultyToString(in.GetDifficulty()).c_str() );
	fprintf( fp, "     %d:\n", in.GetMeter() );
	
	CStringArray asRadarValues;
	for( int r=0; r < NUM_RADAR_CATEGORIES; r++ )
		asRadarValues.push_back( ssprintf("%.3f", in.GetRadarValues()[r]) );
	fprintf( fp, "     %s:\n", join(",",asRadarValues).c_str() );

	fprintf( fp, "%s;\n", in.GetSMNoteData().c_str() );
}

bool NotesWriterSM::Write(CString sPath, const Song &out, bool bSavingCache)
{
	unsigned i;

	FILE* fp = Ragefopen( sPath, "w" );	
	if( fp == NULL )
	{
		LOG->Warn( "Error opening song file '%s' for writing: %s", sPath.c_str(), strerror(errno) );
		return false;
	}

	WriteGlobalTags(fp, out);
	if(bSavingCache) {
		fprintf( fp, "#FIRSTBEAT:%.3f;\n", out.m_fFirstBeat );
		fprintf( fp, "#LASTBEAT:%.3f;\n", out.m_fLastBeat );
	}

	//
	// Save all Steps for this file
	//
	for( i=0; i<out.m_apNotes.size(); i++ ) 
	{
		Steps* pNotes = out.m_apNotes[i];
		if( pNotes->IsAutogen() )
			continue; /* don't write autogen notes */

		WriteSMNotesTag( *out.m_apNotes[i], fp );
	}

	fclose( fp );

	return true;
}
