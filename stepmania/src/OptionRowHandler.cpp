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
#include "StepsUtil.h"
#include "GameManager.h"
#include "Foreach.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"

#define ENTRY(s)					THEME->GetMetric ("ScreenOptionsMaster",s)
#define ENTRY_MODE(s,i)				THEME->GetMetric ("ScreenOptionsMaster",ssprintf("%s,%i",(s).c_str(),(i+1)))
#define ENTRY_DEFAULT(s)			THEME->GetMetric ("ScreenOptionsMaster",(s) + "Default")

#define STEPS_TYPES_TO_HIDE			THEME->GetMetric ("OptionRowHandler","StepsTypesToHide")

static void SelectExactlyOne( int iSelection, vector<bool> &vbSelectedOut )
{
	for( int i=0; i<(int)vbSelectedOut.size(); i++ )
		vbSelectedOut[i] = i==iSelection;
}

static int GetOneSelection( const vector<bool> &vbSelected )
{
	for( unsigned i=0; i<vbSelected.size(); i++ )
		if( vbSelected[i] )
			return i;
	ASSERT(0);	// shouldn't call this if not expecting one to be selected
	return -1;
}


class OptionRowHandlerList : public OptionRowHandler
{
public:
	vector<GameCommand> ListEntries;
	GameCommand Default;
	bool m_bUseModNameForIcon;

	OptionRowHandlerList::OptionRowHandlerList() { Init(); }
	virtual void Init()
	{
		OptionRowHandler::Init();
		ListEntries.clear();
		Default.Init();
		m_bUseModNameForIcon = false;
	}
	virtual void Load( OptionRowDefinition &defOut, CString sParam )
	{
		ASSERT( sParam.size() );

		if(		 sParam.CompareNoCase("NoteSkins")==0 )		{ FillNoteSkins( defOut, sParam );		return; }
		else if( sParam.CompareNoCase("Steps")==0 )			{ FillSteps( defOut, sParam );			return; }
		else if( sParam.CompareNoCase("EditsAndNull")==0 )	{ FillEditsAndNull( defOut, sParam );	return; }
		else if( sParam.CompareNoCase("Characters")==0 )	{ FillCharacters( defOut, sParam );		return; }
		else if( sParam.CompareNoCase("Styles")==0 )		{ FillStyles( defOut, sParam );			return; }
		else if( sParam.CompareNoCase("Groups")==0 )		{ FillGroups( defOut, sParam );			return; }
		else if( sParam.CompareNoCase("Difficulties")==0 )	{ FillDifficulties( defOut, sParam );	return; }
		else if( sParam.CompareNoCase("SongsInCurrentSongGroup")==0 )	{ FillSongsInCurrentSongGroup( defOut, sParam );	return; }

		Init();
		defOut.Init();

		m_bUseModNameForIcon = true;
			
		defOut.name = sParam;

		Default.Load( -1, ParseCommands(ENTRY_DEFAULT(sParam)) );

		/* Parse the basic configuration metric. */
		Commands cmds = ParseCommands( ENTRY(sParam) );
		if( cmds.v.size() < 1 )
			RageException::Throw( "Parse error in OptionRowHandlerUtilEntries::ListName%s", sParam.c_str() );

		defOut.bOneChoiceForAllPlayers = false;
		const int NumCols = atoi( cmds.v[0].m_vsArgs[0] );
		for( unsigned i=1; i<cmds.v.size(); i++ )
		{
			const Command &cmd = cmds.v[i];
			CString sName = cmd.GetName();

			if(		 sName == "together" )			defOut.bOneChoiceForAllPlayers = true;
			else if( sName == "selectmultiple" )	defOut.selectType = SELECT_MULTIPLE;
			else if( sName == "selectnone" )		defOut.selectType = SELECT_NONE;
			else if( sName == "showoneinrow" )		defOut.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
			else if( sName == "reloadrowmessages" )
			{
				for( unsigned a=1; a<cmd.m_vsArgs.size(); a++ )
					m_vsReloadRowMessages.push_back( cmd.m_vsArgs[a] );
			}
			else if( sName == "enabledforplayers" )
			{
				defOut.m_vEnabledForPlayers.clear();
				for( unsigned a=1; a<cmd.m_vsArgs.size(); a++ )
				{
					CString sArg = cmd.m_vsArgs[a];
					PlayerNumber pn = (PlayerNumber)(atoi(sArg)-1);
					ASSERT( pn >= 0 && pn < NUM_PLAYERS );
					defOut.m_vEnabledForPlayers.insert( pn );
				}
			}
			else if( sName == "exportonchange" )	defOut.m_bExportOnChange = true;
			else		RageException::Throw( "Unkown row flag \"%s\"", sName.c_str() );
		}

		for( int col = 0; col < NumCols; ++col )
		{
			GameCommand mc;
			mc.Load( 0, ParseCommands(ENTRY_MODE(sParam, col)) );
			if( mc.m_sName == "" )
				RageException::Throw( "List \"%s\", col %i has no name", sParam.c_str(), col );

			if( !mc.IsPlayable() )
				continue;

			ListEntries.push_back( mc );

			CString sName = mc.m_sName;
			CString sChoice = ENTRY_NAME(mc.m_sName);
			defOut.choices.push_back( sChoice );
		}
	}
	void ImportOption( const OptionRowDefinition &def, PlayerNumber pn, vector<bool> &vbSelectedOut ) const
	{
		int FallbackOption = -1;
		bool UseFallbackOption = true;

		for( unsigned e = 0; e < ListEntries.size(); ++e )
		{
			const GameCommand &mc = ListEntries[e];

			vbSelectedOut[e] = false;

			if( mc.IsZero() )
			{
				/* The entry has no effect.  This is usually a default "none of the
					* above" entry.  It will always return true for DescribesCurrentMode().
					* It's only the selected choice if nothing else matches. */
				if( def.selectType != SELECT_MULTIPLE )
					FallbackOption = e;
				continue;
			}

			if( def.bOneChoiceForAllPlayers )
			{
				if( mc.DescribesCurrentModeForAllPlayers() )
				{
					UseFallbackOption = false;
					if( def.selectType != SELECT_MULTIPLE )
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
					if( def.selectType != SELECT_MULTIPLE )
						SelectExactlyOne( e, vbSelectedOut );
					else
						vbSelectedOut[e] = true;
				}
			}
		}

		if( def.selectType == SELECT_ONE && 
			UseFallbackOption && 
			FallbackOption != -1 )
		{
			SelectExactlyOne( FallbackOption, vbSelectedOut );
		}
	}

