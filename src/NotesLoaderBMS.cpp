#include "global.h"
#include "NotesLoaderBMS.h"
#include "NoteData.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameManager.h"
#include "SongManager.h"
#include "RageFile.h"
#include "SongUtil.h"
#include "StepsUtil.h"
#include "Song.h"
#include "Steps.h"
#include "RageUtil_CharConversions.h"
#include "NoteTypes.h"
#include "NotesLoader.h"
#include "PrefsManager.h"
#include "BackgroundUtil.h"
#include "ActorUtil.h"
#include "RageFileManager.h"

#include <cstddef>
#include <vector>


/* BMS encoding:	tap-hold
 * 4&8panel:	Player1		Player2
 * Left			11-51		21-61
 * Down			13-53		23-63
 * Up			15-55		25-65
 * Right		16-56		26-66
 *
 * 6panel:		Player1
 * Left			11-51
 * Left+Up		12-52
 * Down			13-53
 * Up			14-54
 * Up+Right		15-55
 * Right		16-56
 *
 * Notice that 15 and 25 have double meanings!  What were they thinking???
 * While reading in, use the 6 panel mapping.  After reading in, detect if
 * only 4 notes are used.  If so, shift the Up+Right column back to the Up
 * column
 *
 * BMSes are used for games besides dance and so we're borking up BMSes that are for popn/beat/etc.
 *
 * popn-nine:		11-15,22-25
 * popn-five:   	13-15,21-22
 * beat-single5:	11-16
 * beat-double5:	11-16,21-26
 * beat-single7:	11-16,18-19
 * beat-double7:	11-16,18-19,21-26,28-29
 *
 * So the magics for these are:
 * popn-nine: nothing >5, with 12, 14, 22 and/or 24
 * popn-five: nothing >5, with 14 and/or 22
 * beat-*: can't tell difference between beat-single and dance-solo
 * 	18/19 marks beat-single7, 28/29 marks beat-double7
 * 	beat-double uses 21-26.
*/

// Find the largest common substring at the start of both strings.
static RString FindLargestInitialSubstring( const RString &string1, const RString &string2 )
{
	// First see if the whole first string matches an appropriately-sized
	// substring of the second, then keep chopping off the last character of
	// each until they match.
	unsigned i;
	for( i = 0; i < string1.size() && i < string2.size(); ++i )
		if( string1[i] != string2[i] )
			break;

	return string1.substr( 0, i );
}

static void SearchForDifficulty( RString sTag, Steps *pOut )
{
	sTag.MakeLower();

	// Only match "Light" in parentheses.
	if( sTag.find( "(light" ) != sTag.npos )
	{
		pOut->SetDifficulty( Difficulty_Easy );
	}
	else if( sTag.find( "another" ) != sTag.npos )
	{
		pOut->SetDifficulty( Difficulty_Hard );
	}
	else if( sTag.find( "(solo)" ) != sTag.npos )
	{
		pOut->SetDescription( "Solo" );
		pOut->SetDifficulty( Difficulty_Edit );
	}

	LOG->Trace( "Tag \"%s\" is %s", sTag.c_str(), DifficultyToString(pOut->GetDifficulty()).c_str() );
}

static void SlideDuplicateDifficulties( Song &p )
{
	/* BMS files have to guess the Difficulty from the meter; this is inaccurate,
	* and often leads to duplicates. Slide duplicate difficulties upwards.
	* We only do this with BMS files, since a very common bug was having *all*
	* difficulties slid upwards due to (for example) having two beginner steps.
	* We do a second pass in Song::TidyUpData to eliminate any remaining duplicates
	* after this. */
	FOREACH_ENUM( StepsType,st )
	{
		FOREACH_ENUM( Difficulty, dc )
		{
			if( dc == Difficulty_Edit )
				continue;

			std::vector<Steps*> vSteps;
			SongUtil::GetSteps( &p, vSteps, st, dc );

			StepsUtil::SortNotesArrayByDifficulty( vSteps );
			for( unsigned k=1; k<vSteps.size(); k++ )
			{
				Steps* pSteps = vSteps[k];

				Difficulty dc2 = std::min( (Difficulty)(dc+1), Difficulty_Challenge );
				pSteps->SetDifficulty( dc2 );
			}
		}
	}
}

void BMSLoader::GetApplicableFiles( const RString &sPath, std::vector<RString> &out )
{
	GetDirListing( sPath + RString("*.bms"), out );
	GetDirListing( sPath + RString("*.bme"), out );
	GetDirListing( sPath + RString("*.bml"), out );
	GetDirListing( sPath + RString("*.pms"), out );
}

/*===========================================================================*/

struct BMSObject
{
	int channel;
	int measure;
	float position;
	bool flag;
	RString value;
};

inline bool operator<(BMSObject const &lhs, BMSObject const &rhs)
{
	if (lhs.measure != rhs.measure)
	{
		return lhs.measure < rhs.measure;
	}
	if (lhs.position != rhs.position)
	{
		return lhs.position < rhs.position;
	}
	if (lhs.channel == 1)
	{
		return false;
	}
	if (rhs.channel == 1)
	{
		return true;
	}
	return lhs.channel < rhs.channel;
}
inline bool operator>(BMSObject const &lhs, BMSObject const &rhs)
{
	return operator<(rhs, lhs);
}
inline bool operator<=(BMSObject const &lhs, BMSObject const &rhs)
{
	return !operator<(rhs, lhs);
}
inline bool operator>=(BMSObject const &lhs, BMSObject const &rhs)
{
	return !operator<(lhs, rhs);
}

struct BMSMeasure
{
	float size;
};

const int MaxBMSElements = 1296; // ZZ in b36
typedef std::map<RString, RString> BMSHeaders;
typedef std::map<int, BMSMeasure> BMSMeasures;
typedef std::vector<BMSObject> BMSObjects;

class BMSChart
{

public:
	BMSChart();
	bool Load( const RString &path );
	bool GetHeader( const RString &header, RString &out );
	RString path;

	BMSObjects objects;
	BMSHeaders headers;
	BMSMeasures measures;
	std::map<int, bool> referencedTracks;

	void TidyUpData();
};

BMSChart::BMSChart()
{
}

bool BMSChart::GetHeader( const RString &header, RString &out )
{
	if( headers.find(header) == headers.end() ) return false;
	out = headers[header];
	return true;
}

// az: Implement #RANDOM, #IF, #ELSE, #ELSEIF and #ENDIF.
struct bmsCommandTree
{

	struct bmsNodeS { // Each of these imply one branching level.
		unsigned int branchHeight;
		enum {
			CT_NULL,
			CT_CONDITIONALCHAIN,
			CT_IF,
			CT_ELSEIF,
			CT_ELSE
		} conditionType; // #IF or #ELSE?

		union {
			int conditionValue; // value which we got for this #random
			int conditionTriggerValue; // value which triggers this branch. does not apply to #ELSE
		};

		BMSHeaders Commands;
		std::vector<RString> ChannelCommands;
		std::vector<bmsNodeS*> branches;
		bmsNodeS* parent;

