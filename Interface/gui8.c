/* example-start menu menu.c */

#include <stdio.h>
#include <gtk/gtk.h>

static gint button_press (GtkWidget *, GdkEvent *);
static void menuitem_response (gchar *);

GtkWidget     *view;
GtkWidget* entry;
GtkWidget     *submit;

/* Create a Button Box with the specified parameters */
GtkWidget *create_bbox (gint  horizontal,
                        char* title,
                        gint  spacing,
                        gint  child_w,
                        gint  child_h,
                        gint  layout)
{

    GtkWidget *frame;
    GtkWidget *bbox;
    GtkWidget *button;

    frame = gtk_frame_new (title);

    if (horizontal)
        bbox = gtk_hbutton_box_new ();
    else
        bbox = gtk_vbutton_box_new ();

    gtk_container_set_border_width (GTK_CONTAINER (bbox), 5);
    gtk_container_add (GTK_CONTAINER (frame), bbox);

    /* Set the appearance of the Button Box */
    gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), layout);
    gtk_button_box_set_spacing (GTK_BUTTON_BOX (bbox), spacing);
    gtk_button_box_set_child_size (GTK_BUTTON_BOX (bbox), child_w, child_h);

    button = gtk_button_new_with_label ("Read");
    gtk_container_add (GTK_CONTAINER (bbox), button);

    button = gtk_button_new_with_label ("Upload");
    gtk_container_add (GTK_CONTAINER (bbox), button);

    return(frame);

}

/* Create a Button Box with the specified parameters */
GtkWidget *create_Login (gint  horizontal,
                          char* title,
                          gint  spacing,
                          gint  child_w,
                          gint  child_h,
                          gint  layout)
{

    GtkWidget *frame;
    GtkWidget *bbox;
    GtkWidget *button;

    frame = gtk_frame_new (title);

    if (horizontal)
        bbox = gtk_hbutton_box_new ();
    else
        bbox = gtk_vbutton_box_new ();

    gtk_container_set_border_width (GTK_CONTAINER (bbox), 5);
    gtk_container_add (GTK_CONTAINER (frame), bbox);

    /* Set the appearance of the Button Box */
    gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), layout);
    gtk_button_box_set_spacing (GTK_BUTTON_BOX (bbox), spacing);
    gtk_button_box_set_child_size (GTK_BUTTON_BOX (bbox), child_w, child_h);

    /* create a text entry. */
    entry = gtk_entry_new_with_max_length(15);

    /* insert the following text into the text entry, as its initial value. */
    gtk_entry_set_text(GTK_ENTRY(entry), "spanos");

    submit = gtk_button_new_with_label("Submit");

    gtk_container_add (GTK_CONTAINER (bbox), submit);
    gtk_container_add (GTK_CONTAINER (bbox), entry);

    return(frame);

}


/* "print text" button "clicked" callback function. */
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

    static GtkWidget* window = NULL;

    GtkWidget *main_vbox;

    GtkWidget *frame_horz;

    GtkWidget *frame_horz2;

    GtkWidget *vbox;

    GtkWidget *toolbar;
    GtkToolItem *saveTb;
    GtkToolItem *exitTb;


    /* Initialize GTK */
    gtk_init( &argc, &argv );

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Distributed File System");

    //Set the position the main window in the Center
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    //Set the size the main window
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 800);


    gtk_container_set_border_width (GTK_CONTAINER (window), 4);


    main_vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), main_vbox);


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

    gtk_box_pack_start(GTK_BOX(main_vbox), toolbar, FALSE, FALSE, 5);

    frame_horz = gtk_frame_new ("Layout");
    gtk_box_pack_start (GTK_BOX (main_vbox), frame_horz, TRUE, TRUE, 10);

    frame_horz2 = gtk_frame_new ("Log file");
    gtk_box_pack_start (GTK_BOX (main_vbox), frame_horz2, TRUE, TRUE, 10);

    //Creates a new text view //
    view = gtk_text_view_new ();
   // gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);
    gtk_container_add (GTK_CONTAINER (frame_horz2), view);


    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);
    gtk_container_add (GTK_CONTAINER (frame_horz), vbox);

    gtk_box_pack_start (GTK_BOX (vbox),
                        create_Login (TRUE, "Loggin",40, 85, 20, GTK_BUTTONBOX_SPREAD),
                        TRUE, TRUE, 0);


    gtk_box_pack_start (GTK_BOX (vbox),
                        create_bbox (TRUE, "Operation", 40, 85, 20, GTK_BUTTONBOX_SPREAD),
                        TRUE, TRUE, 0);

  /*  gtk_box_pack_start (GTK_BOX (vbox),
                        create_viewer (TRUE, "Operation2", 40, 85, 20, GTK_BUTTONBOX_SPREAD),
                        TRUE, TRUE, 0);*/


    gtk_signal_connect(GTK_OBJECT(submit), "clicked",
                       GTK_SIGNAL_FUNC(on_button1_clicked),
                       (gpointer)entry);


    /* always display the window as the last step so it all splashes on
     * the screen at once. */
    gtk_widget_show_all (window);

    gtk_main ();

    return(0);
}


