#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: MusicBannerWheel

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Andrew Livy
-----------------------------------------------------------------------------
*/

#include "MusicBannerWheel.h"
#include "RageUtil.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "ThemeManager.h"
#include "RageSoundManager.h"
#include "StyleDef.h"

#define BANNERSPACING 200
#define MAXSONGSINBUFFER 5
#define BANNERTYPE 1

enum
{
	GOINGLEFT=0,
	GOINGRIGHT
};

#define DEFAULT_SCROLL_DIRECTION		THEME->GetMetricI("Notes","DefaultScrollDirection")
#define PREVIEWMUSICMODE		THEME->GetMetricI("ScreenEz2SelectMusic","PreviewMusicMode")

MusicBannerWheel::MusicBannerWheel() 
{ 
	currentPos=0;
	scrlistPos=0;
	SongsExist=0;
	SingleLoad=0;

	if(DEFAULT_SCROLL_DIRECTION && GAMESTATE->m_pCurSong == NULL) /* check the song is null... incase they have just come back from a song and changed their PlayerOptions */
	{
		for(int i=0; i<NUM_PLAYERS; i++)
			GAMESTATE->m_PlayerOptions[i].m_bReverseScroll = true;
	}

	m_ScrollingList.UseSpriteType(BANNERTYPE);
	m_ScrollingList.SetXY( 0, 0 );
	m_ScrollingList.SetSpacing( BANNERSPACING );
	this->AddChild( &m_ScrollingList );

	if( 0 == stricmp(GAMESTATE->m_sPreferredGroup, "All Music") )
		SONGMAN->GetSongs( arraySongs, GAMESTATE->GetNumStagesLeft() );
	else // Get the Group They Want
		SONGMAN->GetSongs( arraySongs, GAMESTATE->m_sPreferredGroup, GAMESTATE->GetNumStagesLeft() );


	if( arraySongs.size() > 0)
	{
		SongsExist=1;
	}
	else
	{
		SongsExist=0;
		return;
	}


	// If there is no currently selected song, select one.
	if( GAMESTATE->m_pCurSong == NULL )
	{
		currentPos=0;
	}
	else // theres a song already selected (i.e. they came back from gameplay)...
	{
		// find our song and change the currentPos to wherever it may be.
		for(unsigned i=0; i<arraySongs.size(); i++)
		{
			if( GAMESTATE->m_pCurSong == arraySongs[i])
			{
				currentPos=i;
				i=(arraySongs.size()-1); // get us out of the loop by moving i ahead to the end
			}
		}
	}

	LoadSongData();

	#ifdef DEBUG
	m_debugtext.LoadFromFont( THEME->GetPathTo("Fonts","small titles") );
	m_debugtext.SetXY( 0, -120 );
	this->AddChild(&m_debugtext);
	#endif
}

