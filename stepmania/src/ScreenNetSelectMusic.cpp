#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenNetSelectMusic.h"
#include "ScreenManager.h"
#include "GameSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "Style.h"
#include "Steps.h"
#include "RageTimer.h"
#include "ActorUtil.h"
#include "Actor.h"
#include "AnnouncerManager.h"
#include "MenuTimer.h"
#include "NetworkSyncManager.h"
#include "StepsUtil.h"
#include "RageUtil.h"

#define GROUPSBG_WIDTH				THEME->GetMetricF(m_sName,"GroupsBGWidth")
#define GROUPSBG_HEIGHT				THEME->GetMetricF(m_sName,"GroupsBGHeight")
#define GROUPSBG_COLOR				THEME->GetMetricC(m_sName,"GroupsBGColor")

#define SONGSBG_WIDTH				THEME->GetMetricF(m_sName,"SongsBGWidth")
#define SONGSBG_HEIGHT				THEME->GetMetricF(m_sName,"SongsBGHeight")
#define SONGSBG_COLOR				THEME->GetMetricC(m_sName,"SongsBGColor")

#define DIFFBG_WIDTH				THEME->GetMetricF(m_sName,"DiffBGWidth")
#define DIFFBG_HEIGHT				THEME->GetMetricF(m_sName,"DiffBGHeight")
#define DIFFBG_COLOR				THEME->GetMetricC(m_sName,"DiffBGColor")

#define EXINFOBG_WIDTH				THEME->GetMetricF(m_sName,"ExInfoBGWidth")
#define EXINFOBG_HEIGHT				THEME->GetMetricF(m_sName,"ExInfoBGHeight")
#define EXINFOBG_COLOR				THEME->GetMetricC(m_sName,"ExInfoBGColor")

#define	NUM_GROUPS_SHOW				THEME->GetMetricI(m_sName,"NumGroupsShow");
#define	NUM_SONGS_SHOW				THEME->GetMetricI(m_sName,"NumSongsShow");

#define SEL_WIDTH					THEME->GetMetricF(m_sName,"SelWidth")
#define SEL_HEIGHT					THEME->GetMetricF(m_sName,"SelHeight")
#define SEL_COLOR					THEME->GetMetricC(m_sName,"SelColor")

#define SUBTITLE_WIDTH				THEME->GetMetricF(m_sName,"SongsSubtitleWidth")
#define ARTIST_WIDTH				THEME->GetMetricF(m_sName,"SongsArtistWidth")
#define GROUP_WIDTH					THEME->GetMetricF(m_sName,"SongsGroupWidth")

const ScreenMessage SM_NoSongs		= ScreenMessage(SM_User+3);
const ScreenMessage SM_ChangeSong	= ScreenMessage(SM_User+5);
const ScreenMessage SM_BackFromOpts	= ScreenMessage(SM_User+6);
const ScreenMessage SM_SMOnlinePack	= ScreenMessage(SM_User+8);	//Unused, but should be known

const CString AllGroups			= "[ALL MUSIC]";

