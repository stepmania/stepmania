#include "global.h"
#include "ModeChoice.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageDisplay.h"
#include "AnnouncerManager.h"
#include "ProfileManager.h"
#include "StepMania.h"
#include "ScreenManager.h"
#include "SongManager.h"
#include "arch/ArchHooks/ArchHooks.h"

void ModeChoice::Init()
{
	m_iIndex = -1;
	m_game = GAME_INVALID;
	m_style = STYLE_INVALID;
	m_pm = PLAY_MODE_INVALID;
	m_dc = DIFFICULTY_INVALID;
	m_sModifiers = "";
	m_sAnnouncer = "";
	m_sName = "";
	m_sScreen = "";
	m_bInvalid = true;
}

bool ComparePlayerOptions( const PlayerOptions &po1, const PlayerOptions &po2 );
bool CompareSongOptions( const SongOptions &so1, const SongOptions &so2 );

bool ModeChoice::DescribesCurrentModeForAllPlayers() const
{
	for( int pn=0; pn<NUM_PLAYERS; pn++ )
		if( !DescribesCurrentMode( (PlayerNumber) pn) )
			return false;

	return true;
}

bool ModeChoice::DescribesCurrentMode( PlayerNumber pn ) const
{
	if( m_game != GAME_INVALID && m_game != GAMESTATE->m_CurGame )
		return false;
	if( m_pm != PLAY_MODE_INVALID && GAMESTATE->m_PlayMode != m_pm )
		return false;
	if( m_style != STYLE_INVALID && GAMESTATE->m_CurStyle != m_style )
		return false;
	if( m_dc != DIFFICULTY_INVALID )
	{
		for( int pn=0; pn<NUM_PLAYERS; pn++ )
			if( GAMESTATE->IsHumanPlayer(pn) && GAMESTATE->m_PreferredDifficulty[pn] != m_dc )
				return false;
	}
		
	if( m_sAnnouncer != "" && m_sAnnouncer != ANNOUNCER->GetCurAnnouncerName() )
		return false;

	if( m_sModifiers != "" )
	{
		/* Save the current modifiers. */
		const PlayerOptions po = GAMESTATE->m_PlayerOptions[pn];
		const SongOptions so = GAMESTATE->m_SongOptions;

		/* Apply modifiers. */
		GAMESTATE->ApplyModifiers( pn, m_sModifiers );

		/* Did anything change? */
		bool Changed = false;
		if( !ComparePlayerOptions(po, GAMESTATE->m_PlayerOptions[pn]) )
			Changed = true;
		if( !CompareSongOptions(so, GAMESTATE->m_SongOptions) )
			Changed = true;

		/* Restore modifiers. */
		GAMESTATE->m_PlayerOptions[pn] = po;
		GAMESTATE->m_SongOptions = so;

		if( Changed )
			return false;
	}

	return true;
}

void ModeChoice::Load( int iIndex, CString sChoice )
{
	m_iIndex = iIndex;

	m_sName = sChoice;
	m_bInvalid = false;

	CStringArray asCommands;
	split( sChoice, ";", asCommands );
	for( unsigned i=0; i<asCommands.size(); i++ )
	{
		CString sCommand = asCommands[i];

		CStringArray asBits;
		split( sCommand, ",", asBits );
		
		CString sName = asBits[0];
		asBits.erase(asBits.begin(), asBits.begin()+1);
		CString sValue = join( ",", asBits );

		sName.MakeLower();
		sValue.MakeLower();

		if( sName == "game" )
		{
			Game game = GAMEMAN->StringToGameType( sValue );
			if( game != GAME_INVALID )
				m_game = game;
			else
				m_bInvalid |= true;
		}


		if( sName == "style" )
		{
			Style style = GAMEMAN->GameAndStringToStyle( GAMESTATE->m_CurGame, sValue );
			if( style != STYLE_INVALID )
			{
				m_style = style;
				// There is a choices that allows players to choose a style.  Allow joining.
				GAMESTATE->m_bPlayersCanJoin = true;
			}
			else
				m_bInvalid |= true;
		}

		if( sName == "playmode" )
		{
			PlayMode pm = StringToPlayMode( sValue );
			if( pm != PLAY_MODE_INVALID )
				m_pm = pm;
			else
				m_bInvalid |= true;
		}

		if( sName == "difficulty" )
		{
			Difficulty dc = StringToDifficulty( sValue );
			if( dc != DIFFICULTY_INVALID )
				m_dc = dc;
			else
				m_bInvalid |= true;
		}

		if( sName == "announcer" )
			m_sAnnouncer = sValue;

		if( sName == "name" )
			m_sName = sValue;

		if( sName == "mod" )
		{
			if( m_sModifiers != "" )
				m_sModifiers += ",";
			m_sModifiers += sValue;
		}
		
		if( sName == "screen" )
			m_sScreen = sValue;
	}
}

