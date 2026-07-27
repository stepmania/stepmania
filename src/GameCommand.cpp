#include "global.h"
#include "GameCommand.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameState.h"
#include "AnnouncerManager.h"
#include "Bookkeeper.h"
#include "PlayerOptions.h"
#include "ProfileManager.h"
#include "Profile.h"
#include "StepMania.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "Game.h"
#include "Style.h"
#include "GameSoundManager.h"
#include "PlayerState.h"
#include "SongManager.h"
#include "Song.h"
#include "UnlockManager.h"
#include "LocalizedString.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "ScreenPrompt.h"

#include <cstddef>
#include <vector>


static LocalizedString COULD_NOT_LAUNCH_BROWSER( "GameCommand", "Could not launch web browser." );

REGISTER_CLASS_TRAITS( GameCommand, new GameCommand(*pCopy) );

void GameCommand::Init()
{
	m_bApplyCommitsScreens = true;
	m_sName = "";
	m_sText = "";
	m_bInvalid = true;
	m_iIndex = -1;
	m_MultiPlayer = MultiPlayer_Invalid;
	m_pStyle = nullptr;
	m_pm = PlayMode_Invalid;
	m_dc = Difficulty_Invalid;
	m_CourseDifficulty = Difficulty_Invalid;
	m_sPreferredModifiers = "";
	m_sStageModifiers = "";
	m_sAnnouncer = "";
	m_sScreen = "";
	m_LuaFunction.Unset();
	m_pSong = nullptr;
	m_pSteps = nullptr;
	m_pCourse = nullptr;
	m_pTrail = nullptr;
	m_pCharacter = nullptr;
	m_SortOrder = SortOrder_Invalid;
	m_sSoundPath = "";
	m_vsScreensToPrepare.clear();
	m_iWeightPounds = -1;
	m_iGoalCalories = -1;
	m_GoalType = GoalType_Invalid;
	m_sProfileID = "";
	m_sUrl = "";
	m_bUrlExits = true;

	m_bInsertCredit = false;
	m_bClearCredits = false;
	m_bStopMusic = false;
	m_bApplyDefaultOptions = false;
	m_bFadeMusic = false;
	m_fMusicFadeOutVolume = -1.0f;
	m_fMusicFadeOutSeconds = -1.0f;
}

class SongOptions;
bool CompareSongOptions( const SongOptions &so1, const SongOptions &so2 );

bool GameCommand::DescribesCurrentModeForAllPlayers() const
{
	FOREACH_HumanPlayer( pn )
		if( !DescribesCurrentMode(pn) )
			return false;

	return true;
}

bool GameCommand::DescribesCurrentMode( PlayerNumber pn ) const
{
	if( m_pm != PlayMode_Invalid && GAMESTATE->m_PlayMode != m_pm )
		return false;
	if( m_pStyle && GAMESTATE->GetCurrentStyle(pn) != m_pStyle )
		return false;
	// HACK: don't compare m_dc if m_pSteps is set.  This causes problems
	// in ScreenSelectOptionsMaster::ImportOptions if m_PreferredDifficulty
	// doesn't match the difficulty of m_pCurSteps.
	if( m_pSteps == nullptr  &&  m_dc != Difficulty_Invalid )
	{
		// Why is this checking for all players?
		FOREACH_HumanPlayer( human )
			if( GAMESTATE->m_PreferredDifficulty[human] != m_dc )
				return false;
	}

	if( m_sAnnouncer != "" && m_sAnnouncer != ANNOUNCER->GetCurAnnouncerName() )
		return false;

	if( m_sPreferredModifiers != "" )
	{
		PlayerOptions po = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetPreferred();
		SongOptions so = GAMESTATE->m_SongOptions.GetPreferred();
		po.FromString( m_sPreferredModifiers );
		so.FromString( m_sPreferredModifiers );

		if( po != GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetPreferred() )
			return false;
		if( so != GAMESTATE->m_SongOptions.GetPreferred() )
			return false;
	}
	if( m_sStageModifiers != "" )
	{
		PlayerOptions po = GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetStage();
		SongOptions so = GAMESTATE->m_SongOptions.GetStage();
		po.FromString( m_sStageModifiers );
		so.FromString( m_sStageModifiers );

		if( po != GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetStage() )
			return false;
		if( so != GAMESTATE->m_SongOptions.GetStage() )
			return false;
	}

	if( m_pSong && GAMESTATE->m_pCurSong.Get() != m_pSong )
		return false;
	if( m_pSteps && GAMESTATE->m_pCurSteps[pn].Get() != m_pSteps )
		return false;
	if( m_pCharacter && GAMESTATE->m_pCurCharacters[pn] != m_pCharacter )
		return false;
	if( m_pCourse && GAMESTATE->m_pCurCourse.Get() != m_pCourse )
		return false;
	if( m_pTrail && GAMESTATE->m_pCurTrail[pn].Get() != m_pTrail )
		return false;
	if( !m_sSongGroup.empty() && GAMESTATE->m_sPreferredSongGroup != m_sSongGroup )
		return false;
	if( m_SortOrder != SortOrder_Invalid && GAMESTATE->m_PreferredSortOrder != m_SortOrder )
		return false;
	if( m_iWeightPounds != -1 && PROFILEMAN->GetProfile(pn)->m_iWeightPounds != m_iWeightPounds )
		return false;
	if( m_iGoalCalories != -1 && PROFILEMAN->GetProfile(pn)->m_iGoalCalories != m_iGoalCalories )
		return false;
	if( m_GoalType != GoalType_Invalid && PROFILEMAN->GetProfile(pn)->m_GoalType != m_GoalType )
		return false;
	if( !m_sProfileID.empty() && ProfileManager::m_sDefaultLocalProfileID[pn].Get() != m_sProfileID )
		return false;

	return true;
}

