#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Profile

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Profile.h"
#include "RageUtil.h"
#include "PrefsManager.h"

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

void HighScoreList::AddHighScore( HighScore hs, int &iIndexOut )
{
	int i;
	for( i=0; i<(int)vHighScores.size(); i++ )
	{
		if( hs >= vHighScores[i] )
			break;
	}
	if( i < NUM_RANKING_LINES )
	{
		vHighScores.insert( vHighScores.begin()+i, hs );
		iIndexOut = i;
		if( vHighScores.size() > unsigned(NUM_RANKING_LINES) )
			vHighScores.erase( vHighScores.begin()+NUM_RANKING_LINES, vHighScores.end() );
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

CString Profile::GetDisplayName()
{
	if( !m_sName.empty() )
		return m_sName;
	else if( !m_sLastUsedHighScoreName.empty() )
		return m_sLastUsedHighScoreName;
	else
		return "NO NAME";
}

CString Profile::GetDisplayCaloriesBurned()
{
	if( m_fWeightPounds == 0 )	// weight not entered
		return "N/A";
	else 
		return ssprintf("%iCal",m_fCaloriesBurned);
}

int Profile::GetTotalNumSongsPlayed()
{
	int iTotal = 0;
	for( int i=0; i<NUM_PLAY_MODES; i++ )
		iTotal += m_iNumSongsPlayedByPlayMode[i];
	return iTotal;
}


//
// Steps high scores
//
void Profile::AddStepsHighScore( const Steps* pSteps, HighScore hs, int &iIndexOut )
{
	std::map<const Steps*,HighScoresForASteps>::iterator iter = m_StepsHighScores.find( pSteps );
	if( iter == m_StepsHighScores.end() )
		m_StepsHighScores[pSteps].hs.AddHighScore( hs, iIndexOut );	// operator[] inserts into map
	else
		iter->second.hs.AddHighScore( hs, iIndexOut );
}

HighScoreList& Profile::GetStepsHighScoreList( const Steps* pSteps )
{
	std::map<const Steps*,HighScoresForASteps>::iterator iter = m_StepsHighScores.find( pSteps );
	if( iter == m_StepsHighScores.end() )
		return m_StepsHighScores[pSteps].hs;	// operator[] inserts into map
	else
		return iter->second.hs;
}

int Profile::GetStepsNumTimesPlayed( const Steps* pSteps ) const
{
	std::map<const Steps*,HighScoresForASteps>::const_iterator iter = m_StepsHighScores.find( pSteps );
	if( iter == m_StepsHighScores.end() )
	{
		return 0;
	}
	else
	{
		int iTotalNumTimesPlayed = 0;
		for( unsigned st = 0; st < NUM_STEPS_TYPES; ++st )
			iTotalNumTimesPlayed += iter->second.hs.iNumTimesPlayed;
		return iTotalNumTimesPlayed;
	}
}

void Profile::IncrementStepsPlayCount( const Steps* pSteps )
{
	GetStepsHighScoreList(pSteps).iNumTimesPlayed++;
}


//
// Course high scores
//
void Profile::AddCourseHighScore( const Course* pCourse, StepsType st, HighScore hs, int &iIndexOut )
{
	std::map<const Course*,HighScoresForACourse>::iterator iter = m_CourseHighScores.find( pCourse );
	if( iter == m_CourseHighScores.end() )
		m_CourseHighScores[pCourse].hs[st].AddHighScore( hs, iIndexOut );	// operator[] inserts into map
	else
		iter->second.hs[st].AddHighScore( hs, iIndexOut );
}

HighScoreList& Profile::GetCourseHighScoreList( const Course* pCourse, StepsType st )
{
	std::map<const Course*,HighScoresForACourse>::iterator iter = m_CourseHighScores.find( pCourse );
	if( iter == m_CourseHighScores.end() )
		return m_CourseHighScores[pCourse].hs[st];	// operator[] inserts into map
	else
		return iter->second.hs[st];
}

int Profile::GetCourseNumTimesPlayed( const Course* pCourse ) const
{
	std::map<const Course*,HighScoresForACourse>::const_iterator iter = m_CourseHighScores.find( pCourse );
	if( iter == m_CourseHighScores.end() )
	{
		return 0;
	}
	else
	{
		int iTotalNumTimesPlayed = 0;
		for( unsigned st = 0; st < NUM_STEPS_TYPES; ++st )
			iTotalNumTimesPlayed += iter->second.hs[st].iNumTimesPlayed;
		return iTotalNumTimesPlayed;
	}
}

void Profile::IncrementCoursePlayCount( const Course* pCourse, StepsType st )
{
	GetCourseHighScoreList(pCourse,st).iNumTimesPlayed++;
}

//
// Category high scores
//
void Profile::AddCategoryHighScore( StepsType st, RankingCategory rc, HighScore hs, int &iIndexOut )
{
	m_CategoryHighScores[st][rc].AddHighScore( hs, iIndexOut );
}

HighScoreList& Profile::GetCategoryHighScoreList( StepsType st, RankingCategory rc )
{
	return m_CategoryHighScores[st][rc];
}

int Profile::GetCategoryNumTimesPlayed( RankingCategory rc ) const
{
	int iNumTimesPlayed = 0;
	for( int st=0; st<NUM_STEPS_TYPES; st++ )
		iNumTimesPlayed += m_CategoryHighScores[st][rc].iNumTimesPlayed;
	return iNumTimesPlayed;
}

void Profile::IncrementCategoryPlayCount( StepsType st, RankingCategory rc )
{
	m_CategoryHighScores[st][rc].iNumTimesPlayed++;
}
