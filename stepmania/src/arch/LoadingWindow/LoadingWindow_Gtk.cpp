#include <gtk/gtk.h>

static GtkWidget *label;
static GtkWidget *window;

void CreateGtkLoadingWindow() {
  GtkWidget *vbox;
  GtkWidget *loadimage;
  char **argv;
  int argc;

  argc = 1;
  argv[0] = "stepmania";
  gtk_init(&argc,&argv);
  loadimage = gtk_image_new_from_file("loading.xpm");
  label = gtk_label_new(NULL);
  gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
  vbox = gtk_vbox_new(FALSE,0);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_add(GTK_CONTAINER(window),vbox);
  gtk_box_pack_start(GTK_BOX(vbox),loadimage,FALSE,FALSE,10);
  gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,20);
  gtk_widget_show(loadimage);
  gtk_widget_show(label);
  gtk_widget_show(vbox);
  gtk_widget_show(window);
  gtk_main();
}

void DestroyGtkLoadingWindow() {
  gtk_main_quit();
}

void SetGtkLoadingWindowText(const char *s) {
  gtk_label_set_text(GTK_LABEL(label),s);
  gtk_widget_show(label);
}
