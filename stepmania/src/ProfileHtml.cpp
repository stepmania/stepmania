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
#include "PrefsManager.h"
#include "CryptManager.h"


const CString STATS_HTML	= "Stats.html";
const CString STYLE_CSS		= "Style.css";

#define TITLE				THEME->GetMetric("ProfileHtml","Title")
#define FOOTER				THEME->GetMetric("ProfileHtml","Footer")
#define VERIFICATION_TEXT	THEME->GetMetric("ProfileHtml","VerificationText")


static int g_Level = 1;

inline CString MakeUniqueId()												{ CString s="id"+ssprintf("%d%d%d",rand(),rand(),rand()); return s; }
inline void PRINT_OPEN(RageFile &f,CString sName,bool bExpanded,CString sID){ g_Level++; ASSERT(g_Level>0 && g_Level<6); f.Write( ssprintf("<div class='section%d'>\n" "<h%d onClick='expandIt(%s); return false' CLASS='outline'>%s</h%d>\n" "<DIV ID='%s' CLASS='%s'>\n", g_Level, g_Level, sID.c_str(), sName.c_str(), g_Level, sID.c_str(), bExpanded?"visibletext":"hiddentext") ); }
inline void PRINT_OPEN(RageFile &f,CString sName,bool bExpanded=false)		{ PRINT_OPEN(f,sName,bExpanded,MakeUniqueId()); }
inline void PRINT_CLOSE(RageFile &f)										{ f.Write( "</div>\n" "</div>\n" ); g_Level--; ASSERT(g_Level>=0); }

#define PRETTY_PERCENT(numerator,denominator) ssprintf("%0.2f%%",(float)numerator/(float)denominator*100)

struct Table
{
	Table() {}

	struct Line
	{
		Line() {}
		Line(CString n)							{ sName = n; }
		Line(CString n,CString v)				{ sName = n; sValue = v; }
		Line(CString n,bool v)					{ sName = n; sValue = ssprintf("%s",v?"yes":"no"); }
		Line(CString n,int v)					{ sName = n; sValue = ssprintf("%d",v); }
		Line(int r,CString n,int v)				{ sRank = ssprintf("%d",r); sName = n; sValue = ssprintf("%d",v); }
		Line(int r,CString n,CString sn,int v)	{ sRank = ssprintf("%d",r); sName = n; sSubName = sn; sValue = ssprintf("%d",v); }
		Line(int r,CString n,CString sn,CString ssn,int v)	{ sRank = ssprintf("%d",r); sName = n; sSubName = sn; sSubSubName = ssn; sValue = ssprintf("%d",v); }
		Line(int r,CString n,CString v)				{ sRank = ssprintf("%d",r); sName = n; sValue = v; }
		Line(int r,CString n,CString sn,CString v)	{ sRank = ssprintf("%d",r); sName = n; sSubName = sn; sValue = v; }
		Line(int r,CString n,CString sn,CString ssn,CString v)	{ sRank = ssprintf("%d",r); sName = n; sSubName = sn; sSubSubName = ssn; sValue = v; }

		CString sRank;
		CString sName;
		CString sSubName;
		CString sSubSubName;
		CString sValue;
	};

	int iNumCols;
	vector<Line> vLines;
};

#define BEGIN_TABLE(cols)			{ Table table; table.iNumCols=cols;
#define TABLE_LINE1(p1)				table.vLines.push_back( Table::Line(p1) );
#define TABLE_LINE2(p1,p2)			table.vLines.push_back( Table::Line(p1,p2) );
#define TABLE_LINE3(p1,p2,p3)		table.vLines.push_back( Table::Line(p1,p2,p3) );
#define TABLE_LINE4(p1,p2,p3,p4)	table.vLines.push_back( Table::Line(p1,p2,p3,p4) );
#define TABLE_LINE5(p1,p2,p3,p4,p5)	table.vLines.push_back( Table::Line(p1,p2,p3,p4,p5) );
#define END_TABLE					PrintTable( f, table ); }

