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

#ifndef f_DISASM_H
#define f_DISASM_H

#include <windows.h>

struct VDDisassemblyContext {
	const unsigned char **pRuleSystem;
	long (*pSymLookup)(VDDisassemblyContext *pctx, unsigned long virtAddr, char *buf, int buf_len);

	bool bSizeOverride;			// 66
	bool bAddressOverride;		// 67
	bool bRepnePrefix;			// F2
	bool bRepePrefix;			// F3
	const char *pszSegmentOverride;

	long	physToVirtOffset;

	void	*pRawBlock;
	char	*heap;
	char	*heap_limit;
	int		*stack;

	void	*pExtraData;
	int		cbExtraData;
};


bool VDDisasmInit(VDDisassemblyContext *, const char *, const char *);
void VDDisasmDeinit(VDDisassemblyContext *);
char *VDDisassemble(VDDisassemblyContext *pvdc, const unsigned char *source, int bytes, int& count);



class CodeDisassemblyWindow {
private:
	static BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

	void *code, *rbase, *abase;
	long length;
	void *pFault;

	class lbent {
	public:
		unsigned char *ip;
		int len;
	} *lbents;
	int num_ents;

	HFONT hFontMono;

	char buf[256];

public:
	VDDisassemblyContext vdc;

	CodeDisassemblyWindow(void *code, long, void *, void *);
	~CodeDisassemblyWindow();

	void DoInitListbox(HWND hwndList);
	BOOL DoMeasureItem(LPARAM lParam);
	BOOL DoDrawItem(LPARAM lParam);
	void parse();
	BOOL post(HWND);
	long getInstruction(char *buf, long val);

	void setFaultAddress(void *_pFault) {
		pFault = _pFault;
	}
};

#endif
