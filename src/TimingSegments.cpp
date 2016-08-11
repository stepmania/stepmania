#include "global.h"
#include "TimingSegments.h"
#include "EnumHelper.h"

using std::vector;

const double TimingSegment::EPSILON = 1e-6;

static const char *TimingSegmentTypeNames[] = {
	"BPM",
	"Stop",
	"Delay",
	"Time Sig",
	"Warp",
	"Label",
	"Tickcount",
	"Combo",
	"Speed",
	"Scroll",
	"Fake"
};
XToString( TimingSegmentType );

#define LTCOMPARE(x)      if(this->x < other.x) return true; if(this->x > other.x) return false;

void TimingSegment::Scale( int start, int length, int newLength )
{
	SetRow( ScalePosition( start, length, newLength, this->GetRow() ) );
}

void TimingSegment::DebugPrint() const
{
	LOG->Trace( "\tTimingSegment(%d [%f])", GetRow(), GetBeat() );
}

void BPMSegment::DebugPrint() const
{
	LOG->Trace( "\t%s(%d [%f], %f)",
		TimingSegmentTypeToString(GetType()).c_str(),
		GetRow(), GetBeat(), GetBPM()
	);
}

void StopSegment::DebugPrint() const
{
	LOG->Trace( "\t%s(%d [%f], %f)",
		TimingSegmentTypeToString(GetType()).c_str(),
		GetRow(), GetBeat(), GetPause()
	);
}

void DelaySegment::DebugPrint() const
{
	LOG->Trace( "\t%s(%d [%f], %f)",
		TimingSegmentTypeToString(GetType()).c_str(),
		GetRow(), GetBeat(), GetPause()
	);
}

void TimeSignatureSegment::DebugPrint() const
{
	LOG->Trace( "\t%s(%d [%f], %d/%d)",
		TimingSegmentTypeToString(GetType()).c_str(),
		GetRow(), GetBeat(), GetNum(), GetDen()
	);
}

void WarpSegment::DebugPrint() const
{
	LOG->Trace( "\t%s(%d [%f], %d [%f])",
		TimingSegmentTypeToString(GetType()).c_str(),
		GetRow(), GetBeat(), GetLengthRows(), GetLengthBeats()
	);
}

void LabelSegment::DebugPrint() const
{
	LOG->Trace( "\t%s(%d [%f], %s)",
		TimingSegmentTypeToString(GetType()).c_str(),
		GetRow(), GetBeat(), GetLabel().c_str()
	);
}

void TickcountSegment::DebugPrint() const
{
	LOG->Trace( "\t%s(%d [%f], %d)",
		TimingSegmentTypeToString(GetType()).c_str(),
		GetRow(), GetBeat(), GetTicks()
	);
}

void ComboSegment::DebugPrint() const
{
	LOG->Trace( "\t%s(%d [%f], %d, %d)",
		TimingSegmentTypeToString(GetType()).c_str(),
		GetRow(), GetBeat(), GetCombo(), GetMissCombo()
	);
}

void SpeedSegment::DebugPrint() const
{
	LOG->Trace( "\t%s(%d [%f], %f, %f, %d)",
		TimingSegmentTypeToString(GetType()).c_str(),
		GetRow(), GetBeat(), GetRatio(), GetDelay(), static_cast<int>(GetUnit())
	);
}

void ScrollSegment::DebugPrint() const
{
	LOG->Trace( "\t%s(%d [%f], %f)",
		TimingSegmentTypeToString(GetType()).c_str(),
		GetRow(), GetBeat(), GetRatio()
	);
}

void FakeSegment::DebugPrint() const
{
	LOG->Trace( "\t%s(%d [%f], %d [%f])",
		TimingSegmentTypeToString(GetType()).c_str(),
		GetRow(), GetBeat(), GetLengthRows(), GetLengthBeats()
	);
}

std::string FakeSegment::ToString(int dec) const
{
	std::string str = "%.0" + std::to_string(dec)
	+ "f=%.0" + std::to_string(dec) + "f";
	return fmt::sprintf(str.c_str(), GetBeat(), GetLength());
}

void FakeSegment::Scale( int start, int length, int newLength )
{
	float startBeat    = GetBeat();
	float endBeat      = startBeat + GetLength();
	float newStartBeat = ScalePosition( NoteRowToBeat(start), NoteRowToBeat(length), NoteRowToBeat(newLength), startBeat );
	float newEndBeat   = ScalePosition( NoteRowToBeat(start), NoteRowToBeat(length), NoteRowToBeat(newLength), endBeat );
	SetLength( newEndBeat - newStartBeat );
	TimingSegment::Scale( start, length, newLength );
}

