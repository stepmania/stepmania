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
#include "Song.h"
#include "SongManager.h"
#include "Character.h"
#include "PrefsManager.h"
#include "SongUtil.h"
#include "StepsUtil.h"
#include "GameManager.h"
#include "GameSoundManager.h"
#include "CommonMetrics.h"
#include "CharacterManager.h"
#include "ScreenManager.h"
#include "ScreenMiniMenu.h"	// for MenuRowDef
#include "FontCharAliases.h"

#include <vector>

#define ENTRY(s)		THEME->GetMetric ("ScreenOptionsMaster",s)
#define ENTRY_MODE(s,i)		THEME->GetMetric ("ScreenOptionsMaster",ssprintf("%s,%i",(s).c_str(),(i+1)))
#define ENTRY_DEFAULT(s)	THEME->GetMetric ("ScreenOptionsMaster",(s) + "Default")
#define NOTE_SKIN_SORT_ORDER	THEME->GetMetric ("ScreenOptionsMaster","NoteSkinSortOrder")
#define STEPS_ROW_LAYOUT_TYPE	THEME->GetMetric("ScreenOptionsMaster","StepsRowLayoutType")
#define STEPS_USE_CHART_NAME	THEME->GetMetricB("ScreenOptionsMaster","StepsUseChartName")

static const char *SelectTypeNames[] = {
	"SelectOne",
	"SelectMultiple",
	"SelectNone",
};
XToString( SelectType );
StringToX( SelectType );
LuaXType( SelectType );

static const char *LayoutTypeNames[] = {
	"ShowAllInRow",
	"ShowOneInRow",
};
XToString( LayoutType );
StringToX( LayoutType );
LuaXType( LayoutType );

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

void OptionRowHandlerUtil::SelectExactlyOne( int iSelection, std::vector<bool> &vbSelectedOut )
{
	ASSERT_M( iSelection >= 0  &&  iSelection < (int) vbSelectedOut.size(),
			  ssprintf("%d/%u",iSelection, unsigned(vbSelectedOut.size())) );
	for( int i=0; i<int(vbSelectedOut.size()); i++ )
		vbSelectedOut[i] = i==iSelection;
}

int OptionRowHandlerUtil::GetOneSelection( const std::vector<bool> &vbSelected )
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

#define ROW_INVALID_IF(condition, message, retval) \
	if(condition) \
	{ \
		LuaHelpers::ReportScriptError("Parse error in option row: " message); \
		return retval; \
	}

#define CHECK_WRONG_NUM_ARGS(num) \
	ROW_INVALID_IF(command.m_vsArgs.size() != num, "Wrong number of args to option row.", false);
#define CHECK_BLANK_ARG \
	ROW_INVALID_IF(sParam.size() == 0, "Blank arg to Steps row.", false);

// begin OptionRow handlers
class OptionRowHandlerList : public OptionRowHandler
{
public:
	std::vector<GameCommand> m_aListEntries;
	GameCommand m_Default;
	bool m_bUseModNameForIcon;
	std::vector<RString> m_vsBroadcastOnExport;

