#include <gtk/gtk.h>

static GtkWidget *label;
static GtkWidget *window;

void CreateGtkLoadingWindow() {
  GtkWidget *vbox;
  GtkWidget *loadimage;
  char **argv;
  gchar *filename;
  int argc;

  argc = 1;
  argv[0] = "stepmania";
  gtk_init(&argc,&argv);
  filename = g_build_filename(DATADIR,"pixmaps","stepmania","loading.xpm",NULL);
  loadimage = gtk_image_new_from_file(filename);
  g_free(filename);
  label = gtk_label_new(NULL);
  gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
  vbox = gtk_vbox_new(FALSE,5);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_add(GTK_CONTAINER(window),vbox);
  gtk_box_pack_start(GTK_BOX(vbox),loadimage,FALSE,FALSE,0);
  gtk_box_pack_end(GTK_BOX(vbox),label,TRUE,TRUE,0);
  gtk_widget_show(loadimage);
  gtk_widget_show(label);
  gtk_widget_show(vbox);
  gtk_widget_show(window);
  gtk_main_iteration_do(FALSE);
}

void DestroyGtkLoadingWindow() {
}

void SetGtkLoadingWindowText(const char *s) {
  gtk_label_set_text(GTK_LABEL(label),s);
  gtk_widget_show(label);
  gtk_main_iteration_do(FALSE);
}
