#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenNetEvaluation.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "RageLog.h"

const int NUM_SCORE_DIGITS	=	9;

#define USERSBG_WIDTH				THEME->GetMetricF("ScreenNetEvaluation","UsersBGWidth")
#define USERSBG_HEIGHT				THEME->GetMetricF("ScreenNetEvaluation","UsersBGHeight")
#define USERSBG_COMMAND				THEME->GetMetricA("ScreenNetEvaluation","UsersBGCommand")

#define USERDX						THEME->GetMetricF("ScreenNetEvaluation","UserDX")
#define USERDY						THEME->GetMetricF("ScreenNetEvaluation","UserDY")

#define MAX_COMBO_NUM_DIGITS		THEME->GetMetricI("ScreenEvaluation","MaxComboNumDigits")

AutoScreenMessage( SM_GotEval ) 

REGISTER_SCREEN_CLASS( ScreenNetEvaluation );
ScreenNetEvaluation::ScreenNetEvaluation (const CString & sClassName) : ScreenEvaluation( sClassName )
{
}

void ScreenNetEvaluation::Init()
{
	ScreenEvaluation::Init();

	m_bHasStats = false;
	m_pActivePlayer = PLAYER_1;	
	m_iCurrentPlayer = 0;

	FOREACH_PlayerNumber (pn)
		if ( GAMESTATE->IsPlayerEnabled( pn ) )
			m_pActivePlayer = pn;

	if (m_pActivePlayer == PLAYER_1)
		m_iShowSide = 2;
	else
		m_iShowSide = 1;

	m_rectUsersBG.SetWidth( USERSBG_WIDTH );
	m_rectUsersBG.SetHeight( USERSBG_HEIGHT );
	m_rectUsersBG.RunCommands( USERSBG_COMMAND );
	m_rectUsersBG.SetName( "UsersBG" );
	ON_COMMAND( m_rectUsersBG );
	
	m_rectUsersBG.SetXY(
		THEME->GetMetricF("ScreenNetEvaluation",ssprintf("UsersBG%dX",m_iShowSide)),
		THEME->GetMetricF("ScreenNetEvaluation",ssprintf("UsersBG%dY",m_iShowSide)) );

	this->AddChild( &m_rectUsersBG );

	RedoUserTexts();

	NSMAN->ReportNSSOnOff( 5 );
}

void ScreenNetEvaluation::RedoUserTexts()
{
	m_iActivePlayers = NSMAN->m_ActivePlayers;
	//If unessiary, just don't do this function.
	if ( m_iActivePlayers == (int)m_textUsers.size() )
		return;

	float cx = THEME->GetMetricF("ScreenNetEvaluation",ssprintf("User%dX",m_iShowSide));
	float cy = THEME->GetMetricF("ScreenNetEvaluation",ssprintf("User%dY",m_iShowSide));
	
	m_iCurrentPlayer = 0;

	for( int i=0; i<m_iActivePlayers; ++i )
		this->RemoveChild( &m_textUsers[i] );

	m_textUsers.resize(m_iActivePlayers);

	for( int i=0; i<m_iActivePlayers; ++i )
	{
		m_textUsers[i].LoadFromFont( THEME->GetPathF(m_sName,"names") );
		m_textUsers[i].SetName( "User" );
		m_textUsers[i].SetShadowLength( 1 );
		m_textUsers[i].SetXY( cx, cy );

		this->AddChild( &m_textUsers[i] );
		cx+=USERDX;
		cy+=USERDY;
	}
}

void ScreenNetEvaluation::MenuLeft( const InputEventPlus &input )
{
	MenuUp( input );
}

void ScreenNetEvaluation::MenuUp( const InputEventPlus &input )
{
	if ( m_iActivePlayers == 0 )
		return;
	if (!m_bHasStats)
		return;
	COMMAND( m_textUsers[m_iCurrentPlayer], "DeSel" );
	m_iCurrentPlayer = (m_iCurrentPlayer + m_iActivePlayers - 1) % m_iActivePlayers;
	COMMAND( m_textUsers[m_iCurrentPlayer], "Sel" );
	UpdateStats();
}

