#include "global.h"

#include "ScreenOptionsMaster.h"
#include "RageException.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "ScreenManager.h"
#include "NoteSkinManager.h"
#include "Course.h"
#include "Steps.h"
#include "StyleDef.h"
#include "song.h"
#include "SongManager.h"
#include "Character.h"
#include "ScreenOptionsMasterPrefs.h"

#define NUM_ROWS				THEME->GetMetricI(m_sName,"NumRows")
#define TOGETHER				THEME->GetMetricI(m_sName,"Together")
#define EXPLANATIONS			THEME->GetMetricB(m_sName,"Explanations")
#define ROW_LINE(i)				THEME->GetMetric (m_sName,ssprintf("Line%i",(i+1)))

#define ENTRY(s)				THEME->GetMetric ("ScreenOptionsMasterEntries",s)
#define ENTRY_MODE(s,i)			THEME->GetMetric ("ScreenOptionsMasterEntries",ssprintf("%s,%i",(s).c_str(),(i+1)))
#define ENTRY_NAME(s,i)			THEME->GetMetric ("ScreenOptionsMasterEntries",ssprintf("%sName,%i",(s).c_str(),(i+1)))
#define ENTRY_DEFAULT(s)		THEME->GetMetric ("ScreenOptionsMasterEntries",(s) + "Default")
#define NEXT_SCREEN				THEME->GetMetric (m_sName,"NextScreen")
// #define NEXT_SCREEN( play_mode )		THEME->GetMetric (m_sName,"NextScreen"+Capitalize(PlayModeToString(play_mode)))
#define PREV_SCREEN				THEME->GetMetric (m_sName,"PrevScreen")
// #define PREV_SCREEN( play_mode )		THEME->GetMetric (m_sName,"PrevScreen"+Capitalize(PlayModeToString(play_mode)))

#define BEGINNER_DESCRIPTION	THEME->GetMetric ("ScreenOptionsMasterEntries","Beginner")
#define EASY_DESCRIPTION		THEME->GetMetric ("ScreenOptionsMasterEntries","Easy")
#define MEDIUM_DESCRIPTION		THEME->GetMetric ("ScreenOptionsMasterEntries","Medium")
#define HARD_DESCRIPTION		THEME->GetMetric ("ScreenOptionsMasterEntries","Hard")
#define CHALLENGE_DESCRIPTION	THEME->GetMetric ("ScreenOptionsMasterEntries","Challenge")

CString ScreenOptionsMaster::ConvertParamToThemeDifficulty( const CString &in ) const
{
	switch( StringToDifficulty(in) )
	{
	case DIFFICULTY_BEGINNER:	return BEGINNER_DESCRIPTION;
	case DIFFICULTY_EASY:		return EASY_DESCRIPTION;
	case DIFFICULTY_MEDIUM:		return MEDIUM_DESCRIPTION;
	case DIFFICULTY_HARD:		return HARD_DESCRIPTION;
	case DIFFICULTY_CHALLENGE:	return CHALLENGE_DESCRIPTION;
	default:					return in;  // something else
	}
}