ScreenNetSelectMusic::ScreenNetSelectMusic( const CString& sName ) : ScreenNetSelectBase( sName )
{
	/* Finish any previous stage.  It's OK to call this when we havn't played a stage yet. */
	GAMESTATE->FinishStage();

	//Groups
	m_rectGroupsBackground.SetDiffuse( GROUPSBG_COLOR );
	m_rectGroupsBackground.SetName( "GroupsBG" );
	m_rectGroupsBackground.SetWidth( GROUPSBG_WIDTH );
	m_rectGroupsBackground.SetHeight( GROUPSBG_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_rectGroupsBackground );
	this->AddChild( &m_rectGroupsBackground );

	m_textGroups.LoadFromFont( THEME->GetPathF(m_sName,"wheel") );
	m_textGroups.SetShadowLength( 0 );
	m_textGroups.SetName( "GroupsList" );
	m_textGroups.SetMaxWidth( GROUPSBG_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textGroups );
	this->AddChild( &m_textGroups);

	//Songs
	m_rectSongsBackground.SetDiffuse( SONGSBG_COLOR );
	m_rectSongsBackground.SetName( "SongsBackground" );
	m_rectSongsBackground.SetWidth( SONGSBG_WIDTH );
	m_rectSongsBackground.SetHeight( SONGSBG_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_rectSongsBackground );
	this->AddChild( &m_rectSongsBackground );

	m_textSongs.LoadFromFont( THEME->GetPathF(m_sName,"wheel") );
	m_textSongs.SetShadowLength( 0 );
	m_textSongs.SetName( "SongsList" );
	m_textSongs.SetMaxWidth( SONGSBG_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textSongs );
	this->AddChild( &m_textSongs);

	m_rectExInfo.SetDiffuse( EXINFOBG_COLOR );
	m_rectExInfo.SetName( "ExInfoBG" );
	m_rectExInfo.SetWidth( EXINFOBG_WIDTH );
	m_rectExInfo.SetHeight( EXINFOBG_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_rectExInfo );
	this->AddChild( &m_rectExInfo );

	m_textArtist.LoadFromFont( THEME->GetPathF(m_sName,"song") );
	m_textArtist.SetShadowLength( 0 );
	m_textArtist.SetName( "SongsArtist" );
	m_textArtist.SetMaxWidth( ARTIST_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textArtist );
	this->AddChild( &m_textArtist);

	m_textGroup.LoadFromFont( THEME->GetPathF(m_sName,"song") );
	m_textGroup.SetShadowLength( 0 );
	m_textGroup.SetName( "SongsGroup" );
	m_textGroup.SetMaxWidth( GROUP_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textGroup );
	this->AddChild( &m_textGroup );

	m_textSubtitle.LoadFromFont( THEME->GetPathF(m_sName,"song") );
	m_textSubtitle.SetShadowLength( 0 );
	m_textSubtitle.SetName( "SongsSubtitle" );
	m_textSubtitle.SetMaxWidth( SUBTITLE_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textSubtitle );
	this->AddChild( &m_textSubtitle );

	//SelectOptions Sprite
	m_sprSelOptions.Load( THEME->GetPathG( m_sName, "options" ) );
	m_sprSelOptions.SetName( "options" );
	SET_XY_AND_ON_COMMAND( m_sprSelOptions );
	this->AddChild( &m_sprSelOptions );

	//Diff Icon background
	m_rectDiff.SetDiffuse( DIFFBG_COLOR );
	m_rectDiff.SetName( "DiffBG" );
	m_rectDiff.SetWidth( DIFFBG_WIDTH );
	m_rectDiff.SetHeight( DIFFBG_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_rectDiff );
	this->AddChild( &m_rectDiff );

	FOREACH_EnabledPlayer (p)
	{
		m_DifficultyIcon[p].SetName( ssprintf("DifficultyIconP%d",p+1) );
		m_DifficultyIcon[p].Load( THEME->GetPathG( "ScreenSelectMusic",
												   ssprintf("difficulty icons 1x%d",
															NUM_DIFFICULTIES)) );
		SET_XY( m_DifficultyIcon[p] );
		this->AddChild( &m_DifficultyIcon[p] );
		ON_COMMAND( m_DifficultyIcon[p] );
		m_DC[p] = GAMESTATE->m_PreferredDifficulty[p];
		m_DifficultyIcon[p].SetFromDifficulty( p, m_DC[p] );

		m_DifficultyMeters[p].SetName( "DifficultyMeter", ssprintf("MeterP%d",p+1) );
		m_DifficultyMeters[p].Load();
		SET_XY_AND_ON_COMMAND( m_DifficultyMeters[p] );
		this->AddChild( &m_DifficultyMeters[p] );
	}

	m_SelectMode = SelectGroup;
	m_rectSelection.SetDiffuse( SEL_COLOR );
	m_rectSelection.SetName( "Sel" );
	m_rectSelection.SetWidth( SEL_WIDTH );
	m_rectSelection.SetHeight( SEL_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_rectSelection );
	this->AddChild( &m_rectSelection );


	SONGMAN->GetGroupNames( m_vGroups );
	if (m_vGroups.size()<1)
	{
		SCREENMAN->SendMessageToTopScreen( SM_NoSongs );
		return;
	}
	
	//Make the last group the full list group.
	//Must be last
	m_vGroups.push_back( AllGroups );
	m_iShowGroups = NUM_GROUPS_SHOW;
	m_iShowSongs = NUM_SONGS_SHOW;
	m_iGroupNum=m_vGroups.size()-1;	//Alphabetical

	UpdateGroupsListPos();
	UpdateSongsList();
	
	if (GAMESTATE->m_pCurSong != NULL)
		for ( unsigned i = 0 ; i<m_vSongs.size() ; ++i )
			if (m_vSongs[i]->GetFullDisplayTitle() == GAMESTATE->m_pCurSong->GetFullDisplayTitle())
				m_iSongNum = i;

	UpdateSongsListPos();

	//Load SFX next
	m_soundChangeOpt.Load( THEME->GetPathToS( m_sName + " change opt" ) );
	m_soundChangeSel.Load( THEME->GetPathToS( m_sName + " change sel" ) );

	NSMAN->ReportNSSOnOff(1);
	NSMAN->ReportPlayerOptions();

	return;
}

