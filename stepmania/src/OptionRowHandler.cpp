#include "global.h"
#include "OptionRowHandler.h"
#include "LuaManager.h"
#include "ScreenOptionsMasterPrefs.h"
#include "NoteSkinManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "GameState.h"
#include "Course.h"
#include "Steps.h"
#include "Style.h"
#include "song.h"
#include "SongManager.h"
#include "Character.h"
#include "PrefsManager.h"
#include "SongUtil.h"
#include "StepsUtil.h"
#include "GameManager.h"
#include "Foreach.h"
#include "GameSoundManager.h"
#include "CommonMetrics.h"
#include "CharacterManager.h"
#include "ScreenManager.h"
#include "ScreenMiniMenu.h"	// for MenuRowDef
#include "FontCharAliases.h"

#define ENTRY(s)		THEME->GetMetric ("ScreenOptionsMaster",s)
#define ENTRY_MODE(s,i)		THEME->GetMetric ("ScreenOptionsMaster",ssprintf("%s,%i",(s).c_str(),(i+1)))
#define ENTRY_DEFAULT(s)	THEME->GetMetric ("ScreenOptionsMaster",(s) + "Default")

static const char *SelectTypeNames[] = {
	"SelectOne",
	"SelectMultiple",
	"SelectNone",
};
XToString( SelectType, NUM_SELECT_TYPES );
StringToX( SelectType );

static const char *LayoutTypeNames[] = {
	"ShowAllInRow",
	"ShowOneInRow",
};
XToString( LayoutType, NUM_LAYOUT_TYPES );
StringToX( LayoutType );

RString OptionRowHandler::OptionTitle() const
{
	bool bTheme = false;
	
	// HACK: Always theme the NEXT_ROW and EXIT items, even if metrics says not to theme.
	if( m_Def.m_bAllowThemeTitle )
		bTheme = true;

	RString s = m_Def.m_sName;
	if( s.empty() )
		return s;

	return bTheme ? THEME->GetString("OptionTitles",s) : s;
}

RString OptionRowHandler::GetThemedItemText( int iChoice ) const
{
	RString s = m_Def.m_vsChoices[iChoice];
	if( s == "" )
		return "";
	bool bTheme = false;
	
	if( m_Def.m_bAllowThemeItems )	bTheme = true;

	// Items beginning with a pipe mean "don't theme".
	// This allows us to disable theming on a per-choice basis for choice names that are just a number
	// and don't need to be localized.
	if( s[0] == '|' )
	{
		s.erase( s.begin() );
		bTheme = false;
	}

	if( bTheme ) 
		s = CommonMetrics::LocalizeOptionItem( s, false ); 
	return s;
}

void OptionRowHandler::GetIconTextAndGameCommand( int iFirstSelection, RString &sIconTextOut, GameCommand &gcOut ) const
{
	sIconTextOut = "";
	gcOut.Init();
}

void OptionRowHandlerUtil::SelectExactlyOne( int iSelection, vector<bool> &vbSelectedOut )
{
	ASSERT_M( iSelection >= 0  &&  iSelection < (int) vbSelectedOut.size(),
			  ssprintf("%d/%u",iSelection, unsigned(vbSelectedOut.size())) );
	for( int i=0; i<int(vbSelectedOut.size()); i++ )
		vbSelectedOut[i] = i==iSelection;
}

int OptionRowHandlerUtil::GetOneSelection( const vector<bool> &vbSelected )
{
	int iRet = -1;
	for( unsigned i=0; i<vbSelected.size(); i++ )
	{
		if( vbSelected[i] )
		{
			ASSERT( iRet == -1 );	// only one should be selected
			iRet = i;
		}
	}
	ASSERT( iRet != -1 );	// shouldn't call this if not expecting one to be selected
	return iRet;
}

static LocalizedString OFF ( "OptionRowHandler", "Off" );

class OptionRowHandlerList : public OptionRowHandler
{
public:
	vector<GameCommand> m_aListEntries;
	GameCommand m_Default;
	bool m_bUseModNameForIcon;
	vector<RString> m_vsBroadcastOnExport;