	int ExportOption( const OptionRowDefinition &def, PlayerNumber pn, const vector<bool> &vbSelected ) const
	{
		Default.Apply( pn );
		for( unsigned i=0; i<vbSelected.size(); i++ )
			if( vbSelected[i] )
				ListEntries[i].Apply( pn );
		return 0;
	}

	virtual CString GetIconText( const OptionRowDefinition &def, int iFirstSelection ) const
	{
		return m_bUseModNameForIcon ?
			ListEntries[iFirstSelection].m_sModifiers :
			def.choices[iFirstSelection];
	}
	virtual CString GetAndEraseScreen( int iChoice )
	{ 
		GameCommand &mc = ListEntries[iChoice];
		if( mc.m_sScreen != "" )
		{
			/* Hack: instead of applying screen commands here, store them in
			* m_sNextScreen and apply them after we tween out.  If we don't set
			* m_sScreen to "", we'll load it twice (once for each player) and
			* then again for m_sNextScreen. */
			CString sNextScreen = mc.m_sScreen;
			mc.m_sScreen = "";
			return sNextScreen;
		}
		return "";
	}

	void FillNoteSkins( OptionRowDefinition &defOut, CString sParam )
	{
		Init();
		defOut.Init();

		ASSERT( sParam.size() );
		m_sName = sParam;

		defOut.name = "NoteSkins";
		defOut.bOneChoiceForAllPlayers = false;

		CStringArray arraySkinNames;
		NOTESKIN->GetNoteSkinNames( arraySkinNames );
		for( unsigned skin=0; skin<arraySkinNames.size(); skin++ )
		{
			arraySkinNames[skin].MakeUpper();

			GameCommand mc;
			mc.m_sModifiers = arraySkinNames[skin];
			ListEntries.push_back( mc );
			defOut.choices.push_back( arraySkinNames[skin] );
		}
	}

	void FillSteps( OptionRowDefinition &defOut, CString sParam )
	{
		Init();
		defOut.Init();

		ASSERT( sParam.size() );
		m_sName = sParam;

		defOut.name = "Steps";
		defOut.bOneChoiceForAllPlayers = false;

		// fill in difficulty names
		if( GAMESTATE->m_bEditing )
		{
			defOut.choices.push_back( "" );
			ListEntries.push_back( GameCommand() );
		}
		else if( GAMESTATE->IsCourseMode() )   // playing a course
		{
			defOut.bOneChoiceForAllPlayers = PREFSMAN->m_bLockCourseDifficulties;

			vector<Trail*> vTrails;
			GAMESTATE->m_pCurCourse->GetTrails( vTrails, GAMESTATE->GetCurrentStyle()->m_StepsType );
			for( unsigned i=0; i<vTrails.size(); i++ )
			{
				Trail* pTrail = vTrails[i];

				CString s = CourseDifficultyToThemedString( pTrail->m_CourseDifficulty );
				defOut.choices.push_back( s );
				GameCommand mc;
				mc.m_pTrail = pTrail;
				ListEntries.push_back( mc );
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
				defOut.choices.push_back( s );
				GameCommand mc;
				mc.m_pSteps = pSteps;
				mc.m_dc = pSteps->GetDifficulty();
				ListEntries.push_back( mc );
			}
		}
	}

	void FillEditsAndNull( OptionRowDefinition &defOut, CString sParam )
	{
		Init();
		defOut.Init();

		ASSERT( sParam.size() );
		m_sName = sParam;

		defOut.name = "EditsAndNull";
		defOut.bOneChoiceForAllPlayers = true;
		defOut.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		defOut.m_bExportOnChange = true;
		m_vsReloadRowMessages.push_back( MessageToString(MESSAGE_CURRENT_SONG_CHANGED) );
		m_vsReloadRowMessages.push_back( MessageToString(MESSAGE_EDIT_STEPS_TYPE_CHANGED) );

		if( GAMESTATE->m_pCurSong != NULL )
		{
			vector<Steps*> vSteps;
			GAMESTATE->m_pCurSong->GetSteps( vSteps, GAMESTATE->m_stEdit, DIFFICULTY_EDIT );
			StepsUtil::SortNotesArrayByDifficulty( vSteps );
			FOREACH_CONST( Steps*, vSteps, p )
			{
				CString s = (*p)->GetDescription();
				defOut.choices.push_back( s );
				GameCommand mc;
				mc.m_pSteps = *p;
				ListEntries.push_back( mc );
			}
		}

		// Add NULL entry for a new edit
		{
			defOut.choices.push_back( ENTRY_NAME("NewEdit") );
			GameCommand mc;
			ListEntries.push_back( mc );
		}
	}

