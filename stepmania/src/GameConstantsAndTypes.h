#pragma once
/*
-----------------------------------------------------------------------------
 File: GameConstantsAndTypes.h

 Desc: Things that are used in many places and don't change often.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "D3DX8Math.h"	// for D3DXCOLOR


/////////////////////////////
// Screen Dimensions
/////////////////////////////
#define	SCREEN_WIDTH	(640)
#define	SCREEN_HEIGHT	(480)

#define	SCREEN_LEFT		(0)
#define	SCREEN_RIGHT	(SCREEN_WIDTH)
#define	SCREEN_TOP		(0)
#define	SCREEN_BOTTOM	(SCREEN_HEIGHT)

#define	CENTER_X		(SCREEN_LEFT + (SCREEN_RIGHT - SCREEN_LEFT)/2.0f)
#define	CENTER_Y		(SCREEN_TOP + (SCREEN_BOTTOM - SCREEN_TOP)/2.0f)

/////////////////////////
// Note definitions
/////////////////////////
typedef unsigned char TapNote;

enum 
{
	TRACK_1 = 0,
	TRACK_2,
	TRACK_3,
	TRACK_4,
	TRACK_5,
	TRACK_6,
	TRACK_7,
	TRACK_8,
	TRACK_9,
	TRACK_10,
	TRACK_11,
	TRACK_12,
	TRACK_13,
	MAX_NOTE_TRACKS		// leave this at the end
};

const int MAX_BEATS			= 1500;	// BMR's Pulse has about 1300
const int BEATS_PER_MEASURE = 4;
const int MAX_MEASURES		= MAX_BEATS / BEATS_PER_MEASURE;

const int ELEMENTS_PER_BEAT	= 12;	// It is important that this number is evenly divisible by 2, 3, and 4.
const int ELEMENTS_PER_MEASURE = ELEMENTS_PER_BEAT * BEATS_PER_MEASURE;
const int MAX_TAP_NOTE_ROWS = MAX_BEATS*ELEMENTS_PER_BEAT;

const int MAX_HOLD_NOTES = 400;	// BMR's Connected has about 300

enum NoteType 
{ 
	NOTE_4TH,	// quarter notes
	NOTE_8TH,	// eighth notes
	NOTE_12TH,	// triplets
	NOTE_16TH,	// sixteenth notes
	NUM_NOTE_TYPES,
	NOTE_INVALID
};

D3DXCOLOR NoteTypeToColor( NoteType nt );

float NoteTypeToBeat( NoteType nt );

NoteType GetNoteType( int iNoteIndex );

bool IsNoteOfType( int iNoteIndex, NoteType t );

D3DXCOLOR GetNoteColorFromIndex( int iStepIndex );




enum RadarCategory	// starting from 12-o'clock rotating clockwise
{
	RADAR_STREAM = 0,
	RADAR_VOLTAGE,
	RADAR_AIR,
	RADAR_FREEZE,
	RADAR_CHAOS,
	NUM_RADAR_VALUES	// leave this at the end
};

enum DifficultyClass 
{ 
	CLASS_EASY,		// corresponds to Basic
	CLASS_MEDIUM,	// corresponds to Trick, Another, Standard
	CLASS_HARD,		// corresponds to Maniac, SSR, Heavy
	NUM_DIFFICULTY_CLASSES,
	CLASS_INVALID
};

D3DXCOLOR DifficultyClassToColor( DifficultyClass dc );

CString DifficultyClassToString( DifficultyClass dc );

DifficultyClass StringToDifficultyClass( CString sDC );


enum NotesType
{
	NOTES_TYPE_DANCE_SINGLE = 0,
	NOTES_TYPE_DANCE_DOUBLE,
	NOTES_TYPE_DANCE_COUPLE,
	NOTES_TYPE_DANCE_SOLO,
	NOTES_TYPE_PUMP_SINGLE,
	NOTES_TYPE_PUMP_DOUBLE,
	NOTES_TYPE_EZ2_SINGLE,
	NOTES_TYPE_EZ2_SINGLE_HARD,
	NOTES_TYPE_EZ2_DOUBLE,
	NOTES_TYPE_EZ2_REAL,
	NOTES_TYPE_EZ2_SINGLE_VERSUS,
	NOTES_TYPE_EZ2_SINGLE_HARD_VERSUS,
	NOTES_TYPE_EZ2_REAL_VERSUS,
	NUM_NOTES_TYPES,		// leave this at the end
	NOTES_TYPE_INVALID,
};

int NotesTypeToNumTracks( NotesType nt );

NotesType StringToNotesType( CString sNotesType );

CString NotesTypeToString( NotesType nt );


//////////////////////////
// Play mode stuff
//////////////////////////
enum PlayMode
{
	PLAY_MODE_ARCADE,
	PLAY_MODE_ONI,
	PLAY_MODE_ENDLESS,
	NUM_PLAY_MODES,
	PLAY_MODE_INVALID
};

enum PlayerNumber {
	PLAYER_1 = 0,
	PLAYER_2,
	NUM_PLAYERS,	// leave this at the end
	PLAYER_INVALID
};

D3DXCOLOR PlayerToColor( const PlayerNumber p );
D3DXCOLOR PlayerToColor( int p );


enum SongSortOrder { 
	SORT_GROUP, 
	SORT_TITLE, 
	SORT_BPM, 
	SORT_MOST_PLAYED, 
	NUM_SORT_ORDERS 
};


///////////////////////////////
// Game/Style definition stuff
///////////////////////////////
enum Game
{
	GAME_DANCE, // Dance Dance Revolution
	GAME_PUMP, // Pump It Up
	GAME_EZ2, // Ez2dancer
	NUM_GAMES,	// leave this at the end
	GAME_INVALID,
};

enum Style
{
	STYLE_DANCE_SINGLE,
	STYLE_DANCE_VERSUS,
	STYLE_DANCE_DOUBLE,
	STYLE_DANCE_COUPLE,
	STYLE_DANCE_SOLO,
	STYLE_PUMP_SINGLE,
	STYLE_PUMP_VERSUS,
	STYLE_PUMP_DOUBLE,
	STYLE_EZ2_SINGLE,
	STYLE_EZ2_SINGLE_HARD,
	STYLE_EZ2_DOUBLE,
	STYLE_EZ2_REAL,
	STYLE_EZ2_SINGLE_VERSUS,
	STYLE_EZ2_SINGLE_HARD_VERSUS,
	STYLE_EZ2_REAL_VERSUS,
	NUM_STYLES,	// leave this at the end
	STYLE_NONE,
};

Game StyleToGame( Style s );

/*

	Chris:
	
	Very cool system :-)  
	
	However, the NotesType is a property of the StyleDef, so we can look it up there.
	Get the the NotesType for a style with:
		GAMESTATE->GetCurrentStyleDef()->m_NotesType		- or -
		GAMEMAN->GetStyleDefForStyle(style)->m_NotesType
	I'll add a new method to GAMEMAN called GetStyleThatPlaysNotesType():
		Style s = GAMEMAN->GetStyleThatPlaysNotesType( nt );	

////////////////////////////////
// NotesType/Style conversions
////////////////////////////////

// Dro Kulix: This part really is necessary.
// Several subroutines call for conversions between
// NotesType and Style, both ways. It would make
// things a bit easier to keep them central.

// The next two functions have self-explanatory
// names, NotesTypeToStyle(nt) and
// StyleToNotesType(s).

// Both of these functions involve a tedious
// switch-based one-to-one mapping between
// NotesTypes and Styles. I have written a
// Perl script which will actually write both
// of the functions for you. All you need to do
// is keep updated the list of Styles and
// NotesTypes near the beginning of the script.

// If you need a copy of Perl for Win32,
// http://www.activeperl.com/Products/ActivePerl/
// will suit nicely.

/*

Begin Perl file ->

# StyleNotesType.pl

# Perl script to write StyleToNotesType and
# NotesTypeToStyle functions for StepMania.

# Usage: perl StyleNotesType.pl > code.txt
# (outputs C++ functions to file code.txt)

# Keep the following list updated to produce
# correct code.

# First column is Styles, second is NotesTypes.

# These mapping sequences only make the first possible
# mapping. That is, STYLE_DANCE_SINGLE will map to
# NOTES_TYPE_DANCE_SINGLE, and STYLE_DANCE_VERSUS will
# map to NOTES_TYPE_DANCE_SINGLE, but
# NOTES_TYPE_DANCE_SINGLE will only map to
# STYLE_DANCE_SINGLE.

my @Types = qw[
	DANCE_SINGLE		DANCE_SINGLE
	DANCE_VERSUS		DANCE_SINGLE
	DANCE_DOUBLE		DANCE_DOUBLE
	DANCE_COUPLE		DANCE_COUPLE
	DANCE_SOLO		DANCE_SOLO
	PUMP_SINGLE		PUMP_SINGLE
	PUMP_VERSUS		PUMP_SINGLE
	PUMP_DOUBLE		PUMP_DOUBLE
	EZ2_SINGLE		EZ2_SINGLE
	EZ2_DOUBLE		EZ2_DOUBLE
	EZ2_REAL		EZ2_REAL
	EZ2_SINGLE_VERSUS	EZ2_SINGLE_VERSUS
	EZ2_REAL_VERSUS		EZ2_REAL_VERSUS
	NONE			INVALID
];



my %StyleToNotesType = ();
{
	my @tmpTypes = @Types;
	# Map Style to NotesType
	while (@tmpTypes) {
		my $key = shift(@tmpTypes);
		my $value = shift(@tmpTypes);
		if (!exists($StyleToNotesType{$key})) {
			$StyleToNotesType{$key} = $value;
		}
	}
}

my %NotesTypeToStyle = ();
{
	my @tmpTypes = @Types;
	# Map NotesType to Style
	while (@tmpTypes) {
		my $value = shift(@tmpTypes);
		my $key = shift(@tmpTypes);
		if (!exists($NotesTypeToStyle{$key})) {
			$NotesTypeToStyle{$key} = $value;
		}
	}
}

# Produce NotesTypeToStyle
print "
//
// NotesTypeToStyle(nt): Converts nt to a Style
//
inline Style NotesTypeToStyle ( NotesType nt )
{
	switch ( nt )
	{
";

foreach (sort keys %NotesTypeToStyle) {
	my $key = "NOTES_TYPE_$_";
	my $value = 'STYLE_' . $NotesTypeToStyle{$_};
	print "\t\tcase $key:\treturn $value;\n";
}

print "\t\tdefault:\treturn STYLE_NONE;
	}
}
";

# Produce StyleToNotesType
print "
//
// StyleToNotesType(s): Converts s to a NotesType
//
inline NotesType StyleToNotesType ( Style s )
{
	switch ( s )
	{
";

foreach (sort keys %StyleToNotesType) {
	my $key = "STYLE_$_";
	my $value = 'NOTES_TYPE_' . $StyleToNotesType{$_};
	print "\t\tcase $key:\treturn $value;\n";
}

print "\t\tdefault:\treturn NOTES_TYPE_INVALID;
	}
}
";

# Finished.

<- End of Perl file
*/

