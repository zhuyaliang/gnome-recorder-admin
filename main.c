/*************************************************************************
  File Name: main.c
  
  Copyright (C) 2020  zhuyaliang https://github.com/zhuyaliang/
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
                                      
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
                                               
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
                                               
  Created Time: 2020年12月09日 星期三 17时40分39秒
 ************************************************************************/

#include <stdio.h>

#include "screen-style.h"
#include "screen-stop.h"
#include "screen-save.h"
#include "screen-count.h"

#define GETTEXT_PACKAGE "screen-admin"
#define LUNAR_CALENDAR_LOCALEDIR "/usr/share/locale"

static void set_lable_font_type(GtkWidget  *lable,
                                const char *color,
                                int         font_size,
                                const char *word,
                                gboolean    blod)
{
    char *lable_type;

    if(blod)
    {
        lable_type = g_strdup_printf ("<span foreground=\'%s\'weight=\'light\'font_desc=\'%d\'><b>%s</b></span>",
                         color,
                         font_size,
                         word);
    }
    else
    {
        lable_type = g_strdup_printf("<span foreground=\'%s\'weight=\'light\'font_desc=\'%d\'>%s</span>",
                        color,
                        font_size,
                        word);
    
    }
    gtk_label_set_markup(GTK_LABEL(lable),lable_type);
    g_free(lable_type);
}
static GtkWidget *create_countdown ()
{
}
static void start_record_screen (GtkWidget *button, gpointer user_data)
{
    ScreenCount *count = SCREEN_COUNT (user_data);
    screen_start_count_down (count);
}

static GtkWidget *create_start_stop (GtkWidget *frame)
{
    GtkWidget *hbox;
    GtkWidget *button_start,*button_stop;

    hbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
    button_start = gtk_button_new_with_label ("Start");
    g_signal_connect (button_start,
                     "clicked",
                     (GCallback)start_record_screen,
                      frame);
 
    gtk_box_pack_start (GTK_BOX (hbox), button_start, FALSE, FALSE, 12);
    button_stop = gtk_button_new_with_label ("Stop");

    return hbox;
}
static void app_quit(GtkWidget *object,
                     gpointer   user_data)
{
    gtk_widget_destroy (object);
    exit (0);
}
int
main (int argc, char **argv)
{
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *frame_style,*frame_stop,*frame_save,*frame_count;

    bindtextdomain (GETTEXT_PACKAGE,LUNAR_CALENDAR_LOCALEDIR);
    textdomain (GETTEXT_PACKAGE);

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), _("Gnome Record Screen"));
    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
    gtk_widget_set_size_request (window, 400, 400);
    g_signal_connect (window, "destroy",
                      G_CALLBACK (app_quit), &window);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);

    frame_style = screen_style_new (_("Recording screen settings"));
    gtk_box_pack_start (GTK_BOX (vbox), frame_style, FALSE, FALSE, 12);

    frame_stop = screen_stop_new (_("Stop Mode"));
    gtk_box_pack_start (GTK_BOX (vbox), frame_stop, FALSE, FALSE, 12);

    frame_save = screen_save_new (_("Save Mode"));
    gtk_box_pack_start (GTK_BOX (vbox), frame_save, FALSE, FALSE, 12);

    frame_count = screen_count_new (_("count down"));
    gtk_box_pack_start (GTK_BOX (vbox), frame_count, FALSE, FALSE, 12);

    hbox = create_start_stop (frame_count);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 12);

    gtk_widget_show_all (window);
    gtk_main ();
    return 0;
}