/********************************************
void MusicBannerWheel::InsertNewBanner(int direction)

Based upon the direction specified it will go off and 
figure out which banner went out of focus, (which is
now our NEW! banner) and replace it with something new...
works only if there are 5 banners in the scrolling list.
********************************************/
void MusicBannerWheel::InsertNewBanner(int direction)
{
	Song* pSong = NULL;
	CString sGraphicPath;
	int elementtoreplace=0;	

	if(	direction == GOINGLEFT)
	{
		/* Imagine this:
			[1][2][3][4][5]
			then they shift <-
			[?][1][2][3][4]
			gotta find out what ? is

		   scrlistpos = 0   0 1 2 [3] [4] [0] [1] [2] 3 4 0 1 2 3 4
		   scrlistpos = 4   4 0 1 [2] [3] [4] [0] [1] 2 3 4 0 1 2 3 // we lost 2 and got a new one
		   scrlistpos = 3   3 4 0 [1] [2] [3] [4] [0] 1 2 3 4 0 1 2 // we lost 1 and got a new one
		 */
		if(currentPos-2 >= 0)
			pSong = arraySongs[currentPos-2];
		else
			pSong = arraySongs[(arraySongs.size()-1) + (currentPos-2)];	// wrap around. (it does honestly!)		

		if( pSong == NULL ) sGraphicPath = (THEME->GetPathTo("Graphics","fallback banner"));
		else if (pSong->HasBanner()) sGraphicPath = (pSong->GetBannerPath());
		else if (PREFSMAN->m_bUseBGIfNoBanner && pSong->HasBackground() ) sGraphicPath = (pSong->GetBannerPath());
		else sGraphicPath = (THEME->GetPathTo("Graphics","fallback banner"));

		elementtoreplace = scrlistPos - 2;
		if(elementtoreplace < 0) 
		{
			elementtoreplace = MAXSONGSINBUFFER + elementtoreplace;
		}
		m_ScrollingList.Replace(sGraphicPath, elementtoreplace);
	}
	else if( direction == GOINGRIGHT)
	{
		/* Imagine this:
			[1][2][3][4][5]
			then they shift ->
			[2][3][4][5][?]
			gotta find out what ? is
			scrlistpos = 0 1 2 [3] [4] [0] [1] [2] 3 4
			scrlistpos = 1 2 3 [4] [0] [1] [2] [3] 4 0 // we lost 3 and got a new one												
		 */
		if((unsigned)currentPos+2 <= arraySongs.size()-1)
			pSong = arraySongs[currentPos+2];
		else
			pSong = arraySongs[0+ (((currentPos+2) - (arraySongs.size()-1)) - 1)]; // wrap around. (it does honestly!)

		if( pSong == NULL ) sGraphicPath = (THEME->GetPathTo("Graphics","fallback banner"));
		else if (pSong->HasBanner()) sGraphicPath = (pSong->GetBannerPath());
		else if (PREFSMAN->m_bUseBGIfNoBanner && pSong->HasBackground() ) sGraphicPath = (pSong->GetBannerPath());
		else sGraphicPath = (THEME->GetPathTo("Graphics","fallback banner"));

		elementtoreplace = scrlistPos + 2;
		if(elementtoreplace > MAXSONGSINBUFFER-1) 
		{
			elementtoreplace = elementtoreplace - MAXSONGSINBUFFER;
		}
		m_ScrollingList.Replace(sGraphicPath, elementtoreplace);
	}
	else
	{
		ASSERT(0); // we should be going in some sort of direction.
	}
	if(PREVIEWMUSICMODE == 0)
		PlayMusicSample();
}

/****************************
void MusicBannerWheel::LoadSongData()

Loads the Song Data, based upon
its relative position. It's fairly
slow, so should only get used once
and once only.
*****************************/
void MusicBannerWheel::LoadSongData()
{
	Song* pSong = NULL;
	CStringArray asGraphicPaths;

	if(MAXSONGSINBUFFER >= arraySongs.size() && SingleLoad != 1)  // less than the MAXSONGSINBUFFER means we can get away with loading the lot in one go
	{
		SingleLoad=2;
		int difference=0;
		// find out just how short of a full buffer we are =)
		difference = MAXSONGSINBUFFER - arraySongs.size();
		for(int i=0; i<=difference; i++)
		{
			for(unsigned c=0; c<arraySongs.size(); c++)
			{
				pSong = arraySongs[c];

				if( pSong == NULL ) asGraphicPaths.push_back(THEME->GetPathTo("Graphics","fallback banner"));
				else if (pSong->HasBanner()) asGraphicPaths.push_back(pSong->GetBannerPath());
				else if (PREFSMAN->m_bUseBGIfNoBanner && pSong->HasBackground() ) asGraphicPaths.push_back(pSong->GetBannerPath());
				else asGraphicPaths.push_back(THEME->GetPathTo("Graphics","fallback banner"));	
			}
		}
	}
	
	if(SingleLoad==0)
	{
		for(int count=0; count<MAXSONGSINBUFFER; count++)
		{
			/* In essence, this element is the central one so we want the scrolling list
			to load in the song as specified by currentPos */
			if(count == scrlistPos)
			{	
				pSong = arraySongs[currentPos];
			}
			/* if it's the next element
			*/
			else if(count == scrlistPos+2 || (scrlistPos == MAXSONGSINBUFFER-2 && count == 0))
			{
				if((unsigned)currentPos+2 <= arraySongs.size()-1)
					pSong = arraySongs[currentPos+2];
				else
					pSong = arraySongs[0+2];
			}
			else if(count == scrlistPos-2 || (scrlistPos == 0 && count == MAXSONGSINBUFFER-2))
			{
				if(currentPos-2 >= 0)
					pSong = arraySongs[currentPos-2];
				else
					pSong = arraySongs[arraySongs.size()-2];			
			}
			else if(count == scrlistPos+1 || (scrlistPos == MAXSONGSINBUFFER-1 && count == 0))
			{
				if((unsigned)currentPos+1 <= arraySongs.size()-1)
					pSong = arraySongs[currentPos+1];
				else
					pSong = arraySongs[0];
			}
			/* if it's the previous element OR if we're at element 0..the 5th element (which will wrap to before element 0)
			will actually appear as 5 songs along and not the one immediately before it,, so pull a sneaky and make the 
			final element actually be the song before... */
			else if(count == scrlistPos-1 || (scrlistPos == 0 && count == MAXSONGSINBUFFER-1))
			{
				if(currentPos-1 >= 0)
					pSong = arraySongs[currentPos-1];
				else
					pSong = arraySongs[arraySongs.size()-1];
			}

			if( pSong == NULL ) asGraphicPaths.push_back(THEME->GetPathTo("Graphics","fallback banner"));
			else if (pSong->HasBanner()) asGraphicPaths.push_back(pSong->GetBannerPath());
			else if (PREFSMAN->m_bUseBGIfNoBanner && pSong->HasBackground() ) asGraphicPaths.push_back(pSong->GetBannerPath());
			else asGraphicPaths.push_back(THEME->GetPathTo("Graphics","fallback banner"));
		}
	}
	if(SingleLoad != 1)
		m_ScrollingList.Load( asGraphicPaths );
	if(SingleLoad == 2)
		SingleLoad = 1;
	if(PREVIEWMUSICMODE == 0)
		PlayMusicSample();
}