void GameCommand::Load( int iIndex, const Commands& cmds )
{
	m_iIndex = iIndex;
	m_bInvalid = false;
	m_Commands = cmds;

	for (Command const &cmd : cmds.v)
		LoadOne( cmd );
}

void GameCommand::LoadOne( const Command& cmd )
{
	RString sName = cmd.GetName();
	if( sName.empty() )
		return;

	RString sValue;
	for( unsigned i = 1; i < cmd.m_vsArgs.size(); ++i )
	{
		if( i > 1 )
			sValue += ",";
		sValue += cmd.m_vsArgs[i];
	}

#define MAKE_INVALID(expr) \
	m_sInvalidReason= (expr);																							\
	LuaHelpers::ReportScriptError(m_sInvalidReason, "INVALID_GAME_COMMAND"); \
	m_bInvalid= true;

#define CHECK_INVALID_COND(member, value, cond, message) \
	if(cond) \
	{ \
		MAKE_INVALID(message); \
	} \
	else \
	{ \
		member= value; \
	}

#define CHECK_INVALID_VALUE(member, value, invalid_value, value_name) \
	CHECK_INVALID_COND(member, value, (value == invalid_value), ssprintf("Invalid "#value_name" \"%s\".", sValue.c_str()));

	if( sName == "style" )
	{
		const Style* style = GAMEMAN->GameAndStringToStyle( GAMESTATE->m_pCurGame, sValue );
		CHECK_INVALID_VALUE(m_pStyle, style, nullptr, style);
	}

	else if( sName == "playmode" )
	{
		PlayMode pm = StringToPlayMode( sValue );
		CHECK_INVALID_VALUE(m_pm, pm, PlayMode_Invalid, playmode);
	}

	else if( sName == "difficulty" )
	{
		Difficulty dc = StringToDifficulty( sValue );
		CHECK_INVALID_VALUE(m_dc, dc, Difficulty_Invalid, difficulty);
	}

	else if( sName == "announcer" )
	{
		m_sAnnouncer = sValue;
	}

	else if( sName == "name" )
	{
		m_sName = sValue;
	}

	else if( sName == "text" )
	{
		m_sText = sValue;
	}

	else if( sName == "mod" )
	{
		if( m_sPreferredModifiers != "" )
			m_sPreferredModifiers += ",";
		m_sPreferredModifiers += sValue;
	}

	else if( sName == "stagemod" )
	{
		if( m_sStageModifiers != "" )
			m_sStageModifiers += ",";
		m_sStageModifiers += sValue;
	}

	else if( sName == "lua" )
	{
		m_LuaFunction.SetFromExpression( sValue );
		if(m_LuaFunction.IsNil())
		{
			MAKE_INVALID("Lua error in game command: \"" + sValue + "\" evaluated to nil");
		}
	}

	else if( sName == "screen" )
	{
		// OptionsList uses the screen command to push onto its stack.
		// OptionsList "screen"s are OptionRow entries in ScreenOptionsMaster.
		// So if the metric exists in ScreenOptionsMaster, consider it valid.
		// Additionally, the screen value can be used to create an OptionRow.
		// When used to create an OptionRow, it pulls a metric from OptionsList.
		// -Kyz

		if(!THEME->HasMetric("ScreenOptionsMaster", sValue))
		{
			if(!THEME->HasMetric("OptionsList", "Line" + sValue))
			{
				if(!SCREENMAN->IsScreenNameValid(sValue))
				{
					MAKE_INVALID("screen arg '" + sValue + "' is not a screen name, ScreenOptionsMaster list or OptionsList entry.");
				}
			}
		}
		if(!m_bInvalid)
		{
			m_sScreen= sValue;
		}
	}

	else if( sName == "song" )
	{
		CHECK_INVALID_COND(m_pSong, SONGMAN->FindSong(sValue),
			(SONGMAN->FindSong(sValue) == nullptr),
			(ssprintf("Song \"%s\" not found", sValue.c_str())));
	}

	else if( sName == "steps" )
	{
		RString sSteps = sValue;

		// This must be processed after "song" and "style" commands.
		if( !m_bInvalid )
		{
			Song *pSong = (m_pSong != nullptr)? m_pSong:GAMESTATE->m_pCurSong;
			const Style *pStyle = m_pStyle ? m_pStyle : GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber());
			if( pSong == nullptr || pStyle == nullptr )
			{
				MAKE_INVALID("Must set Song and Style to set Steps.");
			}
			else
			{
				Difficulty dc = StringToDifficulty( sSteps );
				Steps* st;
				if( dc < Difficulty_Edit )
				{
					st = SongUtil::GetStepsByDifficulty( pSong, pStyle->m_StepsType, dc );
				}
				else
				{
					st = SongUtil::GetStepsByDescription( pSong, pStyle->m_StepsType, sSteps );
				}
				CHECK_INVALID_COND(m_pSteps, st, (st == nullptr),
					(ssprintf("Steps \"%s\" not found", sSteps.c_str())));
			}
		}
	}

	else if( sName == "course" )
	{
		CHECK_INVALID_COND(m_pCourse, SONGMAN->FindCourse("", sValue),
			(SONGMAN->FindCourse("", sValue) == nullptr),
			(ssprintf( "Course \"%s\" not found", sValue.c_str())));
	}

	else if( sName == "trail" )
	{
		RString sTrail = sValue;

		// This must be processed after "course" and "style" commands.
		if( !m_bInvalid )
		{
			Course *pCourse = (m_pCourse != nullptr)? m_pCourse:GAMESTATE->m_pCurCourse;
			const Style *pStyle = m_pStyle ? m_pStyle : GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber());
			if( pCourse == nullptr || pStyle == nullptr )
			{
				MAKE_INVALID("Must set Course and Style to set Trail.");
			}
			else
			{
				const CourseDifficulty cd = StringToDifficulty( sTrail );
				if(cd == Difficulty_Invalid)
				{
					MAKE_INVALID(ssprintf("Invalid difficulty '%s'", sTrail.c_str()));
				}
				else
				{
					Trail* tr = pCourse->GetTrail(pStyle->m_StepsType, cd);
					CHECK_INVALID_COND(m_pTrail, tr, (tr == nullptr),
						("Trail \"" + sTrail + "\" not found."));
				}
			}
		}
	}

	else if( sName == "setenv" )
	{
		if((cmd.m_vsArgs.size() - 1) % 2 != 0)
		{
			MAKE_INVALID("Arguments to setenv game command must be key,value pairs.");
		}
		else
		{
			for(std::size_t i= 1; i < cmd.m_vsArgs.size(); i+= 2)
			{
				m_SetEnv[cmd.m_vsArgs[i]]= cmd.m_vsArgs[i+1];
			}
		}
	}

	else if( sName == "songgroup" )
	{
		CHECK_INVALID_COND(m_sSongGroup, sValue, (!SONGMAN->DoesSongGroupExist(sValue)), ("Song group \"" + sValue + "\" does not exist."));
	}

	else if( sName == "sort" )
	{
		SortOrder so= StringToSortOrder(sValue);
		CHECK_INVALID_VALUE(m_SortOrder, so, SortOrder_Invalid, sortorder);
	}

	else if( sName == "weight" )
	{
		m_iWeightPounds = StringToInt( sValue );
	}

	else if( sName == "goalcalories" )
	{
		m_iGoalCalories = StringToInt( sValue );
	}

	else if( sName == "goaltype" )
	{
		GoalType go= StringToGoalType(sValue);
		CHECK_INVALID_VALUE(m_GoalType, go, GoalType_Invalid, goaltype);
	}

	else if( sName == "profileid" )
	{
		m_sProfileID = sValue;
	}

	else if( sName == "url" )
	{
		m_sUrl = sValue;
		m_bUrlExits = true;
	}

	else if( sName == "sound" )
	{
		m_sSoundPath = sValue;
	}

	else if( sName == "preparescreen" )
	{
		m_vsScreensToPrepare.push_back( sValue );
	}

	else if( sName == "insertcredit" )
	{
		m_bInsertCredit = true;
	}

	else if( sName == "clearcredits" )
	{
		m_bClearCredits = true;
	}

	else if( sName == "stopmusic" )
	{
		m_bStopMusic = true;
	}

	else if( sName == "applydefaultoptions" )
	{
		m_bApplyDefaultOptions = true;
	}

	// sm-ssc additions begin:
	else if( sName == "urlnoexit" )
	{
		m_sUrl = sValue;
		m_bUrlExits = false;
	}

	else if( sName == "setpref" )
	{
		if((cmd.m_vsArgs.size() - 1) % 2 != 0)
		{
			MAKE_INVALID("Arguments to setpref game command must be key,value pairs.");
		}
		else
		{
			for(std::size_t i= 1; i < cmd.m_vsArgs.size(); i+= 2)
			{
				if(IPreference::GetPreferenceByName(cmd.m_vsArgs[i]) == nullptr)
				{
					MAKE_INVALID("Unknown preference \"" + cmd.m_vsArgs[i] + "\".");
				}
				else
				{
					m_SetPref[cmd.m_vsArgs[i]]= cmd.m_vsArgs[i+1];
				}
			}
		}
	}

	else if( sName == "fademusic" )
	{
		if( cmd.m_vsArgs.size() == 3 )
		{
			m_bFadeMusic = true;
			m_fMusicFadeOutVolume = static_cast<float>(atof( cmd.m_vsArgs[1] ));
			m_fMusicFadeOutSeconds = static_cast<float>(atof( cmd.m_vsArgs[2] ));
		}
		else
		{
			MAKE_INVALID("Wrong number of args to fademusic.");
		}
	}

	else
	{
		MAKE_INVALID(ssprintf( "Command '%s' is not valid.", cmd.GetOriginalCommandString().c_str()));
	}
#undef CHECK_INVALID_VALUE
#undef CHECK_INVALID_COND
#undef MAKE_INVALID
}