	OptionRowHandlerList() { Init(); }
	virtual void Init()
	{
		OptionRowHandler::Init();
		m_aListEntries.clear();
		m_Default.Init();
		m_bUseModNameForIcon = false;
		m_vsBroadcastOnExport.clear();
	}
	virtual bool LoadInternal( const Commands &cmds )
	{
		const Command &command = cmds.v[0];
		RString sParam = command.GetArg(1).s;

		m_bUseModNameForIcon = true;
		m_Def.m_sName = sParam;
		m_Default.Load( -1, ParseCommands(ENTRY_DEFAULT(sParam)) );

		{
			// Parse the basic configuration metric.
			Commands lCmds = ParseCommands( ENTRY(sParam) );
			ROW_INVALID_IF(lCmds.v.size() < 1, "Row command is empty.", false);

			m_Def.m_bOneChoiceForAllPlayers = false;
			ROW_INVALID_IF(lCmds.v[0].m_vsArgs.size() != 1, "Row command has invalid args to number of entries.", false);
			const int NumCols = StringToInt( lCmds.v[0].m_vsArgs[0] );
			ROW_INVALID_IF(NumCols < 1, "Not enough entries in list.", false);
			for( unsigned i=1; i<lCmds.v.size(); i++ )
			{
				const Command &cmd = lCmds.v[i];
				RString sName = cmd.GetName();

				if(	 sName == "together" )		m_Def.m_bOneChoiceForAllPlayers = true;
				else if( sName == "selectmultiple" )	m_Def.m_selectType = SELECT_MULTIPLE;
				else if( sName == "selectone" )		m_Def.m_selectType = SELECT_ONE;
				else if( sName == "selectnone" )	m_Def.m_selectType = SELECT_NONE;
				else if( sName == "showoneinrow" )	m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
				else if( sName == "default" )		m_Def.m_iDefault = StringToInt( cmd.GetArg(1).s ) - 1; // match ENTRY_MODE
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
						PlayerNumber pn = (PlayerNumber)(StringToInt(sArg)-1);
						ASSERT( pn >= 0 && pn < NUM_PLAYERS );
						m_Def.m_vEnabledForPlayers.insert( pn );
					}
				}
				else if( sName == "exportonchange" )
				{
					m_Def.m_bExportOnChange = true;
				}
				else if( sName == "broadcastonexport" )
				{
					for( unsigned j=1; j<cmd.m_vsArgs.size(); j++ )
						m_vsBroadcastOnExport.push_back( cmd.m_vsArgs[j] );
				}
				else
				{
					LuaHelpers::ReportScriptErrorFmt("Unknown row flag \"%s\" on row %s.", sName.c_str(), m_Def.m_sName.c_str());
				}
			}
			for( int col = 0; col < NumCols; ++col )
			{
				GameCommand mc;
				mc.ApplyCommitsScreens( false );
				mc.Load( 0, ParseCommands(ENTRY_MODE(sParam, col)) );
				/* If the row has just one entry, use the name of the row as the name of the
				 * entry. If it has more than one, each one must be specified explicitly. */
				if( mc.m_sName == "" && NumCols == 1 )
					mc.m_sName = sParam;
				if( mc.m_sName == "" )
				{
					LuaHelpers::ReportScriptErrorFmt("List \"%s\", choice %i has no name.", sParam.c_str(), col+1);
					mc.m_sName= "";
				}

				RString why;
				if( !mc.IsPlayable(&why) )
				{
					LuaHelpers::ReportScriptErrorFmt("\"%s\" choice %d is not playable: %s", sParam.c_str(), col, why.c_str());
					continue;
				}

				m_aListEntries.push_back( mc );
				RString sChoice = mc.m_sName;
				m_Def.m_vsChoices.push_back( sChoice );
			}
		}

		if( m_Def.m_selectType != SELECT_MULTIPLE && m_Def.m_iDefault == -1 )
		{
			for( unsigned e = 0; e < m_aListEntries.size(); ++e )
			{
				const GameCommand &mc = m_aListEntries[e];
				if( mc.IsZero() )
					m_Def.m_iDefault = e;
			}
		}
		return true;
	}
	void ImportOption( OptionRow *pRow, const std::vector<PlayerNumber> &vpns, std::vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
		for (PlayerNumber const &p : vpns)
		{
			std::vector<bool> &vbSelOut = vbSelectedOut[p];

			bool bUseFallbackOption = true;

			for( unsigned e = 0; e < m_aListEntries.size(); ++e )
			{
				const GameCommand &mc = m_aListEntries[e];

				vbSelOut[e] = false;

				if( mc.IsZero() )
				{
					/* The entry has no effect. This is usually a default "none
					 * of the above" entry. It will always return true for
					 * DescribesCurrentMode(). It's only the selected choice if
					 * nothing else matches. */
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
					if( mc.DescribesCurrentMode(p) )
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
				int iFallbackOption = m_Def.m_iDefault;
				if( iFallbackOption == -1 )
				{
					RString s = ssprintf("No options in row \"list,%s\" were selected, and no fallback row found; selected entry 0", m_Def.m_sName.c_str());
					LOG->Warn( "%s", s.c_str() );
					CHECKPOINT_M( s );
					iFallbackOption = 0;
				}

				OptionRowHandlerUtil::SelectExactlyOne( iFallbackOption, vbSelOut );
			}

			VerifySelected( m_Def.m_selectType, vbSelOut, m_Def.m_sName );
		}
	}

	int ExportOption( const std::vector<PlayerNumber> &vpns, const std::vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		for (PlayerNumber const &p : vpns)
		{
			const std::vector<bool> &vbSel = vbSelected[p];

			m_Default.Apply( p );
			for( unsigned i=0; i<vbSel.size(); i++ )
			{
				if( vbSel[i] )
					m_aListEntries[i].Apply( p );
			}
		}
		for (RString const &s : m_vsBroadcastOnExport)
			MESSAGEMAN->Broadcast( s );
		return 0;
	}

	virtual int GetDefaultOption() const
	{
		return m_Def.m_iDefault;
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

static void SortNoteSkins( std::vector<RString> &asSkinNames )
{
	std::set<RString> setSkinNames;
	setSkinNames.insert( asSkinNames.begin(), asSkinNames.end() );

	std::vector<RString> asSorted;
	split( NOTE_SKIN_SORT_ORDER, ",", asSorted );

	std::set<RString> setUnusedSkinNames( setSkinNames );
	asSkinNames.clear();

	for (RString const &sSkin : asSorted)
	{
		if( setSkinNames.find(sSkin) == setSkinNames.end() )
			continue;
		asSkinNames.push_back( sSkin );
		setUnusedSkinNames.erase( sSkin );
	}

	asSkinNames.insert( asSkinNames.end(), setUnusedSkinNames.begin(), setUnusedSkinNames.end() );
}

class OptionRowHandlerListNoteSkins : public OptionRowHandlerList
{
	virtual bool LoadInternal( const Commands & )
	{
		m_Def.m_sName = "NoteSkins";
		m_Def.m_bOneChoiceForAllPlayers = false;
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself

		std::vector<RString> arraySkinNames;
		NOTESKIN->GetNoteSkinNames( arraySkinNames );
		SortNoteSkins( arraySkinNames );

		for( unsigned skin=0; skin<arraySkinNames.size(); skin++ )
		{
			if( arraySkinNames[skin] == CommonMetrics::DEFAULT_NOTESKIN_NAME.GetValue() )
				m_Def.m_iDefault = skin;
			GameCommand mc;
			mc.m_sPreferredModifiers = arraySkinNames[skin];
			//ms.m_sName = arraySkinNames[skin];
			m_aListEntries.push_back( mc );
			m_Def.m_vsChoices.push_back( arraySkinNames[skin] );
		}
		return true;
	}
};

// XXX: very similar to OptionRowHandlerSteps
class OptionRowHandlerListSteps : public OptionRowHandlerList
{
	virtual bool LoadInternal( const Commands & )
	{
		m_Def.m_sName = "Steps";
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself

		Reload();

		// don't call default
		// OptionRowHandlerList::LoadInternal( cmds );
		return true;
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
		// TODO:  Fix this OptionRow to fetch steps for all styles available.
		// This is broken in kickbox game mode because kickbox uses separated
		// styles. -Kyz
		else if(GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber()) && GAMESTATE->IsCourseMode() && GAMESTATE->m_pCurCourse)   // playing a course
		{
			m_Def.m_bOneChoiceForAllPlayers = (bool)PREFSMAN->m_bLockCourseDifficulties;
			m_Def.m_layoutType = StringToLayoutType( STEPS_ROW_LAYOUT_TYPE );

			std::vector<Trail*> vTrails;
			GAMESTATE->m_pCurCourse->GetTrails( vTrails, GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_StepsType );
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
		else if(GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber()) && GAMESTATE->m_pCurSong) // playing a song
		{
			m_Def.m_layoutType = StringToLayoutType( STEPS_ROW_LAYOUT_TYPE );

			std::vector<Steps*> vpSteps;
			Song *pSong = GAMESTATE->m_pCurSong;
			SongUtil::GetSteps( pSong, vpSteps, GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber())->m_StepsType );
			StepsUtil::RemoveLockedSteps( pSong, vpSteps );
			StepsUtil::SortNotesArrayByDifficulty( vpSteps );
			for( unsigned i=0; i<vpSteps.size(); i++ )
			{
				Steps* pSteps = vpSteps[i];

				RString s;
				if (STEPS_USE_CHART_NAME)
				{
					s = pSteps->GetChartName();
				}
				// TODO: find a way to make this use lua or metrics.
				if (s == "" || s == "blank" || s == "Blank")
				{
					if( pSteps->GetDifficulty() == Difficulty_Edit )
					{
						s = pSteps->GetChartName();
						if (s == "" || s == "blank" || s == "Blank")
							s = pSteps->GetDescription();
					}
					else
					{
						if( pSteps->IsAnEdit() )
						{
							s = pSteps->GetChartName();
							if (s == "" || s == "blank" || s == "Blank")
								s = pSteps->GetDescription();
						}
						else
							s = CustomDifficultyToLocalizedString( GetCustomDifficulty( pSteps->m_StepsType, pSteps->GetDifficulty(), CourseType_Invalid ) );
					}
				}
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
			/* We have neither a song nor a course. We may be preloading the
			 * screen for future use. */
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
	std::vector<Steps*> m_vSteps;
	std::vector<Difficulty> m_vDifficulties;

	OptionRowHandlerSteps() { Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		m_ppStepsToFill = nullptr;
		m_pDifficultyToFill = nullptr;
		m_vSteps.clear();
		m_vDifficulties.clear();
	}

	virtual bool LoadInternal( const Commands &cmds )
	{
		const Command &command = cmds.v[0];
		RString sParam = command.GetArg(1).s;
		CHECK_WRONG_NUM_ARGS(2);
		CHECK_BLANK_ARG;

		if( sParam == "EditSteps" )
		{
			m_ppStepsToFill = &GAMESTATE->m_pCurSteps[0];
			m_pDifficultyToFill = &GAMESTATE->m_PreferredDifficulty[0];
			m_pst = &GAMESTATE->m_stEdit;
			m_vsReloadRowMessages.push_back( MessageIDToString(Message_EditStepsTypeChanged) );
		}
		else if( sParam == "EditSourceSteps" )
		{
			m_ppStepsToFill = &GAMESTATE->m_pEditSourceSteps;
			m_pst = &GAMESTATE->m_stEditSource;
			m_vsReloadRowMessages.push_back( MessageIDToString(Message_EditSourceStepsTypeChanged) );
			if( GAMESTATE->m_pCurSteps[0].Get() != nullptr )
				m_Def.m_vEnabledForPlayers.clear();	// hide row
		}
		else
		{
			ROW_INVALID_IF(true, "Invalid StepsType param \"" + sParam + "\".", false);
		}

		m_Def.m_sName = sParam;
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		m_Def.m_bExportOnChange = true;
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself
		m_vsReloadRowMessages.push_back( MessageIDToString(Message_CurrentSongChanged) );

		m_vDifficulties.clear();
		m_vSteps.clear();

		if( GAMESTATE->m_pCurSong )
		{
			FOREACH_ENUM( Difficulty, dc )
			{
				if( dc == Difficulty_Edit )
					continue;
				m_vDifficulties.push_back( dc );
				Steps* pSteps = SongUtil::GetStepsByDifficulty( GAMESTATE->m_pCurSong, *m_pst, dc );
				m_vSteps.push_back( pSteps );
			}
			SongUtil::GetSteps( GAMESTATE->m_pCurSong, m_vSteps, *m_pst, Difficulty_Edit );
			m_vDifficulties.resize( m_vSteps.size(), Difficulty_Edit );

			if( sParam == "EditSteps" )
			{
				m_vSteps.push_back(nullptr);
				m_vDifficulties.push_back( Difficulty_Edit );
			}

			for( unsigned i=0; i<m_vSteps.size(); i++ )
			{
				Steps* pSteps = m_vSteps[i];
				Difficulty dc = m_vDifficulties[i];

				RString s;
				if( dc == Difficulty_Edit )
				{
					if( pSteps )
						s = pSteps->GetDescription();
					else
						s = "NewEdit";
				}
				else
				{
					s = CustomDifficultyToLocalizedString( GetCustomDifficulty( GAMESTATE->m_stEdit, dc, CourseType_Invalid ) );
				}
				m_Def.m_vsChoices.push_back( s );
			}
		}
		else
		{
			m_vDifficulties.push_back( Difficulty_Edit );
			m_vSteps.push_back(nullptr);
			m_Def.m_vsChoices.push_back( "none" );
		}

		if( m_pDifficultyToFill )
			m_pDifficultyToFill->Set( m_vDifficulties[0] );
		m_ppStepsToFill->Set( m_vSteps[0] );
		return true;
	}
	virtual void ImportOption( OptionRow *pRow, const std::vector<PlayerNumber> &vpns, std::vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
		for (PlayerNumber const &p : vpns)
		{
			std::vector<bool> &vbSelOut = vbSelectedOut[p];

			ASSERT( m_vSteps.size() == vbSelOut.size() );

			// look for matching steps
			std::vector<Steps*>::const_iterator iter = find( m_vSteps.begin(), m_vSteps.end(), m_ppStepsToFill->Get() );
			if( iter != m_vSteps.end() )
			{
				unsigned i = iter - m_vSteps.begin();
				vbSelOut[i] = true;
				continue;
			}
			// look for matching difficulty
			bool matched= false;
			if( m_pDifficultyToFill )
			{
				// use the old style for now.
				for (std::vector<Difficulty>::const_iterator d = m_vDifficulties.begin(); d != m_vDifficulties.end(); ++d)
				{
					unsigned i = d - m_vDifficulties.begin();
					if( *d == GAMESTATE->m_PreferredDifficulty[p] )
					{
						vbSelOut[i] = true;
						matched= true;
						std::vector<PlayerNumber> v;
						v.push_back( p );
						ExportOption( v, vbSelectedOut );	// current steps changed
						break;
					}
				}
			}
			if(!matched)
			{
				// default to 1st
				vbSelOut[0] = true;
			}
		}
	}
	virtual int ExportOption( const std::vector<PlayerNumber> &vpns, const std::vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		for (PlayerNumber const &p : vpns)
		{
			const std::vector<bool> &vbSel = vbSelected[p];

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
	virtual bool LoadInternal( const Commands & )
	{
		m_Def.m_bOneChoiceForAllPlayers = false;
		m_Def.m_bAllowThemeItems = false;
		m_Def.m_sName = "Characters";
		m_Def.m_iDefault = 0;
		m_Default.m_pCharacter = CHARMAN->GetDefaultCharacter();

		{
			m_Def.m_vsChoices.push_back( OFF );
			GameCommand mc;
			mc.m_pCharacter = nullptr;
			m_aListEntries.push_back( mc );
		}

		std::vector<Character*> vpCharacters;
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
		return true;
	}
};

class OptionRowHandlerListStyles: public OptionRowHandlerList
{
	virtual bool LoadInternal( const Commands & )
	{
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_sName = "Style";
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself

		std::vector<const Style*> vStyles;
		GAMEMAN->GetStylesForGame( GAMESTATE->m_pCurGame, vStyles );
		ASSERT( vStyles.size() != 0 );
		for (Style const *s : vStyles)
		{
			m_Def.m_vsChoices.push_back( GAMEMAN->StyleToLocalizedString(s) );
			GameCommand mc;
			mc.m_pStyle = s;
			m_aListEntries.push_back( mc );
		}

		m_Default.m_pStyle = vStyles[0];
		return true;
	}
};

class OptionRowHandlerListGroups: public OptionRowHandlerList
{
	virtual bool LoadInternal( const Commands & )
	{
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself
		m_Def.m_sName = "Group";
		m_Default.m_sSongGroup = GROUP_ALL;

		std::vector<RString> vSongGroups;
		SONGMAN->GetSongGroupNames( vSongGroups );
		ASSERT( vSongGroups.size() != 0 );

		{
			m_Def.m_vsChoices.push_back( "AllGroups" );
			GameCommand mc;
			mc.m_sSongGroup = GROUP_ALL;
			m_aListEntries.push_back( mc );
		}

		for (RString const &g : vSongGroups)
		{
			m_Def.m_vsChoices.push_back( g );
			GameCommand mc;
			mc.m_sSongGroup = g;
			m_aListEntries.push_back( mc );
		}
		return true;
	}
};

class OptionRowHandlerListDifficulties: public OptionRowHandlerList
{
	virtual bool LoadInternal( const Commands & )
	{
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_sName = "Difficulty";
		m_Default.m_dc = Difficulty_Invalid;
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself

		{
			m_Def.m_vsChoices.push_back( "AllDifficulties" );
			GameCommand mc;
			mc.m_dc = Difficulty_Invalid;
			m_aListEntries.push_back( mc );
		}

		for (Difficulty const &d : CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue())
		{
			// TODO: Is this the best thing we can do here?
			StepsType st = GAMEMAN->GetHowToPlayStyleForGame( GAMESTATE->m_pCurGame )->m_StepsType;
			RString s = CustomDifficultyToLocalizedString( GetCustomDifficulty(st, d, CourseType_Invalid) );

			m_Def.m_vsChoices.push_back( s );
			GameCommand mc;
			mc.m_dc = d;
			m_aListEntries.push_back( mc );
		}
		return true;
	}
};

// XXX: very similar to OptionRowHandlerSongChoices
class OptionRowHandlerListSongsInCurrentSongGroup: public OptionRowHandlerList
{
	virtual bool LoadInternal( const Commands & )
	{
		const std::vector<Song*> &vpSongs = SONGMAN->GetSongs( GAMESTATE->m_sPreferredSongGroup );

		if( GAMESTATE->m_pCurSong == nullptr )
			GAMESTATE->m_pCurSong.Set( vpSongs[0] );

		m_Def.m_sName = "SongsInCurrentSongGroup";
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		m_Def.m_bExportOnChange = true;

		for (Song *p : vpSongs)
		{
			m_Def.m_vsChoices.push_back( p->GetTranslitFullTitle() );
			GameCommand mc;
			mc.m_pSong = p;
			m_aListEntries.push_back( mc );
		}
		return true;
	}
};

class OptionRowHandlerLua : public OptionRowHandler
{
public:
	LuaReference *m_pLuaTable;
	LuaReference m_EnabledForPlayersFunc;
	LuaReference m_ReloadFunc;

	bool m_TableIsSane;
	bool m_GoToFirstOnStart;

	OptionRowHandlerLua(): m_TableIsSane(false), m_GoToFirstOnStart(false)
	{ m_pLuaTable = new LuaReference; Init(); }
	virtual ~OptionRowHandlerLua() { delete m_pLuaTable; }
	void Init()
	{
		OptionRowHandler::Init();
		m_pLuaTable->Unset();
	}

	bool SanityCheckTable(lua_State* L, RString& RowName)
	{
		if(m_pLuaTable->GetLuaType() != LUA_TTABLE)
		{
			LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  Result of \"%s\" is not a table.", RowName.c_str());
			return false;
		}
		m_pLuaTable->PushSelf(L);
		lua_getfield(L, -1, "Name");
		const char *pStr = lua_tostring(L, -1);
		if( pStr == nullptr )
		{
			LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"Name\" entry is not a string.", RowName.c_str());
			return false;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "LayoutType");
		pStr = lua_tostring(L, -1);
		if(pStr == nullptr || StringToLayoutType(pStr) == LayoutType_Invalid)
		{
			LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"LayoutType\" entry is not a string.", RowName.c_str());
			return false;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "SelectType");
		pStr = lua_tostring(L, -1);
		if(pStr == nullptr || StringToSelectType(pStr) == SelectType_Invalid)
		{
			LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"SelectType\" entry is not a string.", RowName.c_str());
			return false;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "Choices");
		if(!lua_istable(L, -1))
		{
			LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"Choices\" is not a table.", RowName.c_str());
			return false;
		}
		if(!TableContainsOnlyStrings(L, lua_gettop(L)))
		{
			LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"Choices\" table contains a non-string.", RowName.c_str());
			return false;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "EnabledForPlayers");
		if(!lua_isnil(L, -1))
		{
			if(!lua_isfunction(L, -1))
			{
				LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"EnabledForPlayers\" is not a function.", RowName.c_str());
				return false;
			}
			m_pLuaTable->PushSelf( L );
			RString error= RowName + " \"EnabledForPlayers\": ";
			LuaHelpers::RunScriptOnStack(L, error, 1, 1, true);
			if(!lua_istable(L, -1))
			{
				LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"EnabledForPlayers\" did not return a table.", RowName.c_str());
				return false;
			}
			lua_pushnil(L);
			while(lua_next(L, -2) != 0)
			{
				PlayerNumber pn= Enum::Check<PlayerNumber>(L, -1, true, true);
				if(pn == PlayerNumber_Invalid)
				{
					LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"EnabledForPlayers\" contains a non-PlayerNumber.", RowName.c_str());
					return false;
				}
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "ReloadRowMessages");
		if(!lua_isnil(L, -1))
		{
			if(!lua_istable(L, -1))
			{
				LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"ReloadRowMessages\" is not a table.", RowName.c_str());
				return false;
			}
			if(!TableContainsOnlyStrings(L, lua_gettop(L)))
			{
				LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"ReloadRowMessages\" table contains a non-string.", RowName.c_str());
				return false;
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "Reload");
		if(!lua_isnil(L, -1))
		{
			if(!lua_isfunction(L, -1))
			{
				LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"Reload\" entry is not a function.", RowName.c_str());
				return false;
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "LoadSelections");
		if(!lua_isfunction(L, -1))
		{
			LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"LoadSelections\" entry is not a function.", RowName.c_str());
			return false;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "SaveSelections");
		if(!lua_isfunction(L, -1))
		{
			LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"SaveSelections\" entry is not a function.", RowName.c_str());
			return false;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "NotifyOfSelection");
		if(!lua_isnil(L, -1) && !lua_isfunction(L, -1))
		{
			LuaHelpers::ReportScriptErrorFmt("LUA_ERROR:  \"%s\" \"NotifyOfSelection\" entry is not a function.", RowName.c_str());
			return false;
		}
		lua_pop(L, 1);

		lua_pop(L, 1);
		return true;
	}

	void SetEnabledForPlayers()
	{
		if(!m_TableIsSane)
		{
			return;
		}
		Lua *L = LUA->Get();

		if( m_EnabledForPlayersFunc.IsNil() )
		{
			LUA->Release(L);
			return;
		}

		m_EnabledForPlayersFunc.PushSelf( L );

		// Argument 1 (self):
		m_pLuaTable->PushSelf( L );

		RString error= "EnabledForPlayers: ";
		LuaHelpers::RunScriptOnStack( L, error, 1, 1, true );
		m_Def.m_vEnabledForPlayers.clear();	// and fill in with supplied PlayerNumbers below

		lua_pushnil( L );
		while( lua_next(L, -2) != 0 )
		{
			// `key' is at index -2 and `value' at index -1
			PlayerNumber pn = Enum::Check<PlayerNumber>(L, -1);

			m_Def.m_vEnabledForPlayers.insert( pn );

			lua_pop( L, 1 ); // removes `value'; keeps `key' for next iteration
		}
		lua_pop( L, 1 );

		LUA->Release(L);
	}

	void LoadChoices( Lua *L )
	{
		// Iterate over the "Choices" table.
		lua_getfield(L, -1, "Choices");
		lua_pushnil( L );
		while( lua_next(L, -2) != 0 )
		{
			// `key' is at index -2 and `value' at index -1
			const char *pValue = lua_tostring( L, -1 );
			//LOG->Trace( "choice: '%s'", pValue);
			m_Def.m_vsChoices.push_back( pValue );
			lua_pop( L, 1 ); // removes `value'; keeps `key' for next iteration
		}
		lua_pop( L, 1 ); // pop choices table
	}

	virtual bool LoadInternal( const Commands &cmds )
	{
		const Command &command = cmds.v[0];
		RString sParam = command.GetArg(1).s;
		CHECK_WRONG_NUM_ARGS(2);
		CHECK_BLANK_ARG;

		m_Def.m_bAllowThemeItems = false;	// Lua options are always dynamic and can theme themselves.

		Lua *L = LUA->Get();

		// Run the Lua expression.  It should return a table.
		m_pLuaTable->SetFromExpression( sParam );
		m_TableIsSane= SanityCheckTable(L, sParam);
		if(!m_TableIsSane)
		{
			lua_settop(L, 0); // Release has an assert that forces a clear stack.
			LUA->Release(L);
			return false;
		}
		m_pLuaTable->PushSelf(L);

		lua_getfield(L, -1, "Name");
		const char *pStr = lua_tostring( L, -1 );
		m_Def.m_sName = pStr;
		lua_pop( L, 1 );

		lua_getfield(L, -1, "GoToFirstOnStart");
		m_GoToFirstOnStart = lua_toboolean(L, -1) > 0;
		lua_pop(L, 1);

		lua_getfield(L, -1, "OneChoiceForAllPlayers");
		m_Def.m_bOneChoiceForAllPlayers = lua_toboolean( L, -1 ) > 0;
		lua_pop( L, 1 );

		lua_getfield(L, -1, "ExportOnChange");
		m_Def.m_bExportOnChange = lua_toboolean( L, -1 ) > 0;
		lua_pop( L, 1 );

		// TODO:  Change these to use the proper enum strings like everything
		// else.  This will break theme compatibility, so it has to wait until
		// after SM5.  -Kyz
		lua_getfield(L, -1, "LayoutType");
		pStr = lua_tostring( L, -1 );
		m_Def.m_layoutType = StringToLayoutType( pStr );
		lua_pop( L, 1 );

		lua_getfield(L, -1, "SelectType");
		pStr = lua_tostring( L, -1 );
		m_Def.m_selectType = StringToSelectType( pStr );
		lua_pop( L, 1 );

		LoadChoices( L );

		// Set the EnabledForPlayers function.
		lua_getfield(L, -1, "EnabledForPlayers");
		m_EnabledForPlayersFunc.SetFromStack( L );
		SetEnabledForPlayers();

		// Iterate over the "ReloadRowMessages" table.
		lua_getfield(L, -1, "ReloadRowMessages");
		if( !lua_isnil( L, -1 ) )
		{
			lua_pushnil( L );
			while( lua_next(L, -2) != 0 )
			{
				// `key' is at index -2 and `value' at index -1
				const char *pValue = lua_tostring( L, -1 );
				//LOG->Trace( "Found ReloadRowMessage '%s'", pValue);
				m_vsReloadRowMessages.push_back( pValue );
				lua_pop( L, 1 ); // removes `value'; keeps `key' for next iteration
			}
		}
		lua_pop( L, 1 ); // pop ReloadRowMessages table

		// Set the Reload function
		lua_getfield(L, -1, "Reload");
		m_ReloadFunc.SetFromStack( L );

		lua_pop( L, 1 ); // pop main table
		ASSERT( lua_gettop(L) == 0 );

		LUA->Release(L);
		return m_TableIsSane;
	}

	virtual ReloadChanged Reload()
	{
		if (!m_TableIsSane)
		{
			return RELOAD_CHANGED_NONE;
		}

		/* We'll always call SetEnabledForPlayers, and
		 * return at least RELOAD_CHANGED_ENABLED,
		 * to preserve original OptionRowHandlerLua behavior.
		 *
		 * Will also call the standard OptionRowHandler::Reload
		 * function to determine whether we should declare a full
		 * RELOAD_CHANGED_ALL
		 */
		ReloadChanged effect = RELOAD_CHANGED_ENABLED;

		if (!m_ReloadFunc.IsNil())
		{
			Lua *L = LUA->Get();
			m_ReloadFunc.PushSelf( L );

			// Argument 1: (self)
			m_pLuaTable->PushSelf( L );
			RString error = "Reload: ";

			LuaHelpers::RunScriptOnStack( L, error, 1, 1, true );
			effect = std::max( effect, Enum::Check<ReloadChanged>( L, -1 ));
			lua_pop( L, 1 );

			if (effect == RELOAD_CHANGED_ALL)
			{
				m_Def.m_vsChoices.clear();
				m_pLuaTable->PushSelf( L );
				LoadChoices( L );
				lua_pop( L, 1 );
				ASSERT( lua_gettop(L) == 0 );
			}
			LUA->Release( L );
		}

		SetEnabledForPlayers();
		return effect;
	}

	virtual void ImportOption( OptionRow *pRow, const std::vector<PlayerNumber> &vpns, std::vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
		if(!m_TableIsSane)
		{
			return;
		}
		Lua *L = LUA->Get();

		ASSERT( lua_gettop(L) == 0 );

		for (PlayerNumber const &p : vpns)
		{
			std::vector<bool> &vbSelOut = vbSelectedOut[p];

			/* Evaluate the LoadSelections(self,array,pn) function, where
			 * array is a table representing vbSelectedOut. */

			// All selections default to false.
			for( unsigned i = 0; i < vbSelOut.size(); ++i )
				vbSelOut[i] = false;

			// Create the vbSelectedOut table
			LuaHelpers::CreateTableFromArrayB( L, vbSelOut );
			ASSERT( lua_gettop(L) == 1 ); // vbSelectedOut table

			// Get the function to call from m_LuaTable.
			m_pLuaTable->PushSelf( L );
			ASSERT( lua_istable( L, -1 ) );

			lua_getfield(L, -1, "LoadSelections");

			// Argument 1 (self):
			m_pLuaTable->PushSelf( L );

			// Argument 2 (vbSelectedOut):
			lua_pushvalue( L, 1 );

			// Argument 3 (pn):
			LuaHelpers::Push( L, p );

			ASSERT( lua_gettop(L) == 6 ); // vbSelectedOut, m_iLuaTable, function, self, arg, arg

			RString error= "LoadSelections: ";
			LuaHelpers::RunScriptOnStack( L, error, 3, 0, true );
			ASSERT( lua_gettop(L) == 2 );

			lua_pop( L, 1 ); // pop option table

			LuaHelpers::ReadArrayFromTableB( L, vbSelOut );

			lua_pop( L, 1 ); // pop vbSelectedOut table

			ASSERT( lua_gettop(L) == 0 );
		}

		LUA->Release(L);
	}
	virtual int ExportOption( const std::vector<PlayerNumber> &vpns, const std::vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		if(!m_TableIsSane)
		{
			return 0;
		}
		Lua *L = LUA->Get();

		ASSERT( lua_gettop(L) == 0 );

		int effects = 0;
		for (PlayerNumber const &p : vpns)
		{
			const std::vector<bool> &vbSel = vbSelected[p];

			/* Evaluate SaveSelections(self,array,pn) function, where array is
			 * a table representing vbSelectedOut. */

			std::vector<bool> vbSelectedCopy = vbSel;

			// Create the vbSelectedOut table.
			LuaHelpers::CreateTableFromArrayB( L, vbSelectedCopy );
			ASSERT( lua_gettop(L) == 1 ); // vbSelectedOut table

			// Get the function to call.
			m_pLuaTable->PushSelf( L );
			ASSERT( lua_istable( L, -1 ) );

			lua_getfield(L, -1, "SaveSelections");

			// Argument 1 (self):
			m_pLuaTable->PushSelf( L );

			// Argument 2 (vbSelectedOut):
			lua_pushvalue( L, 1 );

			// Argument 3 (pn):
			LuaHelpers::Push( L, p );

			ASSERT( lua_gettop(L) == 6 ); // vbSelectedOut, m_iLuaTable, function, self, arg, arg

			RString error= "SaveSelections: ";
			LuaHelpers::RunScriptOnStack( L, error, 3, 1, true );
			ASSERT( lua_gettop(L) == 3 ); // SaveSelections *may* return effects flags, otherwise nil
			double ret = lua_tonumber( L, -1 );
			ASSERT_M( (lua_isnumber( L, -1 ) && std::floor( ret ) == ret) || lua_isnil( L, -1 ),
					  "SaveSelections must return integer flags, or nill" );
			effects |= static_cast<int>( ret );

			lua_pop( L, 1 ); // pop effects
			lua_pop( L, 1 ); // pop option table
			lua_pop( L, 1 ); // pop vbSelected table

			ASSERT( lua_gettop(L) == 0 );
		}

		LUA->Release(L);

		return effects;
	}
	virtual bool NotifyOfSelection(PlayerNumber pn, int choice)
	{
		if(!m_TableIsSane)
		{
			return false;
		}
		Lua *L= LUA->Get();
		m_pLuaTable->PushSelf(L);

		lua_getfield(L, -1, "NotifyOfSelection");
		bool changed= false;
		if(lua_isfunction(L, -1))
		{
			m_pLuaTable->PushSelf(L);
			LuaHelpers::Push(L, pn);
			// Convert choice to a lua index so it matches up with the Choices table.
			lua_pushinteger(L, choice+1);
			RString error= "NotifyOfSelection: ";
			LuaHelpers::RunScriptOnStack(L, error, 3, 1, true);
			if(lua_toboolean(L, -1))
			{
				lua_pop(L, 1);
				changed= true;
				m_Def.m_vsChoices.clear();
				// Iterate over the "Choices" table.
				lua_getfield(L, -1, "Choices");
				lua_pushnil( L );
				while( lua_next(L, -2) != 0 )
				{
					// `key' is at index -2 and `value' at index -1
					const char *pValue = lua_tostring( L, -1 );
					//LOG->Trace( "choice: '%s'", pValue);
					m_Def.m_vsChoices.push_back( pValue );
					lua_pop( L, 1 ); // removes `value'; keeps `key' for next iteration
				}
			}
		}
		lua_settop(L, 0); // Release has an assert that forces a clear stack.
		LUA->Release(L);
		return changed;
	}
	virtual bool GoToFirstOnStart() const
	{
		return m_GoToFirstOnStart;
	}
};

