/* GradeDisplay - Grade shown on ScreenEvaluation. */

#ifndef GRADE_DISPLAY_H
#define GRADE_DISPLAY_H

#include "Sprite.h"
#include "Grade.h"
#include "PlayerNumber.h"


class GradeDisplay : public Sprite
{
public:
	GradeDisplay();
	virtual bool Load( RageTextureID ID );

	virtual void Update( float fDeltaTime );

	void SetGrade( PlayerNumber pn, Grade g );
	void Spin();
	void SettleImmediately();
	void SettleQuickly();

	Grade GetGrade () const { return m_Grade; }

protected:
	int GetFrameIndex( PlayerNumber pn, Grade g );

	PlayerNumber m_PlayerNumber;
	Grade m_Grade;

	// for scrolling; 0 = no, 1 = normal, 2 = quick
	int  m_bDoScrolling;
	RectF m_frectStartTexCoords;
	RectF m_frectDestTexCoords;
	RectF m_frectCurTexCoords;
	float m_fTimeLeftInScroll;
};

#endif

/*
 * (c) 2001-2002 Chris Danford
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