	OptionRowHandlerList() { Init(); }
	virtual void Init()
	{
		OptionRowHandler::Init();
		m_aListEntries.clear();
		m_Default.Init();
		m_bUseModNameForIcon = false;
		m_vsBroadcastOnExport.clear();
	}
	virtual void LoadInternal( const Commands &cmds )
	{
		ASSERT( cmds.v.size() == 1 );
		const Command &command = cmds.v[0];
		RString sParam = command.GetArg(1);
		ASSERT( command.m_vsArgs.size() == 2 );
		ASSERT( sParam.size() );

		m_bUseModNameForIcon = true;
			
		m_Def.m_sName = sParam;

		m_Default.Load( -1, ParseCommands(ENTRY_DEFAULT(sParam)) );

		{
			/* Parse the basic configuration metric. */
			Commands cmds = ParseCommands( ENTRY(sParam) );
			if( cmds.v.size() < 1 )
				RageException::Throw( "Parse error in \"ScreenOptionsMaster::%s\".", sParam.c_str() );

			m_Def.m_bOneChoiceForAllPlayers = false;
			const int NumCols = atoi( cmds.v[0].m_vsArgs[0] );
			for( unsigned i=1; i<cmds.v.size(); i++ )
			{
				const Command &cmd = cmds.v[i];
				RString sName = cmd.GetName();

				if(	 sName == "together" )		m_Def.m_bOneChoiceForAllPlayers = true;
				else if( sName == "selectmultiple" )	m_Def.m_selectType = SELECT_MULTIPLE;
				else if( sName == "selectone" )		m_Def.m_selectType = SELECT_ONE;
				else if( sName == "selectnone" )	m_Def.m_selectType = SELECT_NONE;
				else if( sName == "showoneinrow" )	m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
				else if( sName == "reloadrowmessages" )
				{
					for( unsigned a=1; a<cmd.m_vsArgs.size(); a++ )
						m_vsReloadRowMessages.push_back( cmd.m_vsArgs[a] );
				}
				else if( sName == "enabledforplayers" )
				{
					m_Def.m_vEnabledForPlayers.clear();
					for( unsigned a=1; a<cmd.m_vsArgs.size(); a++ )
					{
						RString sArg = cmd.m_vsArgs[a];
						PlayerNumber pn = (PlayerNumber)(atoi(sArg)-1);
						ASSERT( pn >= 0 && pn < NUM_PLAYERS );
						m_Def.m_vEnabledForPlayers.insert( pn );
					}
				}
				else if( sName == "exportonchange" )	m_Def.m_bExportOnChange = true;
				else if( sName == "broadcastonexport" )
				{
					for( unsigned i=1; i<cmd.m_vsArgs.size(); i++ )
						m_vsBroadcastOnExport.push_back( cmd.m_vsArgs[i] );
				}
				else	RageException::Throw( "Unkown row flag \"%s\".", sName.c_str() );
			}

			for( int col = 0; col < NumCols; ++col )
			{
				GameCommand mc;
				mc.ApplyCommitsScreens( false );
				mc.Load( 0, ParseCommands(ENTRY_MODE(sParam, col)) );
				/* If the row has just one entry, use the name of the row as the name of the
				 * entry.  If it has more than one, each one must be specified explicitly. */
				if( mc.m_sName == "" && NumCols == 1 )
					mc.m_sName = sParam;
				if( mc.m_sName == "" )
					RageException::Throw( "List \"%s\", col %i has no name.", sParam.c_str(), col );

				if( !mc.IsPlayable() )
				{
					LOG->Trace( "\"%s\" is not playable.", sParam.c_str() );
					continue;
				}

				m_aListEntries.push_back( mc );

				RString sName = mc.m_sName;
				RString sChoice = mc.m_sName;
				m_Def.m_vsChoices.push_back( sChoice );
			}
		}
	}
	void ImportOption( const vector<PlayerNumber> &vpns, vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
		FOREACH_CONST( PlayerNumber, vpns, pn )
		{
			PlayerNumber p = *pn;
			vector<bool> &vbSelOut = vbSelectedOut[p];

			int iFallbackOption = -1;
			bool bUseFallbackOption = true;

			for( unsigned e = 0; e < m_aListEntries.size(); ++e )
			{
				const GameCommand &mc = m_aListEntries[e];

				vbSelOut[e] = false;

				if( mc.IsZero() )
				{
					/* The entry has no effect.  This is usually a default "none of the
					 * above" entry.  It will always return true for DescribesCurrentMode().
					 * It's only the selected choice if nothing else matches. */
					if( m_Def.m_selectType != SELECT_MULTIPLE )
						iFallbackOption = e;
					continue;
				}

				if( m_Def.m_bOneChoiceForAllPlayers )
				{
					if( mc.DescribesCurrentModeForAllPlayers() )
					{
						bUseFallbackOption = false;
						if( m_Def.m_selectType != SELECT_MULTIPLE )
							OptionRowHandlerUtil::SelectExactlyOne( e, vbSelOut );
						else
							vbSelOut[e] = true;
					}
				}
				else
				{
					if( mc.DescribesCurrentMode( p) )
					{
						bUseFallbackOption = false;
						if( m_Def.m_selectType != SELECT_MULTIPLE )
							OptionRowHandlerUtil::SelectExactlyOne( e, vbSelOut );
						else
							vbSelOut[e] = true;
					}
				}
			}

			if( m_Def.m_selectType == SELECT_ONE && bUseFallbackOption )
			{
				if( iFallbackOption == -1 )
				{
					RString s = ssprintf("No options in row \"list,%s\" were selected, and no fallback row found; selected entry 0", m_Def.m_sName.c_str());
					LOG->Warn( s );
					CHECKPOINT_M( s );
					iFallbackOption = 0;
				}

				OptionRowHandlerUtil::SelectExactlyOne( iFallbackOption, vbSelOut );
			}

			VerifySelected( m_Def.m_selectType, vbSelOut, m_Def.m_sName );
		}
	}

	int ExportOption( const vector<PlayerNumber> &vpns, const vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		FOREACH_CONST( PlayerNumber, vpns, pn )
		{
			PlayerNumber p = *pn;
			const vector<bool> &vbSel = vbSelected[p];
		
			m_Default.Apply( p );
			for( unsigned i=0; i<vbSel.size(); i++ )
			{
				if( vbSel[i] )
					m_aListEntries[i].Apply( p );
			}
		}
		FOREACH_CONST( RString, m_vsBroadcastOnExport, s )
			MESSAGEMAN->Broadcast( *s );
		return 0;
	}

	virtual void GetIconTextAndGameCommand( int iFirstSelection, RString &sIconTextOut, GameCommand &gcOut ) const
	{
		sIconTextOut = m_bUseModNameForIcon ?
			m_aListEntries[iFirstSelection].m_sPreferredModifiers :
			m_Def.m_vsChoices[iFirstSelection];

		gcOut = m_aListEntries[iFirstSelection];
	}
	virtual RString GetScreen( int iChoice ) const
	{ 
		const GameCommand &gc = m_aListEntries[iChoice];
		return gc.m_sScreen;
	}

	virtual ReloadChanged Reload()
	{
		// HACK: always reload "speed", to update the BPM text in the name of the speed line
		if( !m_Def.m_sName.CompareNoCase("speed") )
			return RELOAD_CHANGED_ALL;

		return OptionRowHandler::Reload();
	}
};

