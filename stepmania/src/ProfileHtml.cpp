#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ProfileHtml

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ProfileHtml.h"
#include "ThemeManager.h"
#include "RageFile.h"
#include "RageLog.h"
#include "SongManager.h"
#include "song.h"
#include "Steps.h"
#include <time.h>
#include "GameManager.h"
#include "Course.h"
#include "Bookkeeper.h"


#define STATS_HTML			"stats.html"
#define STYLE_CSS			"style.css"

#define STATS_TITLE			THEME->GetMetric("ProfileManager","StatsTitle")
#define STATS_FOOTER		THEME->GetMetric("ProfileManager","StatsFooter")


static int g_Level = 1;

inline CString MakeUniqueId()												{ CString s="id"+ssprintf("%d%d%d",rand(),rand(),rand()); return s; }
inline void PRINT_OPEN(RageFile &f,CString sName,bool bExpanded,CString sID){ g_Level++; ASSERT(g_Level>0 && g_Level<6); f.Write( ssprintf("<div class='section%d'>\n" "<h%d onClick='expandIt(%s); return false' CLASS='outline'>%s</h%d>\n" "<DIV ID='%s' CLASS='%s'>\n", g_Level, g_Level, sID.c_str(), sName.c_str(), g_Level, sID.c_str(), bExpanded?"visibletext":"hiddentext") ); }
inline void PRINT_OPEN(RageFile &f,CString sName,bool bExpanded=false)		{ PRINT_OPEN(f,sName,bExpanded,MakeUniqueId()); }
inline void PRINT_CLOSE(RageFile &f)										{ f.Write( "</div>\n" "</div>\n" ); g_Level--; ASSERT(g_Level>=0); }
//inline void PRINT_LINK(RageFile &f,CString sName,CString sLink)				{ f.Write( ssprintf("<p><a href='%s'>%s</a></p>\n",sLink.c_str(),sName.c_str()) ); }
//inline void PRINT_LINE_S(RageFile &f,CString sName,CString sVal) 			{ f.Write( ssprintf("<p>%s = <b>%s</b></p>\n",sName.c_str(),sVal.c_str()) ); }
//inline void PRINT_LINE_B(RageFile &f,CString sName,bool bVal) 				{ f.Write( ssprintf("<p>%s = <b>%s</b></p>\n",sName.c_str(),(bVal)?"yes":"no") ); }
//inline void PRINT_LINE_I(RageFile &f,CString sName,int iVal) 				{ f.Write( ssprintf("<p>%s = <b>%d</b></p>\n",sName.c_str(),iVal) ); }
//inline void PRINT_LINE_RANK(RageFile &f,int iRank,CString sName,int iVal)	{ f.Write( ssprintf("<p><b>%d</b> - %s (%d)</p>\n",iRank,sName.c_str(),iVal) ); }
//inline void PRINT_LINE_RANK_LINK(RageFile &f,int iRank,CString sName,CString sLink,int iVal)	{ f.Write( ssprintf("<p><b>%d</b> - <a href='%s'>%s</a> (%d)</p>\n",iRank,sLink.c_str(),sName.c_str(),iVal) ); }


struct Table
{
	Table() {}

	struct Line
	{
		Line() {}
		Line(CString n,CString v)	{ sName = n; sValue = v; }
		Line(CString n,bool v)		{ sName = n; sValue = ssprintf("%s",v?"yes":"no"); }
		Line(CString n,int v)		{ sName = n; sValue = ssprintf("%d",v); }
		Line(int r,CString n,int v)	{ sRank = ssprintf("%d",r); sName = n; sValue = ssprintf("%d",v); }

		CString sRank;
		CString sName;
		CString sValue;
	};

	int iNumCols;
	vector<Line> vLines;
};

//inline CString MAKE_LINE_S(CString sName,CString sVal) 			{ return ssprintf("<p>%s = <b>%s</b></p>\n",sName.c_str(),sVal.c_str()); }
//inline CString MAKE_LINE_B(CString sName,bool bVal) 			{ return ssprintf("<p>%s = <b>%s</b></p>\n",sName.c_str(),(bVal)?"yes":"no"); }
//inline CString MAKE_LINE_I(CString sName,int iVal) 				{ return ssprintf("<p>%s = <b>%d</b></p>\n",sName.c_str(),iVal); }
//inline CString MAKE_LINE_RANK(int iRank,CString sName,int iVal)	{ return ssprintf("<p><b>%d</b> - %s (%d)</p>\n",iRank,sName.c_str(),iVal); }

