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


NoteFieldPositioning::Mode::Mode()
{
	m_fFov = 0;
}

void NoteFieldPositioning::Mode::BeginDrawTrack(int tn)
{
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

void NoteFieldPositioning::Mode::EndDrawTrack(int tn)
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
}

NoteFieldPositioning::NoteFieldPositioning(CString fn)
{
	IniFile ini(fn);
	if(!ini.ReadFile())
		return;

	for(IniFile::const_iterator i = ini.begin(); i != ini.end(); ++i)
	{
		Mode m;

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

bool NoteFieldPositioning::Mode::MatchesCurrentGame() const
{
	if(Styles.empty())
		return true;

	if(Styles.find(GAMESTATE->m_CurStyle) == Styles.end())
		return false;

	return true;
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

int NoteFieldPositioning::GetID(PlayerNumber pn) const
{
	return GetID(GAMESTATE->m_PlayerOptions[pn].m_sPositioning);
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

/* XXX: Lots of GetID calls, which do a bunch of string compares; profile this. */
CString NoteFieldPositioning::GetBackdropBGA(PlayerNumber pn) const
{
	const int mode = GetID(pn);
	if(mode == -1)
		return ""; /* none */
	LOG->Trace("bd %s", Modes[mode].Backdrop.c_str() );
	return Modes[mode].Backdrop;
}

void NoteFieldPositioning::BeginDrawBackdrop(PlayerNumber pn)
{
	const int mode = GetID(pn);
	if(mode == -1)
		return;

	DISPLAY->PushMatrix();
	Modes[mode].BeginDrawTrack(-1);
}

void NoteFieldPositioning::EndDrawBackdrop(PlayerNumber pn)
{
	const int mode = GetID(pn);
	if(mode == -1)
		return;

	Modes[mode].EndDrawTrack(-1);
	DISPLAY->PopMatrix();
}

void NoteFieldPositioning::BeginDrawTrack(PlayerNumber pn, int tn)
{
	const int mode = GetID(pn);

	DISPLAY->PushMatrix();

	if(mode == -1)
	{
		/* No transformation is enabled, so use the one defined in the style
		 * table. */
		const StyleDef *s = GAMESTATE->GetCurrentStyleDef();
		const float fPixelXOffsetFromCenter = s->m_ColumnInfo[pn][tn].fXOffset;
		DISPLAY->Translate(fPixelXOffsetFromCenter, 0, 0);
	} else {
		Modes[mode].BeginDrawTrack(tn);
	}
}

void NoteFieldPositioning::EndDrawTrack(PlayerNumber pn, int tn)
{
	const int mode = GetID(pn);

	if(mode != -1)
		Modes[mode].EndDrawTrack(tn);

	DISPLAY->PopMatrix();
}
