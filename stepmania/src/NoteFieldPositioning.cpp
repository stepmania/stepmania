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
	if(tn != -1)
		m_PositionTrack[tn].BeginDraw();
}

void NoteFieldMode::EndDrawTrack(int tn)
{
	if(tn != -1)
		m_PositionTrack[tn].EndDraw();
	m_Position.EndDraw();

	if(m_fFov)
		DISPLAY->ExitPerspective();

	if(tn != -1)
		m_CenterTrack[tn].EndDraw();
	m_Center.EndDraw();

	DISPLAY->PopMatrix();
}

template <class T>
static bool GetValue(IniFile &ini, int pn,
				   const CString &key, const CString &valuename, T &value )
{
	if(pn != -1 && ini.GetValue(key, ssprintf("P%i%s", pn+1, valuename.c_str()), value))
		return true;
	return ini.GetValue(key, valuename, value);
}

static bool GetValue(IniFile &ini, int pn,
				   const CString &key, const CString &valuename, Actor &value )
{
	CString str;
	if(!GetValue(ini, pn, key, valuename, str))
		return false;

	value.Command(str);
	return true;
}

void NoteFieldMode::Load(IniFile &ini, CString id, int pn)
{
	m_Id = id;

	/* Required: */
	ASSERT( ini.GetValue ( id, "Name",			m_Name ) );

	GetValue( ini, pn, id, "Backdrop",				m_Backdrop );
	GetValue( ini, pn, id, "FOV",					m_fFov );
	GetValue( ini, pn, id, "NearClipDistance",		m_fNear );
	GetValue( ini, pn, id, "FarClipDistance",		m_fFar );
	GetValue( ini, pn, id, "PixelsDrawAheadScale",	m_fFirstPixelToDrawScale );
	GetValue( ini, pn, id, "PixelsDrawBehindScale",	m_fLastPixelToDrawScale );

	GetValue( ini, pn, id, "Center",				m_Center );
	GetValue( ini, pn, id, "Position",				m_Position );
	GetValue( ini, pn, id, "Judgment",				m_JudgmentCmd );
	GetValue( ini, pn, id, "Combo",					m_ComboCmd );

	/* Load per-track data: */
	int t;
	for(t = 0; t < MAX_NOTE_TRACKS; ++t)
	{
		GetValue( ini, pn, id, ssprintf("Center%i", t+1), m_CenterTrack[t] );
		GetValue( ini, pn, id, ssprintf("Position%i", t+1), m_PositionTrack[t] );

		GetValue( ini, pn, id, ssprintf("GrayButton", t+1), GrayButtonNames[t] );
		GetValue( ini, pn, id, ssprintf("GrayButton%i", t+1), GrayButtonNames[t] );

		GetValue( ini, pn, id, ssprintf("NoteButton", t+1), NoteButtonNames[t] );
		GetValue( ini, pn, id, ssprintf("NoteButton%i", t+1), NoteButtonNames[t] );

		GetValue( ini, pn, id, ssprintf("GhostButton", t+1), GhostButtonNames[t] );
		GetValue( ini, pn, id, ssprintf("GhostButton%i", t+1), GhostButtonNames[t] );

		GetValue( ini, pn, id, ssprintf("HoldJudgment", t+1),	m_HoldJudgmentCmd[t] );
		GetValue( ini, pn, id, ssprintf("HoldJudgment%i", t+1), m_HoldJudgmentCmd[t] );
	}

	/* Grab this directly; don't try to set game specs per-player. */
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

	mode.Load(ini, Modes[ModeNum].m_Id, pn);
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
