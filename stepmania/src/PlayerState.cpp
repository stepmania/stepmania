#include "global.h"
#include "PlayerState.h"

void PlayerState::ResetNoteSkins()
{
	m_BeatToNoteSkin.clear();
	m_BeatToNoteSkin[-1000] = m_PlayerOptions.m_sNoteSkin;
}

void PlayerState::Update( float fDelta )
{
	m_CurrentPlayerOptions.Approach( m_PlayerOptions, fDelta );
}

int PlayerState::GetSumOfActiveAttackLevels() const
{
	int iSum = 0;

	for( unsigned s=0; s<m_ActiveAttacks.size(); s++ )
		if( m_ActiveAttacks[s].fSecsRemaining > 0 && m_ActiveAttacks[s].level != NUM_ATTACK_LEVELS )
			iSum += m_pPlayerState[pn]m_ActiveAttacks[s].level;

	return iSum;
}

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez
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
