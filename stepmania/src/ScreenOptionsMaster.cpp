#include "global.h"

#include "ScreenOptionsMaster.h"
#include "RageException.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "ScreenManager.h"
#include "NoteSkinManager.h"
#include "Course.h"
#include "Steps.h"
#include "Style.h"
#include "song.h"
#include "SongManager.h"
#include "Character.h"
#include "PrefsManager.h"
#include "ScreenOptionsMasterPrefs.h"
#include "GameSoundManager.h"
#include "StepMania.h"
#include "RageSoundManager.h"
#include "ProfileManager.h"
#include "StepsUtil.h"
#include "LuaManager.h"
#include "GameManager.h"
#include "Foreach.h"

#define LINE_NAMES				THEME->GetMetric (m_sName,"LineNames")
#define OPTION_MENU_FLAGS		THEME->GetMetric (m_sName,"OptionMenuFlags")
#define LINE(sLineName)			THEME->GetMetric (m_sName,ssprintf("Line%s",sLineName.c_str()))

#define ENTRY(s)				THEME->GetMetric ("ScreenOptionsMaster",s)
#define ENTRY_NAME(s)			THEME->GetMetric ("OptionNames", s)
#define ENTRY_MODE(s,i)			THEME->GetMetric ("ScreenOptionsMaster",ssprintf("%s,%i",(s).c_str(),(i+1)))
#define ENTRY_DEFAULT(s)		THEME->GetMetric ("ScreenOptionsMaster",(s) + "Default")
#define NEXT_SCREEN				THEME->GetMetric (m_sName,"NextScreen")
#define PREV_SCREEN				THEME->GetMetric (m_sName,"PrevScreen")

/* Add the list named "ListName" to the given row/handler. */
void ScreenOptionsMaster::SetList( OptionRowData &row, OptionRowHandler &hand, CString ListName )
{
	hand.type = ROW_LIST;
	hand.m_bUseModNameForIcon = true;

	row.name = ListName;
	if( !ListName.CompareNoCase("noteskins") )
	{
		hand.Default.Init(); /* none */
		row.bOneChoiceForAllPlayers = false;

		CStringArray arraySkinNames;
		NOTESKIN->GetNoteSkinNames( arraySkinNames );
		for( unsigned skin=0; skin<arraySkinNames.size(); skin++ )
		{
			arraySkinNames[skin].MakeUpper();

			GameCommand mc;
			mc.m_sModifiers = arraySkinNames[skin];
			hand.ListEntries.push_back( mc );
			row.choices.push_back( arraySkinNames[skin] );
		}
		return;
	}

	hand.Default.Load( -1, ParseCommands(ENTRY_DEFAULT(ListName)) );

	/* Parse the basic configuration metric. */
	CStringArray asParts;
	split( ENTRY(ListName), ",", asParts );
	if( asParts.size() < 1 )
		RageException::Throw( "Parse error in ScreenOptionsMasterEntries::ListName%s", ListName.c_str() );

	row.bOneChoiceForAllPlayers = false;
	const int NumCols = atoi( asParts[0] );
	for( unsigned i=0; i<asParts.size(); i++ )
	{
		if( asParts[i].CompareNoCase("together") == 0 )
			row.bOneChoiceForAllPlayers = true;
		else if( asParts[i].CompareNoCase("SelectMultiple") == 0 )
			row.type = OptionRowData::SELECT_MULTIPLE;
		else if( asParts[i].CompareNoCase("SelectNone") == 0 )
			row.type = OptionRowData::SELECT_NONE;
	}

	for( int col = 0; col < NumCols; ++col )
	{
		GameCommand mc;
		mc.Load( 0, ParseCommands(ENTRY_MODE(ListName, col)) );
		if( mc.m_sName == "" )
			RageException::Throw( "List \"%s\", col %i has no name", ListName.c_str(), col );

		if( !mc.IsPlayable() )
			continue;

		hand.ListEntries.push_back( mc );

		CString sName = mc.m_sName;
		CString sChoice = ENTRY_NAME(mc.m_sName);
		row.choices.push_back( sChoice );
	}
}

