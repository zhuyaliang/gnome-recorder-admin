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
#include <libappindicator/app-indicator.h>

#include "screen-window.h"
#include "screen-style.h"
#include "screen-stop.h"
#include "screen-save.h"
#include "screen-count.h"

#define GNOME_SCREENCAST_NAME         "org.gnome.Shell.Screencast"
#define GNOME_SCREENCAST_PATH         "/org/gnome/Shell/Screencast"

struct _ScreenWindowPrivate 
{
    GDBusProxy *proxy;

    GtkWidget  *style;
    GtkWidget  *stop;
    GtkWidget  *save;
    GtkWidget  *count;
    gboolean    is_start;
};

G_DEFINE_TYPE_WITH_PRIVATE (ScreenWindow, screen_window, GTK_TYPE_WINDOW)

static void
screen_admin_satrt (GSimpleAction *action,
                    GVariant      *parameter,
                    gpointer       user_data)
{
}
static void
screen_admin_stop (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
}
static void
screen_admin_quit (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
}
static void
screen_admin_skip (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
}
static const GActionEntry actions[] = {
  { "screen-admin-start", screen_admin_satrt},
  { "screen-admin-stop",  screen_admin_stop},
  { "screen-admin-quit",  screen_admin_quit},
  { "screen-admin-skip",  screen_admin_skip},
};

static void create_screencast_indicator (void)
{
    AppIndicator *indicator;
    GSimpleActionGroup *action_group;
    GtkBuilder   *builder;
    GMenuModel   *menu_model;
    GtkWidget    *menu;
	GtkWidget    *settings_item;
	GtkWidget    *start_item;
	GtkWidget    *stop_item;
	GtkWidget    *quit_item;
    GtkWidget    *skip_item;
	GtkWidget    *separator_item;
	GError       *error = NULL;

    indicator = app_indicator_new ("screen-admin",
                                   "camera-video",
                                    APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	menu = gtk_menu_new ();
    settings_item = gtk_menu_item_new_with_label (_("Settings"));
	gtk_widget_set_sensitive (settings_item, TRUE);

    settings_item = gtk_menu_item_new_with_label (_("Start"));
	gtk_widget_set_sensitive (settings_item, TRUE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), settings_item);
    
	stop_item = gtk_menu_item_new_with_label (_("Stop"));
	gtk_widget_set_sensitive (settings_item, FALSE);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), stop_item);
	
	skip_item = gtk_menu_item_new_with_label (_("Skip"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), skip_item);
	
	separator_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), separator_item);
    
    quit_item = gtk_menu_item_new_with_label (_("Quit"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), quit_item);

	gtk_widget_show_all (menu);
	app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_title (indicator, "screen-admin");
    app_indicator_set_menu (indicator, GTK_MENU(menu));
}
static GVariantBuilder *get_screencast_variant (ScreenWindow *screenwin)
{   
    GVariantBuilder *builder;
    gboolean  show_cursor;
    uint       framerate;
    
    ScreenStyle *style = SCREEN_STYLE (screenwin->priv->style);

    show_cursor = screen_style_get_show_cursor (style);
    framerate = screen_style_get_framerate (style);
    
    builder = g_variant_builder_new (G_VARIANT_TYPE ("a{sv}"));
    g_variant_builder_add (builder, "{sv}", "draw-cursor",g_variant_new_boolean (show_cursor));
    g_variant_builder_add (builder, "{sv}", "framerate", g_variant_new_uint32 (framerate));
    
    return builder;

}

static char *get_screencast_save_path (ScreenSave *save)
{
    char     *save_path;
    char     *folder_name;
    char     *file_name;
    char     *new_name;
    int       num = 1;
    folder_name = screen_save_get_folder_name (save);
    file_name = screen_save_get_file_name (save);

    save_path = g_build_filename (folder_name, file_name, NULL);
    if (!g_file_test (save_path, G_FILE_TEST_EXISTS))
        return save_path;

    while (TRUE)
    {
        new_name = g_strdup_printf ("%s-%d", file_name, num);
        save_path = g_build_filename (folder_name, new_name, NULL);
        g_free (new_name);
        if (!g_file_test (save_path, G_FILE_TEST_EXISTS))
            return save_path;
        num++;
    }

}

static void start_screencast (ScreenWindow *screenwin)
{
    GVariantBuilder *variant;
    char            *save_path;
    ScreenSave      *save;
  
    save = SCREEN_SAVE (screenwin->priv->save);
    
    variant = get_screencast_variant (screenwin);
    save_path = get_screencast_save_path (save); 
    
    g_dbus_proxy_call (screenwin->priv->proxy,
                      "Screencast",
                       g_variant_new ("(sa{sv})", save_path, variant),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       NULL,
                       NULL);

}

static void stop_screencast (ScreenWindow *screenwin)
{
    g_dbus_proxy_call_sync (screenwin->priv->proxy,
                           "StopScreencast",
                            g_variant_new ("()"),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            NULL);
}
static void countdown_finished_cb (ScreenCount *count, gpointer user_data)
{
    ScreenWindow    *screenwin = SCREEN_WINDOW (user_data);
    
    if (screenwin->priv->is_start)
    {
        start_screencast (screenwin);
    }
    else
    {
        stop_screencast (screenwin);
    }
}

static void screencast_button_cb (GtkWidget *button, gpointer user_data)
{
    ScreenWindow *screenwin = SCREEN_WINDOW (user_data);
    ScreenCount  *count = SCREEN_COUNT (screenwin->priv->count);
    gboolean active;

	gtk_widget_hide (GTK_WIDGET (screenwin));
    screen_start_count_down (count);
    create_screencast_indicator ();
    active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
    if (active)
        gtk_button_set_label (GTK_BUTTON (button), _("Stop"));
    else
        gtk_button_set_label (GTK_BUTTON (button), _("Start"));
    
    screenwin->priv->is_start = active; 
    g_signal_connect (count,
                      "finished",
                     (GCallback) countdown_finished_cb,
                      screenwin);

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

