#include "global.h"
#include "ModeSwitcher.h"
#include "RageUtil.h"
#include "GameState.h"
#include "SongManager.h"
#include "ThemeManager.h"
#include "Style.h"
#include "song.h"
#include "ActorUtil.h"
#include "GameManager.h"
#include "Game.h"

#define PREVMODE_X		THEME->GetMetricF("ModeSwitcher","PrevModeX")
#define PREVMODE_Y		THEME->GetMetricF("ModeSwitcher","PrevModeY")
#define PREVMODE_ZOOM		THEME->GetMetricF("ModeSwitcher","PrevModeZoom")
#define NEXTMODE_X		THEME->GetMetricF("ModeSwitcher","NextModeX")
#define NEXTMODE_Y		THEME->GetMetricF("ModeSwitcher","NextModeY")
#define NEXTMODE_ZOOM		THEME->GetMetricF("ModeSwitcher","NextModeZoom")
#define NEXTICON_X		THEME->GetMetricF("ModeSwitcher","NextIconX")
#define NEXTICON_Y		THEME->GetMetricF("ModeSwitcher","NextIconY")
#define PREVICON_X		THEME->GetMetricF("ModeSwitcher","PrevIconX")
#define PREVICON_Y		THEME->GetMetricF("ModeSwitcher","PrevIconY")
#define CURRMODE_X		THEME->GetMetricF("ModeSwitcher","CurrentModeX")
#define CURRMODE_Y		THEME->GetMetricF("ModeSwitcher","CurrentModeY")
#define CURRMODE_ZOOM		THEME->GetMetricF("ModeSwitcher","CurrentModeZoom")

ModeSwitcher::ModeSwitcher() 
{ 
	m_Nextmode.LoadFromFont( THEME->GetPathF("ModeSwitcher","stylenames") );
	m_Nextmode.SetXY(NEXTMODE_X,NEXTMODE_Y);
	m_Nextmode.SetZoom(NEXTMODE_ZOOM);
	m_Nextmode.SetText(GetNextStyleName());
	m_Prevmode.LoadFromFont( THEME->GetPathF("ModeSwitcher","stylenames") );
	m_Prevmode.SetXY(PREVMODE_X,PREVMODE_Y);
	m_Prevmode.SetText(GetPrevStyleName());
	m_Prevmode.SetZoom(PREVMODE_ZOOM);
	m_Stylename.LoadFromFont( THEME->GetPathF("ModeSwitcher","stylenames") );
	m_Stylename.SetXY(CURRMODE_X,CURRMODE_Y);
	m_Stylename.SetText(GetStyleName());
	m_Stylename.SetZoom(CURRMODE_ZOOM);


	m_NextIcon.Load( THEME->GetPathG("ModeSwitcher","nexticon"));
	m_NextIcon.SetXY( NEXTICON_X, NEXTICON_Y);
	m_PrevIcon.Load( THEME->GetPathG("ModeSwitcher","previcon"));
	m_PrevIcon.SetXY( PREVICON_X, PREVICON_Y);

	this->AddChild( &m_NextIcon );
	this->AddChild( &m_PrevIcon );

	this->AddChild(&m_Stylename);
	this->AddChild(&m_Nextmode);
	this->AddChild(&m_Prevmode);
}

ModeSwitcher::~ModeSwitcher()
{
	
}