/* Add a list of difficulties/edits to the given row/handler. */
void ScreenOptionsMaster::SetSteps( OptionRowData &row, OptionRowHandler &hand )
{
	hand.type = ROW_LIST;
	row.name = "Steps";
	row.bOneChoiceForAllPlayers = false;

	// fill in difficulty names
	if( GAMESTATE->m_bEditing )
	{
		row.choices.push_back( "" );
		hand.ListEntries.push_back( GameCommand() );
	}
	else if( GAMESTATE->IsCourseMode() )   // playing a course
	{
		row.bOneChoiceForAllPlayers = PREFSMAN->m_bLockCourseDifficulties;

		vector<Trail*> vTrails;
		GAMESTATE->m_pCurCourse->GetTrails( vTrails, GAMESTATE->GetCurrentStyle()->m_StepsType );
		for( unsigned i=0; i<vTrails.size(); i++ )
		{
			Trail* pTrail = vTrails[i];

			CString s = CourseDifficultyToThemedString( pTrail->m_CourseDifficulty );
			row.choices.push_back( s );
			GameCommand mc;
			mc.m_pTrail = pTrail;
			hand.ListEntries.push_back( mc );
		}
	}
	else // !GAMESTATE->IsCourseMode(), playing a song
	{
		vector<Steps*> vSteps;
		GAMESTATE->m_pCurSong->GetSteps( vSteps, GAMESTATE->GetCurrentStyle()->m_StepsType );
		StepsUtil::SortNotesArrayByDifficulty( vSteps );
		for( unsigned i=0; i<vSteps.size(); i++ )
		{
			Steps* pSteps = vSteps[i];

			CString s;
			if( pSteps->GetDifficulty() == DIFFICULTY_EDIT )
				s = pSteps->GetDescription();
			else
				s = DifficultyToThemedString( pSteps->GetDifficulty() );
			s += ssprintf( " (%d)", pSteps->GetMeter() );

			row.choices.push_back( s );
			GameCommand mc;
			mc.m_pSteps = pSteps;
			mc.m_dc = pSteps->GetDifficulty();
			hand.ListEntries.push_back( mc );
		}
	}
}


/* Add the given configuration value to the given row/handler. */
void ScreenOptionsMaster::SetConf( OptionRowData &row, OptionRowHandler &hand, CString param )
{
	/* Configuration values are never per-player. */
	row.bOneChoiceForAllPlayers = true;
	hand.type = ROW_CONFIG;

	ConfOption *pConfOption = ConfOption::Find( param );
	if( pConfOption == NULL )
		RageException::Throw( "Invalid Conf type \"%s\"", param.c_str() );

	pConfOption->UpdateAvailableOptions();

	hand.opt = pConfOption;
	hand.opt->MakeOptionsList( row.choices );

	row.name = hand.opt->name;
}

/* Add a list of available characters to the given row/handler. */
void ScreenOptionsMaster::SetCharacters( OptionRowData &row, OptionRowHandler &hand )
{
	hand.type = ROW_LIST;
	row.bOneChoiceForAllPlayers = false;
	row.name = "Characters";
	hand.Default.m_pCharacter = GAMESTATE->GetDefaultCharacter();

	{
		row.choices.push_back( ENTRY_NAME("Off") );
		GameCommand mc;
		mc.m_pCharacter = NULL;
		hand.ListEntries.push_back( mc );
	}

	vector<Character*> apCharacters;
	GAMESTATE->GetCharacters( apCharacters );
	for( unsigned i=0; i<apCharacters.size(); i++ )
	{
		Character* pCharacter = apCharacters[i];
		CString s = pCharacter->m_sName;
		s.MakeUpper();

		row.choices.push_back( s ); 
		GameCommand mc;
		mc.m_pCharacter = pCharacter;
		hand.ListEntries.push_back( mc );
	}
}

