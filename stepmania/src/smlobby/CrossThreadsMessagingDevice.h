// CrossThreadsMessagingDevice.h

#ifndef _CrossThreadsMessagingDevice_H_
#define	_CrossThreadsMessagingDevice_H_


class CCrossThreadsMessagingDevice
{
public :
	struct ICrossThreadsMessagingDeviceMonitor
	{
		virtual void OnCrossThreadsMessage(WPARAM wParam, LPARAM lParam) = 0;
	};

	CCrossThreadsMessagingDevice();
	virtual ~CCrossThreadsMessagingDevice();

	void SetMonitor(ICrossThreadsMessagingDeviceMonitor* pMonitor) { m_pMonitor = pMonitor; }
	void Post(WPARAM wParam, LPARAM lParam);

	operator bool() const { return ::IsWindow(m_hWnd)==TRUE; }

private :
	enum { HWM_DATA = WM_USER + 1000 };
	static LPCTSTR m_lpszClassName;
	static int m_iCount;
	HWND m_hWnd;
	ICrossThreadsMessagingDeviceMonitor* m_pMonitor;

	static LRESULT WINAPI HiddenWindowProc(HWND, UINT, WPARAM, LPARAM);
};



#endif // _CrossThreadsMessagingDevice_H_

