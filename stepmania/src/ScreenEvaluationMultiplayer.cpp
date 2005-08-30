#include "global.h"
#include "ScreenEvaluationMultiplayer.h"
#include "BitmapText.h"
#include "PlayerState.h"
#include "PlayerStageStats.h"
#include "GameState.h"
#include "StatsManager.h"
#include "GameCommand.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "SongManager.h"
#include "song.h"
#include "ScoreKeeperMAX2.h"
#include "PercentageDisplay.h"


static const CString JudgeLineNames[] = {
	"Marvelous",
	"Perfect",
	"Great",
	"Good",
	"Boo",
	"Miss",
	"Ok",
};
XToString( JudgeLine, NUM_JudgeLine );

static void PositionItem( LuaExpression &expr, Actor *pSelf, int iItemIndex, int iNumItems )
{
	Lua *L = LUA->Get();
	expr.PushSelf( L );
	ASSERT( !lua_isnil(L, -1) );
	pSelf->PushSelf( L );
	LuaHelpers::Push( iItemIndex, L );
	LuaHelpers::Push( iNumItems, L );
	lua_call( L, 3, 0 ); // 3 args, 0 results
	LUA->Release(L);
}

class MultiplayerEvalScoreRow : public ActorFrame
{
protected:
	AutoActor m_sprFrame;
	PercentageDisplay	m_score;
	BitmapText	m_textJudgmentNumber[NUM_JudgeLine];

public:
	MultiplayerEvalScoreRow( MultiPlayer mp, int iRankIndex )
	{
		PlayerState *pPlayerState = GAMESTATE->m_pMultiPlayerState[mp];
		PlayerStageStats *pPlayerStageStats = &STATSMAN->m_CurStageStats.m_multiPlayer[mp];

		ASSERT( GAMESTATE->IsPlayerEnabled(pPlayerState) );


		m_sprFrame.Load( THEME->GetPathG("MultiplayerEvalScoreRow","frame") );
		this->AddChild( m_sprFrame );


		m_score.Load( pPlayerState, pPlayerStageStats, "MultiplayerEvalScoreRow Percent", true );
		m_score.SetName( "Score" );
		ActorUtil::OnCommand( &m_score, "MultiplayerEvalScoreRow" );
		this->AddChild( &m_score );


		LuaExpression expr;
		expr.SetFromExpression( THEME->GetMetric("MultiplayerEvalScoreRow","NumberOnCommandFunction") );

		for( int i=0; i<NUM_JudgeLine; i++ )
		{
			BitmapText &text = m_textJudgmentNumber[i];
			text.LoadFromFont( THEME->GetPathF("MultiplayerEvalScoreRow","JudgmentNumber") );
			this->AddChild( &text );

			int iVal = -1;
			switch( i )
			{
			default:	ASSERT(0);
			case JudgeLine_Marvelous:	iVal = pPlayerStageStats->iTapNoteScores[TNS_MARVELOUS];break;
			case JudgeLine_Perfect:		iVal = pPlayerStageStats->iTapNoteScores[TNS_PERFECT];	break;
			case JudgeLine_Great:		iVal = pPlayerStageStats->iTapNoteScores[TNS_GREAT];	break;
			case JudgeLine_Good:		iVal = pPlayerStageStats->iTapNoteScores[TNS_GOOD];		break;
			case JudgeLine_Boo:			iVal = pPlayerStageStats->iTapNoteScores[TNS_BOO];		break;
			case JudgeLine_Miss:		iVal = pPlayerStageStats->iTapNoteScores[TNS_MISS];		break;
			case JudgeLine_Ok:			iVal = pPlayerStageStats->iHoldNoteScores[HNS_OK];		break;
			}
			
			CString s = ssprintf( "%3d", iVal );
			text.SetText( s );

			PositionItem( expr, &text, i, NUM_JudgeLine );
		}
	}
};


