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


/* This is similar in style to Actor::Command.  However, Actors don't store
 * matrix stacks; they only store offsets and scales, and compound them into
 * a single transformations at once.  This makes some things easy, but it's not
 * convenient for generic 3d transforms.  For that, we have this, which has the
 * small subset of the actor commands that applies to raw matrices, and we apply
 * commands in the order given.  "scale,2;x,1;" is very different from
 * "x,1;scale,2;". */
static CString GetParam( const CStringArray& sParams, int iIndex, int& iMaxIndexAccessed )
{
	iMaxIndexAccessed = max( iIndex, iMaxIndexAccessed );
	if( iIndex < int(sParams.size()) )
		return sParams[iIndex];
	else
		return "";
}

void MatrixCommand(CString sCommandString, RageMatrix &mat)
{
	CStringArray asCommands;
	split( sCommandString, ";", asCommands, true );
	
	for( unsigned c=0; c<asCommands.size(); c++ )
	{
		CStringArray asTokens;
		split( asCommands[c], ",", asTokens, true );

		int iMaxIndexAccessed = 0;

#define sParam(i) (GetParam(asTokens,i,iMaxIndexAccessed))
#define fParam(i) ((float)atof(sParam(i)))
#define iParam(i) (atoi(sParam(i)))
#define bParam(i) (iParam(i)!=0)

		CString& sName = asTokens[0];
		sName.MakeLower();

		RageMatrix b;
		// Act on command
		if( sName=="x" )					RageMatrixTranslation( &b, fParam(1),0,0 );
		else if( sName=="y" )				RageMatrixTranslation( &b, 0,fParam(1),0 );
		else if( sName=="z" )				RageMatrixTranslation( &b, 0,0,fParam(1) );
		else if( sName=="zoomx" )			RageMatrixScaling(&b, fParam(1),1,1 );
		else if( sName=="zoomy" )			RageMatrixScaling(&b, 1,fParam(1),1 );
		else if( sName=="zoomz" )			RageMatrixScaling(&b, 1,1,fParam(1) );
		else if( sName=="rotationx" )		RageMatrixRotationX( &b, fParam(1) );
		else if( sName=="rotationy" )		RageMatrixRotationY( &b, fParam(1) );
		else if( sName=="rotationz" )		RageMatrixRotationZ( &b, fParam(1) );
		else
		{
			CString sError = ssprintf( "Unrecognized matrix command name '%s' in command string '%s'.", sName.GetString(), sCommandString.GetString() );
			LOG->Warn( sError );
#if defined(WIN32) // XXX arch?
			if( DISPLAY->IsWindowed() )
				MessageBox(NULL, sError, "MatrixCommand", MB_OK);
#endif
			continue;
		}


		if( iMaxIndexAccessed != (int)asTokens.size()-1 )
		{
			CString sError = ssprintf( "Wrong number of parameters in command '%s'.  Expected %d but there are %d.", join(",",asTokens).GetString(), iMaxIndexAccessed+1, (int)asTokens.size() );
			LOG->Warn( sError );
#if defined(WIN32) // XXX arch?
			if( DISPLAY->IsWindowed() )
				MessageBox(NULL, sError, "MatrixCommand", MB_OK);
#endif
			continue;
		}

		RageMatrix a(mat);
		RageMatrixMultiply(&mat, &a, &b);
	}
	
}

NoteFieldPositioning::Mode::Mode()
{
	for( int tn=0; tn<MAX_NOTE_TRACKS; tn++ )
	{
		RageMatrixIdentity(&m_Position[tn]);
		RageMatrixIdentity(&m_PerspPosition[tn]);
		m_fFov[tn] = 0;
	}
}

void NoteFieldPositioning::Mode::BeginDrawTrack(int tn) const
{
	DISPLAY->MultMatrix((const float *) m_Position[tn]);

	if(m_fFov[tn])
		DISPLAY->EnterPerspective(m_fFov[tn]);
	
	DISPLAY->MultMatrix((const float *) m_PerspPosition[tn]);
}

void NoteFieldPositioning::Mode::EndDrawTrack(int tn) const
{
	if(m_fFov[tn])
		DISPLAY->ExitPerspective();
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
		for(int t = 0; t < MAX_NOTE_TRACKS; ++t)
		{
			IniFile::key::const_iterator val;

			val = k.find("Name");
			ASSERT(val != k.end()); /* required */
			m.name = val->second;

			val = k.find(ssprintf("Track%i", t+1));
			if(val != k.end())
				MatrixCommand(val->second, m.m_Position[t]);
			val = k.find(ssprintf("PTrack%i", t+1));
			if(val != k.end())
				MatrixCommand(val->second, m.m_PerspPosition[t]);

			val = k.find(ssprintf("FOV%i", t+1));
			if(val != k.end())
				m.m_fFov[t] = float(atof(val->second.c_str()));

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
		LOG->Trace("look for %s", name.c_str());
	for(unsigned i = 0; i < Modes.size(); ++i)
	{
		if(Modes[i].name.CompareNoCase(name))
			continue;

		if(!Modes[i].MatchesCurrentGame())
			continue;

		LOG->Trace("found it");
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


void NoteFieldPositioning::BeginDrawTrack(PlayerNumber pn, int tn) const
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

void NoteFieldPositioning::EndDrawTrack(PlayerNumber pn, int tn) const
{
	const int mode = GetID(pn);

	if(mode != -1)
		Modes[mode].EndDrawTrack(tn);

	DISPLAY->PopMatrix();
}
