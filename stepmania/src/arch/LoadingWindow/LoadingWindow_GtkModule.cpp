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

	if( !gtk_init_check(argc,argv) )
		return "Couldn't initialize gtk (cannot open display)";

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
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
