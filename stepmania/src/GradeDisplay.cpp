#include "global.h"
#include "GradeDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "RageTexture.h"
#include "arch/Dialog/Dialog.h"
#include "RageLog.h"


const float SCROLL_TIME = 5.0f;
const float QUICK_SCROLL_TIME = .25f;
const int NUM_GRADE_FRAMES = 8;
const float GRADE_FRAME_HEIGHT = 1/(float)NUM_GRADE_FRAMES;
const float GRADES_TO_SCROLL = NUM_GRADE_FRAMES*4;


GradeDisplay::GradeDisplay()
{
	m_fTimeLeftInScroll = 0;
	m_bDoScrolling = 0;

	SetGrade( PLAYER_1, Grade_NoData );
}

void GradeDisplay::Load( RageTextureID ID )
{
	ID.bStretch = true;
	Sprite::Load( ID );
	Sprite::StopAnimating();

	bool bWarn = Sprite::GetNumStates() != 8 && Sprite::GetNumStates() != 16;
	if( ID.filename.find("_blank") != RString::npos )
		bWarn = false;
	if( bWarn )
	{
		RString sError = ssprintf( "The grade graphic '%s' must have either 8 or 16 frames.", ID.filename.c_str() );
		LOG->Warn( sError );
		Dialog::OK( sError );
	}
}

void GradeDisplay::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );

	if( m_bDoScrolling )
	{
		m_fTimeLeftInScroll -= fDeltaTime;
		m_fTimeLeftInScroll = max( 0, m_fTimeLeftInScroll );

		float fPercentIntoScrolling;
		if( m_bDoScrolling == 1)
		{
			fPercentIntoScrolling = 1 - (m_fTimeLeftInScroll/SCROLL_TIME);
			if( fPercentIntoScrolling < 0.75 )
				fPercentIntoScrolling = (fPercentIntoScrolling/0.75f) * (1 + 1.0f/NUM_GRADE_FRAMES);
			else if( fPercentIntoScrolling < 0.9 )
				fPercentIntoScrolling = 1 + 1.0f/NUM_GRADE_FRAMES;
			else
				fPercentIntoScrolling = (1 + 1.0f/NUM_GRADE_FRAMES) - ((fPercentIntoScrolling-0.9f)/0.1f) * 1.0f/NUM_GRADE_FRAMES;
		} else {
			fPercentIntoScrolling = 1 - (m_fTimeLeftInScroll/QUICK_SCROLL_TIME);
		}
		
		m_frectCurTexCoords.left   = m_frectStartTexCoords.left*(1-fPercentIntoScrolling)   + m_frectDestTexCoords.left*fPercentIntoScrolling;
		m_frectCurTexCoords.top    = m_frectStartTexCoords.top*(1-fPercentIntoScrolling)    + m_frectDestTexCoords.top*fPercentIntoScrolling;
		m_frectCurTexCoords.right  = m_frectStartTexCoords.right*(1-fPercentIntoScrolling)  + m_frectDestTexCoords.right*fPercentIntoScrolling;
		m_frectCurTexCoords.bottom = m_frectStartTexCoords.bottom*(1-fPercentIntoScrolling) + m_frectDestTexCoords.bottom*fPercentIntoScrolling;

		this->SetCustomTextureRect( m_frectCurTexCoords );
	}
}

int GradeDisplay::GetFrameIndex( PlayerNumber pn, Grade g )
{
	if( this->m_pTexture->GetID().filename.find("_blank") != string::npos )
		return 0;

	// either 8, or 16 states
	int iNumCols;
	switch( Sprite::GetNumStates() )
	{
	case 8:		iNumCols=1;	break;
	case 16:	iNumCols=2;	break;
	default:
		ASSERT(0);
	}

	int iFrame;
	switch( g )
	{
	case Grade_Tier01:	iFrame = 0;	break;
	case Grade_Tier02:	iFrame = 1;	break;
	case Grade_Tier03:	iFrame = 2;	break;
	case Grade_Tier04:	iFrame = 3;	break;
	case Grade_Tier05:	iFrame = 4;	break;
	case Grade_Tier06:	iFrame = 5;	break;
	case Grade_Tier07:	iFrame = 6;	break;
	case Grade_Failed:	iFrame = 7;	break;
	default:			iFrame = 7;	break;
	}
	iFrame *= iNumCols;
	if( iNumCols==2 )
		iFrame += pn;
	return iFrame;
}

void GradeDisplay::SetGrade( PlayerNumber pn, Grade g )
{
	m_PlayerNumber = pn;
	m_Grade = g;

	m_bDoScrolling = false;
	StopUsingCustomCoords();

	if(g != Grade_NoData)
	{
		SetState( GetFrameIndex(pn,g) );
		SetVisible( true );
		SetDiffuseAlpha(1);
	}
	else
	{
		SetVisible( false );
		SetDiffuseAlpha(0);
	}
}

void GradeDisplay::Scroll()
{
	m_bDoScrolling = true;

	int iFrameNo = GetFrameIndex( m_PlayerNumber, m_Grade );

	m_frectDestTexCoords = *m_pTexture->GetTextureCoordRect( iFrameNo );
	m_frectStartTexCoords = m_frectDestTexCoords;
	m_frectStartTexCoords.top += GRADES_TO_SCROLL * GRADE_FRAME_HEIGHT;
	m_frectStartTexCoords.bottom += GRADES_TO_SCROLL * GRADE_FRAME_HEIGHT;

	m_fTimeLeftInScroll = SCROLL_TIME;

	/* Set the initial position. */
	Update(0);
}

void GradeDisplay::SettleImmediately()
{
	m_fTimeLeftInScroll = 0;
}

void GradeDisplay::SettleQuickly()
{
	if(m_bDoScrolling != 1)
		return;

	/* If we're in the last phase of scrolling, don't do this. */
	if( 1 - (m_fTimeLeftInScroll/SCROLL_TIME) >= 0.9 )
		return;

	/* m_frectDestTexCoords.top is between 0 and 1 (inclusive).  m_frectCurTexCoords
	 * is somewhere above that.  Shift m_frectCurTexCoords downwards so it's pointing
	 * at the same physical place (remember, the grade texture is tiled) but no more
	 * than one rotation away from the destination. */
	while(m_frectCurTexCoords.top > m_frectDestTexCoords.top + 1.0f)
	{
		m_frectCurTexCoords.top -= 1.0f;
		m_frectCurTexCoords.bottom -= 1.0f;
	}

	m_frectStartTexCoords = m_frectCurTexCoords;
	m_bDoScrolling = 2;
	m_fTimeLeftInScroll = QUICK_SCROLL_TIME;
}

// lua start
#include "LuaBinding.h"

class LunaGradeDisplay: public Luna<GradeDisplay>
{
public:
	static int scroll( T* p, lua_State *L ) { p->Scroll(); return 0; }

	LunaGradeDisplay()
	{
		ADD_METHOD( scroll );
	}
};

LUA_REGISTER_DERIVED_CLASS( GradeDisplay, Sprite )
// lua end

/*
 * (c) 2001-2002 Chris Danford
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
