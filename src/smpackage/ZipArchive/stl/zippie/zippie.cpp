/*
 * $Archive: /zippie/zippie.cpp $
 * $Author$
 *
 * $History: zippie.cpp $
 * 
 * *****************  Version 4  *****************
 * User: Tadeusz Dracz Date: 02-04-01   Time: 2:05
 * Updated in $/zippie
 * 
 * *****************  Version 3  *****************
 * User: Tadeusz Dracz Date: 02-01-19   Time: 18:01
 * Updated in $/zippie
 * 
 * *****************  Version 2  *****************
 * User: Tadeusz Dracz Date: 01-11-08   Time: 19:54
 * Updated in $/zippie
 * added support for wildcards when extracting or deleting
 * 
 */
/////////////////////////////////////////////////////////////////////////////////
// zippie.cpp : Defines the entry point for the console application.
// An STL program that uses the ZipArchive library
//  
// Copyright (C) 2000 - 2003 Tadeusz Dracz.
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// For the licensing details see the file License.txt
////////////////////////////////////////////////////////////////////////////////


#ifdef __GNUC__
	#include "ZipArchive.h"
	#include "ZipPlatform.h"
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <dirent.h>
	#include <fnmatch.h>
	#include <unistd.h>
#else
	#include "../../ZipArchive.h"
	#include "../../ZipPlatform.h"
#endif
#include "CmdLine.h"
#include <stdlib.h>
#include <list>
#include <time.h>
#include <stdio.h>


ZIPSTRINGCOMPARE pZipComp;


struct CZipAddFileInfo
{
	CZipAddFileInfo(const CZipString& szFilePath, const CZipString& szFileNameInZip)
		:m_szFilePath(szFilePath),	m_szFileNameInZip(szFileNameInZip)
	{
		int is = szFileNameInZip.GetLength();
		m_iSeparators = 0;
		for (int i = 0; i < is; i++)
			if (CZipPathComponent::IsSeparator(szFileNameInZip[i]))
				m_iSeparators++;
	}
	CZipString m_szFilePath, m_szFileNameInZip;
	
	bool CheckOrder(const CZipString& sz1, const CZipString& sz2, 
		int iSep1, int iSep2, bool bCheckTheBeginning = false) const
	{
		if (iSep1)
		{
			if (iSep2)
			{
				if (iSep1 == iSep2)
					return (sz1.*pZipComp)(sz2) < 0;

				if (bCheckTheBeginning)
				{

					int is = sz1.GetLength() > sz2.GetLength() ? sz2.GetLength() : sz1.GetLength();
					int iSeparators = 0;
					// find the common path beginning
					int iLastSepPos = -1;
					for (int i = 0; i < is; i++)
					{
						CZipString sz = sz2.Mid(i, 1);
						if ((sz1.Mid(i, 1).*pZipComp)(sz) != 0) // must be Mid 'cos of case sens. here
							break;
						else if (CZipPathComponent::IsSeparator(sz[0]))
						{
							iLastSepPos = i;
							iSeparators++;
						}
					}
					
					// if necessary remove the common path beginning and check again
					if (iLastSepPos != -1)
						return CheckOrder(sz1.Mid(iLastSepPos), sz2.Mid(iLastSepPos), iSep1 - iSeparators, iSep2 - iSeparators);
							
				}
				return (sz1.*pZipComp)(sz2) < 0;		
			} 
			else
				return false;
		}
		else
			if (iSep2)
				return true;
			else
				return (sz1.*pZipComp)(sz2) < 0;
	}
	bool operator>(const CZipAddFileInfo& wz) const
	{
		bool b = CheckOrder(m_szFileNameInZip, wz.m_szFileNameInZip,
			m_iSeparators, wz.m_iSeparators, true);
		return b;
	}
protected:
	int m_iSeparators; // for a sorting puroposes
};


typedef list<CZipString> FILELIST;
typedef list<CZipString>::iterator FILELISTIT;
typedef list<struct CZipAddFileInfo> FILELISTADD;
typedef list<struct CZipAddFileInfo>::iterator FILELISTADDIT;