class OptionRowHandlerConfig : public OptionRowHandler
{
public:
	const ConfOption *m_pOpt;

	OptionRowHandlerConfig() { Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		m_pOpt = nullptr;
	}
	virtual bool LoadInternal( const Commands &cmds )
	{
		const Command &command = cmds.v[0];
		RString sParam = command.GetArg(1).s;
		CHECK_WRONG_NUM_ARGS(2);
		CHECK_BLANK_ARG;

		Init();

		// Configuration values are never per-player.
		m_Def.m_bOneChoiceForAllPlayers = true;

		ConfOption *pConfOption = ConfOption::Find( sParam );
		ROW_INVALID_IF(pConfOption == nullptr, "Invalid Conf type \"" + sParam + "\".", false);

		pConfOption->UpdateAvailableOptions();

		m_pOpt = pConfOption;
		m_pOpt->MakeOptionsList( m_Def.m_vsChoices );

		m_Def.m_bAllowThemeItems = m_pOpt->m_bAllowThemeItems;

		m_Def.m_sName = m_pOpt->name;
		return true;
	}
	virtual void ImportOption( OptionRow *, const std::vector<PlayerNumber> &vpns, std::vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
		for (PlayerNumber const &p : vpns)
		{
			std::vector<bool> &vbSelOut = vbSelectedOut[p];

			int iSelection = m_pOpt->Get();
			OptionRowHandlerUtil::SelectExactlyOne( iSelection, vbSelOut );
		}
	}
	virtual int ExportOption( const std::vector<PlayerNumber> &vpns, const std::vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		bool bChanged = false;

		for (PlayerNumber const &p : vpns)
		{
			const std::vector<bool> &vbSel = vbSelected[p];

			int iSel = OptionRowHandlerUtil::GetOneSelection(vbSel);

			// Get the original choice.
			int iOriginal = m_pOpt->Get();

			// Apply.
			m_pOpt->Put( iSel );

			// Get the new choice.
			int iNew = m_pOpt->Get();

			// If it didn't change, don't return any side-effects.
			if( iOriginal != iNew )
				bChanged = true;
		}

		return bChanged ? m_pOpt->GetEffects() : 0;
	}
};

