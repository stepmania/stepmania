#include "global.h"
#include "ScreenSaveSync.h"
#include "GameState.h"
#include "Song.h"
#include "PrefsManager.h"
#include "LocalizedString.h"
#include "AdjustSync.h"

#include <vector>


static LocalizedString CHANGED_TIMING_OF	("ScreenSaveSync","You have changed the timing of");
static LocalizedString WOULD_YOU_LIKE_TO_SAVE	("ScreenSaveSync","Would you like to save these changes?");
static LocalizedString CHOOSING_NO_WILL_DISCARD	("ScreenSaveSync","Choosing NO will discard your changes.");
static RString GetPromptText()
{
	RString s;

	{
		std::vector<RString> vs;
		AdjustSync::GetSyncChangeTextGlobal( vs );
		if( !vs.empty() )
			s += join( "\n", vs ) + "\n\n";
	}

	{
		std::vector<RString> vs;
		AdjustSync::GetSyncChangeTextSong( vs );
		if( !vs.empty() )
		{
			s += ssprintf(
				CHANGED_TIMING_OF.GetValue()+"\n"
				"%s:\n"
				"\n",
				GAMESTATE->m_pCurSong->GetDisplayFullTitle().c_str() );

			s += join( "\n", vs ) + "\n\n";
		}
	}

	s += WOULD_YOU_LIKE_TO_SAVE.GetValue()+"\n"+
		CHOOSING_NO_WILL_DISCARD.GetValue();
	return s;
}

static void SaveSyncChanges( void* pThrowAway )
{
	AdjustSync::SaveSyncChanges();
}

static void RevertSyncChanges( void* pThrowAway )
{
	AdjustSync::RevertSyncChanges();
}

void ScreenSaveSync::Init()
{
	ScreenPrompt::Init();

	ScreenPrompt::SetPromptSettings(
		GetPromptText(),
		PROMPT_YES_NO,
		ANSWER_YES,
		SaveSyncChanges,
		RevertSyncChanges,
		nullptr );
}

void ScreenSaveSync::PromptSaveSync( ScreenMessage sm )
{
	ScreenPrompt::Prompt(
		sm,
		GetPromptText(),
		PROMPT_YES_NO,
		ANSWER_YES,
		SaveSyncChanges,
		RevertSyncChanges,
		nullptr );
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
