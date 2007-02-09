#ifndef StreamDisplay_H
#define StreamDisplay_H

#include "Sprite.h"
#include "Quad.h"

class StreamDisplay : public Actor
{
public:
	StreamDisplay();

	virtual void Update( float fDeltaSecs );

	void Load( 
		float fMeterWidth, 
		float fMeterHeight,
		int iNumStrips,
		int iNumChambers, 
		const RString &sNormalPath, 
		const RString &sHotPath, 
		const RString &sPassingPath,
		const apActorCommands &acNormalOnCommand,
		const apActorCommands &acHotOnCommand,
		const apActorCommands &acPassingOnCommand
		);

	void DrawPrimitives();
	void SetPercent( float fPercent );
	void SetPassingAlpha( float fPassingAlpha );
	void SetHotAlpha( float fHotAlpha );

	float GetTrailingLifePercent() { return m_fTrailingPercent; }

private:
	Sprite		m_sprStreamNormal;
	Sprite		m_sprStreamHot;
	Sprite		m_sprStreamPassing;
	Quad		m_quadMask;

	int m_iNumStrips;
	int m_iNumChambers;
	float m_fPercent;
	float m_fTrailingPercent;	// this approaches m_fPercent
	float m_fVelocity;	// of m_fTrailingPercent
	float m_fPassingAlpha;
	float m_fHotAlpha;

	void GetChamberIndexAndOverflow( float fPercent, int& iChamberOut, float& fChamberOverflowPercentOut );
	float GetChamberLeftPercent( int iChamber );
	float GetChamberRightPercent( int iChamber );
	float GetRightEdgePercent( int iChamber, float fChamberOverflowPercent );
	float GetHeightPercent( int iChamber, float fChamberOverflowPercent );
	void DrawStrip( float fRightEdgePercent );
	void DrawMask( float fPercent );
};

#endif

/*
 * (c) 2003-2004 Chris Danford
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
