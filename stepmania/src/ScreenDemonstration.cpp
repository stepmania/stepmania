#include "global.h"
#include "ScreenDemonstration.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "SongManager.h"
#include "StepMania.h"
#include "ScreenManager.h"
#include "RageSoundManager.h"
#include "GameSoundManager.h"
#include "GameManager.h"
#include "Style.h"
#include "PrefsManager.h"


#define SECONDS_TO_SHOW			THEME->GetMetricF(m_sName,"SecondsToShow")
#define NEXT_SCREEN				THEME->GetMetric (m_sName,"NextScreen")
#define ALLOW_STYLE_TYPES		THEME->GetMetric (m_sName,"AllowStyleTypes")

REGISTER_SCREEN_CLASS( ScreenDemonstration );
ScreenDemonstration::ScreenDemonstration( CString sName ) : ScreenJukebox( sName )
{
	LOG->Trace( "ScreenDemonstration::ScreenDemonstration()" );
	m_bDemonstration = true;
}

void ScreenDemonstration::Init()
{
	// Choose a Style
	{
		vector<CString> v;
		split( ALLOW_STYLE_TYPES, ",", v );
		vector<StyleType> vStyleTypeAllow;
		FOREACH_CONST( CString, v, s )
		{
			StyleType st = StringToStyleType( *s );
			ASSERT( st != STYLE_TYPE_INVALID );
			vStyleTypeAllow.push_back( st );
		}

		vector<const Style*> vStylePossible;
		GAMEMAN->GetDemonstrationStylesForGame( GAMESTATE->m_pCurGame, vStylePossible );
		for( int i=(int)(vStylePossible.size())-1; i>=0; i-- )
		{
			bool bAllowThis = find( vStyleTypeAllow.begin(), vStyleTypeAllow.end(), vStylePossible[i]->m_StyleType ) != vStyleTypeAllow.end();
			if( !bAllowThis )
				vStylePossible.erase( vStylePossible.begin()+i );
		}

		ASSERT( vStylePossible.size() > 0 );
		const Style* pStyle = vStylePossible[ rand() % vStylePossible.size() ];
		GAMESTATE->m_pCurStyle.Set( pStyle );
	}

	GAMESTATE->m_PlayMode = PLAY_MODE_REGULAR;

	ScreenJukebox::Init();

	if( GAMESTATE->m_pCurSong == NULL )	// we didn't find a song.
	{
		HandleScreenMessage( SM_GoToNextScreen );	// Abort demonstration.
		return;
	}


	this->SortByDrawOrder();

	ClearMessageQueue();	// remove all of the messages set in ScreenGameplay that drive "ready", "go", etc.

	GAMESTATE->m_bPastHereWeGo = true;

	m_DancingState = STATE_DANCING;
	this->PostScreenMessage( SM_BeginFadingOut, SECONDS_TO_SHOW );	
}

void ScreenDemonstration::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_NotesEnded ||
		SM == SM_BeginFadingOut )
	{
		if(!m_Out.IsTransitioning())
			m_Out.StartTransitioning( SM_GoToNextScreen );
		return;
	}
	else if( SM == SM_GainFocus )
	{
	}
	else if( SM == SM_LoseFocus )
	{
		SOUNDMAN->SetPrefs( PREFSMAN->GetSoundVolume() );	// turn volume back on
	}
	else if( SM == SM_GoToNextScreen )
	{
		if( m_pSoundMusic )
			m_pSoundMusic->Stop();
		SOUNDMAN->SetPrefs( PREFSMAN->GetSoundVolume() );	// turn volume back on
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		return;
	}

	ScreenJukebox::HandleScreenMessage( SM );
}

/*
 * (c) 2003-2004 Chris Danford
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