	void FillCharacters( OptionRowDefinition &defOut, CString sParam )
	{
		Init();
		defOut.Init();

		ASSERT( sParam.size() );
		m_sName = sParam;

		defOut.bOneChoiceForAllPlayers = false;
		defOut.name = "Characters";
		Default.m_pCharacter = GAMESTATE->GetDefaultCharacter();

		{
			defOut.choices.push_back( ENTRY_NAME("Off") );
			GameCommand mc;
			mc.m_pCharacter = NULL;
			ListEntries.push_back( mc );
		}

		vector<Character*> apCharacters;
		GAMESTATE->GetCharacters( apCharacters );
		for( unsigned i=0; i<apCharacters.size(); i++ )
		{
			Character* pCharacter = apCharacters[i];
			CString s = pCharacter->m_sName;
			s.MakeUpper();

			defOut.choices.push_back( s ); 
			GameCommand mc;
			mc.m_pCharacter = pCharacter;
			ListEntries.push_back( mc );
		}
	}

	void FillStyles( OptionRowDefinition &defOut, CString sParam )
	{
		Init();
		defOut.Init();

		ASSERT( sParam.size() );
		m_sName = sParam;

		defOut.bOneChoiceForAllPlayers = true;
		defOut.name = "Style";
		defOut.bOneChoiceForAllPlayers = true;

		vector<const Style*> vStyles;
		GAMEMAN->GetStylesForGame( GAMESTATE->m_pCurGame, vStyles );
		ASSERT( vStyles.size() );
		FOREACH_CONST( const Style*, vStyles, s )
		{
			defOut.choices.push_back( GAMEMAN->StyleToThemedString(*s) ); 
			GameCommand mc;
			mc.m_pStyle = *s;
			ListEntries.push_back( mc );
		}

		Default.m_pStyle = vStyles[0];
	}

	void FillGroups( OptionRowDefinition &defOut, CString sParam )
	{
		Init();
		defOut.Init();

		ASSERT( sParam.size() );
		m_sName = sParam;

		defOut.bOneChoiceForAllPlayers = true;
		defOut.name = "Group";
		Default.m_sSongGroup = GROUP_ALL_MUSIC;

		vector<CString> vGroups;
		SONGMAN->GetGroupNames( vGroups );
		ASSERT( vGroups.size() );

		{
			defOut.choices.push_back( ENTRY_NAME("AllGroups") );
			GameCommand mc;
			mc.m_sSongGroup = GROUP_ALL_MUSIC;
			ListEntries.push_back( mc );
		}

		FOREACH_CONST( CString, vGroups, g )
		{
			defOut.choices.push_back( *g ); 
			GameCommand mc;
			mc.m_sSongGroup = *g;
			ListEntries.push_back( mc );
		}
	}

	void FillDifficulties( OptionRowDefinition &defOut, CString sParam )
	{
		Init();
		defOut.Init();

		ASSERT( sParam.size() );
		m_sName = sParam;

		set<Difficulty> vDifficulties;
		GAMESTATE->GetDifficultiesToShow( vDifficulties );

		defOut.bOneChoiceForAllPlayers = true;
		defOut.name = "Difficulty";
		Default.m_dc = DIFFICULTY_INVALID;

		{
			defOut.choices.push_back( ENTRY_NAME("AllDifficulties") );
			GameCommand mc;
			mc.m_dc = DIFFICULTY_INVALID;
			ListEntries.push_back( mc );
		}

		FOREACHS_CONST( Difficulty, vDifficulties, d )
		{
			CString s = DifficultyToThemedString( *d );

			defOut.choices.push_back( s ); 
			GameCommand mc;
			mc.m_dc = *d;
			ListEntries.push_back( mc );
		}
	}

	void FillSongsInCurrentSongGroup( OptionRowDefinition &defOut, CString sParam )
	{
		Init();
		defOut.Init();

		ASSERT( sParam.size() );
		m_sName = sParam;

		vector<Song*> vpSongs;
		SONGMAN->GetSongs( vpSongs, GAMESTATE->m_sPreferredSongGroup );

		if( GAMESTATE->m_pCurSong == NULL )
			GAMESTATE->m_pCurSong.Set( vpSongs[0] );

		defOut.name = "SongsInCurrentSongGroup";
		defOut.bOneChoiceForAllPlayers = true;
		defOut.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		defOut.m_bExportOnChange = true;

		FOREACH_CONST( Song*, vpSongs, p )
		{
			defOut.choices.push_back( (*p)->GetFullTranslitTitle() ); 
			GameCommand mc;
			mc.m_pSong = *p;
			ListEntries.push_back( mc );
		}
	}
};

class OptionRowHandlerLua : public OptionRowHandler
{
public:
	LuaExpression *m_pLuaTable;

