#ifndef SCREEN_TEST_FONTS_H
#define SCREEN_TEST_FONTS_H

#include "Screen.h"
#include "BitmapText.h"
#include "Quad.h"

class ScreenTestFonts: public Screen
{
public:
	ScreenTestFonts();

	void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	void Draw();
	void HandleScreenMessage( const ScreenMessage SM );

	void SetText(CString txt);
	void SetFont(CString font);

	CString			curtext, curfont;
	BitmapText txt, font;
	Quad Vline, Hline;
};

#endif