/* Add the list named "ListName" to the given row/handler. */
void ScreenOptionsMaster::SetList( OptionRow &row, OptionRowHandler &hand, CString ListName, CString &TitleOut )
{
	hand.type = ROW_LIST;

	TitleOut = ListName;
	if( !ListName.CompareNoCase("noteskins") )
	{
		hand.Default.Init(); /* none */
		row.bOneChoiceForAllPlayers = false;

		CStringArray arraySkinNames;
		NOTESKIN->GetNoteSkinNames( arraySkinNames );
		for( unsigned skin=0; skin<arraySkinNames.size(); skin++ )
		{
			arraySkinNames[skin].MakeUpper();

			ModeChoice mc;
			mc.m_sModifiers = arraySkinNames[skin];
			hand.ListEntries.push_back( mc );
			row.choices.push_back( arraySkinNames[skin] );
		}
		return;
	}

	hand.Default.Load( -1, ENTRY_DEFAULT(ListName) );

	/* Parse the basic configuration metric. */
	CStringArray asParts;
	split( ENTRY(ListName), ",", asParts );
	if( asParts.size() < 1 )
		RageException::Throw( "Parse error in ScreenOptionsMasterEntries::ListName%s", ListName.c_str() );

	row.bOneChoiceForAllPlayers = false;
	const int NumCols = atoi( asParts[0] );
	if( asParts.size() > 1 )
		row.bOneChoiceForAllPlayers = !asParts[1].CompareNoCase("together");

	for( int col = 0; col < NumCols; ++col )
	{
		ModeChoice mc;
		mc.Load( 0, ENTRY_MODE(ListName, col) );

		if( !mc.IsPlayable() )
			continue;

		hand.ListEntries.push_back( mc );
		row.choices.push_back( ENTRY_NAME(ListName, col) );
	}
}

/* Add a list of difficulties/edits to the given row/handler. */
void ScreenOptionsMaster::SetStep( OptionRow &row, OptionRowHandler &hand )
{
	hand.type = ROW_STEP;
	row.bOneChoiceForAllPlayers = false;

	// fill in difficulty names
	if( GAMESTATE->m_bEditing )
	{
		row.choices.push_back( "" );
	}
	else if( GAMESTATE->m_pCurCourse )   // playing a course
	{
		row.bOneChoiceForAllPlayers = true;
		row.choices.push_back( "REGULAR" );
		if( GAMESTATE->m_pCurCourse->HasDifficult( GAMESTATE->GetCurrentStyleDef()->m_StepsType ) )
			row.choices.push_back( "DIFFICULT" );
	}
	else if( GAMESTATE->m_pCurSong )	// playing a song
	{
		vector<Steps*> vNotes;
		GAMESTATE->m_pCurSong->GetSteps( vNotes, GAMESTATE->GetCurrentStyleDef()->m_StepsType );
		SortNotesArrayByDifficulty( vNotes );
		for( unsigned i=0; i<vNotes.size(); i++ )
		{
			CString s = vNotes[i]->GetDescription();
			s.MakeUpper();

			// convert to theme-defined values
			s = ConvertParamToThemeDifficulty(s);

			row.choices.push_back( s );
		}
	}
	else
	{
		row.choices.push_back( "N/A" );
	}
}


/* Add the given configuration value to the given row/handler. */
void ScreenOptionsMaster::SetConf( OptionRow &row, OptionRowHandler &hand, CString param, CString &TitleOut )
{
	/* Configuration values are never per-player. */
	row.bOneChoiceForAllPlayers = true;
	hand.type = ROW_CONFIG;

	hand.opt = FindConfOption( param );
	if( hand.opt == NULL )
		RageException::Throw( "Invalid Conf type \"%s\"", param.c_str() );

	for( unsigned i = 0; i < hand.opt->names.size(); ++i )
		row.choices.push_back( hand.opt->names[i] );

	TitleOut = hand.opt->name;
}

/* Add a list of available characters to the given row/handler. */
void ScreenOptionsMaster::SetCharacter( OptionRow &row, OptionRowHandler &hand )
{
	row.bOneChoiceForAllPlayers = false;
	row.choices.push_back( "OFF" );
	vector<Character*> apCharacters;
	GAMESTATE->GetCharacters( apCharacters );
	for( unsigned i=0; i<apCharacters.size(); i++ )
	{
		CString s = apCharacters[i]->m_sName;
		s.MakeUpper();
		row.choices.push_back( s ); 
	}
}

