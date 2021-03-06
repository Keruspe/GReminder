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
#include "greminder-list-window.h"

#include <string.h>

enum {
    C_ACTIVATE = _G_REMINDER_ACTION_LAST,
    C_FOCUS,
    C_PRESS,
    C_MATCH,
    C_VALID_CHANGED,
    C_CHANGED,
    C_LAST
};

struct _GReminderWindowPrivate
{
    GReminderKeywordsWidget *keywords;
    GtkWidget               *textview;
    GtkTextBuffer           *text;
    GtkSearchEntry          *search;
    GtkEntryCompletion      *completion;

    GReminderActions        *actions;
    GReminderDb             *db;

    GReminderItem           *item;

    GRegex                  *no_blank_regex;

    gboolean                 valid;
    gboolean                 kvalid;
    gboolean                 cvalid;

    gulong                   c_signals[C_LAST];
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
    g_object_get (G_OBJECT (priv->text), "text", &text, NULL);

    g_clear_object (&priv->item);
    priv->item =  g_reminder_item_new (g_reminder_keywords_widget_get_keywords (priv->keywords), text);
}

static void
g_reminder_window_private_reset_completion (GReminderWindowPrivate *priv)
{
    GtkTreeModel *model = gtk_entry_completion_get_model (priv->completion);
    g_clear_object (&model);
    gtk_entry_completion_set_model (priv->completion, GTK_TREE_MODEL (g_reminder_db_get_keywords (priv->db)));
}

ON_ACTION_PROTO (new)
{
    GReminderWindowPrivate *priv = user_data;

    g_clear_object (&priv->item);
    g_reminder_keywords_widget_reset (priv->keywords);
    gtk_text_buffer_set_text (priv->text, "", -1);
}

ON_ACTION_PROTO (delete)
{
    GReminderWindowPrivate *priv = user_data;

    if (g_reminder_db_delete (priv->db, priv->item))
        on_new (actions, user_data);
    g_reminder_window_private_reset_completion (priv);
}

ON_ACTION_PROTO (save)
{
    GReminderWindowPrivate *priv = user_data;

    g_reminder_window_private_set_item (priv);
    g_reminder_db_save (priv->db, priv->item);
    g_reminder_window_private_reset_completion (priv);

    gtk_widget_grab_focus (GTK_WIDGET (priv->textview));
}

ON_ACTION_PROTO (edit)
{
    GReminderWindowPrivate *priv = user_data;
    
    G_REMINDER_CLEANUP_UNREF GReminderItem *old = g_object_ref (priv->item);
    g_reminder_window_private_set_item (priv);
    g_reminder_db_save (priv->db, priv->item);

    if (g_strcmp0 (g_reminder_item_get_contents (old), g_reminder_item_get_contents (priv->item)))
        g_reminder_db_delete_key (priv->db, g_reminder_item_get_checksum (old));

    const gchar *checksum = g_reminder_item_get_checksum (old);
    const GSList *_kk = g_reminder_item_get_keywords (priv->item);
    for (const GSList *k = g_reminder_item_get_keywords (old); k; k = g_slist_next (k))
    {
        gboolean found = FALSE;
        for (const GSList *kk = _kk; kk; kk = g_slist_next (kk))
        {
            if (!g_strcmp0 (k->data, kk->data))
                found = TRUE;
        }
        if (!found)
        {
            g_reminder_db_delete_with_suffix (priv->db, k->data, checksum);
            g_reminder_db_delete_with_suffix (priv->db, checksum, k->data);
        }
    }
    g_reminder_window_private_reset_completion (priv);

    gtk_widget_grab_focus (GTK_WIDGET (priv->textview));
}

ON_ACTION_PROTO (cancel)
{
    GReminderWindowPrivate *priv = user_data;

    gtk_widget_grab_focus (GTK_WIDGET (priv->textview));
}

G_REMINDER_VISIBLE void
g_reminder_window_edit (GReminderWindow *self,
                        GReminderItem   *item)
{
    GReminderWindowPrivate *priv = g_reminder_window_get_instance_private (self);

    g_clear_object (&priv->item);
    priv->item = g_object_ref (item);

    g_reminder_actions_set_state (priv->actions, G_REMINDER_STATE_EDITABLE);
    g_reminder_keywords_widget_reset_with_data (priv->keywords, g_reminder_item_get_keywords (item));
    gtk_text_buffer_set_text (priv->text, g_reminder_item_get_contents (item), -1);

    gtk_widget_grab_focus (GTK_WIDGET (priv->textview));
}