Song* MusicBannerWheel::GetSelectedSong()
{
	Song* pSong;
	pSong=arraySongs[currentPos];
	return(pSong);
}

void MusicBannerWheel::StartBouncing()
{
	m_ScrollingList.StartBouncing();
}

void MusicBannerWheel::StopBouncing()
{
	m_ScrollingList.StopBouncing();
}

void MusicBannerWheel::PlayMusicSample()
{
	Song* pSong = GetSelectedSong();
	if( pSong  &&  pSong->HasMusic() )
	{
		SOUNDMAN->PlayMusic(pSong->GetMusicPath(), true,
			pSong->m_fMusicSampleStartSeconds,
			pSong->m_fMusicSampleLengthSeconds);
	}
}

void MusicBannerWheel::BannersLeft()
{
	StopBouncing();
	if(currentPos==0)
		currentPos=arraySongs.size()-1;
	else
		currentPos--;

	if(scrlistPos==0)
		if(SingleLoad == 0)
			scrlistPos = MAXSONGSINBUFFER-1;
		else
			scrlistPos = arraySongs.size()-1;
	else
		scrlistPos--;

	if(SingleLoad == 0)
	//	m_ScrollingList.Unload();
	// LoadSongData();
	InsertNewBanner(GOINGLEFT);
	#ifdef DEBUG
		m_debugtext.SetText(ssprintf("currentPos: %d scrlistPos: %d",currentPos,scrlistPos));
	#endif
	m_ScrollingList.Left();
	ChangeNotes();
}

void MusicBannerWheel::BannersRight()
{
	StopBouncing();
	if((unsigned)currentPos>=arraySongs.size()-1)
		currentPos=0;
	else
		currentPos++;

	if(scrlistPos==MAXSONGSINBUFFER-1)
		scrlistPos=0;
	else
		scrlistPos++;

	if(SingleLoad == 0)
	//	m_ScrollingList.Unload();
	//	LoadSongData();
	InsertNewBanner(GOINGRIGHT);
	#ifdef DEBUG
		m_debugtext.SetText(ssprintf("currentPos: %d scrlistPos: %d",currentPos,scrlistPos));
	#endif
	m_ScrollingList.Right();
	ChangeNotes();
}

void MusicBannerWheel::ChangeNotes()
{
}


MusicBannerWheel::~MusicBannerWheel()
{
}
