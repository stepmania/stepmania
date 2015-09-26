/* ComboGraph - A bar displaying the player's combo on Evaluation. */
#ifndef COMBO_GRAPH_H
#define COMBO_GRAPH_H

#include "ActorFrame.h"
#include "PlayerNumber.h"
#include "ThemeMetric.h"

class StageStats;
class PlayerStageStats;
class BitmapText;

class ComboGraph: public ActorFrame
{
public:
	ComboGraph();
	void Load( RString sMetricsGroup );
	void Set( const StageStats &s, const PlayerStageStats &pss );
	virtual ComboGraph *Copy() const;
	virtual bool AutoLoadChildren() const { return true; }

	// Commands
	virtual void PushSelf( lua_State *L );

private:
	ThemeMetric<float> BODY_WIDTH;
	ThemeMetric<float> BODY_HEIGHT;
	Actor *m_pBacking;
	Actor *m_pNormalCombo;
	Actor *m_pMaxCombo;
	BitmapText *m_pComboNumber;
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
