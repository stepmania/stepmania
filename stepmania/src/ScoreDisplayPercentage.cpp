#include "global.h"

#include "ScoreDisplayPercentage.h"
#include "ThemeManager.h"

ScoreDisplayPercentage::ScoreDisplayPercentage()
{
	m_sprFrame.Load( THEME->GetPathToG("ScoreDisplayNormal frame") );
	this->AddChild( &m_sprFrame );

	m_Percent.SetName( "ScoreDisplayPercentage Percent" );
	this->AddChild( &m_Percent );
}

void ScoreDisplayPercentage::Init( PlayerNumber pn ) 
{
	m_Percent.Load( pn );
}
