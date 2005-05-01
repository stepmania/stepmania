#include "global.h"
#include "HighScore.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "XmlFile.h"
#include "Foreach.h"

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
	pNode->m_sName = "HighScore";

	// TRICKY:  Don't write "name to fill in" markers.
	pNode->AppendChild( "Name", IsRankingToFillIn(sName) ? CString("") : sName );
	pNode->AppendChild( "Grade",			GradeToString(grade) );
	pNode->AppendChild( "Score",			iScore );
	pNode->AppendChild( "PercentDP",		fPercentDP );
	pNode->AppendChild( "SurviveSeconds",	fSurviveSeconds );
	pNode->AppendChild( "Modifiers",		sModifiers );
	pNode->AppendChild( "DateTime",			dateTime );
	pNode->AppendChild( "PlayerGuid",		sPlayerGuid );
	pNode->AppendChild( "MachineGuid",		sMachineGuid );
	pNode->AppendChild( "ProductID",		iProductID );
	XNode* pTapNoteScores = pNode->AppendChild( "TapNoteScores" );
	FOREACH_TapNoteScore( tns )
		if( tns != TNS_NONE )	// HACK: don't save meaningless "none" count
			pTapNoteScores->AppendChild( TapNoteScoreToString(tns), iTapNoteScores[tns] );
	XNode* pHoldNoteScores = pNode->AppendChild( "HoldNoteScores" );
	FOREACH_HoldNoteScore( hns )
		if( hns != HNS_NONE )	// HACK: don't save meaningless "none" count
			pHoldNoteScores->AppendChild( HoldNoteScoreToString(hns), iHoldNoteScores[hns] );
	pNode->AppendChild( radarValues.CreateNode() );

	return pNode;
}

void HighScore::LoadFromNode( const XNode* pNode ) 
{
	ASSERT( pNode->m_sName == "HighScore" );

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
	pNode->GetChildValue( "DateTime",		dateTime );
	pNode->GetChildValue( "PlayerGuid",		sPlayerGuid );
	pNode->GetChildValue( "MachineGuid",	sMachineGuid );
	pNode->GetChildValue( "ProductID",		iProductID );
	const XNode* pTapNoteScores = pNode->GetChild( "TapNoteScores" );
	if( pTapNoteScores )
		FOREACH_TapNoteScore( tns )
			pTapNoteScores->GetChildValue( TapNoteScoreToString(tns), iTapNoteScores[tns] );
	const XNode* pHoldNoteScores = pNode->GetChild( "HoldNoteScores" );
	if( pHoldNoteScores )
		FOREACH_HoldNoteScore( hns )
			pHoldNoteScores->GetChildValue( HoldNoteScoreToString(hns), iHoldNoteScores[hns] );
	const XNode* pRadarValues = pNode->GetChild( "RadarValues" );
	if( pRadarValues )
		radarValues.LoadFromNode( pRadarValues );

	/* Validate input. */
	grade = clamp( grade, GRADE_TIER01, GRADE_FAILED );
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

void HighScoreList::AddHighScore( HighScore hs, int &iIndexOut, bool bIsMachine )
{
	int i;
	for( i=0; i<(int)vHighScores.size(); i++ )
	{
		if( hs >= vHighScores[i] )
			break;
	}
	const int iMaxScores = bIsMachine ? 
		PREFSMAN->m_iMaxHighScoresPerListForMachine : 
		PREFSMAN->m_iMaxHighScoresPerListForPlayer;
	if( i < iMaxScores )
	{
		vHighScores.insert( vHighScores.begin()+i, hs );
		iIndexOut = i;

		// Delete extra machine high scores in RemoveAllButOneOfEachNameAndClampSize
		// and not here so that we don't end up with less than iMaxScores after 
		// removing HighScores with duplicate names.
		//
		if( !bIsMachine )
			ClampSize( bIsMachine );
	}
}

void HighScoreList::IncrementPlayCount( DateTime dtLastPlayed )
{
	dtLastPlayed = dtLastPlayed;
	iNumTimesPlayed++;
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
	pNode->m_sName = "HighScoreList";

	pNode->AppendChild( "NumTimesPlayed", iNumTimesPlayed );
	pNode->AppendChild( "LastPlayed", dtLastPlayed );

	for( unsigned i=0; i<vHighScores.size(); i++ )
	{
		const HighScore &hs = vHighScores[i];
		pNode->AppendChild( hs.CreateNode() );
	}

	return pNode;
}

void HighScoreList::LoadFromNode( const XNode* pHighScoreList )
{
	Init();

	ASSERT( pHighScoreList->m_sName == "HighScoreList" );
	FOREACH_CONST_Child( pHighScoreList, p )
	{
		if( p->m_sName == "NumTimesPlayed" )
		{
			p->GetValue( iNumTimesPlayed );
		}
		else if( p->m_sName == "LastPlayed" )
		{
			p->GetValue( dtLastPlayed );
		}
		else if( p->m_sName == "HighScore" )
		{
			vHighScores.resize( vHighScores.size()+1 );
			vHighScores.back().LoadFromNode( p );
			
			// ignore all high scores that are 0
			if( vHighScores.back().iScore == 0 )
				vHighScores.pop_back();
		}
	}
}

void HighScoreList::RemoveAllButOneOfEachName()
{
	FOREACH( HighScore, vHighScores, i )
	{
		for( vector<HighScore>::iterator j = i+1; j != vHighScores.end(); j++ )
		{
			if( i->sName == j->sName )
			{
				j--;
				vHighScores.erase( j+1 );
			}
		}
	}
}

void HighScoreList::ClampSize( bool bIsMachine )
{
	const int iMaxScores = bIsMachine ? 
		PREFSMAN->m_iMaxHighScoresPerListForMachine : 
		PREFSMAN->m_iMaxHighScoresPerListForPlayer;
	if( vHighScores.size() > unsigned(iMaxScores) )
		vHighScores.erase( vHighScores.begin()+iMaxScores, vHighScores.end() );
}

XNode* Screenshot::CreateNode() const
{
	XNode* pNode = new XNode;
	pNode->m_sName = "Screenshot";

	// TRICKY:  Don't write "name to fill in" markers.
	pNode->AppendChild( "FileName",		sFileName );
	pNode->AppendChild( "MD5",			sMD5 );
	pNode->AppendChild( highScore.CreateNode() );

	return pNode;
}

void Screenshot::LoadFromNode( const XNode* pNode ) 
{
	ASSERT( pNode->m_sName == "Screenshot" );

	pNode->GetChildValue( "FileName",	sFileName );
	pNode->GetChildValue( "MD5",		sMD5 );
	const XNode* pHighScore = pNode->GetChild( "HighScore" );
	if( pHighScore )
		highScore.LoadFromNode( pHighScore );
}

/*
 * (c) 2004 Chris Danford
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
