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
#include "RageTextureManager.h"
#include "RageUtil.h"
#include "RageMusic.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "StepMania.h"
#include "PrefsManager.h"
#include "RageLog.h"



enum {
	GO_WINDOWED = 0,
	GO_DISPLAY_RESOLUTION,
	GO_TEXTURE_RESOLUTION,
	GO_REFRESH_RATE,
	GO_BGMODE,
	GO_BGBRIGHTNESS,
	GO_MOVIEDECODEMS,
	GO_BGIFNOBANNER,
	GO_VSYNC,
	NUM_GRAPHIC_OPTIONS_LINES
};
OptionRowData g_GraphicOptionsLines[NUM_GRAPHIC_OPTIONS_LINES] = {
	{ "Display\nMode",			2,  {"FULLSCREEN", "WINDOWED"} },
	{ "Display\nResolution",	7,  {"320","400","512","640","800","1024","1280"} },
	{ "Texture\nResolution",	3,  {"256","512","1024"} },
	{ "Refresh\nRate",			11, {"DEFAULT","MAX","60","70","72","75","80","85","90","100","120"} },
	{ "Background\nMode",		4,  {"OFF","ANIMATIONS","VISUALIZATIONS","RANDOM MOVIES"} },
	{ "Background\nBrightness",	11,  {"0%","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%"} },
	{ "Movie\nDecode",			4,  {"1ms","2ms","3ms","4ms"} },
	{ "BG For\nBanner",			2,  {"NO", "YES (slow)"} },
	{ "Wait For\nVsync",		2,  {"NO", "YES"} },
};

static const int HorizRes[] = {
	320, 400, 512, 640, 800, 1024, 1280
};
static const int VertRes[] = {
	240, 300, 384, 480, 600, 768, 1024
};

ScreenGraphicOptions::ScreenGraphicOptions() :
	ScreenOptions(
		THEME->GetPathTo("BGAnimations","graphic options"),
		THEME->GetPathTo("Graphics","graphic options page"),
		THEME->GetPathTo("Graphics","graphic options top edge")
		)
{
	LOG->Trace( "ScreenGraphicOptions::ScreenGraphicOptions()" );

	// fill g_InputOptionsLines with explanation text
	for( int i=0; i<NUM_GRAPHIC_OPTIONS_LINES; i++ )
	{
		CString sLineName = g_GraphicOptionsLines[i].szTitle;
		sLineName.Replace("\n","");
		sLineName.Replace(" ","");
		strcpy( g_GraphicOptionsLines[i].szExplanation, THEME->GetMetric("ScreenGraphicOptions",sLineName) );
	}

	Init(
		INPUTMODE_BOTH, 
		g_GraphicOptionsLines, 
		NUM_GRAPHIC_OPTIONS_LINES,
		false );
	UpdateRefreshRates();
	m_Menu.StopTimer();

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","graphic options music") );
}

int ScreenGraphicOptions::CurrentRefresh() const
{
	int RefreshOption = m_iSelectedOption[0][GO_REFRESH_RATE];
	switch( RefreshOption )
	{
	case 0:	return RageDisplay::REFRESH_DEFAULT;break;
	case 1:	return RageDisplay::REFRESH_MAX;	break;
	default:
		return atoi( g_GraphicOptionsLines[GO_REFRESH_RATE].szOptionsText[RefreshOption] );
	}
}

void ScreenGraphicOptions::UpdateRefreshRates()
{
#if 0
	CArray<int,int> hz;

	int OldSetting = CurrentRefresh();

	/* XXX: We're hardcoded to 16bpp in StepMania.cpp; if we add a bpp option
	 * this needs to be changed. */
	DISPLAY->GetHzAtResolution(HorizRes[m_iSelectedOption[0][GO_DISPLAY_RESOLUTION]],
		VertRes[m_iSelectedOption[0][GO_DISPLAY_RESOLUTION]], 16 /* XXX */, hz);

	/* Set the refresh to default.  If we can find the old selection in the
	 * new data, we'll set it to that later. */

	OptionRowData &opt = g_GraphicOptionsLines[GO_REFRESH_RATE];
	opt.iNumOptions = 2;
	int OldSettingNo = RageDisplay::REFRESH_DEFAULT;

	int i;
	for(i = 2; i < MAX_OPTIONS_PER_LINE; ++i)
		opt.szOptionsText[i][0] = 0;

	for(i = 0; i < hz.GetSize(); ++i)
	{
		if(hz[i] < 60) continue;
		sprintf(opt.szOptionsText[opt.iNumOptions], "%i", hz[i]);
		opt.iNumOptions++;
		if( hz[i] == OldSetting )
			OldSettingNo = i;
	}

	m_iSelectedOption[0][GO_REFRESH_RATE] = 
	m_iSelectedOption[1][GO_REFRESH_RATE] = OldSettingNo;

	InitOptionsText();
#endif

	/* If we're windowed, leave all refresh rates dimmed, but don't
	 * change the actual selection. */
	if(m_iSelectedOption[0][GO_WINDOWED])
		return;

	PositionUnderlines();
}

