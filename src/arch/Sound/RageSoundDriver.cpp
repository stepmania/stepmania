#include "global.h"
#include "RageSoundDriver.h"
#include "RageSoundManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageUtil.hpp"
#include "arch/arch_default.h"

using std::vector;

DriverList RageSoundDriver::m_pDriverList;

RageSoundDriver *RageSoundDriver::Create( const std::string& drivers )
{
	vector<std::string> drivers_to_try;
	if(drivers.empty())
	{
		drivers_to_try = Rage::split(DEFAULT_SOUND_DRIVER_LIST, ",", Rage::EmptyEntries::skip);
	}
	else
	{
		drivers_to_try = Rage::split(drivers, ",", Rage::EmptyEntries::skip);
		size_t to_try= 0;
		bool had_to_erase= false;
		while(to_try < drivers_to_try.size())
		{
			if(m_pDriverList.m_pRegistrees->find(Rage::ci_ascii_string{drivers_to_try[to_try].c_str()})
				== m_pDriverList.m_pRegistrees->end())
			{
				LOG->Warn("Removed unusable sound driver %s", drivers_to_try[to_try].c_str());
				drivers_to_try.erase(drivers_to_try.begin() + to_try);
				had_to_erase= true;
			}
			else
			{
				++to_try;
			}
		}
		if(had_to_erase)
		{
			SOUNDMAN->fix_bogus_sound_driver_pref(Rage::join(",", drivers_to_try));
		}
		if(drivers_to_try.empty())
		{
			drivers_to_try = Rage::split(DEFAULT_SOUND_DRIVER_LIST, ",", Rage::EmptyEntries::skip);
		}
	}

	for (auto const &Driver: drivers_to_try)
	{
		RageDriver *pDriver = m_pDriverList.Create( Driver );
		if( pDriver == nullptr )
		{
			LOG->Trace( "Unknown sound driver: %s", Driver.c_str() );
			continue;
		}

		RageSoundDriver *pRet = dynamic_cast<RageSoundDriver *>( pDriver );
		ASSERT( pRet != nullptr );

		const std::string sError = pRet->Init();
		if( sError.empty() )
		{
			LOG->Info( "Sound driver: %s", Driver.c_str() );
			return pRet;
		}
		LOG->Info( "Couldn't load driver %s: %s", Driver.c_str(), sError.c_str() );
		Rage::safe_delete( pRet );
	}
	return nullptr;
}

std::string RageSoundDriver::GetDefaultSoundDriverList()
{
	return DEFAULT_SOUND_DRIVER_LIST;
}

/*
 * (c) 2002-2006 Glenn Maynard, Steve Checkoway
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