#define BEGIN_TABLE(cols)		{ Table table; table.iNumCols=cols;
#define TABLE_LINE2(p1,p2)		table.vLines.push_back( Table::Line(p1,p2) );
#define TABLE_LINE3(p1,p2,p3)	table.vLines.push_back( Table::Line(p1,p2,p3) );
#define END_TABLE				PrintTable( f, table ); }

inline void PrintTable(RageFile &f,Table &table)
{
	const vector<Table::Line> &vLines = table.vLines;
	int &iNumCols = table.iNumCols;

	ASSERT( iNumCols > 0 );

	if( vLines.empty() )
		return;

	bool bPrintRank = !vLines.empty() && !vLines[0].sRank.empty();

	int iMaxItemsPerCol = (vLines.size()+iNumCols-1) / iNumCols;
	iNumCols = (vLines.size()+iMaxItemsPerCol-1) / iMaxItemsPerCol;	// round up
	f.Write("<table class='group'><tr>\n");
	for( int col=0; col<iNumCols; col++ )
	{
		f.Write("<td>\n");

		int iStartItem = col*iMaxItemsPerCol;

		f.Write("<table class='column'>\n");
		for( int i=iStartItem; i<iStartItem+iMaxItemsPerCol; i++ )
		{
			f.Write("<tr>");

			const Table::Line& line = (i<(int)vLines.size()) ? vLines[i] : Table::Line();
			if( bPrintRank )
			{
				f.Write("<td class='rank'>");
				f.Write( line.sRank );
				f.Write("</td>");
				f.Write("<td>&nbsp;</td>");
			}
			f.Write("<td class='name'>");
			f.Write( line.sName );
			f.Write("</td>");
			f.Write("<td>&nbsp;</td>");
			f.Write("<td class='value'>");
			f.Write( line.sValue );
			f.Write("</td>");
			f.Write( "\n" );

			f.Write("</tr>");
		}
		f.Write("</table>\n");

		f.Write("</td>\n");
	}
	f.Write("</tr></table>\n");
} 

