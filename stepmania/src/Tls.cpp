//	from VirtualDub
//	Copyright (C) 1998-2001 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "tls.h"
#include "crash.h"

__declspec(thread) VirtualDubTLSData g_tlsdata;

void InitThreadData(const char *pszName) {
	VirtualDubTLSData *pData = &g_tlsdata;

	__asm {
		mov		eax,pData
		mov		dword ptr fs:[14h],eax
	}

	VirtualDubInitializeThread(pszName);
}

void DeinitThreadData() {
	VirtualDubDeinitializeThread();
}

