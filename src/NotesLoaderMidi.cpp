/*
 * Header/track parsing adapted from STK's FileInMidi: http://ccrma.stanford.edu/software/stk/
 * STK license documentation is below near the MidiFileIn class.  ("If you make compositions with it, put us in the program notes")

 * MIDI event info is explained here: http://www.sonicspot.com/guide/midifiles.html

 * Explanation of MIDI "running status" and note-on velocity==0 meaning: http://www.borg.com/~jglatt/tech/midispec/run.htm

 * Guitar track mappings are explained here: http://www.scorehero.com/forum/viewtopic.php?t=1179

 * STK license documentation:
 * 
 *  On April 19 2007, StepMania was granted an MIT license by Gary Scavone for any STK code on the condition that all of the the STK credits/disclaimers/copyrights from the README are retained in the source code and in the project documentation.
 *
 *  CREDITS:
 *  By Perry R. Cook and Gary P. Scavone, 1995-2005.
 *
 *  DISCLAIMER:
 *  You probably already guessed this, but just to be sure, we don't guarantee anything works.  :-)  It's free ... what do you expect?  If you find a bug, please let us know and we'll try to correct it.  You can also make suggestions, but again, no guarantees.  Send email to the mail list.
 *
 *  LEGAL AND ETHICAL:
 *  This software was designed and created to be made publicly available for free, primarily for academic purposes, so if you use it, pass it on with this documentation, and for free.  
 *  If you make a million dollars with it, give us some.  If you make compositions with it, put us in the program notes.
 *  Some of the concepts are covered by various patents, some known to us and likely others which are unknown.  Many of the ones known to us are administered by the Stanford Office of Technology and Licensing.  
 *  The good news is that large hunks of the techniques used here are public domain.  To avoid subtle legal issues, we'll not state what's freely useable here, but we'll try to note within the various classes where certain things are likely to be protected by patents.
 *
 *  FURTHER READING:
 *  For complete documentation on this ToolKit, the classes, etc., see the doc directory of the distribution or surf to http://ccrma.stanford.edu/software/stk/.  Also check the platform specific README's for specific system requirements.
 */

#include "global.h"
#include "NotesLoaderMidi.h"
#include "Song.h"
#include "Steps.h"
#include "RageLog.h"
#include "NoteData.h"
#include "IniFile.h"
#include "RageFile.h"

/**********************************************************************/
/*! \class MidiFileIn
\brief A standard MIDI file reading/parsing class.

This class can be used to read events from a standard MIDI file.
Event bytes are copied to a C++ vector and must be subsequently
interpreted by the user.  The function getNextMidiEvent() skips
meta and sysex events, returning only MIDI channel messages.
Event delta-times are returned in the form of "ticks" and a
function is provided to determine the current "seconds per tick".
Tempo changes are internally tracked by the class and reflected in
the values returned by the function getTickSeconds().

by Gary P. Scavone, 2003.
*/
/**********************************************************************/
class MidiFileIn
{
public:
	//! Default constructor.
	/*!
	If an error occurs while opening or parsing the file header, an
	StkError exception will be thrown.
	*/
	MidiFileIn() {}
	void Load( RString fileName );

	//! Class destructor.
	~MidiFileIn();

	MidiFileIn( const MidiFileIn &other ) {}

	//! Return the MIDI file format (0, 1, or 2).
	int getFileFormat() const;

	//! Return the number of tracks in the MIDI file.
	unsigned int getNumberOfTracks() const;

	//! Return the MIDI file division value from the file header.
	/*!
	Note that this value must be "parsed" in accordance with the
	MIDI File Specification.  In particular, if the MSB is set, the
	file uses time-code representations for delta-time values.
	*/
	int getDivision() const;

	//! Move the specified track event reader to the beginning of its track.
	/*!
	The relevant track tempo value is reset as well.  If an invalid
	track number is specified, an StkError exception will be thrown.
	*/
	void rewindTrack( unsigned int track = 0 );

	//! Get the current value, in seconds, of delta-time ticks for the specified track.
	/*!
	This value can change as events are read (via "Set Tempo"
	Meta-Events).  Therefore, one should call this function after
	every call to getNextEvent() or getNextMidiEvent().  If an
	invalid track number is specified, an StkError exception will be
	thrown.
	*/   
	double getTickSeconds( unsigned int track = 0 );

