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
#include "XmlFile.h"

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
	pNode->AppendChild( "Grade",			GradeToString(grade) );
	pNode->AppendChild( "Score",			iScore );
	pNode->AppendChild( "PercentDP",		fPercentDP );
	pNode->AppendChild( "SurviveSeconds",	fSurviveSeconds );
	pNode->AppendChild( "Modifiers",		sModifiers );
	pNode->AppendChild( "Time",				(int)time );
	pNode->AppendChild( "PlayerGuid",		sPlayerGuid );
	pNode->AppendChild( "MachineGuid",		sMachineGuid );
	pNode->AppendChild( "ProductID",		iProductID );
	XNode* pTapNoteScores = pNode->AppendChild( "TapNoteScores" );
	FOREACH_TapNoteScore( tns )
		pTapNoteScores->AppendChild( TapNoteScoreToString(tns), iTapNoteScores[tns] );
	XNode* pHoldNoteScores = pNode->AppendChild( "HoldNoteScores" );
	FOREACH_HoldNoteScore( hns )
		pHoldNoteScores->AppendChild( HoldNoteScoreToString(hns), iHoldNoteScores[hns] );
	XNode* pRadarCategories = pNode->AppendChild( "RadarActuals" );
	FOREACH_RadarCategory( rc )
		pRadarCategories->AppendChild( RadarCategoryToString(rc), fRadarActual[rc] );

	return pNode;
}

void HighScore::LoadFromNode( const XNode* pNode ) 
{
	ASSERT( pNode->name == "HighScore" );

	CString s;

	pNode->GetChildValue( "Name", sName );
	pNode->GetChildValue( "Grade", s );
	/* Pre-a19 compatibility; remove eventually */
	if( IsAnInt(s) )
		grade = (Grade) atoi( s );
	else
		grade = StringToGrade( s );
	pNode->GetChildValue( "Score",			iScore );
	pNode->GetChildValue( "PercentDP",		fPercentDP );
	pNode->GetChildValue( "SurviveSeconds", fSurviveSeconds );
	pNode->GetChildValue( "Modifiers",		sModifiers );
	pNode->GetChildValue( "Time",			(int&)time );
	pNode->GetChildValue( "PlayerGuid",		sPlayerGuid );
	pNode->GetChildValue( "MachineGuid",	sMachineGuid );
	pNode->GetChildValue( "ProductID",		iProductID );
	XNode* pTapNoteScores = pNode->GetChild( "TapNoteScores" );
	if( pTapNoteScores )
		FOREACH_TapNoteScore( tns )
			pTapNoteScores->GetChildValue( TapNoteScoreToString(tns), iTapNoteScores[tns] );
	XNode* pHoldNoteScores = pNode->GetChild( "HoldNoteScores" );
	if( pHoldNoteScores )
		FOREACH_HoldNoteScore( hns )
			pHoldNoteScores->GetChildValue( HoldNoteScoreToString(hns), iHoldNoteScores[hns] );
	XNode* pRadarCategories = pNode->GetChild( "RadarActuals" );
	if( pRadarCategories )
		FOREACH_RadarCategory( rc )
			pRadarCategories->GetChildValue( RadarCategoryToString(rc), fRadarActual[rc] );

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