int GetNumCreditsPaid()
{
	int iNumCreditsPaid = GAMESTATE->GetNumSidesJoined();

	// players other than the first joined for free
	if( GAMESTATE->GetPremium() == Premium_2PlayersFor1Credit )
		iNumCreditsPaid = std::min( iNumCreditsPaid, 1 );

	return iNumCreditsPaid;
}


int GetCreditsRequiredToPlayStyle( const Style *style )
{
	// GameState::GetCoinsNeededToJoin returns 0 if the coin mode isn't
	// CoinMode_Pay, which means the theme can't make sure that there are
	// enough credits available.
	// So we have to check the coin mode here
	// and return 0 if the player doesn't have to pay.
	if( GAMESTATE->GetCoinMode() != CoinMode_Pay )
	{
		return 0;
	}
	if( GAMESTATE->GetPremium() == Premium_2PlayersFor1Credit )
		return 1;

	switch( style->m_StyleType )
	{
	case StyleType_OnePlayerOneSide:
		return 1;
	case StyleType_TwoPlayersSharedSides:
	case StyleType_TwoPlayersTwoSides:
		return 2;
	case StyleType_OnePlayerTwoSides:
		return (GAMESTATE->GetPremium() == Premium_DoubleFor1Credit) ? 1 : 2;
	DEFAULT_FAIL( style->m_StyleType );
	}
}

