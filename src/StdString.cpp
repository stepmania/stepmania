// I've reorganized StdString.h to be optimized for the StepMania engine.
//
// A huge amount of code bloat was traditionally due to these functions being
// inlined. Performance issues with RStrings are largely mitigated by migrating
// frequently used functions here. Aside from the added declaration of 'noexcept',
// everything here is the same as it was when it was in the original header file.
//
// Testing showed a 100-250FPS gain on all platforms with these functions moved in here.
//
// A couple things, like ssasn, were updated to be more idiomatic for C++ but remain
// functionally identical.
//
// This is just meant to hold us over until we get std::string fully worked into
// the engine. The std::string swap was made in ITGmania, maintains full SM5 parity
// and has been proven to be an improvement all around.
//
// --sukibaby

#include "global.h"
#include "StdString.h"

namespace StdString
{
	void ssasn(std::string& sDst, const std::string& sSrc) noexcept
	{
		if (sDst.c_str() != sSrc.c_str())
		{
			sDst = sSrc;
		}
	}

	void ssasn(std::wstring& sDst, const std::wstring& sSrc) noexcept
	{
		if (sDst.c_str() != sSrc.c_str())
		{
			sDst = sSrc;
		}
	}

	void ssasn(std::string& sDst, PCSTR pA) noexcept
	{
		sDst = pA ? pA : "";
	}

	void ssasn(std::wstring& sDst, PCWSTR pA) noexcept
	{
		sDst = pA ? pA : L"";
	}

	void ssadd(std::string& sDst, const std::string& sSrc) noexcept
	{
		sDst += sSrc;
	}

	void ssadd(std::wstring& sDst, const std::wstring& sSrc) noexcept
	{
		sDst += sSrc;
	}

	void ssadd(std::string& sDst, PCSTR pA) noexcept
	{
		if (pA)
		{
			if (pA >= sDst.c_str() && pA <= sDst.c_str() + sDst.length())
			{
				if (sDst.capacity() <= sDst.size() + strlen(pA))
					sDst.append(std::string(pA));
				else
					sDst.append(pA);
			}
			else
			{
				sDst.append(pA);
			}
		}
	}

	void ssadd(std::wstring& sDst, PCWSTR pA) noexcept
	{
		if (pA)
		{
			if (pA >= sDst.c_str() && pA <= sDst.c_str() + sDst.length())
			{
				if (sDst.capacity() <= sDst.size() + wcslen(pA))
					sDst.append(std::wstring(pA));
				else
					sDst.append(pA);
			}
			else
			{
				sDst.append(pA);
			}
		}
	}

	char sstoupper(char ch) noexcept
	{
		return (ch >= 'a' && ch <= 'z') ? char(ch + 'A' - 'a') : ch;
	}

	char sstolower(char ch) noexcept
	{
		return (ch >= 'A' && ch <= 'Z') ? char(ch + 'a' - 'A') : ch;
	}

	wchar_t sstoupper(wchar_t ch) noexcept
	{
		return (ch >= L'a' && ch <= L'z') ? wchar_t(ch + L'A' - L'a') : ch;
	}

	wchar_t sstolower(wchar_t ch) noexcept
	{
		return (ch >= L'A' && ch <= L'Z') ? wchar_t(ch + L'a' - L'A') : ch;
	}

}

// see StdString.h for license