struct AddDirectoryInfo
{
	AddDirectoryInfo(FILELIST& l):m_l(l){}
	FILELIST& m_l;
	CZipString m_lpszFile;
	bool m_bRecursive;
	bool m_bAddEmpty;
};


void DisplayUsage()
{
		printf("\
\n\
Zippie v2.2\n\
Copyright (C) 2000 - 2003 Tadeusz Dracz\n\
E-Mail: tdracz@artpol-software.com\n\
Web   : http://www.artpol-software.com\n\
\n\
This program is free software; you can redistribute it and/or\n\
modify it under the terms of the GNU General Public License\n\
as published by the Free Software Foundation; either version 2\n\
of the License, or (at your option) any later version.\n\
\n\
This is a zipping and unzipping program.\n\
It was created using ZipArchive library in the STL version.\n\
\n\
\n\
USAGE: zippie <commands and (or) switches>\n\
\n\
Notes:\n\
	- the order of commands and switches doesn't matter\n\
	- if some files have spaces inside, put them inside quotation marks\n\
\n\
************  Default switches  ************\n\
\n\
-f <archive>[.zip]\n\
	<archive> file to create or open (can be with or without extension)\n\
	the extension .zip is added automatically if not present\n\
-st\n\
	use this switch if you're opening an existing disk spanning archive\n\
	in tdSpan mode on a removable device (or under Linux)\n\
	(doesn't matter for commands that modify the archive \n\
	e.g. add or delete commands)\n\
\n\
************  Add files to archive commands  ************\n\
\n\
-a <files>\n\
	add <files> to the <archive>; separate them with spaces;\n\
	you can use wildcards (*?)\n\
-af <file>\n\
	add files listed in <file> (one file per line);\n\
	no wildcards allowed in the files\n\
-ax <files>\n\
	prevent <files> from being added to the <archive>;\n\
	separate them with spaces; you can use wildcards (*?)\n\
-afx <file>\n\
	prevent files listed in <file> (one file per line) to be added \n\
	to the <archive>; no wildcards allowed in the files\n\
\n\
	you can use switches -a, -af, -ax, -afx simultaneously\n");
	printf("\
\n\
-r\n\
	recurse subdirectories; don't include in the <archive> directories\n\
	from which no file is added; under linux put file masks into \n\
	quotation marks otherwise they will be automatically expaned by \n\
	the shell and the recursion will be one level only\n\
-re\n\
	recurse subdirectories and add all subdirectories even empty to the\n\
	<archive>; see a note above about using file masks under Linux\n\
-u\n\
	this switch tells the program to not delete the <archive> if it\n\
	already	exists (and if it is not a disk spanning archive), but add\n\
	the <files> to it;\n\
	if this switch is not specified, there is a new archive created\n\
	even if one with the same name already exists (it is overwritten)\n\
-as [<path>]\n\
	(add smartly) if this switch is specified, in the disk spanning mode\n\
	the file is first compressed to a temporary directory to see whether \n\
	it is smaller after compression; if it is not it stored then without \n\
	a compression; (you can specify a <path> where the temporary files \n\
	will be created, if you do not specify it, they will be created in \n\
	the directory specified by the TMP system variable or if this variable\n\
	is not defined the file will be created in the current directory;\n\
	in normal archive if there is a such a situation the file is removed\n\
	from the archive and then stored again but with no compression \n\
	(the <path> argument is ignored)\n\
-c <number>\n\
	set the compression level (from 0 to 9);\n\
	if not specified, 5 is assumed\n\
	0 means no compression - only storing\n\
-v <size>\n\
	create a disk spanning archive\n\
	if <size> is 0 create archive in pkSpan mode,\n\
	if <size> is greater than 0 create archive in tdSpan mode\n\
\n");
		printf("\
************  Extract commands  ************\n\
\n\
-xp <path>\n\
	it specifies the path the files	will be extracted to; if not present,\n\
	the current path is assumed\n\
-x  <files>\n\
	extract <files> to the destination <path>;\n\
	separate them with spaces; wildcards are allowed\n\
-xr <numbers>\n\
	extract files with the given <numbers> to the destination <path>\n\
	separate number with spaces; to specify a range of numbers \n\
	put between them a hyphen without spaces e.g. 3-6; \n\
	put before an exclamation mark to exclude the numbers from the set\n\
	(e.g. 2-20 !6-8 !10);\n\
	(use -lr command to list the files with numbers)\n\
-xf <file>\n\
	extract files listed in <file> (one file per line)\n\
	to the destination <path>; wildcards are allowed\n\
-xa \n\
	extract all files from the archive to the <path>\n\
\n\
	you can use switches -x , -xr and -xf (and even -xa) simultaneously\n\
\n\
************  Delete commands  ************\n\
\n\
-d  <files>\n\
	delete <files> separate them with spaces; wildcards are allowed\n");
		printf("\
-dr <numbers>\n\
	delete files with the given <numbers>\n\
	separate number with spaces; to specify a range of numbers \n\
	put between them a hyphen without spaces e.g. 3-6 \n\
	put before an exclamation mark to exclude the numbers from the set\n\
	(e.g. 2-20 !6-8 !10);\n\
	(use -lr command to list the files with numbers)\n\
-df <file>\n\
	delete files listed in <file> (one file per line)\n\
	wildcards are allowed\n\
-da \n\
	delete all the files from the archive\n\
\n\
	you can use switches -d , -dr and -df (and even -da) simultaneously\n\
\n\
************  Other commands  ************\n\
\n\
-t\n\
	test the <archive>\n\
-p <password>\n\
	set the <password> when adding files to <archive>\n\
	or extracting them; the <password> is set for all the files\n\
	- you cannot set it separately for each file in one session\n\
-l\n\
	list the files inside <archive>\n\
-lr\n\
	list the files inside <archive> with the numbers\n\
-ll\n\
	list the files inside <archive> (only filenames)\n\
	when redirected to a file, it can be used then with command -xf\n\
\n");
		printf("\
************  Special switches  ************\n\
\n\
-cs \n\
	enable case sensitivity when:\n\
	- searching for files inside <archive> while using the command\n\
		-x or -xf (if not specified a search is non-case-sensitive)\n\
	- adding files to <archive> and trying to avoid the same\n\
		filenames in the archive\n\
-g <comment>\n\
	Set the global <comment> of the <archive>\n\
	(cannot be used on an existing disk spanning archive)\n\
	if the <comment> contains spaces, put the whole <comment>\n\
	inside quotation marks\n\
-rp <path>\n\
	set root <path> (see CZipArchive::SetRootPath() function description)\n\
-nfp\n\
	the same as bFullPath set to false in functions\n\
	CZipArchive::AddNewFile() and CZipArchive::ExtractFile()\n\
	(if not present, bFullPath is assumed to be true; if -rp specified\n\
	bFullPath is always false);\n\
-w\n\
	Wait for a key after finishing the work to let the user read the output\n\
	in some environments\n\
-dse\n\
	Display only errors when adding or extracting files\n\
\n\
************  Sample commands  ************\n\
\n\
zippie -f a:\\archive -a * -v 0 -as -c 9 \n\
	(create a disk spanning archive adding all the files from the current \n\
	directory, smart add is used, compression level set to maximum)\n\
\n\
zippie -f a:\\archive -xp d:\\a -x zippie.cpp -xr 2-12 !5-7 !9\n\
	(extract file zippie.cpp from the archive and the files with numbers \n\
	from 2 to 12 and the file number 15 apart from 5 to 7 and 9)\n\
\n\
zippie -f example -xa\n\
	extract all files from example.zip file to the current directory\n\
\n\
");
}