static bool AreStyleAndPlayModeCompatible( const Style *style, PlayMode pm )
{
	if( style == nullptr || pm == PlayMode_Invalid )
		return true;

	switch( pm )
	{
		case PLAY_MODE_BATTLE:
		case PLAY_MODE_RAVE:
			// Can't play rave if there isn't enough room for two players.
			// This is correct for dance (ie, no rave for solo and doubles),
			// and should be okay for pump.. not sure about other game types.
			// Techno Motion scales down versus arrows, though, so allow this.
			if( style->m_iColsPerPlayer >= 6 && RString(GAMESTATE->m_pCurGame->m_szName) != "techno" )
				return false;

			// Don't allow battle modes if the style takes both sides.
			if( style->m_StyleType==StyleType_OnePlayerTwoSides ||
				style->m_StyleType==StyleType_TwoPlayersSharedSides )
				return false;
		default: break;
	}

	return true;
}

bool GameCommand::IsPlayable( RString *why ) const
{
	if( m_bInvalid )
	{
		if( why )
			*why = m_sInvalidReason;
		return false;
	}

	if ( m_pStyle )
	{
		int iCredits;
		if( GAMESTATE->GetCoinMode() == CoinMode_Pay )
			iCredits = GAMESTATE->m_iCoins / PREFSMAN->m_iCoinsPerCredit;
		else
			iCredits = NUM_PLAYERS;

		const int iNumCreditsPaid = GetNumCreditsPaid();
		const int iNumCreditsRequired = GetCreditsRequiredToPlayStyle(m_pStyle);

		/* With PREFSMAN->m_bDelayedCreditsReconcile disabled, enough credits must
		 * be paid. (This means that enough sides must be joined.)  Enabled, simply
		 * having enough credits lying in the machine is sufficient; we'll deduct the
		 * extra in Apply(). */
		int iNumCreditsAvailable = iNumCreditsPaid;
		if( PREFSMAN->m_bDelayedCreditsReconcile )
			iNumCreditsAvailable += iCredits;

		if( iNumCreditsAvailable < iNumCreditsRequired )
		{
			if( why )
				*why = ssprintf( "need %i credits, have %i", iNumCreditsRequired, iNumCreditsAvailable );
			return false;
		}

		/* If both sides are joined, disallow singles modes, since easy to select
		 * them accidentally, instead of versus mode. */
		if( m_pStyle->m_StyleType == StyleType_OnePlayerOneSide &&
			GAMESTATE->GetNumPlayersEnabled() > 1 )
		{
			if( why )
				*why = "too many players joined for ONE_PLAYER_ONE_CREDIT";
			return false;
		}
	}

	/* Don't allow a PlayMode that's incompatible with our current Style (if set),
	 * and vice versa. */
	if( m_pm != PlayMode_Invalid || m_pStyle != nullptr )
	{
		const PlayMode pm = (m_pm != PlayMode_Invalid) ? m_pm : GAMESTATE->m_PlayMode;
		const Style *style = (m_pStyle != nullptr)? m_pStyle: GAMESTATE->GetCurrentStyle(GAMESTATE->GetMasterPlayerNumber());
		if( !AreStyleAndPlayModeCompatible( style, pm ) )
		{
			if( why )
				*why = ssprintf("mode %s is incompatible with style %s",
					PlayModeToString(pm).c_str(), style->m_szName );

			return false;
		}
	}

	if( !m_sScreen.CompareNoCase("ScreenEditCoursesMenu") )
	{
		std::vector<Course*> vCourses;
		SONGMAN->GetAllCourses( vCourses, false );

		if( vCourses.size() == 0 )
		{
			if( why )
				*why = "No courses are installed";
			return false;
		}
	}

	if( (!m_sScreen.CompareNoCase("ScreenJukeboxMenu") ||
		!m_sScreen.CompareNoCase("ScreenEditMenu") ||
		!m_sScreen.CompareNoCase("ScreenEditCoursesMenu")) )
	{
		if( SONGMAN->GetNumSongs() == 0 )
		{
			if( why )
				*why = "No songs are installed";
			return false;
		}
	}

	if( !m_sPreferredModifiers.empty() )
	{
		// TODO: Split this and check each modifier individually
		if( UNLOCKMAN->ModifierIsLocked(m_sPreferredModifiers) )
		{	if( why )
				*why = "Modifier is locked";
			return false;
		}
	}

	if( !m_sStageModifiers.empty() )
	{
		// TODO: Split this and check each modifier individually
		if( UNLOCKMAN->ModifierIsLocked(m_sStageModifiers) )
		{	if( why )
				*why = "Modifier is locked";
			return false;
		}
	}

	return true;
}