class OptionRowHandlerListNoteSkins : public OptionRowHandlerList
{
	virtual void LoadInternal( const Commands &cmds )
	{
		m_Def.m_sName = "NoteSkins";
		m_Def.m_bOneChoiceForAllPlayers = false;
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself

		vector<RString> arraySkinNames;
		NOTESKIN->GetNoteSkinNames( arraySkinNames );
		for( unsigned skin=0; skin<arraySkinNames.size(); skin++ )
		{
			GameCommand mc;
			mc.m_sPreferredModifiers = arraySkinNames[skin];
			m_aListEntries.push_back( mc );
			m_Def.m_vsChoices.push_back( arraySkinNames[skin] );
		}
	}
};

// XXX: very similar to OptionRowHandlerSteps
class OptionRowHandlerListSteps : public OptionRowHandlerList
{
	virtual void LoadInternal( const Commands &cmds )
	{
		m_Def.m_sName = "Steps";
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself

		Reload();

		// don't call default
		// OptionRowHandlerList::LoadInternal( cmds );
	}

	virtual ReloadChanged Reload()
	{
		m_Def.m_vsChoices.clear();
		m_aListEntries.clear();

		// fill in difficulty names
		if( GAMESTATE->IsEditing() )
		{
			m_Def.m_vsChoices.push_back( "" );
			m_aListEntries.push_back( GameCommand() );
		}
		else if( GAMESTATE->IsCourseMode() && GAMESTATE->m_pCurCourse )   // playing a course
		{
			m_Def.m_bOneChoiceForAllPlayers = (bool)PREFSMAN->m_bLockCourseDifficulties;

			vector<Trail*> vTrails;
			GAMESTATE->m_pCurCourse->GetTrails( vTrails, GAMESTATE->GetCurrentStyle()->m_StepsType );
			for( unsigned i=0; i<vTrails.size(); i++ )
			{
				Trail* pTrail = vTrails[i];

				RString s = CourseDifficultyToLocalizedString( pTrail->m_CourseDifficulty );
				s += ssprintf( " %d", pTrail->GetMeter() );
				m_Def.m_vsChoices.push_back( s );
				GameCommand mc;
				mc.m_pTrail = pTrail;
				m_aListEntries.push_back( mc );
			}
		}
		else if( GAMESTATE->m_pCurSong ) // playing a song
		{
			vector<Steps*> vpSteps;
			Song *pSong = GAMESTATE->m_pCurSong;
			SongUtil::GetSteps( pSong, vpSteps, GAMESTATE->GetCurrentStyle()->m_StepsType );
			StepsUtil::RemoveLockedSteps( pSong, vpSteps );
			StepsUtil::SortNotesArrayByDifficulty( vpSteps );
			for( unsigned i=0; i<vpSteps.size(); i++ )
			{
				Steps* pSteps = vpSteps[i];

				RString s;
				if( pSteps->GetDifficulty() == DIFFICULTY_EDIT )
					s = pSteps->GetDescription();
				else
					s = DifficultyToLocalizedString( pSteps->GetDifficulty() );
				s += ssprintf( " %d", pSteps->GetMeter() );
				m_Def.m_vsChoices.push_back( s );
				GameCommand mc;
				mc.m_pSteps = pSteps;
				mc.m_dc = pSteps->GetDifficulty();
				m_aListEntries.push_back( mc );
			}
		}
		else
		{
			/* We have neither a song nor a course.  We may be preloading the screen
			 * for future use. */
			m_Def.m_vsChoices.push_back( "n/a" );
			m_aListEntries.push_back( GameCommand() );
		}

		return RELOAD_CHANGED_ALL;
	}
};

class OptionRowHandlerSteps : public OptionRowHandler
{
public:
	BroadcastOnChangePtr<Steps> *m_ppStepsToFill;
	BroadcastOnChange<Difficulty> *m_pDifficultyToFill;
	const BroadcastOnChange<StepsType> *m_pst;
	vector<Steps*> m_vSteps;
	vector<Difficulty> m_vDifficulties;

	OptionRowHandlerSteps() { Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		m_ppStepsToFill = NULL;
		m_pDifficultyToFill = NULL;
		m_vSteps.clear();
		m_vDifficulties.clear();
	}

