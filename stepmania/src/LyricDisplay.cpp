#include "global.h"
#include "LyricDisplay.h"
#include "ScreenDimensions.h"
#include "GameState.h"
#include "ThemeMetric.h"
#include "song.h"
#include "ActorUtil.h"

static ThemeMetric<float>		IN_LENGTH	("LyricDisplay","InLength");
static ThemeMetric<float>		OUT_LENGTH	("LyricDisplay","OutLength");

LyricDisplay::LyricDisplay()
{
	m_textLyrics[0].SetName( "LyricBack" );
	ActorUtil::LoadAllCommands( m_textLyrics[0], "LyricDisplay" );
	m_textLyrics[0].LoadFromFont( THEME->GetPathF("LyricDisplay","text") );
	this->AddChild( &m_textLyrics[0] );

	m_textLyrics[1].SetName( "LyricFront" );
	ActorUtil::LoadAllCommands( m_textLyrics[1], "LyricDisplay" );
	m_textLyrics[1].LoadFromFont( THEME->GetPathF("LyricDisplay","text") );
	this->AddChild( &m_textLyrics[1] );

	Init();
}

void LyricDisplay::Init()
{
	for( int i=0; i<2; i++ )
		m_textLyrics[i].SetText("");
	m_iCurLyricNumber = 0;

	m_fLastSecond = -500;
}

void LyricDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( GAMESTATE->m_pCurSong == NULL )
		return;

	/* If the song has changed (in a course), reset. */
	if( GAMESTATE->m_fMusicSeconds < m_fLastSecond )
		Init();
	m_fLastSecond = GAMESTATE->m_fMusicSeconds;

	if( m_iCurLyricNumber >= GAMESTATE->m_pCurSong->m_LyricSegments.size() )
		return;

	const Song *pSong = GAMESTATE->m_pCurSong;
	const float fStartTime = (pSong->m_LyricSegments[m_iCurLyricNumber].m_fStartTime) - IN_LENGTH.GetValue();

	if( GAMESTATE->m_fMusicSeconds < fStartTime )
		return;

	/* Clamp this lyric to the beginning of the next or the end of the music. */
	float fEndTime;
	if( m_iCurLyricNumber+1 < GAMESTATE->m_pCurSong->m_LyricSegments.size() )
		fEndTime = pSong->m_LyricSegments[m_iCurLyricNumber+1].m_fStartTime;
	else
		fEndTime = pSong->GetElapsedTimeFromBeat( pSong->m_fLastBeat );

	const float fDistance = fEndTime - pSong->m_LyricSegments[m_iCurLyricNumber].m_fStartTime;
	const float fTweenBufferTime = IN_LENGTH.GetValue() + OUT_LENGTH.GetValue();

	/* If it's negative, two lyrics are so close together that there's no time
	 * to tween properly.  Lyrics should never be this brief, anyway, so just
	 * skip it. */
	float fShowLength = max( fDistance - fTweenBufferTime, 0.0f );

	// Make lyrics show faster for faster song rates.
	fShowLength /= GAMESTATE->m_SongOptions.GetCurrent().m_fMusicRate;

	const LyricSegment &seg = GAMESTATE->m_pCurSong->m_LyricSegments[m_iCurLyricNumber];

	LuaThreadVariable var1( "LyricText", seg.m_sLyric );
	LuaThreadVariable var2( "LyricDuration", LuaReference::Create(fShowLength) );
	LuaThreadVariable var3( "LyricColor", LuaReference::Create(seg.m_Color) );

	PlayCommand( "Changed" );

	m_iCurLyricNumber++;
}

/*
 * (c) 2003-2004 Kevin Slaughter, Glenn Maynard
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