void GameCommand::ApplyToAllPlayers() const
{
	std::vector<PlayerNumber> vpns;

	FOREACH_PlayerNumber( pn )
		vpns.push_back( pn );

	Apply( vpns );
}

void GameCommand::Apply( PlayerNumber pn ) const
{
	std::vector<PlayerNumber> vpns;
	vpns.push_back( pn );
	Apply( vpns );
}

void GameCommand::Apply( const std::vector<PlayerNumber> &vpns ) const
{
	if( m_Commands.v.size() )
	{
		// We were filled using a GameCommand from metrics. Apply the options in order.
		for (Command const &cmd : m_Commands.v)
		{
			GameCommand gc;
			gc.m_bInvalid = false;
			gc.m_bApplyCommitsScreens = m_bApplyCommitsScreens;
			gc.LoadOne( cmd );
			gc.ApplySelf( vpns );
		}
	}
	else
	{
		// We were filled by an OptionRowHandler in code. m_Commands isn't filled,
		// so just apply the values that are already set in this.
		this->ApplySelf( vpns );
	}
}

void GameCommand::ApplySelf( const std::vector<PlayerNumber> &vpns ) const
{
	const PlayMode OldPlayMode = GAMESTATE->m_PlayMode;

	if( m_pm != PlayMode_Invalid )
		GAMESTATE->m_PlayMode.Set( m_pm );

	if( m_pStyle != nullptr )
	{
		GAMESTATE->SetCurrentStyle( m_pStyle, GAMESTATE->GetMasterPlayerNumber() );

		// It's possible to choose a style that didn't have enough players joined.
		// If enough players aren't joined, then  we need to subtract credits
		// for the sides that will be joined as a result of applying this option.
		if( GAMESTATE->GetCoinMode() == CoinMode_Pay )
		{
			int iNumCreditsRequired = GetCreditsRequiredToPlayStyle(m_pStyle);
			int iNumCreditsPaid = GetNumCreditsPaid();
			int iNumCreditsOwed = iNumCreditsRequired - iNumCreditsPaid;
			GAMESTATE->m_iCoins.Set( GAMESTATE->m_iCoins - iNumCreditsOwed * PREFSMAN->m_iCoinsPerCredit );
			LOG->Trace( "Deducted %i coins, %i remaining",
					iNumCreditsOwed * PREFSMAN->m_iCoinsPerCredit, GAMESTATE->m_iCoins.Get() );

            		//Credit Used, make sure to update CoinsFile
            		BOOKKEEPER->WriteCoinsFile(GAMESTATE->m_iCoins.Get());
		}

		// If only one side is joined and we picked a style that requires both
		// sides, join the other side.
		switch( m_pStyle->m_StyleType )
		{
		case StyleType_OnePlayerOneSide:
		case StyleType_OnePlayerTwoSides:
			break;
		case StyleType_TwoPlayersTwoSides:
		case StyleType_TwoPlayersSharedSides:
			{
				FOREACH_PlayerNumber( p )
					GAMESTATE->JoinPlayer( p );
			}
			break;
		default:
			LuaHelpers::ReportScriptErrorFmt("Invalid StyleType: %d", m_pStyle->m_StyleType);
		}
	}
	if( m_dc != Difficulty_Invalid )
		for (PlayerNumber const &pn : vpns)
			GAMESTATE->m_PreferredDifficulty[pn].Set( m_dc );
	if( m_sAnnouncer != "" )
		ANNOUNCER->SwitchAnnouncer( m_sAnnouncer );
	if( m_sPreferredModifiers != "" )
		for (PlayerNumber const &pn : vpns)
			GAMESTATE->ApplyPreferredModifiers( pn, m_sPreferredModifiers );
	if( m_sStageModifiers != "" )
		for (PlayerNumber const &pn : vpns)
			GAMESTATE->ApplyStageModifiers( pn, m_sStageModifiers );
	if( m_LuaFunction.IsSet() && !m_LuaFunction.IsNil() )
	{
		Lua *L = LUA->Get();
		for (PlayerNumber const &pn : vpns)
		{
			m_LuaFunction.PushSelf( L );
			ASSERT( !lua_isnil(L, -1) );

			lua_pushnumber( L, pn ); // 1st parameter
			RString error= "Lua GameCommand error: ";
			LuaHelpers::RunScriptOnStack(L, error, 1, 0, true);
		}
		LUA->Release(L);
	}
	if( m_sScreen != "" && m_bApplyCommitsScreens )
		SCREENMAN->SetNewScreen( m_sScreen );
	if( m_pSong )
	{
		GAMESTATE->m_pCurSong.Set( m_pSong );
		GAMESTATE->m_pPreferredSong = m_pSong;
	}
	if( m_pSteps )
		for (PlayerNumber const &pn : vpns)
			GAMESTATE->m_pCurSteps[pn].Set( m_pSteps );
	if( m_pCourse )
	{
		GAMESTATE->m_pCurCourse.Set( m_pCourse );
		GAMESTATE->m_pPreferredCourse = m_pCourse;
	}
	if( m_pTrail )
		for (PlayerNumber const &pn : vpns)
			GAMESTATE->m_pCurTrail[pn].Set( m_pTrail );
	if( m_CourseDifficulty != Difficulty_Invalid )
		for (PlayerNumber const &pn : vpns)
			GAMESTATE->ChangePreferredCourseDifficulty( pn, m_CourseDifficulty );
	if( m_pCharacter )
		for (PlayerNumber const &pn : vpns)
			GAMESTATE->m_pCurCharacters[pn] = m_pCharacter;
	for( std::map<RString, RString>::const_iterator i = m_SetEnv.begin(); i != m_SetEnv.end(); i++ )
	{
		Lua *L = LUA->Get();
		GAMESTATE->m_Environment->PushSelf(L);
		lua_pushstring( L, i->first );
		lua_pushstring( L, i->second );
		lua_settable( L, -3 );
		lua_pop( L, 1 );
		LUA->Release(L);
	}
	for(std::map<RString, RString>::const_iterator setting= m_SetPref.begin(); setting != m_SetPref.end(); ++setting)
	{
		IPreference* pref= IPreference::GetPreferenceByName(setting->first);
		if(pref != nullptr)
		{
			pref->FromString(setting->second);
		}
	}
	if( !m_sSongGroup.empty() )
		GAMESTATE->m_sPreferredSongGroup.Set( m_sSongGroup );
	if( m_SortOrder != SortOrder_Invalid )
		GAMESTATE->m_PreferredSortOrder = m_SortOrder;
	if( m_sSoundPath != "" )
		SOUND->PlayOnce( THEME->GetPathS( "", m_sSoundPath ) );
	if( m_iWeightPounds != -1 )
		for (PlayerNumber const &pn : vpns)
			PROFILEMAN->GetProfile(pn)->m_iWeightPounds = m_iWeightPounds;
	if( m_iGoalCalories != -1 )
		for (PlayerNumber const &pn : vpns)
			PROFILEMAN->GetProfile(pn)->m_iGoalCalories = m_iGoalCalories;
	if( m_GoalType != GoalType_Invalid )
		for (PlayerNumber const &pn : vpns)
			PROFILEMAN->GetProfile(pn)->m_GoalType = m_GoalType;
	if( !m_sProfileID.empty() )
		for (PlayerNumber const &pn : vpns)
			ProfileManager::m_sDefaultLocalProfileID[pn].Set( m_sProfileID );
	if( !m_sUrl.empty() )
	{
		if( HOOKS->GoToURL( m_sUrl ) )
		{
			if( m_bUrlExits )
				SCREENMAN->SetNewScreen( "ScreenExit" );
		}
		else
			ScreenPrompt::Prompt( SM_None, COULD_NOT_LAUNCH_BROWSER );
	}

	/* If we're going to stop music, do so before preparing new screens, so we
	 * don't stop music between preparing screens and loading screens. */
	if( m_bStopMusic )
		SOUND->StopMusic();
	if( m_bFadeMusic )
		SOUND->DimMusic(m_fMusicFadeOutVolume, m_fMusicFadeOutSeconds);

	for (RString const &s : m_vsScreensToPrepare)
		SCREENMAN->PrepareScreen( s );

	if( m_bInsertCredit )
	{
		StepMania::InsertCredit();
	}

	if( m_bClearCredits )
		StepMania::ClearCredits();

	if( m_bApplyDefaultOptions )
	{
		// applying options affects only the current stage
		FOREACH_PlayerNumber( p )
		{
			PlayerOptions po;
			GAMESTATE->GetDefaultPlayerOptions( po );
			GAMESTATE->m_pPlayerState[p]->m_PlayerOptions.Assign( ModsLevel_Stage, po );
		}

		SongOptions so;
		GAMESTATE->GetDefaultSongOptions( so );
		GAMESTATE->m_SongOptions.Assign( ModsLevel_Stage, so );
	}
	// HACK: Set life type to BATTERY just once here so it happens once and
	// we don't override the user's changes if they back out.
	FOREACH_PlayerNumber(pn)
	{
		if(GAMESTATE->m_PlayMode == PLAY_MODE_ONI &&
			GAMESTATE->m_PlayMode != OldPlayMode &&
			GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.GetStage().m_LifeType ==LifeType_Bar)
		{
			PO_GROUP_ASSIGN(GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions, ModsLevel_Stage, m_LifeType, LifeType_Battery);
		}
	}
}