void ScreenNetSelectMusic::Input( const DeviceInput& DeviceI, const InputEventType type,
								  const GameInput& GameI, const MenuInput& MenuI,
								  const StyleInput& StyleI )
{
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return;

	if( (type != IET_FIRST_PRESS) && (type != IET_SLOW_REPEAT) && (type != IET_FAST_REPEAT ) )
		return;

	bool bHoldingCtrl = 
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_LCTRL)) ||
		INPUTFILTER->IsBeingPressed(DeviceInput(DEVICE_KEYBOARD, KEY_RCTRL)) ||
		(!NSMAN->useSMserver);	//If we are disconnected, assume no chatting

	switch( DeviceI.button )
	{
	case KEY_ENTER:
	case KEY_KP_ENTER:
	case KEY_BACK:
		break;
	default:
		char c;
		c = DeviceI.ToChar();

		//Search list for given letter (to aide in searching)
		if( bHoldingCtrl )
		{
			c = (char)toupper(c);
			for (int i=0; i<(int)m_vSongs.size(); ++i)
				if ( (char) toupper(m_vSongs[i]->GetTranslitMainTitle().c_str()[0]) == (char) c )
				{
					m_iSongNum = i;
					UpdateSongsListPos();
					break;
				}
		}
		break;
	}

	ScreenNetSelectBase::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenNetSelectMusic::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "PrevScreen") );
		break;
	case SM_GoToNextScreen:
		SOUND->StopMusic();
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "NextScreen") );
		break;
	case SM_NoSongs:
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "NoSongsScreen") );
		break;
	case SM_ChangeSong:
		{
			//First check to see if this song is already selected.
			//This is so that if you multiple copies of the "same" song
			//you can chose which copy to play.
			if ( ( !m_vSongs[m_iSongNum % m_vSongs.size()]->GetTranslitArtist().CompareNoCase( NSMAN->m_sArtist ) ) &&
				 ( !m_vSongs[m_iSongNum % m_vSongs.size()]->GetTranslitMainTitle().CompareNoCase( NSMAN->m_sMainTitle ) ) &&
				 ( !m_vSongs[m_iSongNum % m_vSongs.size()]->GetTranslitSubTitle().CompareNoCase( NSMAN->m_sSubTitle ) ) )
			{
				switch ( NSMAN->m_iSelectMode )
				{
				case 0:
				case 1:
					NSMAN->m_iSelectMode = 0;
					NSMAN->SelectUserSong();
					break;
				case 2:	//Proper starting of song
				case 3:	//Blind starting of song
					StartSelectedSong();
					break;
				}
				break;
			}

			//We always need to find the song
			m_iGroupNum=m_vGroups.size()-1;	//Alphabetical

			UpdateGroupsListPos();
			UpdateSongsList();
			
			unsigned i;
			for ( i = 0; i < m_vSongs.size(); ++i)
				if ( ( !m_vSongs[i]->GetTranslitArtist().CompareNoCase( NSMAN->m_sArtist ) ) &&
					 ( !m_vSongs[i]->GetTranslitMainTitle().CompareNoCase( NSMAN->m_sMainTitle ) ) &&
					 ( !m_vSongs[i]->GetTranslitSubTitle().CompareNoCase( NSMAN->m_sSubTitle ) ) )
					 break;
			
			bool haveSong = i != m_vSongs.size();

			switch (NSMAN->m_iSelectMode)
			{
			case 3:
				StartSelectedSong();
				break;
			case 2: //We need to do cmd 1 as well here
				if (haveSong)
				{
					m_iSongNum = i + m_vSongs.size();
					UpdateSongsListPos();
					StartSelectedSong();
				}
				break;
			case 1:	//Scroll to song as well
				if (haveSong)
				{
					m_iSongNum = i + m_vSongs.size();
					UpdateSongsListPos();
				}
				//don't break here
			case 0:	//See if client has song
				if (haveSong)
					NSMAN->m_iSelectMode = 0;
				else
					NSMAN->m_iSelectMode = 1;
				NSMAN->SelectUserSong();
			}
		}
		break;
	case SM_BackFromOpts:
		//XXX: HACK: This will causes ScreenSelectOptions to go back here.
		NSMAN->ReportNSSOnOff(1);
		GAMESTATE->m_bEditing = false;
		NSMAN->ReportPlayerOptions();
		break;
	}
	//Must be at end, as so it is last resort for SMOnline packets.
	//If it doens't know what to do, then it'll just remove them.
	ScreenNetSelectBase::HandleScreenMessage( SM );
}

