/*

For conditions of distribution and use, see the copyright notice in License.txt.
Please read the documentation instead of this document 
(it may be difficult to read it due to formatting tags for an automatic documentation generation).
If you do not have it, please download it from http://www.artpol-software.com/

*/

/** \mainpage ZipArchive library documentation

\section secGen General Information

<I><B>
The ZipArchive library				<BR>
</B>
Copyright &copy; 2000 - 2002 Tadeusz Dracz<BR>
</I>

\b Version: 2.2 				<BR>
\b Date:    10-January-2002


This library adds zip compression and decompression functionality to your program, allowing you to create and modify ZIP files in the compatible way with WinZip and PKZIP.
It's easy and practical interface makes the library suitable for the beginners as well as for the advanced users. 


<B>\ref pageSyst "Platforms supported:" </B>
- Windows 9x\Me\NT\2000\XP (MFC and STL) - \ref sectVisual "Microsoft Visual C++ 6.0", \ref sectBorl "Borland C++"
- \ref sectLinux "Linux (STL)"

\author Tadeusz Dracz		<BR>
E-Mail: \htmlonly <a href="mailto:tdracz@artpol-software.com">tdracz@artpol-software.com</a> \endhtmlonly<BR>
Web Site: \htmlonly <A HREF="http://www.artpol-software.com" target="_blank">http://www.artpol-software.com</A> \endhtmlonly

This library uses \htmlonly <A HREF="http://www.freesoftware.com/pub/infozip/zlib/" target="_blank">the zlib library </A> \endhtmlonly by Jean-loup Gailly and Mark Adler to perform inflate and deflate operations.

\section sectFeat Features Summary:
- work in a compatible way with PKZIP and WinZip (apart from \ref TDSpan "TD disk spanning mode" which is specific to this library)
- create, modify, extract and test zip archives
- create and extract multi-disk archives (on non-removable disks as well)
- password encryption and decryption supported
- possibility to create or extract self-extracting archives
- compression from and decompression to memory, create the whole archive in memory, extract the archive from memory (see \ref sectMemory)
- smart compression, if enabled, prevents the file in the archive to be larger after compression (see CZipArchive::Smartness)
- safe compression with CZipArchive::Flush function
- using functional objects as callback
	- to provide easy disk change in a multi-disk archives
	- for the progress control when adding, extracting, testing or deleting files or saving archive's central directory
- extracting and deleting using wildcard pattern matching (see CZipArchive::FindMatches)
- UNC and Windows Unicode paths recognized 
- wide characters supported
- support for the Java <sup><small>TM</small></sup> Archive (jar) File Format (see CZipArchive::SetIgnoreCRC)
- possibility to integrate help system with MSDN (see \ref sectHelp)
- easy interface
- easy transfer to other system platforms
- speedy
- well documented
- full source code provided
- sample applications provided (for the STL version located in \e stl/zippie,
the MFC version is available separately)

Suggestions and bug reports are welcome. Drop me a note if you wish to share any.

\section secQl Quick Links

\par 
\ref pageGen
\par
\ref pageSyst
\par
\ref pageFaq
\par
\ref pageHist 
\par
\ref pageLic 
\par
\ref pageSubsc


 */

