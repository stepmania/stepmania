#include "global.h"
#include "LyricDisplay.h"

#include "GameState.h"
#include "ThemeManager.h"

#define IN_COMMAND		THEME->GetMetric("LyricDisplay","InCommand")
#define OUT_COMMAND		THEME->GetMetric("LyricDisplay","OutCommand")
#define WIPE_DIM_FACTOR	THEME->GetMetricC("LyricDisplay","WipeDimFactor")

float g_TweenInTime, g_TweenOutTime;

LyricDisplay::LyricDisplay()
{
	for( int i=0; i<2; i++ )
	{
		m_textLyrics[i].LoadFromFont( THEME->GetPathToF("LyricDisplay text") );
		m_textLyrics[i].SetDiffuse( RageColor(1,1,1,1) );
		this->AddChild(&m_textLyrics[i]);
	}

	Init();
}

void LyricDisplay::Init()
{
	for( int i=0; i<2; i++ )
		m_textLyrics[i].SetText("");
	m_iCurLyricNumber = 0;

	/* Update global cache: */
	g_TweenInTime = Actor::GetCommandLength(IN_COMMAND);
	g_TweenOutTime = Actor::GetCommandLength(OUT_COMMAND);
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

	// Make sure we don't go over the array's boundry
	if( m_iCurLyricNumber >= GAMESTATE->m_pCurSong->m_LyricSegments.size() )
		return;

	const Song *song = GAMESTATE->m_pCurSong;
	const float fStartTime = (song->m_LyricSegments[m_iCurLyricNumber].m_fStartTime) - g_TweenInTime;

	if(GAMESTATE->m_fMusicSeconds < fStartTime)
		return;

	const float MaxDisplayTime = 10;

	float fShowLength = MaxDisplayTime;

	/* Clamp this lyric to the beginning of the next, the end of the music,
	 * or 5 seconds. */
	float EndTime;
	if(m_iCurLyricNumber+1 < GAMESTATE->m_pCurSong->m_LyricSegments.size())
		EndTime = song->m_LyricSegments[m_iCurLyricNumber+1].m_fStartTime;
	else
		EndTime = song->GetElapsedTimeFromBeat( song->m_fLastBeat );

	const float Distance = EndTime - song->m_LyricSegments[m_iCurLyricNumber].m_fStartTime;
	const float TweenBufferTime = g_TweenInTime + g_TweenOutTime;

	fShowLength = min(fShowLength, Distance - TweenBufferTime);
	fShowLength = min(fShowLength, 3);

	/* If it's negative, two lyrics are so close together that there's no time
	 * to tween properly.  Lyrics should never be this brief, anyway, so just
	 * skip it. */
	fShowLength = max(fShowLength, 0);

	// make lyrics show faster a larger song rates.
	fShowLength /= GAMESTATE->m_SongOptions.m_fMusicRate;

	for( int i=0; i<2; i++ )
	{
		m_textLyrics[i].SetText( GAMESTATE->m_pCurSong->m_LyricSegments[m_iCurLyricNumber].m_sLyric );

		/*
		 * This really needs a way to define a custom theme command here, so themes
		 * can do things like:
		 * 
		 * "Diffuse=1,1,1,0;linear,.2;Diffuse=1,1,1,1;linear,.2;LyricDiffuse"
		 */

		const float fZoom = min( 1.0f, float(SCREEN_WIDTH)/(m_textLyrics[i].GetZoomedWidth()+1) );

		m_textLyrics[i].StopTweening();
		m_textLyrics[i].SetZoomX(fZoom);

		RageColor color = GAMESTATE->m_pCurSong->m_LyricSegments[m_iCurLyricNumber].m_Color;
		
		if( i==0 )
			color *= WIPE_DIM_FACTOR;

		m_textLyrics[i].SetDiffuse(color);

		/* Crop the bottom layer of text away as we crop the top layer on.  That
		 * prevents overdraw, which reduces AA quality. */
		if( i==0 )
			m_textLyrics[i].SetCropLeft(0);
		if( i==1 )
			m_textLyrics[i].SetCropRight(1);
		m_textLyrics[i].Command(IN_COMMAND);
		m_textLyrics[i].BeginTweening( fShowLength * 0.75f ); /* sleep */
		if( i==0 )
			m_textLyrics[i].SetCropLeft(1);
		if( i==1 )
			m_textLyrics[i].SetCropRight(0);
		m_textLyrics[i].BeginTweening( fShowLength * 0.25f ); /* sleep */
		m_textLyrics[i].Command(OUT_COMMAND);
	}

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