	//! Fill the user-provided vector with the next event in the specified track and return the event delta-time in ticks.
	/*!
	MIDI File events consist of a delta time and a sequence of event
	bytes.  This function returns the delta-time value and writes
	the subsequent event bytes directly to the event vector.  The
	user must parse the event bytes in accordance with the MIDI File
	Specification.  All returned MIDI channel events are complete
	... a status byte is provided even when running status is used
	in the file.  If the track has reached its end, no bytes will be
	written and the event vector size will be zero.  If an invalid
	track number is specified or an error occurs while reading the
	file, an StkError exception will be thrown.
	*/
	unsigned long getNextEvent( std::vector<unsigned char> *event, unsigned int track = 0 );

	//! Fill the user-provided vector with the next MIDI channel event in the specified track and return the event delta time in ticks.
	/*!
	All returned MIDI events are complete ... a status byte is
	provided even when running status is used in the file.  Meta and
	sysex events in the track are skipped though "Set Tempo" events
	are properly parsed for use by the getTickSeconds() function.
	If the track has reached its end, no bytes will be written and
	the event vector size will be zero.  If an invalid track number
	is specified or an error occurs while reading the file, an
	StkError exception will be thrown.
	*/
	unsigned long getNextMidiEvent( std::vector<unsigned char> *midiEvent, unsigned int track = 0 );


	// This structure and the following variables are used to save and
	// keep track of a format 1 tempo map (and the initial tickSeconds
	// parameter for formats 0 and 2).
	struct TempoChange { 
		unsigned long count;
		double tickSeconds;
	};
	std::vector<TempoChange> tempoEvents_;
	struct TimeSignatureChange { 
		unsigned long count;
		unsigned long numerator;
		unsigned long denominator;
	};
	std::vector<TimeSignatureChange> timeSignatureEvents_;
	std::vector<unsigned long> trackCounters_;
	std::vector<unsigned int> trackTempoIndex_;

protected:

	// This protected class function is used for reading variable-length
	// MIDI file values. It is assumed that this function is called with
	// the file read pointer positioned at the start of a
	// variable-length value.  The function returns true if the value is
	// successfully parsed.  Otherwise, it returns false.
	bool readVariableLength( unsigned long *value );

	RageFile file_;
	unsigned int nTracks_;
	int format_;
	int division_;
	bool usingTimeCode_;
	std::vector<double> tickSeconds_;
	std::vector<long> trackPointers_;
	std::vector<long> trackOffsets_;
	std::vector<long> trackLengths_;
	std::vector<char> trackStatus_;

};