inline void PrintTable(RageFile &f,Table &table)
{
	const vector<Table::Line> &vLines = table.vLines;
	int &iNumCols = table.iNumCols;

	ASSERT( iNumCols > 0 );

	if( vLines.empty() )
		return;

	bool bPrintRank = !vLines.empty() && !vLines[0].sRank.empty();
	bool bPrintInstructions = !vLines.empty() && vLines[0].sRank.empty() && vLines[0].sSubName.empty() && vLines[0].sSubSubName.empty() && vLines[0].sValue.empty();

	int iMaxItemsPerCol = (vLines.size()+iNumCols-1) / iNumCols;
	iNumCols = (vLines.size()+iMaxItemsPerCol-1) / iMaxItemsPerCol;	// round up
	f.Write(ssprintf("<table class='%s'><tr>\n",bPrintInstructions?"instructions":"group"));
	if( iNumCols == 0 )
	{
		f.Write("<td>empty</td>\n");
	}
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
			if( bPrintRank )
			{
				f.Write("<td>");
				f.Write("<p class='songtitle'>");
				f.Write( line.sName );
				f.Write("</p>");
				if( !line.sSubName.empty() )
				{
					f.Write("<p class='songsubtitle'>");
					f.Write( line.sSubName );
					f.Write("</p>");
				}
				f.Write("</td>");
			}
			else if( line.sValue.empty() )
			{
				f.Write("<td>");
				f.Write( line.sName );
				f.Write("</td>");
			}
			else
			{
				f.Write("<td class='name'>");
				f.Write( line.sName );
				f.Write("</td>");
			}

			if( !line.sSubSubName.empty() )
			{
				f.Write("<td>&nbsp;</td>");
				f.Write("<td>");
				f.Write("<p class='stepsdescription'>");
				f.Write( line.sSubSubName );
				f.Write("</p>");
				f.Write("</td>");
			}

			if( !line.sValue.empty() )
			{
				f.Write("<td>&nbsp;</td>");
				f.Write("<td class='value'>");
				f.Write( line.sValue );
				f.Write("</td>");
			}
			f.Write( "\n" );

			f.Write("</tr>");
		}
		f.Write("</table>\n");

		f.Write("</td>\n");
	}
	f.Write("</tr></table>\n");
} 

#define STRING_AS_LINK(s) CString("<a href='"+s+"'>"+s+"</a>")

