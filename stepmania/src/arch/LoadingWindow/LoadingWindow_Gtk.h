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

#undef ARCH_LOADING_WINDOW
#define ARCH_LOADING_WINDOW LoadingWindow_Gtk

#endif
