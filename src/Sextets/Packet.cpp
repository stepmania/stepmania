
#include "Sextets/Packet.h"
#include "RageLog.h"
#include "global.h"
#include "LightsManager.h"

#include <vector>

#define PUSH(i) bits.push_back(bitArray[i])
#define PUSH0() bits.push_back(false)

typedef RString::value_type RChr;
typedef std::vector<RChr> RVector;
typedef Sextets::Packet::ProcessEventCallback ProcessEventCallback;

namespace
{
	void RStringFromRVector(RString& result, size_t resultLeft, const RVector& source, size_t sourceLeft, size_t length);

	inline size_t smax(size_t a, size_t b)
	{
		return (a > b) ? a : b;
	}

	inline size_t smin(size_t a, size_t b)
	{
		return (a < b) ? a : b;
	}

	inline size_t smin(size_t a, size_t b, size_t c)
	{
		return smin(smin(a, b), c);
	}

	inline RChr Armored(RChr x)
	{
		return ((x + 0x10) & 0x3F) + 0x30;
	}

	const RChr ARMORED_0 = Armored(0);

	inline bool IsArmored(RChr x)
	{
		return (x >= 0x30) && (x <= 0x6F);
	}

	inline size_t FindCleanPacketLeft(const RString& line)
	{
		size_t left = 0;
		size_t end = line.length();

		while(left < end) {
			if(IsArmored(line[left])) {
				break;
			}
			++left;
		}

		return left;
	}

	inline size_t FindCleanPacketRight(const RString& line, size_t left)
	{
		size_t right = left;
		size_t end = line.length();

		while(right < end) {
			if(!IsArmored(line[right])) {
				break;
			}
			++right;
		}

		return right;
	}

	// Does a = a ^ b for entire RVectors.
	inline void XorVectors(RVector& a, const RVector& b)
	{
		size_t alen = a.size();
		size_t blen = b.size();

		size_t len = smin(alen, blen);

		// Only perform actual XOR on the common length.
		for(size_t i = 0; i < len; ++i) {
			a[i] = Armored(a[i] ^ b[i]);
		}

		// If this packet is shorter, the rest of the result is zero XOR b = b.
		// So, the rest is copied directly from b.
		if(alen < blen) {
			a.reserve(blen);
			for(size_t i = alen; i < blen; ++i) {
				a.push_back(b[i]);
			}
		}

		// If the packets are the same length, the XOR is already complete.
		// If this packet is longer, the rest of the result is zero XOR this = this.
		// Since this is the result, no copying is necessary.
	}

	inline void XorVectors(RVector& result, const RVector& a, const RVector& b)
	{
		// result will start as a copy of a or b, whichever is not shorter
		// than the other, since a shorter destination involves more work.
		if(a.size() > b.size()) {
			result = a;
			XorVectors(result, b);
		} else {
			result = b;
			XorVectors(result, a);
		}
	}

	inline bool VectorRangeAllArmored0(RVector::const_iterator& it, RVector::const_iterator& end)
	{
		while(it != end) {
			if(*it != ARMORED_0) {
				return false;
			}
			it++;
		}
		return true;
	}

	inline bool VectorsEqual(const RVector& a, const RVector& b)
	{
		RVector::const_iterator ait = a.begin();
		RVector::const_iterator aend = a.end();
		RVector::const_iterator bit = b.begin();
		RVector::const_iterator bend = b.end();

		while((ait != aend) && (bit != bend)) {
			if(*ait != *bit) {
				return false;
			}
			++ait;
			++bit;
		}

		return
			(ait != aend) ? VectorRangeAllArmored0(ait, aend) :
			(bit != bend) ? VectorRangeAllArmored0(bit, bend) : true;
	}



	class ProcessEventCallbackDispatcher
	{
	private:
		const RVector& eventSextets;
		const RVector& valueSextets;
		const size_t bitCount;
		void * const context;
		ProcessEventCallback const callback;

		void CallCallback(size_t bitIndex, bool value)
		{
			callback(context, bitIndex, value);
		}

		inline void ProcessOneSextet(RChr eventSextet, RChr valueSextet, size_t bitStartIndex, size_t turns)
		{
			for(size_t subIndex = 0; subIndex < turns; ++subIndex) {
				RChr mask = 1 << subIndex;
				if(eventSextet & mask) {
					size_t bitIndex = bitStartIndex + subIndex;
					int maskedValue = valueSextet & mask;
					bool bitValue = maskedValue != 0;
					CallCallback(bitIndex, bitValue);
				}
			}
		}

