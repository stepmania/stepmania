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
#include "Style.h"
#include "GameDef.h"

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
	DISPLAY->PushMatrix();

	/* It's useful to be able to use Actors like this, functioning only
	 * for a transformation.  However, this is a big waste of matrix
	 * stack space, as each of these will push.  Profile this. XXX */
	m_Center.BeginDraw();
	if(tn != -1)
		m_CenterTrack[tn].BeginDraw();

	if(m_fFov) 
		DISPLAY->EnterPerspective(m_fFov, true, m_fNear, m_fFar);
	
	m_Position.BeginDraw();
	if(tn == -1)
		m_PositionBackdrop.BeginDraw();
	else
		m_PositionTrack[tn].BeginDraw();
}

void NoteFieldMode::EndDrawTrack(int tn)
{
	if(tn == -1)
		m_PositionBackdrop.EndDraw();
	else
		m_PositionTrack[tn].EndDraw();
	m_Position.EndDraw();

	if(m_fFov)
		DISPLAY->ExitPerspective();

	if(tn != -1)
		m_CenterTrack[tn].EndDraw();
	m_Center.EndDraw();

	DISPLAY->PopMatrix();
}

static bool GetValueCmd(IniFile &ini,
				   const CString &key, const CString &valuename, Actor &value )
{
	CString str;
	if(!ini.GetValue(key, valuename, str))
		return false;

	value.Command(str);
	return true;
}

void NoteFieldMode::Load(IniFile &ini, CString id)
{
	m_Id = id;

	/* Required: */
	ASSERT( ini.GetValue ( id, "Name",			m_Name ) );

	ini.GetValue ( id, "Backdrop",				m_Backdrop );
	ini.GetValueF( id, "FOV",					m_fFov );
	ini.GetValueF( id, "NearClipDistance",		m_fNear );
	ini.GetValueF( id, "FarClipDistance",		m_fFar );
	ini.GetValueF( id, "PixelsDrawAheadScale",	m_fFirstPixelToDrawScale );
	ini.GetValueF( id, "PixelsDrawBehindScale",	m_fLastPixelToDrawScale );

	GetValueCmd( ini, id, "Center", m_Center );
	GetValueCmd( ini, id, "Position", m_Position );
	GetValueCmd( ini, id, "BackdropPosition", m_PositionBackdrop );

	for(int t = 0; t < MAX_NOTE_TRACKS; ++t)
	{
		GetValueCmd( ini, id, ssprintf("Center%i", t+1), m_CenterTrack[t] );
		GetValueCmd( ini, id, ssprintf("Position%i", t+1), m_PositionTrack[t] );

		ini.GetValue( id, ssprintf("GrayButton", t+1), GrayButtonNames[t] );
		ini.GetValue( id, ssprintf("GrayButton%i", t+1), GrayButtonNames[t] );

		ini.GetValue( id, ssprintf("NoteButton", t+1), NoteButtonNames[t] );
		ini.GetValue( id, ssprintf("NoteButton%i", t+1), NoteButtonNames[t] );

		ini.GetValue( id, ssprintf("GhostButton", t+1), GhostButtonNames[t] );
		ini.GetValue( id, ssprintf("GhostButton%i", t+1), GhostButtonNames[t] );
	}

	CString sGames;
	if(ini.GetValue( id, "Games", sGames ))
	{
		vector<CString> games;
		split(sGames, ",", games);
		for(unsigned n = 0; n < games.size(); ++n)
		{
			vector<CString> bits;
			split(games[n], "-", bits);
			ASSERT(bits.size() == 2);

			const Game game = GAMEMAN->StringToGameType( bits[0] );
			ASSERT(game != GAME_INVALID);

			const Style style = GAMEMAN->GameAndStringToStyle( game, bits[1] );
			ASSERT(style != STYLE_INVALID);
			Styles.insert(style);
		}
	}
}

NoteFieldPositioning::NoteFieldPositioning(CString fn)
{
	m_Filename = fn;
	IniFile ini(fn);
	if(!ini.ReadFile())
		return;

	for(IniFile::const_iterator i = ini.begin(); i != ini.end(); ++i)
	{
		NoteFieldMode m;
		m.Load(ini, i->first);

		Modes.push_back(m);
	}
}

bool NoteFieldMode::MatchesCurrentGame() const
{
	if(Styles.empty())
		return true;

	if(Styles.find(GAMESTATE->m_CurStyle) == Styles.end())
		return false;

	return true;
}

void NoteFieldPositioning::Load(PlayerNumber pn)
{
	NoteFieldMode &mode = g_NoteFieldMode[pn];

	mode = NoteFieldMode(); /* reset */

	const StyleDef *s = GAMESTATE->GetCurrentStyleDef();

	/* Load the settings in the style table by default. */
	for(int tn = 0; tn < MAX_NOTE_TRACKS; ++tn)
	{
		const float fPixelXOffsetFromCenter = s->m_ColumnInfo[pn][tn].fXOffset;
		mode.m_PositionTrack[tn].SetX(fPixelXOffsetFromCenter);
	}

	/* Is there a custom mode with the current name that fits the current game? */
	const int ModeNum = GetID(GAMESTATE->m_PlayerOptions[pn].m_sPositioning);
	if(ModeNum == -1)
		return; /* No, only use the style table settings. */

	/* We have a custom mode.  Reload the mode on top of the default style
	 * table settings. */
	IniFile ini(m_Filename);
	if(!ini.ReadFile())
		return;

	mode.Load(ini, Modes[ModeNum].m_Id);
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
