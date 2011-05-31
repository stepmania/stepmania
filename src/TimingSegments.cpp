#include "TimingSegments.h"

#define LTCOMPARE(x)      if(this->x < other.x) return true; if(this->x > other.x) return false;

BaseTimingSegment::~BaseTimingSegment() {}


void BaseTimingSegment::SetRow(const int s)
{
	this->startingRow = s;
}

void BaseTimingSegment::SetBeat(const float s)
{
	SetRow(BeatToNoteRow(s));
}

int BaseTimingSegment::GetRow() const
{
	return this->startingRow;
}

float BaseTimingSegment::GetBeat() const
{
	return NoteRowToBeat(GetRow());
}

template <class DerivedSegment>
bool TimingSegment<DerivedSegment>::operator<( const DerivedSegment &other ) const
{ 
	LTCOMPARE(GetRow());
	return false;
}


/* ======================================================
   Here comes the actual timing segments implementation!! */

float FakeSegment::GetLength() const
{
	return this->lengthBeats;
}

void FakeSegment::SetLength(const float b)
{
	this->lengthBeats = b;
}

bool FakeSegment::operator<( const FakeSegment &other ) const
{ 
	LTCOMPARE(GetRow());
	LTCOMPARE(GetLength());
	return false;
}

float WarpSegment::GetLength() const
{
	return this->lengthBeats;
}

void WarpSegment::SetLength(const float b)
{
	this->lengthBeats = b;
}

bool WarpSegment::operator<( const WarpSegment &other ) const
{ 
	LTCOMPARE(GetRow());
	LTCOMPARE(GetLength());
	return false;
}



int TickcountSegment::GetTicks() const
{
	return this->ticks;
}

void TickcountSegment::SetTicks(const int i)
{
	this->ticks = i;
}

bool TickcountSegment::operator<( const TickcountSegment &other ) const
{
	LTCOMPARE(GetRow());
	LTCOMPARE(GetTicks());
	return false;
}

int ComboSegment::GetCombo() const
{
	return this->combo;
}

void ComboSegment::SetCombo(const int i)
{
	this->combo = i;
}

bool ComboSegment::operator<( const ComboSegment &other ) const
{
	LTCOMPARE(GetRow());
	LTCOMPARE(GetCombo());
	return false;
}

RString LabelSegment::GetLabel() const
{
	return this->label;
}

void LabelSegment::SetLabel(const RString l)
{
	this->label = l;
}

bool LabelSegment::operator<( const LabelSegment &other ) const
{ 
	LTCOMPARE(GetRow());
	LTCOMPARE(GetLabel());
	return false;
}

/**
 * @file
 * @author Jason Felds (c) 2011 
 * @section LICENSE
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