		void ProcessAll()
		{
			size_t eventSextetCount = eventSextets.size();
			size_t valueSextetCount = valueSextets.size();

			for(size_t sextetIndex = 0, bitStartIndex = 0; sextetIndex < eventSextetCount; ++sextetIndex, bitStartIndex += 6) {
				RChr eventSextet = eventSextets[sextetIndex];

				if(eventSextet == 0) {
					// No 1 bits this sextet.
					continue;
				}

				size_t turns = smin(6, bitCount - bitStartIndex);

				RChr valueSextet = (sextetIndex < valueSextetCount) ? valueSextets[sextetIndex] : 0;

				if(turns > 0) {
					ProcessOneSextet(eventSextet, valueSextet, bitStartIndex, turns);
				}

				if(turns < 6) {
					// bitCount has been reached.
					break;
				}
			}
		}


	public:
		ProcessEventCallbackDispatcher(
			const RVector& eventSextets,
			const RVector& valueSextets,
			const size_t bitCount,
			void * const context,
			ProcessEventCallback const callback
		) :
			eventSextets(eventSextets),
			valueSextets(valueSextets),
			bitCount(smin(bitCount, eventSextets.size() * 6)),
			context(context),
			callback(callback)
		{
		}

		void RunEvents()
		{
			ProcessAll();
		}
	};

	// sourceLeft + length must be less than source.length().
	// result is automatically expanded.
	void RVectorFromRString(RVector& result, size_t resultLeft, const RString& source, size_t sourceLeft, size_t length)
	{
		RString::const_iterator sourceIt, sourceEnd;
		RVector::iterator resultIt;

		// Expand result if needed
		size_t resultRight = resultLeft + length;
		if(resultRight > result.size()) {
			result.resize(sourceLeft + length, ARMORED_0);
		}

		sourceIt = source.begin();
		std::advance(sourceIt, sourceLeft);

		sourceEnd = sourceIt;
		std::advance(sourceEnd, length);

		resultIt = result.begin();

		for(; sourceIt != sourceEnd; ++sourceIt, ++resultIt) {
			*resultIt = Armored(*sourceIt);
		}
	}

	// sourceLeft + length must be less than source.size().
	// result is automatically expanded.
	void RStringFromRVector(RString& result, size_t resultLeft, const RVector& source, size_t sourceLeft, size_t length)
	{
		RVector::const_iterator sourceIt, sourceEnd;
		RString::iterator resultIt;

		// Expand result if needed
		size_t resultRight = resultLeft + length;
		if(resultRight > result.length()) {
			result.resize(sourceLeft + length, ARMORED_0);
		}

		sourceIt = source.begin();
		std::advance(sourceIt, sourceLeft);

		sourceEnd = sourceIt;
		std::advance(sourceEnd, length);

		resultIt = result.begin();

		for(; sourceIt != sourceEnd; ++sourceIt, ++resultIt) {
			*resultIt = Armored(*sourceIt);
		}
	}

	inline void ProcessEventDataVectors(const RVector& eventSextets, const RVector& valueSextets, size_t bitCount, void * context, ProcessEventCallback callback)
	{
		ProcessEventCallbackDispatcher d(eventSextets, valueSextets, bitCount, context, callback);
		d.RunEvents();
	}
}

namespace Sextets
{
	class Packet::Impl
	{
	private:
		RVector sextets;

		void SetToSextetDataLine(const RString& line, size_t left, size_t right)
		{
			size_t length = right - left;
			RVectorFromRString(sextets, 0, line, left, length);
		}

	public:
		Impl()
		{
		}

		~Impl() {}

		size_t SextetCount() const
		{
			return sextets.size();
		}

		void Clear()
		{
			sextets.clear();
		}

		void Copy(const Packet& packet)
		{
			sextets = packet._impl->sextets;
		}

		void SetToLine(const RString& line)
		{
			size_t left = FindCleanPacketLeft(line);
			size_t right = FindCleanPacketRight(line, left);
			SetToSextetDataLine(line, left, right);
		}

		void SetToBitVector(const std::vector<bool> bits)
		{
			size_t bitCount = bits.size();
			size_t sextetCount = (bitCount + 5) / 6;

			sextets.resize(sextetCount, ARMORED_0);

			size_t sextetIndex = 0;
			size_t bitIndex = 0;
			while(bitIndex < bitCount) {
				RChr s = 0;
				size_t bitsUsed = smin(bitCount - bitIndex, 6);

				

				switch(bitsUsed) {
				case 6:
					s |= (bits[bitIndex + 5] ? 0x20 : 0);
				case 5:
					s |= (bits[bitIndex + 4] ? 0x10 : 0);
				case 4:
					s |= (bits[bitIndex + 3] ? 0x08 : 0);
				case 3:
					s |= (bits[bitIndex + 2] ? 0x04 : 0);
				case 2:
					s |= (bits[bitIndex + 1] ? 0x02 : 0);
				case 1:
					s |= (bits[bitIndex + 0] ? 0x01 : 0);
				case 0:
					break;
				}

				sextets[sextetIndex] = Armored(s);
				bitIndex += bitsUsed;
				sextetIndex++;
			}
		}