/* Add a list of available styles to the given row/handler. */
void ScreenOptionsMaster::SetStyles( OptionRowData &row, OptionRowHandler &hand )
{
	hand.type = ROW_LIST;
	row.bOneChoiceForAllPlayers = true;
	row.name = "Style";

	vector<const Style*> vStyles;
	GAMEMAN->GetStylesForGame( GAMESTATE->m_pCurGame, vStyles );
	ASSERT( vStyles.size() );
	FOREACH_CONST( const Style*, vStyles, s )
	{
		row.choices.push_back( GAMEMAN->StyleToThemedString(*s) ); 
		GameCommand mc;
		mc.m_pStyle = *s;
		hand.ListEntries.push_back( mc );
	}

	hand.Default.m_pStyle = vStyles[0];
}

/* Add a list of available song groups to the given row/handler. */
void ScreenOptionsMaster::SetGroups( OptionRowData &row, OptionRowHandler &hand )
{
	hand.type = ROW_LIST;
	row.bOneChoiceForAllPlayers = true;
	row.name = "Group";
	hand.Default.m_sSongGroup = GROUP_ALL_MUSIC;

	vector<CString> vGroups;
	SONGMAN->GetGroupNames( vGroups );
	ASSERT( vGroups.size() );

	{
		row.choices.push_back( ENTRY_NAME("AllGroups") );
		GameCommand mc;
		mc.m_sSongGroup = GROUP_ALL_MUSIC;
		hand.ListEntries.push_back( mc );
	}

	FOREACH_CONST( CString, vGroups, g )
	{
		row.choices.push_back( *g ); 
		GameCommand mc;
		mc.m_sSongGroup = *g;
		hand.ListEntries.push_back( mc );
	}
}

/* Add a list of available difficulties to the given row/handler. */
void ScreenOptionsMaster::SetDifficulties( OptionRowData &row, OptionRowHandler &hand )
{
	set<Difficulty> vDifficulties;
	GAMESTATE->GetDifficultiesToShow( vDifficulties );

	hand.type = ROW_LIST;
	row.bOneChoiceForAllPlayers = true;
	row.name = "Difficulty";
	hand.Default.m_dc = DIFFICULTY_INVALID;

	{
		row.choices.push_back( ENTRY_NAME("AllDifficulties") );
		GameCommand mc;
		mc.m_dc = DIFFICULTY_INVALID;
		hand.ListEntries.push_back( mc );
	}

	FOREACHS_CONST( Difficulty, vDifficulties, d )
	{
		CString s = DifficultyToThemedString( *d );

		row.choices.push_back( s ); 
		GameCommand mc;
		mc.m_dc = *d;
		hand.ListEntries.push_back( mc );
	}
}