REGISTER_SCREEN_CLASS( ScreenEvaluationMultiplayer );
ScreenEvaluationMultiplayer::ScreenEvaluationMultiplayer( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	//
	// debugging
	//
	if( PREFSMAN->m_bScreenTestMode )
	{
		GAMESTATE->m_PlayMode.Set( PLAY_MODE_REGULAR );
		GAMESTATE->m_pCurStyle.Set( GAMEMAN->GameAndStringToStyle(GAMEMAN->GetDefaultGame(),"single") );

		GAMESTATE->m_bMultiplayer = true;

		STATSMAN->m_CurStageStats.playMode = GAMESTATE->m_PlayMode;
		STATSMAN->m_CurStageStats.pStyle = GAMESTATE->m_pCurStyle;
		STATSMAN->m_CurStageStats.StageType = StageStats::STAGE_NORMAL;
		GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
		GAMESTATE->m_pCurSong.Set( SONGMAN->GetRandomSong() );
		STATSMAN->m_CurStageStats.vpPlayedSongs.push_back( GAMESTATE->m_pCurSong );
		STATSMAN->m_CurStageStats.vpPossibleSongs.push_back( GAMESTATE->m_pCurSong );
		GAMESTATE->m_pCurCourse.Set( SONGMAN->GetRandomCourse() );
		GAMESTATE->m_iCurrentStageIndex = 0;

		GAMESTATE->m_pCurSteps[PLAYER_1].Set( GAMESTATE->m_pCurSong->GetAllSteps()[0] );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].vpPlayedSteps.push_back( GAMESTATE->m_pCurSteps[PLAYER_1] );
		STATSMAN->m_CurStageStats.m_player[PLAYER_1].vpPossibleSteps.push_back( GAMESTATE->m_pCurSteps[PLAYER_1] );
		GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.m_fScrollSpeed = 2;
		GAMESTATE->m_pPlayerState[PLAYER_1]->m_PlayerOptions.ChooseRandomMofifiers();
	
		FOREACH_MultiPlayer( p )
		{
			GAMESTATE->m_bIsMultiPlayerJoined[p] = (rand() % 4) != 0;
		}

		FOREACH_MultiPlayer( p )
		{
			STATSMAN->m_CurStageStats.m_multiPlayer[p].iActualDancePoints = rand()%3;
			STATSMAN->m_CurStageStats.m_multiPlayer[p].iPossibleDancePoints = 2;
			STATSMAN->m_CurStageStats.m_multiPlayer[p].iCurCombo = 0;
			STATSMAN->m_CurStageStats.m_multiPlayer[p].UpdateComboList( 0, false );
			STATSMAN->m_CurStageStats.m_multiPlayer[p].iCurCombo = 1;
			STATSMAN->m_CurStageStats.m_multiPlayer[p].UpdateComboList( 1, false );
			STATSMAN->m_CurStageStats.m_multiPlayer[p].iCurCombo = 50;
			STATSMAN->m_CurStageStats.m_multiPlayer[p].UpdateComboList( 25, false );
			STATSMAN->m_CurStageStats.m_multiPlayer[p].iCurCombo = 250;
			STATSMAN->m_CurStageStats.m_multiPlayer[p].UpdateComboList( 100, false );
			if( rand()%2 )
			{
				STATSMAN->m_CurStageStats.m_multiPlayer[p].iCurCombo = rand()%11000;
				STATSMAN->m_CurStageStats.m_multiPlayer[p].UpdateComboList( 110, false );
			}
			if( rand()%5 == 0 )
			{
				STATSMAN->m_CurStageStats.m_multiPlayer[p].bFailedEarlier = true;
			}
			STATSMAN->m_CurStageStats.m_multiPlayer[p].iTapNoteScores[TNS_MARVELOUS] = rand()%3;
			STATSMAN->m_CurStageStats.m_multiPlayer[p].iTapNoteScores[TNS_PERFECT] = rand()%3;
			STATSMAN->m_CurStageStats.m_multiPlayer[p].iTapNoteScores[TNS_GREAT] = rand()%3;
			STATSMAN->m_CurStageStats.m_multiPlayer[p].iPossibleGradePoints = 4*ScoreKeeperMAX2::TapNoteScoreToGradePoints(TNS_MARVELOUS, false);
			STATSMAN->m_CurStageStats.m_multiPlayer[p].fLifeRemainingSeconds = randomf( 90, 580 );
		}

		STATSMAN->m_vPlayedStageStats.clear();
	}


	ASSERT( GAMESTATE->m_bMultiplayer );
}