void ScreenNetSelectMusic::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	m_soundChangeOpt.Play();
	switch (m_SelectMode)
	{
	case SelectGroup:
		m_iGroupNum--;
		UpdateGroupsListPos();
		UpdateSongsList();
		UpdateSongsListPos();
		break;
	case SelectSong:
		m_iSongNum--;
		UpdateSongsListPos();
		break;
	case SelectDifficulty:
		{
			StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
			vector <Steps *> MultiSteps;
			MultiSteps = GAMESTATE->m_pCurSong->GetAllSteps( st );
			if (MultiSteps.size() == 0)
				m_DC[pn] = NUM_DIFFICULTIES;
			else
			{
				int i;
				bool dcs[NUM_DIFFICULTIES];

				for ( i=0; i<NUM_DIFFICULTIES; ++i )
					dcs[i] = false;

				for ( i=0; i<(int)MultiSteps.size(); ++i )
					dcs[MultiSteps[i]->GetDifficulty()] = true;

				for ( i=NUM_DIFFICULTIES-1; i>=0; --i )
					if ( (dcs[i]) && (i < m_DC[pn]) )
					{
						m_DC[pn] = (Difficulty)i;
						break;
					}
			}
			UpdateDifficulties( pn );
			GAMESTATE->m_PreferredDifficulty[pn] = m_DC[pn];
		}
		break;
	case SelectOptions:
		//XXX: HACK: This will causes ScreenSelectOptions to go back here.
		NSMAN->ReportNSSOnOff(3);
		GAMESTATE->m_bEditing = true;
		SCREENMAN->AddNewScreenToTop( "ScreenPlayerOptions", SM_BackFromOpts );
		break;
	}
}

void ScreenNetSelectMusic::MenuRight( PlayerNumber pn, const InputEventType type )
{
	m_soundChangeOpt.Play();
	switch (m_SelectMode)
	{
	case SelectGroup:
		m_iGroupNum++;
		UpdateGroupsListPos();
		UpdateSongsList();
		UpdateSongsListPos();
		break;
	case SelectSong:
		m_iSongNum++;
		UpdateSongsListPos();
		break;
	case SelectDifficulty:
		{
			StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
			vector <Steps *> MultiSteps;
			MultiSteps = GAMESTATE->m_pCurSong->GetAllSteps( st );
			if (MultiSteps.size() == 0)
				m_DC[pn] = NUM_DIFFICULTIES;
			else
			{
				int i;

				bool dcs[NUM_DIFFICULTIES];

				for ( i=0; i<NUM_DIFFICULTIES; ++i )
					dcs[i] = false;

				for ( i=0; i<(int)MultiSteps.size(); ++i )
					dcs[MultiSteps[i]->GetDifficulty()] = true;

				for ( i=0; i<NUM_DIFFICULTIES; ++i )
				{
					if ( (dcs[i]) && (i > m_DC[pn]) )
					{
						m_DC[pn] = (Difficulty)i;
						break;
					}
				}

				
			}
			UpdateDifficulties( pn );
			GAMESTATE->m_PreferredDifficulty[pn] = m_DC[pn];
		}
		break;
	case SelectOptions:
		//XXX: HACK: This will causes ScreenSelectOptions to go back here.
		NSMAN->ReportNSSOnOff(3);
		GAMESTATE->m_bEditing = true;
		SCREENMAN->AddNewScreenToTop( "ScreenPlayerOptions", SM_BackFromOpts );
		break;
	}
}

