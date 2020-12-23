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
#include <libnotify/notify.h>

#include "screen-window.h"
#include "screen-style.h"
#include "screen-stop.h"
#include "screen-save.h"
#include "screen-count.h"

#define GNOME_SCREENCAST_NAME         "org.gnome.Shell.Screencast"
#define GNOME_SCREENCAST_PATH         "/org/gnome/Shell/Screencast"
#define EXTENSION1_PATH                "/usr/share/gnome-shell/extensions/appindicatorsupport@rgcjonas.gmail.com/metadata.json"
#define EXTENSION2_PATH                "/usr/share/gnome-shell/extensions/TopIcons@phocean.net/metadata.json"

struct _ScreenWindowPrivate 
{
    GDBusProxy   *proxy;
    AppIndicator *indicator;
    NotifyNotification *notify;

    GtkWidget  *style;
    GtkWidget  *stop;
    GtkWidget  *save;
    GtkWidget  *count;
    GtkWidget  *settings_item;
    GtkWidget  *start_item;
    GtkWidget  *stop_item;
    GtkWidget  *quit_item;
    GtkWidget  *skip_item;
    GtkWidget  *dialog;
    gboolean    is_start;
    char       *save_path;

    guint       second;
    guint       minute;
    gboolean    show_label;
    guint       tray_timeout_id;
};

G_DEFINE_TYPE_WITH_PRIVATE (ScreenWindow, screen_window, GTK_TYPE_WINDOW)

static void stop_screencast (ScreenWindow *screenwin);

static gboolean use_appindicator (void)
{
    const char *xdg_session;
    gboolean    is_xorg = TRUE;
    gboolean    is_main;
    gboolean    is_secondary;

    xdg_session = g_getenv ("XDG_SESSION_TYPE");
    if (g_strcmp0 (xdg_session, "wayland") == 0)
    {
        is_xorg = FALSE;
    }

    is_main = g_file_test (EXTENSION1_PATH, G_FILE_TEST_EXISTS);
    is_secondary = g_file_test (EXTENSION2_PATH, G_FILE_TEST_EXISTS);

    if (is_main == TRUE)
        return TRUE;
    else if (is_xorg == FALSE)
    {
        return FALSE;
    }
    else
    {
        return is_secondary;
    }
}

static NotifyNotification *get_notification (void)
{
    NotifyNotification *notify;
 
    notify_init ("Screen-Admin");
    notify = notify_notification_new ("screen-admin",
                                      _("Screen  ready"),
                                      "emblem-default");
    notify_notification_set_urgency (notify, NOTIFY_URGENCY_LOW);
    notify_notification_set_timeout (notify, NOTIFY_EXPIRES_DEFAULT);

    return notify;
}

static void screen_admin_update_notification (NotifyNotification *notify,
                                              const char         *summary,
                                              const char         *body,
                                              const char         *icon)
{
    if (notify == NULL)
        return;
    notify_notification_update (notify, summary, body, icon);
    notify_notification_show (notify, NULL);
}
static void
screen_start_item_cb (GtkMenuItem *item, gpointer user_data)
{
    ScreenWindow *screenwin = SCREEN_WINDOW (user_data);

    gtk_widget_set_sensitive (screenwin->priv->stop_item,  TRUE);
    gtk_widget_set_sensitive (screenwin->priv->start_item, FALSE);

    gtk_widget_show (GTK_WIDGET (screenwin));
}

static void update_tray_time (ScreenWindow *screenwin)
{
    gchar * percentstr;

    g_source_remove (screenwin->priv->tray_timeout_id);
    screenwin->priv->tray_timeout_id = 0;

    screenwin->priv->minute = 0;
    screenwin->priv->second = 0;
    
    if (screenwin->priv->show_label)
    {
        percentstr = g_strdup_printf("%02u:%02u", screenwin->priv->minute, screenwin->priv->second);
        app_indicator_set_label (screenwin->priv->indicator, percentstr, "100%");
        g_free(percentstr);
    }
    app_indicator_set_label (screenwin->priv->indicator, NULL, NULL);
}
static void
screen_stop_item_cb (GtkMenuItem *item, gpointer user_data)
{
    ScreenWindow *screenwin = SCREEN_WINDOW (user_data);

    gtk_widget_set_sensitive (screenwin->priv->stop_item, FALSE);
    stop_screencast (screenwin);
    update_tray_time (screenwin);
    gtk_widget_set_sensitive (screenwin->priv->start_item, TRUE);
}

