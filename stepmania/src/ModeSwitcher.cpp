#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ModeSwitcher

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Andrew Livy
-----------------------------------------------------------------------------
*/

#include "ModeSwitcher.h"
#include "RageUtil.h"
#include "GameState.h"
#include "PrefsManager.h"
#include "SongManager.h"
#include "ThemeManager.h"
#include "RageSoundManager.h"
#include "StyleDef.h"
#include "song.h"
#include "ActorUtil.h"

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
	m_Nextmode.LoadFromFont( THEME->GetPathToF("ModeSwitcher stylenames") );
	m_Nextmode.SetXY(NEXTMODE_X,NEXTMODE_Y);
	m_Nextmode.SetZoom(NEXTMODE_ZOOM);
	m_Nextmode.SetText(GetNextStyleName());
	m_Prevmode.LoadFromFont( THEME->GetPathToF("ModeSwitcher stylenames") );
	m_Prevmode.SetXY(PREVMODE_X,PREVMODE_Y);
	m_Prevmode.SetText(GetPrevStyleName());
	m_Prevmode.SetZoom(PREVMODE_ZOOM);
	m_Stylename.LoadFromFont( THEME->GetPathToF("ModeSwitcher stylenames") );
	m_Stylename.SetXY(CURRMODE_X,CURRMODE_Y);
	m_Stylename.SetText(GetStyleName());
	m_Stylename.SetZoom(CURRMODE_ZOOM);


	m_NextIcon.Load( THEME->GetPathToG("ModeSwitcher nexticon"));
	m_NextIcon.SetXY( NEXTICON_X, NEXTICON_Y);
	m_PrevIcon.Load( THEME->GetPathToG("ModeSwitcher previcon"));
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

