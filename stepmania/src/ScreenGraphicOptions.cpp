#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: ScreenGraphicOptions.cpp

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
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
#include "ThemeManager.h"
#include "RageLog.h"


enum {
	GO_WINDOWED = 0,
	GO_PROFILE,
	GO_RESOLUTION,
	GO_TEXTURE_SIZE,
	GO_DISPLAY_COLOR,
	GO_TEXTURE_COLOR,
	GO_BACKGROUNDS,
	NUM_GRAPHIC_OPTIONS_LINES
};
OptionLineData g_GraphicOptionsLines[NUM_GRAPHIC_OPTIONS_LINES] = {
	{ "Mode",			2, {"Fullscreen", "Windowed"} },
	{ "Profile",		5, {"Super Low", "Low", "Medium", "High", "Custom (use settings below)"} },
	{ "Resolution",		7, {"320", "400", "512", "640", "800", "1024", "1280" } },
	{ "Tex. Size",		4, {"256", "512", "1024", "2048"} },
	{ "Display",		2, {"16bit", "32bit"} },
	{ "Tex. Color",		2, {"16bit", "32bit"} },
};

//const int NUm_SelectedOption_LINES = sizeof(g_GraphicOptionsLines)/sizeof(OptionLine);

ScreenGraphicOptions::ScreenGraphicOptions() :
	ScreenOptions(
		THEME->GetPathTo(GRAPHIC_GRAPHIC_OPTIONS_BACKGROUND),
		THEME->GetPathTo(GRAPHIC_GRAPHIC_OPTIONS_TOP_EDGE)
		)
{
	LOG->WriteLine( "ScreenGraphicOptions::ScreenGraphicOptions()" );

	Init( 
		INPUTMODE_BOTH, 
		g_GraphicOptionsLines, 
		NUM_GRAPHIC_OPTIONS_LINES
		);
}

void ScreenGraphicOptions::ImportOptions()
{
	GraphicProfileOptions* pGPO = PREFS->GetCustomGraphicProfileOptions();

	m_iSelectedOption[0][GO_WINDOWED]		= (PREFS->m_bWindowed ? 1:0 );
	m_iSelectedOption[0][GO_PROFILE]		= PREFS->m_GraphicProfile;

	switch( pGPO->m_iWidth )
	{
	case 320:		m_iSelectedOption[0][GO_RESOLUTION] = 0;		break;
	case 400:		m_iSelectedOption[0][GO_RESOLUTION] = 1;		break;
	case 512:		m_iSelectedOption[0][GO_RESOLUTION] = 2;		break;
	case 640:		m_iSelectedOption[0][GO_RESOLUTION] = 3;		break;
	case 800:		m_iSelectedOption[0][GO_RESOLUTION] = 4;		break;
	case 1024:		m_iSelectedOption[0][GO_RESOLUTION] = 5;		break;
	case 1280:		m_iSelectedOption[0][GO_RESOLUTION] = 6;		break;
	default:		m_iSelectedOption[0][GO_RESOLUTION] = 3;		break;
	}
	switch( pGPO->m_iMaxTextureSize )
	{
	case 256:		m_iSelectedOption[0][GO_TEXTURE_SIZE] = 0;		break;
	case 512:		m_iSelectedOption[0][GO_TEXTURE_SIZE] = 1;		break;
	case 1024:		m_iSelectedOption[0][GO_TEXTURE_SIZE] = 2;		break;
	case 2048:		m_iSelectedOption[0][GO_TEXTURE_SIZE] = 3;		break;
	default:		m_iSelectedOption[0][GO_TEXTURE_SIZE] = 1;		break;
	}
	switch( pGPO->m_iDisplayColor )
	{
	case 16:		m_iSelectedOption[0][GO_DISPLAY_COLOR] = 0;		break;
	case 32:		m_iSelectedOption[0][GO_DISPLAY_COLOR] = 1;		break;
	}
	switch( pGPO->m_iTextureColor )
	{
	case 16:		m_iSelectedOption[0][GO_TEXTURE_COLOR] = 0;		break;
	case 32:		m_iSelectedOption[0][GO_TEXTURE_COLOR] = 1;		break;
	}
}

void ScreenGraphicOptions::ExportOptions()
{
	GraphicProfileOptions* pGPO = PREFS->GetCustomGraphicProfileOptions();

	PREFS->m_bWindowed			= (m_iSelectedOption[0][GO_WINDOWED] == 1);
	PREFS->m_GraphicProfile		= (GraphicProfile)m_iSelectedOption[0][GO_PROFILE];

	switch( m_iSelectedOption[0][GO_RESOLUTION] )
	{
	case 0:		pGPO->m_iWidth = 320;		break;
	case 1:		pGPO->m_iWidth = 400;		break;
	case 2:		pGPO->m_iWidth = 512;		break;
	case 3:		pGPO->m_iWidth = 640;		break;
	case 4:		pGPO->m_iWidth = 800;		break;
	case 5:		pGPO->m_iWidth = 1024;		break;
	case 6:		pGPO->m_iWidth = 1280;		break;
	default:	ASSERT( false );
	}
	pGPO->m_iHeight = GetHeightFromWidth( pGPO->m_iWidth );

	switch( m_iSelectedOption[0][GO_TEXTURE_SIZE] )
	{
	case 0:		pGPO->m_iMaxTextureSize = 256;		break;
	case 1:		pGPO->m_iMaxTextureSize = 512;		break;
	case 2:		pGPO->m_iMaxTextureSize = 1024;		break;
	case 3:		pGPO->m_iMaxTextureSize = 2048;		break;
	default:	ASSERT( false );
	}
	switch( m_iSelectedOption[0][GO_DISPLAY_COLOR] )
	{
	case 0:		pGPO->m_iDisplayColor = 16;	break;
	case 1:		pGPO->m_iDisplayColor = 32;	break;
	default:	ASSERT( false );
	}
	switch( m_iSelectedOption[0][GO_TEXTURE_COLOR] )
	{
	case 0:		pGPO->m_iTextureColor = 16;	break;
	case 1:		pGPO->m_iTextureColor = 32;	break;
	default:	ASSERT( false );
	}

	//
	// put the options into effect
	//
	ApplyGraphicOptions(); 
}

void ScreenGraphicOptions::GoToPrevState()
{
	SCREENMAN->SetNewScreen( new ScreenTitleMenu );
}

void ScreenGraphicOptions::GoToNextState()
{
	SCREENMAN->SetNewScreen( new ScreenTitleMenu );
}





