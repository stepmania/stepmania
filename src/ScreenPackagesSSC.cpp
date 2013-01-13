#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenPackagesSSC.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "RageLog.h"
#include "ScreenManager.h"

REGISTER_SCREEN_CLASS( ScreenPackagesSSC );

void ScreenPackagesSSC::Init()
{
	ScreenWithMenuElements::Init();
}

void ScreenPackagesSSC::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToPrevScreen )
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "PrevScreen") );
	else if( SM ==SM_GoToNextScreen )
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "NextScreen") );

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

bool ScreenPackagesSSC::Input( const InputEventPlus &input )
{
	return ScreenWithMenuElements::Input( input );
}

bool ScreenPackagesSSC::MenuBack( const InputEventPlus &input )
{
	TweenOffScreen();
	Cancel( SM_GoToPrevScreen );
	ScreenWithMenuElements::MenuBack( input );
	return true;
}

void ScreenPackagesSSC::Update( float fDeltaTime )
{
	ScreenWithMenuElements::Update(fDeltaTime);
}

void ScreenPackagesSSC::DrawPrimitives()
{
	ScreenWithMenuElements::DrawPrimitives();
}

RString ScreenPackagesSSC::JSONParse( const RString &string_in )
{
	/* json++ stuff here */
	return "";
}

#endif
/*
 * (c) 2009 Colby Klein
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
