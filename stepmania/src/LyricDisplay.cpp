#include "global.h"
#include "LyricDisplay.h"

#include "GameState.h"
#include "ThemeManager.h"

#define IN_COMMAND		THEME->GetMetric("LyricDisplay","InCommand")
#define OUT_COMMAND		THEME->GetMetric("LyricDisplay","OutCommand")

float g_TweenInTime, g_TweenOutTime;

LyricDisplay::LyricDisplay()
{
	m_textLyrics.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textLyrics.SetDiffuse( RageColor(1,1,1,1) );
	this->AddChild(&m_textLyrics);

	Init();
}

void LyricDisplay::Init()
{
	m_textLyrics.SetText("");
	m_iCurLyricNumber = 0;

	/* Update global cache: */
	g_TweenInTime = Actor::GetCommandLength(IN_COMMAND);
	g_TweenOutTime = Actor::GetCommandLength(OUT_COMMAND);
	m_fLastSecond = -500;
}

void LyricDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	/* If the song has changed (in a course), reset. */
	if( GAMESTATE->m_fMusicSeconds < m_fLastSecond )
		Init();
	m_fLastSecond = GAMESTATE->m_fMusicSeconds;

	// Make sure we don't go over the array's boundry
	if( m_iCurLyricNumber >= GAMESTATE->m_pCurSong->m_LyricSegments.size() )
		return;

	Song *song = GAMESTATE->m_pCurSong;
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
	fShowLength = min(fShowLength, 5);

	/* If it's negative, two lyrics are so close together that there's no time
	 * to tween properly.  Lyrics should never be this brief, anyway, so just
	 * skip it. */
	fShowLength = max(fShowLength, 0);

	m_textLyrics.SetText( GAMESTATE->m_pCurSong->m_LyricSegments[m_iCurLyricNumber].m_sLyric );

	/*
	 * This really needs a way to define a custom theme command here, so themes
	 * can do things like:
	 * 
	 * "Diffuse=1,1,1,0;linear,.2;Diffuse=1,1,1,1;linear,.2;LyricDiffuse"
	 */

	float fZoom = 1.0f;
	fZoom = min(fZoom, float(SCREEN_WIDTH)/(m_textLyrics.GetWidestLineWidthInSourcePixels()+1) );

	m_textLyrics.StopTweening();
	m_textLyrics.SetZoomX(fZoom);

	m_textLyrics.SetDiffuse(GAMESTATE->m_pCurSong->m_LyricSegments[m_iCurLyricNumber].m_Color);
	m_textLyrics.Command(IN_COMMAND);
	m_textLyrics.BeginTweening( fShowLength ); /* sleep */
	m_textLyrics.Command(OUT_COMMAND);

	m_iCurLyricNumber++;
}

/*
-----------------------------------------------------------------------------
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
    Kevin Slaughter
	Glenn Maynard
-----------------------------------------------------------------------------
*/