	OptionRowHandlerLua::OptionRowHandlerLua() { m_pLuaTable = NULL; Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		delete m_pLuaTable;
		m_pLuaTable = new LuaExpression;
	}
	virtual void Load( OptionRowDefinition &defOut, CString sLuaFunction )
	{
		ASSERT( sLuaFunction.size() );

		Init();
		defOut.Init();

		m_sName = sLuaFunction;
	//	m_bUseModNameForIcon = true;

		/* Run the Lua expression.  It should return a table. */
		m_pLuaTable->SetFromExpression( sLuaFunction );

		if( m_pLuaTable->GetLuaType() != LUA_TTABLE )
			RageException::Throw( "Result of \"%s\" is not a table", sLuaFunction.c_str() );

		{
			m_pLuaTable->PushSelf( LUA->L );

			lua_pushstring( LUA->L, "Name" );
			lua_gettable( LUA->L, -2 );
			const char *pStr = lua_tostring( LUA->L, -1 );
			if( pStr == NULL )
				RageException::Throw( "\"%s\" \"Name\" entry is not a string", sLuaFunction.c_str() );
			defOut.name = pStr;
			lua_pop( LUA->L, 1 );


			lua_pushstring( LUA->L, "OneChoiceForAllPlayers" );
			lua_gettable( LUA->L, -2 );
			defOut.bOneChoiceForAllPlayers = !!lua_toboolean( LUA->L, -1 );
			lua_pop( LUA->L, 1 );


			lua_pushstring( LUA->L, "LayoutType" );
			lua_gettable( LUA->L, -2 );
			pStr = lua_tostring( LUA->L, -1 );
			if( pStr == NULL )
				RageException::Throw( "\"%s\" \"LayoutType\" entry is not a string", sLuaFunction.c_str() );
			defOut.layoutType = StringToLayoutType( pStr );
			ASSERT( defOut.layoutType != LAYOUT_INVALID );
			lua_pop( LUA->L, 1 );


			lua_pushstring( LUA->L, "SelectType" );
			lua_gettable( LUA->L, -2 );
			pStr = lua_tostring( LUA->L, -1 );
			if( pStr == NULL )
				RageException::Throw( "\"%s\" \"SelectType\" entry is not a string", sLuaFunction.c_str() );
			defOut.selectType = StringToSelectType( pStr );
			ASSERT( defOut.selectType != SELECT_INVALID );
			lua_pop( LUA->L, 1 );


			/* Iterate over the "Choices" table. */
			lua_pushstring( LUA->L, "Choices" );
			lua_gettable( LUA->L, -2 );
			if( !lua_istable( LUA->L, -1 ) )
				RageException::Throw( "\"%s\" \"Choices\" is not a table", sLuaFunction.c_str() );

			lua_pushnil( LUA->L );
			while( lua_next(LUA->L, -2) != 0 )
			{
				/* `key' is at index -2 and `value' at index -1 */
				const char *pValue = lua_tostring( LUA->L, -1 );
				if( pValue == NULL )
					RageException::Throw( "\"%s\" Column entry is not a string", sLuaFunction.c_str() );
				LOG->Trace( "'%s'", pValue);

				defOut.choices.push_back( pValue );

				lua_pop( LUA->L, 1 );  /* removes `value'; keeps `key' for next iteration */
			}

			lua_pop( LUA->L, 1 ); /* pop choices table */


			/* Iterate over the "EnabledForPlayers" table. */
			lua_pushstring( LUA->L, "EnabledForPlayers" );
			lua_gettable( LUA->L, -2 );
			if( !lua_isnil( LUA->L, -1 ) )
			{
				if( !lua_istable( LUA->L, -1 ) )
					RageException::Throw( "\"%s\" \"EnabledForPlayers\" is not a table", sLuaFunction.c_str() );

				defOut.m_vEnabledForPlayers.clear();	// and fill in with supplied PlayerNumbers below

				lua_pushnil( LUA->L );
				while( lua_next(LUA->L, -2) != 0 )
				{
					/* `key' is at index -2 and `value' at index -1 */
					PlayerNumber pn = (PlayerNumber)luaL_checkint( LUA->L, -1 );

					defOut.m_vEnabledForPlayers.insert( pn );

					lua_pop( LUA->L, 1 );  /* removes `value'; keeps `key' for next iteration */
				}
			}
			lua_pop( LUA->L, 1 ); /* pop EnabledForPlayers table */

			
			/* Look for "ExportOnChange" value. */
			lua_pushstring( LUA->L, "ExportOnChange" );
			lua_gettable( LUA->L, -2 );
			if( !lua_isnil( LUA->L, -1 ) )
			{
				defOut.m_bExportOnChange = !!MyLua_checkboolean( LUA->L, -1 );
			}
			lua_pop( LUA->L, 1 ); /* pop ExportOnChange value */

			
			/* Iterate over the "ReloadRowMessages" table. */
			lua_pushstring( LUA->L, "ReloadRowMessages" );
			lua_gettable( LUA->L, -2 );
			if( !lua_isnil( LUA->L, -1 ) )
			{
				if( !lua_istable( LUA->L, -1 ) )
					RageException::Throw( "\"%s\" \"ReloadRowMessages\" is not a table", sLuaFunction.c_str() );

				m_vsReloadRowMessages.clear();	// and fill in with supplied PlayerNumbers below

				lua_pushnil( LUA->L );
				while( lua_next(LUA->L, -2) != 0 )
				{
					/* `key' is at index -2 and `value' at index -1 */
					const char *pValue = lua_tostring( LUA->L, -1 );
					if( pValue == NULL )
						RageException::Throw( "\"%s\" Column entry is not a string", sLuaFunction.c_str() );
					LOG->Trace( "'%s'", pValue);

					m_vsReloadRowMessages.push_back( pValue );

					lua_pop( LUA->L, 1 );  /* removes `value'; keeps `key' for next iteration */
				}
			}
			lua_pop( LUA->L, 1 ); /* pop ReloadRowMessages table */


			lua_pop( LUA->L, 1 ); /* pop main table */
			ASSERT( lua_gettop(LUA->L) == 0 );
		}
	}
	virtual void ImportOption( const OptionRowDefinition &def, PlayerNumber pn, vector<bool> &vbSelectedOut ) const
	{
		ASSERT( lua_gettop(LUA->L) == 0 );

		/* Evaluate the LoadSelections(self,array,pn) function, where array is a table
		 * representing vbSelectedOut. */

		/* All selections default to false. */
		for( unsigned i = 0; i < vbSelectedOut.size(); ++i )
			vbSelectedOut[i] = false;

		/* Create the vbSelectedOut table. */
		LUA->CreateTableFromArrayB( vbSelectedOut );
		ASSERT( lua_gettop(LUA->L) == 1 ); /* vbSelectedOut table */

		/* Get the function to call from m_LuaTable. */
		m_pLuaTable->PushSelf( LUA->L );
		ASSERT( lua_istable( LUA->L, -1 ) );

		lua_pushstring( LUA->L, "LoadSelections" );
		lua_gettable( LUA->L, -2 );
		if( !lua_isfunction( LUA->L, -1 ) )
			RageException::Throw( "\"%s\" \"LoadSelections\" entry is not a function", def.name.c_str() );

		/* Argument 1 (self): */
		m_pLuaTable->PushSelf( LUA->L );

		/* Argument 2 (vbSelectedOut): */
		lua_pushvalue( LUA->L, 1 );

		/* Argument 3 (pn): */
		LUA->PushStack( (int) pn );

		ASSERT( lua_gettop(LUA->L) == 6 ); /* vbSelectedOut, m_iLuaTable, function, self, arg, arg */

		lua_call( LUA->L, 3, 0 ); // call function with 3 arguments and 0 results
		ASSERT( lua_gettop(LUA->L) == 2 );

		lua_pop( LUA->L, 1 ); /* pop option table */

		LUA->ReadArrayFromTableB( vbSelectedOut );
		
		lua_pop( LUA->L, 1 ); /* pop vbSelectedOut table */

		ASSERT( lua_gettop(LUA->L) == 0 );
	}
    virtual int ExportOption( const OptionRowDefinition &def, PlayerNumber pn, const vector<bool> &vbSelected ) const
	{
		ASSERT( lua_gettop(LUA->L) == 0 );

		/* Evaluate SaveSelections(self,array,pn) function, where array is a table
			* representing vbSelectedOut. */

		vector<bool> vbSelectedCopy = vbSelected;

		/* Create the vbSelectedOut table. */
		LUA->CreateTableFromArrayB( vbSelectedCopy );
		ASSERT( lua_gettop(LUA->L) == 1 ); /* vbSelectedOut table */

		/* Get the function to call. */
		m_pLuaTable->PushSelf( LUA->L );
		ASSERT( lua_istable( LUA->L, -1 ) );

		lua_pushstring( LUA->L, "SaveSelections" );
		lua_gettable( LUA->L, -2 );
		if( !lua_isfunction( LUA->L, -1 ) )
			RageException::Throw( "\"%s\" \"SaveSelections\" entry is not a function", def.name.c_str() );

		/* Argument 1 (self): */
		m_pLuaTable->PushSelf( LUA->L );

		/* Argument 2 (vbSelectedOut): */
		lua_pushvalue( LUA->L, 1 );

		/* Argument 3 (pn): */
		LUA->PushStack( (int) pn );

		ASSERT( lua_gettop(LUA->L) == 6 ); /* vbSelectedOut, m_iLuaTable, function, self, arg, arg */

		lua_call( LUA->L, 3, 0 ); // call function with 3 arguments and 0 results
		ASSERT( lua_gettop(LUA->L) == 2 );

		lua_pop( LUA->L, 1 ); /* pop option table */
		lua_pop( LUA->L, 1 ); /* pop vbSelected table */

		ASSERT( lua_gettop(LUA->L) == 0 );

		// XXX: allow specifying the mask
		return 0;
	}
};