		bmsNodeS()
		{
			parent = nullptr;
			conditionValue = 0;
			conditionType = CT_NULL;
		}

		~bmsNodeS()
		{
			for (bmsNodeS *b : branches)
			{
				delete b;
			}
		}
	};

	bmsNodeS *currentNode;
	bmsNodeS root;
	std::vector<unsigned int> randomStack;

	int line;
	RString path;

	bmsCommandTree()
	{
		line = 0;
		root.branchHeight = 0;
		root.conditionValue = 0;
		root.conditionTriggerValue = -1;
		root.parent = nullptr;
		root.conditionType = bmsNodeS::CT_NULL;

		currentNode = &root;
	}

	~bmsCommandTree()
	{
	}

	bmsNodeS *addConditionalChain()
	{
		bmsNodeS *newNode = new bmsNodeS;

		newNode->conditionValue = randomStack[currentNode->branchHeight];
		newNode->parent = currentNode;
		newNode->branchHeight = currentNode->branchHeight;
		newNode->conditionType = bmsNodeS::CT_CONDITIONALCHAIN;

		currentNode->branches.push_back(newNode);
		return newNode;
	}

	bmsNodeS* createIfNode(bmsNodeS *Chain, int value)
	{
		bmsNodeS *newNode = new bmsNodeS;

		newNode->conditionValue = randomStack[currentNode->branchHeight];
		newNode->parent = Chain;
		newNode->branchHeight = Chain->branchHeight + 1;
		newNode->conditionTriggerValue = value;
		newNode->conditionType = bmsNodeS::CT_IF;
		Chain->branches.push_back(newNode);

		return newNode;
	}

	bmsNodeS* createElseIfNode(bmsNodeS *Chain, int value)
	{
		bmsNodeS *newNode = new bmsNodeS;

		newNode->conditionValue = randomStack[Chain->branchHeight];
		newNode->parent = Chain;
		newNode->branchHeight = Chain->branchHeight + 1;
		newNode->conditionTriggerValue = value;
		newNode->conditionType = bmsNodeS::CT_ELSEIF;
		Chain->branches.push_back(newNode);

		return newNode;
	}

	bmsNodeS* createElseNode(bmsNodeS *Chain)
	{
		bmsNodeS *newNode = new bmsNodeS;

		newNode->conditionValue = randomStack[Chain->branchHeight];
		newNode->parent = Chain;
		newNode->branchHeight = Chain->branchHeight + 1;
		newNode->conditionType = bmsNodeS::CT_ELSE;
		Chain->branches.push_back(newNode);

		return newNode;
	}

	/*
		A condition chain can't ever be the current node.
		A conditional chain will never have more than one #IF.
		The current node is always an #IF/#ELSE/#ELSEIF node or the root node.
		All #IFs must be parented by a condition chain for interpreting #ELSE and #ELSEIF founds on that chain.
		Likewise, all #ELSE and #ELSEIF commands must be parented by a condition chain node.
		Therefore, the root node will only have conditional chains as branches
		and all commands that must be evaluated without question.
		-az
	*/

	void appendNodeElements(bmsNodeS* node, BMSHeaders &headersOut, std::vector<RString> &linesOut)
	{
		for (BMSHeaders::iterator i = node->Commands.begin(); i != node->Commands.end(); ++i)
		{
			headersOut[i->first] = i->second;
		}

		for (std::vector<RString>::iterator i = node->ChannelCommands.begin(); i != node->ChannelCommands.end(); ++i)
		{
			linesOut.push_back(*i);
		}
	}

	bool triggerBranches(bmsNodeS* node, BMSHeaders &headersOut, std::vector<RString> &linesOut)
	{
		for (bmsNodeS *b : node->branches)
			if (evaluateNode(b, headersOut, linesOut))
			{
				return true;
			}

		return false;
	}

	bool evaluateNode(bmsNodeS* node, BMSHeaders &headersOut, std::vector<RString> &linesOut)
	{
		switch (node->conditionType)
		{
		case bmsNodeS::CT_CONDITIONALCHAIN:
			triggerBranches(node, headersOut, linesOut);
			break;
		case bmsNodeS::CT_IF:
		case bmsNodeS::CT_ELSEIF: // Their differences are solved at node creation time.
			if (node->parent->conditionValue == node->conditionTriggerValue)
			{
				appendNodeElements(node, headersOut, linesOut);
				triggerBranches(node, headersOut, linesOut);
				return true; // returning true means to stop trying to triggering branches
			}
			break;
		case bmsNodeS::CT_ELSE:
				appendNodeElements(node, headersOut, linesOut);
				triggerBranches(node, headersOut, linesOut);
				return true;
			break;
		case bmsNodeS::CT_NULL:
			appendNodeElements(node, headersOut, linesOut);
			triggerBranches(node, headersOut, linesOut);
		default:
			break;
		}

		// returning false means to execute any other branches.
		return false;
	}

	void evaluateBMSTree(BMSHeaders &headersOut, std::vector<RString> &linesOut)
	{
		evaluateNode(&root, headersOut, linesOut);
	}

	void doStatement(RString statement, std::map<int, bool> &referencedTracks)
	{
		line++;

		if (statement.length() == 0) // Skip.
			return;

		// LTrim the statement to allow indentation
		std::size_t hash = statement.find('#');

		if (hash == RString::npos)
			return;

		statement = statement.substr(hash);

		std::size_t space = statement.find(' ');
		RString name = statement.substr(0, space);
		RString value = "";

		if (space != statement.npos)
			value = statement.substr(space + 1);
		name.MakeLower();

		if (name == "#if")
		{
			if (randomStack.size() < currentNode->branchHeight + 1)
			{
				LOG->UserLog("Song file", path, "Line %d: Missing #RANDOM. Warning: Branch will be considered false!", line);

				while (randomStack.size() < currentNode->branchHeight + 1)
					randomStack.push_back(0);
			}

			bmsNodeS *chain = addConditionalChain();
			currentNode = createIfNode(chain, atoi(value.c_str()));
		}
		else if (name == "#else")
		{
			if (currentNode->parent != nullptr) // Not the root node.
			{
				if (currentNode->parent->conditionType == bmsNodeS::CT_CONDITIONALCHAIN)
				{
					currentNode = createElseNode(currentNode->parent);
					return;
				}
				else
					LOG->UserLog("Song file", path, "Line %d: #else without matching #if chain.\n", line);
			} else
				LOG->UserLog("Song file", path, "Line %d: #else used at root level.\n", line);
		}
		else if (name == "#elseif")
		{
			if (currentNode->parent != nullptr) // Not the root node.
			{
				if (currentNode->parent->conditionType == bmsNodeS::CT_CONDITIONALCHAIN)
				{
					currentNode = createElseIfNode(currentNode->parent, atoi(value.c_str()));
				}else
					LOG->UserLog("Song file", path, "Line %d: #elseif without matching #if chain.\n", line);
			}else
				LOG->UserLog("Song file", path, "Line %d: #elseif used at root level.\n", line);
		}
		else if (name == "#endif" || name == "#end")
		{
			if (currentNode->parent != nullptr) // not the root node
			{
				currentNode = currentNode->parent;
			}

			if (currentNode->conditionType != bmsNodeS::CT_CONDITIONALCHAIN)
			{
				LOG->UserLog("Song file", path, "Line %d: #endif without a matching #if!", line);
				return;
			}

			// We're in a conditional chain, so that means we can go one level up to our *real* parent.
			currentNode = currentNode->parent;
		}
		else if (name == "#random")
		{
			while (randomStack.size() < currentNode->branchHeight + 1) // if we're on branch level N we need N+1 values.
				randomStack.push_back(0);

			randomStack[currentNode->branchHeight] = rand() % StringToInt(value) + 1;
		}
		else
		{
			if (statement.size() >= 7 &&
				('0' <= statement[1] && statement[1] <= '9') &&
				('0' <= statement[2] && statement[2] <= '9') &&
				('0' <= statement[3] && statement[3] <= '9') &&
				('0' <= statement[4] && statement[4] <= '9') &&
				('0' <= statement[5] && statement[5] <= '9') &&
				statement[6] == ':')
			{
				int channel = atoi(statement.substr(4, 2).c_str());
				currentNode->ChannelCommands.push_back(statement);

				if ((11 <= channel && channel <= 19) || (21 <= channel && channel <= 29))
				{
					referencedTracks[channel] = true;
				}

			}
			else
			{
				currentNode->Commands[name] = value;
			}
		}

		// we're done.
	}
};

