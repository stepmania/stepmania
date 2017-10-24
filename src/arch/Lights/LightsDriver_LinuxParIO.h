/* LightsDriver_LinuxParallel - Parallel port-based lights for Linux using ParIO */

#ifndef LightsDriver_LinuxParIO_H
#define LightsDriver_LinuxParIO_H

#include "LightsDriver.h"
#include <pario.h>

class LightsDriver_LinuxParIO : public LightsDriver
{
public:
	LightsDriver_LinuxParIO();

	virtual ~LightsDriver_LinuxParIO();
	virtual void Set( const LightsState *ls );
private:
	ParIOPort m_cabPort;
	ParIOPort m_padPort;
};

#endif

/*
 * LightsDriver_LinuxParIO
 * Based on LightsDriver_LinuxParallel 
 * (c) 2004 Hugo Hromic M. <hhromic@udec.cl>
 *
 * (c) 2017 Gareth Francis. <gfrancis.dev@gmail.com>
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
