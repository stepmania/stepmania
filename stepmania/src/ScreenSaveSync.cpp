#include "global.h"
#include "ScreenSaveSync.h"
#include "GameState.h"
#include "song.h"
#include "PrefsManager.h"
#include "ScreenPrompt.h"


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
				"You have changed the Global Offset\nfrom %+.3f to %+.3f (change of %+.3f, notes %s).\n\n",
				fOld, 
				fNew,
				fDelta,
				fDelta > 0 ? "earlier":"later"  );
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
				"The song offset changed from %+.3f to %+.3f (change of %+.3f, notes %s).\n\n",
				fOld, 
				fNew,
				fDelta,
				fDelta > 0 ? "earlier":"later" ) );
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
				"The %s BPM segment changed from %+.3f BPS to %+.3f BPS (change of %+.3f).\n\n",
				FormatNumberAndSuffix(i+1).c_str(),
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
				"The %s Stop segment changed from %+.3f seconds to %+.3f seconds (change of %+.3f).\n\n",
				FormatNumberAndSuffix(i+1).c_str(),
				fOld, 
				fNew,
				fDelta ) );
		}
	}


	if( !vsSongChanges.empty() )
	{
		s += ssprintf( 
			"You have changed the timing of\n"
			"%s:\n"
			"\n", 
			GAMESTATE->m_pCurSong->GetDisplayFullTitle().c_str() );

		s += join( "\n", vsSongChanges );
	}

	s +="\n\n"
		"Would you like to save these changes?\n"
		"Choosing NO will discard your changes.";

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
ScreenSaveSync::ScreenSaveSync( CString sClassName ) : ScreenPrompt( sClassName )
{
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
