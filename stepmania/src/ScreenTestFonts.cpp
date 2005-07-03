#include "global.h"
#include "ScreenTestFonts.h"
#include "FontManager.h"
#include "RageTextureManager.h"
#include "ScreenTextEntry.h"
#include "GameConstantsAndTypes.h"
#include "ScreenDimensions.h"


static const float LineWidth = 400;
static const float LineHeight = 50;


CString CustomText;

void ChangeText(CString txt)
{
	CustomText = txt;
}

AutoScreenMessage( SM_ChangeText )

void ScreenTestFonts::HandleScreenMessage( const ScreenMessage SM )
{
	if(SM == ScreenMessage(SM_ChangeText))
		SetText(CustomText);
}

REGISTER_SCREEN_CLASS( ScreenTestFonts );
ScreenTestFonts::ScreenTestFonts( CString sClassName ) : Screen( sClassName )
{
}

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
	font.LoadFromFont( "Themes/default/Fonts/Common Normal" );
	font.SetZoom(.5);
	this->AddChild(&font);

	txt.SetXY( SCREEN_CENTER_X, SCREEN_CENTER_Y );
	SetFont( "Themes/default/Fonts/Common Normal" );
	SetText( "Foo" );
}

void ScreenTestFonts::SetText(CString text)
{
	txt.SetShadowLength( 0 );
	txt.SetText(""); /* force it */
	txt.SetText(text);
	curtext = text;
}
void ScreenTestFonts::SetFont(CString font_)
{
	curfont = font_;

	txt.LoadFromFont(curfont);
	font.SetText(curfont);
	/* The font changed, so we need to reset the text or it'll be
	 * misaligned. */
	SetText(curtext);
}

void ScreenTestFonts::Draw()
{
	/* Draw this manually, so we can breakpoint here ... */
	txt.Draw();
	Screen::Draw();
}


void ScreenTestFonts::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS )
		return;
	switch( DeviceI.button )
	{
	case '[': txt.SetVertAlign(align_bottom); break;
	case '\\': txt.SetVertAlign(align_middle); break;
	case ']': txt.SetVertAlign(align_top); break;

	case ',': txt.SetHorizAlign(align_left); break;
	case '.': txt.SetHorizAlign(align_center); break;
	case '/': txt.SetHorizAlign(align_right); break;

	case '`': if(curtext != CustomText)
				  SetText(CustomText);
			  else
				  ScreenTextEntry::TextEntry( SM_ChangeText, "Edit text.", CustomText, 100, NULL, ChangeText, NULL);
			  break;
	case '1': SetText("Waaai"); break;
	case '2': SetText("WAAI &#9769;"); break;
	case '3': SetText("WAAI &#93bc;"); break;

	case '4': SetText("WAAI\nWAAI"); break;
	case '5': SetText("WAAI &#93bc;\nWAAI"); break;

	case 'q': SetFont( "Themes/default/Fonts/_shared2" ); break;
	case 'w': SetFont( "Themes/default/Fonts/Common Normal" ); break;
	case 'e': SetFont( "Themes/default/Fonts/MusicList titles" ); break;
	case 'r': SetFont( "Themes/default/Fonts/_shared1" ); break;
	case 't': SetFont( "Themes/default/Fonts/ScreenRanking letters" ); break;

	case 'z': FONT->ReloadFonts();
			  TEXTUREMAN->ReloadAll();
			  SetText(curtext);
			  SetFont(curfont);
			  break;
	}
}

/*
 * (c) 2003 Glenn Maynard
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
