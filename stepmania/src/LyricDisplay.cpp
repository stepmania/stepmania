#include "global.h"
#include "LyricDisplay.h"

#include "GameState.h"
#include "ThemeManager.h"

#define IN_COMMAND		THEME->GetMetric("LyricDisplay","InCommand")
#define OUT_COMMAND		THEME->GetMetric("LyricDisplay","OutCommand")

float g_TweenInTime, g_TweenOutTime;

LyricDisplay::LyricDisplay()
{
	// We need to use the Color Tag that's in m_pCurSong->m_LyricSegments[?].m_sColor
	// But since the value there is in Hex, need to convert to RageColor I guess.
	// Until that gets done, this will default to white (&HFFFFFF)

	m_textLyrics.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
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
}

void LyricDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	// Make sure we don't go over the array's boundry
	if( m_iCurLyricNumber >= GAMESTATE->m_pCurSong->m_LyricSegments.size() )
		return;

	Song *song = GAMESTATE->m_pCurSong;
	const float fStartTime = (song->m_LyricSegments[m_iCurLyricNumber].m_fStartTime) - g_TweenInTime;

	if(GAMESTATE->m_fMusicSeconds < fStartTime)
		return;

	const float MaxDisplayTime = 10;

	float fShowLength = MaxDisplayTime;

	if(m_iCurLyricNumber+1 < GAMESTATE->m_pCurSong->m_LyricSegments.size())
	{
		/* Clamp this lyric to the beginning of the next. */
		const float Distance = 
			song->m_LyricSegments[m_iCurLyricNumber+1].m_fStartTime -
			song->m_LyricSegments[m_iCurLyricNumber].m_fStartTime;
		const float TweenBufferTime = g_TweenInTime + g_TweenOutTime;

		fShowLength = min(fShowLength, Distance - TweenBufferTime);
	}

	m_textLyrics.SetText( GAMESTATE->m_pCurSong->m_LyricSegments[m_iCurLyricNumber].m_sLyric );
	// m_textLyrics.SetDiffuse(COLOR);

	m_textLyrics.StopTweening();
	m_textLyrics.Command(IN_COMMAND);
	m_textLyrics.BeginTweening( fShowLength ); /* sleep */
	m_textLyrics.Command(OUT_COMMAND);

	m_iCurLyricNumber++;

		/*I figure for longer lines of text, the Lyric display object should
			be scaled down, if needed, by the .ScaleTo() function. But somehow
			it jus ain't working for me at all.. anyone able to do this
			properly?? We prolly should also add detection of where to put
			the Lyric object, if the arrows are on reverse? Jus an idea, 
			but it kinda defeats the purpose of the Lyric object X/Y being a
			theme element :)
	*/
}

/*
-----------------------------------------------------------------------------
 Copyright (c) 2003 by the person(s) listed below.  All rights reserved.
    Kevin Slaughter
	Glenn Maynard
-----------------------------------------------------------------------------
*/