char ReadKey()
{
	fflush (stdin);
	char c = (char) tolower(getchar());
	return c;
}

struct SpanCallback : public CZipSpanCallback
{
	bool Callback(int iProgress)
	{
		printf ("Insert disk number %d and hit ENTER to contuniue \n or press 'n' key followed by ENTER to abort (code = %d)\n", m_uDiskNeeded, iProgress);	
		return ReadKey() != 'n';
	}
};

void FillFromFile(FILELIST& l, LPCTSTR lpszFile, bool bCheck)
{
	FILE* f = fopen(lpszFile, "rt");
	if (!f)
	{
		printf ("File %s could not be opened\n", lpszFile);
		return;
	}
	fseek(f, 0, SEEK_END);
	int iSize = ftell(f);
	fseek(f, 0, SEEK_SET);
	CZipAutoBuffer buf(iSize + 1);
	iSize = fread(buf, 1, iSize, f);
	fclose(f);
	char* sEnd = buf + iSize;
	char* sBeg = buf;
	for (char* pos = buf; ; pos++)
	{
		bool bEnd = pos == sEnd; // there may be no newline at the end
		if (strncmp(pos, "\n", 1) == 0 || bEnd)
		{
			*pos = '\0';
			CZipString s = sBeg;
			s.TrimLeft(" ");
			s.TrimRight(" ");
			if (!s.IsEmpty() && (!bCheck || ZipPlatform::FileExists(s) != 0))
				l.push_back(s);
			if (bEnd)
				break;
			sBeg = pos + 1;			
		}
	}
}




