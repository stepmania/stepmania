#include "global.h"
#include "AttackDisplay.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "ActorUtil.h"
#include "Character.h"
#include "RageLog.h"
#include <set>
#include "PlayerState.h"


RString GetAttackPieceName( const RString &sAttack )
{
	RString ret = ssprintf( "attack %s", sAttack.c_str() );

	/* 1.5x -> 1_5x.  If we pass a period to THEME->GetPathTo, it'll think
	 * we're looking for a specific file and not search. */
	ret.Replace( ".", "_" );

	return ret;
}

AttackDisplay::AttackDisplay()
{
	if( GAMESTATE->m_PlayMode != PLAY_MODE_BATTLE &&
		GAMESTATE->m_PlayMode != PLAY_MODE_RAVE )
		return;

	m_sprAttack.SetDiffuseAlpha( 0 );	// invisible
	this->AddChild( &m_sprAttack );
}

void AttackDisplay::Init( const PlayerState* pPlayerState )
{
	m_pPlayerState = pPlayerState;

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;
	m_sprAttack.SetName( ssprintf("TextP%d",pn+1) );

	if( GAMESTATE->m_PlayMode != PLAY_MODE_BATTLE &&
		GAMESTATE->m_PlayMode != PLAY_MODE_RAVE )
		return;

	set<RString> attacks;
	for( int al=0; al<NUM_ATTACK_LEVELS; al++ )
	{
		const Character *ch = GAMESTATE->m_pCurCharacters[pn];
		ASSERT( ch );
		const RString* asAttacks = ch->m_sAttacks[al];
		for( int att = 0; att < NUM_ATTACKS_PER_LEVEL; ++att )
			attacks.insert( asAttacks[att] );
	}

	for( set<RString>::const_iterator it = attacks.begin(); it != attacks.end(); ++it )
	{
		const RString path = THEME->GetPathG( "AttackDisplay", GetAttackPieceName( *it ), true );
		if( path == "" )
		{
			LOG->Trace( "Couldn't find \"%s\"", GetAttackPieceName( *it ).c_str() );
			continue;
		}

		m_TexturePreload.Load( path );
	}
}


void AttackDisplay::Update( float fDelta )
{
	ActorFrame::Update( fDelta );

	if( GAMESTATE->m_PlayMode != PLAY_MODE_BATTLE &&
		GAMESTATE->m_PlayMode != PLAY_MODE_RAVE )
		return;

	if( !m_pPlayerState->m_bAttackBeganThisUpdate )
		return;
	// don't handle this again

	for( unsigned s=0; s<m_pPlayerState->m_ActiveAttacks.size(); s++ )
	{
		const Attack& attack = m_pPlayerState->m_ActiveAttacks[s];

		if( attack.fStartSecond >= 0 )
			continue; /* hasn't started yet */

		if( attack.fSecsRemaining <= 0 )
			continue; /* ended already */

		if( attack.IsBlank() )
			continue;

		SetAttack( attack.sModifiers );
		break;
	}
}

void AttackDisplay::SetAttack( const RString &sText )
{
	const RString path = THEME->GetPathG( "AttackDisplay", GetAttackPieceName(sText), true );
	if( path == "" )
		return;

	m_sprAttack.SetDiffuseAlpha( 1 );
	m_sprAttack.Load( path );

	// TODO: Remove use of PlayerNumber.
	PlayerNumber pn = m_pPlayerState->m_PlayerNumber;

	const RString sName = ssprintf( "%sP%i", sText.c_str(), pn+1 );
	m_sprAttack.RunCommands( THEME->GetMetricA("AttackDisplay", sName + "OnCommand") );
}

/*
 * (c) 2003 Chris Danford
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
