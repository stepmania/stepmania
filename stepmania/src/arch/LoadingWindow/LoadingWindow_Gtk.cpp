#include "global.h"
#include "LoadingWindow_Gtk.h"

#include "StepMania.h"
#include <gtk/gtk.h>
#include "loading.xpm"

static GtkWidget *label;
static GtkWidget *window;

LoadingWindow_Gtk::LoadingWindow_Gtk()
{
	GdkPixmap *loadmap;
	GtkWidget *vbox;
	GtkWidget *loadimage;

	gtk_init(&g_argc,&g_argv);
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
}

LoadingWindow_Gtk::~LoadingWindow_Gtk()
{
	gtk_widget_hide_all(window);
	g_signal_emit_by_name (G_OBJECT (window), "destroy");
	while( gtk_events_pending() )
		gtk_main_iteration_do(FALSE);
}

void LoadingWindow_Gtk::SetText( CString s )
{
	gtk_label_set_text(GTK_LABEL(label), s);
	gtk_widget_show(label);
	gtk_main_iteration_do(FALSE);
}
