#include "global.h"
#include "ScreenSelectMode.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageException.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "ActorUtil.h"
#include "CharacterManager.h"

/* Constants */

#define ELEM_SPACING	THEME->GetMetricI("ScreenSelectMode","ElementSpacing")
#define INCLUDE_DOUBLE_IN_JP	THEME->GetMetricI("ScreenSelectMode","IncludeDoubleInJointPremium")
#define SCROLLING_LIST_X	THEME->GetMetricF("ScreenSelectMode","ScrollingListX")
#define SCROLLING_LIST_Y	THEME->GetMetricF("ScreenSelectMode","ScrollingListY")
#define GUIDE_X		THEME->GetMetricF("ScreenSelectMode", "GuideX")
#define GUIDE_Y		THEME->GetMetricF("ScreenSelectMode", "GuideY")
#define USECONFIRM THEME->GetMetricI("ScreenSelectMode","UseConfirm")
#define USE_MODE_SPECIFIC_BGS THEME->GetMetricI("ScreenSelectMode", "UseModeSpecificBGAnims")
#define ENABLE_CHAR_SELECT THEME->GetMetricB("ScreenSelectMode","EnableCharSelect")
#define ONLY_2D_CHARS THEME->GetMetricB("ScreenSelectMode","Only2DChars")

/************************************
ScreenSelectMode (Constructor)
Desc: Sets up the screen display
************************************/

REGISTER_SCREEN_CLASS( ScreenSelectMode );
ScreenSelectMode::ScreenSelectMode( CString sClassName ) : ScreenSelect( sClassName )
{
}

void ScreenSelectMode::Init()
{
	ScreenSelect::Init();

	m_b2DAvailable = m_bCharsAvailable = false;

	int pn;
	for(pn=0;pn<NUM_PLAYERS;pn++)
	{
		m_iCurrentChar[pn]= -1; // minus 1 indicates no character.
		m_CurChar[pn].SetName(ssprintf("CharacterIconP%d",pn+1));
		SET_XY( m_CurChar[pn] );
	}
	m_bSelected = false;
	m_ChoiceListFrame.Load( THEME->GetPathG("ScreenSelectMode","list frame"));
	m_ChoiceListFrame.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y);
	m_ChoiceListFrame.SetName("ChoiceListFrame");
	this->AddChild( &m_ChoiceListFrame );

	m_soundModeChange.Load( THEME->GetPathS("ScreenSelectMode","modechange"));
	m_soundConfirm.Load( THEME->GetPathS("ScreenSelectMode","modeconfirm"));
	m_soundStart.Load( THEME->GetPathS("ScreenSelectMode","menustart"));
	
	for( unsigned i=0; i<m_aGameCommands.size(); i++ )
	{
		const GameCommand& mc = m_aGameCommands[i];

		//
		// Load Sprite
		//
		CString sElementPath = THEME->GetPathG("ScreenSelectMode",mc.m_sName);

		arrayLocations.push_back( sElementPath );
		
/*		if(USE_MODE_SPECIFIC_BGS == 1)
		{	
			BGAnimation templayer;
			templayer.LoadFromAniDir( THEME->GetPathB("ScreenSelectMode", ssprintf("background %s", mc.name )) );
		//	templayer.SetDiffuse(RageColor(0,0,0,0));
			m_Backgrounds.push_back(&templayer);
			this->AddChild( &m_Backgrounds[i] );
		}*/
	}

	// check for character availability
	vector<Character*> vpCharacters;
	if( ENABLE_CHAR_SELECT )
	{
		CHARMAN->GetCharacters( vpCharacters );

		for(unsigned i=0; i<vpCharacters.size(); i++)
		{
			if(vpCharacters[i] != NULL) // check its not null
			{
				m_bCharsAvailable = true;
				if(vpCharacters[i]->Has2DElems())
				{
					CString mpath = vpCharacters[i]->GetSongSelectIconPath();
					LOG->Trace("Char: %d, %s, 2D: true",i,mpath.c_str());						
					m_b2DAvailable = true;
				}
				else
				{
					CString mpath = vpCharacters[i]->GetSongSelectIconPath();
					LOG->Trace("Char: %d, %s, 2D: false",i,mpath.c_str());						
				}
			}
		}
		for(pn=0;pn<NUM_PLAYERS;pn++)
			m_CurChar[pn].Load( THEME->GetPathG("ScreenSelectMode","nochar") );	
	}	

	// m_ScrollingList.UseSpriteType(BANNERTYPE);
	m_ScrollingList.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y );
	m_ScrollingList.SetSpacing( ELEM_SPACING );
	m_ScrollingList.SetName("ScrollingList");
	this->AddChild( &m_ScrollingList );

	m_ChoiceListHighlight.Load( THEME->GetPathG("ScreenSelectMode","list highlight"));
	m_ChoiceListHighlight.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y );
	m_ChoiceListHighlight.SetName("ChoiceListHighlight");
	this->AddChild(&m_ChoiceListHighlight);

	m_Guide.Load( THEME->GetPathG("select mode","guide"));
	m_Guide.SetXY( GUIDE_X, GUIDE_Y );
	m_Guide.SetName("Guide");
	this->AddChild( &m_Guide );

	UpdateSelectableChoices();
}

