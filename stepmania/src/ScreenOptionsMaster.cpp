#include "global.h"

#include "ScreenOptionsMaster.h"
#include "RageException.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "ScreenManager.h"
#include "SongManager.h"
#include "PrefsManager.h"
#include "GameSoundManager.h"
#include "StepMania.h"
#include "RageSoundManager.h"
#include "Foreach.h"
#include "OptionRowHandler.h"
#include "ScreenOptionsMasterPrefs.h"

#define LINE_NAMES					THEME->GetMetric (m_sName,"LineNames")
#define OPTION_MENU_FLAGS			THEME->GetMetric (m_sName,"OptionMenuFlags")
#define LINE(sLineName)				THEME->GetMetric (m_sName,ssprintf("Line%s",sLineName.c_str()))

#define NEXT_SCREEN					THEME->GetMetric (m_sName,"NextScreen")
#define PREV_SCREEN					THEME->GetMetric (m_sName,"PrevScreen")


REGISTER_SCREEN_CLASS( ScreenOptionsMaster );
ScreenOptionsMaster::ScreenOptionsMaster( CString sClassName ):
	ScreenOptions( sClassName )
{
	LOG->Trace("ScreenOptionsMaster::ScreenOptionsMaster(%s)", m_sName.c_str() );
}

void ScreenOptionsMaster::Init()
{
	ScreenOptions::Init();

	CStringArray asLineNames;
	split( LINE_NAMES, ",", asLineNames );
	if( asLineNames.empty() )
		RageException::Throw( "%s::LineNames is empty.", m_sName.c_str() );


	CStringArray Flags;
	split( OPTION_MENU_FLAGS, ";", Flags, true );
	InputMode im = INPUTMODE_INDIVIDUAL;
	bool Explanations = false;
	
	for( unsigned i = 0; i < Flags.size(); ++i )
	{
		CString sFlag = Flags[i];
		sFlag.MakeLower();

		if( sFlag == "together" )
			im = INPUTMODE_SHARE_CURSOR;
		else if( sFlag == "explanations" )
			Explanations = true;
		else if( sFlag == "forceallplayers" )
		{
			FOREACH_PlayerNumber( pn )
				GAMESTATE->m_bSideIsJoined[pn] = true;
			GAMESTATE->m_MasterPlayerNumber = PlayerNumber(0);
		}
		else if( sFlag == "smnavigation" )
			SetNavigation( NAV_THREE_KEY_MENU );
		else if( sFlag == "toggle" || sFlag == "firstchoicegoesdown" )
			SetNavigation( PREFSMAN->m_bArcadeOptionsNavigation? NAV_TOGGLE_THREE_KEY:NAV_TOGGLE_FIVE_KEY );
		else
			RageException::Throw( "Unknown flag \"%s\"", sFlag.c_str() );
	}

	vector<OptionRowDefinition> OptionRowDefs;
	OptionRowDefs.resize( asLineNames.size() );
	OptionRowHandlers.resize( asLineNames.size() );
	for( unsigned i = 0; i < asLineNames.size(); ++i )
	{
		CString sLineName = asLineNames[i];
		OptionRowDefinition &def = OptionRowDefs[i];
		CString sRowCommands = LINE(sLineName);
		OptionRowHandler* &pHand = OptionRowHandlers[i];
		pHand = NULL;
		
		Commands vCommands;
		ParseCommands( sRowCommands, vCommands );
		if( vCommands.v.size() != 1 )
			RageException::Throw( "Parse error in %s::Line%i", m_sName.c_str(), i+1 );

		Command& command = vCommands.v[0];
		pHand = OptionRowHandlerUtil::Make( command, def );
		if( pHand == NULL )
            RageException::Throw( "Invalid OptionRowHandler '%s' in %s::Line%i", command.GetOriginalCommandString().c_str(), m_sName.c_str(), i );
	}

	ASSERT( OptionRowHandlers.size() == asLineNames.size() );

	InitMenu( im, OptionRowDefs, OptionRowHandlers );
}

ScreenOptionsMaster::~ScreenOptionsMaster()
{
	FOREACH( OptionRow*, m_Rows, r )
		(*r)->DetachHandler();
	FOREACH( OptionRowHandler*, OptionRowHandlers, h )
		SAFE_DELETE( *h );
	OptionRowHandlers.clear();
}

void ScreenOptionsMaster::ImportOptions( int r, const vector<PlayerNumber> &vpns )
{
	FOREACH_CONST( PlayerNumber, vpns, pn )
		ASSERT( GAMESTATE->IsHumanPlayer(*pn) );
	OptionRow &row = *m_Rows[r];
	row.ImportOptions( vpns );
}

void ScreenOptionsMaster::ExportOptions( int r, const vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_Rows[r];
	bool bRowHasFocus[NUM_PLAYERS];
	ZERO( bRowHasFocus );
	FOREACH_CONST( PlayerNumber, vpns, p )
	{
		int iCurRow = m_iCurrentRow[*p];
		bRowHasFocus[*p] = iCurRow == r;
	}
	m_iChangeMask |= row.ExportOptions( vpns, bRowHasFocus );
}

