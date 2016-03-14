#include <gtk/gtk.h>


GtkWidget *filew;


static GtkWidget *xpm_label_box( gchar     *xpm_filename,
                                 gchar     *label_text )
{
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *image;

  /* Create box for image and label */
  box = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (box), 2);

  /* Now on to the image stuff */
  image = gtk_image_new_from_file (xpm_filename);

  /* Create a label for the button */
  label = gtk_label_new (label_text);

  /* Pack the image and label into the box */
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 3);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 3);

  gtk_widget_show (image);
  gtk_widget_show (label);

  return box;
}


/* Get the selected filename and print it to the console */
static void file_ok_sel( GtkWidget        *w,
                         GtkFileSelection *fs )
{

  gchar *selected_filename;

  selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  g_print ("%s\n" , g_path_get_basename(selected_filename));

}


/* Our usual callback function */
static void callback( GtkWidget *widget,
                      gpointer   data )
{
  g_print ("testing");

  /* Display that dialog */

  gtk_widget_show (filew);
}


int main(int argc, char *argv[]) {

  GtkWidget *window;
  GtkWidget *vbox;
  
  GtkWidget *toolbar;
  GtkToolItem *saveTb;  
  GtkToolItem *exitTb;

  GtkWidget *box;


  GtkWidget *button;

  gtk_init(&argc, &argv);

  // Create a new window //
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  //Set the position the main window in the Center
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

  //Set the size the main window
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 800);

  //Set the name of the window application
  gtk_window_set_title(GTK_WINDOW(window), "Distributed File System");

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  toolbar = gtk_toolbar_new();
  gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);  

  saveTb = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), saveTb, -1);  

  exitTb = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exitTb, -1);

  gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 5);

  g_signal_connect(G_OBJECT(exitTb), "clicked", 
        G_CALLBACK(gtk_main_quit), NULL);

  g_signal_connect(G_OBJECT(window), "destroy",
        G_CALLBACK(gtk_main_quit), NULL);


  /* Create a new file selection widget */
  filew = gtk_file_selection_new ("File selection");

  g_signal_connect (filew, "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);

  /* Connect the ok_button to file_ok_sel function */
  g_signal_connect (GTK_FILE_SELECTION (filew)->ok_button,
                    "clicked", G_CALLBACK (file_ok_sel), (gpointer) filew);

  /* Connect the cancel_button to destroy the widget */
  g_signal_connect_swapped (GTK_FILE_SELECTION (filew)->cancel_button,
                            "clicked", G_CALLBACK (gtk_widget_destroy),
                            filew);

  /* Create a new button */
  button = gtk_button_new ();



  /* This calls our box creating function */
  box = xpm_label_box ("info.xpm", "Read");

  //Show the button
  gtk_widget_show (button);

  gtk_container_add (GTK_CONTAINER (button), box);

  gtk_container_add (GTK_CONTAINER (window), button);




  /* Connect the "clicked" signal of the button to our callback */
  g_signal_connect (button, "clicked",
                    G_CALLBACK (callback), (gpointer) "read");





  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
