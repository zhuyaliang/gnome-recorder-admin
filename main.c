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
#include <gtk/gtk.h>

static void set_lable_font_type(GtkWidget  *lable ,
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

static GtkTreeModel *
create_icon_store (void)
{
	GtkTreeIter iter;
	GtkListStore *store;

	store = gtk_list_store_new (1, G_TYPE_STRING);
    gtk_list_store_append (store, &iter);
	gtk_list_store_set(store,&iter,0,"30",-1);
    
	gtk_list_store_append (store, &iter);
	gtk_list_store_set(store,&iter,0,"25",-1);

    gtk_list_store_append (store, &iter);
	gtk_list_store_set(store,&iter,0,"20",-1);
    
	gtk_list_store_append (store, &iter);
	gtk_list_store_set(store,&iter,0,"15",-1);
		
	return GTK_TREE_MODEL (store);
}

static GtkWidget *create_screencast_run_setting (void)
{
	GtkWidget *frame;
    GtkWidget *table;
	GtkWidget *label;
	GtkWidget *check_button;
	GtkWidget *combo;
	GtkTreeModel *model;
	gchar     *text;

	text =  g_markup_printf_escaped("<span color = \'grey\' size=\"%s\" weight='bold'>%s</span>","large","Run Record Mode");
    frame = gtk_frame_new ("");
    label = gtk_frame_get_label_widget (GTK_FRAME (frame));

    gtk_label_set_markup (GTK_LABEL (label),text);

    table = gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_grid_set_row_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(table), TRUE);

    label = gtk_label_new("Display mouse when recording");
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(table), label, 0, 0, 1, 1);

	check_button = gtk_check_button_new_with_mnemonic ("Show cursor");
    gtk_grid_attach(GTK_GRID(table), check_button, 1, 0, 1, 1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), TRUE);

	label = gtk_label_new ("framerate");
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(table), label, 0, 1, 1, 1);

	model = create_icon_store ();
    combo = gtk_combo_box_new_with_model (model);
    GtkCellRenderer *Renderer;
	Renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),Renderer,TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo),Renderer,"text",0,NULL);
	gtk_grid_attach(GTK_GRID(table), combo, 1, 1, 1, 1);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0); 
	g_object_unref (model);

	//g_signal_connect (check_button, "toggled",
     //                   G_CALLBACK (toggle_grouping), size_group);
	return frame;
}

static GtkWidget *create_screencast_save_setting (void)
{
    GtkWidget *frame;
    GtkWidget *label;
    gchar     *text;
    GtkWidget *table;
	GtkWidget *picker;
	GtkWidget *entry;

	text =  g_markup_printf_escaped("<span color = \'grey\' size=\"%s\" weight='bold'>%s</span>","large","Save Mode");
    frame = gtk_frame_new ("");
    label = gtk_frame_get_label_widget (GTK_FRAME (frame));

    gtk_label_set_markup (GTK_LABEL (label),text);

    table = gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_grid_set_row_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(table), TRUE);

    label = gtk_label_new ("Folder:");
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
    picker = gtk_file_chooser_button_new ("Pick a Folder",
                                          GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	const char *video = g_get_user_special_dir (G_USER_DIRECTORY_VIDEOS);
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (picker), video);
	gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (table), picker, 1, 0, 1, 1);
    
	label = gtk_label_new ("FileName:");
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
	entry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (entry), "System default: current time");
	gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (table), entry, 1, 1, 1, 1);
	return frame;
}

static GtkWidget *create_screencast_stop_setting (void)
{
	GtkWidget *frame;
    GtkWidget *table;
	GtkWidget *label;
	GtkWidget *spin;
    GtkWidget *radio1,*radio2,*radio3;
    GSList    *radio_group;

	gchar     *text;
    
	text =  g_markup_printf_escaped("<span color = \'grey\' size=\"%s\" weight='bold'>%s</span>","large","Stop Record Mode");
    frame = gtk_frame_new ("");
    label = gtk_frame_get_label_widget (GTK_FRAME (frame));

    gtk_label_set_markup (GTK_LABEL (label),text);

    table = gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_grid_set_row_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_spacing(GTK_GRID(table), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(table), TRUE);

    radio1 = gtk_radio_button_new_with_label (NULL,"stop by time (S)");        
    gtk_grid_attach(GTK_GRID(table), radio1, 0, 0, 1, 1);
    spin = gtk_spin_button_new_with_range (30, 9999, 1);
    gtk_grid_attach(GTK_GRID(table), spin, 1, 0, 1, 1);
    label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(table), label, 2, 0, 1, 1);

    radio_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio1)); 
    radio2 = gtk_radio_button_new_with_label (radio_group, "stop by size (MB)");        
    gtk_grid_attach(GTK_GRID(table), radio2, 0, 1, 1, 1);
    spin = gtk_spin_button_new_with_range (10, 9999, 1);
    gtk_grid_attach(GTK_GRID(table), spin, 1, 1, 1, 1);

    radio_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio1)); 
    radio3 = gtk_radio_button_new_with_label (radio_group, "stop by custom");        
    gtk_grid_attach(GTK_GRID(table), radio3, 0, 2, 1, 1);

	return frame;
}

static GtkWidget *create_countdown ()
{
    GtkWidget *hbox;
    GtkWidget *label;
	GtkWidget *spin;
	
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    label = gtk_label_new(NULL);
    set_lable_font_type(label,"gray",11,("countdown"),TRUE);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 12);
    spin = gtk_spin_button_new_with_range (0, 10, 1);
    gtk_box_pack_start (GTK_BOX (hbox), spin, FALSE, FALSE, 12);

	return hbox;
}

static GtkWidget *create_start_stop ()
{
	GtkWidget *hbox;
	GtkWidget *button_start,*button_stop;

	hbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	button_start = gtk_button_new_with_label ("Start");
    gtk_box_pack_start (GTK_BOX (hbox), button_start, FALSE, FALSE, 12);
	button_stop = gtk_button_new_with_label ("Stop");

	return hbox;
}
int
main (int argc, char **argv)
{
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *frame;

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Gnome Record Screen");
    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
    gtk_widget_set_size_request (window, 400, 400);
    g_signal_connect (window, "destroy",
                        G_CALLBACK (gtk_widget_destroyed), &window);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

	frame = create_screencast_run_setting ();
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 12);
    
	frame = create_screencast_stop_setting ();
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 12);

	frame = create_screencast_save_setting ();
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 20);
	
	hbox = create_countdown ();
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 12);

	hbox = create_start_stop ();
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 12);
	
    gtk_widget_show_all (window);
    gtk_main ();
    return 0;
}
