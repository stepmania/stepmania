#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenNetEvaluation.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "RageLog.h"
#include "Style.h"
#include "SongUtil.h"

static const int NUM_SCORE_DIGITS = 9;

#define USERSBG_WIDTH		THEME->GetMetricF("ScreenNetEvaluation","UsersBGWidth")
#define USERSBG_HEIGHT		THEME->GetMetricF("ScreenNetEvaluation","UsersBGHeight")
#define USERSBG_COMMAND		THEME->GetMetricA("ScreenNetEvaluation","UsersBGCommand")

#define USERDX			THEME->GetMetricF("ScreenNetEvaluation","UserDX")
#define USERDY			THEME->GetMetricF("ScreenNetEvaluation","UserDY")

#define MAX_COMBO_NUM_DIGITS	THEME->GetMetricI("ScreenEvaluation","MaxComboNumDigits")

static AutoScreenMessage( SM_GotEval );

REGISTER_SCREEN_CLASS( ScreenNetEvaluation );

void ScreenNetEvaluation::Init()
{
	ScreenEvaluation::Init();

	m_bHasStats = false;
	m_iCurrentPlayer = 0;

	FOREACH_EnabledPlayer( pn )
	{
		m_pActivePlayer = pn;
	}

	m_iShowSide = (m_pActivePlayer == PLAYER_1) ? 2 : 1;

	m_rectUsersBG.SetWidth( USERSBG_WIDTH );
	m_rectUsersBG.SetHeight( USERSBG_HEIGHT );
	m_rectUsersBG.RunCommands( USERSBG_COMMAND );
	// XXX: The name should be set with m_iShowSide and then
	// LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND should be used. -aj
	m_rectUsersBG.SetName( "UsersBG" );

	m_rectUsersBG.SetXY(
		THEME->GetMetricF("ScreenNetEvaluation",ssprintf("UsersBG%dX",m_iShowSide)),
		THEME->GetMetricF("ScreenNetEvaluation",ssprintf("UsersBG%dY",m_iShowSide)) );
	LOAD_ALL_COMMANDS_AND_ON_COMMAND( m_rectUsersBG );

	this->AddChild( &m_rectUsersBG );

	RedoUserTexts();

	NSMAN->ReportNSSOnOff( 5 );
}

void ScreenNetEvaluation::RedoUserTexts()
{
	m_iActivePlayers = NSMAN->m_ActivePlayers;

	// If unnecessary, just don't do this function.
	if ( m_iActivePlayers == (int)m_textUsers.size() )
		return;

	for( unsigned int i=0; i < m_textUsers.size(); ++i )
		this->RemoveChild( &m_textUsers[i] );

	float cx = THEME->GetMetricF("ScreenNetEvaluation",ssprintf("User%dX",m_iShowSide));
	float cy = THEME->GetMetricF("ScreenNetEvaluation",ssprintf("User%dY",m_iShowSide));

	m_iCurrentPlayer = 0;
	m_textUsers.clear();
	m_textUsers.resize(m_iActivePlayers);

	for( int i=0; i<m_iActivePlayers; ++i )
	{
		m_textUsers[i].LoadFromFont( THEME->GetPathF(m_sName,"names") );
		m_textUsers[i].SetName( ssprintf("User") );
		m_textUsers[i].SetShadowLength( 1 );
		m_textUsers[i].SetXY( cx, cy );

		this->AddChild( &m_textUsers[i] );
		ActorUtil::LoadAllCommands( m_textUsers[i], m_sName );
		cx+=USERDX;
		cy+=USERDY;
	}
}

bool ScreenNetEvaluation::MenuLeft( const InputEventPlus &input )
{
	return MenuUp( input );
}

bool ScreenNetEvaluation::MenuUp( const InputEventPlus &input )
{
	if( m_iActivePlayers == 0 || !m_bHasStats )
		return false;

	COMMAND( m_textUsers[m_iCurrentPlayer], "DeSel" );
	m_iCurrentPlayer = (m_iCurrentPlayer + m_iActivePlayers - 1) % m_iActivePlayers;
	COMMAND( m_textUsers[m_iCurrentPlayer], "Sel" );
	UpdateStats();
	return true;
}

bool ScreenNetEvaluation::MenuRight( const InputEventPlus &input )
{
	return MenuDown( input );
}

bool ScreenNetEvaluation::MenuDown( const InputEventPlus &input )
{
	if ( m_iActivePlayers == 0 || !m_bHasStats )
		return false;

	COMMAND( m_textUsers[m_iCurrentPlayer], "DeSel" );
	m_iCurrentPlayer = (m_iCurrentPlayer + 1) % m_iActivePlayers;
	COMMAND( m_textUsers[m_iCurrentPlayer], "Sel" );
	UpdateStats();
	return true;
}