/*
//
// NotesTypeToStyle(nt): Converts nt to a Style
//
inline Style NotesTypeToStyle ( NotesType nt )
{
	switch ( nt )
	{
		case NOTES_TYPE_DANCE_COUPLE:	return STYLE_DANCE_COUPLE;
		case NOTES_TYPE_DANCE_DOUBLE:	return STYLE_DANCE_DOUBLE;
		case NOTES_TYPE_DANCE_SINGLE:	return STYLE_DANCE_SINGLE;
		case NOTES_TYPE_DANCE_SOLO:	return STYLE_DANCE_SOLO;
		case NOTES_TYPE_EZ2_DOUBLE:	return STYLE_EZ2_DOUBLE;
		case NOTES_TYPE_EZ2_REAL:	return STYLE_EZ2_REAL;
		case NOTES_TYPE_EZ2_REAL_VERSUS:	return STYLE_EZ2_REAL_VERSUS;
		case NOTES_TYPE_EZ2_SINGLE:	return STYLE_EZ2_SINGLE;
		case NOTES_TYPE_EZ2_SINGLE_HARD:	return STYLE_EZ2_SINGLE_HARD;
		case NOTES_TYPE_EZ2_SINGLE_VERSUS:	return STYLE_EZ2_SINGLE_VERSUS;
		case NOTES_TYPE_EZ2_SINGLE_HARD_VERSUS:	return STYLE_EZ2_SINGLE_HARD_VERSUS;
		case NOTES_TYPE_INVALID:	return STYLE_NONE;
		case NOTES_TYPE_PUMP_DOUBLE:	return STYLE_PUMP_DOUBLE;
		case NOTES_TYPE_PUMP_SINGLE:	return STYLE_PUMP_SINGLE;
		default:	return STYLE_NONE;
	}
}

//
// StyleToNotesType(s): Converts s to a NotesType
//
inline NotesType StyleToNotesType ( Style s )
{
	switch ( s )
	{
		case STYLE_DANCE_COUPLE:	return NOTES_TYPE_DANCE_COUPLE;
		case STYLE_DANCE_DOUBLE:	return NOTES_TYPE_DANCE_DOUBLE;
		case STYLE_DANCE_SINGLE:	return NOTES_TYPE_DANCE_SINGLE;
		case STYLE_DANCE_SOLO:	return NOTES_TYPE_DANCE_SOLO;
		case STYLE_DANCE_VERSUS:	return NOTES_TYPE_DANCE_SINGLE;
		case STYLE_EZ2_DOUBLE:	return NOTES_TYPE_EZ2_DOUBLE;
		case STYLE_EZ2_REAL:	return NOTES_TYPE_EZ2_REAL;
		case STYLE_EZ2_REAL_VERSUS:	return NOTES_TYPE_EZ2_REAL_VERSUS;
		case STYLE_EZ2_SINGLE:	return NOTES_TYPE_EZ2_SINGLE;
		case STYLE_EZ2_SINGLE_HARD:	return NOTES_TYPE_EZ2_SINGLE_HARD;
		case STYLE_EZ2_SINGLE_VERSUS:	return NOTES_TYPE_EZ2_SINGLE_VERSUS;
		case STYLE_EZ2_SINGLE_HARD_VERSUS:	return NOTES_TYPE_EZ2_SINGLE_HARD_VERSUS;
		case STYLE_NONE:	return NOTES_TYPE_INVALID;
		case STYLE_PUMP_DOUBLE:	return NOTES_TYPE_PUMP_DOUBLE;
		case STYLE_PUMP_SINGLE:	return NOTES_TYPE_PUMP_SINGLE;
		case STYLE_PUMP_VERSUS:	return NOTES_TYPE_PUMP_SINGLE;
		default:	return NOTES_TYPE_INVALID;
	}
}

*/


