//	disasm - Disassembly module compiler for VirtualDub
//	Copyright (C) 2002 Avery Lee, All Rights Reserved
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

#pragma warning(disable: 4786)

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <conio.h>
#include <assert.h>

#include <string>
#include <list>
#include <vector>
#include <utility>
#include <algorithm>

struct VDDisassemblyContext {
	const unsigned char **pRuleSystem;
        long (*pSymLookup)(VDDisassemblyContext *pctx, unsigned long virtAddr, char *buf, int buf_len);

	bool bSizeOverride;			// 66
	bool bAddressOverride;		// 67
	bool bRepnePrefix;			// F2
	bool bRepePrefix;			// F3
	const char *pszSegmentOverride;

	long	physToVirtOffset;

	char	heap[2048];
	int		stack[32];
};

static char *VDDisasmMatchRule(VDDisassemblyContext *pContext, const unsigned char *source, const unsigned char *pattern, int pattern_len, int bytes, int *sp, char *hp, const unsigned char *&source_end);

char *strtack(char *s, const char *t) {
	while(*s = *t)
		++s, ++t;

	return s;
}


// 1-15 are static lookups
static const char kTarget_r32		= 1;
static const char kTarget_r16		= 2;
static const char kTarget_r8		= 3;
static const char kTarget_rm		= 4;
static const char kTarget_rx		= 5;
static const char kTarget_rc		= 6;
static const char kTarget_rd		= 7;
static const char kTarget_rs		= 8;
static const char kTarget_rf		= 9;

// 16-31 are dynamic translations
static const char kTarget_r1632		= 16;
static const char kTarget_rmx		= 17;
static const char kTarget_x			= 18;
static const char kTarget_hx		= 19;
static const char kTarget_lx		= 20;
static const char kTarget_s			= 21;
static const char kTarget_o			= 22;
static const char kTarget_ho		= 23;
static const char kTarget_lo		= 24;
static const char kTarget_a			= 25;
static const char kTarget_ha		= 26;
static const char kTarget_la		= 27;

static const char kTarget_ap		= (char)224;
static const char kTarget_p_cs		= (char)225;
static const char kTarget_p_ss		= (char)226;
static const char kTarget_p_ds		= (char)227;
static const char kTarget_p_es		= (char)228;
static const char kTarget_p_fs		= (char)229;
static const char kTarget_p_gs		= (char)230;
static const char kTarget_p_66		= (char)231;
static const char kTarget_p_67		= (char)232;
static const char kTarget_p_F2		= (char)233;
static const char kTarget_p_F3		= (char)234;


///////////////////////////////////////////////////////////////////////

static void *VDDisasmDecompress(void *_dst, const unsigned char *src, int src_len) {
	const unsigned char *src_limit = src + src_len;
	unsigned char *dst = (unsigned char *)_dst;

	// read ruleset count
	int rulesets = *src++;
	unsigned char **prstab = (unsigned char **)dst;

	dst += sizeof(unsigned char *) * (rulesets + 1);

	// decompress rulesets sequentially
	for(int rs=0; rs<rulesets; ++rs) {
		prstab[rs+1] = dst;

		const unsigned char *pattern_cache[4][2];
		const unsigned char *result_cache[4][2];

		while(src[0] || src[1]) {
			unsigned char packctl;
			int packsrc, cnt;

			// read pack control byte and copy prematch-literal-postmatch for pattern
			packctl = *src++;
			packsrc = packctl >> 6;

			int prematch = (packctl & 7) * 2;
			int postmatch = ((packctl>>3) & 7) * 2;
			int literal = (*src++ - 1) * 2;

			*dst++ = literal + prematch + postmatch;

			const unsigned char *pattern_start = dst;

			for(cnt=0; cnt<prematch; ++cnt)
				*dst++ = pattern_cache[packsrc][0][cnt];

			for(cnt=0; cnt<literal; ++cnt)
				*dst++ = *src++;

			for(cnt=0; cnt<postmatch; ++cnt)
				*dst++ = pattern_cache[packsrc][1][cnt-postmatch];

			// cycle pattern cache

			for(cnt=3; cnt>0; --cnt) {
				pattern_cache[cnt][0] = pattern_cache[cnt-1][0];
				pattern_cache[cnt][1] = pattern_cache[cnt-1][1];
			}

			pattern_cache[0][0] = pattern_start;
			pattern_cache[0][1] = dst;

			// read pack control byte and copy prematch-literal-postmatch for result

			packctl = *src++;
			packsrc = packctl >> 6;

			prematch = (packctl & 7);
			postmatch = ((packctl>>3) & 7);
			literal = (*src++ - 1);

			*dst++ = prematch + postmatch + literal;

			const unsigned char *result_start = dst;

			for(cnt=0; cnt<prematch; ++cnt)
				*dst++ = result_cache[packsrc][0][cnt];

			for(cnt=0; cnt<literal; ++cnt)
				*dst++ = *src++;

			for(cnt=0; cnt<postmatch; ++cnt)
				*dst++ = result_cache[packsrc][1][cnt-postmatch];

			// cycle result cache

			for(cnt=3; cnt>0; --cnt) {
				result_cache[cnt][0] = result_cache[cnt-1][0];
				result_cache[cnt][1] = result_cache[cnt-1][1];
			}

			result_cache[0][0] = result_start;
			result_cache[0][1] = dst;
		}

		src += 2;

		*dst++ = 0;
		*dst++ = 0;
	}

	prstab[0] = prstab[rulesets];

	return dst;
}