void ScreenNetSelectMusic::MenuUp( PlayerNumber pn, const InputEventType type )
{
	m_soundChangeSel.Play();
	m_SelectMode = (NetScreenSelectModes) ( ( (int)m_SelectMode - 1) % (int)SelectModes);
	if ( (int) m_SelectMode < 0) 
		m_SelectMode = (NetScreenSelectModes) (SelectModes - 1);
	m_rectSelection.StopTweening( );
	COMMAND( m_rectSelection,  ssprintf("To%d", m_SelectMode+1 ) );
}

void ScreenNetSelectMusic::MenuDown( PlayerNumber pn, const InputEventType type )
{
	m_soundChangeSel.Play();
	m_SelectMode = (NetScreenSelectModes) ( ( (int)m_SelectMode + 1) % (int)SelectModes);
	m_rectSelection.StopTweening( );
	COMMAND( m_rectSelection,  ssprintf("To%d", m_SelectMode+1 ) );
}

void ScreenNetSelectMusic::MenuStart( PlayerNumber pn )
{
	if ( NSMAN->useSMserver )
	{
		int j = m_iSongNum % m_vSongs.size();
		NSMAN->m_sArtist = m_vSongs[j]->GetTranslitArtist();
		NSMAN->m_sMainTitle = m_vSongs[j]->GetTranslitMainTitle();
		NSMAN->m_sSubTitle = m_vSongs[j]->GetTranslitSubTitle();
		NSMAN->m_iSelectMode = 2; //Command for user selecting song
		NSMAN->SelectUserSong ();
	}
	else
		StartSelectedSong();
}

void ScreenNetSelectMusic::MenuBack( PlayerNumber pn )
{
	SOUND->StopMusic();
	TweenOffScreen();

	Back( SM_GoToPrevScreen );
}


void ScreenNetSelectMusic::TweenOffScreen()
{
	ScreenNetSelectBase::TweenOffScreen();

	OFF_COMMAND( m_rectSelection );
	OFF_COMMAND( m_textGroups );
	OFF_COMMAND( m_rectGroupsBackground );

	OFF_COMMAND( m_textSongs );
	OFF_COMMAND( m_rectSongsBackground );

	OFF_COMMAND( m_textArtist );
	OFF_COMMAND( m_textSubtitle );
	OFF_COMMAND( m_textGroup );

	OFF_COMMAND( m_rectExInfo );
	OFF_COMMAND( m_rectDiff );

	OFF_COMMAND( m_sprSelOptions );

	FOREACH_EnabledPlayer (pn)
	{
		OFF_COMMAND( m_DifficultyMeters[pn] );
		OFF_COMMAND( m_DifficultyIcon[pn] );
	}
	NSMAN->ReportNSSOnOff(0);
}

void ScreenNetSelectMusic::StartSelectedSong()
{
	Song * pSong = m_vSongs[m_iSongNum%m_vSongs.size()];
	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType; //STEPS_TYPE_DANCE_SINGLE;
	FOREACH_EnabledPlayer (pn)
	{
		GAMESTATE->m_PreferredDifficulty[pn] = m_DC[pn];
		Steps * pSteps = pSong->GetStepsByDifficulty(st,m_DC[pn]);
		GAMESTATE->m_pCurSteps[pn] = pSteps;
	}
	
	TweenOffScreen();
	StartTransitioning( SM_GoToNextScreen );
}

void ScreenNetSelectMusic::UpdateDifficulties( PlayerNumber pn )
{
	if ( ( m_DC[pn] < DIFFICULTY_EDIT ) && ( m_DC[pn] >= DIFFICULTY_BEGINNER ) )
		m_DifficultyIcon[pn].SetFromDifficulty( pn, m_DC[pn] );
	else
		m_DifficultyIcon[pn].SetFromSteps( pn, NULL );	//It will blank it out 

	StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;

	if ( ( m_DC[pn] < NUM_DIFFICULTIES ) && ( m_DC[pn] >= DIFFICULTY_BEGINNER ) )
		m_DifficultyMeters[pn].SetFromSteps( GAMESTATE->m_pCurSong->GetStepsByDifficulty( st, m_DC[pn] ) );
	else
		m_DifficultyMeters[pn].SetFromMeterAndDifficulty( 0, DIFFICULTY_BEGINNER ); 
}

