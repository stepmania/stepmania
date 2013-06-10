/* SoundEffectControl - Control a sound property through user input. */

#ifndef SOUND_EFFECT_CONTROL_H
#define SOUND_EFFECT_CONTROL_H

#include "ThemeMetric.h"

class RageSoundReader;
class PlayerState;
class NoteData;

class SoundEffectControl
{
public:
	SoundEffectControl();
	void Load( const RString &sType, PlayerState *pPlayerState, const NoteData *pNoteData );

	void SetSoundReader( RageSoundReader *pPlayer );
	void ReleaseSound() { SetSoundReader(NULL); }

	void Update( float fDeltaTime );

private:
	void HoldsBeingHeld( int iRow, int &iHoldsHeld, int &iHoldsLetGo ) const;

	ThemeMetric<RString>	SOUND_PROPERTY;
	ThemeMetric<bool>	LOCK_TO_HOLD;
	ThemeMetric<float>	PROPERTY_MIN;
	ThemeMetric<float>	PROPERTY_CENTER;
	ThemeMetric<float>	PROPERTY_MAX;

	PlayerState *m_pPlayerState;

	bool m_bLocked;

	float m_fSample;
	float m_fLastLevel;

	const NoteData *m_pNoteData;
	RageSoundReader *m_pSoundReader;
};

#endif

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
