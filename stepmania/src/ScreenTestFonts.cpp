#include "global.h"

#include "ScreenTestFonts.h"
#include "FontManager.h"
#include "RageTextureManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"


static const float LineWidth = 400;
static const float LineHeight = 50;


CString CustomText;

void ChangeText(CString txt)
{
	CustomText = txt;
}
const ScreenMessage SM_ChangeText		=	ScreenMessage(SM_User+1);

void ScreenTestFonts::HandleScreenMessage( const ScreenMessage SM )
{
	if(SM == ScreenMessage(SM_ChangeText))
		SetText(CustomText);
}

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
	
	font.SetXY( CENTER_X, CENTER_Y+100 );
	font.LoadFromFont( "Themes/default/Fonts/header1" );
	font.SetZoom(.5);
	this->AddChild(&font);

	txt.SetXY( CENTER_X, CENTER_Y );
	SetFont( "Themes/default/Fonts/header1" );
	SetText( "Foo" );
}

void ScreenTestFonts::SetText(CString text)
{
	txt.EnableShadow( false );
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
				  SCREENMAN->TextEntry( SM_ChangeText, "Edit text.", CustomText, ChangeText, NULL);
			  break;
	case '1': SetText("Waaai"); break;
	case '2': SetText("WAAI &kakumei1;"); break;
	case '3': SetText("WAAI &oni;"); break;

	case '4': SetText("WAAI\nWAAI"); break;
	case '5': SetText("WAAI &oni;\nWAAI"); break;

	case 'q': SetFont( "Themes/default/Fonts/_common11" ); break;
	case 'w': SetFont( "Themes/default/Fonts/_common2" ); break;
	case 'e': SetFont( "Themes/default/Fonts/Normal" ); break;
	case 'r': SetFont( "Themes/SMMAX2/Fonts/titlemenu" ); break;
	case 't': SetFont( "Themes/default/Fonts/small titles" ); break;

	case 'z': FONT->ReloadFonts();
			  TEXTUREMAN->ReloadAll();
			  SetText(curtext);
			  SetFont(curfont);
			  break;
	}
}

