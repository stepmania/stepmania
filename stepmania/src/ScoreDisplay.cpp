#include "global.h"
#include "ScoreDisplay.h"

void ScoreDisplay::Init( const PlayerState* pPlayerState, const PlayerStageStats* pPlayerStageStats )
{
	m_pPlayerState = pPlayerState;
	m_pPlayerStageStats = pPlayerStageStats;
}
