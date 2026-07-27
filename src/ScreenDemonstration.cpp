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
#include "ScreenAttract.h"

#include <vector>


#define SECONDS_TO_SHOW			THEME->GetMetricF(m_sName,"SecondsToShow")
#define ALLOW_STYLE_TYPES		THEME->GetMetric (m_sName,"AllowStyleTypes")

REGISTER_SCREEN_CLASS( ScreenDemonstration );
ScreenDemonstration::ScreenDemonstration()
{
	m_bDemonstration = true;
}

void ScreenDemonstration::Init()
{
	GAMESTATE->Reset();
	GAMESTATE->VisitAttractScreen( m_sName );
	ScreenAttract::SetAttractVolume( true );

	// Choose a Style
	{
		std::vector<RString> v;
		split( ALLOW_STYLE_TYPES, ",", v );
		std::vector<StyleType> vStyleTypeAllow;
		for (RString const &s : v)
		{
			StyleType st = StringToStyleType( s );
			ASSERT( st != StyleType_Invalid );
			vStyleTypeAllow.push_back( st );
		}

		std::vector<const Style*> vStylePossible;
		GAMEMAN->GetDemonstrationStylesForGame( GAMESTATE->m_pCurGame, vStylePossible );
		for( int i=(int)(vStylePossible.size())-1; i>=0; i-- )
		{
			bool bAllowThis = find( vStyleTypeAllow.begin(), vStyleTypeAllow.end(), vStylePossible[i]->m_StyleType ) != vStyleTypeAllow.end();
			if( !bAllowThis )
				vStylePossible.erase( vStylePossible.begin()+i );
		}

		ASSERT( vStylePossible.size() > 0 );
		const Style* pStyle = vStylePossible[ RandomInt(vStylePossible.size()) ];
		GAMESTATE->SetCurrentStyle( pStyle, PLAYER_INVALID );
	}

	GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );

	ScreenJukebox::Init();

	if( GAMESTATE->m_pCurSong == nullptr )	// we didn't find a song.
	{
		PostScreenMessage( SM_GoToNextScreen, 0 );	// Abort demonstration.
		return;
	}

	ClearMessageQueue();	// remove all of the messages set in ScreenGameplay that drive "ready", "go", etc.

	GAMESTATE->m_bGameplayLeadIn.Set( false );

	m_DancingState = STATE_DANCING;
	this->PostScreenMessage( SM_BeginFadingOut, SECONDS_TO_SHOW );		// TODO: Use MenuTimer instead?
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
	else if( SM == SM_LoseFocus )
	{
		ScreenAttract::SetAttractVolume( false );
	}
	ScreenJukebox::HandleScreenMessage( SM );
}

void ScreenDemonstration::Cancel( ScreenMessage smSendWhenDone )
{
	ScreenAttract::SetAttractVolume( false ); // unmute attract sounds
	ScreenJukebox::Cancel( smSendWhenDone );
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