bool GameCommand::IsZero() const
{
	if( 	m_pm != PlayMode_Invalid ||
		m_pStyle != nullptr ||
		m_dc != Difficulty_Invalid ||
		m_sAnnouncer != "" ||
		m_sPreferredModifiers != "" ||
		m_sStageModifiers != "" ||
		m_pSong != nullptr ||
		m_pSteps != nullptr ||
		m_pCourse != nullptr ||
		m_pTrail != nullptr ||
		m_pCharacter != nullptr ||
		m_CourseDifficulty != Difficulty_Invalid ||
		!m_sSongGroup.empty() ||
		m_SortOrder != SortOrder_Invalid ||
		m_iWeightPounds != -1 ||
		m_iGoalCalories != -1 ||
		m_GoalType != GoalType_Invalid ||
		!m_sProfileID.empty() ||
		!m_sUrl.empty() )
		return false;

	return true;
}

// lua start
#include "LuaBinding.h"
#include "Game.h"
#include "Steps.h"
#include "Character.h"

/** @brief Allow Lua to have access to the GameCommand. */
class LunaGameCommand: public Luna<GameCommand>
{
public:
	static int GetName( T* p, lua_State *L )	{ lua_pushstring(L, p->m_sName ); return 1; }
	static int GetText( T* p, lua_State *L )	{ lua_pushstring(L, p->m_sText ); return 1; }
	static int GetIndex( T* p, lua_State *L )	{ lua_pushnumber(L, p->m_iIndex ); return 1; }
	static int GetMultiPlayer( T* p, lua_State *L )	{ lua_pushnumber(L, p->m_MultiPlayer); return 1; }
	static int GetStyle( T* p, lua_State *L )	{ if(p->m_pStyle== nullptr) lua_pushnil(L); else {Style *pStyle = (Style*)p->m_pStyle; pStyle->PushSelf(L);} return 1; }
	static int GetScreen( T* p, lua_State *L )	{ lua_pushstring(L, p->m_sScreen ); return 1; }
	static int GetProfileID( T* p, lua_State *L )	{ lua_pushstring(L, p->m_sProfileID ); return 1; }
	static int GetSong( T* p, lua_State *L )	{ if(p->m_pSong== nullptr) lua_pushnil(L); else p->m_pSong->PushSelf(L); return 1; }
	static int GetSteps( T* p, lua_State *L )	{ if(p->m_pSteps== nullptr) lua_pushnil(L); else p->m_pSteps->PushSelf(L); return 1; }
	static int GetCourse( T* p, lua_State *L )	{ if(p->m_pCourse== nullptr) lua_pushnil(L); else p->m_pCourse->PushSelf(L); return 1; }
	static int GetTrail( T* p, lua_State *L )	{ if(p->m_pTrail== nullptr) lua_pushnil(L); else p->m_pTrail->PushSelf(L); return 1; }
	static int GetCharacter( T* p, lua_State *L )	{ if(p->m_pCharacter== nullptr) lua_pushnil(L); else p->m_pCharacter->PushSelf(L); return 1; }
	static int GetSongGroup( T* p, lua_State *L )	{ lua_pushstring(L, p->m_sSongGroup ); return 1; }
	static int GetUrl( T* p, lua_State *L )	{ lua_pushstring(L, p->m_sUrl ); return 1; }
	static int GetAnnouncer( T* p, lua_State *L )	{ lua_pushstring(L, p->m_sAnnouncer ); return 1; }
	static int GetPreferredModifiers( T* p, lua_State *L )	{ lua_pushstring(L, p->m_sPreferredModifiers ); return 1; }
	static int GetStageModifiers( T* p, lua_State *L )	{ lua_pushstring(L, p->m_sStageModifiers ); return 1; }

