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

#ifndef f_CRASH_H
#define f_CRASH_H

#ifdef f_CRASH_CPP
#define EXTERN
#else
#define EXTERN extern
#endif

struct VirtualDubCheckpoint {
	const char *file;
	int line;
	const char *message;

	inline void set(const char *f, int l, const char *m=NULL) { file=f; line=l; message=m; }
};

#define CHECKPOINT_COUNT		(16)

struct VirtualDubThreadState {
	const char				*pszThreadName;
	unsigned long			dwThreadId;
	void *					hThread;

	VirtualDubCheckpoint	cp[CHECKPOINT_COUNT];
	int						nNextCP;
};

EXTERN __declspec(thread) VirtualDubThreadState g_PerThreadState;

#define VDCHECKPOINT (g_PerThreadState.cp[g_PerThreadState.nNextCP++&(CHECKPOINT_COUNT-1)].set(__FILE__, __LINE__))
#define VDCHECKPOINT_M(m) (g_PerThreadState.cp[g_PerThreadState.nNextCP++&(CHECKPOINT_COUNT-1)].set(__FILE__, __LINE__, (m)))

void VirtualDubInitializeThread(const char *pszName);
void VirtualDubDeinitializeThread();
extern long __stdcall CrashHandler(struct _EXCEPTION_POINTERS *ExceptionInfo);

void CrashLog(const char *str);

/* Exactly as advertised. */
void crash();

#endif
