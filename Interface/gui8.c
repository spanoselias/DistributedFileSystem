/* example-start menu menu.c */

#include <stdio.h>
#include <gtk/gtk.h>

static gint button_press (GtkWidget *, GdkEvent *);
static void menuitem_response (gchar *);


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

    button = gtk_button_new_with_label ("OK");
    gtk_container_add (GTK_CONTAINER (bbox), button);

    button = gtk_button_new_with_label ("Cancel");
    gtk_container_add (GTK_CONTAINER (bbox), button);


    return(frame);
}

int main( int   argc,
          char *argv[] )
{

    static GtkWidget* window = NULL;

    GtkWidget *main_vbox;

    GtkWidget *frame_horz;

    GtkWidget *vbox;


    /* Initialize GTK */
    gtk_init( &argc, &argv );

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Button Boxes");


    gtk_container_set_border_width (GTK_CONTAINER (window), 10);


    main_vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), main_vbox);


    frame_horz = gtk_frame_new ("Horizontal Button Boxes");
    gtk_box_pack_start (GTK_BOX (main_vbox), frame_horz, TRUE, TRUE, 10);


    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);
    gtk_container_add (GTK_CONTAINER (frame_horz), vbox);

    gtk_box_pack_start (GTK_BOX (vbox),
                        create_bbox (TRUE, "Spread (spacing 40)", 40, 85, 20, GTK_BUTTONBOX_SPREAD),
                        TRUE, TRUE, 0);



    /* always display the window as the last step so it all splashes on
     * the screen at once. */
    gtk_widget_show_all (window);

    gtk_main ();

    return(0);
}

/* Respond to a button-press by posting a menu passed in as widget.
 *
 * Note that the "widget" argument is the menu being posted, NOT
 * the button that was pressed.
 */