ScreenEvaluationMultiplayer::~ScreenEvaluationMultiplayer()
{
	FOREACH( MultiplayerEvalScoreRow*, m_vpMultiplayerEvalScoreRow, iter )
		SAFE_DELETE( *iter );
	m_vpMultiplayerEvalScoreRow.clear();
}

struct EnabledPlayerIndexAndScore
{
	MultiPlayer m_MultiPlayer;
	float fScore;

	bool operator<( const EnabledPlayerIndexAndScore &other ) const { return fScore > other.fScore; }
};

void ScreenEvaluationMultiplayer::Init()
{
	ScreenWithMenuElements::Init();

	int iNumEnabledPlayers = 0;
	FOREACH_EnabledMultiPlayer( p )
		iNumEnabledPlayers++;

	int iNumLines = iNumEnabledPlayers+1;

	LuaExpression exprBullet;
	exprBullet.SetFromExpression( THEME->GetMetric("ScreenEvaluationMultiplayer","BulletOnCommandFunction") );
	LuaExpression exprLine;
	exprLine.SetFromExpression( THEME->GetMetric("ScreenEvaluationMultiplayer","LineOnCommandFunction") );

	this->AddChild( &m_frameLabels );
	PositionItem( exprLine, &m_frameLabels, 0, iNumLines );
	{
		LuaExpression expr;
		expr.SetFromExpression( THEME->GetMetric("ScreenEvaluationMultiplayer","LabelOnCommandFunction") );

		FOREACH_JudgeLine( i )
		{
			AutoActor &a = m_sprJudgmentLabel[i];
			a.Load( THEME->GetPathG("ScreenEvaluationMultiplayer","JudgmentLabel"+JudgeLineToString(i)) );
			m_frameLabels.AddChild( m_sprJudgmentLabel[i] );

			PositionItem( expr, a, i, NUM_JudgeLine );
		}
	}

	vector<EnabledPlayerIndexAndScore> v;
	FOREACH_EnabledMultiPlayer( p )
	{
		PlayerStageStats *pPlayerStageStats = &STATSMAN->m_CurStageStats.m_multiPlayer[p];
		EnabledPlayerIndexAndScore t = { p, pPlayerStageStats->GetPercentDancePoints() };
		v.push_back( t );
	}
	stable_sort( v.begin(), v.end() );

	GameCommand gc;
	int iRankIndex = 0;
	m_vsprBullet.resize( v.size() );
	FOREACH( EnabledPlayerIndexAndScore, v, iter )
	{
		int i = iter - v.begin();
		gc.m_iIndex = i;
		gc.m_MultiPlayer = iter->m_MultiPlayer;

		Lua *L = LUA->Get();
		gc.PushSelf( L );
		lua_setglobal( L, "ThisGameCommand" );
		LUA->Release( L );

		{
			MultiplayerEvalScoreRow *pRow = new MultiplayerEvalScoreRow( iter->m_MultiPlayer, iRankIndex );
			m_vpMultiplayerEvalScoreRow.push_back( pRow );
			PositionItem( exprLine, pRow, iRankIndex+1, iNumLines );
			this->AddChild( pRow );
		}

		{
			AutoActor &a = m_vsprBullet[i];
			a.Load( THEME->GetPathG("ScreenEvaluationMultiplayer","Bullet") );
			PositionItem( exprBullet, a, iRankIndex+1, iNumLines );
			this->AddChild( a );
		}

		LUA->UnsetGlobal( "ThisGameCommand" );

		iRankIndex++;
	}

	this->SortByDrawOrder();
}

void ScreenEvaluationMultiplayer::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenWithMenuElements::HandleScreenMessage( SM );
}

void ScreenEvaluationMultiplayer::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( this->IsTransitioning() )
		return;

	ScreenWithMenuElements::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenEvaluationMultiplayer::Update( float fDeltaTime )
{
	ScreenWithMenuElements::Update(fDeltaTime);
}

void ScreenEvaluationMultiplayer::DrawPrimitives()
{
	ScreenWithMenuElements::DrawPrimitives();
}

void ScreenEvaluationMultiplayer::MenuBack( PlayerNumber pn )
{
	MenuStart( pn );
}

void ScreenEvaluationMultiplayer::MenuStart( PlayerNumber pn )
{
	
	this->StartTransitioning( SM_GoToNextScreen );
}


/*
 * (c) 2005 Chris Danford
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
