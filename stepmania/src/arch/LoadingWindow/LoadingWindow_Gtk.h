#ifndef LOADING_WINDOW_GTK
#define LOADING_WINDOW_GTK

#include "LoadingWindow.h"

void CreateGtkLoadingWindow();
void DestroyGtkLoadingWindow();
void SetGtkLoadingWindowText(const char *s);

class LoadingWindow_Gtk : public LoadingWindow {
public:
	LoadingWindow_Gtk() { CreateGtkLoadingWindow(); }
	~LoadingWindow_Gtk() { DestroyGtkLoadingWindow(); }

	void SetText(CString str) { SetGtkLoadingWindowText(str.c_str()); }
	void Paint() { }
};

#undef ARCH_LOADING_WINDOW
#define ARCH_LOADING_WINDOW LoadingWindow_Gtk

#endif