bool BMSChart::Load( const RString &chartPath )
{
	bmsCommandTree Tree;
	Tree.path = chartPath;
	path = chartPath;

	RageFile file;
	if( !file.Open(path) )
	{
		LOG->UserLog( "Song file", path, "couldn't be opened: %s", file.GetError().c_str() );
		return false;
	}

	while (!file.AtEOF())
	{
		RString line;
		if (file.GetLine(line) == -1)
		{
			LOG->UserLog("Song file", path, "had a read error: %s", file.GetError().c_str());
			return false;
		}

		StripCrnl(line);

		Tree.doStatement(line, referencedTracks);
	}

	std::vector<RString> lines;
	Tree.evaluateBMSTree(headers, lines);

	for (const RString& line : lines)
	{
		RString data = line.substr(7);
		int measure = atoi(line.substr(1, 3).c_str());
		int channel = atoi(line.substr(4, 2).c_str());
		bool flag = false;
		if (channel == 2)
		{
			// special channel: time signature
			BMSMeasure m = { StringToFloat(data) };
			this->measures[measure] = m;
		}
		else
		{
			if (channel >= 51)
			{
				channel -= 40;
				flag = true;
			}
			int count = data.size() / 2;
			for (int i = 0; i < count; i++)
			{
				RString value = data.substr(2 * i, 2);
				if (value != "00")
				{
					value.MakeLower();
					BMSObject o = { channel, measure, (float)i / count, flag, value };
					objects.push_back(o);
				}
			}
		}
	}



	TidyUpData();

	return true;
}

void BMSChart::TidyUpData()
{
	sort( objects.begin(), objects.end() );
}

class BMSSong {

	std::map<RString, int> mapKeysoundToIndex;
	Song *out;

	bool backgroundsPrecached;
	void PrecacheBackgrounds(const RString &dir);
	std::map<RString, RString> mapBackground;

public:
	BMSSong( Song *song );
	int AllocateKeysound( RString filename, RString path );
	bool GetBackground( RString filename, RString path, RString &bgfile );
	Song *GetSong();
};

BMSSong::BMSSong( Song *song )
{
	out = song;
	backgroundsPrecached = false;

	// import existing keysounds from song
	for( unsigned i = 0; i < out->m_vsKeysoundFile.size(); i ++ )
	{
		mapKeysoundToIndex[out->m_vsKeysoundFile[i]] = i;
	}
}

Song *BMSSong::GetSong()
{
	return out;
}

int BMSSong::AllocateKeysound( RString filename, RString path )
{
	if( mapKeysoundToIndex.find( filename ) != mapKeysoundToIndex.end() )
	{
		return mapKeysoundToIndex[filename];
	}

	// try to normalize the filename first!

	// FIXME: garbled file names seem to crash the app.
	// this might not be the best place to put this code.
	if( !utf8_is_valid(filename) )
		return -1;

	/* Due to bugs in some programs, many BMS files have a "WAV" extension
	 * on files in the BMS for files that actually have some other extension.
	 * Do a search. Don't do a wildcard search; if sData is "song.wav",
	 * we might also have "song.png", which we shouldn't match. */
	RString normalizedFilename = filename;
	RString dir = out->GetSongDir();

	if (dir.empty())
		dir = Dirname(path);

	if( !IsAFile(dir + normalizedFilename) )
	{
		std::vector<RString> const& exts= ActorUtil::GetTypeExtensionList(FT_Sound);
		for(std::size_t i = 0; i < exts.size(); ++i)
		{
			RString fn = SetExtension( normalizedFilename, exts[i] );
			if( IsAFile(dir + fn) )
			{
				normalizedFilename = fn;
				break;
			}
		}
	}

	if( !IsAFile(dir + normalizedFilename) )
	{
		mapKeysoundToIndex[filename] = -1;
		LOG->UserLog( "Song file", dir, "references key \"%s\" that can't be found", normalizedFilename.c_str() );
		return -1;
	}

	if( mapKeysoundToIndex.find( normalizedFilename ) != mapKeysoundToIndex.end() )
	{
		mapKeysoundToIndex[filename] = mapKeysoundToIndex[normalizedFilename];
		return mapKeysoundToIndex[normalizedFilename];
	}

	unsigned index = out->m_vsKeysoundFile.size();
	out->m_vsKeysoundFile.push_back( normalizedFilename );
	mapKeysoundToIndex[filename] = index;
	mapKeysoundToIndex[normalizedFilename] = index;
	return index;

}

bool BMSSong::GetBackground( RString filename, RString path, RString &bgfile )
{
	// Check for already tried backgrounds
	if( mapBackground.find( filename ) != mapBackground.end() )
	{
		RString bg = mapBackground[filename];
		if( bg == "" )
		{
			return false;
		}
		bgfile = bg;
		return true;
	}

	// FIXME: garbled file names seem to crash the app.
	// this might not be the best place to put this code.
	if( !utf8_is_valid(filename) )
		return false;

	RString normalizedFilename = filename;
	RString dir = out->GetSongDir();

	if (dir.empty())
		dir = Dirname(path);

	if( !backgroundsPrecached )
	{
		PrecacheBackgrounds(dir);
	}

	if( !IsAFile(dir + normalizedFilename) )
	{
		std::vector<RString> exts;
		ActorUtil::AddTypeExtensionsToList(FT_Movie, exts);
		ActorUtil::AddTypeExtensionsToList(FT_Bitmap, exts);
		for(std::size_t i = 0; i < exts.size(); ++i)
		{
			RString fn = SetExtension( normalizedFilename, exts[i] );
			if( IsAFile(dir + fn) )
			{
				normalizedFilename = fn;
				break;
			}
		}
	}

	if( !IsAFile(dir + normalizedFilename) )
	{
		mapBackground[filename] = "";
		LOG->UserLog( "Song file", dir, "references bmp \"%s\" that can't be found", normalizedFilename.c_str() );
		return false;
	}

	mapBackground[filename] = normalizedFilename;
	bgfile = normalizedFilename;
	return true;

}

