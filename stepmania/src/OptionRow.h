/* OptionRow - One line in ScreenOptions. */

#ifndef OptionRow_H
#define OptionRow_H

#include "ActorFrame.h"
#include "BitmapText.h"
#include "OptionsCursor.h"
#include "OptionIcon.h"

enum SelectType { SELECT_ONE, SELECT_MULTIPLE, SELECT_NONE, NUM_SELECT_TYPES, SELECT_INVALID };
const CString& SelectTypeToString( SelectType pm );
SelectType StringToSelectType( const CString& s );

enum LayoutType { SHOW_ALL_IN_LINE, SHOW_ONE_IN_LINE, NUM_LAYOUT_TYPES, LAYOUT_INVALID };
const CString& LayoutTypeToString( LayoutType pm );
LayoutType StringToLayoutType( const CString& s );

struct OptionRowDefinition
{
	CString name;
	bool bOneChoiceForAllPlayers;
	enum SelectType selectType;
	enum LayoutType layoutType;
	vector<CString> choices;

	OptionRowDefinition(): name(""), bOneChoiceForAllPlayers(false), selectType(SELECT_ONE), layoutType(SHOW_ALL_IN_LINE) { }

	OptionRowDefinition( const char *n, bool b, const char *c0=NULL, const char *c1=NULL, const char *c2=NULL, const char *c3=NULL, const char *c4=NULL, const char *c5=NULL, const char *c6=NULL, const char *c7=NULL, const char *c8=NULL, const char *c9=NULL, const char *c10=NULL, const char *c11=NULL, const char *c12=NULL, const char *c13=NULL, const char *c14=NULL, const char *c15=NULL, const char *c16=NULL, const char *c17=NULL, const char *c18=NULL, const char *c19=NULL ) :
		name(n), bOneChoiceForAllPlayers(b), selectType(SELECT_ONE), layoutType(SHOW_ALL_IN_LINE)
	{
#define PUSH( c )	if(c) choices.push_back(c);
		PUSH(c0);PUSH(c1);PUSH(c2);PUSH(c3);PUSH(c4);PUSH(c5);PUSH(c6);PUSH(c7);PUSH(c8);PUSH(c9);PUSH(c10);PUSH(c11);PUSH(c12);PUSH(c13);PUSH(c14);PUSH(c15);PUSH(c16);PUSH(c17);PUSH(c18);PUSH(c19);
#undef PUSH
	}
};

class OptionRow : public ActorFrame
{
public:
	OptionRow();
	~OptionRow();
	OptionRowDefinition		m_RowDef;
	enum { ROW_NORMAL, ROW_EXIT } Type;
	vector<BitmapText *>	m_textItems;				// size depends on m_bRowIsLong and which players are joined
	vector<OptionsCursor *>	m_Underline[NUM_PLAYERS];	// size depends on m_bRowIsLong and which players are joined
	Sprite					m_sprBullet;
	BitmapText				m_textTitle;
	OptionIcon				m_OptionIcons[NUM_PLAYERS];

	float m_fY;
	bool m_bHidden; // currently off screen

	int m_iChoiceInRowWithFocus[NUM_PLAYERS];	// this choice has input focus

	// Only one will true at a time if m_RowDef.bMultiSelect
	vector<bool> m_vbSelected[NUM_PLAYERS];	// size = m_RowDef.choices.size().
	int GetOneSelection( PlayerNumber pn ) const
	{
		for( unsigned i=0; i<(unsigned)m_vbSelected[pn].size(); i++ )
			if( m_vbSelected[pn][i] )
				return i;
		ASSERT(0);	// shouldn't call this if not expecting one to be selected
		return -1;
	}
	int GetOneSharedSelection() const
	{
		return GetOneSelection( (PlayerNumber)0 );
	}
	void SetOneSelection( PlayerNumber pn, int iChoice )
	{
		for( unsigned i=0; i<(unsigned)m_vbSelected[pn].size(); i++ )
			m_vbSelected[pn][i] = false;
		m_vbSelected[pn][iChoice] = true;
	}
	void SetOneSharedSelection( int iChoice )
	{
		FOREACH_HumanPlayer( pn )
			SetOneSelection( pn, iChoice );
	}
};

#endif

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