void PrintInstructions( RageFile &f, const Profile *pProfile, CString sTitle )
{
	PRINT_OPEN(f,sTitle);
	{
		PRINT_OPEN(f,"Overview",true);
		{
			BEGIN_TABLE(1);
			TABLE_LINE1("<p>This directory contains all your game profile data.  Please read these instructions before modifying or moving any of these files.  Modifying files may result in irreversible loss of your data.</p>\n");
			END_TABLE;
		}
		PRINT_CLOSE(f);

		PRINT_OPEN(f,"Description of Files");
		{
			BEGIN_TABLE(1);
			TABLE_LINE2(STRING_AS_LINK(EDITS_SUBDIR),		CString("Place edit step files in this directory.  See the Edits section below for more details."));
			TABLE_LINE2(STRING_AS_LINK(SCREENSHOTS_SUBDIR),	CString("All screenshots that you take are saved in this directory."));
			TABLE_LINE2(DONT_SHARE_SIG,						CString("This is a secret file that you should never share with anyone else.  See the Sharing Your Data section below for more details."));
			TABLE_LINE2(STRING_AS_LINK(EDITABLE_INI),		CString("Holds preferences that you can edit offline using your home computer.  This file is not digitally signed."));
			TABLE_LINE2(STATS_HTML,							CString("You're looking at this file now.  It contains a formatted view of all your saved data, plus some data from the last machine you played on."));
			TABLE_LINE2(STATS_HTML+SIGNATURE_APPEND,		CString("The signature file for "+STATS_HTML+"."));
			TABLE_LINE2(STRING_AS_LINK(STATS_XML),			CString("This is the primary data file.  It contains all the score data and statistics, and is read by the game when you join."));
			TABLE_LINE2(STATS_XML+SIGNATURE_APPEND,			CString("The signature file for "+STATS_XML+"."));
			TABLE_LINE2(STYLE_CSS,							CString("Contains style data used by "+STATS_HTML+"."));
			END_TABLE;
		}
		PRINT_CLOSE(f);

		PRINT_OPEN(f,"Digital Signatures");
		{
			BEGIN_TABLE(1);
			TABLE_LINE1("<p>Some files on your memory card have a corresponding digital signature.  Digital signatures are used to verify that your files haven't been modified outside of the game.  This prevents cheaters from changing their score data and then passing it off as real.</p>\n");
			TABLE_LINE1("<p>Before the game reads your memory card data, it verifies that your data and digital signatures match.  If the data and signatures don't match, then your data has been modified outside of the game.  When the game detects this condition, it will completely ignore your tampered data.  It is very important that you -do not- modify any file that has a digital signature because this will cause your data to be permanently unusable.</p>\n");
			TABLE_LINE1("<p>If someone else shares their profile data with you, you can verify their score data using digital signatures.  To verify their data, you'll need 3 things:</p>\n");
			TABLE_LINE1("<p>- the "+STATS_XML+" data file</p>\n");
			TABLE_LINE1("<p>- the digital signature file "+STATS_XML+SIGNATURE_APPEND+"</p>\n");
			TABLE_LINE1("<p>- a <a href='http://www.stepmania.com/stepmania/digitalsignatures/'>small utility</a> that will check data against a signature</p>\n");
			END_TABLE;
		}
		PRINT_CLOSE(f);

		PRINT_OPEN(f,"About Editable Preferences");
		{
			BEGIN_TABLE(1);
			TABLE_LINE1("<p>The file "+STRING_AS_LINK(EDITABLE_INI)+" contains settings that you can modify using your home computer.  If you're using a Windows PC, you can click <a href='"+EDITABLE_INI+"'>here</a> to open the file for editing.  This file is not digitally signed and the game will import any changes you make to this file.</p>\n");
			END_TABLE;
		}
		PRINT_CLOSE(f);

		PRINT_OPEN(f,"About Screenshots");
		{
			BEGIN_TABLE(1);
			TABLE_LINE1("<p>The "+STRING_AS_LINK(SCREENSHOTS_SUBDIR)+" directory contains all screenshots that you've captured while playing the game.  See the Screenshots section later on this page to see thumbnails and more information captured at the time of the screenshot.  The Screenshots section also lists an MD5 hash of the screenshot file.  You can use the MD5 has to verify that the screenshot has not been modified since it was first saved.</p>\n");
			TABLE_LINE1("<p>If your memory card is full, you may delete files from this directory or the move files to another disk.  If you move a screenshot to another disk, you can still verify the screenshot file using the MD5 hash.</p>\n");
			END_TABLE;
		}
		PRINT_CLOSE(f);

		PRINT_OPEN(f,"About Edits");
		{
			BEGIN_TABLE(1);
			TABLE_LINE1("<p>The "+STRING_AS_LINK(EDITS_SUBDIR)+" directory contains edit step files that you've created yourself or downloaded from the internet.  See <a href='http://www.stepmania.com/stepmania/edits'>here</a> for more information about edit files.</p>\n");
			END_TABLE;
		}
		PRINT_CLOSE(f);

		PRINT_OPEN(f,"Sharing Your Data");
		{
			BEGIN_TABLE(1);
			TABLE_LINE1("<p>You can share your score data with other players or submit it to a web site for an internet ranking contest.  When sharing your data though, do -not- share the file "+DONT_SHARE_SIG+".  "+DONT_SHARE_SIG+" is private digital signature required by the game before loading memory card data.  Without "+DONT_SHARE_SIG+", the person you're sharing data with can verify that your data is original, but can't load your data using their memory card or pass your scores off as their own.</p>\n");
			TABLE_LINE1("<p>If you do share your "+DONT_SHARE_SIG+" with someone, then they can completely replicate your memory card and pass your scores off as their own.</p>\n");
			END_TABLE;
		}
		PRINT_CLOSE(f);

		PRINT_OPEN(f,"Backing Up/Moving Your Data");
		{
			BEGIN_TABLE(1);
			TABLE_LINE1("<p>To make a backup of your data, copy the entire "+PREFSMAN->m_sMemoryCardProfileSubdir+"/ directory on the root of your memory card to your local hard drive.</p>\n");
			TABLE_LINE1("<p>To move your data from the current memory card to a new memory card, move the entire "+PREFSMAN->m_sMemoryCardProfileSubdir+"/ directory on your current memory card to the root directory on the new memory card.</p>\n");
			END_TABLE;
		}
		PRINT_CLOSE(f);
	}
	PRINT_CLOSE(f);
}