void PrintStatistics( RageFile &f, const Profile *pProfile, CString sTitle, vector<Song*> &vpSongs, vector<Steps*> &vpAllSteps, vector<StepsType> &vStepsTypesToShow, map<Steps*,Song*> mapStepsToSong, vector<Course*> vpCourses )
{
	PRINT_OPEN(f,sTitle);
	{
		PRINT_OPEN(f,"This Profile",true);
		{
			BEGIN_TABLE(2);
			TABLE_LINE2( "Name",							pProfile->m_sName );
			TABLE_LINE2( "LastUsedHighScoreName",			pProfile->m_sLastUsedHighScoreName );
			TABLE_LINE2( "UsingProfileDefaultModifiers",	pProfile->m_bUsingProfileDefaultModifiers );
			TABLE_LINE2( "DefaultModifiers",				pProfile->m_sDefaultModifiers );
			TABLE_LINE2( "TotalPlays",						pProfile->m_iTotalPlays );
			TABLE_LINE2( "TotalPlaySeconds",				pProfile->m_iTotalPlaySeconds );
			TABLE_LINE2( "TotalGameplaySeconds",			pProfile->m_iTotalGameplaySeconds );
			TABLE_LINE2( "CurrentCombo",					pProfile->m_iCurrentCombo );
			TABLE_LINE2( "CaloriesBurned",					pProfile->m_fCaloriesBurned );
			TABLE_LINE2( "LastMachinePlayed",				pProfile->m_sLastMachinePlayed );
			END_TABLE;
		}
		PRINT_CLOSE(f);
		

		PRINT_OPEN(f,"Num Songs Played by PlayMode",true);
		{
			BEGIN_TABLE(4);
			FOREACH_PlayMode( pm )
				TABLE_LINE2( PlayModeToString(pm), pProfile->m_iNumSongsPlayedByPlayMode[pm] );
			END_TABLE;
		}
		PRINT_CLOSE(f);


		PRINT_OPEN(f,"Num Songs Played by Style",true);
		{
			BEGIN_TABLE(4);
			for( int i=0; i<NUM_STYLES; i++ )
			{
				Style style = (Style)i;
				const StyleDef* pStyleDef = GAMEMAN->GetStyleDefForStyle(style);
				StepsType st = pStyleDef->m_StepsType;
				if( !pStyleDef->m_bUsedForGameplay )
					continue;	// skip
				// only show if this style plays a StepsType that we're showing
				if( find(vStepsTypesToShow.begin(),vStepsTypesToShow.end(),st) == vStepsTypesToShow.end() )
					continue;	// skip
				TABLE_LINE2( pStyleDef->m_szName, pProfile->m_iNumSongsPlayedByStyle[i] );
			}
			END_TABLE;
		}
		PRINT_CLOSE(f);


		PRINT_OPEN(f,"Num Songs Played by Difficulty",true);
		{
			BEGIN_TABLE(4);
			FOREACH_Difficulty( dc )
				TABLE_LINE2( DifficultyToString(dc), pProfile->m_iNumSongsPlayedByDifficulty[dc] );
			END_TABLE;
		}
		PRINT_CLOSE(f);

		PRINT_OPEN(f,"Num Songs Played by Meter",true);
		{
			BEGIN_TABLE(4);
			for( int i=MIN_METER; i<=MAX_METER; i++ )
				TABLE_LINE2( ssprintf("Meter %d",i), pProfile->m_iNumSongsPlayedByMeter[i] );
			END_TABLE;
		}
		PRINT_CLOSE(f);
	}
	PRINT_CLOSE(f);
}

void PrintPopularity( RageFile &f, const Profile *pProfile, CString sTitle, vector<Song*> &vpSongs, vector<Steps*> &vpAllSteps, vector<StepsType> &vStepsTypesToShow, map<Steps*,Song*> mapStepsToSong, vector<Course*> vpCourses )
{
	PRINT_OPEN(f, sTitle );
	{
		SortSongPointerArrayByNumPlays( vpSongs, pProfile, true );
		Song* pSongPopularThreshold = vpSongs[ vpSongs.size()*2/3 ];
		int iPopularNumPlaysThreshold = pProfile->GetSongNumTimesPlayed(pSongPopularThreshold);
		
		// unplayed songs are always considered unpopular
		if( iPopularNumPlaysThreshold == 0 )
			iPopularNumPlaysThreshold = 1;

		unsigned uMaxToShow = min( vpSongs.size(), (unsigned)100 );

		{
			PRINT_OPEN(f, "Most Popular Songs" );
			{
				BEGIN_TABLE(1);
				for( unsigned i=0; i<uMaxToShow; i++ )
				{
					Song* pSong = vpSongs[i];
					int iNumTimesPlayed = pProfile->GetSongNumTimesPlayed(pSong);
					if( iNumTimesPlayed == 0 || iNumTimesPlayed < iPopularNumPlaysThreshold )	// not popular
						break;	// done searching
					TABLE_LINE3(i+1, pSong->GetFullDisplayTitle(), iNumTimesPlayed );
				}
				END_TABLE;
			}
			PRINT_CLOSE(f);
		}

		{
			SortSongPointerArrayByNumPlays( vpSongs, pProfile, false );
			PRINT_OPEN(f, "Least Popular Songs" );
			{
				BEGIN_TABLE(1);
				for( unsigned i=0; i<uMaxToShow; i++ )
				{
					Song* pSong = vpSongs[i];
					int iNumTimesPlayed = pProfile->GetSongNumTimesPlayed(pSong);
					if( iNumTimesPlayed >= iPopularNumPlaysThreshold )	// not unpopular
						break;	// done searching
					TABLE_LINE3(i+1, pSong->GetFullDisplayTitle(), iNumTimesPlayed );
				}
				END_TABLE;
			}
			PRINT_CLOSE(f);
		}

		{
			unsigned uNumToShow = min( vpAllSteps.size(), (unsigned)100 );

			SortStepsPointerArrayByNumPlays( vpAllSteps, pProfile, true );
			PRINT_OPEN(f, "Most Popular Steps" );
			{
				BEGIN_TABLE(1);
				for( unsigned i=0; i<uNumToShow; i++ )
				{
					Steps* pSteps = vpAllSteps[i];
					if( pProfile->GetStepsNumTimesPlayed(pSteps)==0 )
						continue;	// skip
					Song* pSong = mapStepsToSong[pSteps];
					CString s;
					s += pSong->GetFullDisplayTitle();
					s += " - ";
					s += GAMEMAN->NotesTypeToString(pSteps->m_StepsType);
					s += " ";
					s += DifficultyToString(pSteps->GetDifficulty());
					TABLE_LINE3(i+1, s, pProfile->GetStepsNumTimesPlayed(pSteps) );
				}
				END_TABLE;
			}
			PRINT_CLOSE(f);
		}

		{
			unsigned uNumToShow = min( vpCourses.size(), (unsigned)100 );

			SortCoursePointerArrayByNumPlays( vpCourses, pProfile, true );
			PRINT_OPEN(f, "Most Popular Courses" );
			{
				BEGIN_TABLE(2);
				for( unsigned i=0; i<uNumToShow; i++ )
				{
					Course* pCourse = vpCourses[i];
					TABLE_LINE3(i+1, pCourse->m_sName, pProfile->GetCourseNumTimesPlayed(pCourse) );
				}
				END_TABLE;
			}
			PRINT_CLOSE(f);
		}
	}
	PRINT_CLOSE(f);
}