void MidiFileIn :: Load( RString fileName )
{
	// Attempt to open the file.
	if( !file_.Open(fileName) )
	{
		FAIL_M( "MidiFileIn: error opening or finding file (" +  fileName + ")." );
	}

	// Parse header info.
	char chunkType[4];
	int32_t length;
	if( !file_.Read( chunkType, 4 ) ) 
		FAIL_M( "NotesLoaderMidi" );
	if( !file_.Read( &length, 4 ) ) 
		FAIL_M( "NotesLoaderMidi" );
	length = Swap32BE( length );
	if( strncmp( chunkType, "MThd", 4 ) || ( length != 6 ) ) 
	{
		FAIL_M( "MidiFileIn: file (" + fileName + ") does not appear to be a MIDI file!" );
	}

	// Read the MIDI file format.
	int16_t data;
	if( !file_.Read( &data, 2 ) ) 
		FAIL_M( "NotesLoaderMidi" );
	data = Swap16BE( data );
	if( data < 0 || data > 2 ) 
	{
		FAIL_M( "MidiFileIn: the file (" + fileName + ") format is invalid!" );
	}
	format_ = data;

	// Read the number of tracks.
	if( !file_.Read( &data, 2 ) ) 
		FAIL_M( "NotesLoaderMidi" );
	data = Swap16BE( data );
	if( format_ == 0 && data != 1 )
	{
		FAIL_M( "MidiFileIn: invalid number of tracks (>1) for a file format = 0!" );
	}
	nTracks_ = data;

	// Read the beat division.
	if( !file_.Read( &data, 2 ) ) 
		FAIL_M( "NotesLoaderMidi" );
	data = Swap16BE( data );
	division_ = (int)data;
	double tickrate;
	usingTimeCode_ = false;
	if( data & 0x8000 ) 
	{
		// Determine ticks per second from time-code formats.
		tickrate = (double) -(data & 0x7F00);
		// If frames per second value is 29, it really should be 29.97.
		if( tickrate == 29.0 ) 
			tickrate = 29.97;
		tickrate *= (data & 0x00FF);
		usingTimeCode_ = true;
	}
	else 
	{
		tickrate = (double) (data & 0x7FFF); // ticks per quarter note
	}

	// Now locate the track offsets and lengths.  If not using time
	// code, we can initialize the "tick time" using a default tempo of
	// 120 beats per minute.  We will then check for tempo meta-events
	// afterward.
	for( unsigned int i=0; i<nTracks_; i++ ) 
	{
		if( !file_.Read( chunkType, 4 ) ) 
			FAIL_M( "NotesLoaderMidi" );
		if( strncmp( chunkType, "MTrk", 4 ) ) 
			FAIL_M( "NotesLoaderMidi" );
		if( !file_.Read( &length, 4 ) ) 
			FAIL_M( "NotesLoaderMidi" );
		length = Swap32BE( length );
		trackLengths_.push_back( length );
		trackOffsets_.push_back( (long) file_.Tell() );
		trackPointers_.push_back( (long) file_.Tell() );
		trackStatus_.push_back( 0 );
		file_.Seek( length + file_.Tell() );
		if( usingTimeCode_ ) 
			tickSeconds_.push_back( (double) (1.0 / tickrate) );
		else 
			tickSeconds_.push_back( (double) (0.5 / tickrate) );
	}

	// Save the initial tickSeconds parameter.
	{
		TempoChange tempoEvent;
		tempoEvent.count = 0;
		tempoEvent.tickSeconds = tickSeconds_[0];
		tempoEvents_.push_back( tempoEvent );
	}

	// Save the initial tickSeconds parameter.
	{
		TimeSignatureChange timeSignatureEvent;
		timeSignatureEvent.count = 0;
		timeSignatureEvent.numerator = 4;
		timeSignatureEvent.denominator = 4;
		timeSignatureEvents_.push_back( timeSignatureEvent );
	}

	// If format 1 and not using time code, parse and save the tempo map
	// on track 0.
	if( format_ == 1 && !usingTimeCode_ ) 
	{
		// We need to temporarily change the usingTimeCode_ value here so
		// that the getNextEvent() function doesn't try to check the tempo
		// map (which we're creating here).
		usingTimeCode_ = true;

		std::vector<unsigned char> event;
		unsigned long count = getNextEvent( &event, 0 );
		while( event.size() )
		{
			if( ( event.size() == 6 ) && ( event[0] == 0xff ) &&
				( event[1] == 0x51 ) && ( event[2] == 0x03 ) )
			{
				TempoChange tempoEvent;
				tempoEvent.count = count;
				unsigned long value = ( event[3] << 16 ) + ( event[4] << 8 ) + event[5];
				tempoEvent.tickSeconds = (double) (0.000001 * value / tickrate);
				if( count > tempoEvents_.back().count )
					tempoEvents_.push_back( tempoEvent );
				else
					tempoEvents_.back() = tempoEvent;
			}

			if( ( event.size() == 7 ) && ( event[0] == 0xff ) &&
				( event[1] == 0x58 ) && ( event[2] == 0x04 ) )
			{
				TimeSignatureChange timeSignatureEvent;
				timeSignatureEvent.count = count;
				timeSignatureEvent.numerator = event[3];
				timeSignatureEvent.denominator = (int)pow( 2.0f, (int)event[4] );
				//unsigned long metronome = event[5];	// not currently used
				//unsigned long u32nds = event[6];	// not currently used
				if( count > timeSignatureEvents_.back().count )
					timeSignatureEvents_.push_back( timeSignatureEvent );
				else
					timeSignatureEvents_.back() = timeSignatureEvent;
			}

			count += getNextEvent( &event, 0 );
		}
		rewindTrack( 0 );
		for( unsigned int i=0; i<nTracks_; i++ )
		{
			trackCounters_.push_back( 0 );
			trackTempoIndex_.push_back( 0 );
		}
		// Change the time code flag back!
		usingTimeCode_ = false;
	}

	return;
}

MidiFileIn :: ~MidiFileIn()
{
	// An ifstream object implicitly closes itself during destruction
	// but we'll make an explicit call to "close" anyway.
	file_.Close(); 
}

int MidiFileIn :: getFileFormat() const
{
	return format_;
}

unsigned int MidiFileIn :: getNumberOfTracks() const
{
	return nTracks_;
}

int MidiFileIn :: getDivision() const
{
	return division_;
}

