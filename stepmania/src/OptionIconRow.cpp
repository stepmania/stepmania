#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: OptionIconRow

 Desc: A graphic displayed in the OptionIconRow during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "OptionIconRow.h"
#include "ThemeManager.h"
#include "PlayerOptions.h"


#define SPACING_X	THEME->GetMetricF("OptionIconRow","SpacingX")
#define SPACING_Y	THEME->GetMetricF("OptionIconRow","SpacingY")


OptionIconRow::OptionIconRow()
{
	for( int i=0; i<NUM_OPTION_COLS; i++ )
	{
		m_OptionIcon[i].SetXY( i*SPACING_X, i*SPACING_Y );
		this->AddChild( &m_OptionIcon[i] );
	}
}

void OptionIconRow::Refresh( PlayerNumber pn )
{
	for( int i=0; i<NUM_OPTION_COLS; i++ )
		m_OptionIcon[i].Load( pn, "BLAH", i==0 );
}

void OptionIconRow::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();
}