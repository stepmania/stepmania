#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: CharacterHead

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CharacterHead.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "CharacterHead.h"
#include "RageTimer.h"
#include <math.h>
#include "ThemeManager.h"
#include "GameState.h"
#include "Character.h"


const float SECONDS_TO_SHOW_FACE = 1.5f;


CharacterHead::CharacterHead()
{
}

void CharacterHead::LoadFromCharacter( Character* pCharacter )
{
	CString sHeadPath = pCharacter->GetHeadPath();
	if( sHeadPath.empty() )
		return;

	Load( sHeadPath );
	ASSERT( this->GetNumStates() == NUM_FACES );
	StopAnimating();
	SetFace( normal );
}

void CharacterHead::Update( float fDelta )
{
	if( this->GetTexture() == NULL )
		return; /* not loaded */

	if( m_fSecondsUntilReturnToNormal > 0 )
	{
		m_fSecondsUntilReturnToNormal -= fDelta;

		if( m_fSecondsUntilReturnToNormal < 0 )
		{
			m_fSecondsUntilReturnToNormal = 0;
			this->SetState( normal );
		}
	}

	if( GAMESTATE->m_fOpponentHealthPercent == 0 )
		this->SetState( defeated );

	Sprite::Update( fDelta );
}

void CharacterHead::SetFace( Face face )
{
	if( GAMESTATE->m_fOpponentHealthPercent == 0 )
		return;

	this->SetState( face );
	m_fSecondsUntilReturnToNormal = SECONDS_TO_SHOW_FACE;
}