bool IsDots(LPCTSTR lpsz)
{
	return strcmp(lpsz, ".") == 0 || strcmp(lpsz, "..") == 0;
}

void AddDirectory(CZipString szPath, struct AddDirectoryInfo& info, bool bDoNotAdd)
{
	if (!szPath.IsEmpty())
		CZipPathComponent::AppendSeparator(szPath);

	bool bPathAdded = info.m_bAddEmpty || bDoNotAdd;
	if (info.m_bAddEmpty && !szPath.IsEmpty() && !bDoNotAdd)
		info.m_l.push_back(szPath);

#ifdef __GNUC__
	DIR* dp = opendir(szPath.IsEmpty() ? "." : szPath);
	if (!dp)
		return;
	struct dirent* entry;
	while (entry = readdir(dp))
	{
		struct stat sStats;
		CZipString szFullFileName = szPath + entry->d_name;
		if (stat(szFullFileName, &sStats) == -1)
			continue;
		if (S_ISDIR(sStats.st_mode))
		{
			if (info.m_bRecursive)
			{
				if (IsDots(entry->d_name))
					continue;
				
				AddDirectory(szFullFileName, info, false);
			}
		}
		else if (fnmatch(info.m_lpszFile, entry->d_name, FNM_NOESCAPE |FNM_PATHNAME) == 0)
		{
			if (!bPathAdded)
			{
				if (!szPath.IsEmpty())
					info.m_l.push_back(szPath);
				bPathAdded = true;
			}
			info.m_l.push_back(szPath + entry->d_name);
		}
	}
	closedir(dp);

#else
    CZipString szFullFileName = szPath + info.m_lpszFile;
	struct _finddata_t c_file;
	long hFile;
	if( (hFile = _findfirst( szFullFileName, &c_file )) != -1L )
	{
		do
		{
			if (!(c_file.attrib & FILE_ATTRIBUTE_DIRECTORY))
			{
				// add it when the first file comes
				if (!bPathAdded)
				{
					if (!szPath.IsEmpty())
						info.m_l.push_back(szPath);
					bPathAdded = true;
				}
				info.m_l.push_back(szPath + c_file.name);
			}
		}
		while (_findnext(hFile, &c_file) == 0L);
	}
	_findclose(hFile);

	if (info.m_bRecursive)
	{
		szFullFileName = szPath + "*";
		if( (hFile = _findfirst( szFullFileName, &c_file )) != -1L )
		{
			do
			{
				if (c_file.attrib & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (IsDots(c_file.name))
						continue;
					szFullFileName = szPath + c_file.name;
					AddDirectory(szFullFileName, info, false);
				}
			}
			while (_findnext(hFile, &c_file) == 0L);
		}
		_findclose(hFile);		
	}
#endif
}
 
