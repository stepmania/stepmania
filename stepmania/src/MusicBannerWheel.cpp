#include "global.h"

/*
TODO:
1. FIX TIMER
2. MAKE 2PLAYER SUPPORT WORK CORRECTLY
3. FIX THE MODE SWITCHER FOR EXTRAMIX FOR 2 PLAYER
3a. MAKE IT METRICABLE
*/


#include "MusicBannerWheel.h"
#include "RageUtil.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "SongManager.h"
#include "ThemeManager.h"
#include "GameSoundManager.h"
#include "Style.h"
#include "song.h"
#include "Steps.h"
#include "ActorUtil.h"

#define BANNERSPACING THEME->GetMetricI("ScreenEz2SelectMusic","BannerSpacing")
#define MAXSONGSINBUFFER 5
#define BANNERTYPE 1

enum
{
	GOINGLEFT=0,
	GOINGRIGHT
};


#define PREVIEWMUSICMODE			THEME->GetMetricI("ScreenEz2SelectMusic","PreviewMusicMode")

MusicBannerWheel::MusicBannerWheel() 
{ 
	currentPos=0;
	scrlistPos=0;
	SongsExist=0;
	SingleLoad=0;
	bScanning = false;

	// Use pref DefaultModifiers to acomplish this
//	if(DEFAULT_SCROLL_DIRECTION && GAMESTATE->m_pCurSong == NULL) /* check the song is null... incase they have just come back from a song and changed their PlayerOptions */
//	{
//		for(int i=0; i<NUM_PLAYERS; i++)
//			GAMESTATE->m_PlayerOptions[i].m_fReverseScroll = 1;
//	}

	m_ScrollingList.UseSpriteType(BANNERTYPE);
	m_ScrollingList.SetXY( 0, 0 );
	m_ScrollingList.SetSpacing( BANNERSPACING );
	this->AddChild( &m_ScrollingList );

	if( GAMESTATE->m_sPreferredSongGroup == GROUP_ALL )
		SONGMAN->GetSongs( arraySongs, GAMESTATE->GetNumStagesLeft() );
	else // Get the Group They Want
		SONGMAN->GetSongs( arraySongs, GAMESTATE->m_sPreferredSongGroup, GAMESTATE->GetNumStagesLeft() );

	//Detect autogenned songs		
	if ( !PREFSMAN->m_bAutogenSteps )
	{
		LOG->Trace( "Removing all autogen songs from wheel." );
		vector <Song *> pNotAutogen;
		for ( unsigned i = 0; i < arraySongs.size(); i++)
		{
			//ONLY get non-autogenned steps
			Steps* pSteps = arraySongs[i]->GetStepsByDifficulty( GAMESTATE->GetCurrentStyle()->m_StepsType, DIFFICULTY_INVALID, false );
			if ( pSteps != NULL )
				pNotAutogen.push_back( arraySongs[i] );
		}
		arraySongs.clear();
		arraySongs = pNotAutogen;
	}

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
	m_debugtext.LoadFromFont( THEME->GetPathF("small","titles") );
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

		if( pSong == NULL ) 
			sGraphicPath = THEME->GetPathG("Common","fallback banner");
		else if (pSong->HasBanner()) 
			sGraphicPath = pSong->GetBannerPath();
		else 
			sGraphicPath = THEME->GetPathG("Common","fallback banner");

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

		if( pSong == NULL ) 
			sGraphicPath = THEME->GetPathG("Common","fallback banner");
		else if (pSong->HasBanner()) 
			sGraphicPath = pSong->GetBannerPath();
		else 
			sGraphicPath = THEME->GetPathG("Common","fallback banner");

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
	if((PREVIEWMUSICMODE == 0 || PREVIEWMUSICMODE == 3) && !bScanning)
		PlayMusicSample();
}

void MusicBannerWheel::SetScanMode(bool Scanmode)
{
	bScanning = Scanmode;
	if((PREVIEWMUSICMODE == 0 || PREVIEWMUSICMODE == 3) && !Scanmode)
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
	vector<CString> asGraphicPaths;

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

				if( pSong == NULL ) 
					asGraphicPaths.push_back(THEME->GetPathG("Common","fallback banner"));
				else if (pSong->HasBanner()) 
					asGraphicPaths.push_back(pSong->GetBannerPath());
				else 
					asGraphicPaths.push_back(THEME->GetPathG("Common","fallback banner"));	
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

			if( pSong == NULL ) 
				asGraphicPaths.push_back(THEME->GetPathG("Common","fallback banner"));
			else if (pSong->HasBanner()) 
				asGraphicPaths.push_back(pSong->GetBannerPath());
			else 
				asGraphicPaths.push_back(THEME->GetPathG("Common","fallback banner"));
		}
	}
	if(SingleLoad != 1)
		m_ScrollingList.Load( asGraphicPaths );
	if(SingleLoad == 2)
		SingleLoad = 1;
	if((PREVIEWMUSICMODE == 0 || PREVIEWMUSICMODE == 3) && !bScanning)
		PlayMusicSample();
}


void MusicBannerWheel::ScanToNextGroup()
{
	if(arraySongs.size() < 2)
		return; // forget it -- no point groupscanning
	int localPos = currentPos;
	int startingPos = localPos;
	Song* pSong;
	pSong = arraySongs[currentPos];
	CString currentGroupName = pSong->m_sGroupName;
	localPos++;
	while(localPos != startingPos)
	{
		if(localPos >= (int) arraySongs.size()-1)
		{
			localPos = 0;
		}
		pSong = arraySongs[localPos];
		if(currentGroupName != pSong->m_sGroupName)
		{
			break;
		}
		localPos++;
	}
	int iCount = 5;
	currentPos = localPos-iCount;
	while(iCount > 0)
	{
		BannersRight();
		iCount--;
	}
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
		if(SOUND->GetMusicPath().CompareNoCase(pSong->GetMusicPath())) // dont play the same sound over and over
		{
			
			SOUND->StopMusic();
			SOUND->PlayMusic(
				pSong->GetMusicPath(), 
				NULL,
				true,
				pSong->m_fMusicSampleStartSeconds,
				pSong->m_fMusicSampleLengthSeconds );
		}
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

/*
 * (c) 2003 "Frieza"
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
