#ifndef SCREEN_TEST_FONTS_H
#define SCREEN_TEST_FONTS_H

#include "Screen.h"
#include "BitmapText.h"
#include "Quad.h"

class ScreenTestFonts: public Screen
{
public:
	ScreenTestFonts();

	BitmapText txt;
	BitmapText curfont;
	Quad Vline, Hline;
	void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	void Draw();
	CString			curtext;
	void SetText(CString txt);
	void SetFont(CString font);
};

#endif
