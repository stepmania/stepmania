#include "global.h"
#include "ModeChoice.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageDisplay.h"
#include "AnnouncerManager.h"
#include "ProfileManager.h"
#include "arch/ArchHooks/ArchHooks.h"

void ModeChoice::Init()
{
	m_iIndex = -1;
	m_game = GAME_INVALID;
	m_style = STYLE_INVALID;
	m_pm = PLAY_MODE_INVALID;
	m_dc = DIFFICULTY_INVALID;
	m_sAnnouncer = "";
	m_sName = "";
	m_bInvalid = true;
}

bool ModeChoice::DescribesCurrentMode() const
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
		CString sValue = (asBits.size()>1) ? asBits[1] : "";

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
		{
			m_sAnnouncer = sValue;
		}

	}
}

void ModeChoice::ApplyToAllPlayers()
{
	for( int pn=0; pn<NUM_PLAYERS; pn++ )
		if( GAMESTATE->IsHumanPlayer(pn) )
			Apply((PlayerNumber) pn);
}

void ModeChoice::Apply( PlayerNumber pn )
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
