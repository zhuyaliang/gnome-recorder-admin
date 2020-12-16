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

#include "screen-window.h"
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

    bindtextdomain (GETTEXT_PACKAGE,LUNAR_CALENDAR_LOCALEDIR);
    textdomain (GETTEXT_PACKAGE);

    gtk_init (&argc, &argv);

    window = screen_window_new ();
    g_signal_connect (window, "destroy",
                      G_CALLBACK (app_quit), &window);

    gtk_widget_show_all (window);
    gtk_main ();
    return 0;
}