class OptionRowHandlerStepsType : public OptionRowHandler
{
public:
	BroadcastOnChange<StepsType> *m_pstToFill;
	std::vector<StepsType> m_vStepsTypesToShow;

	OptionRowHandlerStepsType() { Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		m_pstToFill = nullptr;
		m_vStepsTypesToShow.clear();
	}

	virtual bool LoadInternal( const Commands &cmds )
	{
		const Command &command = cmds.v[0];
		RString sParam = command.GetArg(1).s;
		CHECK_WRONG_NUM_ARGS(2);
		CHECK_BLANK_ARG;

		if( sParam == "EditStepsType" )
		{
			m_pstToFill = &GAMESTATE->m_stEdit;
		}
		else if( sParam == "EditSourceStepsType" )
		{
			m_pstToFill = &GAMESTATE->m_stEditSource;
			m_vsReloadRowMessages.push_back( MessageIDToString(Message_CurrentStepsP1Changed) );
			m_vsReloadRowMessages.push_back( MessageIDToString(Message_EditStepsTypeChanged) );
			if( GAMESTATE->m_pCurSteps[0].Get() != nullptr )
				m_Def.m_vEnabledForPlayers.clear();	// hide row
		}
		else
		{
			ROW_INVALID_IF(true, "Invalid StepsType param \"" + sParam + "\".", false);
		}

		m_Def.m_sName = sParam;
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		m_Def.m_bExportOnChange = true;
		m_Def.m_bAllowThemeItems = false;	// we theme the text ourself

		// calculate which StepsTypes to show
		m_vStepsTypesToShow = CommonMetrics::STEPS_TYPES_TO_SHOW.GetValue();

		m_Def.m_vsChoices.clear();
		for (StepsType const &st : m_vStepsTypesToShow)
		{
			RString s = GAMEMAN->GetStepsTypeInfo( st ).GetLocalizedString();
			m_Def.m_vsChoices.push_back( s );
		}

		if( *m_pstToFill == StepsType_Invalid )
			m_pstToFill->Set( m_vStepsTypesToShow[0] );
		return true;
	}

