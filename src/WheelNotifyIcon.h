#ifndef WHEEL_NOTIFY_ICON_H
#define WHEEL_NOTIFY_ICON_H

#include "Sprite.h"

#include <vector>


/** @brief A little graphic to the side of the song's text banner in the MusicWheel. */
class WheelNotifyIcon : public Sprite
{
public:
	WheelNotifyIcon();

	struct Flags
	{
		Flags() { bHasBeginnerOr1Meter = bEdits = false; iPlayersBestNumber = iStagesForSong = 0; }
		bool bHasBeginnerOr1Meter;
		int iPlayersBestNumber;
		bool bEdits;
		int iStagesForSong;
	};

	void SetFlags( Flags flags );

	virtual void Update( float fDeltaTime );
	virtual bool EarlyAbortDraw() const;

protected:
	/** @brief What types of icons are available for the Song? */
	enum Icons
	{
		training=0, /**< This song is used for training new Players. */
		best1, /**< This is the most popular Song to play. */
		best2, /**< This is the second most popular Song to play. */
		best3, /**< This is the third most popular Song to play. */
		edits, /**< This Song has edits available. */
		long_ver, /**< This Song is a long version, and will take 2 Songs in most modes. */
		marathon, /**< This Song is a marathon version, and will take 3 Songs in most modes. */
		empty /**< There is no icon meant for this Song. */
	};

	/** @brief the list of Icons to show. */
	std::vector<Icons> m_vIconsToShow;
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
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