static long VDDisasmPack32(const int *src) {
	return src[0] + (src[1]<<8) + (src[2]<<16) + (src[3]<<24);
}

static void VDDisasmExpandRule(VDDisassemblyContext *pContext, char *s, const unsigned char *result, const int *sp_base, const unsigned char *source) {
	static const char *const reg32[8]={"eax","ecx","edx","ebx","esp","ebp","esi","edi"};
	static const char *const reg16[8]={"ax","cx","dx","bx","sp","bp","si","di"};
	static const char *const reg8[8]={"al","cl","dl","bl","ah","ch","dh","bh"};
	static const char *const regmmx[8]={"mm0","mm1","mm2","mm3","mm4","mm5","mm6","mm7"};
	static const char *const regxmm[8]={"xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"};
	static const char *const regcrn[8]={"cr0","cr1","cr2","cr3","cr4","cr5","cr6","cr7"};
	static const char *const regdrn[8]={"dr0","dr1","dr2","dr3","dr4","dr5","dr6","dr7"};
	static const char *const regseg[8]={"es","cs","ss","ds","fs","gs","?6s","?7s"};
	static const char *const regf[8]={"st(0)","st(1)","st(2)","st(3)","st(4)","st(5)","st(6)","st(7)"};

	static const char *const *const sStaticLabels[]={
		reg32,
		reg16,
		reg8,
		regmmx,
		regxmm,
		regcrn,
		regdrn,
		regseg,
		regf
	};

	const unsigned char *result_limit = result + result[0]+1;

	++result;

	while(result < result_limit) {
		char c = *result++;

		if ((unsigned char)(c&0x7f) < 32) {
			if (c & 0x80) {
				c &= 0x7f;
				*s++ = ' ';
			}

			static const unsigned char static_bitfields[8]={
				0x80, 0x30, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00
			};

			unsigned char control_byte = (unsigned char)*result++;
			unsigned char bitfield = static_bitfields[control_byte >> 5];

			if (!bitfield)
				bitfield = (unsigned char)*result++;

			int bf_start = bitfield & 15;
			int bf_siz = bitfield>>4;
			const char *arg_s = (const char *)sp_base[c-1];
			int arg = (sp_base[c-1] >> bf_start) & ((1<<bf_siz)-1);

			control_byte &= 0x1f;

			if (control_byte < 10) {
				s = strtack(s, sStaticLabels[control_byte-1][arg]);
			} else {
				long symoffset = 0;
				char *s_base = s;

				switch(control_byte) {
				case kTarget_r1632:
					s = strtack(s, reg32[arg] + pContext->bSizeOverride);
					break;
				case kTarget_rmx:
					s = strtack(s, (pContext->bSizeOverride ? regxmm : regmmx)[arg]);
					break;
				case kTarget_lx:
					symoffset = VDDisasmPack32(sp_base + c - 1);
					s += sprintf(s, "%08lx", symoffset);
					break;
				case kTarget_hx:
					s += sprintf(s, "%02x%02x"
								, (unsigned char)sp_base[c]
								, (unsigned char)sp_base[c-1]
								);
					break;
				case kTarget_x:
					s += sprintf(s, "%02x", arg);
					break;
				case kTarget_lo:
					symoffset = VDDisasmPack32(sp_base + c - 1);
					s += sprintf(s, "%c%02lx", symoffset<0 ? '-' : '+', abs(symoffset));
					break;
				case kTarget_ho:
					{
						short x =  ((unsigned char)sp_base[c  ] << 8)
								+  (unsigned char)sp_base[c-1];

						s += sprintf(s, "%c%02lx", x<0 ? '-' : '+', abs(x));
					}
					break;
				case kTarget_o:
					s += sprintf(s, "%c%02x", arg&0x80?'-':'+', abs((signed char)arg));
					break;
				case kTarget_la:
					symoffset = (long)source + VDDisasmPack32(sp_base + c - 1) + pContext->physToVirtOffset;
					s += sprintf(s, "%08lx", symoffset);
					break;
				case kTarget_ha:
					symoffset = (long)source + (signed short)(sp_base[c-1] + (sp_base[c]<<8)) + pContext->physToVirtOffset;
					s += sprintf(s, "%08lx", symoffset);
					break;
				case kTarget_a:
					symoffset = (long)source + (signed char)arg + pContext->physToVirtOffset;
					s += sprintf(s, "%08lx", symoffset);
					break;
				case kTarget_s:
					s = strtack(s, arg_s);
					break;
				}

				if (symoffset && pContext->pSymLookup) {
					char buf[1024];

					symoffset = pContext->pSymLookup(pContext, (unsigned long)symoffset, buf, sizeof buf - 16);

					if (symoffset >= 0) {
						int l = strlen(buf);
						int l2 = s - s_base;

						if (symoffset)
							l += sprintf(buf+l, "+%02x", symoffset);

						memmove(s_base+l+2, s_base, l2);
						memcpy(s_base, buf, l);
						s_base[l] = ' ';
						s_base[l+1] = '(';
						s = s_base + l+l2+2;
						*s++ = ')';
					}
				}
			}
		} else if ((unsigned char)c >= 0xe0) {
			switch(c) {
			case kTarget_ap:
				if (pContext->pszSegmentOverride) {
					s = strtack(s, pContext->pszSegmentOverride);
					*s++ = ':';
				}
				break;
			case kTarget_p_cs:	pContext->pszSegmentOverride = regseg[1];	break;
			case kTarget_p_ss:	pContext->pszSegmentOverride = regseg[2];	break;
			case kTarget_p_ds:	pContext->pszSegmentOverride = regseg[3];	break;
			case kTarget_p_es:	pContext->pszSegmentOverride = regseg[0];	break;
			case kTarget_p_fs:	pContext->pszSegmentOverride = regseg[4];	break;
			case kTarget_p_gs:	pContext->pszSegmentOverride = regseg[5];	break;
			case kTarget_p_66:	pContext->bSizeOverride = true;				break;
			case kTarget_p_67:	pContext->bAddressOverride = true;			break;
			case kTarget_p_F2:	pContext->bRepnePrefix = true;				break;
			case kTarget_p_F3:	pContext->bRepePrefix = true;				break;
			}
		} else
			*s++ = c;
	}

	*s = 0;
}