typedef void (*FnPrintSong)(RageFile &f, const Profile *pProfile, Song* pSong );
typedef void (*FnPrintGroup)(RageFile &f, const Profile *pProfile, CString sGroup );
typedef void (*FnPrintStepsType)(RageFile &f, const Profile *pProfile, StepsType st );


void PrintSongsInGroup( RageFile &f, const Profile *pProfile, CString sGroup, FnPrintSong pFn )
{
	PRINT_OPEN(f, sGroup );
	{
		vector<Song*> vpSongs;
		SONGMAN->GetSongs( vpSongs, sGroup );

		for( unsigned i=0; i<vpSongs.size(); i++ )
		{
			Song* pSong = vpSongs[i];
			
			pFn( f, pProfile, pSong );
		}
	}
	PRINT_CLOSE(f);
}

void PrintGroups( RageFile &f, const Profile *pProfile, CString sTitle, FnPrintGroup pFn )
{
	CStringArray asGroups;
	SONGMAN->GetGroupNames( asGroups );
	
	PRINT_OPEN(f, sTitle );
	{
		for( unsigned g=0; g<asGroups.size(); g++ )
		{
			CString sGroup = asGroups[g];
			
			pFn( f, pProfile, sGroup );
		}
	}
	PRINT_CLOSE(f);
}

void PrintStepsTypes( RageFile &f, const Profile *pProfile, CString sTitle, vector<StepsType> vStepsTypesToShow, FnPrintStepsType pFn )
{
	PRINT_OPEN(f, sTitle );
	{
		for( unsigned s=0; s<vStepsTypesToShow.size(); s++ )
		{
			StepsType st = vStepsTypesToShow[s];

			pFn( f, pProfile, st );
		}
	}
	PRINT_CLOSE(f);
}

