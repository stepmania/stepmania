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
	game = GAME_INVALID;
	style = STYLE_INVALID;
	pm = PLAY_MODE_INVALID;
	dc = DIFFICULTY_INVALID;
	sAnnouncer = "";
	strcpy( name, "" );
}

bool ModeChoice::DescribesCurrentMode() const
{
	if( game != GAME_INVALID && game != GAMESTATE->m_CurGame )
		return false;
	if( game != GAME_INVALID && GAMESTATE->m_CurGame != game )
		return false;
	if( pm != PLAY_MODE_INVALID && GAMESTATE->m_PlayMode != pm )
		return false;
	if( style != STYLE_INVALID && GAMESTATE->m_CurStyle != style )
		return false;
	if( dc != DIFFICULTY_INVALID )
	{
		for( int pn=0; pn<NUM_PLAYERS; pn++ )
			if( GAMESTATE->IsHumanPlayer(pn) && GAMESTATE->m_PreferredDifficulty[pn] != dc )
				return false;
	}
		
	if( sAnnouncer != "" && sAnnouncer != ANNOUNCER->GetCurAnnouncerName() )
		return false;
	
	return true;
}

bool ModeChoice::FromString( CString sChoice, bool bIgnoreUnknown )
{
	strncpy( this->name, sChoice, min(sChoice.size()+1, sizeof(this->name)) );
	name[sizeof(this->name)-1] = 0;

	bool bChoiceIsInvalid = false;

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
				this->game = game;
			else
				bChoiceIsInvalid |= true;
		}


		if( sName == "style" )
		{
			Style style = GAMEMAN->GameAndStringToStyle( GAMESTATE->m_CurGame, sValue );
			if( style != STYLE_INVALID )
			{
				this->style = style;
				// There is a choices that allows players to choose a style.  Allow joining.
				GAMESTATE->m_bPlayersCanJoin = true;
			}
			else
				bChoiceIsInvalid |= true;
		}

		if( sName == "playmode" )
		{
			PlayMode pm = StringToPlayMode( sValue );
			if( pm != PLAY_MODE_INVALID )
				this->pm = pm;
			else
				bChoiceIsInvalid |= true;
		}

		if( sName == "difficulty" )
		{
			Difficulty dc = StringToDifficulty( sValue );
			if( dc != DIFFICULTY_INVALID )
				this->dc = dc;
			else
				bChoiceIsInvalid |= true;
		}

		if( sName == "announcer" )
		{
			sAnnouncer = sValue;
		}

	}

	return !bChoiceIsInvalid;
}

void ModeChoice::ApplyToAllPlayers()
{
	for( int pn=0; pn<NUM_PLAYERS; pn++ )
		if( GAMESTATE->IsHumanPlayer(pn) )
			Apply((PlayerNumber) pn);
}

void ModeChoice::Apply( PlayerNumber pn )
{
	if( game != GAME_INVALID )
		GAMESTATE->m_CurGame = game;
	if( pm != PLAY_MODE_INVALID )
		GAMESTATE->m_PlayMode = pm;
	if( style != STYLE_INVALID )
		GAMESTATE->m_CurStyle = style;
	if( dc != DIFFICULTY_INVALID  &&  pn != PLAYER_INVALID )
		GAMESTATE->m_PreferredDifficulty[pn] = dc;
	if( sAnnouncer != "" )
		ANNOUNCER->SwitchAnnouncer( sAnnouncer );

	// HACK:  Set life type to BATTERY just once here so it happens once and 
	// we don't override the user's changes if they back out.
	if( GAMESTATE->m_PlayMode == PLAY_MODE_ONI )
		GAMESTATE->m_SongOptions.m_LifeType = SongOptions::LIFE_BATTERY;


	//
	// We know what players are joined at the time we set the Style
	//
	if( style != STYLE_INVALID )
	{
		PROFILEMAN->TryLoadProfile( pn );
	}
}