class OptionRowHandlerConfig : public OptionRowHandler
{
public:
	const ConfOption *opt;

	OptionRowHandlerConfig::OptionRowHandlerConfig() { Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		opt = NULL;
	}
	virtual void Load( OptionRowDefinition &defOut, CString sParam )
	{
		ASSERT( sParam.size() );

		Init();
		defOut.Init();

		/* Configuration values are never per-player. */
		defOut.bOneChoiceForAllPlayers = true;

		ConfOption *pConfOption = ConfOption::Find( sParam );
		if( pConfOption == NULL )
			RageException::Throw( "Invalid Conf type \"%s\"", sParam.c_str() );

   		pConfOption->UpdateAvailableOptions();

		opt = pConfOption;
		opt->MakeOptionsList( defOut.choices );

		defOut.name = opt->name;
	}
	virtual void ImportOption( const OptionRowDefinition &def, PlayerNumber pn, vector<bool> &vbSelectedOut ) const
	{
		int iSelection = opt->Get();
		SelectExactlyOne( iSelection, vbSelectedOut );
	}
	virtual int ExportOption( const OptionRowDefinition &def, PlayerNumber pn, const vector<bool> &vbSelected ) const
	{
		int sel = GetOneSelection(vbSelected);

		/* Get the original choice. */
		int Original = opt->Get();

		/* Apply. */
		opt->Put( sel );

		/* Get the new choice. */
		int New = opt->Get();

		/* If it didn't change, don't return any side-effects. */
		if( Original == New )
			return 0;

		return opt->GetEffects();
	}
};

