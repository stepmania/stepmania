#include "stdafx.h"

#include "ScreenTestFonts.h"
#include "FontManager.h"

static const float LineWidth = 400;
static const float LineHeight = 50;

ScreenTestFonts::ScreenTestFonts()
{
	Hline.SetXY(CENTER_X, CENTER_Y);
	Hline.SetZoomX(LineWidth);
	Hline.SetDiffuse( RageColor(1, 1, 1, 1) );
	this->AddChild(&Hline);

	Vline.SetXY(CENTER_X, CENTER_Y);
	Vline.SetZoomY(LineHeight);
	Vline.SetDiffuse( RageColor(0, 1, 0, .8f) );
	this->AddChild(&Vline);
	
	curfont.SetXY( CENTER_X, CENTER_Y+100 );
	curfont.LoadFromFont( "Themes/default/Fonts/header1" );
	curfont.SetZoom(.5);
	this->AddChild(&curfont);

	txt.SetXY( CENTER_X, CENTER_Y );
	SetFont( "Themes/default/Fonts/header1" );
	SetText( "Foo" );
}

void ScreenTestFonts::SetText(CString text)
{
	txt.SetText(""); /* force it */
	txt.SetText(text);
	curtext = text;
}
void ScreenTestFonts::SetFont(CString font)
{
	txt.LoadFromFont( font );
	curfont.SetText(font);
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

	case '1': SetText("Waaai"); break;
		/* These two kanji are currently two different sizes (20 and 32 pix),
		 * and they should be vertically centered with the other text: */
	case '2': SetText("WAAI &kakumei1;"); break;
	case '3': SetText("WAAI &oni;"); break;

	case '4': SetText("WAAI\nWAAI"); break;
	case '5': SetText("WAAI &oni;\nWAAI"); break;

	case 'q': SetFont( "Themes/default/Fonts/header1" ); break;
	case 'w': SetFont( "Themes/default/Fonts/header2" ); break;
	case 'e': SetFont( "Themes/default/Fonts/Normal" ); break;
	case 'r': SetFont( "Themes/SMMAX2/Fonts/titlemenu" ); break;

	case 'z': FONT->ReloadFonts(); SetText(curtext); break;
	}
}