REGISTER_SCREEN_CLASS( ScreenOptionsMaster );
ScreenOptionsMaster::ScreenOptionsMaster( CString sClassName ):
	ScreenOptions( sClassName )
{
	LOG->Trace("ScreenOptionsMaster::ScreenOptionsMaster(%s)", m_sName.c_str() );

	/* If this file doesn't exist, leave the music alone (eg. ScreenPlayerOptions music sample
	 * left over from ScreenSelectMusic).  If you really want to play no music, add a redir
	 * to _silent. */
	CString MusicPath = THEME->GetPathToS( ssprintf("%s music", m_sName.c_str()), true );
	if( MusicPath != "" )
		SOUND->PlayMusic( MusicPath );

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
		Flags[i].MakeLower();

		if( Flags[i] == "together" )
			im = INPUTMODE_SHARE_CURSOR;
		if( Flags[i] == "explanations" )
			Explanations = true;
		if( Flags[i] == "forceallplayers" )
		{
			FOREACH_PlayerNumber( pn )
				GAMESTATE->m_bSideIsJoined[pn] = true;
			GAMESTATE->m_MasterPlayerNumber = PlayerNumber(0);
		}
		if( Flags[i] == "smnavigation" )
			SetNavigation( NAV_THREE_KEY_MENU );
		if( Flags[i] == "toggle" || Flags[i] == "firstchoicegoesdown" )
			SetNavigation( PREFSMAN->m_bArcadeOptionsNavigation? NAV_TOGGLE_THREE_KEY:NAV_TOGGLE_FIVE_KEY );
		if( Flags[i] == "hideunderlines" )
			bShowUnderlines = false;
	}

	m_OptionRowAlloc = new OptionRowData[asLineNames.size()];
	for( unsigned i = 0; i < asLineNames.size(); ++i )
	{
		CString sLineName = asLineNames[i];

		OptionRowData &row = m_OptionRowAlloc[i];
		
		CString sRowCommands = LINE(sLineName);
		
		Commands vCommands;
		ParseCommands( sRowCommands, vCommands );
		
		if( vCommands.v.size() < 1 )
			RageException::Throw( "Parse error in %s::Line%i", m_sName.c_str(), i+1 );

		OptionRowHandler hand;
		for( unsigned part = 0; part < vCommands.v.size(); ++part)
		{
			Command& command = vCommands.v[part];

			BeginHandleArgs;

			const CString &name = command.GetName();

			if( !name.CompareNoCase("list") )				SetList( row, hand, sArg(1) );
			else if( !name.CompareNoCase("steps") )			SetSteps( row, hand );
			else if( !name.CompareNoCase("conf") )			SetConf( row, hand, sArg(1) );
			else if( !name.CompareNoCase("characters") )	SetCharacters( row, hand );
			else if( !name.CompareNoCase("styles") )		SetStyles( row, hand );
			else if( !name.CompareNoCase("groups") )		SetGroups( row, hand );
			else if( !name.CompareNoCase("difficulties") )	SetDifficulties( row, hand );
			else
				RageException::Throw( "Unexpected type '%s' in %s::Line%i", name.c_str(), m_sName.c_str(), i );

			EndHandleArgs;
		}

		// TRICKY:  Insert a down arrow as the first choice in the row.
		if( m_OptionsNavigation == NAV_TOGGLE_THREE_KEY )
		{
			row.choices.insert( row.choices.begin(), ENTRY_NAME("NextRow") );
			hand.ListEntries.insert( hand.ListEntries.begin(), GameCommand() );
		}

		OptionRowHandlers.push_back( hand );
	}

	ASSERT( OptionRowHandlers.size() == asLineNames.size() );

	InitMenu( im, m_OptionRowAlloc, asLineNames.size(), bShowUnderlines );
}

ScreenOptionsMaster::~ScreenOptionsMaster()
{
	delete [] m_OptionRowAlloc;
}

void SelectExactlyOne( int iSelection, vector<bool> &vbSelectedOut )
{
	for( int i=0; i<(int)vbSelectedOut.size(); i++ )
		vbSelectedOut[i] = i==iSelection;
}

void ScreenOptionsMaster::ImportOption( const OptionRowData &row, const OptionRowHandler &hand, PlayerNumber pn, int rowno, vector<bool> &vbSelectedOut )
{
	/* Figure out which selection is the default. */
	switch( hand.type )
	{
	case ROW_LIST:
		{
			int FallbackOption = -1;
			bool UseFallbackOption = true;

			for( unsigned e = 0; e < hand.ListEntries.size(); ++e )
			{
				const GameCommand &mc = hand.ListEntries[e];

				vbSelectedOut[e] = false;

				if( mc.IsZero() )
				{
					/* The entry has no effect.  This is usually a default "none of the
					 * above" entry.  It will always return true for DescribesCurrentMode().
					 * It's only the selected choice if nothing else matches. */
					if( row.type != OptionRowData::SELECT_MULTIPLE )
						FallbackOption = e;
					continue;
				}

				if( row.bOneChoiceForAllPlayers )
				{
					if( mc.DescribesCurrentModeForAllPlayers() )
					{
						UseFallbackOption = false;
						if( row.type != OptionRowData::SELECT_MULTIPLE )
							SelectExactlyOne( e, vbSelectedOut );
						else
							vbSelectedOut[e] = true;
					}
				}
				else
				{
					if( mc.DescribesCurrentMode(  pn) )
					{
						UseFallbackOption = false;
						if( row.type != OptionRowData::SELECT_MULTIPLE )
							SelectExactlyOne( e, vbSelectedOut );
						else
							vbSelectedOut[e] = true;
					}
				}
			}

			if( row.type == OptionRowData::SELECT_ONE && 
				UseFallbackOption && 
				FallbackOption != -1 )
			{
				SelectExactlyOne( FallbackOption, vbSelectedOut );
			}

			return;
		}

	case ROW_CONFIG:
		{
			int iSelection = hand.opt->Get();
			SelectExactlyOne( iSelection+(m_OptionsNavigation==NAV_TOGGLE_THREE_KEY?1:0), vbSelectedOut );
			return;
		}

	default:
		ASSERT(0);
	}

	if( row.type != OptionRowData::SELECT_MULTIPLE )
	{
		// The first row ("go down") should not be selected.
		ASSERT( !vbSelectedOut[0] );

		// there should be exactly one option selected
		int iNumSelected = 0;
		for( unsigned e = 1; e < hand.ListEntries.size(); ++e )
			if( vbSelectedOut[e] )
				iNumSelected++;
		ASSERT( iNumSelected == 1 );
	}
}