/************************************
~ScreenSelectMode (Destructor)
Desc: Writes line to log when screen
is terminated.
************************************/
ScreenSelectMode::~ScreenSelectMode()
{
	LOG->Trace( "ScreenSelectMode::~ScreenSelectMode()" );
}

void ScreenSelectMode::MenuLeft( PlayerNumber pn )
{
	if(m_bSelected && USECONFIRM == 1)
	{
		m_bSelected = false;
		m_ScrollingList.StopBouncing();
	}
	m_ScrollingList.Left();
	m_soundModeChange.Play();
	if(USE_MODE_SPECIFIC_BGS == 1)
	{
		ChangeBGA();
	}
}

void ScreenSelectMode::ChangeBGA()
{
/*	for(int i=0; i<m_Backgrounds.size(); i++)
	{
		BGAnimation* templayer;
		templayer = m_Backgrounds[i];
		if(i == m_ScrollingList.GetSelection() )
		{
			templayer->SetDiffuse( RageColor(0,0,0,0));
		}
		else
		{
			templayer->SetDiffuse( RageColor(1,1,1,1));
		}
		m_Backgrounds[i] = templayer;
	}*/
}

void ScreenSelectMode::MenuRight( PlayerNumber pn )
{
	if(m_bSelected && USECONFIRM == 1)
	{
		m_bSelected = false;
		m_ScrollingList.StopBouncing();
	}
	m_ScrollingList.Right();
	m_soundModeChange.Play();
	if(USE_MODE_SPECIFIC_BGS == 1)
	{
		ChangeBGA();
	}
}

