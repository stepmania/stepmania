#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: HighScore

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "HighScore.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"

#define EMPTY_NAME			THEME->GetMetric ("HighScore","EmptyName")


bool HighScore::operator>=( const HighScore& other ) const
{
	/* Make sure we treat AAAA as higher than AAA, even though the score
 	 * is the same.
	 *
	 * XXX: Isn't it possible to beat the grade but not beat the score, since
	 * grading and scores are on completely different systems?  Should we be
	 * checking for these completely separately? */
	if( PREFSMAN->m_bPercentageScoring )
	{
		if( fPercentDP == other.fPercentDP )
			return grade >= other.grade;
		else
			return fPercentDP >= other.fPercentDP;
	}
	else
	{
		if( iScore == other.iScore )
			return grade >= other.grade;
		else
			return iScore >= other.iScore;
	}
}

XNode* HighScore::CreateNode() const
{
	XNode* pNode = new XNode;
	pNode->name = "HighScore";

	// TRICKY:  Don't write "name to fill in" markers.
	pNode->AppendChild( "Name", IsRankingToFillIn(sName) ? "" : sName );
	pNode->AppendChild( "Grade",			grade );
	pNode->AppendChild( "Score",			iScore );
	pNode->AppendChild( "PercentDP",		fPercentDP );
	pNode->AppendChild( "SurviveSeconds",	fSurviveSeconds );
	pNode->AppendChild( "Modifiers",		sModifiers );
	pNode->AppendChild( "Time",				time );
	pNode->AppendChild( "MachineGuid",		sMachineGuid );

	return pNode;
}

void HighScore::LoadFromNode( const XNode* pNode ) 
{
	ASSERT( pNode->name == "HighScore" );
	for( XNodes::const_iterator child = pNode->childs.begin(); 
		child != pNode->childs.end(); 
		child++ )
	{
		if( (*child)->name == "Name" )					(*child)->GetValue( sName );
		else if( (*child)->name == "Grade" )			(*child)->GetValue( (int&)grade );
		else if( (*child)->name == "Score" )			(*child)->GetValue( iScore );
		else if( (*child)->name == "PercentDP" )		(*child)->GetValue( fPercentDP );
		else if( (*child)->name == "SurviveSeconds" )	(*child)->GetValue( fSurviveSeconds );
		else if( (*child)->name == "Modifiers" )		(*child)->GetValue( sModifiers );
		else if( (*child)->name == "Time" )				(*child)->GetValue( (int&)time );
		else if( (*child)->name == "MachineGuid" )		(*child)->GetValue( sMachineGuid );
	}

	/* Validate input. */
	grade = clamp( grade, GRADE_TIER_1, GRADE_FAILED );
}

CString HighScore::GetDisplayName() const
{
	if( sName.empty() )
		return EMPTY_NAME;
	else
		return sName;
}


void HighScoreList::Init()
{
	iNumTimesPlayed = 0;
	vHighScores.clear();
}

void HighScoreList::AddHighScore( HighScore hs, int &iIndexOut )
{
	int i;
	for( i=0; i<(int)vHighScores.size(); i++ )
	{
		if( hs >= vHighScores[i] )
			break;
	}
	const int iMaxScores = PREFSMAN->m_iMaxHighScoresPerList;
	if( i < iMaxScores )
	{
		vHighScores.insert( vHighScores.begin()+i, hs );
		iIndexOut = i;
		if( vHighScores.size() > unsigned(iMaxScores) )
			vHighScores.erase( vHighScores.begin()+iMaxScores, vHighScores.end() );
	}
}

const HighScore& HighScoreList::GetTopScore() const
{
	if( vHighScores.empty() )
	{
		static HighScore hs;
		hs = HighScore();
		return hs;
	}
	else
	{
		return vHighScores[0];
	}
}

XNode* HighScoreList::CreateNode() const
{
	XNode* pNode = new XNode;
	pNode->name = "HighScoreList";

	pNode->AppendChild( "NumTimesPlayed", iNumTimesPlayed );

	for( unsigned i=0; i<vHighScores.size(); i++ )
	{
		const HighScore &hs = vHighScores[i];
		pNode->AppendChild( hs.CreateNode() );
	}

	return pNode;
}

void HighScoreList::LoadFromNode( const XNode* pNode )
{
	Init();

	ASSERT( pNode->name == "HighScoreList" );
	for( XNodes::const_iterator child = pNode->childs.begin();
		child != pNode->childs.end();
		child++)
	{
		if( (*child)->name == "NumTimesPlayed" )
		{
			(*child)->GetValue( iNumTimesPlayed );
		}
		else if( (*child)->name == "HighScore" )
		{
			vHighScores.resize( vHighScores.size()+1 );
			vHighScores.back().LoadFromNode( (*child) );
			
			// ignore all high scores that are 0
			if( vHighScores.back().iScore == 0 )
				vHighScores.pop_back();
		}
	}
}