void BMSSong::PrecacheBackgrounds(const RString &dir)
{
	if( backgroundsPrecached ) return;
	backgroundsPrecached = true;
	std::vector<RString> arrayPossibleFiles;

	std::vector<RString> exts;
	ActorUtil::AddTypeExtensionsToList(FT_Movie, exts);
	ActorUtil::AddTypeExtensionsToList(FT_Bitmap, exts);
	FILEMAN->GetDirListingWithMultipleExtensions(dir + RString("*."), exts, arrayPossibleFiles);

	for( unsigned i = 0; i < arrayPossibleFiles.size(); i++ )
	{
		for (unsigned j = 0; j < exts.size(); j++)
		{
			RString fn = SetExtension( arrayPossibleFiles[i], exts[j] );
			mapBackground[fn] = arrayPossibleFiles[i];
		}
		mapBackground[arrayPossibleFiles[i]] = arrayPossibleFiles[i];
	}
}

struct BMSChartInfo {
	RString title;
	RString artist;
	RString genre;

	RString bannerFile;
	RString backgroundFile;
	RString stageFile;
	RString musicFile;
	RString previewFile;

	std::map<int, RString> backgroundChanges;
	float previewStart;
	BMSChartInfo() { previewStart = 0; }
};

class BMSChartReader {

	BMSChart *in;
	Steps *out;
	BMSSong *song;

	void ReadHeaders();
	void CalculateStepsType();
	bool ReadNoteData();

	StepsType DetermineStepsType();

	int lntype;
	RString lnobj;

	int nonEmptyTracksCount;
	std::map<int, bool> nonEmptyTracks;

	int GetKeysound( const BMSObject &obj );

	std::map<RString, int> mapValueToKeysoundIndex;

public:
	BMSChartReader(BMSChart *chart, Steps *steps, BMSSong *song);
	bool Read();

	Steps *GetSteps();

	BMSChartInfo info;
	int player;
	float initialBPM;

};

BMSChartReader::BMSChartReader(BMSChart *chart, Steps *steps, BMSSong *bmsSong)
{
	this->in   = chart;
	this->out  = steps;
	this->song = bmsSong;
	this->nonEmptyTracks = chart->referencedTracks;
}

bool BMSChartReader::Read()
{
	ReadHeaders();
	CalculateStepsType();
	if( !ReadNoteData() ) return false;
	return true;
}

void BMSChartReader::ReadHeaders()
{
	lntype = 1;
	player = 1;
	for( BMSHeaders::iterator it = in->headers.begin(); it != in->headers.end(); it ++ )
	{
		if( it->first == "#player" )
		{
			player = atoi(it->second.c_str());
		}
		else if( it->first == "#title" )
		{
			info.title = it->second;
		}
		else if( it->first == "#artist" )
		{
			info.artist = it->second;
		}
		else if( it->first == "#genre" )
		{
			info.genre = it->second;
		}
		else if( it->first == "#banner" )
		{
			info.bannerFile = it->second;
		}
		else if( it->first == "#backbmp" )
		{
			/* XXX: don't use #backbmp if StepsType is beat-*.
			 * incorrectly used in other simulators; see
			 * http://www.geocities.jp/red_without_right_stick/backbmp/ */
			info.backgroundFile = it->second;
		}
		else if( it->first == "#stagefile" )
		{
			info.stageFile = it->second;
		}
		else if( it->first == "#bpm" )
		{
			initialBPM = StringToFloat(it->second);
		}
		else if( it->first == "#lntype" )
		{
			int myLntype = atoi(it->second.c_str());
			if( myLntype == 1 )
			{
				lntype = myLntype;
				// XXX: we only support #LNTYPE 1 for now.
			}
		}
		else if( it->first == "#lnobj" )
		{
			lnobj = it->second;
			lnobj.MakeLower();
		}
		else if( it->first == "#playlevel" )
		{
			out->SetMeter( StringToInt(it->second) );
		}
		else if( it->first == "#difficulty")
		{
			// only set the difficulty if the #difficulty tag is between 1 and 6 (beginner~edit)
			int diff = StringToInt(it->second)-1; // BMS uses 1 to 6, SM uses 0 to 5
			if(diff>=0 && diff<NUM_Difficulty) {
				out->SetDifficulty( (Difficulty)diff );
			}
		}
		else if (it->first == "#music")
		{
			info.musicFile = it->second;
			out->SetMusicFile(it->second);
		}
		else if (it->first == "#preview")
		{
			info.previewFile = it->second;
		}
		else if (it->first == "#offset")
		{
			// This gets copied into the real timing data later.
			out->m_Timing.m_fBeat0OffsetInSeconds = -StringToFloat(it->second);
		}
		else if (it->first == "#maker")
		{
			out->SetCredit(it->second);
		}
		else if (it->first == "#previewpoint")
			info.previewStart = StringToFloat(it->second);
	}
}

void BMSChartReader::CalculateStepsType()
{
	nonEmptyTracksCount = nonEmptyTracks.size();
	out->m_StepsType = DetermineStepsType();
	if(out->m_StepsType == StepsType_Invalid)
	{
		out->m_StepsTypeStr = "BMS_loaded_invalid_stepstype";
	}
	else
	{
		out->m_StepsTypeStr = GAMEMAN->GetStepsTypeInfo(out->m_StepsType).szName;
	}
}

enum BmsRawChannel
{
	BMS_RAW_P1_KEY1 = 11,
	BMS_RAW_P1_KEY2 = 12,
	BMS_RAW_P1_KEY3 = 13,
	BMS_RAW_P1_KEY4 = 14,
	BMS_RAW_P1_KEY5 = 15,
	BMS_RAW_P1_TURN = 16,
	BMS_RAW_P1_KEY6 = 18,
	BMS_RAW_P1_KEY7 = 19,
	BMS_RAW_P2_KEY1 = 21,
	BMS_RAW_P2_KEY2 = 22,
	BMS_RAW_P2_KEY3 = 23,
	BMS_RAW_P2_KEY4 = 24,
	BMS_RAW_P2_KEY5 = 25,
	BMS_RAW_P2_TURN = 26,
	BMS_RAW_P2_KEY6 = 28,
	BMS_RAW_P2_KEY7 = 29
};