std::string WarpSegment::ToString(int dec) const
{
	std::string str = "%.0" + std::to_string(dec)
	+ "f=%.0" + std::to_string(dec) + "f";
	return fmt::sprintf(str.c_str(), GetBeat(), GetLength());
}

void WarpSegment::Scale( int start, int length, int newLength )
{
	// XXX: this function is duplicated, there should be a better way
	float startBeat    = GetBeat();
	float endBeat      = startBeat + GetLength();
	float newStartBeat = ScalePosition( NoteRowToBeat(start), NoteRowToBeat(length), NoteRowToBeat(newLength), startBeat );
	float newEndBeat   = ScalePosition( NoteRowToBeat(start), NoteRowToBeat(length), NoteRowToBeat(newLength), endBeat );
	SetLength( newEndBeat - newStartBeat );
	TimingSegment::Scale( start, length, newLength );
}

std::string TickcountSegment::ToString(int dec) const
{
	const std::string str = "%.0" + std::to_string(dec) + "f=%i";
	return fmt::sprintf(str.c_str(), GetBeat(), GetTicks());
}
std::string ComboSegment::ToString(int dec) const
{
	std::string str = "%.0" + std::to_string(dec) + "f=%i";
	if (GetCombo() == GetMissCombo())
	{
		return fmt::sprintf(str.c_str(), GetBeat(), GetCombo());
	}
	str += "=%i";
	return fmt::sprintf(str.c_str(), GetBeat(), GetCombo(), GetMissCombo());
}

vector<float> ComboSegment::GetValues() const
{
	vector<float> ret;
	ret.push_back(static_cast<float>(GetCombo()));
	ret.push_back(static_cast<float>(GetMissCombo()));
	return ret;
}

std::string LabelSegment::ToString(int dec) const
{
	const std::string str = "%.0" + std::to_string(dec) + "f=%s";
	return fmt::sprintf(str.c_str(), GetBeat(), GetLabel().c_str());
}

std::string BPMSegment::ToString(int dec) const
{
	const std::string str = "%.0" + std::to_string(dec)
	+ "f=%.0" + std::to_string(dec) + "f";
	return fmt::sprintf(str.c_str(), GetBeat(), GetBPM());
}

std::string TimeSignatureSegment::ToString(int dec) const
{
	const std::string str = "%.0" + std::to_string(dec) + "f=%i=%i";
	return fmt::sprintf(str.c_str(), GetBeat(), GetNum(), GetDen());
}

vector<float> TimeSignatureSegment::GetValues() const
{
	vector<float> ret;
	ret.push_back(static_cast<float>(GetNum()));
	ret.push_back(static_cast<float>(GetDen()));
	return ret;
}

std::string SpeedSegment::ToString(int dec) const
{
	const std::string str = "%.0" + std::to_string(dec)
		+ "f=%.0" + std::to_string(dec) + "f=%.0"
		+ std::to_string(dec) + "f=%u";
	return fmt::sprintf(str.c_str(), GetBeat(), GetRatio(),
		GetDelay(), static_cast<unsigned int>(GetUnit()));
}

vector<float> SpeedSegment::GetValues() const
{
	vector<float> ret;
	ret.push_back(GetRatio());
	ret.push_back(GetDelay());
	ret.push_back(static_cast<float>(GetUnit()));
	return ret;
}

void SpeedSegment::Scale( int start, int oldLength, int newLength )
{
	if( GetUnit() == 0 )
	{
		// XXX: this function is duplicated, there should be a better way
		float startBeat    = GetBeat();
		float endBeat      = startBeat + GetDelay();
		float newStartBeat = ScalePosition(NoteRowToBeat(start),
						   NoteRowToBeat(oldLength),
						   NoteRowToBeat(newLength),
						   startBeat);
		float newEndBeat   = ScalePosition(NoteRowToBeat(start),
						   NoteRowToBeat(oldLength),
						   NoteRowToBeat(newLength),
						   endBeat);
		SetDelay( newEndBeat - newStartBeat );
	}
	TimingSegment::Scale( start, oldLength, newLength );
}

std::string ScrollSegment::ToString(int dec) const
{
	const std::string str = "%.0" + std::to_string(dec)
		+ "f=%.0" + std::to_string(dec) + "f";
	return fmt::sprintf(str.c_str(), GetBeat(), GetRatio());
}

std::string StopSegment::ToString(int dec) const
{
	const std::string str = "%.0" + std::to_string(dec)
	+ "f=%.0" + std::to_string(dec) + "f";
	return fmt::sprintf(str.c_str(), GetBeat(), GetPause());
}

std::string DelaySegment::ToString(int dec) const
{
	const std::string str = "%.0" + std::to_string(dec)
	+ "f=%.0" + std::to_string(dec) + "f";
	return fmt::sprintf(str.c_str(), GetBeat(), GetPause());
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
