/*************************************************************************
  File Name: screen-stop.c
  
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

#include "screen-stop.h"

#define GNOME_DAEMON_MEDAI_SCHEMA              "org.gnome.settings-daemon.plugins.media-keys"
#define GNOME_MAX_SCREENCAST_LENGTH_KEY        "max-screencast-length"

struct _ScreenStopPrivate
{
    GSettings *settings;
    int        stop_time;
    int        stop_size;
    stop_type  stop_mode;
};
enum
{
    PROP_0,
    PROP_STOP_TIME,
    PROP_STOP_SIZE,
};

G_DEFINE_TYPE_WITH_PRIVATE (ScreenStop, screen_stop, GTK_TYPE_FRAME)
static void
stop_by_time_cb (GtkRadioButton *button,
                 gpointer        user_data)
{
    ScreenStop *stop = SCREEN_STOP (user_data);
    
    stop->priv = screen_stop_get_instance_private (stop);
    stop->priv->stop_mode = STOP_BY_TIME;
}
static void
stop_by_size_cb (GtkRadioButton *button,
                 gpointer        user_data)
{
    ScreenStop *stop = SCREEN_STOP (user_data);
    
    stop->priv = screen_stop_get_instance_private (stop);
    stop->priv->stop_mode = STOP_BY_SIZE;
}
static void
stop_by_manual_cb (GtkRadioButton *button,
                   gpointer        user_data)
{
    ScreenStop *stop = SCREEN_STOP (user_data);
    
    stop->priv = screen_stop_get_instance_private (stop);
    stop->priv->stop_mode = STOP_BY_MANUALL;
}
static void
screen_stop_dispose (GObject *object)
{
    ScreenStop *stop = SCREEN_STOP (object);
    
    if (stop->priv->settings)
        g_object_unref (stop->priv->settings);
    stop->priv->settings = NULL;
}

static void
screen_stop_get_property (GObject    *object,
                          guint       param_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    ScreenStop *stop = SCREEN_STOP (object);

    switch (param_id)
    {
        case PROP_STOP_TIME:
            g_value_set_int (value, stop->priv->stop_time);
            break;
        case PROP_STOP_SIZE:
            g_value_set_int (value, stop->priv->stop_size);
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
            break;
    }
}

static void
screen_stop_set_property (GObject      *object,
                          guint         param_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    ScreenStop *stop = SCREEN_STOP (object);

    switch (param_id)
    {
        case PROP_STOP_TIME:
            stop->priv->stop_time = g_value_get_int (value);
            break;
        case PROP_STOP_SIZE:
            stop->priv->stop_size = g_value_get_int (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
            break;
    }
}

static void
screen_stop_init (ScreenStop *stop)
{
    stop->priv = screen_stop_get_instance_private (stop);
    
    GtkWidget *table;
    GtkWidget *label;
    GtkWidget *spin;
    GtkWidget *radio1,*radio2,*radio3;
    GSList    *radio_group;
    uint       length;
	gchar     *text;
    
    stop->priv->settings = g_settings_new (GNOME_DAEMON_MEDAI_SCHEMA);
    length = g_settings_get_uint (stop->priv->settings, GNOME_MAX_SCREENCAST_LENGTH_KEY);
    if (length <= 1)
       length = 86400; 
    table = gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (stop), table);
    gtk_grid_set_row_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(table), TRUE);

    radio1 = gtk_radio_button_new_with_label (NULL, _("stop by time (S)"));        
    g_signal_connect (radio1,
                     "toggled",
                      G_CALLBACK (stop_by_time_cb),
                      stop);
    gtk_grid_attach(GTK_GRID(table), radio1, 0, 0, 1, 1);
    
    spin = gtk_spin_button_new_with_range (2, 86400, 1);
    g_object_bind_property (spin, "value", stop, "stop_time", 0);
    gtk_grid_attach(GTK_GRID(table), spin, 1, 0, 1, 1);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), (double)length);

    radio_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio1)); 
    radio2 = gtk_radio_button_new_with_label (radio_group, _("stop by size (MB)"));        
    g_signal_connect (radio2,
                     "toggled",
                      G_CALLBACK (stop_by_size_cb),
                      stop);
    gtk_grid_attach(GTK_GRID(table), radio2, 0, 1, 1, 1);
    
    spin = gtk_spin_button_new_with_range (1, 10240, 1);
    g_object_bind_property (spin, "value", stop, "stop_size", 0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), 2);
    gtk_grid_attach(GTK_GRID(table), spin, 1, 1, 1, 1);

    radio_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio1)); 
    radio3 = gtk_radio_button_new_with_label (radio_group, _("stop by manual"));
    g_signal_connect (radio3,
                     "toggled",
                      G_CALLBACK (stop_by_manual_cb),
                      stop);
    gtk_grid_attach(GTK_GRID(table), radio3, 0, 2, 1, 1);

}

static void
screen_stop_class_init (ScreenStopClass *stop_class)
{
    GObjectClass *gobject_class;
    
    gobject_class = G_OBJECT_CLASS (stop_class);
    gobject_class->set_property = screen_stop_set_property;
    gobject_class->get_property = screen_stop_get_property;

    gobject_class->dispose      = screen_stop_dispose;
    g_object_class_install_property (
            gobject_class,
            PROP_STOP_TIME,
            g_param_spec_int (
                    "stop-time",
                    "Stop Time",
                    "Maximum screen recording time",
                     2,86400,30,
                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property (
            gobject_class,
            PROP_STOP_SIZE,
            g_param_spec_int (
                    "stop-size",
                    "Stop size",
                    "Maximum screen recording time",
                     1,10240,5,
                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

GtkWidget *
screen_stop_new (const char *title)
{
    ScreenStop  *stop = NULL;
    GtkWidget   *label;
    char        *text;

    stop = g_object_new (SCREEN_TYPE_STOP, NULL);
    gtk_frame_set_label (GTK_FRAME (stop), "");
    text =  g_markup_printf_escaped("<span color = \'grey\' size=\"%s\" weight='bold'>%s</span>","large",title);
    label = gtk_frame_get_label_widget (GTK_FRAME (stop));
    gtk_label_set_markup (GTK_LABEL (label), text);

    g_free (text);

    return GTK_WIDGET (stop);
}
int screen_stop_get_stop_time (ScreenStop *stop)
{
    return stop->priv->stop_time;
}

int screen_stop_get_stop_size (ScreenStop *stop)
{
    return stop->priv->stop_size;
}

stop_type screen_stop_get_stop_mode (ScreenStop *stop)
{
    return stop->priv->stop_mode;
}