void PrintStatistics( RageFile &f, const Profile *pProfile, CString sTitle, vector<Song*> &vpSongs, vector<Steps*> &vpAllSteps, vector<StepsType> &vStepsTypesToShow, map<Steps*,Song*> mapStepsToSong, vector<Course*> vpCourses )
{
	PRINT_OPEN(f,sTitle,true);
	{
		PRINT_OPEN(f,"General Info",true);
		{
			BEGIN_TABLE(2);
			TABLE_LINE2( "DisplayName",						pProfile->m_sDisplayName );
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
		

		PRINT_OPEN(f,"Num Songs Played by PlayMode");
		{
			BEGIN_TABLE(4);
			FOREACH_PlayMode( pm )
				TABLE_LINE2( PlayModeToString(pm), pProfile->m_iNumSongsPlayedByPlayMode[pm] );
			END_TABLE;
		}
		PRINT_CLOSE(f);


		PRINT_OPEN(f,"Num Songs Played by Style");
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


		PRINT_OPEN(f,"Num Songs Played by Difficulty");
		{
			BEGIN_TABLE(4);
			FOREACH_Difficulty( dc )
				TABLE_LINE2( DifficultyToString(dc), pProfile->m_iNumSongsPlayedByDifficulty[dc] );
			END_TABLE;
		}
		PRINT_CLOSE(f);

		PRINT_OPEN(f,"Num Songs Played by Meter");
		{
			BEGIN_TABLE(4);
			for( int i=MIN_METER; i<=MAX_METER; i++ )
				TABLE_LINE2( ssprintf("Meter %d",i), pProfile->m_iNumSongsPlayedByMeter[i] );
			END_TABLE;
		}
		PRINT_CLOSE(f);

		PRINT_OPEN(f,"Grade Count");
		{
			int iGradeCount[NUM_GRADES];
			ZERO( iGradeCount );

			for( unsigned i=0; i<vpAllSteps.size(); i++ )
			{
				Steps* pSteps = vpAllSteps[i];
				const HighScoreList &hsl = pProfile->GetStepsHighScoreList(pSteps);
				if( hsl.vHighScores.empty() )
					continue;	// no data, skip this one
				Grade g = hsl.GetTopScore().grade;
				ASSERT( g != GRADE_NO_DATA );
				ASSERT( g < NUM_GRADES );
				ASSERT( g >= 0 );
				iGradeCount[g] ++;
			}

			BEGIN_TABLE(6);
			for( int g=0; g<PREFSMAN->m_iNumGradeTiersUsed; g++ )
				TABLE_LINE2( GradeToThemedString((Grade)g), iGradeCount[g] );
			TABLE_LINE2( GradeToThemedString(GRADE_FAILED), iGradeCount[GRADE_FAILED] );
			END_TABLE;
		}
		PRINT_CLOSE(f);
	}
	PRINT_CLOSE(f);
}

void PrintPopularity( RageFile &f, const Profile *pProfile, CString sTitle, vector<Song*> &vpSongs, vector<Steps*> &vpAllSteps, vector<StepsType> &vStepsTypesToShow, map<Steps*,Song*> mapStepsToSong, vector<Course*> vpCourses )
{
	PRINT_OPEN(f, sTitle );
	if( vpSongs.size() )
	{
		SortSongPointerArrayByNumPlays( vpSongs, pProfile, true );
		Song* pSongPopularThreshold = vpSongs[ vpSongs.size()*2/3 ];
		int iPopularNumPlaysThreshold = pProfile->GetSongNumTimesPlayed(pSongPopularThreshold);
		
		// unplayed songs are always considered unpopular
		if( iPopularNumPlaysThreshold == 0 )
			iPopularNumPlaysThreshold = 1;

		unsigned uMaxToShow = min( vpSongs.size(), (unsigned)100 );

		// compute total plays
		int iTotalPlays = 0;
		for( unsigned i=0; i<vpSongs.size(); i++ )
			iTotalPlays += pProfile->GetSongNumTimesPlayed( vpSongs[i] );


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
					TABLE_LINE4(i+1, pSong->GetDisplayMainTitle(), pSong->GetDisplaySubTitle(), PRETTY_PERCENT(iNumTimesPlayed,iTotalPlays) );
				}
				if( i == 0 )
					TABLE_LINE1("empty");
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
					TABLE_LINE4(i+1, pSong->GetDisplayMainTitle(), pSong->GetDisplaySubTitle(), PRETTY_PERCENT(iNumTimesPlayed,iTotalPlays) );
				}
				if( i == 0 )
					TABLE_LINE1("empty");
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
					int iNumTimesPlayed = pProfile->GetStepsNumTimesPlayed(pSteps);
					if( iNumTimesPlayed==0 )
						continue;	// skip
					Song* pSong = mapStepsToSong[pSteps];
					CString s;
					s += GAMEMAN->NotesTypeToString(pSteps->m_StepsType);
					s += " ";
					s += DifficultyToString(pSteps->GetDifficulty());
					TABLE_LINE5(i+1, pSong->GetDisplayMainTitle(), pSong->GetDisplaySubTitle(), s, PRETTY_PERCENT(iNumTimesPlayed,iTotalPlays) );
				}
				if( i == 0 )
					TABLE_LINE1("empty");
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
					int iNumTimesPlayed = pProfile->GetCourseNumTimesPlayed(pCourse);
					TABLE_LINE3(i+1, pCourse->m_sName, PRETTY_PERCENT(iNumTimesPlayed,iTotalPlays) );
				}
				if( i == 0 )
					TABLE_LINE1("empty");
				END_TABLE;
			}
			PRINT_CLOSE(f);
		}
	}
	PRINT_CLOSE(f);
}


