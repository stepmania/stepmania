#include "global.h"
#include "ScreenCenterImage.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "GameManager.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "HelpDisplay.h"

ScreenCenterImage::ScreenCenterImage( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenCenterImage::ScreenCenterImage()" );

#ifdef _XBOX
	PREFSMAN->resizing = true;
	CStringArray strArray;
	CString text("Use the left analog stick to translate the screen and right right analog stick to scale");
	strArray.push_back(text);
	m_textHelp->SetTips(strArray);
#endif
	
	m_textInstructions.LoadFromFont( THEME->GetPathToF("Common normal") );
	m_textInstructions.SetText( "" );
	m_textInstructions.SetXY( CENTER_X, CENTER_Y );
	m_textInstructions.SetDiffuse( RageColor(0,1,0,0) );
	m_textInstructions.SetZoom( 0.8f );
	this->AddChild( &m_textInstructions );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenCenterImage music") );
}



ScreenCenterImage::~ScreenCenterImage()
{
	LOG->Trace( "ScreenCenterImage::~ScreenCenterImage()" );
#ifdef _XBOX
	PREFSMAN->resizing = false;
#endif
}



void ScreenCenterImage::DrawPrimitives()
{
	Screen::DrawPrimitives();
}

void ScreenCenterImage::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( IsTransitioning() )
		return;

	if( type == IET_RELEASE )
		return;

	bool bHoldingShift = 
		INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_RSHIFT)) ||
		INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, SDLK_LSHIFT));

	int iScale;
	switch( type )
	{
	case IET_SLOW_REPEAT:	iScale = 4;		break;
	case IET_FAST_REPEAT:	iScale = 16;	break;
	default:				iScale = 1;		break;
	}

#ifdef _XBOX
	switch(DeviceI.button)
	{
	case JOY_9:
		if(!IsTransitioning())
		{
			SCREENMAN->PlayStartSound();
			StartTransitioning( SM_GoToNextScreen );		
		}
		break;
	case JOY_10:
		if(!IsTransitioning())
		{
			SCREENMAN->PlayBackSound();
			StartTransitioning( SM_GoToPrevScreen );		
		}
		break;
	}
	return;
#endif

	switch( DeviceI.button )
	{
	case SDLK_SPACE:
		PREFSMAN->m_iCenterImageTranslateX = 0;
		PREFSMAN->m_iCenterImageTranslateY = 0;
		PREFSMAN->m_fCenterImageScaleX = 1;
		PREFSMAN->m_fCenterImageScaleY = 1;
		break;
	case SDLK_LEFT:
		if( bHoldingShift )
			PREFSMAN->m_fCenterImageScaleX -= 0.001f * iScale;
		else
			PREFSMAN->m_iCenterImageTranslateX -= 1 * iScale;
		break;
	case SDLK_RIGHT:
		if( bHoldingShift )
			PREFSMAN->m_fCenterImageScaleX += 0.001f * iScale;
		else
			PREFSMAN->m_iCenterImageTranslateX += 1 * iScale;
		break;
	case SDLK_UP:
		if( bHoldingShift )
			PREFSMAN->m_fCenterImageScaleY -= 0.001f * iScale;
		else
			PREFSMAN->m_iCenterImageTranslateY -= 1 * iScale;
		break;
	case SDLK_DOWN:
		if( bHoldingShift )
			PREFSMAN->m_fCenterImageScaleY += 0.001f * iScale;
		else
			PREFSMAN->m_iCenterImageTranslateY += 1 * iScale;
		break;
	}

	switch( DeviceI.button )
	{
	case SDLK_SPACE:
	case SDLK_LEFT:
	case SDLK_RIGHT:
	case SDLK_UP:
	case SDLK_DOWN:
		DISPLAY->ChangeCentering(
			PREFSMAN->m_iCenterImageTranslateX, 
			PREFSMAN->m_iCenterImageTranslateY,
			PREFSMAN->m_fCenterImageScaleX,
			PREFSMAN->m_fCenterImageScaleY );
		break;
	}

	switch( DeviceI.button )
	{
	case SDLK_ESCAPE:
		if(!IsTransitioning())
		{
			SCREENMAN->PlayBackSound();
			StartTransitioning( SM_GoToPrevScreen );		
		}
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		if(!IsTransitioning())
		{
			SCREENMAN->PlayStartSound();
			StartTransitioning( SM_GoToNextScreen );		
		}
	}
}

void ScreenCenterImage::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToPrevScreen:
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
		break;
	}
}

/*
 * (c) 2003-2004 Chris Danford
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