		void SetToLightsState(const LightsState * ls)
		{
			std::vector<bool> bits;
			const bool * bitArray;

			size_t toReserve = 6; // cabinet lights are 1 sextet, or 6 bits
			// TODO: Make this non-iterative
			FOREACH_ENUM(GameController, gc) {
				// Each controller takes 6 sextets, which is 36 bits.
				toReserve += (6 * 6);
			}
			bits.clear();
			bits.reserve(toReserve);

			// cabinet lights
			bitArray = ls->m_bCabinetLights;

			PUSH(LIGHT_MARQUEE_UP_LEFT);
			PUSH(LIGHT_MARQUEE_UP_RIGHT);
			PUSH(LIGHT_MARQUEE_LR_LEFT);
			PUSH(LIGHT_MARQUEE_LR_RIGHT);
			PUSH(LIGHT_BASS_LEFT);
			PUSH(LIGHT_BASS_RIGHT);

			// Game controller lights
			FOREACH_ENUM(GameController, gc) {
				bitArray = ls->m_bGameButtonLights[gc];

				// Menu buttons
				PUSH(GAME_BUTTON_MENULEFT);
				PUSH(GAME_BUTTON_MENURIGHT);
				PUSH(GAME_BUTTON_MENUUP);
				PUSH(GAME_BUTTON_MENUDOWN);
				PUSH(GAME_BUTTON_START);
				PUSH(GAME_BUTTON_SELECT);

				// Other non-sensors
				PUSH(GAME_BUTTON_BACK);
				PUSH(GAME_BUTTON_COIN);
				PUSH(GAME_BUTTON_OPERATOR);
				PUSH(GAME_BUTTON_EFFECT_UP);
				PUSH(GAME_BUTTON_EFFECT_DOWN);
				PUSH0();

				// Sensors
				PUSH(GAME_BUTTON_CUSTOM_01);
				PUSH(GAME_BUTTON_CUSTOM_02);
				PUSH(GAME_BUTTON_CUSTOM_03);
				PUSH(GAME_BUTTON_CUSTOM_04);
				PUSH(GAME_BUTTON_CUSTOM_05);
				PUSH(GAME_BUTTON_CUSTOM_06);

				PUSH(GAME_BUTTON_CUSTOM_07);
				PUSH(GAME_BUTTON_CUSTOM_08);
				PUSH(GAME_BUTTON_CUSTOM_09);
				PUSH(GAME_BUTTON_CUSTOM_10);
				PUSH(GAME_BUTTON_CUSTOM_11);
				PUSH(GAME_BUTTON_CUSTOM_12);

				PUSH(GAME_BUTTON_CUSTOM_13);
				PUSH(GAME_BUTTON_CUSTOM_14);
				PUSH(GAME_BUTTON_CUSTOM_15);
				PUSH(GAME_BUTTON_CUSTOM_16);
				PUSH(GAME_BUTTON_CUSTOM_17);
				PUSH(GAME_BUTTON_CUSTOM_18);

				PUSH(GAME_BUTTON_CUSTOM_19);
				PUSH0();
				PUSH0();
				PUSH0();
				PUSH0();
				PUSH0();
			}

			RString bs;
			for(size_t bi; bi < bits.size(); ++bi) {
				bs += (bits[bi] ? "1" : "0");
			}

			SetToBitVector(bits);
		}

		void SetToXor(const Packet& b)
		{
			XorVectors(sextets, b._impl->sextets);
		}

		void SetToXor(const Packet& a, const Packet& b)
		{
			XorVectors(sextets, a._impl->sextets, b._impl->sextets);
		}

		void ProcessEventData(const Packet& eventData, size_t bitCount, void * context, ProcessEventCallback callback)
		{
			ProcessEventDataVectors(eventData._impl->sextets, sextets, bitCount, context, callback);
		}