RString ModeSwitcher::GetStyleName()
{
	RString sStyleName;
	RString sDiff[NUM_PLAYERS];

	sStyleName = GAMESTATE->GetCurrentStyle()->m_szName;
	sStyleName.MakeUpper();

	FOREACH_PlayerNumber(i)
	{
		if( GAMESTATE->IsPlayerEnabled(i) )
		{
			switch( GAMESTATE->m_PreferredDifficulty[i].Get() )
			{
				case DIFFICULTY_BEGINNER: sDiff[i] = "Beginner\n"; break;
				case DIFFICULTY_EASY:
				{
					if(GAMESTATE->m_pCurGame->m_szName == RString("pump"))
					{
						sDiff[i] = "Normal\n"; break;
					}
					else
					{
						sDiff[i] = "Light\n"; break;
					}
				} 
				case DIFFICULTY_MEDIUM:
				{
					if(GAMESTATE->m_pCurGame->m_szName == RString("pump"))
					{
						sDiff[i] = "Hard\n"; break;
					}
					else
					{
						sDiff[i] = "Standard\n"; break;
					}
				} 				
				case DIFFICULTY_HARD:
				{
					if(GAMESTATE->m_pCurGame->m_szName == RString("pump"))
					{
						sDiff[i] = "Crazy\n"; break;
					}
					else
					{
						sDiff[i] = "Heavy\n"; break;
					}
				} 				
				case DIFFICULTY_CHALLENGE: sDiff[i] = "Challenge\n"; break;
				default: sDiff[i] = ""; break;
			}
		}
		else
		{
			sDiff[i] = "";
		}
	}
	RString returnval;
	returnval = sStyleName + sDiff[0] + sDiff[1];
	return returnval;
}

RString ModeSwitcher::GetNextStyleName()
{
	RString sStyleName[NUM_PLAYERS];
	RString sDiff[NUM_PLAYERS];
	
	FOREACH_PlayerNumber(i)
	{
		if(GAMESTATE->IsPlayerEnabled(i))
		{
			if(GAMESTATE->m_PreferredDifficulty[i] != DIFFICULTY_CHALLENGE)
			{
				sStyleName[i] = GAMESTATE->GetCurrentStyle()->m_szName;
				sStyleName[i].MakeUpper();

				switch(GAMESTATE->m_PreferredDifficulty[i])
				{
					case DIFFICULTY_BEGINNER:
					{
						if(GAMESTATE->m_pCurGame->m_szName == RString("pump"))
						{
							sDiff[i] = "Normal\n"; break;
						}
						else
						{
							sDiff[i] = "Light\n"; break;
						}
					} 				
					case DIFFICULTY_EASY:
					{
						if(GAMESTATE->m_pCurGame->m_szName == RString("pump"))
						{
							sDiff[i] = "Hard\n"; break;
						}
						else
						{
							sDiff[i] = "Standard\n"; break;
						}
					} 	
					case DIFFICULTY_MEDIUM:
					{
						if(GAMESTATE->m_pCurGame->m_szName == RString("pump"))
						{
							sDiff[i] = "Crazy\n"; break;
						}
						else
						{
							sDiff[i] = "Heavy\n"; break;
						}
					} 					
					case DIFFICULTY_HARD: sDiff[i] = "Challenge\n"; break;			
					case DIFFICULTY_CHALLENGE: sDiff[i] = "Beginner\n"; break;
					default: sDiff[i] = ""; break;
				}
			}
			else
			{
				sStyleName[i] = GAMESTATE->GetCurrentStyle()->m_szName;
				sStyleName[i].MakeUpper();

				sDiff[i] = "Beginner\n";
			}
		}
		else
		{
			sStyleName[i] = "";
			sDiff[i] = "";
		}
	}
	RString returnval;
	returnval = sStyleName[0] + sDiff[0] + sStyleName[1] + sDiff[1];
	return returnval;
}