ScreenOptionsMaster::ScreenOptionsMaster( CString sClassName ):
	ScreenOptions( sClassName )
{
	m_OptionRowAlloc = new OptionRow[NUM_ROWS];
	for( int i = 0; i < NUM_ROWS; ++i )
	{
		OptionRow &row = m_OptionRowAlloc[i];
		
		CStringArray asParts;
		split( ROW_LINE(i), ";", asParts );
		if( asParts.size() < 1 )
			RageException::Throw( "Parse error in %s::Line%i", m_sName.c_str(), i+1 );

		OptionRowHandler hand;
		for( unsigned part = 0; part < asParts.size(); ++part)
		{
			CStringArray asBits;
			split( asParts[part], ",", asBits );

			const CString name = asBits[0];
			const CString param = asBits.size() > 1? asBits[1]: "";

			CString Title = "";
			if( !name.CompareNoCase("title") )
				row.name = param;

			else if( !name.CompareNoCase("list") )
			{
				SetList( row, hand, param, Title );
			}
			else if( !name.CompareNoCase("steps") )
			{
				SetStep( row, hand );
				Title = "Steps";
			}
			else if( !name.CompareNoCase("conf") )
			{
				SetConf( row, hand, param, Title );
			}
			else if( !name.CompareNoCase("characters") )
			{
				SetCharacter( row, hand );
				Title = "Charac::-ter";
			}
			else
				RageException::Throw( "Unexpected type '%s' in %s::Line%i", name.c_str(), m_sName.c_str(), i );
			if( row.name == "" )
				row.name = Title;
			
		}
		OptionRowHandlers.push_back( hand );
	}

	ASSERT( (int) OptionRowHandlers.size() == NUM_ROWS );

	InputMode im = (InputMode) TOGETHER;
	Init( im, m_OptionRowAlloc, NUM_ROWS, EXPLANATIONS );
}

ScreenOptionsMaster::~ScreenOptionsMaster()
{
	delete m_OptionRowAlloc;
}

int ScreenOptionsMaster::ImportOption( const OptionRow &row, const OptionRowHandler &hand, int pn )
{
	/* Figure out which selection is the default. */
	switch( hand.type )
	{
	case ROW_LIST:
	{
		for( unsigned e = 0; e < hand.ListEntries.size(); ++e )
		{
			const ModeChoice &mc = hand.ListEntries[e];
			if( row.bOneChoiceForAllPlayers )
			{
				if( mc.DescribesCurrentModeForAllPlayers() )
					return e;
			} else {
				if( mc.DescribesCurrentMode( (PlayerNumber) pn) )
					return e;
			}
		}

		return 0;
	}
	case ROW_STEP:
		if( GAMESTATE->m_bEditing )
			return 0;

		if( GAMESTATE->m_pCurCourse )   // playing a course
		{
			if( GAMESTATE->m_bDifficultCourses &&
				GAMESTATE->m_pCurCourse->HasDifficult( GAMESTATE->GetCurrentStyleDef()->m_StepsType ) )
				return 1;
			return 0;
		}

		if( GAMESTATE->m_pCurSong )	// playing a song
		{
			vector<Steps*> vNotes;
			GAMESTATE->m_pCurSong->GetSteps( vNotes, GAMESTATE->GetCurrentStyleDef()->m_StepsType );
			SortNotesArrayByDifficulty( vNotes );
			for( unsigned i=0; i<vNotes.size(); i++ )
			{
				if( GAMESTATE->m_pCurNotes[pn] == vNotes[i] )
					return i;
			}
		}

		return 0;
	case ROW_CHARACTER:
	{
		vector<Character*> apCharacters;
		GAMESTATE->GetCharacters( apCharacters );
		for( unsigned i=0; i<apCharacters.size(); i++ )
			if( GAMESTATE->m_pCurCharacters[pn] == apCharacters[i] )
				return i+1;
		return 0;
	}

	case ROW_CONFIG:
		return hand.opt->Get();

	default:
		ASSERT(0);
		return 0;
	}
}