	virtual void LoadInternal( const Commands &cmds )
	{
		ASSERT( cmds.v.size() == 1 );
		const Command &command = cmds.v[0];
		RString sParam = command.GetArg(1);
		ASSERT( command.m_vsArgs.size() == 2 );
		ASSERT( sParam.size() );

		if( sParam == "EditSteps" )
		{
			m_ppStepsToFill = &GAMESTATE->m_pCurSteps[0];
			m_pDifficultyToFill = &GAMESTATE->m_PreferredDifficulty[0];
			m_pst = &GAMESTATE->m_stEdit;
			m_vsReloadRowMessages.push_back( MessageToString(Message_EditStepsTypeChanged) );
		}
		else if( sParam == "EditSourceSteps" )
		{
			m_ppStepsToFill = &GAMESTATE->m_pEditSourceSteps;
			m_pst = &GAMESTATE->m_stEditSource;
			m_vsReloadRowMessages.push_back( MessageToString(Message_EditSourceStepsTypeChanged) );
			if( GAMESTATE->m_pCurSteps[0].Get() != NULL )
				m_Def.m_vEnabledForPlayers.clear();	// hide row
		}
		else
		{
			RageException::Throw( "Invalid StepsType param \"%s\".", sParam.c_str() );
		}
		
		m_Def.m_sName = sParam;
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		m_Def.m_bExportOnChange = true;
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself
		m_vsReloadRowMessages.push_back( MessageToString(Message_CurrentSongChanged) );

		m_vDifficulties.clear();
		m_vSteps.clear();

		if( GAMESTATE->m_pCurSong )
		{
			FOREACH_Difficulty( dc )
			{
				if( dc == DIFFICULTY_EDIT )
					continue;
				m_vDifficulties.push_back( dc );
				Steps* pSteps = SongUtil::GetStepsByDifficulty( GAMESTATE->m_pCurSong, *m_pst, dc );
				m_vSteps.push_back( pSteps );
			}
			SongUtil::GetSteps( GAMESTATE->m_pCurSong, m_vSteps, *m_pst, DIFFICULTY_EDIT );
			m_vDifficulties.resize( m_vSteps.size(), DIFFICULTY_EDIT );

			if( sParam == "EditSteps" )
			{
				m_vSteps.push_back( NULL );
				m_vDifficulties.push_back( DIFFICULTY_EDIT );
			}

			for( unsigned i=0; i<m_vSteps.size(); i++ )
			{
				Steps* pSteps = m_vSteps[i];
				Difficulty dc = m_vDifficulties[i];

				RString s;
				if( dc == DIFFICULTY_EDIT )
				{
					if( pSteps )
						s = pSteps->GetDescription();
					else
						s = "NewEdit";
				}
				else
				{
					s = DifficultyToLocalizedString( dc );
				}
				m_Def.m_vsChoices.push_back( s );
			}
		}
		else
		{
			m_vDifficulties.push_back( DIFFICULTY_EDIT );
			m_vSteps.push_back( NULL );
			m_Def.m_vsChoices.push_back( "none" );
		}

		if( m_pDifficultyToFill )
			m_pDifficultyToFill->Set( m_vDifficulties[0] );
		m_ppStepsToFill->Set( m_vSteps[0] );
	}
	virtual void ImportOption( const vector<PlayerNumber> &vpns, vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
		FOREACH_CONST( PlayerNumber, vpns, pn )
		{
			PlayerNumber p = *pn;
			vector<bool> &vbSelOut = vbSelectedOut[p];

			ASSERT( m_vSteps.size() == vbSelOut.size() );

			// look for matching steps
			vector<Steps*>::const_iterator iter = find( m_vSteps.begin(), m_vSteps.end(), m_ppStepsToFill->Get() );
			if( iter != m_vSteps.end() )
			{
				unsigned i = iter - m_vSteps.begin();
				vbSelOut[i] = true;
				return;
			}
			// look for matching difficulty
			if( m_pDifficultyToFill )
			{
				FOREACH_CONST( Difficulty, m_vDifficulties, d )
				{
					unsigned i = d - m_vDifficulties.begin();
					if( *d == GAMESTATE->m_PreferredDifficulty[0] )
					{
						vbSelOut[i] = true;
						vector<PlayerNumber> v;
						v.push_back( p );
						ExportOption( v, vbSelectedOut );	// current steps changed
						continue;
					}
				}
			}
			// default to 1st
			vbSelOut[0] = true;
		}
	}
	virtual int ExportOption( const vector<PlayerNumber> &vpns, const vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		FOREACH_CONST( PlayerNumber, vpns, pn )
		{
			PlayerNumber p = *pn;
			const vector<bool> &vbSel = vbSelected[p];

			int index = OptionRowHandlerUtil::GetOneSelection( vbSel );
			Difficulty dc = m_vDifficulties[index];
			Steps *pSteps = m_vSteps[index];
			if( m_pDifficultyToFill )
				m_pDifficultyToFill->Set( dc );
			m_ppStepsToFill->Set( pSteps );
		}

		return 0;
	}
};

class OptionRowHandlerListCharacters: public OptionRowHandlerList
{
	virtual void LoadInternal( const Commands &cmds )
	{
		m_Def.m_bOneChoiceForAllPlayers = false;
		m_Def.m_bAllowThemeItems = false;
		m_Def.m_sName = "Characters";
		m_Default.m_pCharacter = CHARMAN->GetDefaultCharacter();

		{
			m_Def.m_vsChoices.push_back( OFF );
			GameCommand mc;
			mc.m_pCharacter = NULL;
			m_aListEntries.push_back( mc );
		}

		vector<Character*> vpCharacters;
		CHARMAN->GetCharacters( vpCharacters );
		for( unsigned i=0; i<vpCharacters.size(); i++ )
		{
			Character* pCharacter = vpCharacters[i];
			RString s = pCharacter->GetDisplayName();
			s.MakeUpper();

			m_Def.m_vsChoices.push_back( s ); 
			GameCommand mc;
			mc.m_pCharacter = pCharacter;
			m_aListEntries.push_back( mc );
		}
	}
};

class OptionRowHandlerListStyles: public OptionRowHandlerList
{
	virtual void LoadInternal( const Commands &cmds )
	{
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_sName = "Style";
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself

		vector<const Style*> vStyles;
		GAMEMAN->GetStylesForGame( GAMESTATE->m_pCurGame, vStyles );
		ASSERT( vStyles.size() );
		FOREACH_CONST( const Style*, vStyles, s )
		{
			m_Def.m_vsChoices.push_back( GAMEMAN->StyleToLocalizedString(*s) ); 
			GameCommand mc;
			mc.m_pStyle = *s;
			m_aListEntries.push_back( mc );
		}

		m_Default.m_pStyle = vStyles[0];
	}
};

class OptionRowHandlerListGroups: public OptionRowHandlerList
{
	virtual void LoadInternal( const Commands &cmds )
	{
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself
		m_Def.m_sName = "Group";
		m_Default.m_sSongGroup = GROUP_ALL;

		vector<RString> vSongGroups;
		SONGMAN->GetSongGroupNames( vSongGroups );
		ASSERT( vSongGroups.size() );

		{
			m_Def.m_vsChoices.push_back( "AllGroups" );
			GameCommand mc;
			mc.m_sSongGroup = GROUP_ALL;
			m_aListEntries.push_back( mc );
		}

		FOREACH_CONST( RString, vSongGroups, g )
		{
			m_Def.m_vsChoices.push_back( *g ); 
			GameCommand mc;
			mc.m_sSongGroup = *g;
			m_aListEntries.push_back( mc );
		}
	}
};