	virtual void ImportOption( OptionRow *pRow, const std::vector<PlayerNumber> &vpns, std::vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
		for (PlayerNumber const &p : vpns)
		{
			std::vector<bool> &vbSelOut = vbSelectedOut[p];

			if( GAMESTATE->m_pCurSteps[0] )
			{
				StepsType st = GAMESTATE->m_pCurSteps[0]->m_StepsType;
				std::vector<StepsType>::const_iterator iter = find( m_vStepsTypesToShow.begin(), m_vStepsTypesToShow.end(), st );
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
	virtual int ExportOption( const std::vector<PlayerNumber> &vpns, const std::vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		for (PlayerNumber const &p : vpns)
		{
			const std::vector<bool> &vbSel = vbSelected[p];

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
	virtual bool LoadInternal( const Commands &cmds )
	{
		ROW_INVALID_IF(cmds.v.size() <= 1, "No args to construct GameCommand.", false);

		Commands temp = cmds;
		temp.v.erase( temp.v.begin() );
		m_gc.Load( 0, temp );
		ROW_INVALID_IF(m_gc.m_sName.empty(), "GameCommand row has no name.", false);
		m_Def.m_sName = m_gc.m_sName;
		m_Def.m_bOneChoiceForAllPlayers = true;
		m_Def.m_layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		m_Def.m_selectType = SELECT_NONE;
		m_Def.m_vsChoices.push_back( "" );
		return true;
	}
	virtual void ImportOption( OptionRow *pRow, const std::vector<PlayerNumber> &vpns, std::vector<bool> vbSelectedOut[NUM_PLAYERS] ) const
	{
	}
	virtual int ExportOption( const std::vector<PlayerNumber> &vpns, const std::vector<bool> vbSelected[NUM_PLAYERS] ) const
	{
		if( vbSelected[PLAYER_1][0] || vbSelected[PLAYER_2][0] )
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
	OptionRowHandler* pHand = nullptr;

	ROW_INVALID_IF(cmds.v.size() == 0, "No commands for constructing row.", nullptr);
	const RString &name = cmds.v[0].GetName();
	ROW_INVALID_IF(name != "gamecommand" && cmds.v.size() != 1,
		"Row must be constructed from single command.", nullptr);

	bool load_succeeded= false;
#define MAKE( type )	{ type *p = new type; load_succeeded= p->Load( cmds ); pHand = p; }

	// XXX: merge these, and merge "Steps" and "list,Steps"
	if( name == "list" )
	{
		const Command &command = cmds.v[0];
		RString sParam = command.GetArg(1).s;
		ROW_INVALID_IF(command.m_vsArgs.size() != 2 || !sParam.size(),
			"list row command must be 'list,name' or 'list,type'.", nullptr);

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
	else
	{
		ROW_INVALID_IF(true, "Invalid row type.", nullptr);
	}

	if(load_succeeded)
	{
		return pHand;
	}
	return nullptr;
}

OptionRowHandler* OptionRowHandlerUtil::MakeNull()
{
	OptionRowHandler* pHand = nullptr;
	bool load_succeeded= false; // Part of the MAKE macro, but unused.
	Commands cmds;
	MAKE( OptionRowHandlerNull )
	if(load_succeeded) // Just to get rid of the warning for not using it.
	{
		return pHand;
	}
	return nullptr;
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

	// MISTER CHOICES!
	pHand->m_Def.m_vsChoices = mr.choices;

	// Each row must have at least one choice.
	if( pHand->m_Def.m_vsChoices.empty() )
		pHand->m_Def.m_vsChoices.push_back( "" );

	pHand->m_Def.m_bAllowThemeTitle = mr.bThemeTitle;
	pHand->m_Def.m_bAllowThemeItems = mr.bThemeItems;

	for (RString &c : pHand->m_Def.m_vsChoices)
		FontCharAliases::ReplaceMarkers( c );	// Allow special characters

	return pHand;
}

// Expose ReloadChanged to Lua
static const char *ReloadChangedNames[] =
	{
		"None",
		"Enabled",
		"All"
	};
XToString( ReloadChanged );
StringToX( ReloadChanged );
LuaXType( ReloadChanged );

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
