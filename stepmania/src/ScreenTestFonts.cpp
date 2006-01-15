#include "global.h"
#include "ScreenTestFonts.h"
#include "FontManager.h"
#include "RageTextureManager.h"
#include "ScreenTextEntry.h"
#include "GameConstantsAndTypes.h"
#include "ScreenDimensions.h"
#include "InputEventPlus.h"
#include "RageUtil.h"


static const float LineWidth = 400;
static const float LineHeight = 50;

#define FONT( i )			THEME->GetMetric (m_sName,ssprintf("Font%i",i))
#define TEXT( i )			THEME->GetMetric (m_sName,ssprintf("Text%i",i))

static CString g_sCustomText;

static void ChangeText( const CString &sText )
{
	g_sCustomText = sText;
}

AutoScreenMessage( SM_ChangeText )

void ScreenTestFonts::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == ScreenMessage(SM_ChangeText) )
		SetText( g_sCustomText );
}

REGISTER_SCREEN_CLASS_NEW( ScreenTestFonts );

void ScreenTestFonts::Init()
{
	Screen::Init();

	Hline.SetXY(SCREEN_CENTER_X, SCREEN_CENTER_Y);
	Hline.SetZoomX(LineWidth);
	Hline.SetDiffuse( RageColor(1, 1, 1, 1) );
	this->AddChild(&Hline);

	Vline.SetXY(SCREEN_CENTER_X, SCREEN_CENTER_Y);
	Vline.SetZoomY(LineHeight);
	Vline.SetDiffuse( RageColor(0, 1, 0, .8f) );
	this->AddChild(&Vline);
	
	font.SetXY( SCREEN_CENTER_X, SCREEN_CENTER_Y+100 );
	font.LoadFromFont( THEME->GetPathF("Common", "normal") );
	font.SetZoom(.5);
	this->AddChild(&font);

	txt.SetName( "Text" );
	SetFont( THEME->GetPathF("", FONT(1)) );
	SET_XY_AND_ON_COMMAND( txt );
	SetText( "Foo" );
}

void ScreenTestFonts::SetText( CString sText )
{
	txt.SetShadowLength( 0 );
	txt.SetText( "" ); /* force it */
	txt.SetText( sText );
	m_sCurText = sText;
}

void ScreenTestFonts::SetFont( CString sFont )
{
	m_sFont = sFont;

	txt.LoadFromFont( m_sFont );
	font.SetText( m_sFont );
	/* The font changed, so we need to reset the text or it'll be
	 * misaligned. */
	SetText( m_sCurText );
}

void ScreenTestFonts::DrawPrimitives()
{
	Screen::DrawPrimitives();

	/* Draw this manually, so we can breakpoint here. */
	txt.Draw();
}


void ScreenTestFonts::Input( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS )
		return;
	switch( input.DeviceI.button )
	{
	case '[': txt.SetVertAlign( align_bottom ); break;
	case '\\': txt.SetVertAlign( align_middle ); break;
	case ']': txt.SetVertAlign( align_top ); break;

	case ',': txt.SetHorizAlign( align_left ); break;
	case '.': txt.SetHorizAlign( align_center ); break;
	case '/': txt.SetHorizAlign( align_right ); break;

	case '`': if( m_sCurText != g_sCustomText )
				  SetText( g_sCustomText );
			  else
				  ScreenTextEntry::TextEntry( SM_ChangeText, "Edit text.", g_sCustomText, 100, NULL, ChangeText, NULL);
			  break;
	case '1': SetText( TEXT(1) ); break;
	case '2': SetText( TEXT(2) ); break;
	case '3': SetText( TEXT(3) ); break;
	case '4': SetText( TEXT(4) ); break;
	case '5': SetText( TEXT(5) ); break;

	case 'q': SetFont( THEME->GetPathF("", FONT(1)) ); break;
	case 'w': SetFont( THEME->GetPathF("", FONT(2)) ); break;
	case 'e': SetFont( THEME->GetPathF("", FONT(3)) ); break;
	case 'r': SetFont( THEME->GetPathF("", FONT(4)) ); break;
	case 't': SetFont( THEME->GetPathF("", FONT(5)) ); break;

	case 'a': SetFont( THEME->GetPathF("", FONT(1)) ); break;

	case 'z': FONT->ReloadFonts();
			  TEXTUREMAN->ReloadAll();
			  SetText( m_sCurText );
			  SetFont( m_sFont );
			  break;
	}
}

/*
 * (c) 2003-2005 Glenn Maynard
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