static void
g_reminder_window_search (GReminderWindow *self,
                          const gchar     *text)
{
    GReminderWindowPrivate *priv = g_reminder_window_get_instance_private (self);
    GSList *items = g_reminder_db_find (priv->db, text);

    if (!items)
        return;

    gtk_widget_show_all (g_reminder_list_window_new (self, text, items));
    g_slist_free_full (items, g_object_unref);
}

static void
on_search (GtkEntry *entry,
           gpointer  user_data)
{
    GReminderWindow *self = user_data;
    g_reminder_window_search (self, gtk_entry_get_text (entry));
}

static gboolean
reset_search (GtkWidget *widget,
              GdkEvent  *event    G_GNUC_UNUSED,
              gpointer  user_data G_GNUC_UNUSED)
{
    gtk_entry_buffer_set_text (gtk_entry_get_buffer (GTK_ENTRY (widget)), "", 0);
    return FALSE;
}

static gboolean
on_match_selected (GtkEntryCompletion *completion G_GNUC_UNUSED,
                   GtkTreeModel       *model,
                   GtkTreeIter        *iter,
                   gpointer            user_data)
{
    GReminderWindow *self = user_data;
    gchar *text = NULL;
    gtk_tree_model_get (model, iter, 0, &text, -1);
    g_reminder_window_search (self, text);
    g_free (text);
    return TRUE;
}

static void
g_reminder_window_private_update_actions_state (GReminderWindowPrivate *priv)
{
    gboolean valid = priv->kvalid && priv->cvalid;

    if (valid != priv->valid)
    {
        priv->valid = valid;
        g_reminder_actions_set_state (priv->actions, (!valid) ? G_REMINDER_STATE_BLANK : (priv->item) ? G_REMINDER_STATE_EDITABLE : G_REMINDER_STATE_VALID);;
    }
}

static void
on_valid_changed (GReminderKeywordsWidget *keywords G_GNUC_UNUSED,
                  gboolean                 valid,
                  gpointer                 user_data)
{
    GReminderWindowPrivate *priv = user_data;

    if (priv->kvalid != valid)
    {
        priv->kvalid = valid;
        g_reminder_window_private_update_actions_state (priv);
    }
}

static void
on_contents_changed (GtkTextBuffer *textbuffer,
                     gpointer       user_data)
{
    GReminderWindowPrivate *priv = user_data;
    
    const gchar *text;
    g_object_get (G_OBJECT (textbuffer), "text", &text, NULL);

    gboolean valid = g_regex_match (priv->no_blank_regex, text, 0, NULL);
    if (valid != priv->cvalid)
    {
        priv->cvalid = valid;
        g_reminder_window_private_update_actions_state (priv);
    }
}

static struct
{
    const gchar *name;
    GCallback    callback;
} actions[_G_REMINDER_ACTION_LAST] = {
    [G_REMINDER_ACTION_NEW]    = { "new",    G_CALLBACK (on_new)    },
    [G_REMINDER_ACTION_DELETE] = { "delete", G_CALLBACK (on_delete) },
    [G_REMINDER_ACTION_EDIT]   = { "edit",   G_CALLBACK (on_edit)   },
    [G_REMINDER_ACTION_SAVE]   = { "save",   G_CALLBACK (on_save)   },
    [G_REMINDER_ACTION_CANCEL] = { "cancel", G_CALLBACK (on_cancel) }
};

