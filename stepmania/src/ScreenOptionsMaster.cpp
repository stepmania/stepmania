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

// keep this in sync with OptionRowHandler.cpp
#define ENTRY_NAME(s)				THEME->GetMetric ("OptionNames", s)

#define NEXT_SCREEN					THEME->GetMetric (m_sName,"NextScreen")
#define PREV_SCREEN					THEME->GetMetric (m_sName,"PrevScreen")

const CString NEXT_ROW_NAME = "NextRow";


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
	bool bShowUnderlines = true;
	
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
		else if( sFlag == "hideunderlines" )
			bShowUnderlines = false;
		else
			RageException::Throw( "Unknown flag \"%s\"", sFlag.c_str() );
	}

	m_OptionRowAlloc.resize( asLineNames.size() );
	OptionRowHandlers.resize( asLineNames.size() );
	for( unsigned i = 0; i < asLineNames.size(); ++i )
	{
		CString sLineName = asLineNames[i];
		OptionRowDefinition &def = m_OptionRowAlloc[i];
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


		// TRICKY:  Insert a down arrow as the first choice in the row.
		if( m_OptionsNavigation == NAV_TOGGLE_THREE_KEY )
			def.choices.insert( def.choices.begin(), ENTRY_NAME(NEXT_ROW_NAME) );
	}

	ASSERT( OptionRowHandlers.size() == asLineNames.size() );

	InitMenu( im, &m_OptionRowAlloc[0], asLineNames.size(), bShowUnderlines );
}

ScreenOptionsMaster::~ScreenOptionsMaster()
{
	FOREACH( OptionRowHandler*, OptionRowHandlers, h )
		SAFE_DELETE( *h );
	OptionRowHandlers.clear();
}

void ScreenOptionsMaster::Update( float fDelta )
{
	if( m_bFirstUpdate )
	{
		/*
		 * Don't play sounds during the ctor, since derived classes havn't loaded yet.
		 * If this file doesn't exist, leave the music alone (eg. ScreenPlayerOptions music sample
		 * left over from ScreenSelectMusic).  If you really want to play no music, add a redir
		 * to _silent.
		 */
		CString MusicPath = THEME->GetPathS( m_sName, "music", true );
		if( MusicPath != "" )
			SOUND->PlayMusic( MusicPath );
	}

	ScreenOptions::Update( fDelta );
}

//if( row.selectType != SELECT_MULTIPLE )
//{
//	// The first row ("go down") should not be selected.
//	ASSERT( !vbSelectedOut[0] );

//	// there should be exactly one option selected
//	int iNumSelected = 0;
//	for( unsigned e = 1; e < pHand->ListEntries.size(); ++e )
//		if( vbSelectedOut[e] )
//			iNumSelected++;
//	ASSERT( iNumSelected == 1 );
//}

/* Hack: the NextRow entry is never set, and should be transparent.  Remove
 * it, and readd it below. */
#define ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( vbSelected ) \
	if( m_OptionsNavigation == NAV_TOGGLE_THREE_KEY ) \
		vbSelected.erase( vbSelected.begin() );
#define INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( vbSelected ) \
	if( m_OptionsNavigation == NAV_TOGGLE_THREE_KEY ) \
		vbSelected.insert( vbSelected.begin(), false );


void ScreenOptionsMaster::ImportOptions( int r )
{
	const OptionRowHandler *pHand = OptionRowHandlers[r];
	const OptionRowDefinition &def = m_OptionRowAlloc[r];
	OptionRow &row = *m_Rows[r];

	if( def.bOneChoiceForAllPlayers )
	{
		ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( row.m_vbSelected[0] );
		pHand->ImportOption(def, PLAYER_1, row.m_vbSelected[0] );
		INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( row.m_vbSelected[0] );
	}
	else
	{
		FOREACH_HumanPlayer( p )
		{
			ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( row.m_vbSelected[p] );
			pHand->ImportOption( def, p, row.m_vbSelected[p] );
			INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( row.m_vbSelected[p] );
		}
	}
}

/* Import only settings specific to the given player. */
void ScreenOptionsMaster::ImportOptionsForPlayer( PlayerNumber pn )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;

	for( unsigned i = 0; i < OptionRowHandlers.size(); ++i )
	{
		const OptionRowHandler *pHand = OptionRowHandlers[i];
		const OptionRowDefinition &def = m_OptionRowAlloc[i];
		OptionRow &row = *m_Rows[i];

		if( def.bOneChoiceForAllPlayers )
			continue;
		pHand->ImportOption( def, pn, row.m_vbSelected[pn] );
	}
}

