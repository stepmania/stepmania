#include "global.h"
#include "DifficultyDisplay.h"
#include "song.h"
#include "ThemeManager.h"

#define ICONONCOMMAND			THEME->GetMetric ("DifficultyDisplay","IconOnCommand")
#define ICONOFFCOMMAND			THEME->GetMetric ("DifficultyDisplay","IconOffCommand")

DifficultyDisplay::DifficultyDisplay()
{
	float fWidth = 0;
	int diff;
	for( diff = DIFFICULTY_BEGINNER; diff <= DIFFICULTY_CHALLENGE; ++diff )
	{
		m_difficulty[diff].Load( THEME->GetPathToG("DifficultyDisplay bar 5x1") );
		m_difficulty[diff].SetState(diff);
		m_difficulty[diff].StopAnimating();
		this->AddChild( &m_difficulty[diff] );

		fWidth += m_difficulty[diff].GetUnzoomedWidth();
	}

	float fX = -fWidth/2;
	for( diff = DIFFICULTY_BEGINNER; diff <= DIFFICULTY_CHALLENGE; ++diff )
	{
		m_difficulty[diff].SetHorizAlign( align_left );
		m_difficulty[diff].SetX( fX );
		fX += m_difficulty[diff].GetUnzoomedWidth();
	}
}

void DifficultyDisplay::SetDifficulties( const Song* pSong, NotesType curType )
{
	for( int diff = DIFFICULTY_BEGINNER; diff <= DIFFICULTY_CHALLENGE; ++diff )
	{
		if( pSong->SongHasNotesTypeAndDifficulty( curType, Difficulty(diff) ) )
			m_difficulty[diff].Command( ICONONCOMMAND );
		else
			m_difficulty[diff].Command( ICONOFFCOMMAND );
	}
}

/*
   Copyright (c) 2003 by the person(s) listed below. All rights reserved.
     Steven Towle
*/
