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
		DISPLAY->EnterPerspective(m_fFov);
	
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

NoteFieldPositioning::NoteFieldPositioning(CString fn)
{
	IniFile ini(fn);
	if(!ini.ReadFile())
		return;

	for(IniFile::const_iterator i = ini.begin(); i != ini.end(); ++i)
	{
		NoteFieldMode m;

		const IniFile::key &k = i->second;
		IniFile::key::const_iterator val;

		val = k.find("Name");
		ASSERT(val != k.end()); /* required */
		m.name = val->second;

		val = k.find("Backdrop");
		if(val != k.end())
			m.Backdrop = val->second;
		
		val = k.find("Center");
		if(val != k.end())
			m.m_Center.Command(val->second);

		val = k.find("FOV");
		if(val != k.end())
			m.m_fFov = float(atof(val->second.c_str()));

		val = k.find("Position");
		if(val != k.end())
			m.m_Position.Command(val->second);

		val = k.find("BackdropPosition");
		if(val != k.end())
			m.m_PositionBackdrop.Command(val->second);
		
		for(int t = 0; t < MAX_NOTE_TRACKS; ++t)
		{
			val = k.find(ssprintf("Center%i", t+1));
			if(val != k.end())
				m.m_CenterTrack[t].Command(val->second);
			val = k.find(ssprintf("Position%i", t+1));
			if(val != k.end())
				m.m_PositionTrack[t].Command(val->second);

			CString sGames;
			val = k.find("Games");
			if(val != k.end())
			{
				vector<CString> games;
				split(val->second, ",", games);
				for(unsigned n = 0; n < games.size(); ++n)
				{
					vector<CString> bits;
					split(games[n], "-", bits);
					ASSERT(bits.size() == 2);

					const Game game = GAMEMAN->StringToGameType( bits[0] );
					ASSERT(game != GAME_INVALID);

					const Style style = GAMEMAN->GameAndStringToStyle( game, bits[1] );
					ASSERT(style != STYLE_INVALID);
					m.Styles.insert(style );
				}
			}
		}

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
	const int ModeNum = GetID(GAMESTATE->m_PlayerOptions[pn].m_sPositioning);
	if(ModeNum != -1)
	{
		/* We have a custom mode; copy it. */
		mode = Modes[ModeNum];
		return;
	}

	/* No transformation is enabled, so use the one defined in the style
		* table. */
	mode = NoteFieldMode(); /* reset */

	const StyleDef *s = GAMESTATE->GetCurrentStyleDef();

	/* Set the m_PositionTrack[] value for each track. */
	for(int tn = 0; tn < MAX_NOTE_TRACKS; ++tn)
	{
		const float fPixelXOffsetFromCenter = s->m_ColumnInfo[pn][tn].fXOffset;
		mode.m_PositionTrack[tn].SetX(fPixelXOffsetFromCenter);
	}
}


/* Get the unique ID of the given name, for the current game/style.  If it
 * doesn't exist, return "". */
int NoteFieldPositioning::GetID(const CString &name) const
{
	for(unsigned i = 0; i < Modes.size(); ++i)
	{
		if(Modes[i].name.CompareNoCase(name))
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
		
		IDs.push_back(Modes[i].name);
	}
}
