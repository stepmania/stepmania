#include "global.h"
#include "DifficultyRating.h"
#include "ThemeManager.h"
#include "RageUtil.h"

/****************************
DifficultyRating
Desc: see header
Copyright: Andrew Livy
*****************************/

const int ORIENTATION_LEFT = 0;
const int ORIENTATION_CENTER = 1;

#define ELEMSPACING		THEME->GetMetricF("ScreenEz2SelectMusic","DifficultyRatingSpacing")

DifficultyRating::DifficultyRating()
{
	iMaxElements=10;
	for(int i=0; i<iMaxElements; i++)
	{
		Sprite* pNewSprite = new Sprite;
		pNewSprite->Load( THEME->GetPathTo("Graphics","Select Music DifficultyRatingIcon") );
		m_apSprites.push_back( pNewSprite );
		m_apSprites[i]->SetDiffuse( RageColor(1,1,1,0) );
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
