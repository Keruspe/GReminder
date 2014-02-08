/*
 *      This file is part of GReminder.
 *
 *      Copyright 2014 Marc-Antoine Perennou <Marc-Antoine@Perennou.com>
 *
 *      GReminder is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      GReminder is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with GReminder.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "greminder-list-window-private.h"

#include "greminder-row.h"

struct _GReminderListWindowPrivate
{
    GtkListBox      *list;

    GReminderWindow *win;

    gulong           activated_id;
};

G_DEFINE_TYPE_WITH_PRIVATE (GReminderListWindow, g_reminder_list_window, GTK_TYPE_WINDOW)

static void
on_row_activated (GtkListBox    *list_box G_GNUC_UNUSED,
                  GtkListBoxRow *row,
                  gpointer       user_data)
{
    GReminderListWindow *self = user_data;
    GReminderListWindowPrivate *priv = g_reminder_list_window_get_instance_private (self);

    g_reminder_window_edit (priv->win, g_reminder_row_get_item (G_REMINDER_ROW (row)));
    gtk_window_close (GTK_WINDOW (self));
}

static void
g_reminder_list_window_dispose (GObject *object)
{
    GReminderListWindowPrivate *priv = g_reminder_list_window_get_instance_private (G_REMINDER_LIST_WINDOW (object));

    if (priv->activated_id)
    {
        g_signal_handler_disconnect (priv->list, priv->activated_id);
        priv->activated_id = 0;
    }

    G_OBJECT_CLASS (g_reminder_list_window_parent_class)->dispose (object);
}

static void
g_reminder_list_window_class_init (GReminderListWindowClass *klass)
{
    G_OBJECT_CLASS (klass)->dispose = g_reminder_list_window_dispose;
}

static void
g_reminder_list_window_init (GReminderListWindow *self)
{
    GReminderListWindowPrivate *priv = g_reminder_list_window_get_instance_private ((GReminderListWindow *) self);

    GtkWidget *lbox = gtk_list_box_new ();
    priv->list = GTK_LIST_BOX (lbox);
    priv->activated_id = g_signal_connect (G_OBJECT (lbox),
                                           "row-activated",
                                           G_CALLBACK (on_row_activated),
                                           self);
    gtk_container_add (GTK_CONTAINER (self), lbox);
}

G_REMINDER_VISIBLE GtkWindow *
g_reminder_list_window_new (GReminderWindow *win,
                            const GSList    *items)
{
    GtkWidget *self = gtk_widget_new (G_REMINDER_TYPE_LIST_WINDOW,
                                      "type",            GTK_WINDOW_TOPLEVEL,
                                      "window-position", GTK_WIN_POS_CENTER,
                                      "resizable",       FALSE,
                                      NULL);
    GReminderListWindowPrivate *priv = g_reminder_list_window_get_instance_private (G_REMINDER_LIST_WINDOW (self));

    priv->win = win;

    GtkContainer *lb = GTK_CONTAINER (priv->list);
    for (const GSList *i = items; i; i = g_slist_next (i))
        gtk_container_add (lb, g_reminder_row_new (i->data));

    return GTK_WINDOW (self);
}