static char *VDDisasmApplyRuleset(VDDisassemblyContext *pContext, const unsigned char *rs, int *sp, char *hp, const unsigned char *source, int bytes, const unsigned char *&source_end) {
	char *hpr;

	while(rs[0] || rs[1]) {
		const unsigned char *src_end;
		const unsigned char *result = rs + rs[0] + 1;
		const unsigned char *match_next = result + result[0] + 1;

		hpr = VDDisasmMatchRule(pContext, source, rs+1, rs[0]>>1, bytes, sp, hp, src_end);

		if (hpr) {
			VDDisasmExpandRule(pContext, hpr, result, sp, src_end);

			source_end = src_end;
			return hpr;
		}

		rs = match_next;
	}

	return NULL;
}

static char *VDDisasmMatchRule(VDDisassemblyContext *pContext, const unsigned char *source, const unsigned char *pattern, int pattern_len, int bytes, int *sp, char *hp, const unsigned char *&source_end) {
	while(pattern_len) {
		if (!pattern[1] && pattern[0]) {
			if (pattern[0] & 0x80) {
				int count = pattern[0] & 0x3f;

				if (pattern[0] & 0x40) {
					--source;
					++bytes;
				}
			
				const unsigned char *src_end;

				hp = VDDisasmApplyRuleset(pContext, pContext->pRuleSystem[count+1], sp, hp, source, bytes, src_end);

				if (!hp)
					return NULL;

				*sp++ = *source;
				*sp++ = (int)hp;

				while(*hp++);

				bytes -= (src_end - source)-1;
				source = src_end;
			} else if (pattern[0] < 16) {
				if (pattern[0] > bytes)
					return NULL;

				for(int i=0; i<pattern[0]; ++i) {
					*sp++ = *source++;
				}

				bytes -= pattern[0]-1;
			} else {
				switch(pattern[0]) {
				case 16:	if (!pContext->bSizeOverride)		return NULL;	break;
				case 17:	if (!pContext->bAddressOverride)	return NULL;	break;
				case 18:	if (!pContext->bRepnePrefix)		return NULL;	break;
				case 19:	if (!pContext->bRepePrefix)			return NULL;	break;
				case 20:	if (pContext->pszSegmentOverride)	return NULL;	break;
				}
			}
		} else {
			if (!bytes)
				return NULL;

			unsigned char b = *source++;

			if ((b & pattern[1]) != pattern[0])
				return NULL;

			*sp++ = b;
		}
		pattern += 2;
		--bytes;
		--pattern_len;
	}

	if (!pattern_len) {
		source_end = source;
		return hp;
	}

	return NULL;
}