class OptionRowHandlerListDifficulties: public OptionRowHandlerList
{
	virtual void LoadInternal( const Commands &cmds )
	{
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_sName = "Difficulty";
		m_Default.m_dc = DIFFICULTY_INVALID;
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself

		{
			m_Def.m_vsChoices.push_back( "AllDifficulties" );
			GameCommand mc;
			mc.m_dc = DIFFICULTY_INVALID;
			m_aListEntries.push_back( mc );
		}

		FOREACH_CONST( Difficulty, CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue(), d )
		{
			RString s = DifficultyToLocalizedString( *d );

			m_Def.m_vsChoices.push_back( s ); 
			GameCommand mc;
			mc.m_dc = *d;
			m_aListEntries.push_back( mc );
		}
	}
};

// XXX: very similar to OptionRowHandlerSongChoices
class OptionRowHandlerListSongsInCurrentSongGroup: public OptionRowHandlerList
{
	virtual void LoadInternal( const Commands &cmds )
	{
		vector<Song*> vpSongs;
		SONGMAN->GetSongs( vpSongs, GAMESTATE->m_sPreferredSongGroup );

		if( GAMESTATE->m_pCurSong == NULL )
			GAMESTATE->m_pCurSong.Set( vpSongs[0] );

		m_Def.m_sName = "SongsInCurrentSongGroup";
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		m_Def.m_bExportOnChange = true;

		FOREACH_CONST( Song*, vpSongs, p )
		{
			m_Def.m_vsChoices.push_back( (*p)->GetTranslitFullTitle() ); 
			GameCommand mc;
			mc.m_pSong = *p;
			m_aListEntries.push_back( mc );
		}
	}
};

class OptionRowHandlerLua : public OptionRowHandler
{
public:
	LuaReference *m_pLuaTable;
	LuaReference m_EnabledForPlayersFunc;

	OptionRowHandlerLua() { m_pLuaTable = new LuaReference; Init(); }
	virtual ~OptionRowHandlerLua() { delete m_pLuaTable; }
	void Init()
	{
		OptionRowHandler::Init();
		m_pLuaTable->Unset();
	}

	void SetEnabledForPlayers()
	{
		Lua *L = LUA->Get();

		if( m_EnabledForPlayersFunc.IsNil() )
		{
			LUA->Release(L);
			return;
		}

		m_EnabledForPlayersFunc.PushSelf( L );
		
		/* Argument 1 (self): */
		m_pLuaTable->PushSelf( L );
		
		lua_call( L, 1, 1 ); // call function with 1 argument and 1 result
		if( !lua_istable(L, -1) )
			RageException::Throw( "\"EnabledForPlayers\" did not return a table." );

		m_Def.m_vEnabledForPlayers.clear();	// and fill in with supplied PlayerNumbers below

		lua_pushnil( L );
		while( lua_next(L, -2) != 0 )
		{
			/* `key' is at index -2 and `value' at index -1 */
			PlayerNumber pn = (PlayerNumber)luaL_checkint( L, -1 );

			m_Def.m_vEnabledForPlayers.insert( pn );

			lua_pop( L, 1 );  /* removes `value'; keeps `key' for next iteration */
		}
		lua_pop( L, 1 );

		LUA->Release(L);
	}

	virtual void LoadInternal( const Commands &cmds )
	{
		ASSERT( cmds.v.size() == 1 );
		const Command &command = cmds.v[0];
		ASSERT( command.m_vsArgs.size() == 2 );
		RString sLuaFunction = command.m_vsArgs[1];
		ASSERT( sLuaFunction.size() );

		m_Def.m_bAllowThemeItems = false;	// Lua options are always dynamic and can theme themselves.

		Lua *L = LUA->Get();

		/* Run the Lua expression.  It should return a table. */
		m_pLuaTable->SetFromExpression( sLuaFunction );

		if( m_pLuaTable->GetLuaType() != LUA_TTABLE )
			RageException::Throw( "Result of \"%s\" is not a table.", sLuaFunction.c_str() );

		m_pLuaTable->PushSelf( L );

		lua_pushstring( L, "Name" );
		lua_gettable( L, -2 );
		const char *pStr = lua_tostring( L, -1 );
		if( pStr == NULL )
			RageException::Throw( "\"%s\" \"Name\" entry is not a string.", sLuaFunction.c_str() );
		m_Def.m_sName = pStr;
		lua_pop( L, 1 );


		lua_pushstring( L, "OneChoiceForAllPlayers" );
		lua_gettable( L, -2 );
		m_Def.m_bOneChoiceForAllPlayers = !!lua_toboolean( L, -1 );
		lua_pop( L, 1 );


		lua_pushstring( L, "ExportOnChange" );
		lua_gettable( L, -2 );
		m_Def.m_bExportOnChange = !!lua_toboolean( L, -1 );
		lua_pop( L, 1 );


		lua_pushstring( L, "LayoutType" );
		lua_gettable( L, -2 );
		pStr = lua_tostring( L, -1 );
		if( pStr == NULL )
			RageException::Throw( "\"%s\" \"LayoutType\" entry is not a string.", sLuaFunction.c_str() );
		m_Def.m_layoutType = StringToLayoutType( pStr );
		ASSERT( m_Def.m_layoutType != LAYOUT_INVALID );
		lua_pop( L, 1 );


		lua_pushstring( L, "SelectType" );
		lua_gettable( L, -2 );
		pStr = lua_tostring( L, -1 );
		if( pStr == NULL )
			RageException::Throw( "\"%s\" \"SelectType\" entry is not a string.", sLuaFunction.c_str() );
		m_Def.m_selectType = StringToSelectType( pStr );
		ASSERT( m_Def.m_selectType != SELECT_INVALID );
		lua_pop( L, 1 );


		/* Iterate over the "Choices" table. */
		lua_pushstring( L, "Choices" );
		lua_gettable( L, -2 );
		if( !lua_istable( L, -1 ) )
			RageException::Throw( "\"%s\" \"Choices\" is not a table.", sLuaFunction.c_str() );

		lua_pushnil( L );
		while( lua_next(L, -2) != 0 )
		{
			/* `key' is at index -2 and `value' at index -1 */
			const char *pValue = lua_tostring( L, -1 );
			if( pValue == NULL )
				RageException::Throw( "\"%s\" Column entry is not a string.", sLuaFunction.c_str() );
//				LOG->Trace( "'%s'", pValue);

			m_Def.m_vsChoices.push_back( pValue );

			lua_pop( L, 1 );  /* removes `value'; keeps `key' for next iteration */
		}

		lua_pop( L, 1 ); /* pop choices table */


		/* Set the EnabledForPlayers function. */
		lua_pushstring( L, "EnabledForPlayers" );
		lua_gettable( L, -2 );
		if( !lua_isfunction( L, -1 ) && !lua_isnil( L, -1 ) )
			RageException::Throw( "\"%s\" \"EnabledForPlayers\" is not a table.", sLuaFunction.c_str() );
		m_EnabledForPlayersFunc.SetFromStack( L );
		SetEnabledForPlayers();
		
		/* Iterate over the "ReloadRowMessages" table. */
		lua_pushstring( L, "ReloadRowMessages" );
		lua_gettable( L, -2 );
		if( !lua_isnil( L, -1 ) )
		{
			if( !lua_istable( L, -1 ) )
				RageException::Throw( "\"%s\" \"ReloadRowMessages\" is not a table.", sLuaFunction.c_str() );

			lua_pushnil( L );
			while( lua_next(L, -2) != 0 )
			{
				/* `key' is at index -2 and `value' at index -1 */
				const char *pValue = lua_tostring( L, -1 );
				if( pValue == NULL )
					RageException::Throw( "\"%s\" Column entry is not a string.", sLuaFunction.c_str() );
				LOG->Trace( "Found ReloadRowMessage '%s'", pValue);

				m_vsReloadRowMessages.push_back( pValue );

				lua_pop( L, 1 );  /* removes `value'; keeps `key' for next iteration */
			}
		}
		lua_pop( L, 1 ); /* pop ReloadRowMessages table */

		
		/* Look for "ExportOnChange" value. */
		lua_pushstring( L, "ExportOnChange" );
		lua_gettable( L, -2 );
		if( !lua_isnil( L, -1 ) )
		{
			m_Def.m_bExportOnChange = !!MyLua_checkboolean( L, -1 );
		}
		lua_pop( L, 1 ); /* pop ExportOnChange value */


		lua_pop( L, 1 ); /* pop main table */
		ASSERT( lua_gettop(L) == 0 );

		LUA->Release(L);
	}

