#ifndef REGISTRY_H
#define REGISTRY_H

class CRegistry
{
public:
	CRegistry();
	~CRegistry();

	int m_nLastError;

// CRegistry properties	
protected:
	HKEY m_hRootKey;
	BOOL m_bLazyWrite;
	CString m_strCurrentPath;

public:
	inline BOOL PathIsValid() const { return m_strCurrentPath.GetLength() > 0; }
	inline CString GetCurrentPath() const { return m_strCurrentPath; }
	inline HKEY GetRootKey() const { return m_hRootKey; }

public:
	BOOL ClearKey();
	BOOL SetRootKey(HKEY hRootKey);
	BOOL CreateKey(CString strKey);
	BOOL DeleteKey(CString strKey);
	BOOL DeleteValue(CString strName);
	int GetDataSize(CString strValueName);
	DWORD GetDataType(CString strValueName);
	int GetSubKeyCount();
	int GetValueCount();
	BOOL KeyExists(CString strKey, HKEY hRootKey = NULL);
	BOOL SetKey(CString strKey, BOOL bCanCreate);
	BOOL ValueExists(CString strName);
	void RenameValue(CString strOldName, CString strNewName);

	// data reading functions
	COleDateTime ReadDateTime(CString strName, COleDateTime dtDefault);
	double ReadFloat(CString strName, double fDefault);
	CString ReadString(CString strName, CString strDefault);
	int ReadInt(CString strName, int nDefault);
	BOOL ReadBool(CString strName, BOOL bDefault);
	COLORREF ReadColor(CString strName, COLORREF rgbDefault);
	BOOL ReadFont(CString strName, CFont* pFont);
	BOOL ReadPoint(CString strName, CPoint* pPoint);
	BOOL ReadSize(CString strName, CSize* pSize);
	BOOL ReadRect(CString strName, CRect* pRect);
	DWORD ReadDword(CString strName, DWORD dwDefault);

	/* Readers converted to a less silly mechanism below (do these as needed): 
	 * return the value in result if found, otherwise return false. */
	bool Read(CString strName, bool &result);

	// data writing functions
	BOOL WriteBool(CString strName, BOOL bValue);
	BOOL WriteDateTime(CString strName, COleDateTime dtValue);
	BOOL WriteString(CString strName, CString strValue);
	BOOL WriteFloat(CString strName, double fValue);
	BOOL WriteInt(CString strName, int nValue);
	BOOL WriteColor(CString strName, COLORREF rgbValue);
	BOOL WriteFont(CString strName, CFont* pFont);
	BOOL WritePoint(CString strName, CPoint* pPoint);
	BOOL WriteSize(CString strName, CSize* pSize);
	BOOL WriteRect(CString strName, CRect* pRect);
	BOOL WriteDword(CString strName, DWORD dwValue);

};// end of CRegistry class definition


#endif