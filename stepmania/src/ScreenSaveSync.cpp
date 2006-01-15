#include "global.h"
#include "ScreenSaveSync.h"
#include "GameState.h"
#include "song.h"
#include "PrefsManager.h"
#include "LocalizedString.h"


static LocalizedString EARLIER					("ScreenSaveSync","earlier");
static LocalizedString LATER					("ScreenSaveSync","later");
static LocalizedString CHANGED_GLOBAL_OFFSET	("ScreenSaveSync","You have changed the Global Offset from %+.3f to %+.3f (change of %+.3f, making the notes %s).");
static LocalizedString CHANGED_SONG_OFFSET		("ScreenSaveSync","You have changed the Song Offset from %+.3f to %+.3f (change of %+.3f, making the notes %s).");
static LocalizedString CHANGED_BPM				("ScreenSaveSync","The BPM segment number #%d changed from %+.3f BPS to %+.3f BPS (change of %+.3f).");
static LocalizedString CHANGED_STOP				("ScreenSaveSync","The stop segment #%d changed from %+.3f seconds to %+.3f seconds (change of %+.3f).");
static LocalizedString CHANGED_TIMING_OF		("ScreenSaveSync","You have changed the timing of");
static LocalizedString WOULD_YOU_LIKE_TO_SAVE	("ScreenSaveSync","Would you like to save these changes?");
static LocalizedString CHOOSING_NO_WILL_DISCARD	("ScreenSaveSync","Choosing NO will discard your changes.");
static CString GetPromptText()
{
	CString s;

	{
		float fOld = GAMESTATE->m_fGlobalOffsetSecondsOriginal;
		float fNew = PREFSMAN->m_fGlobalOffsetSeconds;
		float fDelta = fNew - fOld;

		if( fabs(fDelta) > 0.00001 )
		{
			s += ssprintf( 
				CHANGED_GLOBAL_OFFSET.GetValue()+"\n\n",
				fOld, 
				fNew,
				fDelta,
				(fDelta > 0 ? EARLIER:LATER).GetValue().c_str()  );
		}
	}

	vector<CString> vsSongChanges;

	{
		float fOld = GAMESTATE->m_pTimingDataOriginal->m_fBeat0OffsetInSeconds;
		float fNew = GAMESTATE->m_pCurSong->m_Timing.m_fBeat0OffsetInSeconds;
		float fDelta = fNew - fOld;

		if( fabs(fDelta) > 0.00001 )
		{
			vsSongChanges.push_back( ssprintf( 
				CHANGED_SONG_OFFSET.GetValue()+"\n\n",
				fOld, 
				fNew,
				fDelta,
				(fDelta > 0 ? EARLIER:LATER).GetValue().c_str() ) );
		}
	}

	for( unsigned i=0; i<GAMESTATE->m_pCurSong->m_Timing.m_BPMSegments.size(); i++ )
	{
		float fOld = GAMESTATE->m_pTimingDataOriginal->m_BPMSegments[i].m_fBPS;
		float fNew = GAMESTATE->m_pCurSong->m_Timing.m_BPMSegments[i].m_fBPS;
		float fDelta = fNew - fOld;

		if( fabs(fDelta) > 0.00001 )
		{
			vsSongChanges.push_back( ssprintf( 
				CHANGED_BPM.GetValue()+"\n\n",
				i+1,
				fOld, 
				fNew,
				fDelta ) );
		}
	}

	for( unsigned i=0; i<GAMESTATE->m_pCurSong->m_Timing.m_StopSegments.size(); i++ )
	{
		float fOld = GAMESTATE->m_pTimingDataOriginal->m_StopSegments[i].m_fStopSeconds;
		float fNew = GAMESTATE->m_pCurSong->m_Timing.m_StopSegments[i].m_fStopSeconds;
		float fDelta = fNew - fOld;

		if( fabs(fDelta) > 0.00001 )
		{
			vsSongChanges.push_back( ssprintf( 
				CHANGED_STOP.GetValue()+"\n\n",
				i+1,
				fOld, 
				fNew,
				fDelta ) );
		}
	}


	if( !vsSongChanges.empty() )
	{
		s += ssprintf( 
			CHANGED_TIMING_OF.GetValue()+"\n"
			"%s:\n"
			"\n", 
			GAMESTATE->m_pCurSong->GetDisplayFullTitle().c_str() );

		s += join( "\n", vsSongChanges );
	}

	s +="\n\n"+
		WOULD_YOU_LIKE_TO_SAVE.GetValue()+"\n"+
		CHOOSING_NO_WILL_DISCARD.GetValue();
	return s;
}
			
static void SaveSyncChanges( void* pThrowAway )
{
	GAMESTATE->SaveSyncChanges();
}

static void RevertSyncChanges( void* pThrowAway )
{
	GAMESTATE->RevertSyncChanges();
}

REGISTER_SCREEN_CLASS( ScreenSaveSync );
void ScreenSaveSync::Init()
{
	ScreenPrompt::Init();

	ScreenPrompt::SetPromptSettings(
		GetPromptText(), 
		PROMPT_YES_NO, 
		ANSWER_YES, 
		SaveSyncChanges, 
		RevertSyncChanges, 
		NULL );
}

void ScreenSaveSync::PromptSaveSync()
{
	ScreenPrompt::Prompt(
		SM_None,
		GetPromptText(), 
		PROMPT_YES_NO, 
		ANSWER_YES, 
		SaveSyncChanges, 
		RevertSyncChanges, 
		NULL );
}


/*
 * (c) 2001-2005 Chris Danford
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