void ScreenNetSelectMusic::UpdateSongsListPos()
{
	int i,j;
	if (m_iSongNum<m_iShowSongs)
		m_iSongNum+=m_vSongs.size();
	CString SongsDisplay="";
	for (i=m_iSongNum-m_iShowSongs; i<=m_iSongNum+m_iShowSongs; ++i)
	{
		j= i % m_vSongs.size();
		SongsDisplay+=m_vSongs.at(j)->GetTranslitMainTitle();
		if (i<m_iSongNum+m_iShowSongs)
			SongsDisplay+='\n';
	}
	m_textSongs.SetText( SongsDisplay );

	j= m_iSongNum % m_vSongs.size();

	m_textArtist.SetText( m_vSongs[j]->GetTranslitArtist() );
	m_textSubtitle.SetText( m_vSongs[j]->GetTranslitSubTitle() );
	m_textGroup.SetText( m_vSongs[j]->m_sGroupName );
	GAMESTATE->m_pCurSong = m_vSongs[j];

	//Update the difficulty Icons
	//Handle difficulty
	FOREACH_EnabledPlayer (pn)
	{
		m_DC[pn] = GAMESTATE->m_PreferredDifficulty[pn];
		StepsType st = GAMESTATE->GetCurrentStyle()->m_StepsType;
		vector <Steps *> MultiSteps;
		MultiSteps = m_vSongs[j]->GetAllSteps( st );
		if (MultiSteps.size() == 0)
			m_DC[pn] = NUM_DIFFICULTIES;
		else
		{
			int i;
			Difficulty Target = DIFFICULTY_EASY;

			bool dcs[NUM_DIFFICULTIES];

			for ( i=0; i<NUM_DIFFICULTIES; ++i )
				dcs[i] = false;

			for ( i=0; i<(int)MultiSteps.size(); ++i )
				dcs[MultiSteps[i]->GetDifficulty()] = true;

			for ( i=0; i<NUM_DIFFICULTIES; ++i )
				if ( dcs[i] )
				{
					Target = (Difficulty)i;
					if ( i >= m_DC[pn] )
					{
						m_DC[pn] = (Difficulty)i;
						break;
					}
				}

			if ( i == NUM_DIFFICULTIES )
				m_DC[pn] = Target;
		}
		UpdateDifficulties( pn );
	}

	//Then handle sound (Copied from MusicBannerWheel)
	SOUND->StopMusic();
	Song* pSong = m_vSongs[j];
	if( pSong  &&  pSong->HasMusic() )
	{
		if(SOUND->GetMusicPath().CompareNoCase(pSong->GetMusicPath())) // dont play the same sound over and over
		{
			
			SOUND->StopMusic();
			SOUND->PlayMusic(pSong->GetMusicPath(), true,
				pSong->m_fMusicSampleStartSeconds,
				pSong->m_fMusicSampleLengthSeconds);
		}
	}

}

void ScreenNetSelectMusic::UpdateGroupsListPos()
{
	int i,j;
	if (m_iGroupNum<m_iShowGroups)
		m_iGroupNum+=m_vGroups.size();
	CString GroupsDisplay="";
	for (i=m_iGroupNum-m_iShowGroups; i<=m_iGroupNum+m_iShowGroups; ++i)
	{
		j=i%m_vGroups.size();
		GroupsDisplay+=m_vGroups[j];
		if (i<m_iGroupNum+m_iShowGroups)
			GroupsDisplay+='\n';
	}	
	m_textGroups.SetText( GroupsDisplay );
}

void ScreenNetSelectMusic::UpdateSongsList()
{
	m_vSongs.clear();
	SONGMAN->SortSongs();
	if (m_iGroupNum<m_iShowGroups)
		m_iGroupNum+=m_vGroups.size();
	int j=m_iGroupNum%m_vGroups.size();
	if ( m_vGroups[j]==AllGroups )
		SONGMAN->GetSongs( m_vSongs );	//this gets it alphabetically
	else
		SONGMAN->GetSongs( m_vSongs, m_vGroups[j] );
}

#endif

/*
 * (c) 2004 Charles Lohr
 * All rights reserved.
 *		Based off of ScreenEz2SelectMusic by Frieza
 *      Elements from ScreenTextEntry
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
