#ifndef OPTIONS_CURSOR_H
#define OPTIONS_CURSOR_H

#include "Sprite.h"
#include "ActorFrame.h"
#include "PlayerNumber.h"
#include "AutoActor.h"
/** @brief A cursor for ScreenOptions. */
class OptionsCursor : public ActorFrame
{
public:
	/** @brief Set up a default OptionsCursor. */
	OptionsCursor();
	/**
	 * @brief Set up an OptionsCursor based on an existing copy.
	 * @param cpy the OptionsCursor we are copying. */
	OptionsCursor( const OptionsCursor &cpy );

	void Load( const RString &sMetricsGroup, bool bLoadCanGos );

	void StopTweening();
	void BeginTweening( float fSecs );
	void SetBarWidth( int iWidth );
	int GetBarWidth() const;
	void SetCanGo( bool bCanGoLeft, bool bCanGoRight );

protected:
	AutoActor m_sprMiddle;
	AutoActor m_sprLeft;
	AutoActor m_sprRight;

	AutoActor m_sprCanGoLeft;
	AutoActor m_sprCanGoRight;

	// save the metrics-set X because it gets obliterated on a call to SetBarWidth
	int m_iOriginalLeftX;
	int m_iOriginalRightX;
	int m_iOriginalCanGoLeftX;
	int m_iOriginalCanGoRightX;
};

#endif

/*
 * (c) 2001-2003 Chris Danford
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