void PrintHighScoresForSong( RageFile &f, const Profile *pProfile, Song* pSong )
{
	int iNumTimesPlayed = pProfile->GetSongNumTimesPlayed(pSong);
	if( iNumTimesPlayed == 0 )
		return;	// skip

	vector<Steps*> vpSteps = pSong->GetAllSteps();

	PRINT_OPEN(f, pSong->GetFullDisplayTitle() );
	{
		BEGIN_TABLE(2);
		TABLE_LINE2( "NumTimesPlayed", iNumTimesPlayed );
		END_TABLE;

		//
		// Print Steps list
		//
		for( unsigned j=0; j<vpSteps.size(); j++ )
		{
			Steps* pSteps = vpSteps[j];
			if( pSteps->IsAutogen() )
				continue;	// skip autogen
			if( pProfile->GetStepsNumTimesPlayed(pSteps)==0 )
				continue;	// skip

			const HighScoreList &hsl = pProfile->GetStepsHighScoreList( pSteps );
			CString s = 
				GAMEMAN->NotesTypeToString(pSteps->m_StepsType) + 
				" - " +
				DifficultyToString(pSteps->GetDifficulty());
			PRINT_OPEN(f, s, true);
			{
				BEGIN_TABLE(2);
				TABLE_LINE2( "NumTimesPlayed", hsl.iNumTimesPlayed );
				END_TABLE;
			
				BEGIN_TABLE(2);
				for( unsigned i=0; i<hsl.vHighScores.size(); i++ )
				{
					const HighScore &hs = hsl.vHighScores[i];
					CString sName = ssprintf("#%d",i+1);
					CString sHSName = hs.sName.empty() ? "????" : hs.sName;
					CString sValue = ssprintf("%s, %s, %i, %.2f%%", sHSName.c_str(), GradeToString(hs.grade).c_str(), hs.iScore, hs.fPercentDP*100);
					TABLE_LINE2( sName.c_str(), sValue );
				}
				END_TABLE;
			}
			PRINT_CLOSE(f);
		}
	}
	PRINT_CLOSE(f);
}

void PrintHighScoresForGroup(RageFile &f, const Profile *pProfile, CString sGroup )
{
	PrintSongsInGroup( f, pProfile, sGroup, PrintHighScoresForSong );
}

void PrintHighScores( RageFile &f, const Profile *pProfile, CString sTitle, vector<Song*> &vpSongs, vector<Steps*> &vpAllSteps, vector<StepsType> &vStepsTypesToShow, map<Steps*,Song*> mapStepsToSong, vector<Course*> vpCourses )
{
	PrintGroups( f, pProfile, sTitle, PrintHighScoresForGroup );
}

void PrintDifficultyTableForStepsType( RageFile &f, const Profile *pProfile, StepsType st )
{
	unsigned i;
	const vector<Song*> &vpSongs = SONGMAN->GetAllSongs();

 	PRINT_OPEN(f, GAMEMAN->NotesTypeToString(st).c_str() );
	{
		f.PutLine( "<table class='difficulty'>\n" );

		// table header row
		f.Write( "<tr><td>&nbsp;</td>" );
		FOREACH_Difficulty( dc )
		{
			if( dc == DIFFICULTY_EDIT )
				continue;	// skip
			f.PutLine( ssprintf("<td>%s</td>", Capitalize(DifficultyToString(dc).Left(3)).c_str()) );
		}
		f.PutLine( "</tr>" );

		// table body rows
		for( i=0; i<vpSongs.size(); i++ )
		{
			Song* pSong = vpSongs[i];

			f.PutLine( "<tr>" );
			
			f.Write( "<td>" );
			f.Write( ssprintf("<p class='songtitle'>%s</p>", pSong->GetDisplayMainTitle().c_str()) );
			f.Write( ssprintf("<p class='songsubtitle'>%s</p>", pSong->GetDisplaySubTitle().c_str()) );
			f.Write( "</td>" );

			FOREACH_Difficulty( dc )
			{
				if( dc == DIFFICULTY_EDIT )
					continue;	// skip

				Steps* pSteps = pSong->GetStepsByDifficulty( st, dc, false );
				if( pSteps )
					f.PutLine( ssprintf("<td><p class='meter'>%d</p></td>",pSteps->GetMeter()) );
				else
					f.PutLine( "<td>&nbsp;</td>" );
			}

			f.Write( "</tr>" );
		}

		f.PutLine( "</table>\n" );
	}
	PRINT_CLOSE(f);
}

void PrintDifficultyTable( RageFile &f, const Profile *pProfile, CString sTitle, vector<Song*> &vpSongs, vector<Steps*> &vpAllSteps, vector<StepsType> &vStepsTypesToShow, map<Steps*,Song*> mapStepsToSong, vector<Course*> vpCourses )
{
	PrintStepsTypes( f, pProfile, sTitle, vStepsTypesToShow, PrintDifficultyTableForStepsType );
}