void VDDisassemble(VDDisassemblyContext *pvdc, const unsigned char *source, int bytes) {
	while(bytes > 0) {
		const unsigned char *src2 = source;
		const unsigned char *src_end;
		char *s;

		pvdc->bAddressOverride = false;
		pvdc->bSizeOverride = false;
		pvdc->bRepePrefix = false;
		pvdc->bRepnePrefix = false;
		pvdc->pszSegmentOverride = NULL;

		do {
			s = VDDisasmApplyRuleset(pvdc, pvdc->pRuleSystem[0], pvdc->stack, pvdc->heap, src2, bytes, src_end);

		for(int i = 0; i < src_end-src2; ++i)
			bytes -= (src_end - src2);
			src2 = src_end;
		} while(!*s && bytes);

		if (!bytes)
			break;

		int count = src_end - source;
		int linecnt;

		printf("%08lx:", (unsigned long)source + pvdc->physToVirtOffset);

		for(linecnt=0; linecnt<7 && source < src_end; ++linecnt)
			printf(" %02x", (unsigned char)*source++);

		char *t = s;
		while(*t && *t != ' ')
			++t;

		if (*t)
			*t++ = 0;

		printf("%*c%-7s%s\n", 2 + 3*(7-linecnt), ' ', s, t);

		// flush remaining bytes

		while(source < src_end) {
			printf("         ");
			for(linecnt=0; linecnt<7 && source < src_end; ++linecnt)
				printf(" %02x", (unsigned char)*source++);
			putchar('\n');
		}

		bytes -= count;
	}
}

///////////////////////////////////////////////////////////////////////////

typedef unsigned char byte;

struct rule {
	std::vector<std::pair<byte, byte> > match_stream;
	std::string result;
	std::string	rule_line;
	int argcount;
};

struct ruleset {
	std::list<rule>		rules;
	std::string			name;
};

typedef std::list<ruleset>				tRuleSystem;
tRuleSystem		g_RuleSystem;

void oops(const char *format, ...) {
	va_list val;

	va_start(val, format);
	vprintf(format, val);
	va_end(val);
	getch();
	exit(5);
}