	virtual ReloadChanged Reload()
	{
		SetEnabledForPlayers();
		return RELOAD_CHANGED_ENABLED;
	}

	virtual void ImportOption( const vector<PlayerNumber> &vpns, vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
		Lua *L = LUA->Get();

		ASSERT( lua_gettop(L) == 0 );

		FOREACH_CONST( PlayerNumber, vpns, pn )
		{
			PlayerNumber p = *pn;
			vector<bool> &vbSelOut = vbSelectedOut[p];

			/* Evaluate the LoadSelections(self,array,pn) function, where array is a table
			* representing vbSelectedOut. */

			/* All selections default to false. */
			for( unsigned i = 0; i < vbSelOut.size(); ++i )
				vbSelOut[i] = false;

			/* Create the vbSelectedOut table. */
			LuaHelpers::CreateTableFromArrayB( L, vbSelOut );
			ASSERT( lua_gettop(L) == 1 ); /* vbSelectedOut table */

			/* Get the function to call from m_LuaTable. */
			m_pLuaTable->PushSelf( L );
			ASSERT( lua_istable( L, -1 ) );

			lua_pushstring( L, "LoadSelections" );
			lua_gettable( L, -2 );
			if( !lua_isfunction( L, -1 ) )
				RageException::Throw( "\"%s\" \"LoadSelections\" entry is not a function.", m_Def.m_sName.c_str() );

			/* Argument 1 (self): */
			m_pLuaTable->PushSelf( L );

			/* Argument 2 (vbSelectedOut): */
			lua_pushvalue( L, 1 );

			/* Argument 3 (pn): */
			LuaHelpers::Push( L, p );

			ASSERT( lua_gettop(L) == 6 ); /* vbSelectedOut, m_iLuaTable, function, self, arg, arg */

			lua_call( L, 3, 0 ); // call function with 3 arguments and 0 results
			ASSERT( lua_gettop(L) == 2 );

			lua_pop( L, 1 ); /* pop option table */

			LuaHelpers::ReadArrayFromTableB( L, vbSelOut );
			
			lua_pop( L, 1 ); /* pop vbSelectedOut table */

			ASSERT( lua_gettop(L) == 0 );
		}

		LUA->Release(L);
	}
	virtual int ExportOption( const vector<PlayerNumber> &vpns, const vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		Lua *L = LUA->Get();

		ASSERT( lua_gettop(L) == 0 );

		FOREACH_CONST( PlayerNumber, vpns, pn )
		{
			PlayerNumber p = *pn;
			const vector<bool> &vbSel = vbSelected[p];

			/* Evaluate SaveSelections(self,array,pn) function, where array is a table
			 * representing vbSelectedOut. */

			vector<bool> vbSelectedCopy = vbSel;

			/* Create the vbSelectedOut table. */
			LuaHelpers::CreateTableFromArrayB( L, vbSelectedCopy );
			ASSERT( lua_gettop(L) == 1 ); /* vbSelectedOut table */

			/* Get the function to call. */
			m_pLuaTable->PushSelf( L );
			ASSERT( lua_istable( L, -1 ) );

			lua_pushstring( L, "SaveSelections" );
			lua_gettable( L, -2 );
			if( !lua_isfunction( L, -1 ) )
				RageException::Throw( "\"%s\" \"SaveSelections\" entry is not a function.", m_Def.m_sName.c_str() );

			/* Argument 1 (self): */
			m_pLuaTable->PushSelf( L );

			/* Argument 2 (vbSelectedOut): */
			lua_pushvalue( L, 1 );

			/* Argument 3 (pn): */
			LuaHelpers::Push( L, p );

			ASSERT( lua_gettop(L) == 6 ); /* vbSelectedOut, m_iLuaTable, function, self, arg, arg */

			lua_call( L, 3, 0 ); // call function with 3 arguments and 0 results
			ASSERT( lua_gettop(L) == 2 );

			lua_pop( L, 1 ); /* pop option table */
			lua_pop( L, 1 ); /* pop vbSelected table */

			ASSERT( lua_gettop(L) == 0 );
		}

		LUA->Release(L);

		// XXX: allow specifying the mask
		return 0;
	}
};

