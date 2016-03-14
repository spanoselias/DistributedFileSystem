#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <gtk/gtk.h>

/* The file selection widget and the string to store the chosen filename */
GtkWidget *file_selector;
gchar *selected_filename;

/* The Text Buffer as a global variabel*/
GtkTextBuffer *buffer;
/* File descriptors for pipe. */
int fds[2];

GtkWidget     *view;

/* This is a callback function to watch data in standard output and write to a view text widget. */
void input_callback( gpointer          data,
                     gint              source,
                     GdkInputCondition condition )
{

    //Clear buffer
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_set_text (buffer, "", -1);

    gchar buf[1024];
    gint chars_read;
    GtkTextIter iter;
    chars_read = 1024;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    while (chars_read == 1024)
	{
      chars_read = read(fds[0], buf, 1024);
      // fprintf(stderr, "%i chars: %s\n", chars_read, buf);
      gtk_text_buffer_insert (buffer, &iter, buf, chars_read);
    }
}

/* This is a callback function. The data arguments are ignored
 * in this example. More on callbacks below. */
static void hello( GtkWidget *widget,
                   gpointer   data )
{

    gtk_widget_show (file_selector);

}

static void delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    exit(1);
}

void store_filename(GtkFileSelection *selector, gpointer user_data)
{
    char cwd[1024];
    char command[2048];

    bzero(cwd,sizeof(cwd));
    bzero(command,sizeof(command));

    getcwd(cwd, sizeof(cwd));

    // g_print("Current working dir: %s\n", cwd);
    selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION(file_selector));

   // g_print ("%s\n" ,selected_filename);

    // g_print ("%s\n" , g_path_get_basename(selected_filename));

   // g_print("%s\n", cwd);

    sprintf(command,"cp %s %s", selected_filename, cwd);

    g_print("%s\n", command);

    system(command);

    gtk_widget_hide(file_selector);

}

void hide_file_selector(GtkFileSelection *selector, gpointer user_data)
{

    gtk_widget_hide(file_selector);
}

/* Another callback */
static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    g_print ("destroy event occurred\n");
    gtk_main_quit ();
}

void
on_button1_clicked(GtkButton* button, gpointer data)
{
    /* cast the data back to a GtkEntry*  */
    GtkEntry* entry = (GtkEntry*)data;

    g_print("%s\n", gtk_entry_get_text(entry));

    //Request ClientID
   // reqClientID(entry);

}

int main( int   argc,
          char *argv[] )
{
    // GtkWidget is the storage type for widgets//
    GtkWidget     *window;
    GtkWidget     *vbox;
    GtkWidget     *vbox2;

    GtkWidget     *button;
    GtkWidget     *store;
    GtkWidget     *submit;


    GtkWidget *bbox;

    /* this will store a text entry. */
    GtkWidget* entry;

    GtkWidget *toolbar;
    GtkToolItem *saveTb;
    GtkToolItem *exitTb;

    // Create a pipe. File descriptors for the two ends of the pipe are placed in fds.
    pipe (fds);
    // Redirect fds[1] to be writed with the standard output.
    dup2 (fds[1], STDOUT_FILENO);

    /* This is called in all GTK applications. Arguments are parsed
     * from the command line and are returned to the application. */
    gtk_init (&argc, &argv);
   
    // Create a new window //
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    //Set the position the main window in the Center
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    //Set the size the main window
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 800);

    //Set the name of the window application
    gtk_window_set_title(GTK_WINDOW(window), "Distributed File System");

    /* Create the selector */
    file_selector = gtk_file_selection_new("Please select a file for editing.");


    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
                        "clicked", GTK_SIGNAL_FUNC (store_filename), NULL);

    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->cancel_button),
                        "clicked", GTK_SIGNAL_FUNC (hide_file_selector), NULL);

    /* When the window is given the "delete_event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    g_signal_connect (G_OBJECT (window), "delete_event",
		      G_CALLBACK (delete_event), NULL);

    /* Here we connect the "destroy" event to a signal handler.
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete_event" callback. */
    g_signal_connect (G_OBJECT (window), "destroy",
		      G_CALLBACK (destroy), NULL);

    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    /* create a text entry. */
     entry = gtk_entry_new_with_max_length(20);

    /* insert the following text into the text entry, as its initial value. */
    gtk_entry_set_text(GTK_ENTRY(entry), "spanos");


    submit = gtk_button_new_with_label("Submit");


    gtk_signal_connect(GTK_OBJECT(submit), "clicked",
                       GTK_SIGNAL_FUNC(on_button1_clicked),
                       (gpointer)entry);


    toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

    saveTb = gtk_tool_button_new_from_stock(GTK_STOCK_ADD );
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), saveTb, -1);

    exitTb = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exitTb, -1);


    g_signal_connect(G_OBJECT(exitTb), "clicked",
                     G_CALLBACK(gtk_main_quit), NULL);

    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(gtk_main_quit), NULL);


    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
                        "clicked", GTK_SIGNAL_FUNC (store_filename), NULL);


    /* Creates a new button with the label "Hello World". */
    button = gtk_button_new_with_label ("Upload");


    bbox = gtk_hbutton_box_new ();

    store = gtk_button_new_with_label ("Store");

    /* When the button receives the "clicked" signal, it will call the
     * function hello() passing it NULL as its argument.  The hello()
     * function is defined above. */
    g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (hello), NULL);

    g_signal_connect (G_OBJECT (store), "clicked",
                      G_CALLBACK (hello), NULL);

    /* This will cause the window to be destroyed by calling
     * gtk_widget_destroy(window) when "clicked".  Again, the destroy
     * signal could come from here, or the window manager. */