void MidiFileIn :: rewindTrack( unsigned int track )
{
	if( track >= nTracks_ )
	{
		FAIL_M( ssprintf("MidiFileIn::getNextEvent: invalid track argument (%u).", track) );
	}

	trackPointers_[track] = trackOffsets_[track];
	trackStatus_[track] = 0;
	tickSeconds_[track] = tempoEvents_[0].tickSeconds;
}

double MidiFileIn :: getTickSeconds( unsigned int track )
{
	// Return the current tick value in seconds for the given track.
	if( track >= nTracks_ )
	{
		FAIL_M( ssprintf("MidiFileIn::getTickSeconds: invalid track argument (%u).", track) );
	}

	return tickSeconds_[track];
}

unsigned long MidiFileIn :: getNextEvent( std::vector<unsigned char> *event, unsigned int track )
{
	// Fill the user-provided vector with the next event in the
	// specified track (default = 0) and return the event delta time in
	// ticks.  This function assumes that the stored track pointer is
	// positioned at the start of a track event.  If the track has
	// reached its end, the event vector size will be zero.
	//
	// If we have a format 0 or 2 file and we're not using timecode, we
	// should check every meta-event for tempo changes and make
	// appropriate updates to the tickSeconds_ parameter if so.
	//
	// If we have a format 1 file and we're not using timecode, keep a
	// running sum of ticks for each track and update the tickSeconds_
	// parameter as needed based on the stored tempo map.

	if( track >= nTracks_ ) 
	{
		FAIL_M( ssprintf("MidiFileIn::getNextEvent: invalid track argument (%u).", track) );
	}

	event->clear();
	// Check for the end of the track.
	if( (trackPointers_[track] - trackOffsets_[track]) >= trackLengths_[track] )
		return 0;

	unsigned long ticks = 0;
	unsigned long bytes = 0;
	bool isTempoEvent = false;

	// Read the event delta time.
	file_.Seek( trackPointers_[track] );
	if( !readVariableLength( &ticks ) ) 
		FAIL_M( "NotesLoaderMidi" );

	// Parse the event stream to determine the event length.
	unsigned char c;
	if( !file_.Read( (char *)&c, 1 ) ) 
		FAIL_M( "NotesLoaderMidi" );
	switch( c ) 
	{
		case 0xFF: // A Meta-Event
			unsigned long position;
			
			// RealTime Category messages (ie, Status of 0xF8 to 0xFF) do not affect running status
			
			event->push_back( c );
			if( !file_.Read( (char *)&c, 1 ) ) 
				FAIL_M( "NotesLoaderMidi" );
			event->push_back( c );
			if( format_ != 1 && ( c == 0x51 ) ) 
				isTempoEvent = true;
			position = file_.Tell();
			if( !readVariableLength( &bytes ) ) 
				FAIL_M( "NotesLoaderMidi" );
			bytes += ( (unsigned long)file_.Tell() - position );
			file_.Seek( position );
			break;

		case 0xF0:
		case 0xF7: // The start or continuation of a Sysex event
			trackStatus_[track] = 0;
			event->push_back( c );
			position = file_.Tell();
			if( !readVariableLength( &bytes ) ) 
				FAIL_M( "NotesLoaderMidi" );
			bytes += ( (unsigned long)file_.Tell() - position );
			file_.Seek( position );
			break;

		default: // Should be a MIDI channel event
			if( c & 0x80 )  // MIDI status byte
			{
				if( c > 0xF0 ) 
					FAIL_M( "NotesLoaderMidi" );
				trackStatus_[track] = c;
				event->push_back( c );
				c &= 0xF0;
				if( (c == 0xC0) || (c == 0xD0) ) 
					bytes = 1;
				else 
					bytes = 2;
			}
			else if( trackStatus_[track] & 0x80 )  // Running status
			{
				event->push_back( trackStatus_[track] );
				event->push_back( c );
				c = trackStatus_[track] & 0xF0;
				if( (c != 0xC0) && (c != 0xD0) ) 
					bytes = 1;
			}
			else 
			{
				FAIL_M( "NotesLoaderMidi" );
			}
			break;

	}

	// Read the rest of the event into the event vector.
	for( unsigned long i=0; i<bytes; i++ ) 
	{
		if( !file_.Read( (char *)&c, 1 ) ) 
			FAIL_M( "NotesLoaderMidi" );
		event->push_back( c );
	}

	if( !usingTimeCode_ ) 
	{
		if( isTempoEvent ) 
		{
			// Parse the tempo event and update tickSeconds_[track].
			double tickrate = (double) (division_ & 0x7FFF);
			unsigned long value = ( event->at(3) << 16 ) + ( event->at(4) << 8 ) + event->at(5);
			tickSeconds_[track] = (double) (0.000001 * value / tickrate);
		}

		if( format_ == 1 ) 
		{
			// Update track counter and check the tempo map.
			trackCounters_[track] += ticks;
			TempoChange tempoEvent = tempoEvents_[ trackTempoIndex_[track] ];
			if( trackCounters_[track] >= tempoEvent.count ) 
			{
				if( trackTempoIndex_[track] < tempoEvents_.size()-1 )
					trackTempoIndex_[track]++;
				tickSeconds_[track] = tempoEvent.tickSeconds;
			}
		}
	}

	// Save the current track pointer value.
	trackPointers_[track] = file_.Tell();

	return ticks;
}

