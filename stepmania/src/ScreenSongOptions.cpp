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



OptionLineData g_SongOptionsLines[] = {
	{ "Fail", 3, {"ARCADE","END OF SONGMAN","OFF"} },	
	{ "Assist", 2, {"OFF","TICK"} },	
	{ "Rate", 7, {"70%","80%","90%","100%","110%","120%","130%"} },	
	{ "Pitch", 7, {"-1.5","-1","-0.5","+0","+0.5","+1","+1.5"} },	
	{ "Bars", 2, {"OFF","ON"} },	
};
const int NUM_SONG_OPTIONS_LINES = sizeof(g_SongOptionsLines)/sizeof(OptionLineData);

enum {
	SO_FAIL = 0,
	SO_ASSIST,
	SO_RATE,
	SO_PITCH,
	SO_BARS
};



ScreenSongOptions::ScreenSongOptions() :
	ScreenOptions(
		THEME->GetPathTo(GRAPHIC_SONG_OPTIONS_BACKGROUND),
		THEME->GetPathTo(GRAPHIC_SONG_OPTIONS_TOP_EDGE)
		)
{
	LOG->WriteLine( "ScreenSongOptions::ScreenSongOptions()" );

	Init( INPUTMODE_BOTH, 
		g_SongOptionsLines, 
		NUM_SONG_OPTIONS_LINES
		);
}

void ScreenSongOptions::ImportOptions()
{
	SongOptions &so = PREFSMAN->m_SongOptions;

	m_iSelectedOption[0][SO_FAIL] = so.m_FailType;
	m_iSelectedOption[0][SO_ASSIST] = so.m_AssistType;

	if(		 so.m_fMusicRate == 0.70f )		m_iSelectedOption[0][SO_RATE] = 0;
	else if( so.m_fMusicRate == 0.80f )		m_iSelectedOption[0][SO_RATE] = 1;
	else if( so.m_fMusicRate == 0.90f )		m_iSelectedOption[0][SO_RATE] = 2;
	else if( so.m_fMusicRate == 1.00f )		m_iSelectedOption[0][SO_RATE] = 3;
	else if( so.m_fMusicRate == 1.10f )		m_iSelectedOption[0][SO_RATE] = 4;
	else if( so.m_fMusicRate == 1.20f )		m_iSelectedOption[0][SO_RATE] = 5;
	else if( so.m_fMusicRate == 1.30f )		m_iSelectedOption[0][SO_RATE] = 6;
	else									m_iSelectedOption[0][SO_RATE] = 3;

	if(		 so.m_fMusicRate == -1.5f )		m_iSelectedOption[0][SO_PITCH] = 0;
	else if( so.m_fMusicRate == -1.0f )		m_iSelectedOption[0][SO_PITCH] = 1;
	else if( so.m_fMusicRate == -0.5f )		m_iSelectedOption[0][SO_PITCH] = 2;
	else if( so.m_fMusicRate ==  0.0f )		m_iSelectedOption[0][SO_PITCH] = 3;
	else if( so.m_fMusicRate ==  0.5f )		m_iSelectedOption[0][SO_PITCH] = 4;
	else if( so.m_fMusicRate ==  1.0f )		m_iSelectedOption[0][SO_PITCH] = 5;
	else if( so.m_fMusicRate ==  1.5f )		m_iSelectedOption[0][SO_PITCH] = 6;
	else									m_iSelectedOption[0][SO_PITCH] = 3;
}

void ScreenSongOptions::ExportOptions()
{
	SongOptions &so = PREFSMAN->m_SongOptions;

	so.m_FailType = (SongOptions::FailType)m_iSelectedOption[0][SO_FAIL];
	so.m_AssistType = (SongOptions::AssistType)m_iSelectedOption[0][SO_ASSIST];

	switch( m_iSelectedOption[0][SO_RATE] )
	{
	case 0:	so.m_fMusicRate = 0.70f;	break;
	case 1:	so.m_fMusicRate = 0.80f;	break;
	case 2:	so.m_fMusicRate = 0.90f;	break;
	case 3:	so.m_fMusicRate = 1.00f;	break;
	case 4:	so.m_fMusicRate = 1.10f;	break;
	case 5:	so.m_fMusicRate = 1.20f;	break;
	case 6:	so.m_fMusicRate = 1.30f;	break;
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




