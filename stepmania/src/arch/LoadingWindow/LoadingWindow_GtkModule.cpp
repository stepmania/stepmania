#include "global.h"
#include "LoadingWindow_GtkModule.h"

#include <gtk/gtk.h>
#include "loading.xpm"

static GtkWidget *label;
static GtkWidget *window;

extern "C" const char *Init( int *argc, char ***argv )
{
	GdkPixmap *loadmap;
	GtkWidget *vbox;
	GtkWidget *loadimage;

	gtk_disable_setlocale();
	if( !gtk_init_check(argc,argv) )
		return "Couldn't initialize gtk (cannot open display)";

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position( GTK_WINDOW(window), GTK_WIN_POS_CENTER );
	gtk_widget_realize(window);
	loadmap = gdk_pixmap_create_from_xpm_d(window->window,NULL,NULL,loading);
	loadimage = gtk_image_new_from_pixmap(loadmap,NULL);
	label = gtk_label_new(NULL);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
	vbox = gtk_vbox_new(FALSE,5);
	gtk_container_add(GTK_CONTAINER(window),vbox);
	gtk_box_pack_start(GTK_BOX(vbox),loadimage,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(vbox),label,TRUE,TRUE,0);

	gtk_widget_show_all(window);
	gtk_main_iteration_do(FALSE);
	return NULL;
}

extern "C" void Shutdown()
{
	gtk_widget_hide_all(window);
	g_signal_emit_by_name (G_OBJECT (window), "destroy");
	while( gtk_events_pending() )
		gtk_main_iteration_do(FALSE);
}

extern "C" void SetText( const char *s )
{
	gtk_label_set_text(GTK_LABEL(label), s);
	gtk_widget_show(label);
	gtk_main_iteration_do(FALSE);
}

/*
 * (c) 2003-2004 Glenn Maynard, Sean Burke
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
