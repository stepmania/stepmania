#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenSongOptions.h

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "ScreenSongOptions.h"
#include <assert.h>
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageMusic.h"
#include "ScreenManager.h"
#include "ScreenSelectMusic.h"
#include "ScreenGameplay.h"
#include "GameConstantsAndTypes.h"
#include "ScreenPlayerOptions.h"
#include "RageLog.h"
#include "GameState.h"



OptionLineData g_SongOptionsLines[] = {
	{ "Life Type",	2, {"BAR","BATTERY"} },	
	{ "Bar Drain",	3, {"NORMAL","NO RECOVER","SUDDEN DEATH"} },	
	{ "Bat.Lives",	10, {"1","2","3","4","5","6","7","8","9","10"} },	
	{ "Fail",		3, {"ARCADE","END OF SONG","OFF"} },	
	{ "Assist",		2, {"OFF","TICK"} },	
	{ "Rate",		9, {"x0.7","x0.8","x0.9","x1.0","x1.1","x1.2","x1.3","x1.4","x1.5"} },	
	{ "Bars",		2, {"OFF","ON"} },	
};
const int NUM_SONG_OPTIONS_LINES = sizeof(g_SongOptionsLines)/sizeof(OptionLineData);

enum {
	SO_LIFE = 0,
	SO_DRAIN,
	SO_BAT_LIVES,
	SO_FAIL,
	SO_ASSIST,
	SO_RATE,
	SO_BARS
};



ScreenSongOptions::ScreenSongOptions() :
	ScreenOptions(
		THEME->GetPathTo(GRAPHIC_SONG_OPTIONS_BACKGROUND),
		THEME->GetPathTo(GRAPHIC_SONG_OPTIONS_TOP_EDGE)
		)
{
	LOG->Trace( "ScreenSongOptions::ScreenSongOptions()" );

	Init( INPUTMODE_BOTH, 
		g_SongOptionsLines, 
		NUM_SONG_OPTIONS_LINES
		);
}

void ScreenSongOptions::ImportOptions()
{
	SongOptions &so = GAMESTATE->m_SongOptions;

	m_iSelectedOption[0][SO_LIFE] = so.m_LifeType;
	m_iSelectedOption[0][SO_BAT_LIVES] = so.m_iBatteryLives-1;
	m_iSelectedOption[0][SO_FAIL] = so.m_FailType;
	m_iSelectedOption[0][SO_ASSIST] = so.m_AssistType;

	if(		 so.m_fMusicRate == 0.7f )		m_iSelectedOption[0][SO_RATE] = 0;
	else if( so.m_fMusicRate == 0.8f )		m_iSelectedOption[0][SO_RATE] = 1;
	else if( so.m_fMusicRate == 0.9f )		m_iSelectedOption[0][SO_RATE] = 2;
	else if( so.m_fMusicRate == 1.0f )		m_iSelectedOption[0][SO_RATE] = 3;
	else if( so.m_fMusicRate == 1.1f )		m_iSelectedOption[0][SO_RATE] = 4;
	else if( so.m_fMusicRate == 1.2f )		m_iSelectedOption[0][SO_RATE] = 5;
	else if( so.m_fMusicRate == 1.3f )		m_iSelectedOption[0][SO_RATE] = 6;
	else if( so.m_fMusicRate == 1.4f )		m_iSelectedOption[0][SO_RATE] = 7;
	else if( so.m_fMusicRate == 1.5f )		m_iSelectedOption[0][SO_RATE] = 8;
	else									m_iSelectedOption[0][SO_RATE] = 3;
}

void ScreenSongOptions::ExportOptions()
{
	SongOptions &so = GAMESTATE->m_SongOptions;

	so.m_LifeType = (SongOptions::LifeType)m_iSelectedOption[0][SO_LIFE];
	so.m_DrainType = (SongOptions::DrainType)m_iSelectedOption[0][SO_DRAIN];
	so.m_iBatteryLives = m_iSelectedOption[0][SO_BAT_LIVES]+1;
	so.m_FailType = (SongOptions::FailType)m_iSelectedOption[0][SO_FAIL];
	so.m_AssistType = (SongOptions::AssistType)m_iSelectedOption[0][SO_ASSIST];

	switch( m_iSelectedOption[0][SO_RATE] )
	{
	case 0:	so.m_fMusicRate = 0.7f;	break;
	case 1:	so.m_fMusicRate = 0.8f;	break;
	case 2:	so.m_fMusicRate = 0.9f;	break;
	case 3:	so.m_fMusicRate = 1.0f;	break;
	case 4:	so.m_fMusicRate = 1.1f;	break;
	case 5:	so.m_fMusicRate = 1.2f;	break;
	case 6:	so.m_fMusicRate = 1.3f;	break;
	case 7:	so.m_fMusicRate = 1.4f;	break;
	case 8:	so.m_fMusicRate = 1.5f;	break;
	default:	ASSERT( false );
	}
}

void ScreenSongOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( new ScreenPlayerOptions );
}

void ScreenSongOptions::GoToNextState()
{
	MUSIC->Stop();

	SCREENMAN->SetNewScreen( new ScreenGameplay );
}




