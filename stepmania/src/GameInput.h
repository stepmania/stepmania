#pragma once
/*
-----------------------------------------------------------------------------
 Class: GameInput

 Desc: An input event specific to a Game definied by an instrument and a button space.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
enum InstrumentNumber
{
	INSTRUMENT_1 = 0,
	INSTRUMENT_2,
	MAX_INSTRUMENTS,	// leave this at the end
	INSTRUMENT_NONE,
};

enum InstrumentButton
{
	INSTRUMENT_BUTTON_1 = 0,
	INSTRUMENT_BUTTON_2,
	INSTRUMENT_BUTTON_3,
	INSTRUMENT_BUTTON_4,
	INSTRUMENT_BUTTON_5,
	INSTRUMENT_BUTTON_6,
	INSTRUMENT_BUTTON_7,
	INSTRUMENT_BUTTON_8,
	INSTRUMENT_BUTTON_9,
	INSTRUMENT_BUTTON_10,
	INSTRUMENT_BUTTON_11,
	INSTRUMENT_BUTTON_12,
	INSTRUMENT_BUTTON_13,
	INSTRUMENT_BUTTON_14,
	INSTRUMENT_BUTTON_15,
	INSTRUMENT_BUTTON_16,
	MAX_INSTRUMENT_BUTTONS,		// leave this at the end
	INSTRUMENT_BUTTON_NONE
};


struct GameInput
{
	GameInput() { MakeBlank(); };
	GameInput( InstrumentNumber n, InstrumentButton b ) { number = n; button = b; };


	InstrumentNumber number;	// instrument number
	InstrumentButton button;	// instrument button

	bool operator==( const GameInput &other ) { return number == other.number && button == other.button; };

	inline bool IsBlank() const { return number == INSTRUMENT_NONE; };
	inline bool IsValid() const { return !IsBlank(); };
	inline void MakeBlank() { number = INSTRUMENT_NONE; button = INSTRUMENT_BUTTON_NONE; };


	CString toString() 
	{
		return ssprintf("%d-%d", number, button );
	}

	bool fromString( CString s )
	{ 
		CStringArray a;
		split( s, "-", a);

		if( a.GetSize() != 2 ) {
			MakeBlank();
			return false;
		}

		number = (InstrumentNumber)atoi( a[0] );
		button = (InstrumentButton)atoi( a[1] );
		return true;
	};
};


