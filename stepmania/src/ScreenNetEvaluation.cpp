#include "ScreenNetEvaluation.h"
#include "ThemeManager.h"
#include "GameState.h"

const int NUM_SCORE_DIGITS	=	9;

#define USERSBG_WIDTH				THEME->GetMetricF("ScreenNetEvaluation","UsersBGWidth")
#define USERSBG_HEIGHT				THEME->GetMetricF("ScreenNetEvaluation","UsersBGHeight")
#define USERSBG_COLOR				THEME->GetMetricC("ScreenNetEvaluation","UsersBGColor")

#define USERDX						THEME->GetMetricF("ScreenNetEvaluation","UserDX")
#define USERDY						THEME->GetMetricF("ScreenNetEvaluation","UserDY")

#define MAX_COMBO_NUM_DIGITS		THEME->GetMetricI("ScreenEvaluation","MaxComboNumDigits")

ScreenNetEvaluation::ScreenNetEvaluation (const CString & sClassName) : ScreenEvaluation( sClassName )
{
	m_pActivePlayer = PLAYER_1;	

	FOREACH_PlayerNumber (pn)
		if ( GAMESTATE->IsPlayerEnabled( pn ) )
			m_pActivePlayer = pn;

	int ShowSide;

	if (m_pActivePlayer == PLAYER_1)
		ShowSide = 2;
	else
		ShowSide = 1;

	m_rectUsersBG.SetWidth( USERSBG_WIDTH );
	m_rectUsersBG.SetHeight( USERSBG_HEIGHT );
	m_rectUsersBG.SetDiffuse( USERSBG_COLOR );
	m_rectUsersBG.SetName( "UsersBG" );
	ON_COMMAND( m_rectUsersBG );
	
	m_rectUsersBG.SetXY(
		THEME->GetMetricF("ScreenNetEvaluation",ssprintf("UsersBG%dX",ShowSide)),
		THEME->GetMetricF("ScreenNetEvaluation",ssprintf("UsersBG%dY",ShowSide)) );

	this->AddChild( &m_rectUsersBG );

	float cx = THEME->GetMetricF("ScreenNetEvaluation",ssprintf("User%dX",ShowSide));
	float cy = THEME->GetMetricF("ScreenNetEvaluation",ssprintf("User%dY",ShowSide));
	
	m_iActivePlayers = NSMAN->m_ActivePlayers;
	m_iCurrentPlayer = 0;

	for( int i=0; i<m_iActivePlayers; i++ )
	{
		if (NSMAN->m_PlayerNames[NSMAN->m_ActivePlayer[i]] == GAMESTATE->GetPlayerDisplayName(m_pActivePlayer) )
			m_iCurrentPlayer = i;

		m_textUsers[i].LoadFromFont( THEME->GetPathF(m_sName,"names") );
		m_textUsers[i].SetName( "User" );
		m_textUsers[i].SetShadowLength( 1 );
		m_textUsers[i].SetXY( cx, cy );
		m_textUsers[i].SetText( NSMAN->m_PlayerNames[NSMAN->m_ActivePlayer[i]] );

		ON_COMMAND( m_textUsers[i] );
		this->AddChild( &m_textUsers[i] );
		cx+=USERDX;
		cy+=USERDY;
	}

	COMMAND( m_textUsers[m_iCurrentPlayer], "Sel" );
}

void ScreenNetEvaluation::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	MenuUp( pn, type );
}

void ScreenNetEvaluation::MenuUp( PlayerNumber pn, const InputEventType type )
{
	COMMAND( m_textUsers[m_iCurrentPlayer], "DeSel" );
	m_iCurrentPlayer = (m_iCurrentPlayer + m_iActivePlayers - 1) % m_iActivePlayers;
	COMMAND( m_textUsers[m_iCurrentPlayer], "Sel" );
	UpdateStats();
}

void ScreenNetEvaluation::MenuRight( PlayerNumber pn, const InputEventType type )
{
	MenuDown( pn, type );
}

void ScreenNetEvaluation::MenuDown( PlayerNumber pn, const InputEventType type )
{
	COMMAND( m_textUsers[m_iCurrentPlayer], "DeSel" );
	m_iCurrentPlayer = (m_iCurrentPlayer + 1) % m_iActivePlayers;
	COMMAND( m_textUsers[m_iCurrentPlayer], "Sel" );
	UpdateStats();
}

void ScreenNetEvaluation::UpdateStats()
{
	m_Grades[m_pActivePlayer].SetGrade( (PlayerNumber)m_pActivePlayer, (Grade)NSMAN->m_EvalPlayerData[m_iCurrentPlayer].grade );
	m_Grades[m_pActivePlayer].Spin();
	m_Grades[m_pActivePlayer].SettleImmediately();

	m_textScore[m_pActivePlayer].SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS, NSMAN->m_EvalPlayerData[m_iCurrentPlayer].score) );

	m_DifficultyIcon[m_pActivePlayer].SetFromDifficulty( m_pActivePlayer, NSMAN->m_EvalPlayerData[m_iCurrentPlayer].difficulty );

	for (int j=0;j<NETNUMTAPSCORES;j++)
	{
		int iNumDigits = (j==max_combo) ? MAX_COMBO_NUM_DIGITS : 4;
		m_textJudgeNumbers[j][m_pActivePlayer].SetText( ssprintf( "%*d", iNumDigits, NSMAN->m_EvalPlayerData[m_iCurrentPlayer].tapScores[j] ) );
	}
}