StepsType BMSChartReader::DetermineStepsType()
{
	switch( player )
	{
		case 1:	// "1 player"
			switch( nonEmptyTracksCount )
			{
				case 4:		return StepsType_dance_single;
				case 5:
					if( nonEmptyTracks.find(BMS_RAW_P2_KEY2) != nonEmptyTracks.end() ) return StepsType_popn_five;

					[[fallthrough]];
				case 6:
					// FIXME: There's no way to distinguish between these types.
					// They use the same number of tracks. Assume it's a Beat
					// type, since they are more common.
					//return StepsType_dance_solo;
					return StepsType_beat_single5;
					// az: Allow kb7 style charts
				case 7:
				{
					// az (for nixtrix): kb7 layouts do not leave any gaps using either of these layouts.
					// if we find a compatible layout that doesn't have gaps, we've stumbled upon a real
					// kb7 file.
					BmsRawChannel layoutA[] = {
						BMS_RAW_P1_TURN,
						BMS_RAW_P1_KEY1,
						BMS_RAW_P1_KEY2,
						BMS_RAW_P1_KEY3,
						BMS_RAW_P1_KEY4,
						BMS_RAW_P1_KEY5,
						BMS_RAW_P1_KEY6,
					};

					BmsRawChannel layoutB[] = {
						BMS_RAW_P1_KEY1,
						BMS_RAW_P1_KEY2,
						BMS_RAW_P1_KEY3,
						BMS_RAW_P1_KEY4,
						BMS_RAW_P1_KEY5,
						BMS_RAW_P1_KEY6,
						BMS_RAW_P1_KEY7,
					};

					int gaps = 0;
					for (int i = 0; i < 7; i++)
					{
						if (nonEmptyTracks.find(layoutA[i]) == nonEmptyTracks.end())
							gaps++;
					}

					if (gaps == 0) // kb7 file is a layout A file
						return StepsType_kb7_single;

					gaps = 0;
					for (int i = 0; i < 7; i++)
					{
						if (nonEmptyTracks.find(layoutB[i]) == nonEmptyTracks.end())
							gaps++;
					}

					if (gaps == 0) // kb7 file is a layout B file
						return StepsType_kb7_single;

					// neither huh, then it's a beatmania file that for some reason
					// um, chose to not fill in a lane.
					return StepsType_beat_single7;
				}
				case 8:		return StepsType_beat_single7;
				case 9:		return StepsType_popn_nine;
				// XXX: Some double files doesn't have #player.
				case 12:	return StepsType_beat_double5;
				case 16:	return StepsType_beat_double7;
				default:
					if( nonEmptyTracksCount > 8 )
						return StepsType_beat_double7;
					else
						return StepsType_beat_single7;
			}
		case 2:	// couple/battle
			return StepsType_dance_couple;
		case 3:	// double
			switch( nonEmptyTracksCount )
			{
				case 8:		return StepsType_dance_double;
				case 12:		return StepsType_beat_double5;
				case 16:		return StepsType_beat_double7;
				case 5:		return StepsType_popn_five;
				case 9:		return StepsType_popn_nine;
				default:		return StepsType_beat_double7;
			}
		default:
			LOG->UserLog( "Song file", in->path, "has an invalid #PLAYER value %d.", player );
			return StepsType_Invalid;
	}
}

int BMSChartReader::GetKeysound( const BMSObject &obj )
{
	std::map<RString, int>::iterator it = mapValueToKeysoundIndex.find(obj.value);
	if( it == mapValueToKeysoundIndex.end() )
	{
		int index = -1;
		BMSHeaders::iterator iu = in->headers.find("#wav" + obj.value);
		if( iu != in->headers.end() )
		{
			index = song->AllocateKeysound(iu->second, in->path);
		}
		mapValueToKeysoundIndex[obj.value] = index;
		return index;
	}
	else
	{
		return it->second;
	}
}

struct BMSAutoKeysound {
	int row;
	int index;
};

struct bmFrac {
	bmFrac(long long n, long long d)
		:num(n), den(d)
	{}
	long long num;
	long long den;
};

bmFrac toFraction(double f)
{
	double df;
	long long upper = 1LL, lower = 1LL;
	df = 1;

	while (std::abs(df - f) > 0.000001)
	{
		if (df < f)
		{
			upper++;
		}
		else
		{
			lower++;
		}
		df = (double)upper / lower;
	}

	return bmFrac( upper, lower );
}