void ExpandFile(FILELIST& l, LPCTSTR lpszPath, 
			 	bool bRecursive, bool bAddEmpty, bool bFullPath)
{
// check if we need to expand it
//         size_t pos = strcspn(lpszFile, "*?");
//         if (pos == strlen(lpszFile))
//         {
//                 l.push_back(lpszFile);
//                 return;
//         }

	CZipPathComponent zpc(lpszPath);
	CZipString szDir = zpc.GetFilePath();
// 	if (szDir.IsEmpty())
// 		if (!ZipPlatform::GetCurrentDirectory(szDir))
// 			return;
	struct AddDirectoryInfo adi(l);
	adi.m_bAddEmpty = bAddEmpty;
	adi.m_bRecursive = bRecursive;
	adi.m_lpszFile = zpc.GetFileName();
	AddDirectory(szDir, adi, !bFullPath); // when not full path is specified for a single file with a path, do not add a directory then

		
}


void FindInZip(CZipArchive& zip, FILELIST& l, CZipWordArray& n)
{

	for (FILELISTIT it = l.begin(); it != l.end(); ++it)
		zip.FindMatches(*it, n);
}

void ProcessData(CZipArchive& zip, CCmdLine& cmd, CZipWordArray& vRevised, bool bExtract)
{
		
		if (cmd.HasSwitch(bExtract ? "-xa" : "-da"))
		{
			int iMax = zip.GetCount();
			for (int i = 0; i < iMax; i++)
				vRevised.Add(i);
		}
		else
		{
			CZipWordArray numbers;
			CZipString temp = bExtract ? "-x" : "-d";
			int iCount = cmd.GetArgumentCount(temp);
			if (iCount > 0)
			{
				FILELIST lFiles;
				for (int i = 0; i < iCount; i++)
					lFiles.push_back(cmd.GetArgument(temp, i));
				FindInZip(zip, lFiles, numbers);
			}
			temp = bExtract ? "-xf" : "-df";
			if (cmd.GetArgumentCount(temp) > 0)
			{
				FILELIST lFiles;
				FillFromFile(lFiles, cmd.GetArgument(temp, 0), false);
				FindInZip(zip, lFiles, numbers);
			}

			temp = bExtract ? "-xr" : "-dr";
			iCount = cmd.GetArgumentCount(temp);
			CZipWordArray notNumbers;
			if (iCount > 0)
			{
				for (int i = 0; i < iCount; i++)
				{
					CZipString sz = cmd.GetArgument(temp, i);
					bool bNot = !sz.IsEmpty() && sz[0] == '!';
					CZipWordArray& vN = bNot ? notNumbers : numbers;
					if (bNot)
						sz.TrimLeft('!');
					size_t pos = strcspn(sz, "-");
					if (pos == sz.GetLength() )
						vN.Add(atoi(sz) - 1);
					else
					{
						int b = atoi (sz.Left(pos));
						int e = atoi (sz.Mid(pos + 1));
						for (int i = b; i <= e ; i++)
							vN.Add(i - 1);
					}
				}
				
			}
			int iSize = notNumbers.GetSize();
			if (iSize)
			{
				for (int j = 0; j < iSize; ++j)
					for (int i = numbers.GetSize() ; i >= 0 ;i--)
						if (numbers[i] == notNumbers[j])
							numbers.RemoveAt(i);
			}
			
			int iMax = zip.GetCount() - 1;
			for (int i = 0; i < numbers.GetSize(); ++i)
			{
				int x = numbers[i];
				if (x < 0 || x > iMax)
					continue;
				bool bNew = true;
				for (int j = 0; j < vRevised.GetSize(); ++j)
					if (vRevised[j] == numbers[i])
					{
						bNew = false;
						break;
					}
				if (bNew)
					vRevised.Add(x);
			}
		}
		

}


