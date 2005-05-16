/* ScreenDebugOverlay - credits and statistics drawn on top of everything else. */

#ifndef ScreenDebugOverlay_H
#define ScreenDebugOverlay_H

#include "Screen.h"
#include "BitmapText.h"
#include "Quad.h"

enum DebugLine
{
	DebugLine_Autoplay,
	DebugLine_CoinMode,
	DebugLine_Slow,
	DebugLine_Halt,
	DebugLine_LightsDebug,
	DebugLine_MonkeyInput,
	DebugLine_Stats,
	DebugLine_Vsync,
	DebugLine_ScreenTestMode,
	DebugLine_ClearMachineStats,
	DebugLine_FillMachineStats,
	DebugLine_SendNotesEnded,
	DebugLine_Volume,
	DebugLine_CurrentScreen,
	DebugLine_Uptime,
	NUM_DEBUG_LINES
};
#define FOREACH_DebugLine( i ) FOREACH_ENUM( DebugLine, NUM_DEBUG_LINES, i )

class ScreenDebugOverlay : public Screen
{
public:
	ScreenDebugOverlay( const CString &sName );
	virtual ~ScreenDebugOverlay();
	virtual void Init();
	
	bool OverlayInput( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );

	void Update( float fDeltaTime );

private:
	Quad m_Quad;
	BitmapText m_Header;
	BitmapText m_Text[NUM_DEBUG_LINES];
};


#endif

/*
 * (c) 2001-2005 Chris Danford, Glenn Maynard
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
