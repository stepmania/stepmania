#ifndef LOADING_WINDOW_GTK
#define LOADING_WINDOW_GTK

#include "LoadingWindow.h"

class LoadingWindow_Gtk : public LoadingWindow {
public:
	LoadingWindow_Gtk();
	~LoadingWindow_Gtk();

	void SetText(CString str);
	void Paint() { }
};

#define HAVE_LOADING_WINDOW_GTK

#endif