RString ModeSwitcher::GetPrevStyleName()
{
	RString sStyleName[NUM_PLAYERS];
	RString sDiff[NUM_PLAYERS];
	
	FOREACH_PlayerNumber(i)
	{
		if(GAMESTATE->IsPlayerEnabled(i))
		{
			if(GAMESTATE->m_PreferredDifficulty[i] != DIFFICULTY_BEGINNER)
			{
				sStyleName[i] = GAMESTATE->GetCurrentStyle()->m_szName;
				sStyleName[i].MakeUpper();

				switch(GAMESTATE->m_PreferredDifficulty[i])
				{
					case DIFFICULTY_CHALLENGE:
					{
						if(GAMESTATE->m_pCurGame->m_szName == RString("pump"))
						{
							sDiff[i] = "Crazy\n"; break;
						}
						else
						{
							sDiff[i] = "Heavy\n"; break;
						}
					} 				
					case DIFFICULTY_EASY: sDiff[i] = "Beginner\n"; break;
					case DIFFICULTY_MEDIUM:
					{
						if(GAMESTATE->m_pCurGame->m_szName == RString("pump"))
						{
							sDiff[i] = "Normal\n"; break;
						}
						else
						{
							sDiff[i] = "Light\n"; break;
						}
					} 					
					case DIFFICULTY_HARD:
					{
						if(GAMESTATE->m_pCurGame->m_szName == RString("pump"))
						{
							sDiff[i] = "Hard\n"; break;
						}
						else
						{
							sDiff[i] = "Standard\n"; break;
						}					
					}		
					case DIFFICULTY_BEGINNER: sDiff[i] = "Challenge\n"; break;
					default: sDiff[i] = ""; break;
				}
			}
			else
			{
				sStyleName[i] = GAMESTATE->GetCurrentStyle()->m_szName;
				sStyleName[i].MakeUpper();

				sDiff[i] = "Challenge\n";
			}
		}
		else
		{
			sStyleName[i] = "";
			sDiff[i] = "";
		}
	}
	RString returnval;
	returnval = sStyleName[0] + sDiff[0] + sStyleName[1] + sDiff[1];
	return returnval;
}

void ModeSwitcher::ChangeMode(PlayerNumber pn, int dir)
{
	if(GAMESTATE->m_pCurGame->m_szName == RString("pump"))
	{
		if(GAMESTATE->IsPlayerEnabled(pn))
		{
			if(GAMESTATE->m_PreferredDifficulty[pn] != 	DIFFICULTY_CHALLENGE)
			{
				switch(GAMESTATE->m_PreferredDifficulty[pn])
				{
					case DIFFICULTY_BEGINNER: GAMESTATE->m_PreferredDifficulty[pn].Set( DIFFICULTY_EASY ); break;
					case DIFFICULTY_EASY: GAMESTATE->m_PreferredDifficulty[pn].Set( DIFFICULTY_MEDIUM ); break;
					case DIFFICULTY_MEDIUM: GAMESTATE->m_PreferredDifficulty[pn].Set( DIFFICULTY_HARD ); break;
					case DIFFICULTY_HARD: GAMESTATE->m_PreferredDifficulty[pn].Set( DIFFICULTY_CHALLENGE ); break;
				}
				m_Stylename.SetText(GetStyleName());
				m_Nextmode.SetText(GetNextStyleName());
				m_Prevmode.SetText(GetPrevStyleName());
				return;
			}
			else
			{
				if(GAMESTATE->IsPlayerEnabled(PLAYER_1))
					GAMESTATE->m_PreferredDifficulty[PLAYER_1].Set( DIFFICULTY_CHALLENGE );
				if(GAMESTATE->IsPlayerEnabled(PLAYER_2))
					GAMESTATE->m_PreferredDifficulty[PLAYER_2].Set( DIFFICULTY_CHALLENGE );
			}
		}

		// Make a list of all styles for the current Game.
		vector<const Style*> vPossibleStyles;
		GAMEMAN->GetStylesForGame( GAMESTATE->m_pCurGame, vPossibleStyles );
		ASSERT( !vPossibleStyles.empty() );

		int index = 0;
		vector<const Style*>::const_iterator iter = find(vPossibleStyles.begin(), vPossibleStyles.end(), GAMESTATE->GetCurrentStyle() );
		if( iter != vPossibleStyles.end() )
		{
			index = iter - vPossibleStyles.begin();
			index += dir;
			wrap( index, vPossibleStyles.size() );
		}

		GAMESTATE->m_pCurStyle.Set( vPossibleStyles[index] );
	}
	m_Stylename.SetText(GetStyleName());
	m_Nextmode.SetText(GetNextStyleName());
	m_Prevmode.SetText(GetPrevStyleName());
}


/*
 * (c) 2003 "Frieza"
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
