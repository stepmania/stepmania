#include "stdafx.h"

#include "ScreenTestFonts.h"

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
	
	txt.SetXY( CENTER_X, CENTER_Y );
	txt.LoadFromFont( "Themes/default/Fonts/header1" );
	txt.SetText( "Foo" );
	this->AddChild(&txt);
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

	case '1': txt.SetText("Waaai"); break;
		/* These two kanji are currently two different sizes (20 and 32 pix),
		 * and they should be vertically centered with the other text: */
	case '2': txt.SetText("WAAI &kakumei1;"); break;
	case '3': txt.SetText("WAAI &oni;"); break;

	case '4': txt.SetText("WAAI\nWAAI"); break;
	case '5': txt.SetText("WAAI &oni;\nWAAI"); break;

	case 'q': txt.LoadFromFont( "Themes/default/Fonts/header1" ); break;
	case 'w': txt.LoadFromFont( "Themes/default/Fonts/header2" ); break;
	case 'e': txt.LoadFromFont( "Themes/default/Fonts/Normal" ); break;
	case 'r': txt.LoadFromFont( "Themes/default/Fonts/titlemenu" ); break;
	}
}

