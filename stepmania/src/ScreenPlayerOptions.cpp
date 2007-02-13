#include "global.h"
#include "ScreenPlayerOptions.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "GameSoundManager.h"
#include "ScreenSongOptions.h"
#include "PrefsManager.h"
#include "CodeDetector.h"
#include "ScreenDimensions.h"
#include "PlayerState.h"
#include "Foreach.h"
#include "InputEventPlus.h"

REGISTER_SCREEN_CLASS( ScreenPlayerOptions );

void ScreenPlayerOptions::Init()
{
	ScreenOptionsMaster::Init();

	FOREACH_PlayerNumber( p )
	{
		m_sprDisqualify[p].Load( THEME->GetPathG(m_sName,"disqualify") );
		m_sprDisqualify[p]->SetName( ssprintf("DisqualifyP%i",p+1) );
		SET_XY( m_sprDisqualify[p] );
		m_sprDisqualify[p]->SetVisible( false );	// unhide later if handicapping options are discovered
		m_sprDisqualify[p]->SetDrawOrder( 2 );
		m_framePage.AddChild( m_sprDisqualify[p] );
	}

	m_bAskOptionsMessage =
		!GAMESTATE->IsEditing() && PREFSMAN->m_ShowSongOptions == Maybe_ASK;

	/* If we're going to "press start for more options" or skipping options
	 * entirely, we need a different fade out. XXX: this is a hack */
	if( PREFSMAN->m_ShowSongOptions == Maybe_NO || GAMESTATE->IsEditing() )
	{
		m_Out.Load( THEME->GetPathB("ScreenPlayerOptions","direct out") ); /* direct to stage */
	}
	else if( m_bAskOptionsMessage )
	{
		m_Out.Load( THEME->GetPathB("ScreenPlayerOptions","option out") ); /* optional song options */

		m_sprOptionsMessage.Load( THEME->GetPathG("ScreenPlayerOptions","options") );
		m_sprOptionsMessage.StopAnimating();
		m_sprOptionsMessage.SetXY( SCREEN_CENTER_X, SCREEN_CENTER_Y );
		m_sprOptionsMessage.SetZoom( 1 );
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
		m_sprOptionsMessage.SetDrawOrder( DRAW_ORDER_TRANSITIONS+1 );
		this->AddChild( &m_sprOptionsMessage );
	}

	m_bAcceptedChoices = false;
	m_bGoToOptions = ( PREFSMAN->m_ShowSongOptions == Maybe_YES );

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("player options intro") );

	FOREACH_PlayerNumber( p )
		m_bRowCausesDisqualified[p].resize( m_pRows.size(), false );
}


void ScreenPlayerOptions::BeginScreen()
{
	FOREACH_PlayerNumber( p )
		ON_COMMAND( m_sprDisqualify[p] );

	ScreenOptionsMaster::BeginScreen();

	FOREACH_HumanPlayer( p )
	{
		for( unsigned r=0; r<m_pRows.size(); r++ )
			UpdateDisqualified( r, p );
	}
}

void ScreenPlayerOptions::Input( const InputEventPlus &input )
{
	if( m_bAskOptionsMessage &&
		input.type == IET_FIRST_PRESS  &&
		!m_In.IsTransitioning()  &&
		input.MenuI == MENU_BUTTON_START )
	{
		if( m_bAcceptedChoices  &&  !m_bGoToOptions )
		{
			m_bGoToOptions = true;
			m_sprOptionsMessage.SetState( 1 );
			SCREENMAN->PlayStartSound();
		}
	}

	PlayerNumber pn = input.pn;
	if( GAMESTATE->IsHumanPlayer(pn) && CodeDetector::EnteredCode(input.GameI.controller,CODE_CANCEL_ALL_PLAYER_OPTIONS) )
	{
		// apply the game default mods, but not the Profile saved mods
		PlayerOptions po;
		GAMESTATE->GetDefaultPlayerOptions( po );
		GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.Assign( ModsLevel_Preferred, po );
		
		MESSAGEMAN->Broadcast( ssprintf("CancelAllP%i", pn+1) );

		for( unsigned r=0; r<m_pRows.size(); r++ )
		{
			vector<PlayerNumber> v;
			v.push_back( pn );
			int iOldFocus = m_pRows[r]->GetChoiceInRowWithFocus( pn );
			this->ImportOptions( r, v );
			m_pRows[r]->AfterImportOptions( pn );
			this->UpdateDisqualified( r, pn );
			m_pRows[r]->SetChoiceInRowWithFocus( pn, iOldFocus );
		}
	}

	ScreenOptionsMaster::Input( input );

	// UGLY: Update m_Disqualified whenever Start is pressed
	if( GAMESTATE->IsHumanPlayer(pn) && input.MenuI == MENU_BUTTON_START )
	{
		int row = m_iCurrentRow[pn];
		UpdateDisqualified( row, pn );
	}
}

void ScreenPlayerOptions::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_BeginFadingOut && m_bAskOptionsMessage ) // user accepts the page of options
	{
		m_bAcceptedChoices = true;

		float fShowSeconds = max( m_Out.GetLengthSeconds()-0.3f, 0 );

		// show "hold START for options"
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
		m_sprOptionsMessage.BeginTweening( 0.15f );     // fade in
		m_sprOptionsMessage.SetZoomY( 1 );
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,1) );
		m_sprOptionsMessage.BeginTweening( fShowSeconds ); // sleep
		m_sprOptionsMessage.BeginTweening( 0.15f );     // fade out
		m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
		m_sprOptionsMessage.SetZoomY( 0 );
	}

	ScreenOptionsMaster::HandleScreenMessage( SM );
}

void ScreenPlayerOptions::UpdateDisqualified( int row, PlayerNumber pn )
{
	ASSERT( GAMESTATE->IsHumanPlayer(pn) );

	// save original player options 
	PlayerOptions poOrig = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetPreferred();

	// Find out if the current row when exprorted causes disqualification.
	// Exporting the row will fill GAMESTATE->m_PlayerOptions.
	PO_GROUP_CALL( GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions, ModsLevel_Preferred, Init );
	vector<PlayerNumber> v;
	v.push_back( pn );
	ExportOptions( row, v );
	bool bRowCausesDisqualified = GAMESTATE->IsDisqualified( pn );
	m_bRowCausesDisqualified[pn][row] = bRowCausesDisqualified;

	// Update disqualified graphic
	bool bDisqualified = false;
	FOREACH_CONST( bool, m_bRowCausesDisqualified[pn], b )
	{
		if( *b )
		{
			bDisqualified = true;
			break;
		}
	}
	m_sprDisqualify[pn]->SetVisible( bDisqualified );

	// restore previous player options in case the user escapes back after this
	GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.Assign( ModsLevel_Preferred, poOrig );
}

// lua start
#include "LuaBinding.h"

class LunaScreenPlayerOptions: public Luna<ScreenPlayerOptions>
{
public:
	static int GetGoToOptions( T* p, lua_State *L ) { lua_pushboolean( L, p->GetGoToOptions() ); return 1; }

	LunaScreenPlayerOptions()
	{
  		ADD_METHOD( GetGoToOptions );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenPlayerOptions, ScreenWithMenuElements )
// lua end

/*
 * (c) 2001-2004 Chris Danford
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
