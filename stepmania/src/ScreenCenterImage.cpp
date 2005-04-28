#include "global.h"
#include "ScreenCenterImage.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameManager.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "HelpDisplay.h"
#include "ScreenDimensions.h"

REGISTER_SCREEN_CLASS( ScreenCenterImage );
ScreenCenterImage::ScreenCenterImage( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenCenterImage::ScreenCenterImage()" );

	ZERO( m_fScale );
}

void ScreenCenterImage::Init()
{
	ScreenWithMenuElements::Init();

#if defined(XBOX)
	CStringArray strArray;
	CString text("Use the left analog stick to translate the screen and right right analog stick to scale");
	strArray.push_back(text);
	m_textHelp->SetTips(strArray);
#endif
	
	m_textInstructions.LoadFromFont( THEME->GetPathF("Common","normal") );
	m_textInstructions.SetText( "" );
	m_textInstructions.SetXY( SCREEN_CENTER_X, SCREEN_CENTER_Y );
	m_textInstructions.SetDiffuse( RageColor(0,1,0,0) );
	m_textInstructions.SetZoom( 0.8f );
	this->AddChild( &m_textInstructions );

	SOUND->PlayMusic( THEME->GetPathS("ScreenCenterImage","music") );
}


ScreenCenterImage::~ScreenCenterImage()
{
	LOG->Trace( "ScreenCenterImage::~ScreenCenterImage()" );
}



void ScreenCenterImage::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( IsTransitioning() )
		return;

	if( type == IET_FIRST_PRESS  )
	{
		if( DeviceI.device == DEVICE_KEYBOARD && DeviceI.button == KEY_SPACE )
		{
			PREFSMAN->m_iCenterImageTranslateX = 0;
			PREFSMAN->m_iCenterImageTranslateY = 0;
			PREFSMAN->m_fCenterImageAddWidth = 0;
			PREFSMAN->m_fCenterImageAddHeight = 0;
			return;
		}

		switch( MenuI.button )
		{
		case MENU_BUTTON_START:
			SCREENMAN->PlayStartSound();
			StartTransitioning( SM_GoToNextScreen );		
			return;

		case MENU_BUTTON_BACK:
			StartTransitioning( SM_GoToPrevScreen );		
			return;
		}
	}

	bool bIncrease = false;

	Axis axis = NUM_AXES;
	switch( MenuI.button )
	{
	case MENU_BUTTON_UP:	axis = AXIS_TRANS_Y; bIncrease = false;	break;
	case MENU_BUTTON_DOWN:	axis = AXIS_TRANS_Y; bIncrease = true;	break;
	case MENU_BUTTON_LEFT:	axis = AXIS_TRANS_X; bIncrease = false;	break;
	case MENU_BUTTON_RIGHT:	axis = AXIS_TRANS_X; bIncrease = true;	break;
	default:
		if( !DeviceI.IsJoystick() )
			return;

		/* Secondary axes aren't always mapped correctly; for example, PS2 converters
		 * tend to map the right stick to weird things, and every one is different,
		 * so they usually won't work. */
		switch( DeviceI.button )
		{
		case JOY_UP:		axis = AXIS_TRANS_Y;	bIncrease = false;	break;
		case JOY_DOWN:		axis = AXIS_TRANS_Y;	bIncrease = true;	break;
		case JOY_LEFT:		axis = AXIS_TRANS_X;	bIncrease = false;	break;
		case JOY_RIGHT:		axis = AXIS_TRANS_X;	bIncrease = true;	break;
		case JOY_UP_2:		axis = AXIS_SCALE_Y;	bIncrease = false;	break;
		case JOY_DOWN_2:	axis = AXIS_SCALE_Y;	bIncrease = true;	break;
		case JOY_LEFT_2:	axis = AXIS_SCALE_X;	bIncrease = false;	break;
		case JOY_RIGHT_2:	axis = AXIS_SCALE_X;	bIncrease = true;	break;
		}
	}
	
	if( axis == NUM_AXES )
		return;

	if( INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_RSHIFT)) ||
		INPUTFILTER->IsBeingPressed( DeviceInput(DEVICE_KEYBOARD, KEY_LSHIFT)) )
	{
		const Axis switch_axis[] = { AXIS_SCALE_X, AXIS_SCALE_Y, AXIS_TRANS_X, AXIS_TRANS_Y };
		axis = switch_axis[axis];
	}

	float fScale = 1.0f;
	if( type == IET_RELEASE )
		fScale = 0;

	if( DeviceI.level >= 0 )
	{
		/* Increase the dead zone.  XXX: input drivers should handle dead zones */
		if( DeviceI.level < 0.10f )
			fScale = 0;
		else
			fScale = SCALE( DeviceI.level, 0.10f, 1.0f, 0.0f, 1.0f );
	}

	if( DeviceI.level < 0 )
	{
		switch( type )
		{
		case IET_SLOW_REPEAT:	fScale *= 4;	break;
		case IET_FAST_REPEAT:	fScale *= 16;	break;
		}

		if( !bIncrease )
			fScale *= -1;

		Move( axis, fScale );
		return;
	}

	fScale = SCALE( fScale, 0.0f, 1.0f, 0.0f, 25.0f );
	fScale = powf( fScale, 2.0f );

	if( !bIncrease )
		fScale *= -1;

	m_fScale[axis] = fScale;
}

void ScreenCenterImage::Move( Axis axis, float fDelta )
{
	Preference<int> *piValues[4] =
	{
		&PREFSMAN->m_iCenterImageTranslateX,
		&PREFSMAN->m_iCenterImageTranslateY,
		&PREFSMAN->m_fCenterImageAddWidth,
		&PREFSMAN->m_fCenterImageAddHeight
	};

	*piValues[axis] = *piValues[axis] + lrintf( fDelta );

	DISPLAY->ChangeCentering(
		PREFSMAN->m_iCenterImageTranslateX, 
		PREFSMAN->m_iCenterImageTranslateY,
		PREFSMAN->m_fCenterImageAddWidth,
		PREFSMAN->m_fCenterImageAddHeight );

	CString sMessage = 
		ssprintf( "Centering: x=%d, y=%d, width=%d, height=%d",
		PREFSMAN->m_iCenterImageTranslateX.Value(), 
		PREFSMAN->m_iCenterImageTranslateY.Value(),
		PREFSMAN->m_fCenterImageAddWidth.Value(),
		PREFSMAN->m_fCenterImageAddHeight.Value() );
	SCREENMAN->SystemMessageNoAnimate( sMessage );
}

void ScreenCenterImage::Update( float fDeltaTime )
{
	for( int i = 0; i < NUM_AXES; ++i )
		Move( (Axis) i, m_fScale[i] * fDeltaTime );

	Screen::Update( fDeltaTime );
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
