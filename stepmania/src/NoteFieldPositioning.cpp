#include "global.h"
#include "NoteFieldPositioning.h"

#include "RageDisplay.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageMath.h"

#include "GameState.h"
#include "GameManager.h"
#include "IniFile.h"
#include "Game.h"
#include "ScreenDimensions.h"
#include "PlayerState.h"
#include "XmlFile.h"

/* Copies of the current mode.  Update this by calling Load. */
NoteFieldMode g_NoteFieldMode[NUM_PLAYERS];

NoteFieldMode::NoteFieldMode()
{
	m_fFov = 0;
	m_fNear = 5;
	m_fFar = 1000;
	m_fFirstPixelToDrawScale = m_fLastPixelToDrawScale = 1.0f;
}

void NoteFieldMode::BeginDrawTrack(int tn)
{
	DISPLAY->CameraPushMatrix();

	/* It's useful to be able to use Actors like this, functioning only
	 * for a transformation.  However, this is a big waste of matrix
	 * stack space, as each of these will push.  Profile this. XXX */
	if(m_fFov) 
		DISPLAY->LoadMenuPerspective(m_fFov, SCREEN_CENTER_X, SCREEN_CENTER_Y);
	
	if(tn != -1)
	{
		DISPLAY->PushMatrix();
		DISPLAY->Translate( m_fPositionTrackX[tn], 0, 0 );
	}
}

void NoteFieldMode::EndDrawTrack(int tn)
{
	if(tn != -1)
		DISPLAY->PopMatrix();

	DISPLAY->CameraPopMatrix();
}

template <class T>
static bool GetValue(const XNode *pNode, int pn, const CString &valuename, T &value )
{
	if(pn != -1 && pNode->GetAttrValue( ssprintf("P%i%s", pn+1, valuename.c_str()), value))
		return true;
	return pNode->GetAttrValue( valuename, value );
}

void NoteFieldMode::Load(const XNode *pNode, int pn)
{
	m_Id = pNode->m_sName;

	/* Required: */
	ASSERT( pNode->GetAttrValue("Name",			m_Name ) );

	// if we aren't loading a player, we can bail here.
	if(pn == -1)
		return;

	GetValue( pNode, pn, "Backdrop",				m_Backdrop );
	GetValue( pNode, pn, "FOV",					m_fFov );
	GetValue( pNode, pn, "NearClipDistance",		m_fNear );
	GetValue( pNode, pn, "FarClipDistance",		m_fFar );
	GetValue( pNode, pn, "PixelsDrawAheadScale",	m_fFirstPixelToDrawScale );
	GetValue( pNode, pn, "PixelsDrawBehindScale",	m_fLastPixelToDrawScale );

	CString s;
	if( GetValue( pNode, pn, "Judgment",			s ) )	m_JudgmentCmd = ParseCommands(s);
	if( GetValue( pNode, pn, "Combo",				s ) )	m_ComboCmd = ParseCommands(s);

	/* Load per-track data: */
	for( int t = 0; t < MAX_NOTE_TRACKS; ++t )
	{
		GetValue( pNode, pn, ssprintf("GrayButton"), GrayButtonNames[t] );
		GetValue( pNode, pn, ssprintf("GrayButton%i", t+1), GrayButtonNames[t] );

		GetValue( pNode, pn, ssprintf("NoteButton"), NoteButtonNames[t] );
		GetValue( pNode, pn, ssprintf("NoteButton%i", t+1), NoteButtonNames[t] );

		GetValue( pNode, pn, ssprintf("GhostButton"), GhostButtonNames[t] );
		GetValue( pNode, pn, ssprintf("GhostButton%i", t+1), GhostButtonNames[t] );

		if( GetValue( pNode, pn, ssprintf("HoldJudgment"),       s ) )	m_HoldJudgmentCmd[t] = ParseCommands(s);
		if( GetValue( pNode, pn, ssprintf("HoldJudgment%i",t+1), s ) )	m_HoldJudgmentCmd[t] = ParseCommands(s);
	}
}

NoteFieldPositioning::NoteFieldPositioning(CString fn)
{
	m_Filename = fn;
	IniFile ini;
	if( !ini.ReadFile(fn) )
		return;

	FOREACH_CONST_Child( &ini, i )
	{
		NoteFieldMode m;
		m.Load( i );

		Modes.push_back(m);
	}
}

bool NoteFieldMode::MatchesCurrentGame() const
{
	return true;
}

void NoteFieldPositioning::Load(PlayerNumber pn)
{
	NoteFieldMode &mode = g_NoteFieldMode[pn];

	mode = NoteFieldMode(); /* reset */

	const Style *s = GAMESTATE->GetCurrentStyle();

	/* Load the settings in the style table by default. */
	for(int tn = 0; tn < MAX_NOTE_TRACKS; ++tn)
	{
		const float fPixelXOffsetFromCenter = s->m_ColumnInfo[pn][tn].fXOffset;
		mode.m_fPositionTrackX[tn] = fPixelXOffsetFromCenter;
	}

	/* Is there a custom mode with the current name that fits the current game? */
	const int ModeNum = GetID(GAMESTATE->m_pPlayerState[pn]->m_PlayerOptions.m_sPositioning);
	if(ModeNum == -1)
		return; /* No, only use the style table settings. */

	/* We have a custom mode.  Reload the mode on top of the default style
	 * table settings. */
	IniFile ini;
	if( !ini.ReadFile(m_Filename) )
		return;

	XNode* pNode = ini.GetChild( Modes[ModeNum].m_Id );
	ASSERT( pNode );
	mode.Load( pNode, pn );
}


/* Get the unique ID of the given name, for the current game/style.  If it
 * doesn't exist, return "". */
int NoteFieldPositioning::GetID(const CString &name) const
{
	for(unsigned i = 0; i < Modes.size(); ++i)
	{
		if(Modes[i].m_Name.CompareNoCase(name))
			continue;

		if(!Modes[i].MatchesCurrentGame())
			continue;

		return i;
	}

	return -1;
}


/* Get all arrow modifier names for the current game. */
void NoteFieldPositioning::GetNamesForCurrentGame(vector<CString> &IDs)
{ // XXX dupes
	/* Iterate over all keys. */
	for(unsigned i = 0; i < Modes.size(); ++i)
	{
		if(!Modes[i].MatchesCurrentGame())
			continue;
		
		IDs.push_back(Modes[i].m_Name);
	}
}

bool NoteFieldPositioning::IsValidModeForAnyStyle(CString mode) const
{
	for(unsigned i = 0; i < Modes.size(); ++i)
		if(Modes[i].m_Name.CompareNoCase(mode)==0)
			return true;

	return false;
}

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
