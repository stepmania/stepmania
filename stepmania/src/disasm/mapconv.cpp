//	mapconv - symbolic debugging info generator for VirtualDub
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

#include <vector>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CNAMBUF		(0x20000)
#define MAX_FNAMBUF		(0x200000)
#define MAX_SEGMENTS	(64)
#define MAX_GROUPS		(64)

struct RVAEnt {
	long rva;
	char *line;
};

std::vector<RVAEnt> rvabuf;

char fnambuf[MAX_FNAMBUF];
char *fnamptr = fnambuf;

long segbuf[MAX_SEGMENTS][2];
int segcnt=0;
int seggrp[MAX_SEGMENTS];
long grpstart[MAX_GROUPS];

char line[8192];
long codeseg_flags = 0;
FILE *f, *fo;

char *strtack(char *s, const char *t, const char *s_max) {
	while(s < s_max && (*s = *t))
		++s, ++t;

	if (s == s_max)
		return NULL;

	return s+1;
}

bool readline() {
	if (!fgets(line, sizeof line, f))
		return false;

	int l = strlen(line);

	if (l>0 && line[l-1]=='\n')
		line[l-1]=0;

	return true;
}

bool findline(const char *searchstr) {
	while(readline()) {
		if (strstr(line, searchstr))
			return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////

void parsename(long rva, char *func_name) {
	fnamptr = strtack(fnamptr, func_name, fnambuf+MAX_FNAMBUF);
	if(!fnamptr)
		throw "Too many func names; increase MAX_FNAMBUF.";
}

struct RVASorter {
	bool operator()(const RVAEnt& e1, const RVAEnt& e2) {
		return e1.rva < e2.rva;
	}
};

int main(int argc, char **argv) {
	int ver=0;
	int i;
	long load_addr;

	if (argc<3) {
		printf("mapconv <listing-file> <output-name>\n");
		return 0;
	}

	if (f=fopen("version.bin", "rb")) {
		fread(&ver,4,1,f);
		fclose(f);
	} else {
		printf("can't read version file\n");
		return 20;
	}

	if (!(f=fopen(argv[1], "r"))) {
		printf("can't open listing file \"%s\"\n", argv[1]);
		return 20;
	}

	if (!(fo=fopen(argv[2], "wb"))) {
		printf("can't open output file \"%s\"\n", argv[2]);
		return 20;
	}

	// Begin parsing file

	try {
		line[0] = 0;

//		printf("Looking for segment list.\n");

		if (!findline("Start         Length"))
			throw "can't find segment list";

//		printf("Reading in segment list.\n");

		while(readline()) {
			long grp, start, len;

			if (3!=sscanf(line, "%lx:%lx %lx", &grp, &start, &len))
				break;

			if (strstr(line+49, "CODE")) {
//				printf("%04x:%08lx %08lx type code\n", grp, start, len);

				codeseg_flags |= 1<<grp;

				segbuf[segcnt][0] = start;
				segbuf[segcnt][1] = len;
				seggrp[segcnt] = grp;
				++segcnt;
			}
		}

//		printf("Looking for public symbol list.\n");

		if (!findline("Publics by Value"))
			throw "Can't find public symbol list.";

		readline();

//		printf("Found public symbol list.\n");

		while(readline()) {
			long grp, start, rva;
			char symname[2048];
			int i;

			if (4!=sscanf(line, "%lx:%lx %s %lx", &grp, &start, symname, &rva))
				break;

			if (!(codeseg_flags & (1<<grp)))
				continue;

			RVAEnt entry = { rva, strdup(line) };

			rvabuf.push_back(entry);

//			parsename(rva,symname);
		}

//		printf("Looking for static symbol list.\n");

		if (!findline("Static symbols"))
			printf("WARNING: No static symbols found!\n");
		else {
			readline();

			while(readline()) {
				long grp, start, rva;
				char symname[4096];

				if (4!=sscanf(line, "%lx:%lx %s %lx", &grp, &start, symname, &rva))
					break;

				if (!(codeseg_flags & (1<<grp)))
					continue;

				RVAEnt entry = { rva, strdup(line) };

				rvabuf.push_back(entry);

	//			parsename(rva,symname);
			}
		}

//		printf("Sorting RVA entries...\n");

		std::sort(rvabuf.begin(), rvabuf.end(), RVASorter());

//		printf("Processing RVA entries...\n");

		for(i=0; i<rvabuf.size(); i++) {
			long grp, start, rva;
			char symname[4096];

			sscanf(rvabuf[i].line, "%lx:%lx %s %lx", &grp, &start, symname, &rva);

			grpstart[grp] = rva - start;

			parsename(rva, symname);
		}
		
//		printf("Processing segment entries...\n");

		for(i=0; i<segcnt; i++) {
			segbuf[i][0] += grpstart[seggrp[i]];
//			printf("\t#%-2d  %08lx-%08lx\n", i+1, segbuf[i][0], segbuf[i][0]+segbuf[i][1]-1);
		}
/*
		printf("Raw statistics:\n");
		printf("\tRVA bytes:        %ld\n", rvabuf.size()*4);
		printf("\tFunc name bytes:  %ld\n", fnamptr - fnambuf);

		printf("\nPacking RVA data..."); fflush(stdout);
*/
		std::vector<RVAEnt>::iterator itRVA = rvabuf.begin(), itRVAEnd = rvabuf.end();
		std::vector<char> rvaout;
		long firstrva = (*itRVA++).rva;
		long lastrva = firstrva;

		for(; itRVA != itRVAEnd; ++itRVA) {
			long rvadiff = (*itRVA).rva - lastrva;

			lastrva += rvadiff;

			if (rvadiff & 0xF0000000) rvaout.push_back((char)(0x80 | ((rvadiff>>28) & 0x7F)));
			if (rvadiff & 0xFFE00000) rvaout.push_back((char)(0x80 | ((rvadiff>>21) & 0x7F)));
			if (rvadiff & 0xFFFFC000) rvaout.push_back((char)(0x80 | ((rvadiff>>14) & 0x7F)));
			if (rvadiff & 0xFFFFFF80) rvaout.push_back((char)(0x80 | ((rvadiff>> 7) & 0x7F)));
			rvaout.push_back((char)(rvadiff & 0x7F));
		}

//		printf("%ld bytes\n", rvaout.size());

		// dump data

		static const char header[64]="StepMania symbolic debug information\r\n\x1A";

		fwrite(header, 64, 1, fo);

		long t;

		t = ver;
		fwrite(&t, 4, 1, fo);

		t = rvaout.size() + 4;
		fwrite(&t, 4, 1, fo);

		t = fnamptr - fnambuf;
		fwrite(&t, 4, 1, fo);

		t = segcnt;
		fwrite(&t, 4, 1, fo);

		fwrite(&firstrva, 4, 1, fo);
		fwrite(&rvaout[0], rvaout.size(), 1, fo);
		fwrite(fnambuf, fnamptr - fnambuf, 1, fo);
		fwrite(segbuf, segcnt*8, 1, fo);

		// really all done

		if (fclose(fo))
			throw "output file close failed";
		
	} catch(const char *s) {
		fprintf(stderr, "%s: %s\n", argv[1], s);
	}

	fclose(f);

	return 0;
}