class OptionRowHandlerStepsType : public OptionRowHandler
{
public:
	BroadcastOnChange<StepsType> *m_pstToFill;
	vector<StepsType> m_vStepsTypesToShow;

	OptionRowHandlerStepsType::OptionRowHandlerStepsType() { Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		m_pstToFill = NULL;
		m_vStepsTypesToShow.clear();
	}

	virtual void Load( OptionRowDefinition &defOut, CString sParam )
	{
		ASSERT( sParam.size() );

		Init();
		defOut.Init();

		if( sParam == "EditStepsType" )
		{
			m_pstToFill = &GAMESTATE->m_stEdit;
		}
		else if( sParam == "EditSourceStepsType" )
		{
			m_pstToFill = &GAMESTATE->m_stEditSource;
			m_vsReloadRowMessages.push_back( MessageToString(MESSAGE_CURRENT_STEPS_P1_CHANGED) );
			m_vsReloadRowMessages.push_back( MessageToString(MESSAGE_EDIT_STEPS_TYPE_CHANGED) );
			if( GAMESTATE->m_pCurSteps[0].Get() != NULL )
				defOut.m_vEnabledForPlayers.clear();	// hide row
		}
		else
		{
			RageException::Throw( "invalid StepsType param \"%s\"", sParam.c_str() );
		}

		m_sName = sParam;
		defOut.name = sParam;
		defOut.bOneChoiceForAllPlayers = true;
		defOut.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		defOut.m_bExportOnChange = true;

		// calculate which StepsTypes to show
		GAMEMAN->GetStepsTypesForGame( GAMESTATE->m_pCurGame, m_vStepsTypesToShow );

		// subtract hidden StepsTypes
		vector<CString> vsStepsTypesToHide;
		split( STEPS_TYPES_TO_HIDE, ",", vsStepsTypesToHide, true );
		for( unsigned i=0; i<vsStepsTypesToHide.size(); i++ )
		{
			StepsType st = GameManager::StringToStepsType(vsStepsTypesToHide[i]);
			if( st != STEPS_TYPE_INVALID )
			{
				const vector<StepsType>::iterator iter = find( m_vStepsTypesToShow.begin(), m_vStepsTypesToShow.end(), st );
				if( iter != m_vStepsTypesToShow.end() )
					m_vStepsTypesToShow.erase( iter );
			}
		}

		FOREACH_CONST( StepsType, m_vStepsTypesToShow, st )
		{
			CString s = GAMEMAN->StepsTypeToThemedString( *st );
			defOut.choices.push_back( s );
		}

		if( *m_pstToFill == STEPS_TYPE_INVALID )
			m_pstToFill->Set( m_vStepsTypesToShow[0] );
	}
	virtual void ImportOption( const OptionRowDefinition &def, PlayerNumber pn, vector<bool> &vbSelectedOut ) const
	{
		if( GAMESTATE->m_pCurSteps[0] )
		{
			StepsType st = GAMESTATE->m_pCurSteps[0]->m_StepsType;
			vector<StepsType>::const_iterator iter = find( m_vStepsTypesToShow.begin(), m_vStepsTypesToShow.end(), st );
			if( iter != m_vStepsTypesToShow.end() )
			{
				unsigned i = iter - m_vStepsTypesToShow.begin();
				vbSelectedOut[i] = true;
				return;
			}
		}
		vbSelectedOut[0] = true;
	}
	virtual int ExportOption( const OptionRowDefinition &def, PlayerNumber pn, const vector<bool> &vbSelected ) const
	{
		int index = GetOneSelection( vbSelected );
		m_pstToFill->Set( m_vStepsTypesToShow[index] );
		return 0;
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

	OptionRowHandlerSteps::OptionRowHandlerSteps() { Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		m_ppStepsToFill = NULL;
		m_pDifficultyToFill = NULL;
		m_vSteps.clear();
		m_vDifficulties.clear();
	}

	virtual void Load( OptionRowDefinition &defOut, CString sParam )
	{
		ASSERT( sParam.size() );

		Init();
		defOut.Init();

		if( sParam == "EditSteps" )
		{
			m_ppStepsToFill = &GAMESTATE->m_pCurSteps[0];
			m_pDifficultyToFill = &GAMESTATE->m_PreferredDifficulty[0];
			m_pst = &GAMESTATE->m_stEdit;
			m_vsReloadRowMessages.push_back( MessageToString(MESSAGE_EDIT_STEPS_TYPE_CHANGED) );
		}
		else if( sParam == "EditSourceSteps" )
		{
			m_ppStepsToFill = &GAMESTATE->m_pEditSourceSteps;
			m_pst = &GAMESTATE->m_stEditSource;
			m_vsReloadRowMessages.push_back( MessageToString(MESSAGE_EDIT_SOURCE_STEPS_TYPE_CHANGED) );
			if( GAMESTATE->m_pCurSteps[0].Get() != NULL )
				defOut.m_vEnabledForPlayers.clear();	// hide row
		}
		else
		{
			RageException::Throw( "invalid StepsType param \"%s\"", sParam.c_str() );
		}
		
		m_sName = sParam;
		defOut.name = sParam;
		defOut.bOneChoiceForAllPlayers = true;
		defOut.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		defOut.m_bExportOnChange = true;
		m_vsReloadRowMessages.push_back( MessageToString(MESSAGE_CURRENT_SONG_CHANGED) );

		if( GAMESTATE->m_pCurSong )
		{
			FOREACH_Difficulty( dc )
			{
				if( dc == DIFFICULTY_EDIT )
					continue;
				m_vDifficulties.push_back( dc );
				Steps* pSteps = GAMESTATE->m_pCurSong->GetStepsByDifficulty( *m_pst, dc );
				m_vSteps.push_back( pSteps );
			}
			GAMESTATE->m_pCurSong->GetSteps( m_vSteps, *m_pst, DIFFICULTY_EDIT );
			m_vDifficulties.resize( m_vSteps.size(), DIFFICULTY_EDIT );

			if( m_sName == "EditSteps" )
			{
				m_vSteps.push_back( NULL );
				m_vDifficulties.push_back( DIFFICULTY_EDIT );
			}

			for( unsigned i=0; i<m_vSteps.size(); i++ )
			{
				Steps* pSteps = m_vSteps[i];
				Difficulty dc = m_vDifficulties[i];

				CString s;
				if( dc == DIFFICULTY_EDIT )
				{
					if( pSteps )
						s = pSteps->GetDescription();
					else
						s = ENTRY_NAME("NewEdit");
				}
				else
				{
					s = DifficultyToThemedString( dc );
				}
				defOut.choices.push_back( s );
			}
		}
		else
		{
			m_vDifficulties.push_back( DIFFICULTY_EDIT );
			m_vSteps.push_back( NULL );
			defOut.choices.push_back( "none" );
		}

		if( m_pDifficultyToFill )
			m_pDifficultyToFill->Set( m_vDifficulties[0] );
		m_ppStepsToFill->Set( m_vSteps[0] );
	}
	virtual void ImportOption( const OptionRowDefinition &def, PlayerNumber pn, vector<bool> &vbSelectedOut ) const
	{
		ASSERT( m_vSteps.size() == vbSelectedOut.size() );

		// look for matching steps
		vector<Steps*>::const_iterator iter = find( m_vSteps.begin(), m_vSteps.end(), m_ppStepsToFill->Get() );
		if( iter != m_vSteps.end() )
		{
			unsigned i = iter - m_vSteps.begin();
			vbSelectedOut[i] = true;
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
					vbSelectedOut[i] = true;
					ExportOption( def, pn, vbSelectedOut );	// current steps changed
					return;
				}
			}
		}
		// default to 1st
		vbSelectedOut[0] = true;
	}
	virtual int ExportOption( const OptionRowDefinition &def, PlayerNumber pn, const vector<bool> &vbSelected ) const
	{
		int index = GetOneSelection( vbSelected );
		Difficulty dc = m_vDifficulties[index];
		Steps *pSteps = m_vSteps[index];
		if( m_pDifficultyToFill )
			m_pDifficultyToFill->Set( dc );
		m_ppStepsToFill->Set( pSteps );
		return 0;
	}
};

