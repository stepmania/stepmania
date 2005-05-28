/* OptionRowHandler - Shows PlayerOptions and SongOptions in icon form. */

#ifndef OptionRowHandler_H
#define OptionRowHandler_H

#include "OptionRow.h"
#include "GameCommand.h"
#include "LuaReference.h"
#include "RageUtil.h"

struct ConfOption;

class OptionRowHandler
{
public:
	CString m_sName;
	vector<CString> m_vsReloadRowMessages;	// refresh this row on on these messages
	
	OptionRowHandler::OptionRowHandler() { Init(); }
	virtual ~OptionRowHandler() { }
	virtual void Init()
	{
		m_sName = "";
		m_vsReloadRowMessages.clear();
	}
	virtual void Load( OptionRowDefinition &defOut, CString sParam ) = 0;
	virtual void Reload( OptionRowDefinition &defOut ) { this->Load(defOut,m_sName); }
	virtual void ImportOption( const OptionRowDefinition &row, const vector<PlayerNumber> &vpns, vector<bool> vbSelectedOut[NUM_PLAYERS] ) const = 0;
	/* Returns an OPT mask. */
	virtual int ExportOption( const OptionRowDefinition &def, const vector<PlayerNumber> &vpns, const vector<bool> vbSelected[NUM_PLAYERS] ) const = 0;
	virtual CString GetIconText( const OptionRowDefinition &def, int iFirstSelection ) const { return ""; }
	virtual bool HasScreen( int iChoice ) const { return false; }
};


namespace OptionRowHandlerUtil
{
	OptionRowHandler* Make( const Command &cmd, OptionRowDefinition &defOut );
}

inline void VerifySelected( SelectType st, const vector<bool> &vbSelected, const CString &sName )
{
	int iNumSelected = 0;
	if( st == SELECT_ONE )
	{
		ASSERT_M( vbSelected.size() > 0, ssprintf("%s: %i/%i", sName.c_str(), iNumSelected, int(vbSelected.size())) );
		for( unsigned e = 0; e < vbSelected.size(); ++e )
			if( vbSelected[e] )
				iNumSelected++;
		ASSERT_M( iNumSelected == 1, ssprintf("%s: %i/%i", sName.c_str(), iNumSelected, int(vbSelected.size())) );
	}
}

#endif

/*
 * (c) 2002-2004 Chris Danford
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