/**
 
	\page pageSyst Compilation & Integration

	\subsection stlNotes STL version notes
	- <B>[Windows only]</B> If your locale is different from English and you wish to use non-English 
	characters in zip files, you need to set your locale with function
	\e std::locale::global(std::locale("")) to set it to be the same as your 
	system locale or e.g. \e std::locale::global(std::locale("French"))
	to set it to the specified value (do not use \e _T() macro here when using 
	Unicode); \e setlocale() function is not sufficient in this case.
	- There is a sample application that compiles under Windows (MSVC) and Linux (see below 
	to find out \ref stlLinuxTest "how to compile it under Linux"). This sample application demonstrates most of the 
	ZipArchive library features and is located in \e stl/zippie.

	\subsection MFCsample MFC sample application (ZipArc) notes
	MFC sample application using ZipArchive library is available separately. Main features:
		- MDI application
		- multithreaded - you can work with many zip files at one time
		- shell integration (remembers the last application used to open zip files and can restore it correctly)
		- drag & drop support
		- detailed error reports
		- you can open and modify SFX archives
		- it demonstrates the use of the following functions (most of them are placed in ZipArcDoc.cpp) :
		CZipArchive::AddNewFile,
		CZipArchive::Close,
		CZipArchive::CloseFile,
		CZipArchive::CloseNewFile,
		CZipArchive::DeleteFiles,
		CZipArchive::EnableFindFast,
		CZipArchive::ExtractFile,
		CZipArchive::FindFile,
		CZipArchive::FindMatches,
		CZipArchive::Flush,
		CZipArchive::GetArchivePath, 
		CZipArchive::GetCentralDirInfo,
		CZipArchive::GetCentralDirSize,
		CZipArchive::GetCurrentDisk,
		CZipArchive::GetFileInfo,
		CZipArchive::GetFindFastIndex,
		CZipArchive::GetGlobalComment,
		CZipArchive::GetCount,
		CZipArchive::GetPassword,
		CZipArchive::GetSpanMode,
		CZipArchive::IsClosed,
		CZipArchive::IsReadOnly,
		CZipArchive::Open,
		CZipArchive::PredictExtractedFileName,
		CZipArchive::SetAdvanced,
		CZipArchive::SetCallback,
		CZipArchive::SetFileComment,
		CZipArchive::SetGlobalComment,
		CZipArchive::SetIgnoreCRC,
		CZipArchive::SetPassword,
		CZipArchive::SetRootPath,
		CZipArchive::SetSpanCallback,
		CZipArchive::SetTempPath,
		CZipArchive::TestFile, <BR>
		CZipFileHeader::IsEncrypted,
		CZipFileHeader::IsDirectory,
		CZipFileHeader::GetTime,
		CZipFileHeader::GetSystemCompatibility,
		CZipFileHeader::GetSystemAttr,
		CZipFileHeader::GetSize,
		CZipFileHeader::GetFileName,
		CZipFileHeader::GetEffComprSize,
		CZipFileHeader::GetCompressionRatio,
		CZipFileHeader::GetComment,
		CZipFileHeader::CompressionEfficient,

	\section secCompil Compiling for different implementations and platforms

	
	The files required for all the library versions are located in the program 
	root directory. You also need to copy additional files to the library 
	root directory from the two more subfolders. Depending on the configuration 
	these files are located in:

	
	\subsection winMFC Windows MFC
		\e \Windows and \e \mfc <BR>
		You can just execute <EM> _copy from Win-MFC.bat </EM> batch file.

	\subsection winSTL Windows STL
		\e \Windows and \e \stl <BR>
		You can just execute <EM> _copy from Win-STL.bat </EM> batch file.

	\subsection LnxSTL Linux (STL version)
		\e \Linux and \e \stl <BR>
		You can just execute <EM> _copy_from_Linux.sh </EM> script file
		(don't forget to set executable rights before e.g. with the command:
		<EM> chmod +x _copy_from_Linux.sh </EM>).
		
	\note If you use one of the mentioned scripts to copy the files then for easy
	orientation there will be a file \e __[...].zcfg created with the 
	name depending on the current configuration

	\section sectVisual Visual C++ : integrating with the project
	
	To add ZipArchive library functionality into your project you need to link 
	the library to the project. You can do this in at least two ways 
	(in both cases you need to include ZipArchive.h header in your sources).

	\subsection subsM1 Method 1

	Add \e ZipArchive.lib with the proper path e.g. <EM> ..\ZipArchive\debug\ZipArchive.lib </EM> to <EM> Project Settings->Link->Input->Object/library modules </EM> 
	and add ZipArchive library directory to the preprocessor searches (<EM> Project Settings -> C++ -> Preprocessor -> Additional include directories </EM>).

	\subsection subsM2 Method 2 (simpler)
	Insert Zip project into workspace and set project dependencies: your project dependent on ZipArchive project
	(<EM> Project -> Dependencies </EM> and then on the dialog that will appear
	you select your project name from the combo box and check the box next to ZipArchive project name).
	When you use this method, you link configurations in your project with
	configurations in the ZipArchive project that have the same name in both projects. So if you need to use
	for example "Static Release" configuration from ZipArchive project, you need to create one with the same name
	in your application project and make sure that your project uses MFC library and run-time library in same way
	(<em> Project->Settings->General->Microsoft Fundation Classes </em> and <EM> Project->Settings-> c/c++ ->Code Generation->Use run-time library </EM>).

	In case you experience linking problems, please see the \ref pageFaq.

	\section sectBorl Borland C++ compatibility
	The library contains a project files for Borland C++ 5.0 (
	They were created using Visual C++ Project Conversion Utility (VCTOBPR.EXE).
	You can start it with the command <I> Tools->Visual C++ Project Conversion Utility </I>.
	- The project \e ZipArchive.bpr was converted from a Release configuration
	of the MFC version of the library (\e ZipArchive.dsp).
	- The project \e ZipArchive_no_MFC.bpr was converted from a Release configuration
	of the non MFC version of the library (\e ZipArchive_no_MFC.dsp).

	\note Be sure to create \e Release subfolder before compiling one of these projects,
	 otherwise you'll get a write error.

	In case the projects provided don't work for you, you can create your own. You need to copy 
	to the root directory appropriate files for \ref winMFC "MFC" or \ref winSTL "STL" versions. 
	You may use the Borland project conversion utility.

	<EM><B>The library contains also \e makefiles which should work with every version of Borland.</B></EM>

	
	\subsection subExample Compiling the sample application
	There is a Borland C++ project provided with the sample application \c ZipArc. <BR>
	To compile it you need compiled MFC version of ZipArchive library.

	\note Be sure to create \e Release subfolder first, otherwise you'll get 
	a write error.
	
	Add the library (\e ZipArchive.lib ) to the project (<I>Project->Add To Project</I>) and compile.

	If you wish to convert Visual C++ project using Visual C++ Project Conversion Utility then after 
	converting to properly compile the application you need to remove \e odbccp32.lib from the project file
	and comment everything out in \e ZipArc_TLB.cpp or remove this file from the project files. 

	\section sectLinux Linux platform

	\subsection subsLnxNot Notes


<EM> \b Usage </EM>

	When using the library under Linux you should be aware of a few things:
	- after you get the system attributes with CZipFileHeader::GetSystemAttr() function, 
	you need to shift them right by 16 (<EM> e.g. uAttr = header.GetSystemAttr() >> 16 </EM>) - 
	the reason for that is the way the attributes are stored in the archive created under (or for) Linux.
	- due to lack of implementation of ZipPlatform::IsDriveRemovable(), which has proven to be a kind of difficult to do,
	the device containing the archive is always assumed to be removable; the only effect of this
	is that you need to set \e iVolumeSize to a value different from 0 when opening with the function
	CZipArchive::Open() the archive created	in tdSpan mode.


<EM> \b Compiling </EM>

	- the library was tested under g++ version 2.96
	- warnings that use of the \c tempnam and \c mktemp is not safe can be ignored, because the
	temporary file is created almost straight away.


	\subsection subsLnxCom Compiling the library and liniking with the application
	- First you need to copy the appropriate files (\ref LnxSTL "see above")<BR>
	- If you haven't got the zlib library already installed on your system, you can install
	it using \e Makefile in the \e zlib subdirectory (type \e make and then <EM>make install </EM>)
	- only static version (you can download a full version of the zlib library from the ZipArchive library site).
	If you don't want to install the zlib library, you need to include it in the ZipArchive library
	( edit the \e Makefile in the main directory and change comments, so that
	OBJSZLIB is not an empty value but points to zlib library object files)
	- Compile the library by typing \e make. The resulting file is a static library \e libziparch.a
	- You can copy the library and the headers (if you have an appropriate rights) to \e /usr/lib and
	\e /usr/include/ziparchive (you can change them in \e Makefile) with the command <EM>make install</EM>
	- Now you can link the library to your application <BR>
	e.g. with the command (if the library is in the same directory)<BR>
	<EM> g++ $I. -o app app.cpp -lstdc++ -lz libziparch.a</EM> <BR>
	or if you have the library installed in the /usr subdirectories:<BR>
	<EM> g++ $I/usr/include/ziparchive -o app app.cpp -lstdc++ -lz -lziparch</EM><BR>
	If you haven't got the zlib library installed, you need to omit the switch \e -lz  in the above commands.
	\anchor stlLinuxTest
	- There is a <B> test application </B>named \e zippie.cpp (in \e stl/zippie which you can compile typing <EM>make zippie</EM>
	(providing that you have installed the ZipArchive library). If you haven't got the zlib library installed,
	you need to switch the comments (comment one line and uncomment another) in the \e Makefile in the section \e "zippie:".
	- If you wish to uninstall the library type <EM>make uninstall</EM>

*/


