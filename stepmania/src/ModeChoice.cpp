#include "global.h"
#include "ModeChoice.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageDisplay.h"
#include "AnnouncerManager.h"
#include "arch/ArchHooks/ArchHooks.h"

void ModeChoice::Init()
{
	game = GAME_INVALID;
	style = STYLE_INVALID;
	pm = PLAY_MODE_INVALID;
	dc = DIFFICULTY_INVALID;
	sAnnouncer = "";
	strcpy( name, "" );
	numSidesJoinedToPlay = 1;
}

bool ModeChoice::FromString( CString sChoice )
{
	strncpy( this->name, sChoice, min(sChoice.size()+1, sizeof(this->name)) );
	sChoice[sizeof(this->name)-1] = 0;

	bool bChoiceIsInvalid = false;

	CStringArray asBits;
	split( sChoice, "-", asBits );
	for( unsigned b=0; b<asBits.size(); b++ )
	{
		CString sBit = asBits[b];

		Game game = GAMEMAN->StringToGameType( sBit );
		if( game != GAME_INVALID )
		{
			this->game = game;
			continue;
		}

		Style style = GAMEMAN->GameAndStringToStyle( GAMESTATE->m_CurGame, sBit );
		if( style != STYLE_INVALID )
		{
			this->style = style;
			// There is a choices that allows players to choose a style.  Allow joining.
			GAMESTATE->m_bPlayersCanJoin = true;
			continue;
		}

		PlayMode pm = StringToPlayMode( sBit );
		if( pm != PLAY_MODE_INVALID )
		{
			this->pm = pm;
			continue;
		}

		Difficulty dc = StringToDifficulty( sBit );
		if( dc != DIFFICULTY_INVALID )
		{
			this->dc = dc;
			continue;
		}

		CString sError = ssprintf( "The choice token '%s' is not recognized as a Game, Style, PlayMode, or Difficulty.  The choice containing this token will be ignored.", sBit.c_str() );
		LOG->Warn( sError );
		if( DISPLAY->IsWindowed() )
			HOOKS->MessageBoxOK( sError );
		bChoiceIsInvalid |= true;
	}

	if( this->style != STYLE_INVALID )
	{
		const StyleDef* pStyleDef = GAMEMAN->GetStyleDefForStyle(this->style);
		switch( pStyleDef->m_StyleType )
		{
		case StyleDef::ONE_PLAYER_ONE_CREDIT:
			this->numSidesJoinedToPlay = 1;
			break;
		case StyleDef::TWO_PLAYERS_TWO_CREDITS:
		case StyleDef::ONE_PLAYER_TWO_CREDITS:
			this->numSidesJoinedToPlay = 2;
			break;
		default:
			ASSERT(0);
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
}
