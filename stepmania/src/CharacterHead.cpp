#include "global.h"
#include "CharacterHead.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "CharacterHead.h"
#include "RageTimer.h"
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
