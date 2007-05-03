#include "global.h"
#include "GradeDisplay.h"
#include "RageUtil.h"
#include "arch/Dialog/Dialog.h"
#include "RageLog.h"

void GradeDisplay::Load( RageTextureID ID )
{
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

int GradeDisplay::GetFrameIndex( PlayerNumber pn, Grade g )
{
	if( Sprite::GetNumStates() == 1 )
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
	default:		iFrame = 6;	break;
	case Grade_Failed:	iFrame = 7;	break;
	}
	iFrame *= iNumCols;
	if( iNumCols==2 )
		iFrame += pn;
	return iFrame;
}

void GradeDisplay::SetGrade( PlayerNumber pn, Grade g )
{
	if( g != Grade_NoData )
	{
		SetState( GetFrameIndex(pn,g) );
		SetVisible( true );
	}
	else
	{
		SetVisible( false );
	}
}

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
