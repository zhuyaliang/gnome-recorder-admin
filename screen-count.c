/*************************************************************************
  File Name: screen-frame.c
  
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
                                               
  Created Time: 2020年12月14日 星期一 09时30分29秒
 ************************************************************************/

#include "screen-count.h"


enum
{
    FINISHED,
    LAST_SIGNAL
};
struct _ScreenCountPrivate
{
    int         count_down;
    GtkWidget  *window;
};
static guint signals[LAST_SIGNAL] = { 0 };
G_DEFINE_TYPE_WITH_PRIVATE (ScreenCount, screen_count, GTK_TYPE_FRAME)

const char *count_images[] = {"cb-1.png","cb-2.png","cb-3.png","cb-4.png","cb-5.png","cb-6.png","cb-7.png","cb-8.png","cb-9.png","cb-10.png"};

static gboolean on_darw (GtkWidget *widget, cairo_t *cr1, gpointer data)
{
    ScreenCount *count = SCREEN_COUNT (data);
    cairo_surface_t *image;
    GdkWindow       *window;
    cairo_t         *cr;
    char            *file;
    int              i;

    i = count->priv->count_down;
    file = g_strdup_printf ("%s/%s","/usr/share/screen-admin/counter",count_images[i]);
    window = gtk_widget_get_window (widget);
    image = cairo_image_surface_create_from_png(file);
    cr = gdk_cairo_create (window);
    cairo_set_source_rgba(cr,0.0, 0.0, 0.0, 0.45);
    cairo_set_operator (cr,CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_set_source_surface (cr,image,0,0);
    cairo_paint (cr);
    
    g_free (file);
    return FALSE;
}

static gboolean screen_countdown (gpointer data)
{
    ScreenCount *count = SCREEN_COUNT (data);
	
    count->priv->count_down -= 1;
    gtk_widget_hide (count->priv->window);
	gtk_widget_show (count->priv->window );

    if (count->priv->count_down == 0)    
		return FALSE;
    
    return TRUE;
}
static void count_down_changed_cb (GtkSpinButton *spin_button,
                                   gpointer       user_data)
{
    ScreenCount *count = SCREEN_COUNT (user_data);
    gint value;

    value = gtk_spin_button_get_value_as_int (spin_button);
    count->priv->count_down = value;
}

static GtkWidget *create_count_down_window (void)
{
    GtkWidget *window;
    GdkVisual *visual;
    GtkWidget *toplevel;
    GdkScreen *screen;
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    toplevel = gtk_widget_get_toplevel (window);
    screen = gtk_widget_get_screen(GTK_WIDGET(toplevel));
    visual = gdk_screen_get_rgba_visual(screen);
    gtk_widget_set_visual(GTK_WIDGET(toplevel), visual);
    
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 380, 380);
    gtk_widget_set_app_paintable(window, TRUE);
    gtk_window_set_resizable (GTK_WINDOW(window), TRUE);
    gtk_window_set_decorated (GTK_WINDOW(window), FALSE);
    
    return window;
}
static void
screen_count_dispose (GObject *object)
{
//    ScreenCount *count;
}

static void
screen_count_init (ScreenCount *count)
{
    count->priv = screen_count_get_instance_private (count);
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *spin;

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_add (GTK_CONTAINER (count), hbox);
    label = gtk_label_new(_("count down"));
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 12);
    spin = gtk_spin_button_new_with_range (0, 10, 1);
    g_signal_connect (spin,
                     "value-changed",
                      G_CALLBACK (count_down_changed_cb),
                      count);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), 5);
    gtk_box_pack_start (GTK_BOX (hbox), spin, FALSE, FALSE, 12);
    
    count->priv->window = create_count_down_window ();
}

static void
screen_count_class_init (ScreenCountClass *count_class)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (count_class);
    gobject_class->dispose      = screen_count_dispose;
    
    signals [FINISHED] =
         g_signal_new ("finished",
                       G_TYPE_FROM_CLASS (count_class),
                       G_SIGNAL_RUN_LAST,
                       0,
                       NULL, NULL,
                       g_cclosure_marshal_VOID__VOID,
                       G_TYPE_NONE, 0);

}

GtkWidget *
screen_count_new (const char *title)
{
    ScreenCount *count = NULL;
    GtkWidget   *label;
    char        *text;

    count = g_object_new (SCREEN_TYPE_COUNT, NULL);
    gtk_frame_set_label (GTK_FRAME (count),"");
    text =  g_markup_printf_escaped("<span color = \'grey\' size=\"%s\" weight='bold'>%s</span>","large",title);
    label = gtk_frame_get_label_widget (GTK_FRAME (count));
    gtk_label_set_markup (GTK_LABEL (label),text);

    g_free (text);

    return GTK_WIDGET (count);
}

gboolean screen_start_count_down (ScreenCount *count)
{
    g_return_val_if_fail (SCREEN_IS_COUNT (count), FALSE);
    g_return_val_if_fail (count->priv->window != NULL, FALSE);
    
    if (count->priv->count_down == 0)
        return TRUE;
    g_signal_connect(count->priv->window, "draw", G_CALLBACK (on_darw), count);
	g_timeout_add (1000, (GSourceFunc)screen_countdown, count);
    
    return TRUE;
}
