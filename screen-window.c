/*************************************************************************
  File Name: screen-window.c
  
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
                                               
  Created Time: 2020年12月15日 星期二 11时17分28秒
 ************************************************************************/

#include "screen-window.h"
#include "screen-style.h"
#include "screen-stop.h"
#include "screen-save.h"
#include "screen-count.h"

#define GNOME_SCREENCAST_NAME         "org.gnome.Shell.Screencast"
#define GNOME_SCREENCAST_PATH         "/org/gnome/Shell/Screencast"

struct _ScreenWindowPrivate 
{
    GDBusProxy  *proxy;

    GtkWidget  *style;
    GtkWidget  *stop;
    GtkWidget  *save;
    GtkWidget  *count;
};

G_DEFINE_TYPE_WITH_PRIVATE (ScreenWindow, screen_window, GTK_TYPE_WINDOW)


static void screencast_button_cb (GtkWidget *button, gpointer user_data)
{
    ScreenWindow *screenwin = SCREEN_WINDOW (user_data);
    ScreenCount  *count = SCREEN_COUNT (screenwin->priv->count);
    gboolean active;

    screen_start_count_down (count);
    active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
    if (active)
        gtk_button_set_label (GTK_BUTTON (button), _("Stop"));
    else
        gtk_button_set_label (GTK_BUTTON (button), _("Start"));

}

static GtkWidget *create_start_and_stop_screencast (ScreenWindow *screenwin)
{
    GtkWidget *hbox;
    GtkWidget *button;

    hbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
    button = gtk_toggle_button_new_with_label ("Start");
    
    g_signal_connect (button,
                     "clicked",
                     (GCallback) screencast_button_cb,
                      screenwin);

    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 12);
    
    return hbox;
}
static void
screen_window_fill (ScreenWindow *screenwin)
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *frame_style,*frame_stop,*frame_save,*frame_count;
    
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add (GTK_CONTAINER (screenwin), vbox);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);

    frame_style = screen_style_new (_("Recording screen settings"));
    gtk_box_pack_start (GTK_BOX (vbox), frame_style, FALSE, FALSE, 12);
    screenwin->priv->style = frame_style;

    frame_stop = screen_stop_new (_("Stop Mode"));
    gtk_box_pack_start (GTK_BOX (vbox), frame_stop, FALSE, FALSE, 12);
    screenwin->priv->stop = frame_stop;

    frame_save = screen_save_new (_("Save Mode"));
    gtk_box_pack_start (GTK_BOX (vbox), frame_save, FALSE, FALSE, 12);
    screenwin->priv->save = frame_save;

    frame_count = screen_count_new (_("count down"));
    gtk_box_pack_start (GTK_BOX (vbox), frame_count, FALSE, FALSE, 12);
    screenwin->priv->count = frame_count;

    hbox = create_start_and_stop_screencast (screenwin);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 12);

}
static GObject *
screen_window_constructor (GType                  type,
                           guint                  n_construct_properties,
                           GObjectConstructParam *construct_properties)
{   
    GObject        *obj;
    ScreenWindow   *screenwin;
    
    obj = G_OBJECT_CLASS (screen_window_parent_class)->constructor (type,
                                      n_construct_properties,
                                      construct_properties);
    
    screenwin = SCREEN_WINDOW (obj);
    screen_window_fill (screenwin);
    
    return obj;
}
static void
screen_window_dispose (GObject *object)
{   
    ScreenWindow *screenwin;
    
    screenwin = SCREEN_WINDOW (object);
    g_object_unref (screenwin->priv->proxy);
    G_OBJECT_CLASS (screen_window_parent_class)->dispose (object);
}

static void
screen_window_class_init (ScreenWindowClass *klass)
{   
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);
    
    gobject_class->constructor = screen_window_constructor;
    gobject_class->dispose = screen_window_dispose;
}
static void
screen_window_init (ScreenWindow *screenwin)
{   
    GtkWindow *window;

    screenwin->priv = screen_window_get_instance_private (screenwin);
    screenwin->priv->proxy = g_dbus_proxy_new_for_bus_sync (
                  G_BUS_TYPE_SESSION,
                  G_DBUS_PROXY_FLAGS_NONE,
                  NULL,
                  GNOME_SCREENCAST_NAME,
                  GNOME_SCREENCAST_PATH,
                  GNOME_SCREENCAST_NAME,
                  NULL, NULL);
    
    window = GTK_WINDOW (screenwin);
    gtk_window_set_title (GTK_WINDOW (window), _("Gnome Record Screen"));
    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
    
    gtk_window_set_position (window, GTK_WIN_POS_CENTER);
    gtk_window_set_default_size (GTK_WINDOW (window),
                                 400, 400);
}

GtkWidget *
screen_window_new (void)
{
    ScreenWindow *screenwin;

    screenwin = g_object_new (SCREEN_TYPE_WINDOW,
                           "type", GTK_WINDOW_TOPLEVEL,
                            NULL);

    return GTK_WIDGET (screenwin);
}

