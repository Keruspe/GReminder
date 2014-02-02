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

#include "greminder-window-private.h"

#include "greminder-actions.h"
#include "greminder-keywords-widget.h"
#include "greminder-row.h"

struct _GReminderWindowPrivate
{
    GReminderKeywordsWidget *keywords;
    GtkTextView             *text;
    GtkSearchEntry          *search;

    GReminderActions        *actions;
    GReminderDb             *db;

    GReminderItem           *item;
    GtkWindow               *list;
    GtkListBox              *listbox;

    gulong                   c_signals[G_REMINDER_ACTION_LAST + 2];
};

G_DEFINE_TYPE_WITH_PRIVATE (GReminderWindow, g_reminder_window, GTK_TYPE_APPLICATION_WINDOW)

#define ON_ACTION_PROTO(name)                           \
    static void                                         \
    on_##name (GReminderActions *actions G_GNUC_UNUSED, \
               gpointer          user_data)

static void
g_reminder_window_private_set_item (GReminderWindowPrivate *priv)
{
    const gchar *text;
    g_object_get (G_OBJECT ((gtk_text_view_get_buffer (priv->text))), "text", &text, NULL);

    g_clear_object (&priv->item);
    priv->item =  g_reminder_item_new (g_reminder_keywords_widget_get_keywords (priv->keywords), text);
}

ON_ACTION_PROTO (new)
{
    GReminderWindowPrivate *priv = user_data;

    g_clear_object (&priv->item);
    g_reminder_actions_set_editable (priv->actions, FALSE);
    g_reminder_keywords_widget_reset (priv->keywords);
    gtk_text_buffer_set_text (gtk_text_view_get_buffer (priv->text), "", -1);
}

ON_ACTION_PROTO (delete)
{
    GReminderWindowPrivate *priv = user_data;

    g_reminder_db_delete (priv->db, priv->item);
    on_new (actions, user_data);
}

ON_ACTION_PROTO (save)
{
    GReminderWindowPrivate *priv = user_data;

    g_reminder_window_private_set_item (priv);
    g_reminder_db_save (priv->db, priv->item);
    g_reminder_actions_set_editable (priv->actions, TRUE);
}

ON_ACTION_PROTO (edit)
{
    GReminderWindowPrivate *priv = user_data;
    
    G_REMINDER_CLEANUP_UNREF GReminderItem *old = g_object_ref (priv->item);
    on_save (actions, user_data);

    if (g_strcmp0 (g_reminder_item_get_contents (old), g_reminder_item_get_contents (priv->item))) /* FIXME: g_reminder_item_equals (for keywords) */
        g_reminder_db_delete (priv->db, old);
}

static void
g_reminder_window_private_destroy_list (GReminderWindowPrivate *priv)
{
    if (priv->list)
    {
        g_signal_handler_disconnect (priv->listbox, priv->c_signals[G_REMINDER_ACTION_LAST + 1]);
        gtk_window_close (priv->list);
        priv->list = NULL;
    }
}

static void
on_row_activated (GtkListBox    *list_box G_GNUC_UNUSED,
                  GtkListBoxRow *row,
                  gpointer       user_data)
{
    GReminderWindowPrivate *priv = user_data;

    g_clear_object (&priv->item);
    priv->item = g_object_ref (g_reminder_row_get_item (G_REMINDER_ROW (row)));

    g_reminder_actions_set_editable (priv->actions, TRUE);
    g_reminder_keywords_widget_reset_with_data (priv->keywords, g_reminder_item_get_keywords (priv->item));
    gtk_text_buffer_set_text (gtk_text_view_get_buffer (priv->text), g_reminder_item_get_contents (priv->item), -1);

    g_reminder_window_private_destroy_list (priv);
}

static void
on_search (GtkEntry *entry,
           gpointer  user_data)
{
    GReminderWindowPrivate *priv = user_data;

    GSList *items = g_reminder_db_find (priv->db, gtk_entry_get_text (entry));

    GtkWidget *listbox = gtk_list_box_new ();
    priv->listbox = GTK_LIST_BOX (listbox);
    priv->c_signals[G_REMINDER_ACTION_LAST + 1] = g_signal_connect (G_OBJECT (listbox),
                                                                    "row-activated",
                                                                    G_CALLBACK (on_row_activated),
                                                                    priv);

    GtkContainer *lb = GTK_CONTAINER (listbox);
    for (GSList *i = items; i; i = g_slist_next (i))
        gtk_container_add (lb, g_reminder_row_new (i->data));

    GtkWidget *win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    priv->list = GTK_WINDOW (win);
    gtk_container_add (GTK_CONTAINER (win), listbox);
    gtk_widget_show_all (win);

    g_slist_free_full (items, g_object_unref);
}

static struct
{
    const gchar *name;
    GCallback    callback;
} actions[G_REMINDER_ACTION_LAST] = {
    [G_REMINDER_ACTION_NEW]    = { "new",    G_CALLBACK (on_new)    },
    [G_REMINDER_ACTION_DELETE] = { "delete", G_CALLBACK (on_delete) },
    [G_REMINDER_ACTION_EDIT]   = { "edit",   G_CALLBACK (on_edit)   },
    [G_REMINDER_ACTION_SAVE]   = { "save",   G_CALLBACK (on_save)   }
};

