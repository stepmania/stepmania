/* PaneDisplay - An Actor that displays song information. */

#ifndef PANE_DISPLAY_H
#define PANE_DISPLAY_H

#include "ActorFrame.h"
#include "ActorUtil.h"
#include "PlayerNumber.h"
#include "BitmapText.h"
#include "AutoActor.h"
#include "GameConstantsAndTypes.h"
#include "ThemeMetric.h"
class XNode;
#include "LocalizedString.h"

/* If the same piece of data is in multiple panes, use separate contents entries,
 * so it can be themed differently. */
enum PaneCategory
{
	PaneCategory_NumSteps,
	PaneCategory_Jumps,
	PaneCategory_Holds,
	PaneCategory_Rolls,
	PaneCategory_Mines,
	PaneCategory_Hands,
	PaneCategory_MachineHighScore,
	PaneCategory_MachineHighName,
	PaneCategory_ProfileHighScore,
	NUM_PaneCategory,
	PaneCategory_Invalid,
};

class PaneDisplay: public ActorFrame
{
public:
	virtual PaneDisplay *Copy() const;

	void Load( const RString &sMetricsGroup, PlayerNumber pn );
	void SetFromGameState();

	void LoadFromNode( const XNode *pNode );

	// Lua
	void PushSelf( lua_State *L );

private:
	void SetContent( PaneCategory c );

	BitmapText		m_textContents[NUM_PaneCategory];
	AutoActor		m_Labels[NUM_PaneCategory];
	ActorFrame		m_ContentsFrame;

	PlayerNumber	m_PlayerNumber;

	LocalizedString EMPTY_MACHINE_HIGH_SCORE_NAME;
	LocalizedString NOT_AVAILABLE;
	ThemeMetric<RString> COUNT_FORMAT;
};

#endif

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
