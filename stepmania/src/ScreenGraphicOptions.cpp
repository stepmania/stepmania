#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenGraphicOptions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenGraphicOptions.h"
#include <assert.h>
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageMusic.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "ScreenOptions.h"
#include "ScreenTitleMenu.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"



enum {
	GO_WINDOWED = 0,
	GO_DISPLAY_RESOLUTION,
	GO_TEXTURE_RESOLUTION,
	GO_REFRESH_RATE,
	GO_SHOWSTATS,
	GO_BGMODE,
	GO_BGBRIGHTNESS,
	GO_MOVIEDECODEMS,
	NUM_GRAPHIC_OPTIONS_LINES
};
OptionLineData g_GraphicOptionsLines[NUM_GRAPHIC_OPTIONS_LINES] = {
	{ "Display",		2,  {"FULLSCREEN", "WINDOWED"} },
	{ "Display Res",	7,  {"320","400","512","640","800","1024","1280"} },
	{ "Texture Res",	3,  {"256","512","1024"} },
	{ "Refresh Rate",	10, {"DEFAULT","60","70","72","75","80","85","90","100","120"} },
	{ "Show Stats",		2,  {"OFF","ON"} },
	{ "BG Mode",		4,  {"OFF","ANIMATIONS","VISUALIZATIONS","RANDOM MOVIES"} },
	{ "BG Brightness",	5,  {"20%","40%","60%","80%","100%"} },
	{ "Movie Decode",	5,  {"1ms","2ms","3ms","4ms","5ms"} },
};

ScreenGraphicOptions::ScreenGraphicOptions() :
	ScreenOptions(
		THEME->GetPathTo("Graphics","graphic options background"),
		THEME->GetPathTo("Graphics","graphic options page"),
		THEME->GetPathTo("Graphics","graphic options top edge")
		)
{
	LOG->Trace( "ScreenGraphicOptions::ScreenGraphicOptions()" );

	Init( 
		INPUTMODE_BOTH, 
		g_GraphicOptionsLines, 
		NUM_GRAPHIC_OPTIONS_LINES
		);
	m_Menu.StopTimer();
}

void ScreenGraphicOptions::ImportOptions()
{
	m_iSelectedOption[0][GO_WINDOWED]				= PREFSMAN->m_bWindowed ? 1:0;

	switch( PREFSMAN->m_iDisplayResolution )
	{
	case 320:	m_iSelectedOption[0][GO_DISPLAY_RESOLUTION] = 0;	break;
	case 400:	m_iSelectedOption[0][GO_DISPLAY_RESOLUTION] = 1;	break;
	case 512:	m_iSelectedOption[0][GO_DISPLAY_RESOLUTION] = 2;	break;
	case 640:	m_iSelectedOption[0][GO_DISPLAY_RESOLUTION] = 3;	break;
	case 800:	m_iSelectedOption[0][GO_DISPLAY_RESOLUTION] = 4;	break;
	case 1024:	m_iSelectedOption[0][GO_DISPLAY_RESOLUTION] = 5;	break;
	case 1280:	m_iSelectedOption[0][GO_DISPLAY_RESOLUTION] = 6;	break;
	default:	m_iSelectedOption[0][GO_DISPLAY_RESOLUTION] = 3;	break;
	}
	
	switch( PREFSMAN->m_iTextureResolution )
	{
	case 256:	m_iSelectedOption[0][GO_TEXTURE_RESOLUTION] = 0;	break;
	case 512:	m_iSelectedOption[0][GO_TEXTURE_RESOLUTION] = 1;	break;
	case 1024:	m_iSelectedOption[0][GO_TEXTURE_RESOLUTION] = 2;	break;
	default:	m_iSelectedOption[0][GO_TEXTURE_RESOLUTION] = 1;	break;
	}
	
	switch( PREFSMAN->m_iRefreshRate )
	{
	case 0:		m_iSelectedOption[0][GO_REFRESH_RATE] = 0;	break;
	case 60:	m_iSelectedOption[0][GO_REFRESH_RATE] = 1;	break;
	case 70:	m_iSelectedOption[0][GO_REFRESH_RATE] = 2;	break;
	case 72:	m_iSelectedOption[0][GO_REFRESH_RATE] = 3;	break;
	case 75:	m_iSelectedOption[0][GO_REFRESH_RATE] = 4;	break;
	case 80:	m_iSelectedOption[0][GO_REFRESH_RATE] = 5;	break;
	case 85:	m_iSelectedOption[0][GO_REFRESH_RATE] = 6;	break;
	case 90:	m_iSelectedOption[0][GO_REFRESH_RATE] = 7;	break;
	case 100:	m_iSelectedOption[0][GO_REFRESH_RATE] = 8;	break;
	case 120:	m_iSelectedOption[0][GO_REFRESH_RATE] = 9;	break;
	default:	m_iSelectedOption[0][GO_REFRESH_RATE] = 0;	break;
	}

	m_iSelectedOption[0][GO_SHOWSTATS]				= PREFSMAN->m_bShowStats ? 1:0;
	m_iSelectedOption[0][GO_BGMODE]					= PREFSMAN->m_BackgroundMode;

	if(      PREFSMAN->m_fBGBrightness == 0.2f )	m_iSelectedOption[0][GO_BGBRIGHTNESS] = 0;
	else if( PREFSMAN->m_fBGBrightness == 0.4f )	m_iSelectedOption[0][GO_BGBRIGHTNESS] = 1;
	else if( PREFSMAN->m_fBGBrightness == 0.6f )	m_iSelectedOption[0][GO_BGBRIGHTNESS] = 2;
	else if( PREFSMAN->m_fBGBrightness == 0.8f )	m_iSelectedOption[0][GO_BGBRIGHTNESS] = 3;
	else if( PREFSMAN->m_fBGBrightness == 1.0f )	m_iSelectedOption[0][GO_BGBRIGHTNESS] = 4;
	else											m_iSelectedOption[0][GO_BGBRIGHTNESS] = 2;
	
	m_iSelectedOption[0][GO_MOVIEDECODEMS]			= PREFSMAN->m_iMovieDecodeMS-1;
}