///////////////////////////
// Scoring stuff
///////////////////////////

enum TapNoteScore { 
	TNS_NONE, 
	TNS_MISS,
	TNS_BOO,
	TNS_GOOD,
	TNS_GREAT,
	TNS_PERFECT,
	NUM_TAP_NOTE_SCORES
};

inline int TapNoteScoreToDancePoints( TapNoteScore tns )
{
	switch( tns )
	{
	case TNS_PERFECT:	return +2;
	case TNS_GREAT:		return +1;
	case TNS_GOOD:		return +0;
	case TNS_BOO:		return -4;
	case TNS_MISS:		return -8;
	case TNS_NONE:		return 0;
	default:	ASSERT(0);	return 0;
	}
}

//enum TapNoteTiming { 
//	TNT_NONE, 
//	TNT_EARLY, 
//	TNT_LATE 
//};


enum HoldNoteScore 
{ 
	HNS_NONE,	// this HoldNote has not been scored yet
	HNS_OK,		// the HoldNote has passed and was successfully held all the way through
	HNS_NG,		// the HoldNote has passed and they missed it
	NUM_HOLD_NOTE_SCORES
};


inline int HoldNoteScoreToDancePoints( HoldNoteScore hns )
{
	switch( hns )
	{
	case HNS_OK:	return +6;
	case HNS_NG:	return +0;
	default:	ASSERT(0);	return 0;
	}
}