	DEFINE_METHOD( GetCourseDifficulty,	m_CourseDifficulty )
	DEFINE_METHOD( GetDifficulty,	m_dc )
	DEFINE_METHOD( GetPlayMode,		m_pm )
	DEFINE_METHOD( GetSortOrder,	m_SortOrder )

	LunaGameCommand()
	{
		ADD_METHOD( GetName );
		ADD_METHOD( GetText );
		ADD_METHOD( GetIndex );
		ADD_METHOD( GetMultiPlayer );
		ADD_METHOD( GetStyle );
		ADD_METHOD( GetDifficulty );
		ADD_METHOD( GetCourseDifficulty );
		ADD_METHOD( GetScreen );
		ADD_METHOD( GetPlayMode );
		ADD_METHOD( GetProfileID );
		ADD_METHOD( GetSong );
		ADD_METHOD( GetSteps );
		ADD_METHOD( GetCourse );
		ADD_METHOD( GetTrail );
		ADD_METHOD( GetCharacter );
		ADD_METHOD( GetSongGroup );
		ADD_METHOD( GetSortOrder );
		ADD_METHOD( GetUrl );
		ADD_METHOD( GetAnnouncer );
		ADD_METHOD( GetPreferredModifiers );
		ADD_METHOD( GetStageModifiers );
	}
};

LUA_REGISTER_CLASS( GameCommand )
// lua end

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
