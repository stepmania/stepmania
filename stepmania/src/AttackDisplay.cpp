#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: AttackDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "AttackDisplay.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "ActorUtil.h"
#include "Character.h"
#include "RageLog.h"
#include "RageTextureManager.h"
#include <set>

CString GetAttackPath( const CString &sAttack )
{
	CString ret = ssprintf( "AttackDisplay attack %s", sAttack.c_str() );

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

	m_sprAttack.SetName( ssprintf("TextP%d",m_PlayerNumber+1) );
	m_sprAttack.SetDiffuseAlpha( 0 );	// invisible
	this->AddChild( &m_sprAttack );
}

void AttackDisplay::Init( PlayerNumber pn )
{
	m_PlayerNumber = pn;

	if( GAMESTATE->m_PlayMode != PLAY_MODE_BATTLE &&
		GAMESTATE->m_PlayMode != PLAY_MODE_RAVE )
		return;

	set<CString> attacks;
	for( int al=0; al<NUM_ATTACK_LEVELS; al++ )
	{
		const Character *ch = GAMESTATE->m_pCurCharacters[pn];
		ASSERT( ch );
		const CString* asAttacks = ch->m_sAttacks[al];
		for( int att = 0; att < NUM_ATTACKS_PER_LEVEL; ++att )
			attacks.insert( asAttacks[att] );
	}

	for( set<CString>::const_iterator it = attacks.begin(); it != attacks.end(); ++it )
	{
		const CString ThemePath = GetAttackPath( *it );
		const CString path = THEME->GetPathToG( ThemePath, true );
		if( path == "" )
		{
			LOG->Trace( "Couldn't find \"%s\"", ThemePath.c_str() );
			continue;
		}

		TEXTUREMAN->CacheTexture( path );
	}
}


void AttackDisplay::Update( float fDelta )
{
	ActorFrame::Update( fDelta );

	if( GAMESTATE->m_PlayMode != PLAY_MODE_BATTLE &&
		GAMESTATE->m_PlayMode != PLAY_MODE_RAVE )
		return;

	if( !GAMESTATE->m_bAttackBeganThisUpdate[m_PlayerNumber] )
		return;
	// don't handle this again

	for( unsigned s=0; s<GAMESTATE->m_ActiveAttacks[m_PlayerNumber].size(); s++ )
	{
		if( GAMESTATE->m_ActiveAttacks[m_PlayerNumber][s].fStartSecond >= 0 )
			continue; /* hasn't started yet */

		if( GAMESTATE->m_ActiveAttacks[m_PlayerNumber][s].fSecsRemaining <= 0 )
			continue; /* ended already */

		if( GAMESTATE->m_ActiveAttacks[m_PlayerNumber][s].IsBlank() )
			continue;

		SetAttack( GAMESTATE->m_ActiveAttacks[m_PlayerNumber][s].sModifier );
		break;
	}
}

void AttackDisplay::SetAttack( const CString &sText )
{
	const CString path = THEME->GetPathToG( GetAttackPath( sText ), true );
	if( path == "" )
		return;

	m_sprAttack.SetDiffuseAlpha( 1 );
	m_sprAttack.Load( path );
	const CString sName = ssprintf( "%sP%i", sText.c_str(), m_PlayerNumber+1 );
	m_sprAttack.Command( THEME->GetMetric("AttackDisplay", sName + "OnCommand" ) );
}