// return true if anything was printed
typedef bool (*FnPrintSong)(RageFile &f, const Profile *pProfile, Song* pSong );
typedef bool (*FnPrintGroup)(RageFile &f, const Profile *pProfile, CString sGroup );
typedef bool (*FnPrintStepsType)(RageFile &f, const Profile *pProfile, StepsType st );


bool PrintSongsInGroup( RageFile &f, const Profile *pProfile, CString sGroup, FnPrintSong pFn )
{
	vector<Song*> vpSongs;
	SONGMAN->GetSongs( vpSongs, sGroup );

	if( vpSongs.empty() )
		return false;

	PRINT_OPEN(f, sGroup );
	{
		bool bPrintedAny = false;
		for( unsigned i=0; i<vpSongs.size(); i++ )
		{
			Song* pSong = vpSongs[i];
			bPrintedAny |= pFn( f, pProfile, pSong );
		}

		if( !bPrintedAny )
		{
			BEGIN_TABLE(1);
			TABLE_LINE1("empty");
			END_TABLE;
		}
	}
	PRINT_CLOSE(f);

	return true;
}

bool PrintGroups( RageFile &f, const Profile *pProfile, CString sTitle, FnPrintGroup pFn )
{
	CStringArray asGroups;
	SONGMAN->GetGroupNames( asGroups );
	
	if( asGroups.empty() )
		return false;

	PRINT_OPEN(f, sTitle );
	{
		bool bPrintedAny = false;

		for( unsigned g=0; g<asGroups.size(); g++ )
		{
			CString sGroup = asGroups[g];
			bPrintedAny |= pFn( f, pProfile, sGroup );
		}

		if( !bPrintedAny )
		{
			BEGIN_TABLE(1);
			TABLE_LINE1("empty");
			END_TABLE;
		}
	}
	PRINT_CLOSE(f);

	return true;
}