CString ModeSwitcher::GetStyleName()
{
	CString sStyleName;
	CString sDiff[2];


	switch(GAMESTATE->m_CurStyle)
	{
		case STYLE_PUMP_SINGLE: sStyleName = "SINGLE\n"; break;
		case STYLE_PUMP_DOUBLE: sStyleName = "FULL-DOUBLE\n"; break;
		case STYLE_PUMP_HALFDOUBLE: sStyleName = "HALFDOUBLE\n"; break;
		default: sStyleName = ""; break;
	}

	for(int i=0; i<2; i++)
	{
		if(GAMESTATE->IsPlayerEnabled(i))
		{
			switch(GAMESTATE->m_PreferredDifficulty[i])
			{
				case DIFFICULTY_BEGINNER: sDiff[i] = "Beginner\n"; break;
				case DIFFICULTY_EASY:
				{
					if(GAMESTATE->m_CurGame == GAME_PUMP)
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
					if(GAMESTATE->m_CurGame == GAME_PUMP)
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
					if(GAMESTATE->m_CurGame == GAME_PUMP)
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
	CString returnval;
	returnval = sStyleName + sDiff[0] + sDiff[1];
	return returnval;
}

CString ModeSwitcher::GetNextStyleName()
{
	CString sStyleName[2];
	CString sDiff[2];
	
	for(int i=0; i<2; i++)
	{
		if(GAMESTATE->IsPlayerEnabled(i))
		{
			if(GAMESTATE->m_PreferredDifficulty[i] != DIFFICULTY_CHALLENGE)
			{
				switch(GAMESTATE->m_CurStyle)
				{
					case STYLE_PUMP_SINGLE: sStyleName[i] = "SINGLE\n"; break;
					case STYLE_PUMP_DOUBLE: sStyleName[i] = "FULL-DOUBLE\n"; break;
					case STYLE_PUMP_HALFDOUBLE: sStyleName[i] = "HALFDOUBLE\n"; break;
					default: sStyleName[i] = ""; break;
				}

				switch(GAMESTATE->m_PreferredDifficulty[i])
				{
					case DIFFICULTY_BEGINNER:
					{
						if(GAMESTATE->m_CurGame == GAME_PUMP)
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
						if(GAMESTATE->m_CurGame == GAME_PUMP)
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
						if(GAMESTATE->m_CurGame == GAME_PUMP)
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
				switch(GAMESTATE->m_CurStyle)
				{
					case STYLE_PUMP_SINGLE: sStyleName[i] = "HALFDOUBLE\n"; break;
					case STYLE_PUMP_DOUBLE: sStyleName[i] = "SINGLE\n"; break;
					case STYLE_PUMP_HALFDOUBLE: sStyleName[i] = "FULL-DOUBLE\n"; break;
					default: sStyleName[i] = ""; break;
				}
				sDiff[i] = "Beginner\n";
			}
		}
		else
		{
			sStyleName[i] = "";
			sDiff[i] = "";
		}
	}
	CString returnval;
	returnval = sStyleName[0] + sDiff[0] + sStyleName[1] + sDiff[1];
	return returnval;
}

CString ModeSwitcher::GetPrevStyleName()
{
	CString sStyleName[2];
	CString sDiff[2];
	
	for(int i=0; i<2; i++)
	{
		if(GAMESTATE->IsPlayerEnabled(i))
		{
			if(GAMESTATE->m_PreferredDifficulty[i] != DIFFICULTY_BEGINNER)
			{
				switch(GAMESTATE->m_CurStyle)
				{
					case STYLE_PUMP_SINGLE: sStyleName[i] = "SINGLE\n"; break;
					case STYLE_PUMP_DOUBLE: sStyleName[i] = "FULL-DOUBLE\n"; break;
					case STYLE_PUMP_HALFDOUBLE: sStyleName[i] = "HALFDOUBLE\n"; break;
					default: sStyleName[i] = ""; break;
				}

				switch(GAMESTATE->m_PreferredDifficulty[i])
				{
					case DIFFICULTY_CHALLENGE:
					{
						if(GAMESTATE->m_CurGame == GAME_PUMP)
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
						if(GAMESTATE->m_CurGame == GAME_PUMP)
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
						if(GAMESTATE->m_CurGame == GAME_PUMP)
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
				switch(GAMESTATE->m_CurStyle)
				{
					case STYLE_PUMP_SINGLE: sStyleName[i] = "FULL-DOUBLE\n"; break;
					case STYLE_PUMP_DOUBLE: sStyleName[i] = "HALFDOUBLE\n"; break;
					case STYLE_PUMP_HALFDOUBLE: sStyleName[i] = "SINGLE\n"; break;
					default: sStyleName[i] = ""; break;
				}
				sDiff[i] = "Challenge\n";
			}
		}
		else
		{
			sStyleName[i] = "";
			sDiff[i] = "";
		}
	}
	CString returnval;
	returnval = sStyleName[0] + sDiff[0] + sStyleName[1] + sDiff[1];
	return returnval;
}

void ModeSwitcher::NextMode(int pn)
{
	if(GAMESTATE->m_CurGame == GAME_PUMP)
	{
		if(GAMESTATE->IsPlayerEnabled(pn))
		{
			if(GAMESTATE->m_PreferredDifficulty[pn] != 	DIFFICULTY_CHALLENGE)
			{
				switch(GAMESTATE->m_PreferredDifficulty[pn])
				{
					case DIFFICULTY_BEGINNER: GAMESTATE->m_PreferredDifficulty[pn] = DIFFICULTY_EASY; break;
					case DIFFICULTY_EASY: GAMESTATE->m_PreferredDifficulty[pn] = DIFFICULTY_MEDIUM; break;
					case DIFFICULTY_MEDIUM: GAMESTATE->m_PreferredDifficulty[pn] = DIFFICULTY_HARD; break;
					case DIFFICULTY_HARD: GAMESTATE->m_PreferredDifficulty[pn] = DIFFICULTY_CHALLENGE; break;
				}
				m_Stylename.SetText(GetStyleName());
				m_Nextmode.SetText(GetNextStyleName());
				m_Prevmode.SetText(GetPrevStyleName());
				return;
			}
			else
			{
				if(GAMESTATE->IsPlayerEnabled(PLAYER_1))
					GAMESTATE->m_PreferredDifficulty[PLAYER_1] = DIFFICULTY_BEGINNER;
				if(GAMESTATE->IsPlayerEnabled(PLAYER_2))
					GAMESTATE->m_PreferredDifficulty[PLAYER_2] = DIFFICULTY_BEGINNER;
			}
		}

		int iStyle = GAMESTATE->m_CurStyle;
		switch(iStyle)
		{
			case STYLE_PUMP_SINGLE:
			{
				GAMESTATE->m_CurStyle = STYLE_PUMP_HALFDOUBLE;
			}
			break;
			case STYLE_PUMP_HALFDOUBLE:
			{
				GAMESTATE->m_CurStyle = STYLE_PUMP_DOUBLE;
			}
			break;
			case STYLE_PUMP_DOUBLE:
			{
				GAMESTATE->m_CurStyle = STYLE_PUMP_SINGLE;
			}
			break;
		}
	}
	m_Stylename.SetText(GetStyleName());
	m_Nextmode.SetText(GetNextStyleName());
	m_Prevmode.SetText(GetPrevStyleName());
}

void ModeSwitcher::PrevMode(int pn)
{
	if(GAMESTATE->IsPlayerEnabled(pn))
	{
		if(GAMESTATE->m_PreferredDifficulty[pn] != 	DIFFICULTY_BEGINNER)
		{
			switch(GAMESTATE->m_PreferredDifficulty[pn])
			{
				case DIFFICULTY_CHALLENGE: GAMESTATE->m_PreferredDifficulty[pn] = DIFFICULTY_HARD; break;
				case DIFFICULTY_EASY: GAMESTATE->m_PreferredDifficulty[pn] = DIFFICULTY_BEGINNER; break;
				case DIFFICULTY_MEDIUM: GAMESTATE->m_PreferredDifficulty[pn] = DIFFICULTY_EASY; break;
				case DIFFICULTY_HARD: GAMESTATE->m_PreferredDifficulty[pn] = DIFFICULTY_MEDIUM; break;
			}
			m_Stylename.SetText(GetStyleName());
			m_Nextmode.SetText(GetNextStyleName());
			m_Prevmode.SetText(GetPrevStyleName());
			return;
		}
		else
		{
			if(GAMESTATE->IsPlayerEnabled(PLAYER_1))
				GAMESTATE->m_PreferredDifficulty[PLAYER_1] = DIFFICULTY_CHALLENGE;
			if(GAMESTATE->IsPlayerEnabled(PLAYER_2))
				GAMESTATE->m_PreferredDifficulty[PLAYER_2] = DIFFICULTY_CHALLENGE;
		}
	}

	int iStyle = GAMESTATE->m_CurStyle;
	switch(iStyle)
	{
		case STYLE_PUMP_SINGLE:
		{
			GAMESTATE->m_CurStyle = STYLE_PUMP_DOUBLE;
		}
		break;
		case STYLE_PUMP_HALFDOUBLE:
		{
			GAMESTATE->m_CurStyle = STYLE_PUMP_SINGLE;
		}
		break;
		case STYLE_PUMP_DOUBLE:
		{
			GAMESTATE->m_CurStyle = STYLE_PUMP_HALFDOUBLE;
		}
		break;
	}
	m_Stylename.SetText(GetStyleName());
	m_Nextmode.SetText(GetNextStyleName());
	m_Prevmode.SetText(GetPrevStyleName());
}