void ScreenGraphicOptions::OnChange()
{
	ScreenOptions::OnChange();
	UpdateRefreshRates();
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
	case RageDisplay::REFRESH_DEFAULT:	m_iSelectedOption[0][GO_REFRESH_RATE] = 0;	break;
	case RageDisplay::REFRESH_MAX:		m_iSelectedOption[0][GO_REFRESH_RATE] = 1;	break;
	case 60:	m_iSelectedOption[0][GO_REFRESH_RATE] = 2;	break;
	case 70:	m_iSelectedOption[0][GO_REFRESH_RATE] = 3;	break;
	case 72:	m_iSelectedOption[0][GO_REFRESH_RATE] = 4;	break;
	case 75:	m_iSelectedOption[0][GO_REFRESH_RATE] = 5;	break;
	case 80:	m_iSelectedOption[0][GO_REFRESH_RATE] = 6;	break;
	case 85:	m_iSelectedOption[0][GO_REFRESH_RATE] = 7;	break;
	case 90:	m_iSelectedOption[0][GO_REFRESH_RATE] = 8;	break;
	case 100:	m_iSelectedOption[0][GO_REFRESH_RATE] = 9;	break;
	case 120:	m_iSelectedOption[0][GO_REFRESH_RATE] = 10;	break;
	default:	m_iSelectedOption[0][GO_REFRESH_RATE] = 1;	break;
	}

	m_iSelectedOption[0][GO_BGMODE]					= PREFSMAN->m_BackgroundMode;
	m_iSelectedOption[0][GO_BGBRIGHTNESS]			= (int)( PREFSMAN->m_fBGBrightness*10+0.5f ); 
	m_iSelectedOption[0][GO_MOVIEDECODEMS]			= PREFSMAN->m_iMovieDecodeMS-1;
	m_iSelectedOption[0][GO_BGIFNOBANNER]			= PREFSMAN->m_bUseBGIfNoBanner ? 1:0;
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
	PREFSMAN->m_iDisplayResolution = 
		HorizRes[m_iSelectedOption[0][GO_DISPLAY_RESOLUTION]];
	
	switch( m_iSelectedOption[0][GO_TEXTURE_RESOLUTION] )
	{
	case 0:	PREFSMAN->m_iTextureResolution = 256;	break;
	case 1:	PREFSMAN->m_iTextureResolution = 512;	break;
	case 2:	PREFSMAN->m_iTextureResolution = 1024;	break;
	default:	ASSERT(0);	PREFSMAN->m_iTextureResolution = 512;	break;
	}
	
	PREFSMAN->m_iRefreshRate			= CurrentRefresh();
	PREFSMAN->m_BackgroundMode			= PrefsManager::BackgroundMode( m_iSelectedOption[0][GO_BGMODE] );
	PREFSMAN->m_fBGBrightness			= m_iSelectedOption[0][GO_BGBRIGHTNESS] / 10.0f;
	PREFSMAN->m_iMovieDecodeMS			= m_iSelectedOption[0][GO_MOVIEDECODEMS]+1;
	PREFSMAN->m_bUseBGIfNoBanner		= m_iSelectedOption[0][GO_BGIFNOBANNER] == 1;
	PREFSMAN->m_bVsync					= m_iSelectedOption[0][GO_VSYNC] == 1;
}

void ScreenGraphicOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
	PREFSMAN->SaveGlobalPrefsToDisk();
	ApplyGraphicOptions();
}

void ScreenGraphicOptions::GoToNextState()
{
	GoToPrevState();
}

