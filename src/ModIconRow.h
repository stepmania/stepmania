/* ModIconRow - Shows a row of ModIcons. */

#ifndef ModIconRow_H
#define ModIconRow_H

#include "ActorFrame.h"
#include "ModIcon.h"
#include "ThemeMetric.h"

#include <vector>


class PlayerOptions;
struct lua_State;

class ModIconRow : public ActorFrame
{
public:
	ModIconRow();
	~ModIconRow();

	void Load( const RString &sMetricsGroup, PlayerNumber pn );

	virtual ModIconRow *Copy() const;
	void SetFromGameState();

	virtual void HandleMessage( const Message &msg );

	// Commands
	virtual void PushSelf( lua_State *L );

protected:
	RString m_sMetricsGroup;
	PlayerNumber m_pn;

	ThemeMetric<float>	SPACING_X;
	ThemeMetric<float>	SPACING_Y;
	ThemeMetric<int>	NUM_OPTION_ICONS;
	ThemeMetric<RString>	OPTION_ICON_METRICS_GROUP;

	std::vector<ModIcon*> m_vpModIcon;
};

#endif

/*
 * (c) 2002-2004 Chris Danford
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