void ScreenOptionsMaster::ImportOptions()
{
	for( unsigned i = 0; i < OptionRowHandlers.size(); ++i )
	{
		const OptionRowHandler &hand = OptionRowHandlers[i];
		const OptionRowData &data = m_OptionRowAlloc[i];
		Row &row = *m_Rows[i];

		if( data.bOneChoiceForAllPlayers )
		{
			ImportOption(data, hand, PLAYER_1, i, row.m_vbSelected[0] );
		}
		else
		{
			FOREACH_HumanPlayer( p )
			{
				ImportOption( data, hand, p, i, row.m_vbSelected[p] );
			}
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
		const OptionRowHandler &hand = OptionRowHandlers[i];
		const OptionRowData &data = m_OptionRowAlloc[i];
		Row &row = *m_Rows[i];

		if( data.bOneChoiceForAllPlayers )
			continue;
		ImportOption( data, hand, pn, i, row.m_vbSelected[pn] );
	}
}

int GetOneSelection( const vector<bool> &vbSelected )
{
	for( unsigned i=0; i<vbSelected.size(); i++ )
		if( vbSelected[i] )
			return i;
	ASSERT(0);	// shouldn't call this if not expecting one to be selected
	return -1;
}

/* Returns an OPT mask. */
int ScreenOptionsMaster::ExportOption( const OptionRowData &row, const OptionRowHandler &hand, PlayerNumber pn, const vector<bool> &vbSelected )
{
	/* Figure out which selection is the default. */
	switch( hand.type )
	{
	case ROW_LIST:
		{
			hand.Default.Apply( pn );
			for( unsigned i=0; i<vbSelected.size(); i++ )
				if( vbSelected[i] )
					hand.ListEntries[i].Apply( pn );
		}
		break;

	case ROW_CONFIG:
		{
			int sel = GetOneSelection(vbSelected) - (m_OptionsNavigation==NAV_TOGGLE_THREE_KEY?1:0);

			/* Get the original choice. */
			int Original = hand.opt->Get();

			/* Apply. */
			hand.opt->Put( sel );

			/* Get the new choice. */
			int New = hand.opt->Get();

			/* If it didn't change, don't return any side-effects. */
			if( Original == New )
				return 0;

			return hand.opt->GetEffects();
		}
		break;

	default:
		ASSERT(0);
		break;
	}
	return 0;
}

void ScreenOptionsMaster::ExportOptions()
{
	int ChangeMask = 0;

	CHECKPOINT;
	for( unsigned i = 0; i < OptionRowHandlers.size(); ++i )
	{
		CHECKPOINT_M( ssprintf("%i/%i", i, int(OptionRowHandlers.size())) );
		
		const OptionRowHandler &hand = OptionRowHandlers[i];
		const OptionRowData &data = m_OptionRowAlloc[i];
		Row &row = *m_Rows[i];

		FOREACH_HumanPlayer( p )
		{
			vector<bool> &vbSelected = row.m_vbSelected[p];

			ChangeMask |= ExportOption( data, hand, p, vbSelected );
		}
	}

	CHECKPOINT;
	/* If the selection is on a LIST, and the selected LIST option sets the screen,
	 * honor it. */
	m_sNextScreen = "";

	const int row = this->GetCurrentRow();
	if( row != -1 )
	{
		const OptionRowHandler &hand = OptionRowHandlers[row];
		if( hand.type == ROW_LIST )
		{
			const int choice = m_Rows[row]->m_iChoiceInRowWithFocus[GAMESTATE->m_MasterPlayerNumber];
			const GameCommand &mc = hand.ListEntries[choice];
			if( mc.m_sScreen != "" )
				m_sNextScreen = mc.m_sScreen;
			// Why were we re-applying his here?  ExportOption() is where options
			// are applied.
			// mc.Apply( GAMESTATE->m_MasterPlayerNumber );
		}
	}
	CHECKPOINT;

	// NEXT_SCREEN;
	if( m_sNextScreen == "" )
		m_sNextScreen = NEXT_SCREEN;

	if( ChangeMask & OPT_APPLY_ASPECT_RATIO )
	{
		THEME->UpdateLuaGlobals();	// This needs to be done before resetting the projection matrix below
		SCREENMAN->ThemeChanged();	// recreate ScreenSystemLayer and SharedBGA
	}

	/* If the theme changes, we need to reset RageDisplay to apply new theme 
	 * window title and icon. */
	/* If the aspect ratio changes, we need to reset RageDisplay so that the 
	 * projection matrix is re-created using the new screen dimensions. */
	if( (ChangeMask & OPT_APPLY_THEME) || 
		(ChangeMask & OPT_APPLY_GRAPHICS) ||
		(ChangeMask & OPT_APPLY_ASPECT_RATIO) )
		ApplyGraphicOptions();

	if( ChangeMask & OPT_SAVE_PREFERENCES )
	{
		/* Save preferences. */
		LOG->Trace("ROW_CONFIG used; saving ...");
		PREFSMAN->SaveGlobalPrefsToDisk();
		SaveGamePrefsToDisk();
	}

	if( ChangeMask & OPT_RESET_GAME )
	{
		ResetGame();
		m_sNextScreen = "";
	}

	if( ChangeMask & OPT_APPLY_SOUND )
	{
		SOUNDMAN->SetPrefs( PREFSMAN->m_fSoundVolume );
	}
	
	if( ChangeMask & OPT_APPLY_SONG )
		SONGMAN->SetPreferences();

	CHECKPOINT;
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
			if( m_Rows[i]->Type == Row::ROW_EXIT )
				continue;	// skip

			Row &row = *m_Rows[i];
			const OptionRowData &data = row.m_RowDef;

			// find first selection and whether multiple are selected
			int iFirstSelection = -1;
			bool bMultipleSelected = false;
			for( unsigned j=0; j<row.m_vbSelected[p].size(); j++ )
			{
				if( row.m_vbSelected[p][j] )
				{
					if( iFirstSelection != -1 )
						bMultipleSelected = true;
					else
						iFirstSelection = j;
				}

			}

			// set icon name
			CString sIcon;

			if( bMultipleSelected )
			{
				sIcon = "Multi";
			}
			else if( iFirstSelection != -1 )
			{
				const OptionRowHandler &handler = OptionRowHandlers[i];
				switch( handler.type )
				{
				case ROW_LIST:
					sIcon = handler.m_bUseModNameForIcon ?
						handler.ListEntries[iFirstSelection].m_sModifiers :
						data.choices[iFirstSelection];
					break;
				case ROW_CONFIG:
					break;
				}
			}
			

			/* XXX: hack to not display text in the song options menu */
			if( data.bOneChoiceForAllPlayers )
				sIcon = "";

			LoadOptionIcon( p, i, sIcon );
		}
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