void PrintInventoryForSong( RageFile &f, const Profile *pProfile, Song* pSong )
{
	vector<Steps*> vpSteps = pSong->GetAllSteps();

	PRINT_OPEN(f, pSong->GetFullDisplayTitle().c_str() );
	{
		BEGIN_TABLE(2);
		TABLE_LINE2( "Artist", pSong->GetDisplayArtist() );
		TABLE_LINE2( "GroupName", pSong->m_sGroupName );
		float fMinBPM, fMaxBPM;
		pSong->GetDisplayBPM( fMinBPM, fMaxBPM );
		CString sBPM = (fMinBPM==fMaxBPM) ? ssprintf("%.1f",fMinBPM) : ssprintf("%.1f - %.1f",fMinBPM,fMaxBPM);
		TABLE_LINE2( "BPM", sBPM );
		TABLE_LINE2( "Credit", pSong->m_sCredit );
		TABLE_LINE2( "MusicLength", SecondsToTime(pSong->m_fMusicLengthSeconds) );
		TABLE_LINE2( "Lyrics", !pSong->m_sLyricsFile.empty() );
		END_TABLE;

		//
		// Print Steps list
		//
		for( unsigned j=0; j<vpSteps.size(); j++ )
		{
			Steps* pSteps = vpSteps[j];
			if( pSteps->IsAutogen() )
				continue;	// skip autogen
			CString s = 
				GAMEMAN->NotesTypeToString(pSteps->m_StepsType) + 
				" - " +
				DifficultyToString(pSteps->GetDifficulty());
			PRINT_OPEN(f, s, true);	// use poister value as the hash
			{
				BEGIN_TABLE(2);
				TABLE_LINE2( "Description", pSteps->GetDescription() );
				TABLE_LINE2( "Meter", pSteps->GetMeter() );
				END_TABLE;

				BEGIN_TABLE(2);
				FOREACH_RadarCategory( cat )
				{
					CString sCat = RadarCategoryToString(cat);
					float fVal = pSteps->GetRadarValues()[cat];
					CString sVal = ssprintf( "%.2f", fVal );
					TABLE_LINE2( sCat, sVal );
				}
				END_TABLE;
			}
			PRINT_CLOSE(f);
		}
	}
	PRINT_CLOSE(f);
}

void PrintInventoryForGroup( RageFile &f, const Profile *pProfile, CString sGroup )
{
	PrintSongsInGroup( f, pProfile, sGroup, PrintInventoryForSong );
}

void PrintInventoryList( RageFile &f, const Profile *pProfile, CString sTitle, vector<Song*> &vpSongs, vector<Steps*> &vpAllSteps, vector<StepsType> &vStepsTypesToShow, map<Steps*,Song*> mapStepsToSong, vector<Course*> vpCourses )
{
	PrintGroups( f, pProfile, sTitle, PrintInventoryForGroup );
}

