#include "global.h"
#include "NoteFieldPositioning.h"

#include "RageDisplay.h"
#include "RageLog.h"
#include "GameState.h"
#include "Style.h"
#include "PlayerState.h"

void NoteFieldMode::BeginDrawTrack( const PlayerState* pPlayerState, int iTrack )
{
	DISPLAY->CameraPushMatrix();

	ASSERT( iTrack != -1 );

	DISPLAY->PushMatrix();

	const Style *s = GAMESTATE->GetCurrentStyle();
	// TODO: Remove indexing by PlayerNumber.
	float fPixelXOffsetFromCenter = s->m_ColumnInfo[pPlayerState->m_PlayerNumber][iTrack].fXOffset;

	/* Allow Mini to pull tracks together, but not to push them apart. */
	const float* fEffects = pPlayerState->m_CurrentPlayerOptions.m_fEffects;
	float fMiniPercent = fEffects[PlayerOptions::EFFECT_MINI];
	fMiniPercent = min( powf(0.5f, fMiniPercent), 1.0f );
	fPixelXOffsetFromCenter *= fMiniPercent;

	DISPLAY->Translate( fPixelXOffsetFromCenter, 0, 0 );
}

void NoteFieldMode::EndDrawTrack( int iTrack )
{
	ASSERT( iTrack != -1 );

	DISPLAY->PopMatrix();
	DISPLAY->CameraPopMatrix();
}


/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