void ScreenNetEvaluation::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GotEval)
	{
		m_bHasStats = true;

		LOG->Trace( "[SMNETDebug] num active players: %d (local), %d (NSMAN)",m_iActivePlayers,NSMAN->m_ActivePlayers );

		RedoUserTexts();

		LOG->Trace( "SMNETCheckpoint" );
		for( int i=0; i<m_iActivePlayers; ++i )
		{
			// Strange occurences because of timing cause these things not to
			// work right and will sometimes cause a crash. We should make SURE
			// we won't crash!
			if ( size_t(i) >= NSMAN->m_EvalPlayerData.size() )
				break;

			if ( size_t(NSMAN->m_EvalPlayerData[i].name) >= NSMAN->m_PlayerNames.size() )
				break;

			if ( NSMAN->m_EvalPlayerData[i].name < 0 )
				break;

			if ( size_t(i) >= m_textUsers.size() )
				break;

			m_textUsers[i].SetText( NSMAN->m_PlayerNames[NSMAN->m_EvalPlayerData[i].name] );

			// Yes, hardcoded (I'd like to leave it that way) -CNLohr (in reference to Grade_Tier03)
			// Themes can read this differently. The correct solution depends...
			// TODO: make this a server-side variable. -aj
			if( NSMAN->m_EvalPlayerData[i].grade < Grade_Tier03 )
				m_textUsers[i].PlayCommand("Tier02OrBetter");

			ON_COMMAND( m_textUsers[i] );
			LOG->Trace( "SMNETCheckpoint%d", i );
		}
		return; // No need to let ScreenEvaluation get a hold of this.
	}
	else if( SM == SM_GoToNextScreen )
	{
		NSMAN->ReportNSSOnOff( 4 );
	}
	ScreenEvaluation::HandleScreenMessage( SM );
}

void ScreenNetEvaluation::TweenOffScreen( )
{
	for( int i=0; i<m_iActivePlayers; ++i )
		OFF_COMMAND( m_textUsers[i] );
	OFF_COMMAND( m_rectUsersBG );
	ScreenEvaluation::TweenOffScreen();
}

void ScreenNetEvaluation::UpdateStats()
{
	if( m_iCurrentPlayer >= (int) NSMAN->m_EvalPlayerData.size() )
		return;

	// Only run these commands if the theme has these things shown; not every
	// theme has them, so don't assume. -aj
	if( THEME->GetMetricB(m_sName,"ShowGradeArea") )
		m_Grades[m_pActivePlayer].SetGrade( (Grade)NSMAN->m_EvalPlayerData[m_iCurrentPlayer].grade );
	if( THEME->GetMetricB(m_sName,"ShowScoreArea") )
		m_textScore[m_pActivePlayer].SetTargetNumber( NSMAN->m_EvalPlayerData[m_iCurrentPlayer].score );

	//Values greater than 6 will cause a crash
	if( NSMAN->m_EvalPlayerData[m_iCurrentPlayer].difficulty < 6 )
	{
		m_DifficultyIcon[m_pActivePlayer].SetPlayer( m_pActivePlayer );
		m_DifficultyIcon[m_pActivePlayer].SetFromDifficulty( NSMAN->m_EvalPlayerData[m_iCurrentPlayer].difficulty );
	}

	for( int j=0; j<NETNUMTAPSCORES; ++j )
	{
		// The name will be blank if ScreenEvaluation determined the line
		// should not be shown.
		if( !m_textJudgmentLineNumber[j][m_pActivePlayer].GetName().empty() )
		{
			m_textJudgmentLineNumber[j][m_pActivePlayer].SetTargetNumber( NSMAN->m_EvalPlayerData[m_iCurrentPlayer].tapScores[j] );
		}
	}

	m_textPlayerOptions[m_pActivePlayer].SetText( NSMAN->m_EvalPlayerData[m_iCurrentPlayer].playerOptions );

	StepsType st = GAMESTATE->GetCurrentStyle(PLAYER_INVALID)->m_StepsType;
	Difficulty dc = NSMAN->m_EvalPlayerData[m_iCurrentPlayer].difficulty;
	Steps *pSteps = SongUtil::GetOneSteps( GAMESTATE->m_pCurSong, st, dc );

	// broadcast a message so themes know that the active player has changed. -aj
	Message msg("UpdateNetEvalStats");
	msg.SetParam( "ActivePlayerIndex", m_pActivePlayer );
	msg.SetParam( "Difficulty", NSMAN->m_EvalPlayerData[m_iCurrentPlayer].difficulty );
	msg.SetParam( "Score", NSMAN->m_EvalPlayerData[m_iCurrentPlayer].score );
	msg.SetParam( "Grade", NSMAN->m_EvalPlayerData[m_iCurrentPlayer].grade );
	msg.SetParam( "PlayerOptions", NSMAN->m_EvalPlayerData[m_iCurrentPlayer].playerOptions );
	if( pSteps )
		msg.SetParam( "Steps", pSteps );
	MESSAGEMAN->Broadcast(msg);
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the ScreenNetEvaluation. */ 
class LunaScreenNetEvaluation: public Luna<ScreenNetEvaluation>
{
public:
	static int GetNumActivePlayers( T* p, lua_State *L ) { lua_pushnumber( L, p->GetNumActivePlayers() ); return 1; }

	LunaScreenNetEvaluation()
	{
  		ADD_METHOD( GetNumActivePlayers );
	}
};

LUA_REGISTER_DERIVED_CLASS( ScreenNetEvaluation, ScreenEvaluation )
// lua end

#endif

/*
 * (c) 2004-2005 Charles Lohr, Joshua Allen
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