static void
g_reminder_window_dispose (GObject *object)
{
    GReminderWindowPrivate *priv = g_reminder_window_get_instance_private (G_REMINDER_WINDOW (object));

    if (priv->db)
    {
        for (GReminderAction a = G_REMINDER_ACTION_FIRST; a != G_REMINDER_ACTION_LAST; ++a)
            g_signal_handler_disconnect (priv->actions, priv->c_signals[a]);
        g_signal_handler_disconnect (priv->search, priv->c_signals[G_REMINDER_ACTION_LAST]);
    }

    g_clear_object (&priv->db);
    g_clear_object (&priv->item);

    g_reminder_window_private_destroy_list (priv);

    G_OBJECT_CLASS (g_reminder_window_parent_class)->dispose (object);
}

static void
g_reminder_window_class_init (GReminderWindowClass *klass)
{
    G_OBJECT_CLASS (klass)->dispose = g_reminder_window_dispose;
}

static void
g_reminder_window_init (GReminderWindow *self)
{
    GReminderWindowPrivate *priv = g_reminder_window_get_instance_private ((GReminderWindow *) self);

    priv->item = NULL;
    priv->list = NULL;

    GtkWidget *bar = gtk_header_bar_new ();
    GtkHeaderBar *header_bar = GTK_HEADER_BAR (bar);
    gtk_header_bar_set_title (header_bar, PACKAGE_NAME);
    gtk_header_bar_set_show_close_button (header_bar, TRUE);
    gtk_window_set_titlebar (GTK_WINDOW (self), bar);

    GtkWidget *sentry = gtk_search_entry_new ();
    priv->search = GTK_SEARCH_ENTRY (sentry);
    gtk_entry_set_width_chars (GTK_ENTRY (sentry), 40);
    priv->c_signals[G_REMINDER_ACTION_LAST] = g_signal_connect (G_OBJECT (sentry),
                                                                "activate",
                                                                G_CALLBACK (on_search),
                                                                priv);
    gtk_header_bar_pack_start (header_bar, sentry);

    GtkWidget *as = g_reminder_actions_new ();
    priv->actions = G_REMINDER_ACTIONS (as);
    for (GReminderAction a = G_REMINDER_ACTION_FIRST; a != G_REMINDER_ACTION_LAST; ++a)
    {
        priv->c_signals[a] = g_signal_connect (G_OBJECT (as),
                                               actions[a].name,
                                               actions[a].callback,
                                               priv);
    }
    gtk_header_bar_pack_end (header_bar, as);

    GtkWidget *grid = gtk_grid_new ();
    GtkGrid *g = GTK_GRID (grid);
    gtk_grid_set_column_spacing (g, 10);
    gtk_grid_set_row_spacing (g, 10);
    g_object_set (G_OBJECT (grid), "margin", 12, NULL);

    GtkWidget *align = gtk_alignment_new (0, 0, 0, 0);
    gtk_container_add (GTK_CONTAINER (align), gtk_label_new ("Keywords:"));
    gtk_grid_attach (g, align, 0, 0, 1, 1);

    GtkWidget *keywords = g_reminder_keywords_widget_new ();
    priv->keywords = G_REMINDER_KEYWORDS_WIDGET (keywords);
    gtk_grid_attach_next_to (g, keywords, align, GTK_POS_RIGHT, 2, 1);

    align = gtk_alignment_new (0, 0, 0, 0);
    gtk_container_add (GTK_CONTAINER (align), gtk_label_new ("Contents:"));
    gtk_grid_attach (g, align, 0, 1, 1, 1);

    GtkWidget *text = gtk_text_view_new ();
    priv->text = GTK_TEXT_VIEW (text);
    gtk_text_view_set_wrap_mode (priv->text, GTK_WRAP_WORD);
    GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);
    GtkScrolledWindow *s = GTK_SCROLLED_WINDOW (scroll);
    gtk_scrolled_window_set_min_content_height (s, 500);
    gtk_scrolled_window_set_min_content_width (s, 800);
    //gtk_scrolled_window_set_policy (s, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (scroll), text);
    gtk_grid_attach_next_to (g, scroll, align, GTK_POS_RIGHT, 2, 1);

    gtk_container_add (GTK_CONTAINER (self), grid);
}

G_REMINDER_VISIBLE GtkWidget *
g_reminder_window_new (GtkApplication *app,
                       GReminderDb    *db)
{
    g_return_val_if_fail (GTK_IS_APPLICATION (app), NULL);
    g_return_val_if_fail (G_REMINDER_IS_DB (db), NULL);

    GtkWidget *self = gtk_widget_new (G_REMINDER_TYPE_WINDOW,
                                      "application",     app,
                                      "type",            GTK_WINDOW_TOPLEVEL,
                                      "window-position", GTK_WIN_POS_CENTER,
                                      "resizable",       FALSE,
                                      NULL);
    GReminderWindowPrivate *priv = g_reminder_window_get_instance_private (G_REMINDER_WINDOW (self));

    priv->db = g_object_ref (db);

    return self;
}