void ScreenGraphicOptions::ExportOptions()
{
	PREFSMAN->m_bWindowed				= m_iSelectedOption[0][GO_WINDOWED] == 1;

	switch( m_iSelectedOption[0][GO_DISPLAY_RESOLUTION] )
	{
	case 0:	PREFSMAN->m_iDisplayResolution = 320;	break;
	case 1:	PREFSMAN->m_iDisplayResolution = 400;	break;
	case 2:	PREFSMAN->m_iDisplayResolution = 512;	break;
	case 3:	PREFSMAN->m_iDisplayResolution = 640;	break;
	case 4:	PREFSMAN->m_iDisplayResolution = 800;	break;
	case 5:	PREFSMAN->m_iDisplayResolution = 1024;	break;
	case 6:	PREFSMAN->m_iDisplayResolution = 1280;	break;
	default:	ASSERT(0);	PREFSMAN->m_iDisplayResolution = 640;	break;
	}
	
	switch( m_iSelectedOption[0][GO_TEXTURE_RESOLUTION] )
	{
	case 0:	PREFSMAN->m_iTextureResolution = 256;	break;
	case 1:	PREFSMAN->m_iTextureResolution = 512;	break;
	case 2:	PREFSMAN->m_iTextureResolution = 1024;	break;
	default:	ASSERT(0);	PREFSMAN->m_iTextureResolution = 512;	break;
	}
		
	switch( m_iSelectedOption[0][GO_REFRESH_RATE] )
	{
	case 0:	PREFSMAN->m_iRefreshRate = 0;	break;
	case 1:	PREFSMAN->m_iRefreshRate = 60;	break;
	case 2:	PREFSMAN->m_iRefreshRate = 70;	break;
	case 3:	PREFSMAN->m_iRefreshRate = 72;	break;
	case 4:	PREFSMAN->m_iRefreshRate = 75;	break;
	case 5:	PREFSMAN->m_iRefreshRate = 80;	break;
	case 6:	PREFSMAN->m_iRefreshRate = 85;	break;
	case 7:	PREFSMAN->m_iRefreshRate = 90;	break;
	case 8:	PREFSMAN->m_iRefreshRate = 100;	break;
	case 9:	PREFSMAN->m_iRefreshRate = 120;	break;
	default:	ASSERT(0);	PREFSMAN->m_iRefreshRate = 0;	break;
	}

	PREFSMAN->m_bShowStats				= m_iSelectedOption[0][GO_SHOWSTATS] == 1;
	PREFSMAN->m_BackgroundMode			= PrefsManager::BackgroundMode( m_iSelectedOption[0][GO_BGMODE] );

	switch( m_iSelectedOption[0][GO_BGBRIGHTNESS] )
	{
	case 0:	PREFSMAN->m_fBGBrightness = 0.2f;	break;
	case 1:	PREFSMAN->m_fBGBrightness = 0.4f;	break;
	case 2:	PREFSMAN->m_fBGBrightness = 0.6f;	break;
	case 3:	PREFSMAN->m_fBGBrightness = 0.8f;	break;
	case 4:	PREFSMAN->m_fBGBrightness = 1.0f;	break;
	default:	ASSERT(0);
	}

	PREFSMAN->m_iMovieDecodeMS			= m_iSelectedOption[0][GO_MOVIEDECODEMS]+1;
}

void ScreenGraphicOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( new ScreenTitleMenu );
	PREFSMAN->SaveGlobalPrefsToDisk();
	ApplyGraphicOptions();
}

void ScreenGraphicOptions::GoToNextState()
{
	GoToPrevState();
}