bool BMSChartReader::ReadNoteData()
{
	if( out->m_StepsType == StepsType_Invalid )
	{
		LOG->UserLog( "Song file", in->path, "has an unknown steps type" );
		return false;
	}

	float currentBPM;
	int tracks = GAMEMAN->GetStepsTypeInfo( out->m_StepsType ).iNumTracks;

	NoteData   nd;
	TimingData td;

	td.m_fBeat0OffsetInSeconds = out->m_Timing.m_fBeat0OffsetInSeconds;
	nd.SetNumTracks( tracks );
	td.SetBPMAtRow( 0, currentBPM = initialBPM );

	// set up note transformation vector.
	int *transform = new int[tracks];
	int *holdStart = new int[tracks];
	int *lastNote = new int[tracks];

	for( int i = 0; i < tracks; i ++ ) holdStart[i] = -1;
	for( int i = 0; i < tracks; i ++ ) lastNote[i] = -1;

	switch( out->m_StepsType )
	{
	case StepsType_dance_single:
		if (nonEmptyTracks.find(BMS_RAW_P1_KEY5) != nonEmptyTracks.end()) // Old style 4k charts
		{
			transform[0] = BMS_RAW_P1_KEY1;
			transform[1] = BMS_RAW_P1_KEY3;
			transform[2] = BMS_RAW_P1_KEY5;
			transform[3] = BMS_RAW_P1_TURN;
		} else // myo2/rd style 4k chart
		{
			transform[0] = BMS_RAW_P1_TURN;
			transform[1] = BMS_RAW_P1_KEY1;
			transform[2] = BMS_RAW_P1_KEY2;
			transform[3] = BMS_RAW_P1_KEY3;
		}
		break;
	case StepsType_dance_double:
	case StepsType_dance_couple:
		transform[0] = BMS_RAW_P1_KEY1;
		transform[1] = BMS_RAW_P1_KEY3;
		transform[2] = BMS_RAW_P1_KEY5;
		transform[3] = BMS_RAW_P1_TURN;
		transform[4] = BMS_RAW_P2_KEY1;
		transform[5] = BMS_RAW_P2_KEY3;
		transform[6] = BMS_RAW_P2_KEY5;
		transform[7] = BMS_RAW_P2_TURN;
		break;
	case StepsType_dance_solo:
	case StepsType_beat_single5:
		// Hey! Why are these exactly the same? :-)
		if (nonEmptyTracks.find(BMS_RAW_P1_TURN) != nonEmptyTracks.end()) { // Linear beat-5 layout
			transform[0] = BMS_RAW_P1_KEY1;
			transform[1] = BMS_RAW_P1_KEY2;
			transform[2] = BMS_RAW_P1_KEY3;
			transform[3] = BMS_RAW_P1_KEY4;
			transform[4] = BMS_RAW_P1_KEY5;
			transform[5] = BMS_RAW_P1_TURN;
		} else // Linear solo layout
		{
			transform[0] = BMS_RAW_P1_KEY1;
			transform[1] = BMS_RAW_P1_KEY2;
			transform[2] = BMS_RAW_P1_KEY3;
			transform[3] = BMS_RAW_P1_KEY4;
			transform[4] = BMS_RAW_P1_KEY5;
			transform[5] = BMS_RAW_P1_KEY6;
		}
		break;
	case StepsType_popn_five:
		transform[0] = BMS_RAW_P1_KEY3;
		transform[1] = BMS_RAW_P1_KEY4;
		transform[2] = BMS_RAW_P1_KEY5;
		// fix these columns!
		transform[3] = BMS_RAW_P2_KEY2;
		transform[4] = BMS_RAW_P2_KEY3;
		break;
	case StepsType_popn_nine:
		transform[0] = BMS_RAW_P1_KEY1; // lwhite
		transform[1] = BMS_RAW_P1_KEY2; // lyellow
		transform[2] = BMS_RAW_P1_KEY3; // lgreen
		transform[3] = BMS_RAW_P1_KEY4; // lblue
		transform[4] = BMS_RAW_P1_KEY5; // red
		// fix these columns!
		transform[5] = BMS_RAW_P2_KEY2; // rblue
		transform[6] = BMS_RAW_P2_KEY3; // rgreen
		transform[7] = BMS_RAW_P2_KEY4; // ryellow
		transform[8] = BMS_RAW_P2_KEY5; // rwhite
		break;
	case StepsType_beat_double5:
		transform[0] = BMS_RAW_P1_KEY1;
		transform[1] = BMS_RAW_P1_KEY2;
		transform[2] = BMS_RAW_P1_KEY3;
		transform[3] = BMS_RAW_P1_KEY4;
		transform[4] = BMS_RAW_P1_KEY5;
		transform[5] = BMS_RAW_P1_TURN;
		transform[6] = BMS_RAW_P2_KEY1;
		transform[7] = BMS_RAW_P2_KEY2;
		transform[8] = BMS_RAW_P2_KEY3;
		transform[9] = BMS_RAW_P2_KEY4;
		transform[10] = BMS_RAW_P2_KEY5;
		transform[11] = BMS_RAW_P2_TURN;
		break;
	case StepsType_beat_single7:
	case StepsType_kb7_single:
		if(    nonEmptyTracks.find(BMS_RAW_P1_KEY7) == nonEmptyTracks.end()
			&& nonEmptyTracks.find(BMS_RAW_P1_TURN) != nonEmptyTracks.end() )
		{
			/* special case for o2mania style charts:
			 * the turntable is used for first key while the real 7th key is not used. */
			transform[0] = BMS_RAW_P1_TURN;
			transform[1] = BMS_RAW_P1_KEY1;
			transform[2] = BMS_RAW_P1_KEY2;
			transform[3] = BMS_RAW_P1_KEY3;
			transform[4] = BMS_RAW_P1_KEY4;
			transform[5] = BMS_RAW_P1_KEY5;
			transform[6] = BMS_RAW_P1_KEY6;

			if (tracks != 7)
				transform[7] = BMS_RAW_P1_KEY7;
		}
		else
		{
			transform[0] = BMS_RAW_P1_KEY1;
			transform[1] = BMS_RAW_P1_KEY2;
			transform[2] = BMS_RAW_P1_KEY3;
			transform[3] = BMS_RAW_P1_KEY4;
			transform[4] = BMS_RAW_P1_KEY5;
			transform[5] = BMS_RAW_P1_KEY6;
			transform[6] = BMS_RAW_P1_KEY7;

			if (tracks != 7)
				transform[7] = BMS_RAW_P1_TURN;
		}
		break;
	case StepsType_beat_double7:
		transform[0] = BMS_RAW_P1_KEY1;
		transform[1] = BMS_RAW_P1_KEY2;
		transform[2] = BMS_RAW_P1_KEY3;
		transform[3] = BMS_RAW_P1_KEY4;
		transform[4] = BMS_RAW_P1_KEY5;
		transform[5] = BMS_RAW_P1_KEY6;
		transform[6] = BMS_RAW_P1_KEY7;
		transform[7] = BMS_RAW_P1_TURN;
		transform[8] = BMS_RAW_P2_KEY1;
		transform[9] = BMS_RAW_P2_KEY2;
		transform[10] = BMS_RAW_P2_KEY3;
		transform[11] = BMS_RAW_P2_KEY4;
		transform[12] = BMS_RAW_P2_KEY5;
		transform[13] = BMS_RAW_P2_KEY6;
		transform[14] = BMS_RAW_P2_KEY7;
		transform[15] = BMS_RAW_P2_TURN;
		break;
	default:
		ASSERT_M(0, ssprintf("Invalid StepsType when parsing BMS file %s!", in->path.c_str()));
	}

	int reverseTransform[30];
	for( int i = 0; i < 30; i ++ ) reverseTransform[i] = -1;
	for( int i = 0; i < tracks; i ++ ) reverseTransform[transform[i]] = i;

	int trackMeasure = -1;
	float measureStartBeat = 0.0f;
	double measureSize = 0.0f;
	float adjustedMeasureSize = 0.0f;
	float measureAdjust = 1.0f;
	int firstNoteMeasure = 0;

	for( unsigned i = 0; i < in->objects.size(); i ++ )
	{
		BMSObject &obj = in->objects[i];
		int channel = obj.channel;
		firstNoteMeasure = obj.measure;
		if( channel == 3 || channel == 8 || channel == 9 ||  channel == 1 || (11 <= channel && channel <= 19) || (21 <= channel && channel <= 29) )
		{
			break;
		}
	}

	std::vector<BMSAutoKeysound> autos;

	for( unsigned i = 0; i < in->objects.size(); i ++ )
	{
		BMSObject &obj = in->objects[i];
		while( trackMeasure < obj.measure )
		{
			trackMeasure ++;
			measureStartBeat += adjustedMeasureSize;
			measureSize = 4.0f;
			BMSMeasures::iterator it = in->measures.find(trackMeasure);
			if( it != in->measures.end() ) measureSize = it->second.size * 4.0;
			adjustedMeasureSize = measureSize;
			if( trackMeasure < firstNoteMeasure ) adjustedMeasureSize = measureSize = 4.0f;

			// measure size adjustment
			{
				bmFrac numFrac = toFraction(measureSize);
				long long num = numFrac.num;
				long long den = 4 * numFrac.den;

				while ( num % 2 == 0 && den % 2 == 0 && den > 4  ) { // Both are multiples of 2
					num /= 2;
					den /= 2;
				}
				td.SetTimeSignatureAtRow( BeatToNoteRow(measureStartBeat), num, den );

				// Since BMS measure events only last through the measure, we need to restore the default measure length.
				td.SetTimeSignatureAtRow(BeatToNoteRow(measureStartBeat + measureSize), 4, 4);
			}
			// end measure size adjustment
		}

		int row = BeatToNoteRow( measureStartBeat + adjustedMeasureSize * obj.position );
		int channel = obj.channel;
		bool hold = obj.flag;

		if( channel == 3 ) // bpm change
		{
			unsigned int bpm;
			if( sscanf(obj.value, "%x", &bpm) == 1 )
			{
				if( bpm > 0 ) td.SetBPMAtRow( row, measureAdjust * (currentBPM = bpm) );
			}
		}
		else if( channel == 4 ) // bga change
		{
			/*
			if( !bgaFound )
			{
				info.bgaRow = row;
				bgaFound = true;
			}
			 */
			RString search = ssprintf( "#bga%s", obj.value.c_str() );
			BMSHeaders::iterator it = in->headers.find( search );
			if( it != in->headers.end() )
			{
				// TODO: #BGA isn't supported yet.
			}
			else
			{
				search = ssprintf( "#bmp%s", obj.value.c_str() );
				it = in->headers.find( search );

				if (it != in->headers.end()) // To elaborate, this means this is an unknown key.
				{
					RString bg;
					if (song->GetBackground(it->second, in->path, bg))
					{
						info.backgroundChanges[row] = bg;
					}
				}
				else
				{
					LOG->UserLog("Song file", in->path.c_str(), "uses key \"%s\" for a bmp change which is undefined.", obj.value.c_str());
				}
			}
		}
		else if( channel == 8 ) // bpm change (extended)
		{
			RString search = ssprintf( "#bpm%s", obj.value.c_str() );
			BMSHeaders::iterator it = in->headers.find( search );
			if( it != in->headers.end() )
			{
				td.SetBPMAtRow( row, measureAdjust * (currentBPM = StringToFloat(it->second)) );
			}
			else
			{
				LOG->UserLog( "Song file", in->path.c_str(), "has tag \"%s\" which cannot be found.", search.c_str() );
			}
		}
		else if( channel == 9 ) // stops
		{
			RString search = ssprintf( "#stop%s", obj.value.c_str() );
			BMSHeaders::iterator it = in->headers.find( search );
			if( it != in->headers.end() )
			{
				td.SetStopAtRow( row, (StringToFloat(it->second) / 48.0f) * (60.0f / currentBPM) );
			}
			else
			{
				LOG->UserLog( "Song file", in->path.c_str(), "has tag \"%s\" which cannot be found.", search.c_str() );
			}
		}
		else if( channel < 30 && reverseTransform[channel] != -1 ) // player notes!
		{
			int track = reverseTransform[channel];
			if( holdStart[track] != -1 )
			{
				// this object is the end of the hold note.
				TapNote tn = nd.GetTapNote(track, holdStart[track]);
				tn.type = TapNoteType_HoldHead;
				tn.subType = TapNoteSubType_Hold;
				nd.AddHoldNote( track, holdStart[track], row, tn );
				holdStart[track] = -1;
				lastNote[track] = -1;
			}
			else if( obj.value == lnobj && lastNote[track] != -1 )
			{
				// this object is the end of the hold note.
				// lnobj: set last note to hold head.
				TapNote tn = nd.GetTapNote(track, lastNote[track]);
				tn.type = TapNoteType_HoldHead;
				tn.subType = TapNoteSubType_Hold;
				nd.AddHoldNote( track, lastNote[track], row, tn );
				holdStart[track] = -1;
				lastNote[track] = -1;
			}
			else
			{
				TapNote tn = TAP_ORIGINAL_TAP;
				tn.iKeysoundIndex = GetKeysound(obj);
				nd.SetTapNote( track, row, tn );
				if( hold ) holdStart[track] = row;
				lastNote[track] = row;
			}
		}
		else if( channel == 1 || (11 <= channel && channel <= 19) || (21 <= channel && channel <= 29) ) // auto-keysound and other notes
		{
			BMSAutoKeysound ak = { row, GetKeysound(obj) };
			autos.push_back( ak );
		}
	}

	int rowsToLook[3] = { 0, -1, 1 };
	for( unsigned i = 0; i < autos.size(); i ++ )
	{
		BMSAutoKeysound &ak = autos[i];
		bool found = false;
		for( int j = 0; j < 3; j ++ )
		{
			int row = ak.row + rowsToLook[j];
			for( int t = 0; t < tracks; t ++ )
			{
				if( nd.GetTapNote( t, row ) == TAP_EMPTY && !nd.IsHoldNoteAtRow( t, row ) )
				{
					TapNote tn = TAP_ORIGINAL_AUTO_KEYSOUND;
					tn.iKeysoundIndex = ak.index;
					nd.SetTapNote( t, row, tn );
					found = true;
					break;
				}
			}
			if( found ) break;
		}
	}

	delete[] transform;
	delete[] holdStart;
	delete[] lastNote;

	td.TidyUpData( false );
	out->SetNoteData(nd);
	out->m_Timing = td;
	out->TidyUpData();
	out->SetSavedToDisk( true );	// we're loading from disk, so this is by definintion already saved

	return true;
}