unsigned long MidiFileIn :: getNextMidiEvent( std::vector<unsigned char> *midiEvent, unsigned int track )
{
	// Fill the user-provided vector with the next MIDI event in the
	// specified track (default = 0) and return the event delta time in
	// ticks.  Meta-Events preceeding this event are skipped and ignored.
	if( track >= nTracks_ ) 
	{
		FAIL_M( ssprintf("MidiFileIn::getNextMidiEvent: invalid track argument (%u).", track) );
	}

	unsigned long ticks = getNextEvent( midiEvent, track );
	while( midiEvent->size() && ( midiEvent->at(0) >= 0xF0 ) ) 
	{
		//for( unsigned int i=0; i<midiEvent->size(); i++ )
		//std::cout << "event byte = " << i << ", value = " << (int)midiEvent->at(i) << std::endl;
		ticks = getNextEvent( midiEvent, track );
	}

	//for( unsigned int i=0; i<midiEvent->size(); i++ )
	//std::cout << "event byte = " << i << ", value = " << (int)midiEvent->at(i) << std::endl;

	return ticks;
}

bool MidiFileIn :: readVariableLength( unsigned long *value )
{
	// It is assumed that this function is called with the file read
	// pointer positioned at the start of a variable-length value.  The
	// function returns "true" if the value is successfully parsed and
	// "false" otherwise.
	*value = 0;
	char c;

	if( !file_.Read( &c, 1 ) )
		return false;
	*value = (unsigned long) c;
	if( *value & 0x80 ) 
	{
		*value &= 0x7f;
		do 
		{
			if( !file_.Read( &c, 1 ) ) 
				return false;
			*value = ( *value << 7 ) + ( c & 0x7f );
		} while( c & 0x80 );
	}

	return true;
} 
/**
 * @brief Utilities for working with the Guitar mode.
 *
 * Is this going to be kept in sm-ssc? */
namespace Guitar
{
	enum GuitarDifficulty { easy, medium, hard, expert, NUM_GuitarDifficulty };
	enum NoteNumberType { 
		green, 
		red, 
		yellow, 
		blue, 
		orange, 
		star_power, 
		player1_section, 
		player2_section, 
		NUM_NoteNumberType
	};
	const int NUM_FRETS = 5;
	enum MidiEventType 
	{
		note_off = 0x8,
		note_on = 0x9,
		note_aftertouch = 0xA,
		controller = 0xB,
		program_change = 0xC,
		channel_aftertouch = 0xD,
		pitch_bend = 0xE,
	};
	enum MetaEventType 
	{
		sequence_number = 0x00,
		text_event = 0x01,
		copyright_notice = 0x02,
		track_name = 0x03,
		instrument_name = 0x04,
		lyrics = 0x05,
		marker = 0x06,
		cue_point = 0x07,
		midi_channel_prefix = 0x20,
		end_of_track = 0x2F,
		set_tempo = 0x51,
		smpte_offset = 0x54,
		time_signature = 0x58,
		key_signature = 0x59,
		sequencer_specific = 0x7F,
	};
	struct MidiEvent
	{
		long count;
		MidiEventType midiEventType;
	};
};

using namespace Guitar;