bool PrintStepsTypes( RageFile &f, const Profile *pProfile, CString sTitle, vector<StepsType> vStepsTypesToShow, FnPrintStepsType pFn )
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

	return true;
}

bool PrintHighScoresForSong( RageFile &f, const Profile *pProfile, Song* pSong )
{
	int iNumTimesPlayed = pProfile->GetSongNumTimesPlayed(pSong);
	if( iNumTimesPlayed == 0 )
		return false;	// skip

	vector<Steps*> vpSteps = pSong->GetAllSteps();

	PRINT_OPEN(f, pSong->GetFullDisplayTitle() );
	{
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

	return true;
}

bool PrintHighScoresForGroup(RageFile &f, const Profile *pProfile, CString sGroup )
{
	return PrintSongsInGroup( f, pProfile, sGroup, PrintHighScoresForSong );
}

void PrintHighScores( RageFile &f, const Profile *pProfile, CString sTitle, vector<Song*> &vpSongs, vector<Steps*> &vpAllSteps, vector<StepsType> &vStepsTypesToShow, map<Steps*,Song*> mapStepsToSong, vector<Course*> vpCourses )
{
	PrintGroups( f, pProfile, sTitle, PrintHighScoresForGroup );
}

bool PrintGradeTableForStepsType( RageFile &f, const Profile *pProfile, StepsType st )
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
			f.PutLine( ssprintf("<td>%s</td>", Capitalize(DifficultyToString(dc)).c_str()) );
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
				if( pSteps && !pSteps->IsAutogen() )
				{
					f.PutLine("<td>");
					f.PutLine("<table class='meterAndGrade'><tr>");
					f.PutLine( ssprintf("<td class='meter'><p class='meter'>%d</p></td>",pSteps->GetMeter()) );
					HighScore hs = pProfile->GetStepsHighScoreList( pSteps ).GetTopScore();
					Grade grade = hs.grade;
					if( grade != GRADE_NO_DATA )
						f.PutLine( ssprintf("<td class='grade'><p class='grade'>%s</p></td>",GradeToThemedString(grade).c_str()) );
					f.PutLine("</tr></table>");
					f.PutLine("</td>");
				}
				else
				{
					f.PutLine( "<td>&nbsp;</td>" );
				}
			}

			f.Write( "</tr>" );
		}

		f.PutLine( "</table>\n" );
	}
	PRINT_CLOSE(f);

	return true;
}

void PrintGradeTable( RageFile &f, const Profile *pProfile, CString sTitle, vector<Song*> &vpSongs, vector<Steps*> &vpAllSteps, vector<StepsType> &vStepsTypesToShow, map<Steps*,Song*> mapStepsToSong, vector<Course*> vpCourses )
{
	PrintStepsTypes( f, pProfile, sTitle, vStepsTypesToShow, PrintGradeTableForStepsType );
}

bool PrintInventoryForSong( RageFile &f, const Profile *pProfile, Song* pSong )
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
		TABLE_LINE2( "NumTimesPlayed", pProfile->GetSongNumTimesPlayed(pSong) );
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
				BEGIN_TABLE(3);
				TABLE_LINE2( "Meter", pSteps->GetMeter() );
				TABLE_LINE2( "Description", pSteps->GetDescription() );
				TABLE_LINE2( "NumTimesPlayed", pProfile->GetStepsNumTimesPlayed(pSteps) );
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

	return true;
}

