// Adapted from http://www.codeproject.com/KB/files/zip_utils.aspx

#ifndef CreateZip_H
#define CreateZip_H


// ZIP functions -- for creating zip files
// This file is a repackaged form of the Info-Zip source code available
// at www.info-zip.org. The original copyright notice may be found in
// zip.cpp. The repackaging was done by Lucian Wischik to simplify and
// extend its use in Windows/C++. Also to add encryption and unicode.


class TZip;
class RageFile;

typedef unsigned long ZRESULT;
// return codes from any of the zip functions. Listed later.


class CreateZip
{
	TZip* hz;
public:
	CreateZip();
	bool Start(RageFile *f);
	bool AddFile(RString fn);
	bool AddDir(RString fn);
	bool Finish();
	RString GetError();
};




// e.g.
//
// (1) Traditional use, creating a zipfile from existing files
//     HZIP hz = CreateZip("c:\\simple1.zip",0);
//     ZipAdd(hz,"znsimple.bmp", "c:\\simple.bmp");
//     ZipAdd(hz,"znsimple.txt", "c:\\simple.txt");
//     CloseZip(hz);
//
// (2) Memory use, creating an auto-allocated mem-based zip file from various sources
//     HZIP hz = CreateZip(0,100000, 0);
//     // adding a conventional file...
//     ZipAdd(hz,"src1.txt",  "c:\\src1.txt");
//     // adding something from memory...
//     char buf[1000]; for (int i=0; i<1000; i++) buf[i]=(char)(i&0x7F);
//     ZipAdd(hz,"file.dat",  buf,1000);
//     // adding something from a pipe...
//     HANDLE hread,hwrite; CreatePipe(&hread,&hwrite,nullptr,0);
//     HANDLE hthread = CreateThread(0,0,ThreadFunc,(void*)hwrite,0,0);
//     ZipAdd(hz,"unz3.dat",  hread,1000);  // the '1000' is optional.
//     WaitForSingleObject(hthread,INFINITE);
//     CloseHandle(hthread); CloseHandle(hread);
//     ... meanwhile DWORD WINAPI ThreadFunc(void *dat)
//                   { HANDLE hwrite = (HANDLE)dat;
//                     char buf[1000]={17};
//                     DWORD writ; WriteFile(hwrite,buf,1000,&writ,nullptr);
//                     CloseHandle(hwrite);
//                     return 0;
//                   }
//     // and now that the zip is created, let's do something with it:
//     void *zbuf; unsigned long zlen; ZipGetMemory(hz,&zbuf,&zlen);
//     HANDLE hfz = CreateFile("test2.zip",GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
//     DWORD writ; WriteFile(hfz,zbuf,zlen,&writ,nullptr);
//     CloseHandle(hfz);
//     CloseZip(hz);
//
// (3) Handle use, for file handles and pipes
//     HANDLE hzread,hzwrite; CreatePipe(&hzread,&hzwrite,0,0);
//     HANDLE hthread = CreateThread(0,0,ZipReceiverThread,(void*)hzread,0,0);
//     HZIP hz = CreateZipHandle(hzwrite,0);
//     // ... add to it
//     CloseZip(hz);
//     CloseHandle(hzwrite);
//     WaitForSingleObject(hthread,INFINITE);
//     CloseHandle(hthread);
//     ... meanwhile DWORD WINAPI ZipReceiverThread(void *dat)
//                   { HANDLE hread = (HANDLE)dat;
//                     char buf[1000];
//                     for(;;)
//                     { DWORD red; ReadFile(hread,buf,1000,&red,nullptr);
//                       // ... and do something with this zip data we're receiving
//                       if (red==0) break;
//                     }
//                     CloseHandle(hread);
//                     return 0; 
//                   }


#endif