/*
60: guitar note GREEN, easy (C) 
61: guitar note RED, easy (C#) 
62: guitar note YELLOW, easy (D) 
63: guitar note BLUE, easy (D#) 
64: guitar note ORANGE, easy (E) 
67: star power group, easy (G) 
69: player 1 section, easy (A) 
70: player 2 section, easy (A#) 
72: guitar note GREEN, medium (C) 
73: guitar note RED, medium (C#) 
74: guitar note YELLOW, medium (D) 
75: guitar note BLUE, medium (D#) 
76: guitar note ORANGE, medium (E) 
79: star power group, medium (G) 
81: player 1 section, medium (A) 
82: player 2 section, medium (A#) 
84: guitar note GREEN, hard (C) 
85: guitar note RED, hard (C#) 
86: guitar note YELLOW, hard (D) 
87: guitar note BLUE, hard (D#) 
88: guitar note ORANGE, hard (E) 
91: star power group, hard (G) 
93: player 1 section, hard (A) 
94: player 2 section, hard (A#) 
96: guitar note GREEN, expert (C) 
97: guitar note RED, expert (C#) 
98: guitar note YELLOW, expert (D) 
99: guitar note BLUE, expert (D#) 
100: guitar note ORANGE, expert (E) 
103: star power group, expert (G) 
105: player 1 section, expert (A) 
106: player 2 section, expert (A#) 
108: vocal track (C)
*/
static void DifficultyAndNoteNumberTypeToNoteNumber( GuitarDifficulty gd, NoteNumberType nnt, uint8_t &uNoteNumberOut )
{
	ASSERT( gd >= 0  &&  gd < NUM_GuitarDifficulty );
	ASSERT( nnt >= 0  &&  nnt < NUM_NoteNumberType );

	uNoteNumberOut = 60 + gd*12 + nnt;
}

// TODO: Generalize these.  These values are specific to guitar midi files.
static long GUITAR_MIDI_COUNTS_PER_EIGHTH_NOTE = 240;
static long GUITAR_MIDI_COUNTS_PER_BEAT = GUITAR_MIDI_COUNTS_PER_EIGHTH_NOTE * 2;

static int MidiCountToNoteRow( long iMidiCount )
{
	return (iMidiCount * ROWS_PER_BEAT) / GUITAR_MIDI_COUNTS_PER_BEAT;
}