/**

\page pageGen General Information

\par
\ref sectCompress
\par
\ref sectSpan
\par
\ref sectPass 
\par
\ref sectSE 
\par
\ref sectExc 
\par
\ref sectMemory
\par
\ref sectCallb
\par
\ref sectHelp
\par

 

\section sectCompress Compression and decompression

There are some functions defined for fast operations on archive: CZipArchive::AddNewFile,
 CZipArchive::ExtractFile, CZipArchive::DeleteFile, CZipArchive::TestFile. 
 You only need to call functions CZipArchive::Open - before and CZipArchive::Close - after using them. Calling CZipArchive::Close function after you've done modifying the archive is necessary 
for the archive to be intact.

\section sectSpan Multi-disk archives

This library supports two kinds of multi-disk archives:

- Disk spanning that is performed in the compatible way with all other main zip programs. It means that:
	- the archive can only be created on a removable device, 
	- the size of the volume is auto-detected
	- the label is written to the disk
	- you need to define a callback functor for changing disks and set it with CZipArchive::SetSpanCallback function.

- Disk spanning that is performed in the internal mode, called in the sources TDSpan mode and it means that:
	- the archive can be created on non-removable device as well
	- you need to define the single volume size
	- there is no need to set callback functor in this mode.

\anchor convertZips
These two disk spanning modes create volumes with compatible internal structure. It means that you can easily convert the volumes created in one mode to the other one by renaming the files (in TDSpan mode each volume but last has a number as an extension). To convert the archive from TD to PKZIP compatible archive, copy each file to the removable media, giving them the extension ".zip". You should also label each disk with the appropriate label starting from "pkback# 001"
(note the space between '#' and '0').

There is a limited functions set available while working with multi-disk archives. Only adding is allowed when creating the archive and only extracting and testing after opening an existing one. Deleting files from these archives is not allowed at all.

Class CZipArchive uses write buffer to make write operations as fast as possible. You can change its size with CZipArchive::SetAdvanced function. While creating a multi-disk archive, set the size of the buffer to the maximum size of the volume for the best performance.

The popular archivers such as PKZIP and WinZip cannot operate on archive in TDSpan mode. You need to convert them to PKZIP span mode (\ref convertZips "have a look above"). Remember about copying the files to the removable media (it does not comply with Winzip, which can extract a multi-disk archive from any media but only from the fixed location on the drive).

\section sectPass Password encryption and decryption

This library supports creating and extracting the password protected archives. There are several issues you should be aware of when using this feature. To set the password for the file to be added or extracted call the function CZipArchive::SetPassword with the password as the argument. To clear the password call this function without arguments or with an empty string argument. The function has no effect on a closed archive and on the currently opened file (whether new or existing) inside archive. During opening the archive the password is cleared. You can set different passwords for different files inside the same archive, but remember to set it BEFORE opening the file. You cannot use ASCII characters with codes above 127 in the password, if you do so, the function CZipArchive::SetPassword returns false and the password is cleared.
You can find out what files are password encrypted by calling CZipArchive::GetFileInfo, which fills the structure CZipFileHeader with data, and then call the method ZipFileHeader::IsEncrypted. If it returns true, the file needs a password to extract.
The successful extraction of the encrypted file doesn't always mean that the password is correct. You also need to check the return value of CZipArchive::CloseFile. You could also check the size of the extracted file since it can be smaller than it should be in case of the bad password.

\section sectSE Self extract support

The library is capable of extracting and modifying self-extracting archives. You can create your own SFX archive as well. This is the simplest code responsible for the self-extracting:

\code
//Windows code

int APIENTRY WinMain(HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR lpCmdLine,
int nCmdShow)
{
	CZipArchive zip;

	// get the path of the executable
	TCHAR szBuff[_MAX_PATH];
	if (!::GetModuleFileName(hInstance, szBuff, _MAX_PATH))
		return -1;

	CZipString szDest;
	// ...
	// add some code here to get the destination directory from the user 
	// for example:
	// CBrowseForFolder bf;
	//   bf.strTitle = _T("Select directory to extract files");
	//   if (!bf.GetFolder(szDest))
	//       return -1;
	//
	// class CBrowseForFolder is included in the sample application project
	// remember about including the header!
	zip.Open(szBuff, CZipArchive::openReadOnly); 
	// openReadOnly mode is necessary for self extract archives
	for (WORD i = 0; i < zip.GetCount(); i++)
		zip.ExtractFile(i, szDest);

	zip.Close();
	return 0;
	// this code will not work for the encrypted archives since it is needed
	// to get the password from the user ( a small addition to the 
	// existing code I suppose )
}

\endcode

After compiling it and appending a zip archive to it (e.g. with the DOS command: <EM> copy /b SelfExtract.exe + ZipFile.zip FinalFile.exe </EM>) you have a self extracting archive.


\section sectExc Exceptions

\subsection excmfc MFX version
The library throws the following exceptions inherited from \c CException: \c CMemoryException*, \c CFileExeption* and \c CZipException*. The first two don't need an explanation. The last is thrown when some internal error occurs. Handling them may be done in the following way:

\code

try
{
	// ...
	// some operations on the ZipArchive library
}
catch (CException* e)
{
	if (e->IsKindOf( RUNTIME_CLASS( CZipException )))
	{
		CZipException* p = (CZipException*) e;
		//... and so on 
	}
	else if (e->IsKindOf( RUNTIME_CLASS( CFileException )))
	{
		CFileException* p = (CFileException*) e;
		//... and so on 
	} 
	else
	{
		// the only possibility is a memory exception I suppose
		//... and so on
	}
	e->Delete();
}


\endcode

\subsection excstl STL version
The library throws exceptions inherited from \c std::exception. In this case you should catch \c std::exception object <VAR><B>(not a pointer to it)</B></VAR>.

\section sectMemory Creating and extracting archives from/in memory

With the function CZipArchive::Open(CZipMemFile&, int) you can create the archive in memory and then write to disk, e.g.:

\code

CZipArchive zip;
CZipMemFile mf;
// create archive in memory
zip.Open(mf, CZipArchive::zipCreate);
// ...
// add some files to archive here e.g. by calling CZipArchive::AddNewFile
// ...
zip.Close();
// write the archive to disk
CZipFile f;
if (f.Open("c:\\temp.zip", CZipFile::modeWrite|CZipFile::modeCreate, false)
{
	int iLen = mf.GetLength();
	BYTE* b = mf.Detach();
	f.Write(b, iLen);
	f.Close();
	// we must free the detached memory
	free(b);
}
\endcode

You can as well read the archive from disk and then extract files, e.g.:

\code
CZipFile f;
if (f.Open("c:\\temp.zip", CZipFile::modeRead, false)
{
	int iLen = f.GetLength();	
	BYTE* b = (BYTE*)malloc((UINT)iLen);
	f.Read(b, iLen);
	f.Close();
	CZipMemFile mf;
	mf.Attach(b, iLen);
	CZipArchive zip;
	zip.Open(mf);
	// ...
	// extract files here from the archive e.g. by calling CZipArchive::ExtractFile
	// ...
	zip.Close();
}
\endcode

With functions <CODE> CZipArchive::AddNewFile(CZipMemFile&, LPCTSTR, int, int, unsigned long) </CODE> and 
<CODE>CZipArchive::ExtractFile(WORD, CZipMemFile&, DWORD)</CODE> you can add files to archive from memory 
and extract them to a memory file. Now a bit larger example:

\code
// create the archive in memory with two files inside
CZipMemFile mf;
CZipArchive zip;
zip.Open(mf, CZipArchive::zipCreate);
zip.AddNewFile("c:\\testfile1.txt");
zip.AddNewFile("c:\\testfile2.txt");
zip.Close();
//create the archive on disk and add a previously zipped file from memory
zip.Open("c:\\test.zip", CZipArchive::zipCreate);
zip.AddNewFile(mf, "File1.zip");
zip.Close();
// we have now zip-in-zip file on the disk, 
// let's extract the embedded zip file back to memory
zip.Open("c:\\test.zip");
// reset the contents of the CZipMemFile object
mf.SetLength(0);
zip.ExtractFile(0, mf);
zip.Close();
// write the file from memory to disk
CZipFile f;
if (f.Open("c:\\File1.zip", CZipFile::modeWrite|CZipFile::modeCreate, false))
{
	int iLen = mf.GetLength();
	BYTE* b = mf.Detach();
	f.Write(b, iLen);
	f.Close();
	// we must free the detached memory
	free(b);
}
\endcode


<B>One important thing:</B> when you operate on the archive in memory, you must ensure
that CZipMemory object will not be destroyed before calling CZipArchive::Close.
In some cases you'll need to construct the object using the \c new operator, e.g.:
\code
// ...
CZipMemFile* pmf = new CZipMemFile;
zip.Open(*pmf, CZipArchive::zipCreate);
// ...
zip.Close();
delete pmf;
\endcode

\section sectCallb Action progress notifications (callbacks)
 
The library has the possibility to notify about the progress of the various actions (see CZipArchive::CallbackType).
To use this feature you need to define a new class derived from CZipActionCallback or from CZipSpanCallback and override
CZipCallback::Callback function. Then you need to declare an object of your class and pass its address to 
function CZipArchive::SetCallback or CZipArchive::SetSpanCallback. Make sure that the object exists while the library 
performs the action the functor was assigned to.
or tell the library not to use the callback (use the same functions).


\section sectHelp Integrating with MSDN

If you wish to integrate the ZipArchive help system with the MSDN library you need to:
- download from 
\htmlonly
<A HREF="http://www.artpol-software.com" target="_blank">the Artpol Software site</A>
\endhtmlonly
ZipArchive HTML Help documentation or ZipArchive HTML documentation if you don't have it.
- in the latter case you need to compile the html files to the HTML Help format
with HTML Web Workshop by Microsoft (at the moment of writing available i.a. 
\htmlonly
<A HREF="http://msdn.microsoft.com/library/default.asp?url=/library/en-us/htmlhelp/html/hwMicrosoftHTMLHelpDownloads.asp" target="_blank">here</A>)
\endhtmlonly
and using provided \e index.hhp file (it is at the same location as ZipArchive html help files)
- now you should have \e index.chm and \e index.chi files, rename them if you want to and put them
to the directory of your choice
- you need to download a free <B> MSDN Integration Utility</B> by Kirk Stowell; you can download it from 
\htmlonly
<A HREF="http://www.codeproject.com/winhelp/msdnintegrator.asp" target="_blank">the Code Project site</A>
or from <A HREF="http://www.artpol-software.com" target="_blank">the Artpol Software site</A> 
\endhtmlonly
(Download->ZipArchive)
- use the <B> MSDN Integration Utility</B> for the files you have prepared
- now pressing the F1 key on the ZipArchive library method or class in the Visual Studio brings up the MSDN help;
you have also a searchable ZipArchive collection inside MSDN
\note After integrating the ZipArchive help system with the MSDN library, you need
to be patient when you use the Index for the first time, because it'll be rebuilt then which can
be a lengthy process.
*/

/**
\page pageSubsc ZipArchive Newsletter

To be notified about ZipArchive library updates, enter your
e-mail into the input field below and press \e Subscribe! button.

\htmlonly

<form method="post" action="http://www.artpol-software.com/cgi-bin/subscribe.cgi">
<input type="hidden" name="op" value="add">
<input type="hidden" name="list" value="ZipArchive">
<input type="text" name="email" value="">
<input type="submit" value="Subscribe!">
</form>


<H4>Privacy Policy of the ZipArchvie library Newsletter List</h4>
<UL>
 <li> The information you provide, will never be made available to any third party company. 
 <li> Only the Web Master of the Artpol Software site will have an access to the mailing list.
 <li> If at anytime you wish to be removed from the mailing list, you can do it by 
following the link that you will receive with every newsletter.
</UL>

\endhtmlonly

*/
 
