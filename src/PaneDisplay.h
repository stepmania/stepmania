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

/**
 * @brief The various categories used to display data on Steps.
 *
 * If the same piece of data is in multiple panes, use separate contents entries,
 * so it can be themed differently. */
enum PaneCategory
{
	PaneCategory_NumSteps, /**< The number of steps for the chart. */
	PaneCategory_Jumps, /**< The number of jumps for the chart. */
	PaneCategory_Holds, /**< The number of holds for the chart. */
	PaneCategory_Rolls, /**< The number of rolls for the chart. */
	PaneCategory_Mines, /**< The number of mines for the chart. */
	PaneCategory_Hands, /**< The number of hands for the chart. */
	PaneCategory_Lifts, /**< The number of lifts for the chart. */
	PaneCategory_Fakes, /**< The number of fakes for the chart. */
	PaneCategory_MachineHighScore, /**< The high score on the machine. */
	PaneCategory_MachineHighName, /**< The name associated with the machine high score. */
	PaneCategory_ProfileHighScore, /**< The personal profile's highest score. */
	NUM_PaneCategory,
	PaneCategory_Invalid,
};
/** @brief An Actor that displays Song information. */
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
	void GetPaneTextAndLevel( PaneCategory c, RString & sTextOut, float & fLevelOut );
	void SetContent( PaneCategory c );

	BitmapText		m_textContents[NUM_PaneCategory];
	AutoActor		m_Labels[NUM_PaneCategory];
	ActorFrame		m_ContentsFrame;

	PlayerNumber	m_PlayerNumber;

	LocalizedString EMPTY_MACHINE_HIGH_SCORE_NAME;
	LocalizedString NOT_AVAILABLE;
	ThemeMetric<RString> COUNT_FORMAT;
	ThemeMetric<RString> NULL_COUNT_STRING;
};

#endif

/**
 * @file
 * @author Glenn Maynard (c) 2003
 * @section LICENSE
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
