#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenGraphicOptions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenGraphicOptions.h"
#include "RageUtil.h"
#include "RageSounds.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "RageDisplay.h"


enum {
	GO_WINDOWED = 0,
	GO_DISPLAY_RESOLUTION,
	GO_DISPLAY_COLOR_DEPTH,
	GO_MAX_TEXTURE_RESOLUTION,
	GO_TEXTURE_COLOR_DEPTH,
	GO_KEEP_TEXTURES_IN_MEM,
	GO_REFRESH_RATE,
	GO_MOVIEDECODEMS,
	GO_VSYNC,
	NUM_GRAPHIC_OPTIONS_LINES
};
OptionRow g_GraphicOptionsLines[NUM_GRAPHIC_OPTIONS_LINES] = {
	OptionRow( "Display\nMode",				"FULLSCREEN", "WINDOWED" ),
	OptionRow( "Display\nResolution",		"320","400","512","640","800","1024","1280" ),
	OptionRow( "Display\nColor",			"16BIT","32BIT" ),
	OptionRow( "Max Texture\nResolution",	"256","512","1024","2048" ),
	OptionRow( "Texture\nColor",			"16BIT","32BIT" ),
	OptionRow( "Keep Textures\nIn Memory",	"NO","YES" ),
	OptionRow( "Refresh\nRate",				"DEFAULT","60","70","72","75","80","85","90","100","120","150" ),
	OptionRow( "Movie\nDecode",				"1ms","2ms","3ms","4ms" ),
	OptionRow( "Wait For\nVsync",			"NO", "YES" ),
};

static const int HorizRes[] = {
	320, 400, 512, 640, 800, 1024, 1280
};
static const int VertRes[] = {
	240, 300, 384, 480, 600, 768, 960
};
static const int TextureRes[] = {
	256, 512, 1024, 2048
};

ScreenGraphicOptions::ScreenGraphicOptions() :
	ScreenOptions("ScreenGraphicOptions",false)
{
	LOG->Trace( "ScreenGraphicOptions::ScreenGraphicOptions()" );

	Init(
		INPUTMODE_BOTH, 
		g_GraphicOptionsLines, 
		NUM_GRAPHIC_OPTIONS_LINES,
		false, true );
	m_Menu.m_MenuTimer.Disable();

	SOUND->PlayMusic( THEME->GetPathToS("ScreenGraphicOptions music") );
}

void ScreenGraphicOptions::ImportOptions()
{
	m_iSelectedOption[0][GO_WINDOWED]				= PREFSMAN->m_bWindowed ? 1:0;

	switch( PREFSMAN->m_iDisplayWidth )
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
		
	switch( PREFSMAN->m_iDisplayColorDepth )
	{
	case 16:	m_iSelectedOption[0][GO_DISPLAY_COLOR_DEPTH] = 0;	break;
	case 32:	m_iSelectedOption[0][GO_DISPLAY_COLOR_DEPTH] = 1;	break;
	}

	switch( PREFSMAN->m_iMaxTextureResolution )
	{
	case 256:	m_iSelectedOption[0][GO_MAX_TEXTURE_RESOLUTION] = 0;	break;
	case 512:	m_iSelectedOption[0][GO_MAX_TEXTURE_RESOLUTION] = 1;	break;
	case 1024:	m_iSelectedOption[0][GO_MAX_TEXTURE_RESOLUTION] = 2;	break;
	case 2048:	m_iSelectedOption[0][GO_MAX_TEXTURE_RESOLUTION] = 3;	break;
	default:	m_iSelectedOption[0][GO_MAX_TEXTURE_RESOLUTION] = 3;	break;
	}
		
	switch( PREFSMAN->m_iTextureColorDepth )
	{
	case 16:	m_iSelectedOption[0][GO_TEXTURE_COLOR_DEPTH] = 0;	break;
	case 32:	m_iSelectedOption[0][GO_TEXTURE_COLOR_DEPTH] = 1;	break;
	}

	m_iSelectedOption[0][GO_KEEP_TEXTURES_IN_MEM]			= PREFSMAN->m_bDelayedTextureDelete ? 1:0;

	switch(PREFSMAN->m_iRefreshRate)
	{
	case REFRESH_DEFAULT:	m_iSelectedOption[0][GO_REFRESH_RATE]		= 0; break;
	default:
		for(unsigned i = 2; i < g_GraphicOptionsLines[GO_REFRESH_RATE].choices.size(); ++i) {
			if(atoi(g_GraphicOptionsLines[GO_REFRESH_RATE].choices[i]) <= PREFSMAN->m_iRefreshRate) 
				m_iSelectedOption[0][GO_REFRESH_RATE] = i;
		}
	}

	m_iSelectedOption[0][GO_MOVIEDECODEMS]			= PREFSMAN->m_iMovieDecodeMS-1;
	m_iSelectedOption[0][GO_VSYNC]					= PREFSMAN->m_bVsync ? 1:0;	
}

void ScreenGraphicOptions::ExportOptions()
{
	PREFSMAN->m_bWindowed				= m_iSelectedOption[0][GO_WINDOWED] == 1;

	if(m_iSelectedOption[0][GO_DISPLAY_RESOLUTION] > 6)
	{
		ASSERT(0);
		m_iSelectedOption[0][GO_DISPLAY_RESOLUTION] = 3;
	}
	PREFSMAN->m_iDisplayWidth = HorizRes[m_iSelectedOption[0][GO_DISPLAY_RESOLUTION]];
	PREFSMAN->m_iDisplayHeight = VertRes[m_iSelectedOption[0][GO_DISPLAY_RESOLUTION]];
	
	switch( m_iSelectedOption[0][GO_DISPLAY_COLOR_DEPTH] )
	{
	case 0:	PREFSMAN->m_iDisplayColorDepth = 16;	break;
	case 1:	PREFSMAN->m_iDisplayColorDepth = 32;	break;
	default:	ASSERT(0);	PREFSMAN->m_iDisplayColorDepth = 16;	break;
	}
	
	PREFSMAN->m_iMaxTextureResolution = TextureRes[m_iSelectedOption[0][GO_MAX_TEXTURE_RESOLUTION]];

	switch( m_iSelectedOption[0][GO_TEXTURE_COLOR_DEPTH] )
	{
	case 0:	PREFSMAN->m_iTextureColorDepth = 16;	break;
	case 1:	PREFSMAN->m_iTextureColorDepth = 32;	break;
	default:	ASSERT(0);	PREFSMAN->m_iTextureColorDepth = 16;	break;
	}
	
	PREFSMAN->m_bDelayedTextureDelete		= (m_iSelectedOption[0][GO_KEEP_TEXTURES_IN_MEM] == 1);

	if(m_iSelectedOption[0][GO_REFRESH_RATE] == 0)
		PREFSMAN->m_iRefreshRate = REFRESH_DEFAULT;
	else 
	{
		int n = m_iSelectedOption[0][GO_REFRESH_RATE];
		PREFSMAN->m_iRefreshRate = atoi(g_GraphicOptionsLines[GO_REFRESH_RATE].choices[n]);
	}

	PREFSMAN->m_iMovieDecodeMS			= m_iSelectedOption[0][GO_MOVIEDECODEMS]+1;
	PREFSMAN->m_bVsync					= m_iSelectedOption[0][GO_VSYNC] == 1;
}

void ScreenGraphicOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
}

void ScreenGraphicOptions::GoToNextState()
{
	PREFSMAN->SaveGlobalPrefsToDisk();
	ApplyGraphicOptions();
	GoToPrevState();
}