class OptionRowHandlerConfig : public OptionRowHandler
{
public:
	const ConfOption *opt;

	OptionRowHandlerConfig() { Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		opt = NULL;
	}
	virtual void LoadInternal( const Commands &cmds )
	{
		ASSERT( cmds.v.size() == 1 );
		const Command &command = cmds.v[0];
		RString sParam = command.GetArg(1);
		ASSERT( command.m_vsArgs.size() == 2 );
		ASSERT( sParam.size() );

		Init();

		/* Configuration values are never per-player. */
		m_Def.m_bOneChoiceForAllPlayers = true;

		ConfOption *pConfOption = ConfOption::Find( sParam );
		if( pConfOption == NULL )
		{
			LOG->Warn( "Invalid Conf type \"%s\"", sParam.c_str() );
			pConfOption = ConfOption::Find( "Invalid" );
			ASSERT_M( pConfOption != NULL, "ConfOption::Find(Invalid)" );
		}

   		pConfOption->UpdateAvailableOptions();

		opt = pConfOption;
		opt->MakeOptionsList( m_Def.m_vsChoices );

		m_Def.m_bAllowThemeItems = opt->m_bAllowThemeItems;

		m_Def.m_sName = opt->name;
	}
	virtual void ImportOption( const vector<PlayerNumber> &vpns, vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
		FOREACH_CONST( PlayerNumber, vpns, pn )
		{
			PlayerNumber p = *pn;
			vector<bool> &vbSelOut = vbSelectedOut[p];

			int iSelection = opt->Get();
			OptionRowHandlerUtil::SelectExactlyOne( iSelection, vbSelOut );
		}
	}
	virtual int ExportOption( const vector<PlayerNumber> &vpns, const vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		bool bChanged = false;

		FOREACH_CONST( PlayerNumber, vpns, pn )
		{
			PlayerNumber p = *pn;
			const vector<bool> &vbSel = vbSelected[p];

			int sel = OptionRowHandlerUtil::GetOneSelection(vbSel);

			/* Get the original choice. */
			int Original = opt->Get();

			/* Apply. */
			opt->Put( sel );

			/* Get the new choice. */
			int New = opt->Get();

			/* If it didn't change, don't return any side-effects. */
			if( Original != New )
				bChanged = true;
		}

		return bChanged ? opt->GetEffects() : 0;
	}
};

class OptionRowHandlerStepsType : public OptionRowHandler
{
public:
	BroadcastOnChange<StepsType> *m_pstToFill;
	vector<StepsType> m_vStepsTypesToShow;

	OptionRowHandlerStepsType() { Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		m_pstToFill = NULL;
		m_vStepsTypesToShow.clear();
	}

	virtual void LoadInternal( const Commands &cmds )
	{
		ASSERT( cmds.v.size() == 1 );
		const Command &command = cmds.v[0];
		RString sParam = command.GetArg(1);
		ASSERT( command.m_vsArgs.size() == 2 );
		ASSERT( sParam.size() );

		if( sParam == "EditStepsType" )
		{
			m_pstToFill = &GAMESTATE->m_stEdit;
		}
		else if( sParam == "EditSourceStepsType" )
		{
			m_pstToFill = &GAMESTATE->m_stEditSource;
			m_vsReloadRowMessages.push_back( MessageToString(Message_CurrentStepsP1Changed) );
			m_vsReloadRowMessages.push_back( MessageToString(Message_EditStepsTypeChanged) );
			if( GAMESTATE->m_pCurSteps[0].Get() != NULL )
				m_Def.m_vEnabledForPlayers.clear();	// hide row
		}
		else
		{
			RageException::Throw( "Invalid StepsType param \"%s\".", sParam.c_str() );
		}

		m_Def.m_sName = sParam;
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		m_Def.m_bExportOnChange = true;
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself

		// calculate which StepsTypes to show
		m_vStepsTypesToShow = CommonMetrics::STEPS_TYPES_TO_SHOW.GetValue();

		m_Def.m_vsChoices.clear();
		FOREACH_CONST( StepsType, m_vStepsTypesToShow, st )
		{
			RString s = GAMEMAN->StepsTypeToLocalizedString( *st );
			m_Def.m_vsChoices.push_back( s );
		}

		if( *m_pstToFill == STEPS_TYPE_INVALID )
			m_pstToFill->Set( m_vStepsTypesToShow[0] );
	}

	virtual void ImportOption( const vector<PlayerNumber> &vpns, vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
		FOREACH_CONST( PlayerNumber, vpns, pn )
		{
			PlayerNumber p = *pn;
			vector<bool> &vbSelOut = vbSelectedOut[p];

			if( GAMESTATE->m_pCurSteps[0] )
			{
				StepsType st = GAMESTATE->m_pCurSteps[0]->m_StepsType;
				vector<StepsType>::const_iterator iter = find( m_vStepsTypesToShow.begin(), m_vStepsTypesToShow.end(), st );
				if( iter != m_vStepsTypesToShow.end() )
				{
					unsigned i = iter - m_vStepsTypesToShow.begin();
					vbSelOut[i] = true;
					continue;	// done with this player
				}
			}
			vbSelOut[0] = true;
		}
	}
	virtual int ExportOption( const vector<PlayerNumber> &vpns, const vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		FOREACH_CONST( PlayerNumber, vpns, pn )
		{
			PlayerNumber p = *pn;
			const vector<bool> &vbSel = vbSelected[p];

			int index = OptionRowHandlerUtil::GetOneSelection( vbSel );
			m_pstToFill->Set( m_vStepsTypesToShow[index] );
		}

		return 0;
	}
};