// helpers for MenuStart() below
static void DeleteCurNotes( void* pThrowAway )
{
	Song* pSong = GAMESTATE->m_pCurSong;
	Steps* pStepsToDelete = GAMESTATE->m_pCurSteps[PLAYER_1];
	pSong->RemoveSteps( pStepsToDelete );
	pSong->Save();

	// refresh the list
	MESSAGEMAN->Broadcast( MESSAGE_CURRENT_SONG_CHANGED );
}


class OptionRowHandlerEditMenuAction : public OptionRowHandler
{
public:
	vector<EditMenuAction> m_vEditMenuActions;

	OptionRowHandlerEditMenuAction::OptionRowHandlerEditMenuAction() { Init(); }
	void Init()
	{
		OptionRowHandler::Init();
		m_vEditMenuActions.clear();
	}

	virtual void Load( OptionRowDefinition &defOut, CString sParam )
	{
		ASSERT( sParam.size() );

		Init();
		defOut.Init();

		m_sName = sParam;
		defOut.name = sParam;
		defOut.bOneChoiceForAllPlayers = true;
		defOut.layoutType = LAYOUT_SHOW_ONE_IN_ROW;
		m_vsReloadRowMessages.push_back( MessageToString(MESSAGE_CURRENT_STEPS_P1_CHANGED) );
		m_vsReloadRowMessages.push_back( MessageToString(MESSAGE_EDIT_SOURCE_STEPS_CHANGED) );

		bool bHasSteps = GAMESTATE->m_pCurSteps[0] != NULL;
		bool bHasSourceSteps = GAMESTATE->m_pEditSourceSteps != NULL;
		FOREACH_EditMenuAction( ema )
		{
			switch( ema )
			{
			case EDIT_MENU_ACTION_EDIT:
			case EDIT_MENU_ACTION_DELETE:
				if( !bHasSteps )
					continue;	// skip
				break;
			case EDIT_MENU_ACTION_COPY:
			case EDIT_MENU_ACTION_AUTOGEN:
				if( bHasSteps || !bHasSourceSteps )
					continue;	// skip
				break;
			case EDIT_MENU_ACTION_BLANK:
				if( bHasSteps )
					continue;	// skip
				break;
			default:
				ASSERT(0);
			}

			m_vEditMenuActions.push_back( ema );
			CString s = EditMenuActionToThemedString( ema );
			defOut.choices.push_back( s );
		}
	}
	virtual void ImportOption( const OptionRowDefinition &def, PlayerNumber pn, vector<bool> &vbSelectedOut ) const
	{
		vbSelectedOut[0] = true;
	}
	virtual int ExportOption( const OptionRowDefinition &def, PlayerNumber pn, const vector<bool> &vbSelected ) const
	{
		return 0;
	}
	virtual CString GetAndEraseScreen( int iChoice )
	{
		Song* pSong = GAMESTATE->m_pCurSong;
		Steps *pSteps = GAMESTATE->m_pCurSteps[0];
		Difficulty dc = GAMESTATE->m_PreferredDifficulty[0];
		StepsType st = GAMESTATE->m_stEdit;
		Steps *pSourceNotes = GAMESTATE->m_pEditSourceSteps;

		EditMenuAction ema = m_vEditMenuActions[iChoice];
		switch( ema )
		{
		case EDIT_MENU_ACTION_EDIT:
			// Prepare prepare for ScreenEdit
			ASSERT( pSong );
			ASSERT( pSteps );
			GAMESTATE->m_pCurStyle = GAMEMAN->GetEditorStyleForStepsType( st );
			return "ScreenEdit";
			break;
		case EDIT_MENU_ACTION_DELETE:
			ASSERT( pSteps );
			SCREENMAN->Prompt( SM_None, "These notes will be lost permanently.\n\nContinue with delete?", true, false, DeleteCurNotes );
			break;
		case EDIT_MENU_ACTION_COPY:
			ASSERT( !pSteps );
			ASSERT( pSourceNotes );
			{
				// Yuck.  Doing the memory allocation doesn't seem right since
				// Song allocates all of the other Steps.
				Steps* pNewSteps = new Steps;
				pNewSteps->CopyFrom( pSourceNotes, st );
				pNewSteps->SetDifficulty( dc );
				pSong->AddSteps( pNewSteps );
			
				SCREENMAN->SystemMessage( "Steps created from copy." );
				SOUND->PlayOnce( THEME->GetPathS("ScreenEditMenu","create") );
				pSong->Save();
			}
			break;
		case EDIT_MENU_ACTION_AUTOGEN:
			ASSERT( !pSteps );
			ASSERT( pSourceNotes );
			{
				// Yuck.  Doing the memory allocation doesn't seem right since
				// Song allocates all of the other Steps.
				Steps* pNewNotes = new Steps;
				pNewNotes->AutogenFrom( pSourceNotes, st );
				pNewNotes->DeAutogen();
				pNewNotes->SetDifficulty( dc );	// override difficulty with the user's choice
				pSong->AddSteps( pNewNotes );
			
				SCREENMAN->SystemMessage( "Steps created from AutoGen." );
				SOUND->PlayOnce( THEME->GetPathS("ScreenEditMenu","create") );
				pSong->Save();
			}
			break;
		case EDIT_MENU_ACTION_BLANK:
			ASSERT( !pSteps );
			{
				// Yuck.  Doing the memory allocation doesn't seem right since
				// Song allocates all of the other Steps.
				Steps* pNewNotes = new Steps;
				pNewNotes->CreateBlank( st );
				pNewNotes->SetDifficulty( dc );
				pNewNotes->SetMeter( 1 );
				pSong->AddSteps( pNewNotes );
			
				SCREENMAN->SystemMessage( "Blank Steps created." );
				SOUND->PlayOnce( THEME->GetPathS("ScreenEditMenu","create") );
				pSong->Save();
			}
			break;
		default:
			ASSERT(0);
		}
		
		// refresh the screen since we deleted or added steps
		MESSAGEMAN->Broadcast( MESSAGE_CURRENT_SONG_CHANGED );
		return "";
	}
};


///////////////////////////////////////////////////////////////////////////////////


OptionRowHandler* OptionRowHandlerUtil::Make( const Command &command, OptionRowDefinition &defOut )
{
	OptionRowHandler* pHand = NULL;

	BeginHandleArgs;

	const CString &name = command.GetName();

#define MAKE( type )	{ type *p = new type; p->Load( defOut, sArg(1) ); pHand = p; }

	if(		 name == "list" )			MAKE( OptionRowHandlerList )
	else if( name == "lua" )			MAKE( OptionRowHandlerLua )
	else if( name == "conf" )			MAKE( OptionRowHandlerConfig )
	else if( name == "stepstype" )		MAKE( OptionRowHandlerStepsType )
	else if( name == "steps" )			MAKE( OptionRowHandlerSteps )
	else if( name == "editmenuaction" )	MAKE( OptionRowHandlerEditMenuAction )

	EndHandleArgs;

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