/*
    g_signal_connect_swapped (G_OBJECT (button), "clicked",
			      G_CALLBACK (gtk_widget_destroy),
                              G_OBJECT (window));*/

    //Creates a new text view with a text buffer "Hello, this is some text". //
    view = gtk_text_view_new ();

    //Creates a new vertical box.//
    vbox = gtk_vbox_new (FALSE, 3);

    //Creates a new vertical box.//
    vbox2 = gtk_vbox_new (TRUE, 20);

    gtk_container_set_border_width (GTK_CONTAINER (bbox), 5);
    gtk_container_add (GTK_CONTAINER (vbox2), bbox);

    gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_SPREAD);

    gtk_container_add (GTK_CONTAINER (bbox), submit);

    gtk_container_add (GTK_CONTAINER (bbox), store  );

    gtk_container_add (GTK_CONTAINER (bbox), button);

    //gtk_container_set_border_width (GTK_CONTAINER (vbox2), 10);

    //This packs the vbox into the window (a gtk container).
    gtk_container_add (GTK_CONTAINER (window), vbox);

    //This packs the vbox into the window (a gtk container).
    gtk_container_add (GTK_CONTAINER (vbox), vbox2);

    /* This packs the text view into the vbox. */
    gtk_box_pack_end (GTK_BOX (vbox), view, TRUE, TRUE, 20);

    /* This packs the button into the vbox. */
    gtk_box_pack_end (GTK_BOX (vbox2), button, FALSE, FALSE, 5);

    /* This packs the button into the vbox. */
    gtk_box_pack_end (GTK_BOX (vbox2), store, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox2), toolbar, FALSE, FALSE, 5);

    /* This packs the button into the vbox. */
    gtk_box_pack_start (GTK_BOX (vbox2), entry, FALSE, FALSE, 5);

    /* This packs the button into the vbox. */
    gtk_box_pack_start (GTK_BOX (vbox2), submit, FALSE, FALSE, 5);

    gtk_text_view_set_border_window_size(view,GTK_TEXT_WINDOW_TOP,400);

    gtk_button_set_alignment(button,0.5 , 0.0);
    gtk_button_set_alignment(store,0.5 ,  0.0);

    /* This is the singnal connection to call input_callback when we have data in standard output read end pipe. */
    gdk_input_add(fds[0], GDK_INPUT_READ, input_callback, NULL);

    //The final step is to display this newly created widget.
    gtk_widget_show (button);
    gtk_widget_show (store);
    gtk_widget_show (view);
    gtk_widget_show (vbox);
    /* make the text entry visible. */
    gtk_widget_show(entry);
    gtk_widget_show(submit);


    /* and the window */
    gtk_widget_show_all(window);

    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main ();

    return 0;
}

