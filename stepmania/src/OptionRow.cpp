#include "global.h"
#include "OptionRow.h"
#include "RageUtil.h"

static const CString SelectTypeNames[NUM_SELECT_TYPES] = {
	"SelectOne",
	"SelectMultiple",
	"SelectNone",
};
XToString( SelectType );
StringToX( SelectType );

static const CString LayoutTypeNames[NUM_LAYOUT_TYPES] = {
	"ShowAllInLine",
	"ShowOneLine",
};
XToString( LayoutType );
StringToX( LayoutType );


OptionRow::OptionRow()
{
}

OptionRow::~OptionRow()
{
	for( unsigned i = 0; i < m_textItems.size(); ++i )
		SAFE_DELETE( m_textItems[i] );
	FOREACH_PlayerNumber( p )
		for( unsigned i = 0; i < m_Underline[p].size(); ++i )
			SAFE_DELETE( m_Underline[p][i] );
}


int OptionRow::GetOneSelection( PlayerNumber pn ) const
{
	for( unsigned i=0; i<(unsigned)m_vbSelected[pn].size(); i++ )
		if( m_vbSelected[pn][i] )
			return i;
	ASSERT(0);	// shouldn't call this if not expecting one to be selected
	return -1;
}

int OptionRow::GetOneSharedSelection() const
{
	return GetOneSelection( (PlayerNumber)0 );
}

void OptionRow::SetOneSelection( PlayerNumber pn, int iChoice )
{
	for( unsigned i=0; i<(unsigned)m_vbSelected[pn].size(); i++ )
		m_vbSelected[pn][i] = false;
	m_vbSelected[pn][iChoice] = true;
}

void OptionRow::SetOneSharedSelection( int iChoice )
{
	FOREACH_HumanPlayer( pn )
		SetOneSelection( pn, iChoice );
}

/*
 * (c) 2001-2004 Chris Danford
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
