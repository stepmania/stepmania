#include "TimingSegments.h"

TimingSegment::TimingSegment() : 
	startingRow(-1) {}

TimingSegment::TimingSegment(int s) : 
	startingRow(s) {}

TimingSegment::TimingSegment(float s) : 
	startingRow(BeatToNoteRow(s)) {}

TimingSegment::~TimingSegment() {}

void TimingSegment::SetRow(const int s)
{
	this->startingRow = s;
}
void TimingSegment::SetBeat(const float s)
{
	this->startingRow = BeatToNoteRow(s);
}

int TimingSegment::GetRow() const
{
	return this->startingRow;
}

float TimingSegment::GetBeat() const
{
	return NoteRowToBeat(this->startingRow);
}

FakeSegment::FakeSegment():
	TimingSegment(-1), lengthBeats(-1) {}

FakeSegment::FakeSegment( int s, int r ):
	TimingSegment(max(0, s)), lengthBeats(NoteRowToBeat(max(0, r))) {}

FakeSegment::FakeSegment( int s, float b ):
	TimingSegment(max(0, s)), lengthBeats(max(0, b)) {}

FakeSegment::FakeSegment( float s, int r ):
	TimingSegment(max(0, s)), lengthBeats(max(0, NoteRowToBeat(r))) {}

FakeSegment::FakeSegment( float s, float b ):
	TimingSegment(max(0, s)), lengthBeats(max(0, b)) {}

float FakeSegment::GetLength() const
{
	return this->lengthBeats;
}

void FakeSegment::SetLength(const float b)
{
	this->lengthBeats = b;
}

bool FakeSegment::operator==( const FakeSegment &other ) const
{
	if (this->GetRow() != other.GetRow()) return false;
	if (this->GetLength() != other.GetLength()) return false;
	return true;
}

bool FakeSegment::operator!=( const FakeSegment &other ) const
{
	return !this->operator==(other);
}

bool FakeSegment::operator<( const FakeSegment &other ) const
{ 
	if (this->GetRow() < other.GetRow()) return true;
	if (this->GetRow() > other.GetRow()) return false;
	return this->GetLength() < other.GetLength();
}

bool FakeSegment::operator<=( const FakeSegment &other ) const
{
	return ( this->operator<(other) || this->operator==(other) );
}

bool FakeSegment::operator>( const FakeSegment &other ) const
{
	return !this->operator<=(other);
}

bool FakeSegment::operator>=( const FakeSegment &other ) const
{
	return !this->operator<(other);
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
