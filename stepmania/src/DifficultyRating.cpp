#include "global.h"
#include "DifficultyRating.h"
#include "ThemeManager.h"
#include "RageUtil.h"

const int ORIENTATION_LEFT = 0;
const int ORIENTATION_CENTER = 1;

#define ELEMSPACING		THEME->GetMetricF("ScreenEz2SelectMusic","DifficultyRatingSpacing")

DifficultyRating::DifficultyRating()
{
	iMaxElements=10;
	for(int i=0; i<iMaxElements; i++)
	{
		Sprite* pNewSprite = new Sprite;
		pNewSprite->Load( THEME->GetPathToG("Select Music DifficultyRatingIcon") );
		m_apSprites.push_back( pNewSprite );
		m_apSprites[i]->SetDiffuse( RageColor(1,1,1,0) );
//		this->AddChild(m_apSprites[i]);
	}

} 

DifficultyRating::~DifficultyRating()
{

}

void DifficultyRating::DrawPrimitives()
{
	for(int i=0; i<iMaxElements; i++)
	{
		m_apSprites[i]->Draw();
	}
}

void DifficultyRating::SetDifficulty(int Difficulty)
{
	iCurrentDifficulty = Difficulty;
	for(int i=0; i<iMaxElements; i++)
	{
		if(i < iCurrentDifficulty)
		{
			m_apSprites[i]->SetDiffuse( RageColor(1,1,1,1) );	
		}
		else
		{
			m_apSprites[i]->SetDiffuse( RageColor(1,1,1,0) );
		}
	}

}

void DifficultyRating::Update(float fDeltaTime)
{
	for(int i=0; i<iMaxElements; i++)
	{
		m_apSprites[i]->Update(fDeltaTime);
	}	
}

void DifficultyRating::SetOrientation(int Orientation)
{
	if(Orientation == ORIENTATION_LEFT)
	{
		for(int i=0; i<iMaxElements; i++)
		{
			m_apSprites[i]->SetX(0 + (ELEMSPACING * i) );
		}
	}
	else // central orientation
	{
		if(iCurrentDifficulty % 2 == 0) // even number?
		{
			//damn! in that case everything is to the side of the middle....
			int incrementor=1; // start on one... 0 is reserved for 'central' (read odd number)
			float offset=0.5f;
			for(int i=0; i<iMaxElements; i++)
			{
				if(incrementor == 1) // + offset
				{
					m_apSprites[i]->SetX(0  + offset);
					incrementor++;
				}
				else // - offset
				{
					m_apSprites[i]->SetX(0 - offset);
					offset += ELEMSPACING + 0.5f; // increase offset
					incrementor=1;
				}
			}			
		}
		else
		{
			int incrementor=0;
			float offset=0;
			for(int i=0; i<iMaxElements; i++)
			{
				if(incrementor == 0) // special case.... this means we're in the middle.
				{
					m_apSprites[i]->SetX(0); // set to the middle.
					offset=ELEMSPACING;
					incrementor++;
				}
				//the rest go either side....
				else
				{
					if(incrementor == 1) // + offset
					{
						m_apSprites[i]->SetX(0+offset);
						incrementor++;
					}
					else // - offset
					{
						m_apSprites[i]->SetX(0-offset);
						offset += ELEMSPACING; // increase offset
						incrementor=1;
					}
				}
			}
		}
	}
}

/*
 * (c) 2001-2002 "Frieza"
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
