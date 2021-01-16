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
                                               
  Created Time: 2021年01月14日 星期四 12时02分57秒
 ************************************************************************/

#include <stdio.h>
#include <math.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <libintl.h>
#include <locale.h>


#define GETTEXT_PACKAGE          "SCREEN_AREA"
#define LUNAR_CALENDAR_LOCALEDIR "/usr/share/locale"

GdkWindow *win;
GdkCursor *last_cursor;
GdkDevice  *device;
int startx = 0;
int starty = 0;
int endx = 0;
int endy = 0;
int g_startx = 0;
int g_starty = 0;
int g_endx = 0;
int g_endy = 0;
int height = 0;
int width = 0;
int resize_handle = -1;
int move_offsetx = 0;
int move_offsety = 0;

gboolean compositing;

const int HANDLE_CURSORS[]= {
    GDK_TOP_LEFT_CORNER,
    GDK_TOP_SIDE,
    GDK_TOP_RIGHT_CORNER,
    GDK_LEFT_SIDE,
    GDK_FLEUR,
    GDK_RIGHT_SIDE,
    GDK_BOTTOM_LEFT_CORNER,
    GDK_BOTTOM_SIDE,
    GDK_BOTTOM_RIGHT_CORNER
};
typedef enum
{
    HANDLE_TL = 0,
    HANDLE_TC,
    HANDLE_TR,
    HANDLE_CL,
    HANDLE_MOVE,
    HANDLE_CR,
    HANDLE_BL,
    HANDLE_BC,
    HANDLE_BR,
}Resize;
static double in_circle (double center_x, double center_y, int radius, int x, int y)
{
    double dist;
    double x1, y1;
    
    x1 = pow ((center_x - x), 2);
    y1 = pow ((center_y - y), 2);
    dist = sqrt(x1 + y1);
    
    return dist <= radius;
}