void ScreenNetEvaluation::MenuRight( const InputEventPlus &input )
{
	MenuDown( input );
}

void ScreenNetEvaluation::MenuDown( const InputEventPlus &input )
{
	if ( m_iActivePlayers == 0 )
		return;
	if ( !m_bHasStats )
		return;
	COMMAND( m_textUsers[m_iCurrentPlayer], "DeSel" );
	m_iCurrentPlayer = (m_iCurrentPlayer + 1) % m_iActivePlayers;
	COMMAND( m_textUsers[m_iCurrentPlayer], "Sel" );
	UpdateStats();
}

void ScreenNetEvaluation::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GotEval )
	{
		m_bHasStats = true;

		LOG->Trace("SMNETDebug:%d,%d",m_iActivePlayers,NSMAN->m_ActivePlayers);

		//XXX: This does remove functionalty, but until
		// the cause for the strange snowball ScreenManager 
		// crash can be found... this is the only way.
		if ( m_iActivePlayers != NSMAN->m_ActivePlayers )
			return;

		RedoUserTexts();

		LOG->Trace("SMNETCheckpoint");
		for( int i=0; i<m_iActivePlayers; ++i )
		{
			//Strange occourances because of timing
			//cause these things not to work right
			//and will sometimes cause a crash.
			//We should make SURE we won't crash!
			if ( size_t(i) >= NSMAN->m_EvalPlayerData.size() )
				break;

			if ( size_t(NSMAN->m_EvalPlayerData[i].name) >= NSMAN->m_PlayerNames.size() )
				break;

			if ( NSMAN->m_EvalPlayerData[i].name < 0 )
				break;

			if ( size_t(i) >= m_textUsers.size() )
				break;

			m_textUsers[i].SetText( NSMAN->m_PlayerNames[NSMAN->m_EvalPlayerData[i].name] );
			if ( NSMAN->m_EvalPlayerData[i].grade < Grade_Tier03 )	//Yes, hardcoded (I'd like to leave it that way)
				m_textUsers[i].TurnRainbowOn();
			else
				m_textUsers[i].TurnRainbowOff();
			ON_COMMAND( m_textUsers[i] );
			LOG->Trace("SMNETCheckpoint%d",i);
		}
		return;	//no need to let ScreenEvaluation get ahold of this.
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

	m_Grades[m_pActivePlayer].SetGrade( m_pActivePlayer, (Grade)NSMAN->m_EvalPlayerData[m_iCurrentPlayer].grade );
	m_Grades[m_pActivePlayer].Spin();
	m_Grades[m_pActivePlayer].SettleImmediately();

	m_textScore[m_pActivePlayer].SetText( ssprintf("%*.0i", NUM_SCORE_DIGITS, NSMAN->m_EvalPlayerData[m_iCurrentPlayer].score) );

	//Values greater than 6 will cause crash
	if ( NSMAN->m_EvalPlayerData[m_iCurrentPlayer].difficulty < 6 )
		m_DifficultyIcon[m_pActivePlayer].SetFromDifficulty( m_pActivePlayer, NSMAN->m_EvalPlayerData[m_iCurrentPlayer].difficulty );

	for (int j=0; j<NETNUMTAPSCORES; ++j)
	{
		int iNumDigits = (j==JudgeLine_MaxCombo) ? MAX_COMBO_NUM_DIGITS : 4;
		if (m_textJudgeNumbers[j][m_pActivePlayer].m_pFont != NULL)
			m_textJudgeNumbers[j][m_pActivePlayer].SetText( ssprintf( "%*d", iNumDigits, NSMAN->m_EvalPlayerData[m_iCurrentPlayer].tapScores[j] ) );
	}

	m_textPlayerOptions[m_pActivePlayer].SetText( NSMAN->m_EvalPlayerData[m_iCurrentPlayer].playerOptions );
}

#endif