static void
screen_quit_item_cb (GtkMenuItem *item, gpointer user_data)
{
    ScreenWindow *screenwin = SCREEN_WINDOW (user_data);

    screen_admin_update_notification (screenwin->priv->notify,
                                      _("Close application"),
                                      _("Application closed successfully"),
                                      "face-worried");

    destroy_screen_window (screenwin);
}

static void
screen_skip_item_cb (GtkMenuItem *item, gpointer user_data)
{
    ScreenWindow *screenwin = SCREEN_WINDOW (user_data);

    gtk_widget_set_sensitive (screenwin->priv->skip_item, FALSE);
    screen_stop_count_down (SCREEN_COUNT (screenwin->priv->count)); 
}

static void
screen_time_item_cb (GtkCheckMenuItem *item, gpointer user_data)
{
    ScreenWindow *screenwin = SCREEN_WINDOW (user_data);
 
    screenwin->priv->show_label = gtk_check_menu_item_get_active (item);
    if (!screenwin->priv->show_label)
        app_indicator_set_label (screenwin->priv->indicator, NULL, NULL);
    else if (screenwin->priv->minute == 0 && screenwin->priv->second == 0)
    {
        app_indicator_set_label (screenwin->priv->indicator, "00:00", "100%");
    }
}
static GtkWidget *get_menu_button (ScreenWindow *screenwin)
{
    GtkWidget *menu;
    GtkWidget *separator_item;
    GtkWidget *time_item;
    
    menu = gtk_menu_new ();

    screenwin->priv->settings_item = gtk_menu_item_new_with_label (_("Settings"));
    gtk_widget_set_sensitive (screenwin->priv->settings_item, TRUE);

    screenwin->priv->start_item = gtk_menu_item_new_with_label (_("Start new recording"));
    g_signal_connect (screenwin->priv->start_item,
                     "activate",
                      G_CALLBACK (screen_start_item_cb),
                      screenwin);
    gtk_widget_set_sensitive (screenwin->priv->start_item, FALSE);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), screenwin->priv->start_item);

    screenwin->priv->stop_item = gtk_menu_item_new_with_label (_("Stop recording"));
    g_signal_connect (screenwin->priv->stop_item,
                     "activate",
                      G_CALLBACK (screen_stop_item_cb),
                      screenwin);
    gtk_widget_set_sensitive (screenwin->priv->stop_item, FALSE);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), screenwin->priv->stop_item);

    screenwin->priv->skip_item = gtk_menu_item_new_with_label (_("Skip countdown"));
    gtk_widget_set_sensitive (screenwin->priv->skip_item, FALSE);
    g_signal_connect (screenwin->priv->skip_item,
                     "activate",
                      G_CALLBACK (screen_skip_item_cb),
                      screenwin);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), screenwin->priv->skip_item);

    separator_item = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), separator_item);

    time_item = gtk_check_menu_item_new_with_label (_("Show Time"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), time_item);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (time_item), TRUE);
    g_signal_connect (time_item,
                     "toggled",
                      G_CALLBACK (screen_time_item_cb),
                      screenwin);

    separator_item = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), separator_item);

    screenwin->priv->quit_item = gtk_menu_item_new_with_label (_("Quit recording"));
    g_signal_connect (screenwin->priv->quit_item,
                     "activate",
                      G_CALLBACK (screen_quit_item_cb),
                      screenwin);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), screenwin->priv->quit_item);

    gtk_widget_show_all (menu);

    return menu;
}
static void set_widget_css (GtkWidget *box)
{
    GtkCssProvider  *provider;
    GtkStyleContext *context;
    gchar           *css = NULL;

    provider = gtk_css_provider_new ();
    context = gtk_widget_get_style_context (box);
    css = g_strdup_printf ("* {background-color:rgba(252,252,252,100);min-height: 1px;}");
    gtk_css_provider_load_from_data (provider, css, -1, NULL);
    gtk_style_context_add_provider (context,
                                    GTK_STYLE_PROVIDER (provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref (provider);
    g_free (css);
}

static void create_custom_indicator (ScreenWindow *screenwin)
{
    GtkWidget *dialog;
    GtkWidget *button;
    GtkWidget *image;
    GIcon     *icon;
    GtkWidget *menu;

    button = gtk_menu_button_new ();
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);

    icon = g_themed_icon_new ("camera-video");
    image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_BUTTON);
    g_object_unref (icon);

    gtk_container_add (GTK_CONTAINER (button), image);
    gtk_widget_show (button);

    dialog = gtk_dialog_new_with_buttons (_("Recording Management"),GTK_WINDOW (screenwin),GTK_DIALOG_DESTROY_WITH_PARENT,NULL,NULL);
    set_widget_css (dialog);
    gtk_container_set_border_width (GTK_CONTAINER (dialog), 0);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button,GTK_RESPONSE_OK);	

    menu = get_menu_button (screenwin);
    gtk_widget_show_all (menu);
    gtk_menu_button_set_popup (GTK_MENU_BUTTON (button), menu); 
    gtk_window_set_deletable(GTK_WINDOW (dialog), FALSE);
    gtk_window_set_resizable(GTK_WINDOW (dialog), FALSE); 
    screenwin->priv->dialog = dialog;
    //gtk_widget_show_all (dialog);
}
static void create_screencast_indicator (ScreenWindow *screenwin)
{
    GtkWidget *menu;

    menu = get_menu_button (screenwin);

    screenwin->priv->indicator = app_indicator_new ("screen-admin",
                                   "camera-video",
                                    APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_attention_icon_full(screenwin->priv->indicator, "screen-start", "Local Attention Icon");
    app_indicator_set_status (screenwin->priv->indicator, APP_INDICATOR_STATUS_ATTENTION);
    app_indicator_set_label (screenwin->priv->indicator, "00:00", "100%");

    app_indicator_set_title (screenwin->priv->indicator, "screen-admin");
    app_indicator_set_menu (screenwin->priv->indicator, GTK_MENU(menu));
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
    ScreenSave      *save;

    save = SCREEN_SAVE (screenwin->priv->save);

    variant = get_screencast_variant (screenwin);
    screenwin->priv->save_path = get_screencast_save_path (save);

    g_dbus_proxy_call (screenwin->priv->proxy,
                      "Screencast",
                       g_variant_new ("(sa{sv})", screenwin->priv->save_path, variant),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       NULL,
                       NULL);

}
static void
stop_screencast_done (GObject      *source_object,
                      GAsyncResult *res,
                      gpointer      data)
{
    g_autoptr(GError) error = NULL;
    GVariant    *result;
    ScreenWindow *screenwin = SCREEN_WINDOW (data);

    result = g_dbus_proxy_call_finish (G_DBUS_PROXY (source_object), res, &error);

    if (result == NULL)
    {
        if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        {
            g_printerr ("Error setting OSD's visibility: %s\n", error->message);
        }
    }

    screen_admin_update_notification (screenwin->priv->notify,
                                      _("End of recording"),
                                      screenwin->priv->save_path,
                                      "face-cool"); 

}

static void stop_screencast (ScreenWindow *screenwin)
{
    g_dbus_proxy_call(screenwin->priv->proxy,
                     "StopScreencast",
                      g_variant_new ("()"),
                      G_DBUS_CALL_FLAGS_NONE,
                      -1,
                      NULL,
                      (GAsyncReadyCallback) stop_screencast_done,
                      screenwin);
}

static gboolean
screen_time_changed (gpointer user_data)
{
    ScreenWindowPrivate *priv = (ScreenWindowPrivate*) user_data;

    priv->second++;
    if (priv->second >= 60)
    {
        priv->second = 0;
        priv->minute++;
    }
    if (priv->show_label)
    {
        gchar * percentstr = g_strdup_printf("%02u:%02u", priv->minute, priv->second);
        app_indicator_set_label (priv->indicator, percentstr, "100%");
        g_free(percentstr);
    }
    else 
    {
        app_indicator_set_label (priv->indicator, NULL, NULL);
    }
    return TRUE;
}

static void create_indicator_time (ScreenWindowPrivate *priv)
{
    if (priv->show_label)
        app_indicator_set_label (priv->indicator, "00:01", "100%");
    priv->second = 1;
    priv->tray_timeout_id  = g_timeout_add_seconds(1, screen_time_changed, priv);
}
static void countdown_finished_cb (ScreenCount *count, gpointer user_data)
{
    ScreenWindow    *screenwin = SCREEN_WINDOW (user_data);

    start_screencast (screenwin);
    create_indicator_time (screenwin->priv);
    gtk_widget_set_sensitive (screenwin->priv->stop_item, TRUE);
    gtk_widget_set_sensitive (screenwin->priv->skip_item, FALSE);
}

static void screencast_button_cb (GtkWidget *button, gpointer user_data)
{
    ScreenWindow *screenwin = SCREEN_WINDOW (user_data);
    ScreenCount  *count = SCREEN_COUNT (screenwin->priv->count);

    gtk_widget_hide (GTK_WIDGET (screenwin));
    if (screenwin->priv->dialog != NULL)
        gtk_widget_show_all (screenwin->priv->dialog);

    gtk_widget_set_sensitive (screenwin->priv->skip_item, TRUE);
    screen_start_count_down (count);

}

static GtkWidget *create_start_and_stop_screencast (ScreenWindow *screenwin)
{
    GtkWidget *hbox;
    GtkWidget *button;

    ScreenCount  *count = SCREEN_COUNT (screenwin->priv->count);
    hbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
    button = gtk_button_new_with_label (_("Start Recording"));

    g_signal_connect (button,
                     "clicked",
                     (GCallback) screencast_button_cb,
                      screenwin);

    g_signal_connect (count,
                      "finished",
                     (GCallback) countdown_finished_cb,
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
    screen_admin_update_notification (screenwin->priv->notify,
                                     _("Start application"),
                                     _("The recording program is ready. Please start recording"),
                                     "face-smile");    

    return obj;
}
static void
screen_window_dispose (GObject *object)
{
    ScreenWindow *screenwin;

    screenwin = SCREEN_WINDOW (object);
    g_object_unref (screenwin->priv->proxy);

    if (screenwin->priv->save_path != NULL)
    {
        g_free (screenwin->priv->save_path);
    }
    if (screenwin->priv->tray_timeout_id != 0)
    {
        g_source_remove (screenwin->priv->tray_timeout_id);
        screenwin->priv->tray_timeout_id = 0;
    }
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
    GtkWindow  *window;

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
    screenwin->priv->show_label = TRUE;
    if (use_appindicator () != TRUE)
    {
        create_custom_indicator (screenwin);
    }
    else
    {
        if (screenwin->priv->indicator == NULL)
        {
            create_screencast_indicator (screenwin);
        }
    }
    screenwin->priv->notify = get_notification ();
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

void destroy_screen_window (ScreenWindow *screenwin)
{
    gtk_widget_destroy (screenwin->priv->style);
    gtk_widget_destroy (screenwin->priv->save);
    gtk_widget_destroy (screenwin->priv->stop);
    gtk_widget_destroy (screenwin->priv->count);
    gtk_widget_destroy (GTK_WIDGET (screenwin));
}