static bool LoadFromMidi( const RString &sPath, Song &songOut )
{
	MidiFileIn midi;
	midi.Load( sPath );
	
	FOREACH_CONST( MidiFileIn::TempoChange, midi.tempoEvents_, iter )
	{
		BPMSegment bpmSeg;
		bpmSeg.m_iStartRow = MidiCountToNoteRow( iter->count );
		double fSecondsPerBeat = (iter->tickSeconds * GUITAR_MIDI_COUNTS_PER_BEAT);
		bpmSeg.m_fBPS = float( 1. / fSecondsPerBeat );

		songOut.m_SongTiming.AddBPMSegment( bpmSeg );
	}

	FOREACH_CONST( MidiFileIn::TimeSignatureChange, midi.timeSignatureEvents_, iter )
	{
		TimeSignatureSegment seg;
		seg.m_iStartRow = MidiCountToNoteRow( iter->count );
		seg.m_iNumerator = iter->numerator;
		seg.m_iDenominator = iter->denominator;

		songOut.m_SongTiming.AddTimeSignatureSegment( seg );
	}


	/* Read the MIDI events into per-difficulty, pre-NoteNumber vector.  The processing 
	 * rules for MIDI events often need to look at the next or previous event with the same 
	 * NoteNumber.  This is difficult using MidiFileIn interfaces, so this intermediate processing 
	 * step is helpful. */
	const int MAX_MIDI_NOTE_NUMBERS = 256;
	vector<MidiEvent> vMidiEvent[MAX_MIDI_NOTE_NUMBERS];

	/* Iterate through each track looking for the track that contains notes by checking the track name.
	 * This is usually track 1, but sometimes is track 2. */
	for( int iTrack = 1; iTrack<(int)midi.getNumberOfTracks(); iTrack++ )
	{
		std::vector<unsigned char> event;
		unsigned long count = midi.getNextEvent( &event, iTrack );
		double fSeconds = midi.getTickSeconds( iTrack );
		while( event.size() ) 
		{
			if( event[0] == 0xFF )	// meta event
			{
				uint8_t uMetaEventType = event[1];
				switch( uMetaEventType )
				{
				case track_name:
					{
						uint8_t uStringLength = event[2];
						RString s;
						for( int i=3; i<3 + uStringLength; i++ )
							s += event[i];
						if( s != "T1 GEMS"  &&  s != "PART GUITAR" )
							goto skip_track;
					}
					break;
				default:
					LOG->Trace( "Unhandled MIDI meta event type %X", uMetaEventType );
				}
			}
			else
			{
				MidiEventType midiEventType = (MidiEventType)(event[0] >> 4);
				//uint8_t uMidiChannel = event[0] & 0xF;	// currently unused
				uint8_t uParam1 = event[1];	// meaning is MidiEventType-specific
				uint8_t uParam2 = event[2];	// currently unused, meaning is MidiEventType-specific
				
				switch( midiEventType )
				{
				case note_off:
				case note_on:
					{
						const uint8_t &uNoteNumber = uParam1;
						const uint8_t &uVelocity = uParam2;

						// velocity == 0, by convention, means note-off
						if( uVelocity == 0 )
							midiEventType = note_off;

						MidiEvent mEvent = { count, midiEventType };
						//float fBeat = NoteRowToBeat( MidiCountToNoteRow(count) );
						vMidiEvent[uNoteNumber].push_back( mEvent );
					}
					break;
				default:
					LOG->Trace( "Unhandled MIDI event type %X", midiEventType );
					break;
				}
			}

			count += midi.getNextEvent( &event, iTrack );
			fSeconds = midi.getTickSeconds( iTrack );
		}
skip_track:
		;
	}

		
	FOREACH_ENUM( GuitarDifficulty, gd )
	{
		NoteData noteData;
		noteData.SetNumTracks( NUM_FRETS );

		FOREACH_ENUM( NoteNumberType, nnt )
		{
			if( nnt >= NUM_FRETS )
				continue;	// data other than the frets is not handled yet

			uint8_t uNoteNumber;
			DifficultyAndNoteNumberTypeToNoteNumber( gd, nnt, uNoteNumber );

			bool bNonTerminatedNote = false;
			long countOfLastNote = 0;
			FOREACH_CONST( MidiEvent, vMidiEvent[uNoteNumber], iter )
			{				 
				/*
				1) Note-on events indicate the start of a guitar note 
				2) The end of a note is indicated by a corresponding note-off event or by another note-on event, 
				   both occuring at a non-zero number of pulses after the start of the first note-on event 
				3) The pulse duration of a note is determined by subtracting the timestamp of the note-on event 
				   from the corresponding endpoint event. 
				4) Notes with a pulse duration shorter than 240 pulses are considered to be non-sustained notes 

				Not yet implemented:
				5) Valid non-sustained notes must have a corresponding note-off event. If a note endpoint is a 
				   second note-on event and the duration of the note is less than 161 pulses, the game considers 
				   the note to be an invalid note and it is ignored for all purposes (as exhibited by Cheat on 
				   the Church) 
				5) If a player section note-off event occurs more than 15 (30?) pulses prior to the endpoint of 
				   a sustained note, the sustained note is ignored by the game for all purposes, even in single 
				   player mode (as exhibited in the solo of You Got Another Thing Comin') 
				6) At any given time stamp, note-off events will be placed before note-on events. Duplicate 
				   note-on and note-off events within the same timestamp are ignored for purposes of determining 
				   note endpoints. In this case, the term "duplicate" means a situation where a note-on or 
				   note-off event specifies the same note-number as a prior event occuring at the same timestamp.
				*/
				MidiEventType midiEventType = iter->midiEventType;
				long count = iter->count;
				//float fBeat = NoteRowToBeat( MidiCountToNoteRow(count) );
				bool bNoteHandled = false;
				long length = count - countOfLastNote;


				// Check for termination of a sustain note
				switch( midiEventType )
				{
				case note_off:
				case note_on:
					if( bNonTerminatedNote )
					{
						if( length >= 240 )
						{
							TapNote tn = TAP_ORIGINAL_HOLD_HEAD;
							tn.iDuration = MidiCountToNoteRow( length );
							noteData.SetTapNote( nnt, MidiCountToNoteRow(countOfLastNote), tn );

//							if( gd == expert  &&  fBeat >= 27*4-2  &&  nnt == green )
//								LOG->Trace( "Added hold at %f, length %d", fBeat, length );
						}

						bNonTerminatedNote = false;
						bNoteHandled = true;
					}
					break;
				}


				switch( midiEventType )
				{
				case note_on:
					{
						TapNote tn = TAP_ORIGINAL_TAP;

						// We're about to add a tap note.  If the previous note was a sustain note that ended 
						// on this row, then make the sustain note shorter so that it doesn't end on 
						// the same row as the tap note we're about to add.  NoteData cannot handle a 
						// hold note ending on the same row as a tap note.
						NoteData::TrackMap::iterator begin, end;
						noteData.GetTapNoteRangeInclusive( nnt, MidiCountToNoteRow(count), MidiCountToNoteRow(count), begin, end, true );
						for( NoteData::TrackMap::iterator lIter = begin; lIter != end; lIter++ )
						{
//							if( gd == expert  &&  fBeat >= 27*4-2  &&  nnt == green )
//								LOG->Trace( "shortening hold at %f, length %d", fBeat, length );

							ASSERT( lIter->second.type == TapNote::hold_head );
							lIter->second.iDuration = MidiCountToNoteRow(count) - lIter->first - 2;
						}

						noteData.SetTapNote( nnt, MidiCountToNoteRow(count), tn );


//						if( gd == expert  &&  fBeat >= 27*4-2  &&  nnt == green )
//							LOG->Trace( "Added tap at %f, length %d", fBeat, length );
		
						bNonTerminatedNote = true;
						bNoteHandled = true;
					}
				}

				countOfLastNote = count;

				if( !bNoteHandled )
					LOG->Warn( "Unexpected MIDI event type %X at count %ld", midiEventType, count );
			}
		}

		Steps *pSteps = songOut.CreateSteps();
		pSteps->m_StepsType = StepsType_guitar_five;
		pSteps->SetDifficulty( (Difficulty)(gd+1) );
		pSteps->SetNoteData( noteData );
		songOut.AddSteps( pSteps );
	}

	return true;
}

