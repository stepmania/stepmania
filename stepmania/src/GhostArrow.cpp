#include "global.h"
#include "GhostArrow.h"
#include "NoteSkinManager.h"
#include "RageUtil.h"

GhostArrow::GhostArrow()
{
}

void GhostArrow::Load( const CString &sButton, const CString &sElement )
{
	FOREACH_TapNoteScore( i )
	{
		CString sJudge = TapNoteScoreToString( i );
		
		CString sFullElement = sElement  + " " + sJudge;

		ASSERT( !m_sprTap[i].IsLoaded() );	// don't double-load

		m_sprTap[i].Load( NOTESKIN->GetPath(sButton, sFullElement) );
		m_sprTap[i]->SetHidden( true );
		this->AddChild( m_sprTap[i] );

		
		CString sCommand = Capitalize(sJudge)+"Command";
		m_acTap[i] = NOTESKIN->GetMetricA(m_sName,sCommand);
	}

	FOREACH_HoldNoteScore( i )
	{
		CString sJudge = HoldNoteScoreToString( i );
		
		CString sFullElement = sElement  + " " + sJudge;

		ASSERT( !m_sprHold[i].IsLoaded() );	// don't double-load

		m_sprHold[i].Load( NOTESKIN->GetPath(sButton, sFullElement) );
		m_sprHold[i]->SetHidden( true );
		this->AddChild( m_sprHold[i] );


		CString sCommand = Capitalize(sJudge)+"Command";
		m_acHold[i] = NOTESKIN->GetMetricA(m_sName,sCommand);
	}
}

void GhostArrow::StepTap( TapNoteScore score )
{
	FOREACH_TapNoteScore( i )
	{
		// HACK: never hide the mine explosion
		if( i == TNS_HIT_MINE )
			continue;
		m_sprTap[i]->StopTweening();
		m_sprTap[i]->SetHidden( true );
	}
	FOREACH_HoldNoteScore( i )
	{
		m_sprHold[i]->StopTweening();
		m_sprHold[i]->SetHidden( true );
	}

	m_sprTap[score]->SetHidden( false );
	m_sprTap[score]->StopTweening();
	m_sprTap[score]->RunCommands( m_acTap[score] );
}

void GhostArrow::StepHold( HoldNoteScore score )
{
	FOREACH_TapNoteScore( i )
	{
		// HACK: never hide the mine explosion
		if( i == TNS_HIT_MINE )
			continue;
		m_sprTap[i]->StopTweening();
		m_sprTap[i]->SetHidden( true );
	}
	FOREACH_HoldNoteScore( i )
	{
		m_sprHold[i]->StopTweening();
		m_sprHold[i]->SetHidden( true );
	}

	m_sprHold[score]->SetHidden( false );
	m_sprHold[score]->StopTweening();
	m_sprHold[score]->RunCommands( m_acHold[score] );
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
