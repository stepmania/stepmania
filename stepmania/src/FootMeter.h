/*
-----------------------------------------------------------------------------
 File: FootMeter.h

 Desc: The song's foot difficulty displayed in SelectSteps.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _FootBar_H_
#define _FootBar_H_


#include "Sprite.h"
#include "BitmapText.h"


const int MAX_NUM_FEET = 9;


class FootMeter : public Sprite
{
public:

	void SetNumFeet( int iNumFeet )
	{
		m_iNumFeet = iNumFeet;
	}

	void Draw()
	{
		float fCenterX = GetX();
		float fCenterY = GetY();
		float fWidth = GetZoomedWidth() * 0.8f;
		D3DXCOLOR color = GetColor();

		for( int i=0; i<MAX_NUM_FEET; i++ ) {
			// set X acording to offset
			SetX( fCenterX + fWidth * i - fWidth * MAX_NUM_FEET / 2.0f );
			
			// set color depending on m_iNumFeet
			if( i < m_iNumFeet )
				SetColor( color );
			else
				SetColor( color * 0.5f + D3DXCOLOR(0,0,0,1) );	// full alpha

			Sprite::Draw();
		}

		// set properties back to original
		SetXY( fCenterX, fCenterY );
		SetColor( color );
	};

	int m_iNumFeet;
};


#endif