void strtrim(char *s) {
	char *t = s;
	char *u = s;

	while(*t)
		++t;

	while(t>s && isspace((unsigned char)t[-1]))
		--t;

	while(u<t && isspace((unsigned char)*u))
		++u;

	memmove(s, u, t-u);
	s[t-u] = 0;
}
void parse_ia(FILE *f) {
	char linebuf[4096];
	ruleset		*pRuleset = NULL;

	while(fgets(linebuf, sizeof linebuf, f)) {
		strtrim(linebuf);

		if (!linebuf[0] || linebuf[0] == '#')
			continue;

		puts(linebuf);

		if (linebuf[0] == '%') {			// ruleset definition
			strtrim(linebuf+1);

			ruleset r;
			r.name = linebuf+1;
			g_RuleSystem.push_back(r);
			pRuleset = &g_RuleSystem.back();
		} else {							// rule definition

			if (!pRuleset)
				oops("Not in ruleset:\n>%s\n", linebuf);

			rule r;

			r.rule_line = linebuf;
			r.argcount = 0;

			// Find colon

			char *colon = linebuf;

			while(*colon != ':') {
				if (!*colon)
					oops("Colon missing in rule:\n>%s\n", linebuf);

				++colon;
			}

			// Nuke colon

			*colon++ = 0;

			// Parse tokens until colon is found

			static const char whitespace[]=" \t\n\v";
			const char *token = strtok(linebuf, whitespace);

			if (token) do {
				if (*token == '*') {						// any character
					if (!r.match_stream.empty() && !r.match_stream.rbegin()->second && r.match_stream.rbegin()->first < 15)
						++r.match_stream.rbegin()->first;
					else
						r.match_stream.push_back(std::pair<byte, byte>(1,0));

					++r.argcount;
				} else if (*token == '[') {
					if (!strcmp(token+1, "66]"))
						r.match_stream.push_back(std::pair<byte, byte>(16,0));
					else if (!strcmp(token+1, "67]"))
						r.match_stream.push_back(std::pair<byte, byte>(17,0));
					else if (!strcmp(token+1, "F2]"))
						r.match_stream.push_back(std::pair<byte, byte>(18,0));
					else if (!strcmp(token+1, "F3]"))
						r.match_stream.push_back(std::pair<byte, byte>(19,0));
					else if (!strcmp(token+1, "!s]"))
						r.match_stream.push_back(std::pair<byte, byte>(20,0));
					else
						oops("unknown prefix match token '%s'\n", token);
				} else if (isxdigit((unsigned char)token[0]) && isxdigit((unsigned char)token[1])
						&& (token[2] == '-' || !token[2])) {		// match character
					int byteval, byteend;
					int c;

					c = sscanf(token, "%x-%x", &byteval, &byteend);

					if (byteval < 0 || byteval >= 256)
						oops("byte start value out of range\n");

					if (c<2) {
						byteend = byteval;
					} else if (byteend != byteval) {
						if (byteend < 0 || byteend >= 256)
							oops("byte end value out of range\n");
					}

					r.match_stream.push_back(std::pair<byte, byte>(byteval, ~(byteval ^ byteend)));
					++r.argcount;

				} else {									// macro invocation
					tRuleSystem::iterator it = g_RuleSystem.begin();
					tRuleSystem::iterator itEnd = g_RuleSystem.end();
					int index = 128;

					if (*token == '!') {	// reuse last byte char
						index = 192;
						++token;
					}

					for(; it!=itEnd; ++it, ++index) {
						if (!stricmp((*it).name.c_str(), token))
							break;
					}

					if (it == itEnd)
						oops("unknown ruleset '%s'\n", token);

					r.match_stream.push_back(std::pair<byte, byte>(index, 0));
					r.argcount += 2;
				}
			} while(token = strtok(NULL, whitespace));

			// match sequence parsed -- parse the result string.

			char *s = colon;

			for(;;) {
				while(*s && strchr(whitespace, *s))
					++s;

				if (!*s || *s == '#')
					break;

				if (*s == '"') {	// string literal
					const char *start = ++s;

					while(*s != '"') {
						if (!*s)
							oops("unterminated string constant\n");

						++s;
					}
					
//					r.result.append(start, s);
					r.result.append(start, s-start);
					++s;
				} else if (*s == '$') {	// macro expansion
					++s;

					if (!strnicmp(s, "p_cs", 4)) {
						r.result += kTarget_p_cs;
						s += 4;
					} else if (!strnicmp(s, "p_ss", 4)) {
						r.result += kTarget_p_ss;
						s += 4;
					} else if (!strnicmp(s, "p_ds", 4)) {
						r.result += kTarget_p_ds;
						s += 4;
					} else if (!strnicmp(s, "p_es", 4)) {
						r.result += kTarget_p_es;
						s += 4;
					} else if (!strnicmp(s, "p_fs", 4)) {
						r.result += kTarget_p_fs;
						s += 4;
					} else if (!strnicmp(s, "p_gs", 4)) {
						r.result += kTarget_p_gs;
						s += 4;
					} else if (!strnicmp(s, "p_66", 4)) {
						r.result += kTarget_p_66;
						s += 4;
					} else if (!strnicmp(s, "p_67", 4)) {
						r.result += kTarget_p_67;
						s += 4;
					} else if (!strnicmp(s, "p_F2", 4)) {
						r.result += kTarget_p_F2;
						s += 4;
					} else if (!strnicmp(s, "p_F3", 4)) {
						r.result += kTarget_p_F3;
						s += 4;
					} else if (!strnicmp(s, "ap", 2)) {
						r.result += kTarget_ap;
						s += 2;
					} else {
						unsigned long id = strtoul(s, &s, 10);

						if (!id || id > r.argcount)
							oops("macro argument $%lu out of range\n", id);

						if (!r.result.empty() && *r.result.rbegin() == ' ')
							*r.result.rbegin() = (char)(id + 0x80);
						else
							r.result += (char)id;

						int firstbit = 0;
						int lastbit = 7;

						if (*s == '[') {
							++s;

							firstbit = strtol(s, &s, 10);

							if (*s++ != '-')
								oops("macro argument bitfield range missing '-'\n");

							lastbit = strtol(s, &s, 10);

							if (firstbit < 0 || lastbit > 7 || firstbit > lastbit)
								oops("invalid bitfield %d-%d\n", firstbit, lastbit);

							if (*s++ != ']')
								oops("invalid bitfield\n");
						}

						if (!*s)
							oops("macro expansion missing format\n");

						char *t = s;

						while(*t && !isspace((unsigned char)*t))
							++t;

						*t = 0;

						char control_byte;

						if (!stricmp(s, "r32")) {
							control_byte = kTarget_r32;
						} else if (!stricmp(s, "r16")) {
							control_byte = kTarget_r16;
						} else if (!stricmp(s, "r1632")) {
							control_byte = kTarget_r1632;
						} else if (!stricmp(s, "r8")) {
							control_byte = kTarget_r8;
						} else if (!stricmp(s, "rm")) {
							control_byte = kTarget_rm;
						} else if (!stricmp(s, "rx")) {
							control_byte = kTarget_rx;
						} else if (!stricmp(s, "rmx")) {
							control_byte = kTarget_rmx;
						} else if (!stricmp(s, "rc")) {
							control_byte = kTarget_rc;
						} else if (!stricmp(s, "rd")) {
							control_byte = kTarget_rd;
						} else if (!stricmp(s, "rs")) {
							control_byte = kTarget_rs;
						} else if (!stricmp(s, "rf")) {
							control_byte = kTarget_rf;
						} else if (!stricmp(s, "x")) {
							control_byte = kTarget_x;
						} else if (!stricmp(s, "hx")) {
							control_byte = kTarget_hx;
						} else if (!stricmp(s, "lx")) {
							control_byte = kTarget_lx;
						} else if (!stricmp(s, "o")) {
							control_byte = kTarget_o;
						} else if (!stricmp(s, "ho")) {
							control_byte = kTarget_ho;
						} else if (!stricmp(s, "lo")) {
							control_byte = kTarget_lo;
						} else if (!stricmp(s, "a")) {
							control_byte = kTarget_a;
						} else if (!stricmp(s, "ha")) {
							control_byte = kTarget_ha;
						} else if (!stricmp(s, "la")) {
							control_byte = kTarget_la;
						} else if (!stricmp(s, "s")) {
							control_byte = kTarget_s;
						} else {
							oops("unknown macro expansion mode: '%s'\n", s);
						}

						if (firstbit == 0 && lastbit == 2) {
							r.result += (char)(control_byte + 0x20);
						} else if (firstbit == 3 && lastbit == 5) {
							r.result += (char)(control_byte + 0x40);
						} else if (firstbit != 0 || lastbit != 7) {
							r.result += (char)(control_byte + 0xe0);
							r.result += (char)((lastbit+1-firstbit)*16 + firstbit);
						} else {
							r.result += (char)control_byte;
						}

						s = t+1;
					}
				} else
					oops("indecipherable result string\n");
			}

			pRuleset->rules.push_back(r);
		}
	}
}

