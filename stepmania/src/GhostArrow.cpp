#include "global.h"
#include "GhostArrow.h"
#include "NoteSkinManager.h"
#include "RageUtil.h"

GhostArrow::GhostArrow()
{
	FOREACH_TapNoteScore( i )
	{
		m_spr[i].SetHidden( true );
		this->AddChild( &m_spr[i] );
	}
}

void GhostArrow::Load( CString sNoteSkin, CString sButton, CString sElement )
{
	FOREACH_TapNoteScore( i )
	{
		CString sJudge = TapNoteScoreToString( i );
		
		CString sFullElement = sElement  + " " + sJudge;

		// HACK: for backward noteskin compatibility
		CString sPath = NOTESKIN->GetPathToFromNoteSkinAndButton(sNoteSkin, sButton, sFullElement, true);	// optional
		if( sPath.empty() )
			sPath = NOTESKIN->GetPathToFromNoteSkinAndButton(sNoteSkin, sButton, sElement);	// not optional
		m_spr[i].Load( sPath );
	}

	FOREACH_TapNoteScore( i )
	{
		CString sJudge = TapNoteScoreToString( i );
		CString sCommand = Capitalize(sJudge)+"Command";
		m_sScoreCommand[i] = NOTESKIN->GetMetric( sNoteSkin, m_sName, sCommand );
	}
}

void GhostArrow::Init( PlayerNumber pn )
{
	m_PlayerNumber = pn;
}

void GhostArrow::Step( TapNoteScore score )
{
	FOREACH_TapNoteScore( i )
	{
		// HACK: never hide the mine explosion
		if( i == TNS_HIT_MINE )
			continue;
		m_spr[i].StopTweening();
		m_spr[i].SetHidden( true );
	}

	m_spr[score].SetHidden( false );
	m_spr[score].StopTweening();
	m_spr[score].Command( m_sScoreCommand[score] );
}

/*
 * (c) 2001-2004 Ben Nordstrom, Chris Danford
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