void ScreenSelectMode::UpdateSelectableChoices()
{
	CStringArray GraphicPaths;
	m_iNumChoices = 0;
	unsigned i=0;
	unsigned j=0;
	for( i=0; i<m_aGameCommands.size(); i++ )
	{
		const GameCommand& mc = m_aGameCommands[i];
		CString modename = mc.m_sName;
		modename.MakeUpper();


		// FIXME for new premium prefs
		const int SidesJoinedToPlay = 
			(mc.m_pStyle == NULL) ?
			1 :
			1;
			if( GAMESTATE->GetPremium()!=PREMIUM_JOINT ||
			(
				(GAMESTATE->GetPremium()==PREMIUM_JOINT) && 
				( 
					(INCLUDE_DOUBLE_IN_JP == 1 && (GAMESTATE->GetNumSidesJoined() == SidesJoinedToPlay)) || 
					(
						INCLUDE_DOUBLE_IN_JP == 0 && 
						(
							GAMESTATE->GetNumSidesJoined() == SidesJoinedToPlay || 
							(modename.substr(0, 6) == "DOUBLE" || modename.substr(0, 13) == "ARCADE-DOUBLE" ||
							modename.substr(0, 10) == "HALFDOUBLE" || modename.substr(0, 17) == "ARCADE-HALFDOUBLE") &&
							GAMESTATE->GetNumSidesJoined() != 2
						)
					)
				) 
			)			
			
		)
		{
			m_iNumChoices++;
			if(j<=MAX_ELEMS)
			{
				m_iSelectableChoices[j] = i;
				j++;
			}
			else
			{
				ASSERT(0); // too many choices, can't track them all. Quick Fix: If You Get This Just Increase MAX_ELEMS
			}
			GraphicPaths.push_back(arrayLocations[i]);
		}
	}
	m_ScrollingList.SetSelection(0);
	m_ScrollingList.Unload();
	m_ScrollingList.Load(GraphicPaths);
	if(USE_MODE_SPECIFIC_BGS == 1)
	{
		// TODO: Finish implementing this! (Got exams no time to finish...)
	}
}

void ScreenSelectMode::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		{
			m_bSelected = true;
			SetCharacters();
		}
		break;
	}
	ScreenSelect::HandleScreenMessage(SM);
}

void ScreenSelectMode::SetCharacters()
{
	vector<Character*> vpCharactersToUse;
	if(ENABLE_CHAR_SELECT && m_bCharsAvailable)
	{
		if(ONLY_2D_CHARS && m_b2DAvailable)
		{
			vector<Character*> vpCharacters;
			CHARMAN->GetCharacters( vpCharacters );
			for(unsigned i=0; i<vpCharacters.size(); i++)
			{
				if(vpCharacters[i] != NULL) // check its not null
				{
					if(vpCharacters[i]->Has2DElems())
					{
						vpCharactersToUse.push_back(vpCharacters[i]);
					}
				}
			}
		}
		else if(m_bCharsAvailable)
		{
			CHARMAN->GetCharacters( vpCharactersToUse );
		}
	}
	FOREACH_EnabledPlayer( pn )
	{
		if(ENABLE_CHAR_SELECT && m_iCurrentChar[pn] != -1)
		{
			Character* pChar = vpCharactersToUse[m_iCurrentChar[pn]];
			GAMESTATE->m_pCurCharacters[pn] = pChar;
		}
	}
}

void ScreenSelectMode::MenuStart( PlayerNumber pn )
{
	if(!m_bSelected && USECONFIRM == 1)
	{
		m_soundConfirm.Play();
		m_ScrollingList.StartBouncing();
		m_bSelected = true;
		return;
	}
	m_soundStart.Play();
	SetCharacters();
	OFF_COMMAND( m_ScrollingList );
	OFF_COMMAND( m_Guide );
	OFF_COMMAND( m_ChoiceListHighlight );
	OFF_COMMAND( m_ChoiceListFrame );
	for(int i=0; i<NUM_PLAYERS; i++)
		OFF_COMMAND( m_CurChar[i] );

	SCREENMAN->PostMessageToTopScreen( SM_AllDoneChoosing, 0.5f );
}

int ScreenSelectMode::GetSelectionIndex( PlayerNumber pn )
{
	return m_iSelectableChoices[m_ScrollingList.GetSelection()];
}

void ScreenSelectMode::Update( float fDelta )
{
/*	if(m_Backgrounds.empty() && USE_MODE_SPECIFIC_BGS == 1)
	{
		ASSERT(0);
	}
	else if(USE_MODE_SPECIFIC_BGS == 1)
	{
//		for(int i=0; i<m_Backgrounds.size(); i++)
//		{
//			m_Backgrounds[i]->Draw();
//		}
	}*/

	for(int pn=0; pn<NUM_PLAYERS; pn++)
	{
		if(ENABLE_CHAR_SELECT && m_iCurrentChar[pn] != -1)
		{
			m_CurChar[pn].Update( fDelta );
		}
	}

	ScreenSelect::Update( fDelta );
}