#define iterate_forward(type, obj, it) if(0);else for(type::iterator it = (obj).begin(), it##End = (obj).end(); it != it##End; ++it)

std::vector<char> ruleTestHeap;

void dump_ia(FILE *f) {
	std::vector<char> ruleHeap;
	long decomp_bytes = 0;
	long packed_bytes = 0;

	ruleHeap.push_back(g_RuleSystem.size());

	iterate_forward(tRuleSystem, g_RuleSystem, it) {
		ruleset& rs = *it;
		std::vector<std::pair<byte, byte> > last_match[4];
		std::string last_result[4];

		iterate_forward(std::list<rule>, rs.rules, itRule) {
			rule& r = *itRule;
			std::vector<char>::size_type s, l;
			int prematch, postmatch;
			int i, x, ibest;
			
			l = r.match_stream.size();

			ibest = 0;
			prematch = postmatch = 0;

			for(i=0; i<4; ++i) {
				int tprematch = std::mismatch(last_match[i].begin(), last_match[i].end(), r.match_stream.begin()).first - last_match[i].begin();
				int tpostmatch = std::mismatch(last_match[i].rbegin(), last_match[i].rend(), r.match_stream.rbegin()).first - last_match[i].rbegin();

				if (tprematch+tpostmatch > prematch+postmatch) {
					prematch = tprematch;
					postmatch = tpostmatch;
					ibest = i;
				}
			}

			if (prematch > 7)
				prematch = 7;

			if (postmatch > 7)
				postmatch = 7;

			if (postmatch > l - prematch)
				postmatch = l - prematch;

			ruleHeap.push_back(ibest*64 + postmatch*8 + prematch);
			ruleHeap.push_back(1+l - prematch - postmatch);

			for(x=prematch; x<l - postmatch; ++x) {
				ruleHeap.push_back(r.match_stream[x].first);
				ruleHeap.push_back(r.match_stream[x].second);
			}

			decomp_bytes += l*2+1;

			std::rotate(last_match, last_match+3, last_match+4);
			last_match[0] = r.match_stream;

			//////////////

			l = r.result.size();

			ibest = 0;
			prematch = postmatch = 0;

			for(i=0; i<4; ++i) {
				int tprematch = std::mismatch(last_result[i].begin(), last_result[i].end(), r.result.begin()).first - last_result[i].begin();
				int tpostmatch = std::mismatch(last_result[i].rbegin(), last_result[i].rend(), r.result.rbegin()).first - last_result[i].rbegin();

				if (tprematch+tpostmatch > prematch+postmatch) {
					prematch = tprematch;
					postmatch = tpostmatch;
					ibest = i;
				}
			}

			if (prematch > 7)
				prematch = 7;

			if (postmatch > 7)
				postmatch = 7;

			if (postmatch > l - prematch)
				postmatch = l - prematch;

			ruleHeap.push_back(ibest*64 + postmatch*8 + prematch);
			ruleHeap.push_back(1+l - prematch - postmatch);
			s = ruleHeap.size();
			ruleHeap.resize(s + l - prematch - postmatch);
			std::copy(r.result.begin() + prematch, r.result.begin() + l - postmatch, &ruleHeap[s]);

			decomp_bytes += l+1;

			std::rotate(last_result, last_result+3, last_result+4);
			last_result[0] = r.result;
		}

		ruleHeap.push_back(0);
		ruleHeap.push_back(0);

		decomp_bytes += 2;
	}

	static const char header[64]="[01|01] StepMania disasm module (IA32:P4/Athlon V1.00)\r\n\x1A";

	fwrite(header, 64, 1, f);

	packed_bytes = ruleHeap.size();
	fwrite(&packed_bytes, 4, 1, f);

	decomp_bytes += g_RuleSystem.size() * 4 + 4;

	fwrite(&decomp_bytes, 4, 1, f);

	fwrite(&ruleHeap[0], packed_bytes, 1, f);
	
	ruleTestHeap.resize(decomp_bytes);
	void *dst_end = VDDisasmDecompress(&ruleTestHeap[0], (const unsigned char *)&ruleHeap[0], packed_bytes);
}


