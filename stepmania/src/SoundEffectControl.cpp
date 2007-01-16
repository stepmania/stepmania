#include "global.h"
#include "SoundEffectControl.h"
#include "RageSoundReader.h"
#include "InputMapper.h"
#include "GameState.h"
#include "NoteData.h"
#include "PlayerState.h"
#include "RageSoundReader.h"

SoundEffectControl::SoundEffectControl()
{
	m_bLocked = false;
	m_fSample = 0.0f;
	m_fLastLevel = 0.0f;
	m_pPlayerState = NULL;
	m_pNoteData = NULL;
}

void SoundEffectControl::Load( const RString &sType, PlayerState *pPlayerState, const NoteData *pNoteData )
{
	SOUND_PROPERTY.Load( sType, "SoundProperty" );
	LOCK_TO_HOLD.Load( sType, "LockToHold" );
	PROPERTY_MIN.Load( sType, "PropertyMin" );
	PROPERTY_CENTER.Load( sType, "PropertyCenter" );
	PROPERTY_MAX.Load( sType, "PropertyMax" );

	m_pPlayerState = pPlayerState;
	m_pNoteData = pNoteData;
}

void SoundEffectControl::SetSoundReader( RageSoundReader *pPlayer )
{
	m_pSoundReader = pPlayer;
}

void SoundEffectControl::Update( float fDeltaTime )
{
	if( SOUND_PROPERTY == "" )
		return;

	float fLevel = INPUTMAPPER->GetLevel( GAME_BUTTON_EFFECT_UP, m_pPlayerState->m_PlayerNumber );
	fLevel -= INPUTMAPPER->GetLevel( GAME_BUTTON_EFFECT_DOWN, m_pPlayerState->m_PlayerNumber );
	CLAMP( fLevel, -1.0f, +1.0f );

	if( LOCK_TO_HOLD )
	{
		int iRow = BeatToNoteRow( GAMESTATE->m_fSongBeat );
		int iHoldsHeld, iHoldsLetGo;
		HoldsBeingHeld( iRow, iHoldsHeld, iHoldsLetGo );

		/* If no holds are being held, or any have been missed, lock the effect off until
		 * the button has been released. */
		if( iHoldsLetGo > 0 || iHoldsHeld == 0 )
			m_bLocked = true;

		/* If the button is released, unlock it when the level crosses or reaches 0. */
		if( m_bLocked )
		{
			if( (fLevel <= 0.0f && m_fLastLevel >= 0.0f) ||
				(fLevel >= 0.0f && m_fLastLevel <= 0.0f) )
				m_bLocked = false;
		}
	}

	m_fLastLevel = fLevel;

	if( m_bLocked )
		fLevel = 0.0f;

	m_fSample = fLevel;
	m_pPlayerState->m_EffectHistory.AddSample( m_fSample, fDeltaTime );

	float fPropertyMin = PROPERTY_MIN;
	float fPropertyCenter = PROPERTY_CENTER;
	float fPropertyMax = PROPERTY_MAX;

	float fCurrent;
	if( m_fSample < 0 )
		fCurrent = SCALE( m_fSample, 0.0f, -1.0f, fPropertyCenter, fPropertyMin );
	else
		fCurrent = SCALE( m_fSample, 0.0f, +1.0f, fPropertyCenter, fPropertyMax );

	if( m_pSoundReader )
		m_pSoundReader->SetProperty( SOUND_PROPERTY, fCurrent );
}

/* Return false if any holds have been LetGo.  Otherwise, return true if at least
 * one hold is active. */
void SoundEffectControl::HoldsBeingHeld( int iRow, int &iHoldsHeld, int &iHoldsLetGo ) const
{
	iHoldsHeld = iHoldsLetGo = 0;
	for( int c=0; c < m_pNoteData->GetNumTracks(); ++c )
	{
		NoteData::TrackMap::const_iterator begin, end;
		m_pNoteData->GetTapNoteRangeInclusive( c, iRow, iRow+1, begin, end );
		if( begin == end )
			continue;

		const TapNote &tn = begin->second;
		if( tn.type != TapNote::hold_head )
			continue;
		if( tn.HoldResult.bActive )
			++iHoldsHeld;
		else if( tn.HoldResult.hns == HNS_LetGo )
			++iHoldsLetGo;
	}
}

/*
 * (c) 2006-2007 Glenn Maynard
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