void ScreenSelectMode::DrawPrimitives()
{
	ScreenSelect::DrawPrimitives();
	FOREACH_EnabledPlayer( pn )
	{
		if(ENABLE_CHAR_SELECT)
		{
			m_CurChar[pn].Draw();
		}
	}
}

// todo: optimize the following - Frieza

void ScreenSelectMode::MenuUp(PlayerNumber pn)
{
	vector<Character*> vpCharactersToUse;
	if(ENABLE_CHAR_SELECT && m_bCharsAvailable)
	{
		if(ONLY_2D_CHARS && m_b2DAvailable)
		{
			vector<Character*> vpCharacters;
			CHARMAN->GetCharacters( vpCharacters );
			for(unsigned i=0; i<vpCharacters.size(); i++)
			{
				if(vpCharacters[i] != NULL) // check its not null
				{
					if(vpCharacters[i]->Has2DElems())
					{
						vpCharactersToUse.push_back(vpCharacters[i]);
					}
				}
			}
		}
		else if(m_bCharsAvailable)
		{
			CHARMAN->GetCharacters( vpCharactersToUse );
		}
	}

	m_CurChar[pn].UnloadTexture();
	if(ENABLE_CHAR_SELECT && m_bCharsAvailable)
	{
		if(m_iCurrentChar[pn] <= -1)
			m_iCurrentChar[pn] = vpCharactersToUse.size() -1;
		else
			m_iCurrentChar[pn]--; // set to no character
	
		if(m_iCurrentChar[pn] != -1)
		{
			if(vpCharactersToUse[m_iCurrentChar[pn]]->GetSongSelectIconPath() != "")
				m_CurChar[pn].Load( vpCharactersToUse[m_iCurrentChar[pn]]->GetSongSelectIconPath() );
			else
				m_CurChar[pn].Load( THEME->GetPathG("ScreenSelectMode","chariconmissing") );
		}
		else
			m_CurChar[pn].Load( THEME->GetPathG("ScreenSelectMode","nochar") );	
	}	
}

void ScreenSelectMode::MenuDown(PlayerNumber pn)
{
	vector<Character*> vpCharactersToUse;
	if(ENABLE_CHAR_SELECT && m_bCharsAvailable)
	{
		if(ONLY_2D_CHARS && m_b2DAvailable)
		{
			vector<Character*> vpCharacters;
			CHARMAN->GetCharacters( vpCharacters );
			for(unsigned i=0; i<vpCharacters.size(); i++)
			{
				if(vpCharacters[i] != NULL) // check its not null
				{
					if(vpCharacters[i]->Has2DElems())
					{
						vpCharactersToUse.push_back(vpCharacters[i]);
					}
				}
			}
		}
		else if(m_bCharsAvailable)
		{
			CHARMAN->GetCharacters( vpCharactersToUse );
		}
	}

	m_CurChar[pn].UnloadTexture();
	if(ENABLE_CHAR_SELECT && m_bCharsAvailable)
	{
		if(m_iCurrentChar[pn] < (int)vpCharactersToUse.size() - 1 || m_iCurrentChar[pn] == -1)
			m_iCurrentChar[pn]++;
		else
			m_iCurrentChar[pn] = -1; // set to no character
		if(m_iCurrentChar[pn] != -1)
		{
			if(vpCharactersToUse[m_iCurrentChar[pn]]->GetSongSelectIconPath() != "")
				m_CurChar[pn].Load( vpCharactersToUse[m_iCurrentChar[pn]]->GetSongSelectIconPath() );
			else
				m_CurChar[pn].Load( THEME->GetPathG("ScreenSelectMode","chariconmissing") );
		}
		else
			m_CurChar[pn].Load( THEME->GetPathG("ScreenSelectMode","nochar") );
	}
}

/*
 * (c) 2001-2003 "Frieza", Chris Danford
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