void __declspec(naked) test1() {
	__asm {
//		pavgusb		mm0,[eax]
//		prefetch	[eax]
//		prefetchw	[eax]
//		pswapd		mm1, mm0
///		push		[eax]
//		push		word ptr [eax]
/*
		cvtsi2ss	xmm4, ecx
		cvtsi2ss	xmm4, [ecx]
		cvtpi2ps	xmm4, mm2
		cvtpi2ps	xmm4, [ecx]

		cvtss2si	eax, xmm4
		cvtss2si	eax, [ecx]
		cvtps2pi	mm2, xmm4
		cvtps2pi	mm2, [ecx]

		cvttss2si	eax, xmm4
		cvttss2si	eax, [ecx]
		cvttps2pi	mm2, xmm4
		cvttps2pi	mm2, [ecx]

		cvtsi2sd	xmm4, ecx
		cvtsi2sd	xmm4, [ecx]
		cvtpi2pd	xmm4, mm2
		cvtpi2pd	xmm4, [ecx]

		cvtsd2si	eax, xmm4
		cvtsd2si	eax, [ecx]
		cvtpd2pi	mm2, xmm4
		cvtpd2pi	mm2, [ecx]

		cvttsd2si	eax, xmm4
		cvttsd2si	eax, [ecx]
		cvttpd2pi	mm2, xmm4
		cvttpd2pi	mm2, [ecx]

		rep movsw
		lock rep movs es:word ptr [edi], cs:word ptr [esi]

		lock mov cs:dword ptr [eax+ecx*4+12300000h], 12345678h
*/
		__emit 0x2e
		jc x1

		__emit 0x3e
		jc y1

		call esi

		ret
x1:
y1:
		nop

	}
}

