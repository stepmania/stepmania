#include "global.h"
/****************************************
ScreenSelectMode.cpp
Desc: See Header
Copyright (C):
Andrew Livy
Chris Danford
*****************************************/

/* Includes */

#include "ScreenSelectMode.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "ActorUtil.h"

/* Constants */

#define ELEM_SPACING	THEME->GetMetricI("ScreenSelectMode","ElementSpacing")
#define INCLUDE_DOUBLE_IN_JP	THEME->GetMetricI("ScreenSelectMode","IncludeDoubleInJointPremium")
#define SCROLLING_LIST_X	THEME->GetMetricF("ScreenSelectMode","ScrollingListX")
#define SCROLLING_LIST_Y	THEME->GetMetricF("ScreenSelectMode","ScrollingListY")
#define GUIDE_X		THEME->GetMetricF("ScreenSelectMode", "GuideX")
#define GUIDE_Y		THEME->GetMetricF("ScreenSelectMode", "GuideY")
#define USECONFIRM THEME->GetMetricI("ScreenSelectMode","UseConfirm")
#define USE_MODE_SPECIFIC_BGS THEME->GetMetricI("ScreenSelectMode", "UseModeSpecificBGAnims")

/************************************
ScreenSelectMode (Constructor)
Desc: Sets up the screen display
************************************/

ScreenSelectMode::ScreenSelectMode() : ScreenSelect( "ScreenSelectMode" )
{
	m_bSelected = false;
	m_ChoiceListFrame.Load( THEME->GetPathToG("ScreenSelectMode list frame"));
	m_ChoiceListFrame.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y);
	m_ChoiceListFrame.SetName("ChoiceListFrame");
	this->AddChild( &m_ChoiceListFrame );

	m_soundModeChange.Load( THEME->GetPathToS("ScreenSelectMode modechange"));
	m_soundConfirm.Load( THEME->GetPathToS("ScreenSelectMode modeconfirm"));
	m_soundStart.Load( THEME->GetPathToS("ScreenSelectMode menustart"));
		unsigned i;
	for( i=0; i<m_aModeChoices.size(); i++ )
	{
		const ModeChoice& mc = m_aModeChoices[i];

		//
		// Load Sprite
		//
		CString sElementName = ssprintf("ScreenSelectMode %s", mc.name );
		CString sElementPath = THEME->GetPathToG(sElementName);

		arrayLocations.push_back( sElementPath );
		
/*		if(USE_MODE_SPECIFIC_BGS == 1)
		{	
			BGAnimation templayer;
			templayer.LoadFromAniDir( THEME->GetPathToB(ssprintf("ScreenSelectMode background %s", mc.name )) );
		//	templayer.SetDiffuse(RageColor(0,0,0,0));
			m_Backgrounds.push_back(&templayer);
			this->AddChild( &m_Backgrounds[i] );
		}*/
	}
	
	// m_ScrollingList.UseSpriteType(BANNERTYPE);
	m_ScrollingList.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y );
	m_ScrollingList.SetSpacing( ELEM_SPACING );
	m_ScrollingList.SetName("ScrollingList");
	this->AddChild( &m_ScrollingList );

	m_ChoiceListHighlight.Load( THEME->GetPathToG("ScreenSelectMode list highlight"));
	m_ChoiceListHighlight.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y );
	m_ChoiceListHighlight.SetName("ChoiceListHighlight");
	this->AddChild(&m_ChoiceListHighlight);

	m_Guide.Load( THEME->GetPathToG("select mode guide"));
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
	for( i=0; i<m_aModeChoices.size(); i++ )
	{
		const ModeChoice& mc = m_aModeChoices[i];
		CString modename = mc.name;
		modename.MakeUpper();


		// if its joint premium and inclusive of double consider double and versus as needing another coin
		// if its joint premium and non-inclusive of double consider double as appearing only when one player is available.
		// if its joint premium, everythings available for play
/*		if( (PREFSMAN->m_bJointPremium && INCLUDE_DOUBLE_IN_JP == 1 && (GAMESTATE->GetNumSidesJoined() == mc.numSidesJoinedToPlay) ) ||
			(PREFSMAN->m_bJointPremium && INCLUDE_DOUBLE_IN_JP == 0 && 
			modename.substr(0, 6) == "DOUBLE" || modename.substr(0, 13) == "ARCADE-DOUBLE" ||
			modename.substr(0, 10) == "HALFDOUBLE" || modename.substr(0, 17) == "ARCADE-HALFDOUBLE" ||
			(GAMESTATE->GetNumSidesJoined() != mc.numSidesJoinedToPlay)) ||
			(!PREFSMAN->m_bJointPremium)
		)*/

		if( (!PREFSMAN->m_bJointPremium ) ||
			(
				PREFSMAN->m_bJointPremium && 
				( 
					(INCLUDE_DOUBLE_IN_JP == 1 && (GAMESTATE->GetNumSidesJoined() == mc.numSidesJoinedToPlay)) || 
					(
						INCLUDE_DOUBLE_IN_JP == 0 && 
						(
							GAMESTATE->GetNumSidesJoined() == mc.numSidesJoinedToPlay || 
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
		}
		break;
	}
	ScreenSelect::HandleScreenMessage(SM);
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
	OFF_COMMAND( m_ScrollingList );
	OFF_COMMAND( m_Guide );
	OFF_COMMAND( m_ChoiceListHighlight );
	OFF_COMMAND( m_ChoiceListFrame );

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

	ScreenSelect::Update( fDelta );
}