static void
g_reminder_window_dispose (GObject *object)
{
    GReminderWindowPrivate *priv = g_reminder_window_get_instance_private (G_REMINDER_WINDOW (object));

    if (priv->db)
    {
        for (GReminderAction a = G_REMINDER_ACTION_FIRST; a != _G_REMINDER_ACTION_LAST; ++a)
            g_signal_handler_disconnect (priv->actions, priv->c_signals[a]);
        g_signal_handler_disconnect (priv->search,     priv->c_signals[C_ACTIVATE]);
        g_signal_handler_disconnect (priv->search,     priv->c_signals[C_FOCUS]);
        g_signal_handler_disconnect (priv->search,     priv->c_signals[C_PRESS]);
        g_signal_handler_disconnect (priv->completion, priv->c_signals[C_MATCH]);
        g_signal_handler_disconnect (priv->keywords,   priv->c_signals[C_VALID_CHANGED]);
        g_signal_handler_disconnect (priv->text,       priv->c_signals[C_CHANGED]);
    }

    g_clear_object (&priv->db);
    g_clear_object (&priv->item);

    if (priv->no_blank_regex)
    {
        g_regex_unref (priv->no_blank_regex);
        priv->no_blank_regex = NULL;
    }

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
    priv->valid = FALSE;
    priv->kvalid = FALSE;
    priv->cvalid = FALSE;

    priv->no_blank_regex = g_regex_new ("[^ \t\r\n]", G_REGEX_MULTILINE|G_REGEX_OPTIMIZE , 0, NULL);

    GtkWidget *bar = gtk_header_bar_new ();
    GtkHeaderBar *header_bar = GTK_HEADER_BAR (bar);
    gtk_header_bar_set_title (header_bar, PACKAGE_NAME);
    gtk_header_bar_set_show_close_button (header_bar, TRUE);
    gtk_window_set_titlebar (GTK_WINDOW (self), bar);

    GtkWidget *sentry = gtk_search_entry_new ();
    priv->search = GTK_SEARCH_ENTRY (sentry);
    gtk_entry_set_width_chars (GTK_ENTRY (sentry), 40);
    priv->c_signals[C_ACTIVATE] = g_signal_connect (G_OBJECT (sentry),
                                                    "activate",
                                                    G_CALLBACK (on_search),
                                                    self);
    priv->c_signals[C_FOCUS] = g_signal_connect (G_OBJECT (sentry),
                                                 "focus-in-event",
                                                 G_CALLBACK (reset_search),
                                                 NULL);
    priv->c_signals[C_PRESS] = g_signal_connect (G_OBJECT (sentry),
                                                 "button-release-event",
                                                 G_CALLBACK (reset_search),
                                                 NULL);
    gtk_header_bar_pack_start (header_bar, sentry);

    priv->completion = gtk_entry_completion_new ();
    gtk_entry_completion_set_text_column (priv->completion, 0);
    gtk_entry_completion_set_minimum_key_length (priv->completion, 0);
    gtk_entry_set_completion (GTK_ENTRY (priv->search), priv->completion);
    priv->c_signals[C_MATCH] = g_signal_connect (G_OBJECT (priv->completion),
                                                 "match-selected",
                                                 G_CALLBACK (on_match_selected),
                                                 self);

    GtkWidget *as = g_reminder_actions_new ();
    priv->actions = G_REMINDER_ACTIONS (as);
    for (GReminderAction a = G_REMINDER_ACTION_FIRST; a != _G_REMINDER_ACTION_LAST; ++a)
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

    GtkWidget *align = gtk_alignment_new (0, 0, 0, 0);
    gtk_container_add (GTK_CONTAINER (align), gtk_label_new ("Keywords:"));
    gtk_grid_attach (g, align, 0, 0, 1, 1);

    GtkWidget *keywords = g_reminder_keywords_widget_new ();
    priv->keywords = G_REMINDER_KEYWORDS_WIDGET (keywords);
    priv->c_signals[C_VALID_CHANGED] = g_signal_connect (G_OBJECT (keywords),
                                                         "valid-changed",
                                                         G_CALLBACK (on_valid_changed),
                                                         priv);
    gtk_grid_attach_next_to (g, keywords, align, GTK_POS_RIGHT, 2, 1);

    align = gtk_alignment_new (0, 0, 0, 0);
    gtk_container_add (GTK_CONTAINER (align), gtk_label_new ("Contents:"));
    gtk_grid_attach (g, align, 0, 1, 1, 1);

    GtkWidget *text = gtk_text_view_new ();
    priv->textview = text;
    GtkTextView *tv = GTK_TEXT_VIEW (text);
    priv->text = gtk_text_view_get_buffer (tv);
    priv->c_signals[C_CHANGED] = g_signal_connect (G_OBJECT (priv->text),
                                                   "changed",
                                                   G_CALLBACK (on_contents_changed),
                                                   priv);
    gtk_text_view_set_wrap_mode (tv, GTK_WRAP_WORD);
    GtkWidget *scroll = gtk_scrolled_window_new (NULL, NULL);
    GtkScrolledWindow *s = GTK_SCROLLED_WINDOW (scroll);
    gtk_scrolled_window_set_min_content_height (s, 500);
    gtk_scrolled_window_set_min_content_width (s, 800);
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
                                      "border-width",    12,
                                      "resizable",       FALSE,
                                      NULL);
    GReminderWindowPrivate *priv = g_reminder_window_get_instance_private (G_REMINDER_WINDOW (self));

    priv->db = g_object_ref (db);

    g_reminder_window_private_reset_completion (priv);

    return self;
}
