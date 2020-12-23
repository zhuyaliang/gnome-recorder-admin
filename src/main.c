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
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "screen-window.h"
#include "screen-stop.h"
#include "screen-save.h"
#include "screen-count.h"
#include "config.h"

static void app_quit(GtkWidget *object,
                     gpointer   user_data)
{
    gtk_widget_destroy (object);
    exit (0);
}

static void create_pid_file (void)
{
    pid_t pid;
    int   fd;
    char *lock_file;

    pid = getpid();
    lock_file = g_strdup_printf ("%s/%d", LOCKDIR, pid);
    fd = creat (lock_file, 0777);
    g_free (lock_file);
    close (fd);
}
static gboolean check_process_already_running (void)
{
    DIR  *dir;
    pid_t pid;
    struct dirent *ptr;

    umask(0);

    if (access(LOCKDIR,F_OK) !=0)
    {
        mkdir (LOCKDIR, S_IRWXU|S_IRWXO|S_IRWXG);
        create_pid_file ();
        return FALSE;
    }
    else
    {
        dir = opendir(LOCKDIR);
        while ((ptr = readdir(dir)) != NULL)
        {
            if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)
                continue;
            if(ptr->d_type == 8)
            {
                pid = atoi (ptr->d_name);
                if(kill(pid, 0) == 0)
                {
                    return TRUE;
                }
            }
        }
        closedir(dir);
    }
    create_pid_file ();
    return FALSE;
}
int
main (int argc, char **argv)
{
    GtkWidget *window;

    bindtextdomain (GETTEXT_PACKAGE,LUNAR_CALENDAR_LOCALEDIR);
    textdomain (GETTEXT_PACKAGE);

    if (check_process_already_running ())
    {
        return 0;
    }

    gtk_init (&argc, &argv);

    window = screen_window_new ();
    g_signal_connect (window, "destroy",
                      G_CALLBACK (app_quit), &window);

    gtk_widget_show_all (window);
    gtk_main ();
    return 0;
}
