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

/* Constants */

#define ELEM_SPACING	THEME->GetMetricI("ScreenSelectMode","ElementSpacing")
#define INCLUDE_DOUBLE_IN_JP	THEME->GetMetricI("ScreenSelectMode","IncludeDoubleInJointPremium")
#define SCROLLING_LIST_X	THEME->GetMetricI("ScreenSelectMode","ScrollingListX")
#define SCROLLING_LIST_Y	THEME->GetMetricI("ScreenSelectMode","ScrollingListY")
#define GUIDE_X		THEME->GetMetricF("ScreenSelectMode", "GuideX")
#define GUIDE_Y		THEME->GetMetricF("ScreenSelectMode", "GuideY")
#define USECONFIRM THEME->GetMetricI("ScreenSelectMode","UseConfirm")

/************************************
ScreenSelectMode (Constructor)
Desc: Sets up the screen display
************************************/

ScreenSelectMode::ScreenSelectMode() : ScreenSelect( "ScreenSelectMode" )
{
	m_bSelected = false;
	m_ChoiceListFrame.Load( THEME->GetPathToG("ScreenSelectMode list frame"));
	m_ChoiceListFrame.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y);
	this->AddChild( &m_ChoiceListFrame );

	m_soundModeChange.Load( THEME->GetPathToS("ScreenSelectMode modechange"));
	m_soundConfirm.Load( THEME->GetPathToS("ScreenSelectMode modeconfirm"));
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
	}
	
	// m_ScrollingList.UseSpriteType(BANNERTYPE);
	m_ScrollingList.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y );
	m_ScrollingList.SetSpacing( ELEM_SPACING );
	this->AddChild( &m_ScrollingList );

	m_ChoiceListHighlight.Load( THEME->GetPathToG("ScreenSelectMode list highlight"));
	m_ChoiceListHighlight.SetXY( SCROLLING_LIST_X, SCROLLING_LIST_Y );
	this->AddChild(&m_ChoiceListHighlight);

	m_Guide.Load( THEME->GetPathToG("select mode guide"));
	m_Guide.SetXY( GUIDE_X, GUIDE_Y );
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
	m_ScrollingList.StopTweening();
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
		if(
		(PREFSMAN->m_bJointPremium && INCLUDE_DOUBLE_IN_JP == 1 && ((GAMESTATE->GetNumSidesJoined() == 1 && mc.numSidesJoinedToPlay == 1) || (GAMESTATE->GetNumSidesJoined() == 2 && mc.numSidesJoinedToPlay == 2))) ||
		(PREFSMAN->m_bJointPremium && INCLUDE_DOUBLE_IN_JP == 0 && 
			(((modename.substr(0, 6) == "DOUBLE" || modename.substr(0, 13) == "ARCADE-DOUBLE")  && GAMESTATE->GetNumSidesJoined() != 2 ) || GAMESTATE->GetNumSidesJoined() == 1 && mc.numSidesJoinedToPlay == 1) || 
			((modename.substr(0, 6) != "DOUBLE" || modename.substr(0, 13) != "ARCADE-DOUBLE") && GAMESTATE->GetNumSidesJoined() == 2 && mc.numSidesJoinedToPlay == 2)) ||
		(!PREFSMAN->m_bJointPremium)
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
	SCREENMAN->PostMessageToTopScreen( SM_AllDoneChoosing, 0 );
	m_ScrollingList.BeginTweening(1.0f);
	m_ScrollingList.SetRotationZ(0.0f);
}

int ScreenSelectMode::GetSelectionIndex( PlayerNumber pn )
{
	return m_iSelectableChoices[m_ScrollingList.GetSelection()];
}

void ScreenSelectMode::Update( float fDelta )
{
	ScreenSelect::Update( fDelta );
}