class OptionRowHandlerGameCommand : public OptionRowHandler
{
public:
	GameCommand m_gc;

	OptionRowHandlerGameCommand() { Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		m_gc.Init();
		m_gc.ApplyCommitsScreens( false );
	}
	virtual void LoadInternal( const Commands &cmds )
	{
		ASSERT( cmds.v.size() > 1 );

		Commands temp = cmds;
		temp.v.erase( temp.v.begin() );
		m_gc.Load( 0, temp );
		ASSERT( !m_gc.m_sName.empty() );
		m_Def.m_sName = m_gc.m_sName;
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		m_Def.m_selectType = SELECT_NONE;
		m_Def.m_vsChoices.push_back( "" );
	}
	virtual void ImportOption( const vector<PlayerNumber> &vpns, vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
	}
	virtual int ExportOption( const vector<PlayerNumber> &vpns, const vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		if( vbSelected[PLAYER_1][0] )
			m_gc.ApplyToAllPlayers();
		return 0;
	}
	virtual void GetIconTextAndGameCommand( int iFirstSelection, RString &sIconTextOut, GameCommand &gcOut ) const
	{
		sIconTextOut = "";
		gcOut = m_gc;
	}
	virtual RString GetScreen( int iChoice ) const
	{ 
		return m_gc.m_sScreen;
	}
};

class OptionRowHandlerNull: public OptionRowHandler
{
public:
	OptionRowHandlerNull() { Init(); }
};


///////////////////////////////////////////////////////////////////////////////////

OptionRowHandler* OptionRowHandlerUtil::Make( const Commands &cmds )
{
	OptionRowHandler* pHand = NULL;

	if( cmds.v.size() == 0 )
		return NULL;

	const RString &name = cmds.v[0].GetName();

#define MAKE( type )	{ type *p = new type; p->Load( cmds ); pHand = p; }

	// XXX: merge these, and merge "Steps" and "list,Steps"
	if( name == "list" )
	{
		const Command &command = cmds.v[0];
		RString sParam = command.GetArg(1);
		if( command.m_vsArgs.size() != 2 || !sParam.size() )
			return NULL;

		if(	 sParam.CompareNoCase("NoteSkins")==0 )		MAKE( OptionRowHandlerListNoteSkins )
		else if( sParam.CompareNoCase("Steps")==0 )		MAKE( OptionRowHandlerListSteps )
		else if( sParam.CompareNoCase("StepsLocked")==0 )
		{
			MAKE( OptionRowHandlerListSteps ); 
			pHand->m_Def.m_bOneChoiceForAllPlayers = true;
		}
		else if( sParam.CompareNoCase("Characters")==0 )	MAKE( OptionRowHandlerListCharacters )
		else if( sParam.CompareNoCase("Styles")==0 )		MAKE( OptionRowHandlerListStyles )
		else if( sParam.CompareNoCase("Groups")==0 )		MAKE( OptionRowHandlerListGroups )
		else if( sParam.CompareNoCase("Difficulties")==0 )	MAKE( OptionRowHandlerListDifficulties )
		else if( sParam.CompareNoCase("SongsInCurrentSongGroup")==0 )	MAKE( OptionRowHandlerListSongsInCurrentSongGroup )
		else MAKE( OptionRowHandlerList )
	}
	else if( name == "lua" )		MAKE( OptionRowHandlerLua )
	else if( name == "conf" )		MAKE( OptionRowHandlerConfig )
	else if( name == "stepstype" )		MAKE( OptionRowHandlerStepsType )
	else if( name == "steps" )		MAKE( OptionRowHandlerSteps )
	else if( name == "gamecommand" )	MAKE( OptionRowHandlerGameCommand )

	return pHand;
}

OptionRowHandler* OptionRowHandlerUtil::MakeNull()
{
	OptionRowHandler* pHand = NULL;
	Commands cmds;
	MAKE( OptionRowHandlerNull )
	return pHand;
}

OptionRowHandler* OptionRowHandlerUtil::MakeSimple( const MenuRowDef &mr )
{
	OptionRowHandler *pHand = OptionRowHandlerUtil::MakeNull();

	pHand->m_Def.m_sName = mr.sName;
	FontCharAliases::ReplaceMarkers( pHand->m_Def.m_sName );	// Allow special characters
	
	pHand->m_Def.m_vEnabledForPlayers.clear();
	if( mr.pfnEnabled? mr.pfnEnabled():mr.bEnabled )
	{
		FOREACH_EnabledPlayer( pn )
			pHand->m_Def.m_vEnabledForPlayers.insert( pn );
	}

	pHand->m_Def.m_bOneChoiceForAllPlayers = true;
	pHand->m_Def.m_selectType = SELECT_ONE;
	pHand->m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
	pHand->m_Def.m_bExportOnChange = false;//true;
		
	pHand->m_Def.m_vsChoices = mr.choices;

	// Each row must have at least one choice.
	if( pHand->m_Def.m_vsChoices.empty() )
		pHand->m_Def.m_vsChoices.push_back( "" );
	
	pHand->m_Def.m_bAllowThemeTitle = mr.bThemeTitle;
	pHand->m_Def.m_bAllowThemeItems = mr.bThemeItems;

	FOREACH( RString, pHand->m_Def.m_vsChoices, c )
		FontCharAliases::ReplaceMarkers( *c );	// Allow special characters

	return pHand;
}

/*
 * (c) 2002-2004 Chris Danford
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
