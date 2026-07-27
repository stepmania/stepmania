#ifndef ModIcon_H
#define ModIcon_H

#include "ActorFrame.h"
#include "AutoActor.h"
#include "BitmapText.h"
#include "PlayerNumber.h"
#include "ThemeMetric.h"

#include <vector>


/** @brief Shows PlayerOptions and SongOptions in icon form. */
class ModIcon : public ActorFrame
{
public:
	ModIcon();
	ModIcon( const ModIcon &cpy );
	void Load( RString sMetricsGroup );
	void Set( const RString &sText );

protected:
	BitmapText	m_text;
	AutoActor	m_sprFilled;
	AutoActor	m_sprEmpty;

	ThemeMetric<int> CROP_TEXT_TO_WIDTH;
	ThemeMetric<RString> STOP_WORDS;
	std::vector<RString> m_vStopWords;
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
