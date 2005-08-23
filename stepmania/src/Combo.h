/* Combo - Text that displays the size of the current combo. */

#ifndef COMBO_H
#define COMBO_H

#include "ActorFrame.h"
#include "AutoActor.h"
#include "BitmapText.h"
#include "PlayerNumber.h"
#include "ThemeMetric.h"
class PlayerState;
class PlayerStageStats;

class Combo : public ActorFrame
{
public:
	Combo();

	void Load( PlayerState *pPlayerState, PlayerStageStats *pPlayerStageStats );

	void SetCombo( int iCombo, int iMisses );

protected:
	ThemeMetric<int>				SHOW_COMBO_AT;
	ThemeMetric<float>				LABEL_X;
	ThemeMetric<float>				LABEL_Y;
	ThemeMetric<apActorCommands>	LABEL_ON_COMMAND;
	ThemeMetric<float>				NUMBER_X;
	ThemeMetric<float>				NUMBER_Y;
	ThemeMetric<apActorCommands>	NUMBER_ON_COMMAND;
	ThemeMetric<float>				NUMBER_MIN_ZOOM;
	ThemeMetric<float>				NUMBER_MAX_ZOOM;
	ThemeMetric<float>				NUMBER_MAX_ZOOM_AT;
	ThemeMetric<apActorCommands>	PULSE_COMMAND;
	ThemeMetric<float>				TWEEN_SECONDS;
	ThemeMetric<apActorCommands>	FULL_COMBO_GREATS_COMMAND;
	ThemeMetric<apActorCommands>	FULL_COMBO_PERFECTS_COMMAND;
	ThemeMetric<apActorCommands>	FULL_COMBO_MARVELOUSES_COMMAND;
	ThemeMetric<apActorCommands>	FULL_COMBO_BROKEN_COMMAND;
	ThemeMetric<bool>				SHOW_MISS_COMBO;

	PlayerState *m_pPlayerState;
	PlayerStageStats *m_pPlayerStageStats;

	AutoActor	m_spr100Milestone;
	AutoActor	m_spr1000Milestone;
	AutoActor	m_sprComboLabel;
	AutoActor	m_sprMissesLabel;
	BitmapText	m_textNumber;

	int m_iLastSeenCombo;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
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