void PrintBookkeeping( RageFile &f, const Profile *pProfile, CString sTitle, vector<Song*> &vpSongs, vector<Steps*> &vpAllSteps, vector<StepsType> &vStepsTypesToShow, map<Steps*,Song*> mapStepsToSong, vector<Course*> vpCourses)
{
	PRINT_OPEN(f, sTitle );
	{
		// GetCoinsLastDays
		{
			int coins[NUM_LAST_DAYS];
			BOOKKEEPER->GetCoinsLastDays( coins );
			PRINT_OPEN(f, ssprintf("Coins for Last %d Days",NUM_LAST_DAYS), true );
			{
				BEGIN_TABLE(4);
				for( int i=0; i<NUM_LAST_DAYS; i++ )
				{
					CString sDay = (i==0) ? "Today" : ssprintf("%d day(s) ago",i);
					TABLE_LINE2( sDay, coins[i] );
				}
				END_TABLE;
			}
			PRINT_CLOSE(f);
		}

		// GetCoinsLastWeeks
		{
			int coins[NUM_LAST_WEEKS];
			BOOKKEEPER->GetCoinsLastWeeks( coins );
			PRINT_OPEN(f, ssprintf("Coins for Last %d Weeks",NUM_LAST_WEEKS), true );
			{
				BEGIN_TABLE(4);
				for( int i=0; i<NUM_LAST_WEEKS; i++ )
				{
					CString sWeek;
					switch( i )
					{
					case 0:		sWeek = "This week";	break;
					case 1:		sWeek = "Last week";	break;
					default:	sWeek = ssprintf("%d weeks ago",i);	break;
					}
					TABLE_LINE2( sWeek, coins[i] );
				}
				END_TABLE;
			}
			PRINT_CLOSE(f);
		}

		// GetCoinsByDayOfWeek
		{
			int coins[DAYS_IN_WEEK];
			BOOKKEEPER->GetCoinsByDayOfWeek( coins );
			PRINT_OPEN(f, "Coins by Day of Week", true );
			{
				BEGIN_TABLE(4);
				for( int i=0; i<DAYS_IN_WEEK; i++ )
				{
					CString sDay = DAY_OF_WEEK_TO_NAME[i];
					TABLE_LINE2( sDay, coins[i] );
				}
				END_TABLE;
			}
			PRINT_CLOSE(f);
		}

		// GetCoinsByHour
		{
			int coins[HOURS_PER_DAY];
			BOOKKEEPER->GetCoinsByHour( coins );
			PRINT_OPEN(f, ssprintf("Coins for Last %d Hours",HOURS_PER_DAY), true );
			{
				BEGIN_TABLE(4);
				for( int i=0; i<HOURS_PER_DAY; i++ )
				{
					CString sHour = ssprintf("hour %d",i);
					TABLE_LINE2( sHour, coins[i] );
				}
				END_TABLE;
			}
			PRINT_CLOSE(f);
		}
	}
	PRINT_CLOSE(f);
}

enum SaveType { SAVE_TYPE_PLAYER, SAVE_TYPE_MACHINE };