void ScreenOptionsMaster::ImportOptions()
{
	for( unsigned i = 0; i < OptionRowHandlers.size(); ++i )
	{
		const OptionRowHandler &hand = OptionRowHandlers[i];
		const OptionRow &row = m_OptionRowAlloc[i];

		if( row.bOneChoiceForAllPlayers )
		{
			int col = ImportOption( row, hand, 0 );
			m_iSelectedOption[0][i] = col;
		}
		else
			for( int pn=0; pn<NUM_PLAYERS; pn++ )
			{
				if( !GAMESTATE->IsHumanPlayer(pn) )
					continue;

				int col = ImportOption( row, hand, pn );
				m_iSelectedOption[pn][i] = col;
			}
	}
}

void ScreenOptionsMaster::ExportOption( const OptionRow &row, const OptionRowHandler &hand, int pn, int sel )
{
	/* Figure out which selection is the default. */
	switch( hand.type )
	{
	case ROW_LIST:
	{
		const ModeChoice &mc = hand.ListEntries[sel];

		hand.Default.Apply( (PlayerNumber)pn );
		mc.Apply( (PlayerNumber)pn );
		if( mc.m_sScreen != "" )
			m_NextScreen = mc.m_sScreen;
	}
		break;

	case ROW_CONFIG:
		hand.opt->Put( sel );
		break;

	case ROW_CHARACTER:
		if( sel == 0 )
			GAMESTATE->m_pCurCharacters[pn] = NULL;
		else
		{
			vector<Character*> apCharacters;
			GAMESTATE->GetCharacters( apCharacters );
			GAMESTATE->m_pCurCharacters[pn] = apCharacters[sel - 1];
		}
		break;

	case ROW_STEP:
		if( GAMESTATE->m_bEditing )
		{
			// do nothing
		}
		else if( GAMESTATE->m_pCurCourse )   // playing a course
		{
			if( sel == 1 )
			{
				GAMESTATE->m_bDifficultCourses = true;
				LOG->Trace("ScreenPlayerOptions: Using difficult course");
			}
			else
			{
				GAMESTATE->m_bDifficultCourses = false;
				LOG->Trace("ScreenPlayerOptions: Using normal course");
			}
		}
		else if( GAMESTATE->m_pCurSong )   // playing a song
		{
			vector<Steps*> vNotes;
			GAMESTATE->m_pCurSong->GetSteps( vNotes, GAMESTATE->GetCurrentStyleDef()->m_StepsType );
			SortNotesArrayByDifficulty( vNotes );
			GAMESTATE->m_pCurNotes[pn] = vNotes[ sel ];
		}

		break;
	default:
		ASSERT(0);
		break;
	}
}

void ScreenOptionsMaster::ExportOptions()
{
	m_NextScreen = "";

	for( unsigned i = 0; i < OptionRowHandlers.size(); ++i )
	{
		const OptionRowHandler &hand = OptionRowHandlers[i];
		const OptionRow &row = m_OptionRowAlloc[i];

		if( row.bOneChoiceForAllPlayers )
		{
			ExportOption( row, hand, 0, m_iSelectedOption[0][i] );
		}
		else
			for( int pn=0; pn<NUM_PLAYERS; pn++ )
			{
				if( !GAMESTATE->IsHumanPlayer(pn) )
					continue;

				ExportOption( row, hand, pn, m_iSelectedOption[pn][i] );
			}
	}

	// NEXT_SCREEN(GAMESTATE->m_PlayMode) );
	// XXX: handle different destinations based on play mode?
	if( m_NextScreen == "" )
		m_NextScreen = NEXT_SCREEN;
}

void ScreenOptionsMaster::GoToNextState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else
		SCREENMAN->SetNewScreen( m_NextScreen );
}

void ScreenOptionsMaster::GoToPrevState()
{
	/* XXX: A better way to handle this would be to check if we're a pushed screen. */
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	// XXX: handle different destinations based on play mode?
	else
		SCREENMAN->SetNewScreen( PREV_SCREEN ); // (GAMESTATE->m_PlayMode) );
}