int main(int argc, char* argv[])
{
#ifndef __GNUC__
	// set the locale the same as the system locale
	// to handle local characters (non-English) properly by CZipString
	std::locale::global(std::locale(""));
#endif
	int iRet = 0;	
	
	CCmdLine cmd;
	CZipArchive zip;
	CZipString szArchive;
	try
	{
		
		if (cmd.SplitLine(argc, argv) < 1)
			throw 0;
		if (cmd.GetArgumentCount("-f") <= 0)
			throw 0;
		int iVolumeSize = 0;
		int iMode = CZipArchive::zipOpen;
		bool bIsAdding = cmd.GetArgumentCount("-a") > 0 || cmd.GetArgumentCount("-af") > 0;
		bool bIsExtracting = cmd.GetArgumentCount("-x") > 0 || cmd.GetArgumentCount("-xr") > 0
			|| cmd.GetArgumentCount("-xf") > 0 || cmd.HasSwitch("-xa");
		bool bIsDeleting = cmd.GetArgumentCount("-d") > 0 || cmd.GetArgumentCount("-dr") > 0 
			|| cmd.GetArgumentCount("-df") > 0 || cmd.HasSwitch("-da");

		szArchive = cmd.GetArgument("-f", 0);
		CZipPathComponent zpc(szArchive);
		if (zpc.GetFileExt().IsEmpty())
			szArchive += ".zip";
		bool bUpdateMode = cmd.HasSwitch("-u");
		bool bSetComment = cmd.GetArgumentCount("-g") > 0;
		bool bIsListing = cmd.HasSwitch("-l") || cmd.HasSwitch("-ll") || 
			cmd.HasSwitch("-lr");
		bool bOnlyErrors = cmd.HasSwitch("-dse");
		if (bIsAdding)
		{
			if (cmd.GetArgumentCount("-v") > 0)
			{
				iMode = CZipArchive::zipCreateSpan;
				iVolumeSize = atoi(cmd.GetArgument("-v", 0));
			}
			else
			{
				if (!bUpdateMode || !ZipPlatform::FileExists(szArchive))
					iMode = CZipArchive::zipCreate;
			}
		}
		else if (bIsExtracting || cmd.HasSwitch("-t") || bIsListing)
		{
			if (cmd.HasSwitch("-st"))
				iVolumeSize = 1;
		}
		else if (!bSetComment && !bIsDeleting)
			throw 0;

		SpanCallback span;
		zip.SetSpanCallback(&span);

		bool bAddEmpty = cmd.HasSwitch("-re");
		bool bRecursive = cmd.HasSwitch("-r") || bAddEmpty;
		
		
		

		bool bCaseSensitiveInZip = cmd.HasSwitch("-cs");
		pZipComp = GetCZipStrCompFunc(bCaseSensitiveInZip);
	
		zip.SetCaseSensitivity(bCaseSensitiveInZip);
		try
		{
			zip.Open(szArchive, iMode, iVolumeSize);
			if (cmd.GetArgumentCount("-p") > 0)
				zip.SetPassword(cmd.GetArgument("-p", 0));
		}
		catch(...)
		{
			bool bContinue = false;
			if (iMode == CZipArchive::zipOpen && !bIsDeleting && !bSetComment)
			{
				try
				{
					// try to open in read only mode (required if there is no write access to the storage)
					zip.Open(szArchive, CZipArchive::zipOpenReadOnly, iVolumeSize);
					bContinue = true;
				}
				catch(...)
				{
					throw;					
				}
			}
			if (!bContinue)
				throw;
		}

		if (cmd.GetArgumentCount("-rp") > 0)
			zip.SetRootPath(cmd.GetArgument("-rp", 0));

		bool bFullPath = !cmd.HasSwitch("-nfp") && zip.GetRootPath().IsEmpty();

		bool bIsSpan = zip.GetSpanMode() != 0;
		if (bSetComment && !bIsSpan)
		{
			CZipString sz = cmd.GetArgument("-g", 0);
			sz.TrimLeft("\"");
			sz.TrimRight("\"");
			zip.SetGlobalComment(sz);
		}

		if (bIsAdding)
		{
			if (bUpdateMode && bIsSpan)
			{
				printf ("Cannot update an existing disk spanning archive\n");
				zip.Close();
				return 1;
			}
			int iLevel = atoi(cmd.GetSafeArgument("-c", 0, "5"));
			int iSmartLevel;
			if (cmd.HasSwitch("-as"))
			{
				iSmartLevel = CZipArchive::zipsmSmartest;
				zip.SetTempPath(cmd.GetSafeArgument("-as", 0, ""));
			}
			else
				iSmartLevel = CZipArchive::zipsmSafeSmart;

			FILELIST lFiles;
			int iCount = cmd.GetArgumentCount("-a");
			if (iCount > 0)
			{
				
				for (int i = 0; i < iCount; i++)
					
					ExpandFile(lFiles, cmd.GetArgument("-a", i), bRecursive,
					bAddEmpty, bFullPath);
			}
			
			iCount = cmd.GetArgumentCount("-af");
			if (iCount > 0)
				FillFromFile(lFiles, cmd.GetArgument("-af", 0), true);

			FILELIST excl;

			iCount = cmd.GetArgumentCount("-ax");
			if (iCount > 0)
			{
				for (int i = 0; i < iCount; i++)
					
					ExpandFile(excl, cmd.GetArgument("-ax", i), bRecursive,
					bAddEmpty, bFullPath);
			}
			
			iCount = cmd.GetArgumentCount("-afx");
			if (iCount > 0)
				FillFromFile(excl, cmd.GetArgument("-afx", 0), true);

			FILELISTADD rev;
			for (FILELISTIT it = lFiles.begin(); it != lFiles.end(); ++it)				
			{
				// that is how the filename will look in the archive
				CZipString sz = zip.PredictFileNameInZip(*it, bFullPath);
				if (!sz.IsEmpty())
				{
					bool bAdd = true;
					for (FILELISTIT itt = excl.begin(); itt != excl.end(); ++itt)
					{
						if (!((*itt).*pZipComp)(*it))
						{
							bAdd = false;
							break;
						}
					}
					if (bAdd)
						rev.push_back(CZipAddFileInfo(*it, sz));
				}
			}
			lFiles.clear();
			excl.clear();
	
			// remove duplicates
			FILELISTADDIT it1;
			for (it1 = rev.begin(); it1 != rev.end();)
			{
				bool bErase = false;
				FILELISTADDIT it2 = it1;
				for (++it2; it2 != rev.end(); ++it2)
				{					
					int x = ((*it1).m_szFileNameInZip.*pZipComp)((*it2).m_szFileNameInZip);
					if (x == 0)
					{
						bErase = true;
						break;
					}
				}
				if (bErase)
					rev.erase(it1++);
				else
					++it1;
			}
					
			
			// sort
			rev.sort(std::greater<CZipAddFileInfo>());
			printf ("\n");
			for (it1 = rev.begin(); it1 != rev.end(); ++it1)
			{	
				if (zip.AddNewFile((*it1).m_szFilePath, iLevel, bFullPath, iSmartLevel))
				{
					if (!bOnlyErrors)
						printf ("%s added\n", (LPCTSTR)(*it1).m_szFileNameInZip);
				}
				else
					printf ("%s not added\n", (LPCTSTR)(*it1).m_szFilePath);
			}

		}
		else if (bIsExtracting)
		{
			CZipString szPath = cmd.GetSafeArgument("-xp", 0, ".");
			
			CZipWordArray vRevised;
			ProcessData(zip, cmd, vRevised, true);
			printf ("\n");
			for (int k = 0; k < vRevised.GetSize(); ++k)
			{
				int iFile = vRevised[k];
				try
				{
					zip.ExtractFile(iFile, szPath, bFullPath);
					CZipFileHeader fh;
					if (zip.GetFileInfo(fh, iFile))
					{
						if (!bOnlyErrors )
							printf ("%s extracted\n", (LPCTSTR)fh.GetFileName());
					}

				}
				catch (...)
				{
					CZipFileHeader fh;
					if (zip.GetFileInfo(fh, iFile))
						printf("Error extracting file %s\n", (LPCTSTR)fh.GetFileName());
					else
						printf("There are troubles with getting info from file number %d\n", iFile);

				}
			}
			printf("\n");
		}
		else if (bIsDeleting)
		{
			if (bIsSpan)
			{
				printf ("Cannot delete from an existing disk spanning archive\n");
				zip.Close();
				return 1;
			}
			CZipWordArray vRevised;
			ProcessData(zip, cmd, vRevised, false);			
			try
			{
				zip.DeleteFiles(vRevised);
			}
			catch (...)
			{
				printf("Error occured while deleting files\n");
			}
		}
		else if (cmd.HasSwitch("-t"))
		{
			FILELIST lFiles;
			int iCount = zip.GetCount();
			for (int i = 0; i < iCount; i++)
			{
				bool bOK = false;
				try	
				{
					bOK = zip.TestFile(i);
					printf("Tested: %d of %d                \r", i, iCount);
				}
				catch (...)
				{
					
				}
				if (!bOK)
				{	
					CZipFileHeader fh;
					if (zip.GetFileInfo(fh, i))
						lFiles.push_back(fh.GetFileName());
					else
					{
						char buf[50];
						sprintf(buf, "There are troubles with getting info from file number %d", i);
						lFiles.push_back(buf);
					}
				}
			}
			printf("\n");
			if (lFiles.size())
			{
				printf ("There were errors found in the following files:\n");
				for (FILELISTIT it = lFiles.begin(); it != lFiles.end(); ++it)
					printf("%s\n", (LPCTSTR)(*it));
			}
			else
				printf ("There were no errors found in the archive\n");
		}
		else if (bIsListing)
		{
			bool bNumbers = cmd.HasSwitch("-lr");
			bool bDescription = !cmd.HasSwitch("-ll");
			int iCount = zip.GetCount();
			if (bDescription)
				printf("\n  File name\tSize\t\tRatio\tTime Stamp\n\n");
			for (int i = 0; i < iCount; i++)
			{
				CZipFileHeader fh;
				if (zip.GetFileInfo(fh, i))
				{
					if (bNumbers)
						printf("%d.  " ,i + 1);

					printf("%s\n", (LPCTSTR)fh.GetFileName());
					if (bDescription)
					{
						printf("\t\t");
						if (fh.IsDirectory())
							printf("<DIR>\t\t");
						else
						{
							printf("%u Bytes\t", fh.m_uUncomprSize);
							printf("%.2f%%", fh.GetCompressionRatio());
						}
						time_t t = fh.GetTime();
						printf ("\t%s", ctime(&t));
					}
				}
				else
					printf("There are troubles with getting info from file number %d\n", i);

			}
			printf("\n");
			CZipString sz = zip.GetGlobalComment();
			if (!sz.IsEmpty())
				printf("Global archive comment:\n%s\n", (LPCTSTR)sz);
		}

		zip.Close();
	}
	catch (int)
	{
		DisplayUsage();
		iRet = 1;
	}
	catch (CZipException e)
	{
		printf ("Error while processing archive %s\n%s\n", (LPCTSTR) szArchive, (LPCTSTR)e.GetErrorDescription());
		if (e.m_szFileName.IsEmpty())
			printf("\n");
		else
			printf("Filename in error object: %s\n\n", (LPCTSTR)e.m_szFileName);
		zip.Close(true);
		iRet = 1;
	}
	catch (...)
	{
		printf ("Unknown error while processing archive %s\n\n", (LPCTSTR) szArchive);
		zip.Close(true);
		iRet = 1;

	}

	if (cmd.HasSwitch("-w"))
	{
		printf("\nPress <ENTER> to exit.\n");
		ReadKey();
		printf ("\n");
	}
	return iRet;
}