bool ModeChoice::IsPlayable( CString *why ) const
{
	if( m_bInvalid )
		return false;

	if ( m_style != STYLE_INVALID )
	{
		const int SidesJoinedToPlay = GAMEMAN->GetStyleDefForStyle(m_style)->NumSidesJoinedToPlay();
		if( SidesJoinedToPlay != GAMESTATE->GetNumSidesJoined() )
			return false;
	}

	/* Don't allow a PlayMode that's incompatible with our current Style (if set),
	 * and vice versa. */
	const PlayMode &rPlayMode = (m_pm != PLAY_MODE_INVALID) ? m_pm : GAMESTATE->m_PlayMode;
	if( rPlayMode == PLAY_MODE_RAVE || rPlayMode == PLAY_MODE_BATTLE )
	{
		// Can't play rave if there isn't enough room for two players.
		// This is correct for dance (ie, no rave for solo and doubles),
		// and should be okay for pump .. not sure about other game types.
		const Style &rStyle = m_style != STYLE_INVALID? m_style: GAMESTATE->m_CurStyle;
		if( rStyle != STYLE_INVALID &&
			GAMEMAN->GetStyleDefForStyle(rStyle)->m_iColsPerPlayer >= 6 )
			return false;
	}

	if( !m_sScreen.CompareNoCase("ScreenEditCoursesMenu") )
	{
		vector<Course*> vCourses;
		SONGMAN->GetAllCourses( vCourses, false );

		if( vCourses.size() == 0 )
		{
			if( why )
				*why = "No courses are installed";
			return false;
		}
	}

	if( !m_sScreen.CompareNoCase("ScreenJukeboxMenu") ||
		!m_sScreen.CompareNoCase("ScreenEditMenu") ||
		!m_sScreen.CompareNoCase("ScreenEditCoursesMenu") )
	{
		if( SONGMAN->GetNumSongs() == 0 )
		{
			if( why )
				*why = "No songs are installed";
			return false;
		}
	}

	return true;
}

void ModeChoice::ApplyToAllPlayers() const
{
	for( int pn=0; pn<NUM_PLAYERS; pn++ )
		if( GAMESTATE->IsHumanPlayer(pn) )
			Apply((PlayerNumber) pn);

	if( m_sScreen != "" )
		SCREENMAN->SetNewScreen( m_sScreen );
}

void ModeChoice::Apply( PlayerNumber pn ) const
{
	if( m_game != GAME_INVALID )
		GAMESTATE->m_CurGame = m_game;
	if( m_pm != PLAY_MODE_INVALID )
		GAMESTATE->m_PlayMode = m_pm;
	if( m_style != STYLE_INVALID )
		GAMESTATE->m_CurStyle = m_style;
	if( m_dc != DIFFICULTY_INVALID  &&  pn != PLAYER_INVALID )
		GAMESTATE->m_PreferredDifficulty[pn] = m_dc;
	if( m_sAnnouncer != "" )
		ANNOUNCER->SwitchAnnouncer( m_sAnnouncer );
	if( m_sModifiers != "" )
		GAMESTATE->ApplyModifiers( pn, m_sModifiers );

	// HACK:  Set life type to BATTERY just once here so it happens once and 
	// we don't override the user's changes if they back out.
	if( GAMESTATE->m_PlayMode == PLAY_MODE_ONI )
		GAMESTATE->m_SongOptions.m_LifeType = SongOptions::LIFE_BATTERY;


	//
	// We know what players are joined at the time we set the Style
	//
	if( m_style != STYLE_INVALID )
	{
		PROFILEMAN->TryLoadProfile( pn );
	}
}