bool PrintInventoryForGroup( RageFile &f, const Profile *pProfile, CString sGroup )
{
	return PrintSongsInGroup( f, pProfile, sGroup, PrintInventoryForSong );
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

void PrintScreenshot( RageFile &f, const Profile::Screenshot &ss )
{
	CString sHtmlPath = "Screenshots/"+ss.sFileName;
	CString sImgTag = ssprintf("<a href='%s' target='_new'><img class='screenshot' src='%s' width='160' height='120'></a>", sHtmlPath.c_str(), sHtmlPath.c_str() );
	CString sDetails = "<p>This is a screenshot</p>\n<p>We have no idea where it came from</p>";

	BEGIN_TABLE(1);

	TABLE_LINE2( sImgTag, sDetails );

	END_TABLE;
}

void PrintScreenshots( RageFile &f, const Profile *pProfile, CString sTitle, CString sProfileDir )
{
	PRINT_OPEN(f, sTitle );
	{
		PRINT_OPEN(f, "Less Than 1 Month Old" );
		{
			for( unsigned i=0; i<pProfile->m_vScreenshots.size(); i++ )
			{
				const Profile::Screenshot &ss = pProfile->m_vScreenshots[i];
				PrintScreenshot( f, ss );
			}
		}
		PRINT_CLOSE(f);
	}
	PRINT_CLOSE(f);
}


void SaveStatsWebPage( 
	CString sDir, 
	const Profile *pProfile, 	
	const Profile *pMachineProfile, 	
	HtmlType htmlType
	)
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
TITLE.c_str(), STYLE_CSS.c_str() ) );
	}

	CString sName = pProfile->GetDisplayName();
	CString sMachineName = pMachineProfile->GetDisplayName();
	time_t ltime = time( NULL );
	CString sTime = ctime( &ltime );

	CString sNameCell;
	switch( htmlType )
	{
	case HTML_TYPE_PLAYER:
		sNameCell = ssprintf(
			"Player Name: %s<br>\n"
			"Last Machine: %s<br>\n"
			"%s\n",
			sName.c_str(), sMachineName.c_str(), sTime.c_str() );
		break;
	case HTML_TYPE_MACHINE:
		sNameCell = ssprintf(
			"Machine: %s<br>\n"
			"%s\n",
			sName.c_str(), sTime.c_str() );
		break;
	default:
		ASSERT(0);
	}

	f.Write( ssprintf(
		"<table border='0' cellpadding='0' cellspacing='0' width='100%%' cellspacing='5'><tr><td><h1>%s</h1></td><td>%s</td></tr></table>\n",
		TITLE.c_str(), sNameCell.c_str() ) );

	switch( htmlType )
	{
	case HTML_TYPE_PLAYER:
		PrintInstructions(	f, pProfile,	"Instructions" );
		PrintStatistics(	f, pProfile,	"My Statistics",			vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintPopularity(	f, pProfile,	"My Popularity",			vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintHighScores(	f, pProfile,	"My High Scores",			vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintScreenshots(	f, pProfile,	"My Screenshots",			sDir );
		PrintGradeTable(	f, pProfile,	"My Grade Table",			vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintPopularity(	f, pProfile,	"Last Machine Popularity",	vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintHighScores(	f, pProfile,	"Last Machine High Scores",	vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		break;
	case HTML_TYPE_MACHINE:
		PrintStatistics(	f, pProfile,	"Statistics",				vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintPopularity(	f, pProfile,	"Popularity",				vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintHighScores(	f, pProfile,	"High Scores",				vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintScreenshots(	f, pProfile,	"Screenshots",				sDir );
		PrintGradeTable(	f, pProfile,	"Grade Table",				vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintInventoryList(	f, pProfile,	"Song Information",			vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );
		PrintBookkeeping(	f, pProfile,	"Bookkeeping",				vpSongs, vpAllSteps, vStepsTypesToShow, mapStepsToSong, vpCourses );	
		break;
	default:
		ASSERT(0);
	}

	f.PutLine( ssprintf("<p class='footer'>%s</p>\n", FOOTER.c_str()) );

	f.PutLine( "</body>\n" );
	f.PutLine( "</html>\n" );
	f.Close();

	//
	// Sign Stats.html
	//
	if( PREFSMAN->m_bSignProfileData )
		CryptManager::SignFileToFile(fn);
	
	//
	// Copy CSS file from theme.  If the copy fails, oh well...
	// 
	CString sStyleFile = THEME->GetPathToO("ProfileManager style.css");
	FileCopy( sStyleFile, sDir+STYLE_CSS );
	LOG->Trace( "Done." );		

}