Steps *BMSChartReader::GetSteps()
{
	return out;
}

struct BMSStepsInfo {
	Steps *steps;
	BMSChartInfo info;
};

class BMSSongLoader
{
	RString dir;
	BMSSong song;
	std::vector<BMSStepsInfo> loadedSteps;
public:
	BMSSongLoader( RString songDir, Song *outSong );
	bool Load( RString fileName );
	void AddToSong();
};

BMSSongLoader::BMSSongLoader( RString songDir, Song *outSong ): dir(songDir), song(outSong)
{
}

bool BMSSongLoader::Load( RString fileName )
{
	// before doing anything else, load the chart first!
	BMSChart chart;
	if( !chart.Load( dir + fileName ) ) return false;

	// and then read the chart into the steps.
	Steps *steps = song.GetSong()->CreateSteps();
	steps->SetFilename( dir + fileName );

	BMSChartReader reader( &chart, steps, &song );
	if( !reader.Read() )
	{
		delete steps;
		return false;
	}

	// add it to our song
	song.GetSong()->AddSteps( steps );

	// add the chart reader instance to our list.
	BMSStepsInfo si = { steps, reader.info };
	loadedSteps.push_back(si);

	return true;

}

void BMSSongLoader::AddToSong()
{
    if( loadedSteps.size() == 0 )
    {
        return;
    }

	RString commonSubstring = "";

	{
		bool found = false;
		for( unsigned i = 0; i < loadedSteps.size(); i ++ )
		{
			if( loadedSteps[i].info.title == "" ) continue;
			if( !found )
			{
				commonSubstring = loadedSteps[i].info.title;
				found = true;
			}
			else
			{
				commonSubstring = FindLargestInitialSubstring( commonSubstring, loadedSteps[i].info.title );
			}
		}
		if( commonSubstring == "" )
		{
			// All bets are off; the titles don't match at all.
			// At this rate we're lucky if we even get the title right.
			LOG->UserLog( "Song", dir, "has BMS files with inconsistent titles." );
		}
	}

	if( commonSubstring == "" )
	{
		// As said before, all bets are off.
		// From here on in, it's nothing but guesswork.

		// Try to figure out the difficulty of each file.
		for( unsigned i = 0; i < loadedSteps.size(); i ++ )
		{
			Steps *steps = loadedSteps[i].steps;

			RString title = loadedSteps[i].info.title;

			// XXX: Is this really effective if Common Substring parsing failed?
			if( title != "" ) SearchForDifficulty( title, steps );
		}
	}
	else
	{
		// Now, with our fancy little substring, trim the titles and
		// figure out where each goes.
		for( unsigned i = 0; i < loadedSteps.size(); i ++ )
		{
			Steps *steps = loadedSteps[i].steps;

			RString title = loadedSteps[i].info.title;

			if( title != "" && title.size() != commonSubstring.size() )
			{
				RString tag = title.substr( commonSubstring.size(), title.size() - commonSubstring.size() );
				tag.MakeLower();

				// XXX: We should do this with filenames too, I have plenty of examples.
				// however, filenames will be trickier, as stuff at the beginning AND
				// end change per-file, so we'll need a fancier FindLargestInitialSubstring()

				// XXX: This matches (double), but I haven't seen it used. Again, MORE EXAMPLES NEEDED
				if( tag.find('l') != tag.npos )
				{
					unsigned pos = tag.find('l');
					if( pos > 2 && tag.substr(pos - 2, 4) == "solo" )
					{
						// (solo) -- an edit, apparently (Thanks Glenn!)
						steps->SetDifficulty( Difficulty_Edit );
					}
					else
					{
						// Any of [L7] [L14] (LIGHT7) (LIGHT14) (LIGHT) [L] <LIGHT7> <L7>... you get the idea.
						steps->SetDifficulty( Difficulty_Easy );
					}
				}
				// [x] [Expert]
				else if( tag.find('x') != tag.npos )
					steps->SetDifficulty( Difficulty_Challenge );
				// [A] <A> (A) [ANOTHER] <ANOTHER> (ANOTHER) (ANOTHER7) Another (DP ANOTHER) (Another) -ANOTHER- [A7] [A14] etc etc etc
				else if( tag.find('a') != tag.npos )
					steps->SetDifficulty( Difficulty_Hard );
				// XXX: Can also match (double), but should match [B] or [B7]
				else if( tag.find('b') != tag.npos )
					steps->SetDifficulty( Difficulty_Beginner );
				// Other tags I've seen here include (5KEYS) (10KEYS) (7keys) (14keys) (dp) [MIX] [14] (14 Keys Mix)
				// XXX: I'm sure [MIX] means something... anyone know?
			}
		}
	}

	/* Prefer to read global tags from a Difficulty_Medium file. These tend to
	 * have the least cruft in the #TITLE tag, so it's more likely to get a clean
	 * title. */
	int mainIndex = 0;
	for( unsigned i = 0; i < loadedSteps.size(); i ++ )
		if( loadedSteps[i].steps->GetDifficulty() == Difficulty_Medium )
			mainIndex = i;

	Song *out = song.GetSong();

	{
		const BMSStepsInfo &main = loadedSteps[mainIndex];

		out->m_sSongFileName = main.steps->GetFilename();
		if( main.info.title != "" )
			NotesLoader::GetMainAndSubTitlesFromFullTitle( main.info.title, out->m_sMainTitle, out->m_sSubTitle );
		out->m_sArtist = main.info.artist;
		out->m_sGenre = main.info.genre;
		out->m_sBannerFile = main.info.bannerFile;

		switch( main.steps->m_StepsType )
		{
			case StepsType_beat_single5:
			case StepsType_beat_single7:
			case StepsType_beat_double5:
			case StepsType_beat_double7:
			case StepsType_beat_versus5:
			case StepsType_beat_versus7:
				out->m_sBackgroundFile = main.info.stageFile;
				break;
			default:
				if ( main.info.backgroundFile != "" )
 					out->m_sBackgroundFile = main.info.backgroundFile;
 				else
 					out->m_sBackgroundFile = main.info.stageFile;
				break;
		}

		std::map<int, RString>::const_iterator it = main.info.backgroundChanges.begin();

		for (; it != main.info.backgroundChanges.end(); it++)
		{
			out->AddBackgroundChange(BACKGROUND_LAYER_1,
									 BackgroundChange(NoteRowToBeat(it->first),
													  it->second,
													  "",
													  1.f,
													  it->second.substr(it->second.length()-4)==".lua"?SBE_Centered:SBE_StretchNoLoop));
		}

		out->m_sMusicFile = main.info.musicFile;

		// Preview file only if it's different from one specified on #MUSIC so that previewStart is valid. -az
		if (main.info.previewFile.length() && main.info.previewFile != main.info.musicFile)
		{
			out->m_PreviewFile = main.info.previewFile;
			out->m_fMusicSampleLengthSeconds = 0.00f; // to ensure whole preview file is heard
		}

		out->m_fMusicSampleStartSeconds = main.info.previewStart;
		out->m_SongTiming = main.steps->m_Timing;
	}

	// The brackets before the difficulty are in common substring, so remove them if it's found.
	if( commonSubstring.size() > 2 && commonSubstring[commonSubstring.size() - 2] == ' ' )
	{
		switch( commonSubstring[commonSubstring.size() - 1] )
		{
		case '[':
		case '(':
		case '<':
			commonSubstring = commonSubstring.substr(0, commonSubstring.size() - 2);
		}
	}

	// Override what that global tag said about the title if we have a good substring.
	// Prevents clobbering and catches "MySong (7keys)" / "MySong (Another) (7keys)"
	// Also catches "MySong (7keys)" / "MySong (14keys)"
	if( commonSubstring != "" )
		NotesLoader::GetMainAndSubTitlesFromFullTitle( commonSubstring, out->m_sMainTitle, out->m_sSubTitle );

	SlideDuplicateDifficulties( *out );

	ConvertString( out->m_sMainTitle, "utf-8,japanese" );
	ConvertString( out->m_sArtist, "utf-8,japanese" );
	ConvertString( out->m_sGenre, "utf-8,japanese" );

}

/*===========================================================================*/

bool BMSLoader::LoadNoteDataFromSimfile( const RString & cachePath, Steps & out )
{
	Song *pSong = out.m_pSong;

	// before doing anything else, load the chart first!
	BMSChart chart;
	if( !chart.Load( cachePath ) ) return false;

	BMSSong song(pSong);

	BMSChartReader reader( &chart, &out, &song );
	if( !reader.Read() ) return false;

	return true;
}

bool BMSLoader::LoadFromDir( const RString &sDir, Song &out )
{
	LOG->Trace( "Song::LoadFromBMSDir(%s)", sDir.c_str() );

	ASSERT( out.m_vsKeysoundFile.empty() );

	std::vector<RString> arrayBMSFileNames;
	GetApplicableFiles( sDir, arrayBMSFileNames );

	/* We should have at least one; if we had none, we shouldn't have been
	 * called to begin with. */
	ASSERT( arrayBMSFileNames.size() != 0 );

	BMSSongLoader loader( sDir, &out );
	for( unsigned i=0; i<arrayBMSFileNames.size(); i++ )
	{
		loader.Load( arrayBMSFileNames[i] );
	}
	loader.AddToSong();

	return true;

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
