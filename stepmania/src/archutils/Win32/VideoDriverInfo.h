#ifndef VIDEO_DRIVER_INFO_H
#define VIDEO_DRIVER_INFO_H

struct VideoDriverInfo
{
	CString sProvider;
	CString sDescription;
	CString sVersion;
	CString sDate;
	CString sDeviceID;
};

CString GetPrimaryVideoName9xAnd2k();
bool GetVideoDriverInfo(int cardno, VideoDriverInfo &info);
CString GetPrimaryVideoDriverInfo();

#endif