///////////////////////////////////////////////////////////////////////////

long symLookup(VDDisassemblyContext *pctx, unsigned long virtAddr, char *buf, int buf_len) {
	unsigned long offs;

	if ((offs = (virtAddr - (unsigned long)symLookup)) < 256) {
		strcpy(buf, "symLookup");
		return (long)offs;
	}

	if ((offs = (virtAddr - (unsigned long)VDDisassemble)) < 256) {
		strcpy(buf, "VDDisassemble");
		return (long)offs;
	}

	if ((offs = (virtAddr - (unsigned long)VDDisasmApplyRuleset)) < 256) {
		strcpy(buf, "VDDisasmApplyRuleset");
		return (long)offs;
	}

	if ((offs = (virtAddr - (unsigned long)printf)) < 16) {
		strcpy(buf, "printf");
		return (long)offs;
	}

	return -1;
}

int main(int argc, char **argv) {
	FILE *f = fopen(argc>1?argv[1]:"ia32.txt", "r");
	parse_ia(f);
	fclose(f);

	f = fopen(argc>2?argv[2]:"ia32.vdi", "wb");
	dump_ia(f);
	fclose(f);

//	disassemble((const byte *)&parse_ia, 2048);
//	disassemble((const byte *)&test1, 300);

	VDDisassemblyContext vdc;

	vdc.pRuleSystem = (const unsigned char **)&ruleTestHeap[0];
	vdc.pSymLookup = symLookup;
	vdc.physToVirtOffset = 0;

/*	{
		char buf[1024];
		int bufl;
		FILE *f = fopen("c:/temp/stepmania/buf2", "r");
		bufl = fread(buf, 1, 1024, f);
		assert(bufl);
		VDDisassemble(&vdc, (const byte *)&buf, bufl);
x}
*/

//	VDDisassemble(&vdc, (const byte *)&test1, 1024);
	VDDisassemble(&vdc, (const byte *)&VDDisassemble, 1024);

	getch();

	return 0;
}