void MidiLoader::GetApplicableFiles( const RString &sPath, vector<RString> &out )
{
	GetDirListing( sPath + RString("*.mid"), out );
}

bool MidiLoader::LoadFromDir( const RString &sDir, Song &out )
{
	LOG->Trace( "MidiLoader::LoadFromDir(%s)", sDir.c_str() );

	// Discover song title
	{
		out.m_sMainTitle = Basename( sDir );
		IniFile ini;
		if( ini.ReadFile(sDir+"song.ini") )
		{
			ini.GetValue("song","artist",out.m_sArtist);
			ini.GetValue("song","name",out.m_sMainTitle);
		}
	}
	
	{
		vector<RString> vsFiles;
		GetDirListing( sDir + RString("*song.oga"), vsFiles );
		GetDirListing( sDir + RString("*song.ogg"), vsFiles );
		GetDirListing( sDir + RString("*song.mp3"), vsFiles );
		GetDirListing( sDir + RString("*song.wav"), vsFiles );
		if( vsFiles.size() > 0 )
			out.m_sMusicFile = vsFiles[0];
	}

	{
		vector<RString> vsFiles;
		GetDirListing( sDir + RString("*guitar.oga"), vsFiles );
		GetDirListing( sDir + RString("*guitar.ogg"), vsFiles );
		GetDirListing( sDir + RString("*guitar.mp3"), vsFiles );
		GetDirListing( sDir + RString("*guitar.wav"), vsFiles );
		if( vsFiles.size() > 0 )
			out.m_sInstrumentTrackFile[InstrumentTrack_Guitar] = vsFiles[0];
	}

	{
		vector<RString> vsFiles;
		GetDirListing( sDir + RString("*rhythm.oga"), vsFiles );
		GetDirListing( sDir + RString("*rhythm.ogg"), vsFiles );
		GetDirListing( sDir + RString("*rhythm.mp3"), vsFiles );
		GetDirListing( sDir + RString("*rhythm.wav"), vsFiles );
		if( vsFiles.size() > 0 )
			out.m_sInstrumentTrackFile[InstrumentTrack_Rhythm] = vsFiles[0];
	}

	{
		vector<RString> vsFiles;
		GetDirListing( sDir + RString("*bass.oga"), vsFiles );
		GetDirListing( sDir + RString("*bass.ogg"), vsFiles );
		GetDirListing( sDir + RString("*bass.mp3"), vsFiles );
		GetDirListing( sDir + RString("*bass.wav"), vsFiles );
		if( vsFiles.size() > 0 )
			out.m_sInstrumentTrackFile[InstrumentTrack_Bass] = vsFiles[0];
	}

	vector<RString> vsFiles;
	GetDirListing( sDir + RString("*.mid"), vsFiles );

	/* We shouldn't have been called to begin with if there were no KSFs. */
	ASSERT( vsFiles.size() );

	if( !LoadFromMidi(sDir+vsFiles[0], out) )
		return false;

	return true;
}

/*
 * (c) 2007 Chris Danford
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