		void ProcessEachBit(size_t bitCount, void * context, ProcessEventCallback callback) const
		{
			size_t sextetCount = sextets.size();
			size_t bitIndex = 0;

			for(size_t sextetIndex = 0; sextetIndex < sextetCount; ++sextetIndex) {
				for(size_t subBitIndex = 0; subBitIndex < 6; ++subBitIndex, ++bitIndex) {
					if(bitIndex >= bitCount) {
						return;
					}
					callback(context, bitIndex, (sextets[sextetIndex] & (1 << subBitIndex)) != 0);
				}
			}

			while(bitIndex < bitCount) {
				callback(context, bitIndex, false);
				++bitIndex;
			}
		}

		bool Equals(const Packet& b) const
		{
			VectorsEqual(sextets, b._impl->sextets);
		}

		void GetUntrimmedLine(RString& line)
		{
			RStringFromRVector(line, 0, sextets, 0, sextets.size());
		}

		RString GetUntrimmedLine()
		{
			RString line;
			GetUntrimmedLine(line);
			return line;
		}

		void GetLine(RString& line)
		{
			GetUntrimmedLine(line);

			// An empty line stays empty.
			if(line.empty()) {
				return;
			}

			// Find the last non-trimmable character.
			size_t i = line.find_last_not_of(ARMORED_0);
			if(i == RString::npos) {
				// There are no non-trimmable characters.
				// The canonical form of this is a single armored 0.
				line.replace(0, RString::npos, 1, ARMORED_0);
			}
			else {
				// The first trailing zero character to erase, if any, is at
				// i + 1.
				line.erase(i + 1);
			}
		}

		RString GetLine()
		{
			RString line;
			GetLine(line);
			return line;
		}

		void Trim()
		{
			RString line;
			GetLine(line);
			SetToSextetDataLine(line, 0, line.length());
		}

		bool IsEmpty() const
		{
			return sextets.size() == 0;
		}

		bool IsZeroed() const
		{
			RVector::const_iterator it = sextets.begin();
			RVector::const_iterator end = sextets.end();

			for(; it != end; ++it) {
				if(*it != ARMORED_0) {
					return false;
				}
			}
			return true;
		}
	};

	Packet::Packet()
	{
		_impl = new Packet::Impl();
	}

	Packet::Packet(const Packet& packet)
	{
		_impl = new Packet::Impl();
		_impl->Copy(packet);
	}

	Packet::~Packet()
	{
		delete _impl;
	}

	size_t Packet::SextetCount() const
	{
		return _impl->SextetCount();
	}

	void Packet::Clear()
	{
		_impl->Clear();
	}

	void Packet::Copy(const Packet& packet)
	{
		_impl->Copy(packet);
	}

	Packet& Packet::operator=(const Packet& other)
	{
		Copy(other);
		return *this;
	}

	void Packet::SetToLine(const RString& line)
	{
		_impl->SetToLine(line);
	}

	void Packet::SetToLightsState(const LightsState * ls)
	{
		_impl->SetToLightsState(ls);
	}

	void Packet::SetToXor(const Packet& b)
	{
		_impl->SetToXor(b);
	}

	Packet& Packet::operator^=(const Packet& other)
	{
		SetToXor(other);
		return *this;
	}

	void Packet::SetToXor(const Packet& a, const Packet& b)
	{
		_impl->SetToXor(a, b);
	}

	// This is a non-member
	Packet operator^(const Packet& a, const Packet& b)
	{
		Packet result;
		result.SetToXor(a, b);
		return result;
	}

	void Packet::ProcessEachBit(size_t bitCount, void * context, ProcessEventCallback callback) const
	{
		_impl->ProcessEachBit(bitCount, context, callback);
	}

	void Packet::ProcessEventData(const Packet& eventData, size_t bitCount, void * context, ProcessEventCallback callback) const
	{
		_impl->ProcessEventData(eventData, bitCount, context, callback);
	}

	bool Packet::Equals(const Packet& b) const
	{
		return _impl->Equals(b);
	}

	bool Packet::operator==(const Packet& other) const
	{
		return Equals(other);
	}

	RString Packet::GetUntrimmedLine() const
	{
		return _impl->GetUntrimmedLine();
	}

	void Packet::GetUntrimmedLine(RString& line) const
	{
		_impl->GetUntrimmedLine(line);
	}

	RString Packet::GetLine() const
	{
		return _impl->GetLine();
	}

	void Packet::GetLine(RString& line) const
	{
		_impl->GetLine(line);
	}

	bool Packet::IsEmpty() const
	{
		return _impl->IsEmpty();
	}

	bool Packet::IsZeroed() const
	{
		return _impl->IsZeroed();
	}
}

/*
 * Copyright © 2016 Peter S. May
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