void SaveStatsWebPageToDir( CString sDir, SaveType saveType, const Profile *pProfile, const Profile *pProfileMachine )
{
	CString fn = sDir + STATS_HTML;

	LOG->Trace( "Writing %s ...", fn.c_str() );

	//
	// Open file
	//
	RageFile f;
	if( !f.Open( fn, RageFile::WRITE ) )
	{
		LOG->Warn( "Couldn't open file '%s'", fn.c_str() );
		return;
	}

	//
	// Gather data
	//
	vector<Song*> vpSongs = SONGMAN->GetAllSongs();
	vector<Steps*> vpAllSteps;
	map<Steps*,Song*> mapStepsToSong;
	{
		for( unsigned i=0; i<vpSongs.size(); i++ )
		{
			Song* pSong = vpSongs[i];
			vector<Steps*> vpSteps = pSong->GetAllSteps();
			for( unsigned j=0; j<vpSteps.size(); j++ )
			{
				Steps* pSteps = vpSteps[j];
				if( pSteps->IsAutogen() )
					continue;	// skip
				vpAllSteps.push_back( pSteps );
				mapStepsToSong[pSteps] = pSong;
			}
		}
	}
	vector<Course*> vpCourses;
	SONGMAN->GetAllCourses( vpCourses, false );

	//
	// Calculate which StepTypes to show
	//
	vector<StepsType> vStepsTypesToShow;
	{
		for( StepsType st=(StepsType)0; st<NUM_STEPS_TYPES; st=(StepsType)(st+1) )
		{
			// don't show if there are no Steps of this StepsType 
			bool bOneSongHasStepsForThisStepsType = false;
			for( unsigned i=0; i<vpSongs.size(); i++ )
			{
				Song* pSong = vpSongs[i];
				vector<Steps*> vpSteps;
				pSong->GetSteps( vpSteps, st, DIFFICULTY_INVALID, -1, -1, "", false );
				if( !vpSteps.empty() )
				{
					bOneSongHasStepsForThisStepsType = true;
					break;
				}
			}

			if( bOneSongHasStepsForThisStepsType )
				vStepsTypesToShow.push_back( st );
		}
	}

	//
	// Print HTML headers
	//
	{
		f.PutLine( ssprintf("\
<html>\n\
<head>\n\
<META HTTP-EQUIV=\"Content-Type\" Content=\"text/html; charset=UTF-8\">\n\
<title>%s</title>\n\
<SCRIPT LANGUAGE=\"JavaScript\" TYPE=\"text/javascript\">\n\
<!-- // hide from old browsers\n\
\n\
// hide text from MSIE browsers\n\
\n\
with (document)\n\
{\n\
	write(\"<STYLE TYPE='text/css'>\");\n\
	if (navigator.appName == 'Microsoft Internet Explorer')\n\
		{\n\
		write(\".hiddentext {display:none} .visibletext {display:block} .outline {cursor:hand; text-decoration:underline}\");\n\
		}\n\
	write(\"</STYLE>\");\n\
}\n\
\n\
// show text on click for MSIE browsers\n\
\n\
function expandIt(whichEl)\n\
{\n\
	if (navigator.appName == 'Microsoft Internet Explorer')\n\
		{\n\
		whichEl.style.display = (whichEl.style.display == \"block\" ) ? \"none\" : \"block\";\n\
		}\n\
	else return;\n\
}\n\
// end hiding from old browsers -->\n\
</SCRIPT>\n\
<link rel='stylesheet' type='text/css' href='%s'>\n\
</head>\n\
<body>",
STATS_TITLE.c_str(), STYLE_CSS ) );
	}

	CString sType;
	switch( saveType )
	{
	case SAVE_TYPE_PLAYER:	sType = "Player: ";		break;
	case SAVE_TYPE_MACHINE:	sType = "Machine: ";	break;
	}

	CString sName = 
		pProfile->m_sLastUsedHighScoreName.empty() ? 
		pProfile->m_sName :
		pProfile->m_sLastUsedHighScoreName;
	time_t ltime = time( NULL );
	CString sTime = ctime( &ltime );

	f.Write( ssprintf(
		"<table border='0' cellpadding='0' cellspacing='0' width='100%%' cellspacing='5'><tr><td><h1>%s</h1></td><td>%s %s<br>%s</td></tr></table>\n",
		STATS_TITLE.c_str(), sType.c_str(), sName.c_str(), sTime.c_str() ) );

	CString sPlayerName = pProfile->GetDisplayName();
	CString sMachineName = pProfileMachine->GetDisplayName();

	switch( saveType )
	{
	case SAVE_TYPE_PLAYER:
		PrintStatistics(		f, pProfile,		sPlayerName+"'s Statistics",	vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintPopularity(		f, pProfile,		sPlayerName+"'s Popularity",	vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintHighScores(		f, pProfile,		sPlayerName+"'s High Scores",	vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintPopularity(		f, pProfileMachine, sMachineName+"'s Popularity",	vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintHighScores(		f, pProfileMachine, sMachineName+"'s High Scores",	vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		break;
	case SAVE_TYPE_MACHINE:
		PrintStatistics(		f, pProfile,		"Statistics",				vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintPopularity(		f, pProfile,		"Popularity",				vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintHighScores(		f, pProfile,		"High Scores",				vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintDifficultyTable(	f, pProfile,		"Difficulty Table",			vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintInventoryList(		f, pProfile,		"Song Information",			vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintBookkeeping(		f, pProfile,		"Bookkeeping",				vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );	
		break;
	default:
		ASSERT(0);
	}

	f.PutLine( ssprintf("<p class='footer'>%s</p>\n", STATS_FOOTER.c_str()) );

	f.PutLine( "</body>" );
	f.PutLine( "</html>" );

	//
	// Copy CSS file from theme.  If the copy fails, oh well...
	// 
	CString sStyleFile = THEME->GetPathToO("ProfileManager style.css");
	FileCopy( sStyleFile, sDir+STYLE_CSS );
	LOG->Trace( "Done." );		
}

void SavePlayerHtmlToDir( CString sDir, const Profile* pProfilePlayer, const Profile* pProfileMachine )
{
	SaveStatsWebPageToDir( sDir, SAVE_TYPE_PLAYER, pProfilePlayer, pProfileMachine );
}

void SaveMachineHtmlToDir( CString sDir, const Profile* pProfileMachine )
{
	SaveStatsWebPageToDir( sDir, SAVE_TYPE_MACHINE, pProfileMachine, pProfileMachine );
}