static void accept_area(GtkWidget *widget)
{
    int tmp;

    gdk_window_set_cursor (win, last_cursor);
    gtk_widget_hide (widget);

    if (startx > endx)
    {
        tmp = startx;
        startx = endx;
        endx = tmp;
    }

    if (g_startx > g_endx)
    {
        tmp = g_startx;
        g_startx = g_endx;
        g_endx = tmp;
    }

    if (starty > endy)
    {
        tmp = starty;
        starty = endy;
        endy = tmp;
    }

    if (g_starty > g_endy)
    {
        tmp = g_starty;
        g_starty = g_endy;
        g_endy = tmp;
    }

    if (startx < 0)
        startx = 0;

    if (starty < 0)
        starty = 0;

    width  = ABS(endx - startx);
    height = ABS(endy - starty);

    g_print ("Selected coords: = g_startx = %d g_starty = %d g_endx = %d g_endy = %d\r\n",g_startx, g_starty, g_endx, g_endy);
}
static gboolean
cb_keypress_event (GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   user_data)
{
    guint16 keycode;
    
    gdk_event_get_keycode (event, &keycode);
    if (keycode == 36 || keycode == 104)// Enter
    {
        accept_area (widget);
        //emit("area-selected")
    
    } 
    else if (keycode == 9) //ESC
    {
        gdk_window_set_cursor (win, last_cursor);
        gtk_widget_hide (widget);
        //emit("area-canceled")
    }
}
static void outline_text(cairo_t *cr, int w, int h, double size, const char *text)
{
    cairo_text_extents_t extents;
    double cx, cy;

    cairo_set_font_size (cr, size);
    cairo_select_font_face (cr, "Ubuntu", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_text_extents (cr, text, &extents);
    cairo_set_line_width (cr, 2.0);

    cx = w/2 - extents.width/2;
    cy = h/2 - extents.height/2;
    
    if (compositing)
    {
        cairo_set_source_rgba(cr, 0.4, 0.4, 0.4, 1.0);
    } 
    else
    {
        cairo_set_source_rgb (cr, 0.4, 0.4, 0.4);
    }
    cairo_move_to (cr, cx, cy);
    cairo_text_path (cr, text);
    cairo_stroke (cr);
    if (compositing)
    {
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    } 
    else
    {
        cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    }
    cairo_move_to (cr, cx, cy);
    cairo_show_text (cr, text);
}

static gboolean
cb_draw (GtkWidget    *widget,
         cairo_t      *cairo,
         gpointer      user_data)
{
    int w, h;
    double centerx, centery;
    double x, y;
    g_autofree char *size = NULL; 
    int i = 0;
    
    cairo_pattern_t *grad;

    gtk_window_get_size (GTK_WINDOW (user_data), &w, &h);
    if (compositing)
    {
        cairo_set_source_rgba(cairo, 0.0, 0.0, 0.0, 0.45);
    } 
    else
    {
        cairo_set_source_rgb (cairo, 0.5, 0.5, 0.5);
    }
    cairo_set_operator (cairo, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cairo);
    cairo_set_line_width (cairo, 1.0);
    cairo_set_source_rgb (cairo, 1.0, 1.0, 1.0);
    cairo_rectangle (cairo, startx, starty, width, height);
    cairo_stroke (cairo);
    if (compositing)
    {
        cairo_set_source_rgba(cairo, 0.0, 0.0, 0.0, 0.0);
    } 
    else
    {
        cairo_set_source_rgb (cairo, 0.0, 0.0, 0.0);
    }
    cairo_rectangle (cairo, startx + 1, starty + 1, width - 2, height - 2);
    cairo_fill (cairo);
    cairo_set_operator (cairo, CAIRO_OPERATOR_OVER);
    
    for (i ; i < 9; i++)
    {
        if (i == HANDLE_MOVE)
            continue;
        x = floor(i % 3) / 2;
        y = floor(i / 3) / 2;
        
        centerx = startx + width * x;
        centery = starty + height * y;
        grad = cairo_pattern_create_radial(centerx, centery, 0, centerx, centery + 2, 10);
        cairo_pattern_add_color_stop_rgba (grad, 0.6, 0.0, 0.0, 0.0, 0.6);
        cairo_pattern_add_color_stop_rgba (grad, 0.75, 0.0, 0.0, 0.0, 0.25);
        cairo_pattern_add_color_stop_rgba (grad, 1.0, 0.0, 0.0, 0.0, 0.0);
        cairo_arc (cairo, centerx, centery, 10, 0, 2 * M_PI);
        cairo_set_source (cairo, grad);
        cairo_fill (cairo);
        
        grad = cairo_pattern_create_linear (centerx, centery - 8, centerx, centery + 8);
        cairo_pattern_add_color_stop_rgb (grad, 0.0, 0.75, 0.75, 0.75);
        cairo_pattern_add_color_stop_rgb (grad, 0.75, 0.95, 0.95, 0.95);
        cairo_arc (cairo, centerx, centery, 8, 0, 2 * M_PI);
        cairo_set_source (cairo, grad);
        cairo_fill (cairo);
    
        cairo_set_source_rgb (cairo, 1.0, 1.0, 1.0);
        cairo_arc (cairo, centerx, centery, 8, 0, 2 * M_PI);
        cairo_stroke (cairo);
    }
    size = g_strdup_printf ("%d X %d",ABS(width+1), ABS(height+1));
    outline_text(cairo, w, h, 30, _("Select an area by clicking and dragging."));
    outline_text(cairo, w, h + 50, 26, _("Press ENTER to confirm or ESC to cancel"));
    outline_text(cairo, w, h + 100, 20, size);
    cairo_set_operator (cairo, CAIRO_OPERATOR_SOURCE);
}
static gboolean
cb_draw_motion_notify_event (GtkWidget       *widget,
                             GdkEventMotion  *event,
                             gpointer         user_data)
{
    GdkWindow    *window;
    double x, y;
    int sh, sw, x1, y1;
    int ex, ey, sx, sy;
    double offsetx, offsety;
    GdkDisplay   *ds;
    GdkMonitor   *monitor;
    GdkRectangle  rect;
    GdkCursor    *cursor;
    GdkModifierType mask;
    int num;
    int i = 0;
    gboolean cursor_changed = FALSE;
    
    window = event->window;
    gdk_window_get_device_position (window, device, &x1, &y1, &mask);
    ds = gdk_device_get_display (device);
    num = gdk_display_get_n_monitors (ds);
    monitor = gdk_display_get_monitor (ds, num-1);
    gdk_monitor_get_geometry (monitor, &rect);

    ex = event->x;
    ey = event->y;
    sx = rect.x;
    sy = rect.y;
    
    for (i = 0; i < 9; i++)
    {
        x = floor(i % 3) / 2;
        y = floor(i / 3) / 2;
        offsetx = width * x;
        offsety = height * y;
    
        if (g_startx > g_endx)
           offsetx *= -1;
        if (g_starty > g_endy)
           offsety *= -1;
        
        if (in_circle(MIN(g_startx, g_endx) + offsetx, MIN(g_starty, g_endy) + offsety, 8, sx + ex, sy + ey))
        {
            cursor_changed = TRUE;
            cursor = gdk_cursor_new_for_display (ds, HANDLE_CURSORS[i]);
            gdk_window_set_cursor(win, cursor);
            break;
        }
        cursor = gdk_cursor_new_for_display (ds, GDK_CROSSHAIR);
        gdk_window_set_cursor(win, cursor);
    }
    if (!cursor_changed && \
         MIN(startx, endx) < ex  &&\
         ex < MAX(startx, endx) &&\
         MIN(starty, endy) < ey &&\
         ey < MAX(starty, endy))
    {
        cursor = gdk_cursor_new_for_display (ds, HANDLE_CURSORS[HANDLE_MOVE]);
        gdk_window_set_cursor(win, cursor);
    
    }

    if (mask & GDK_BUTTON1_MASK)
    {
        if (resize_handle == HANDLE_TL)
        {
            startx = ex;
            starty = ey;
            g_startx = sx + ex;
            g_starty = sy + ey;
        }
        else if(resize_handle == HANDLE_TC)
        {
            starty = ey;
            g_starty = sy + ey;
        
        }
        else if (resize_handle == HANDLE_TR)
        {
            endx = ex;
            starty = ey;
            g_endx = sx + ex;
            g_starty = sy + ey;
        }
        else if (resize_handle == HANDLE_CL)
        {
            startx = ex;
            g_startx = sx + ex;
        }
        else if (resize_handle == HANDLE_CR)
        {
            endx = ex;
            g_endx = sx + ex;
        }
        else if (resize_handle == HANDLE_BL)
        {
            startx = ex;
            endy = ey;
            g_startx = sx + ex;
            g_endy = sy + ey;
        }
        else if (resize_handle == HANDLE_BC)
        {
            endy = ey;
            g_endy = sy + ey;
        }
        else if (resize_handle == HANDLE_BR)
        {
            endx = ex;
            endy = ey;
            g_endx = sx + ex;
            g_endy = sy + ey;
        
        }
        else if (resize_handle == HANDLE_MOVE)
        {
            if (move_offsetx == move_offsety && move_offsety == 0)
            {
                move_offsetx = ex - startx;
                move_offsety = ey - starty;
            } 
            startx = MAX(0, ex - move_offsetx);
            starty = MAX(0, ey - move_offsety);
            endx = startx + width;
            endy = starty + height;
 
            sw = rect.width;
            sh = rect.height;

            if  (endx > sw)
            {
                startx -= endx - sw;
                endx = sw;
            }
            if (endy > sh)
            {
                starty -= endy - sh;
                endy = sh;
            }
            g_startx = sx + startx;
            g_starty = sy + starty;
            g_endx = sx + endx;
            g_endy = sy + endy;
        }
        else
        {
            endx = ex;
            endy = ey;
            g_endx = sx + ex;
            g_endy = sy + ey;
        }
        width  = endx - startx;
        height = endy - starty;

    } 
    gtk_widget_queue_draw (widget);
    return TRUE;
}

static gboolean
cb_draw_button_press_event (GtkWidget       *widget,
                            GdkEventButton  *event,
                            gpointer         user_data)
{
    GdkDisplay   *ds;
    int           num;
    double        x, y;
    int           ex, ey;
    double        offsetx, offsety;
    int           gx, gy;
    GdkMonitor   *monitor;
    GdkRectangle  rect;
    GdkCursor    *cursor;
    GdkModifierType mask;
    int i = 0;
    gboolean cursor_changed = FALSE;
    
    ds = gdk_device_get_display (device);
    num = gdk_display_get_n_monitors (ds);
    monitor = gdk_display_get_monitor (ds, num-1);
    gdk_monitor_get_geometry (monitor, &rect);

    ex = event->x;
    ey = event->y;
    gx = rect.x + ex;
    gy = rect.y + ey;

    for(i; i < 9; i++)
    {
        x = floor(i % 3) / 2;
        y = floor(i / 3) / 2;
        offsetx = width * x;
        offsety = height * y;
        
        if (in_circle(g_startx + offsetx, g_starty + offsety, 8, gx, gy))
        {
            resize_handle = i;
            return TRUE;
        }
    }
    if (MIN(startx, endx) < ex && ex < MAX(startx, endx) &&\
        MIN(starty, endy) < ey && ey < MAX(starty, endy))
    {
        if (event->type == GDK_2BUTTON_PRESS)
        {
            accept_area(GTK_WIDGET (user_data));
           // emit("area-selected")
        }

        resize_handle = HANDLE_MOVE;
        return TRUE;
    }
    startx = ex;
    starty = ey;
    g_startx = gx;
    g_starty = gy;
    endx = 0;
    endy = 0;
    g_endx = 0;
    g_endy = 0;
    width  = 0;
    height = 0;
}
 
static gboolean
cb_draw_button_release_event (GtkWidget       *widget,
                              GdkEventButton  *event,
                              gpointer         user_data)
{
    resize_handle = -1;
    move_offsetx = 0;
    move_offsety = 0;
}
static gboolean
cb_leave_notify_event (GtkWidget       *widget,
                       GdkEvent        *event,
                       gpointer         user_data)
{
    GdkDisplay   *ds;
    int           num;
    int           x, y;
    GdkMonitor   *monitor;
    GdkScreen    *screen;
    GdkRectangle  rect;
    
    gdk_device_get_position (device, &screen, &x, &y);
    ds = gdk_device_get_display (device);
    num = gdk_display_get_n_monitors (ds);
    monitor = gdk_display_get_monitor (ds, num-1);
    gdk_monitor_get_geometry (monitor, &rect);
    
    if( x > 0 || y > 0)
    {
        gtk_window_unfullscreen (GTK_WINDOW (user_data));
        gtk_window_move (GTK_WINDOW (user_data), rect.x, rect.y); 
        gtk_window_fullscreen (GTK_WINDOW (user_data));
    }
    
    return TRUE;
}
int
main (int argc, char **argv)
{
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *drawing;
    GdkVisual *visual;   
    int           num;
    GdkMonitor   *monitor;
    GdkRectangle  rect;
    GdkCursor    *cursor;
    GdkScreen  *screen;
    GdkDisplay *display;
    GdkSeat    *seat;

    bindtextdomain (GETTEXT_PACKAGE,LUNAR_CALENDAR_LOCALEDIR);
    textdomain (GETTEXT_PACKAGE);

    gtk_init (&argc, &argv);
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    
    screen = gtk_widget_get_screen (window);
    visual = gdk_screen_get_rgba_visual (screen);
    win = gdk_screen_get_root_window (screen);
    display = gdk_window_get_display (win);
    last_cursor = gdk_cursor_new_for_display (display, GDK_LEFT_PTR);
    seat = gdk_display_get_default_seat (display);
    device = gdk_seat_get_pointer (seat);
    cursor = gdk_cursor_new_for_display (display, GDK_CROSSHAIR);
    gdk_window_set_cursor(win, cursor);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    drawing = gtk_drawing_area_new ();
    gtk_box_pack_start (GTK_BOX (vbox), drawing, TRUE, TRUE, 6);
    gtk_widget_set_size_request (drawing, 500, 500);

    g_signal_connect (window,
                     "delete-event",
                      G_CALLBACK (gtk_main_quit),
                      NULL);

    g_signal_connect (window,
                     "key-press-event",
                      G_CALLBACK (cb_keypress_event),
                      NULL);
    
    g_signal_connect (drawing,
                     "draw",
                      G_CALLBACK (cb_draw),
                      window);
    
    g_signal_connect (drawing,
                     "motion-notify-event",
                      G_CALLBACK (cb_draw_motion_notify_event),
                      window);

    g_signal_connect(drawing,
                    "button-press-event",
                     G_CALLBACK (cb_draw_button_press_event),
                     window);
    g_signal_connect(drawing,
                    "button-release-event",
                     G_CALLBACK (cb_draw_button_release_event),
                     window);
    g_signal_connect(drawing,
                    "leave-notify-event",
                     G_CALLBACK (cb_leave_notify_event),
                     window);
    gtk_widget_add_events(drawing, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                          GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | 
                          GDK_LEAVE_NOTIFY_MASK);

    gtk_container_set_border_width (GTK_CONTAINER (window), 0);
    gtk_widget_set_app_paintable (window, TRUE); 
    gtk_window_set_resizable (GTK_WINDOW (window), TRUE);    
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
    g_object_set (window, "skip-taskbar-hint", TRUE, NULL);
    gtk_window_set_keep_above (GTK_WINDOW (window), TRUE);

    if (visual != NULL && gdk_screen_is_composited (screen))
    {
        gtk_widget_set_visual (window, visual); 
        compositing = TRUE;
    
    }
    else
    {
        compositing = FALSE;
    }
    
    num = gdk_display_get_n_monitors (display);
    monitor = gdk_display_get_monitor (display, num - 1);
    gdk_monitor_get_geometry (monitor, &rect);

    gtk_window_move(GTK_WINDOW (window),rect.x,rect.y);
    gtk_window_fullscreen (GTK_WINDOW (window));


    gtk_widget_show_all (window);
    gtk_main ();
    return 0;
}
