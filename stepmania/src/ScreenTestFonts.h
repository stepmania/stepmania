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
	Quad Vline, Hline;
	void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
};

#endif