void ScreenOptionsMaster::BeginFadingOut()
{
	/* If the selection is on a LIST, and the selected LIST option sets the screen,
	* honor it. */
	m_bExportWillSetANewScreen = false;

	int iCurRow = this->GetCurrentRow();
	ASSERT( iCurRow >= 0 && iCurRow < (int)m_Rows.size() );
	OptionRow &row = *m_Rows[iCurRow];

	if( row.GetRowType() != OptionRow::ROW_EXIT )
	{
		int iChoice = row.GetChoiceInRowWithFocus( GAMESTATE->m_MasterPlayerNumber );
		if( row.GetFirstItemGoesDown() )
			iChoice--;
		OptionRowHandler *pHand = OptionRowHandlers[iCurRow];
		m_bExportWillSetANewScreen = pHand->HasScreen( iChoice );
	}

	// If options set a NextScreen or one is specified in metrics, then fade out
	if( m_bExportWillSetANewScreen || NEXT_SCREEN != "" )
		ScreenOptions::BeginFadingOut();
}

void ScreenOptionsMaster::GoToNextScreen()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen( SM_None );
	else if( m_bExportWillSetANewScreen )
		;	// Do nothing.  Let Export set the screen.
	else if( NEXT_SCREEN != "" )
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
}

void ScreenOptionsMaster::GoToPrevScreen()
{
	/* XXX: A better way to handle this would be to check if we're a pushed screen. */
	if( GAMESTATE->m_bEditing )
	{
		SCREENMAN->PopTopScreen( SM_None );
		// XXX: handle different destinations based on play mode?
	}
	else
	{
		SCREENMAN->DeletePreparedScreens();
		SCREENMAN->SetNewScreen( PREV_SCREEN ); // (GAMESTATE->m_PlayMode) );
	}
}

void ScreenOptionsMaster::RefreshIcons( int r, PlayerNumber pn )
{
	OptionRow &row = *m_Rows[r];
	
	if( row.GetRowType() == OptionRow::ROW_EXIT )
		return;	// skip

	const OptionRowDefinition &def = row.GetRowDef();

	// find first selection and whether multiple are selected
	int iFirstSelection = row.GetOneSelection( pn, true );

	// set icon name
	CString sIcon;

	if( iFirstSelection == -1 )
	{
		sIcon = "Multi";
	}
	else if( iFirstSelection != -1 )
	{
		const OptionRowHandler *pHand = OptionRowHandlers[r];
		sIcon = pHand->GetIconText( def, iFirstSelection+(m_OptionsNavigation==NAV_TOGGLE_THREE_KEY?-1:0) );
	}
	

	/* XXX: hack to not display text in the song options menu */
	if( def.bOneChoiceForAllPlayers )
		sIcon = "";

	LoadOptionIcon( pn, r, sIcon );
}

void ScreenOptionsMaster::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
		{
			//
			// Override ScreenOptions's calling of ExportOptions
			//
			m_iChangeMask = 0;
		
			CHECKPOINT;

			for( unsigned r = 0; r < OptionRowHandlers.size(); ++r )
			{
				CHECKPOINT_M( ssprintf("%i/%i", r, int(OptionRowHandlers.size())) );

				vector<PlayerNumber> vpns;
				FOREACH_OptionsPlayer( p )
					vpns.push_back( p );
				ExportOptions( r, vpns );
			}

			if( m_iChangeMask & OPT_APPLY_ASPECT_RATIO )
			{
				THEME->UpdateLuaGlobals();	// This needs to be done before resetting the projection matrix below
				SCREENMAN->ThemeChanged();	// recreate ScreenSystemLayer and SharedBGA
			}

			/* If the theme changes, we need to reset RageDisplay to apply new theme 
			* window title and icon. */
			/* If the aspect ratio changes, we need to reset RageDisplay so that the 
			* projection matrix is re-created using the new screen dimensions. */
			if( (m_iChangeMask & OPT_APPLY_THEME) || 
				(m_iChangeMask & OPT_APPLY_GRAPHICS) ||
				(m_iChangeMask & OPT_APPLY_ASPECT_RATIO) )
				ApplyGraphicOptions();

			if( m_iChangeMask & OPT_SAVE_PREFERENCES )
			{
				/* Save preferences. */
				LOG->Trace("ROW_CONFIG used; saving ...");
				PREFSMAN->SaveGlobalPrefsToDisk();
				SaveGamePrefsToDisk();
			}

			if( m_iChangeMask & OPT_RESET_GAME )
			{
				ResetGame();
				m_bExportWillSetANewScreen = false;
			}

			if( m_iChangeMask & OPT_APPLY_SOUND )
			{
				SOUNDMAN->SetPrefs( PREFSMAN->m_fSoundVolume );
			}
			
			if( m_iChangeMask & OPT_APPLY_SONG )
				SONGMAN->SetPreferences();

			CHECKPOINT;
			if( !(m_iChangeMask & OPT_RESET_GAME) )
				this->GoToNextScreen();
		}
		break;
	default:
		ScreenOptions::HandleScreenMessage( SM );
		break;
	}
}

/*
 * (c) 2003-2004 Glenn Maynard
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