void ScreenOptionsMaster::ExportOptions( int iRow )
{
	const OptionRowHandler *pHand = OptionRowHandlers[iRow];
	const OptionRowDefinition &def = m_OptionRowAlloc[iRow];
	OptionRow &row = *m_Rows[iRow];

	if( def.bOneChoiceForAllPlayers )
	{
		ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( row.m_vbSelected[0] );
		m_iChangeMask |= pHand->ExportOption( def, PLAYER_1, row.m_vbSelected[0] );
		INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( row.m_vbSelected[0] );
	}
	else
	{
		FOREACH_HumanPlayer( p )
		{
			ERASE_ONE_BOOL_AT_FRONT_IF_NEEDED( row.m_vbSelected[p] );
			m_iChangeMask |= pHand->ExportOption( def, p, row.m_vbSelected[p] );
			INSERT_ONE_BOOL_AT_FRONT_IF_NEEDED( row.m_vbSelected[p] );
		}
	}
}

void ScreenOptionsMaster::GoToNextScreen()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else if( m_sNextScreen != "" )
		SCREENMAN->SetNewScreen( m_sNextScreen );
}

void ScreenOptionsMaster::GoToPrevScreen()
{
	/* XXX: A better way to handle this would be to check if we're a pushed screen. */
	if( GAMESTATE->m_bEditing )
	{
		SCREENMAN->PopTopScreen();
		// XXX: handle different destinations based on play mode?
	}
	else
	{
		SCREENMAN->DeletePreparedScreens();
		SCREENMAN->SetNewScreen( PREV_SCREEN ); // (GAMESTATE->m_PlayMode) );
	}
}

void ScreenOptionsMaster::RefreshIcons()
{
	FOREACH_HumanPlayer( p )
	{
        for( unsigned i=0; i<m_Rows.size(); ++i )     // foreach options line
		{
			if( m_Rows[i]->GetRowType() == OptionRow::ROW_EXIT )
				continue;	// skip

			OptionRow &row = *m_Rows[i];
			const OptionRowDefinition &def = row.GetRowDef();

			// find first selection and whether multiple are selected
			int iFirstSelection = row.GetOneSelection( p, true );

			// set icon name
			CString sIcon;

			if( iFirstSelection == -1 )
			{
				sIcon = "Multi";
			}
			else if( iFirstSelection != -1 )
			{
				const OptionRowHandler *pHand = OptionRowHandlers[i];
				sIcon = pHand->GetIconText( def, iFirstSelection+(m_OptionsNavigation==NAV_TOGGLE_THREE_KEY?-1:0) );
			}
			

			/* XXX: hack to not display text in the song options menu */
			if( def.bOneChoiceForAllPlayers )
				sIcon = "";

			LoadOptionIcon( p, i, sIcon );
		}
	}
}

void ScreenOptionsMaster::ChangeValueInRow( PlayerNumber pn, int iDelta, bool Repeat )
{
	ScreenOptions::ChangeValueInRow( pn, iDelta, Repeat );

	int iRow = m_iCurrentRow[pn];

	const OptionRowHandler *pHand = OptionRowHandlers[iRow];

	FOREACH_CONST( CString, pHand->m_vsRefreshRowNames, sRowToRefreshName )
	{
		for( unsigned r=0; r<m_Rows.size(); r++ )
		{
			OptionRow &rowOther = *m_Rows[r];

			if( rowOther.GetRowType() == OptionRow::ROW_EXIT )
				continue;

			OptionRowHandler *pHandOther = OptionRowHandlers[r];
			OptionRowDefinition &defOther = m_OptionRowAlloc[r];

			if( *sRowToRefreshName == pHandOther->m_sName )
			{
				pHandOther->Reload( defOther );
				ScreenOptions::RefreshRowChoices( r, defOther );
			}
		}
	}
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

			const unsigned uFocusRow = this->GetCurrentRow();
			/* If the selection is on a LIST, and the selected LIST option sets the screen,
			* honor it. */
			m_sNextScreen = "";

			for( unsigned r = 0; r < OptionRowHandlers.size(); ++r )
			{
				OptionRow &row = *m_Rows[r];

				CHECKPOINT_M( ssprintf("%i/%i", r, int(OptionRowHandlers.size())) );
				
				/* If SELECT_NONE, only apply it if it's the selected option. */
				const OptionRowDefinition &def = m_OptionRowAlloc[r];
				if( def.selectType == SELECT_NONE && r != uFocusRow )
					continue;

				OptionRowHandler *pHand = OptionRowHandlers[r];

				const int iChoice = row.m_iChoiceInRowWithFocus[GAMESTATE->m_MasterPlayerNumber];
				CString sScreen = pHand->GetAndEraseScreen( iChoice );
				if( !sScreen.empty() )
					m_sNextScreen = sScreen;

				ExportOptions( r );
			}
			CHECKPOINT;

			// NEXT_SCREEN;
			if( m_sNextScreen == "" )
				m_sNextScreen = NEXT_SCREEN;

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
				m_sNextScreen = "";
			}

			if( m_iChangeMask & OPT_APPLY_SOUND )
			{
				SOUNDMAN->SetPrefs( PREFSMAN->m_fSoundVolume );
			}
			
			if( m_iChangeMask & OPT_APPLY_SONG )
				SONGMAN->SetPreferences();

			CHECKPOINT;
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
