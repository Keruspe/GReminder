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

#include "greminder-keywords-widget-private.h"

#include "greminder-keyword-widget.h"

struct _GReminderKeywordsWidgetPrivate
{
    GSList                 *keywords;

    gboolean                valid;

    GReminderKeywordWidget *last;
    gulong                  valid_id;
};

G_DEFINE_TYPE_WITH_PRIVATE (GReminderKeywordsWidget, g_reminder_keywords_widget, GTK_TYPE_BOX)

enum
{
    VALID_CHANGED,

    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

typedef struct
{
    GReminderKeywordsWidget *keywords;

    GReminderKeywordWidget  *keyword;
    gulong                   signal;
} _Keyword;

G_REMINDER_VISIBLE gboolean
g_reminder_keywords_widget_is_valid (const GReminderKeywordsWidget *self)
{
    g_return_val_if_fail (G_REMINDER_IS_KEYWORDS_WIDGET (self), FALSE);

    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private ((GReminderKeywordsWidget *) self);

    if (!priv->valid)
        return FALSE;

    if (g_slist_next (priv->keywords))
        return TRUE;

    _Keyword *k = priv->keywords->data;
    return !!g_strcmp0 ("", g_reminder_keyword_widget_get_keyword (k->keyword));
}

G_REMINDER_VISIBLE const GSList *
g_reminder_keywords_widget_get_keywords (const GReminderKeywordsWidget *self)
{
    g_return_val_if_fail (G_REMINDER_IS_KEYWORDS_WIDGET (self), NULL);

    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private ((GReminderKeywordsWidget *) self);

    GSList *ks = NULL;

    for (GSList *k = priv->keywords; k; k = g_slist_next (k))
    {
        _Keyword *_k = k->data;
        const gchar *keyword = g_reminder_keyword_widget_get_keyword (_k->keyword);
        if (keyword[0])
            ks = g_slist_append (ks, (gpointer) keyword);
    }

    return ks;
}

static void on_button_pressed (GReminderKeywordWidget *keyword,
                               gboolean                active,
                               gpointer                user_data);

static _Keyword *
_keyword_new (GReminderKeywordsWidget *ks)
{
    _Keyword *k = g_new0 (_Keyword, 1);
    k->keywords = ks;
    GtkWidget *keyword = g_reminder_keyword_widget_new ();
    k->keyword = G_REMINDER_KEYWORD_WIDGET (keyword);
    k->signal = g_signal_connect (G_OBJECT (keyword),
                                  "button-pressed",
                                  G_CALLBACK (on_button_pressed),
                                  k);
    gtk_container_add (GTK_CONTAINER (ks), g_object_ref (keyword));
    gtk_widget_show_all (keyword);
    return k;
}

static void
_keyword_free (gpointer data)
{
    _Keyword *k = data;
    g_signal_handler_disconnect (k->keyword, k->signal);
    gtk_container_remove (GTK_CONTAINER (k->keywords), GTK_WIDGET (k->keyword));
    g_object_unref (k->keyword);
    g_free (k);
}

static void
g_reminder_keywords_widget_private_remove_keyword (GReminderKeywordsWidgetPrivate *priv,
                                                   _Keyword                       *k)
{
    priv->keywords = g_slist_remove (priv->keywords, k);
    _keyword_free (k);
}

static void
g_reminder_keywords_widget_private_untrack_last (GReminderKeywordsWidgetPrivate *priv)
{
    if (priv->last)
    {
        g_signal_handler_disconnect (priv->last, priv->valid_id);
        g_clear_object (&priv->last);
    }
}

static void
g_reminder_keywords_widget_set_valid (GReminderKeywordsWidget *self,
                                      gboolean                 valid)
{
    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private (self);

    if (priv->valid != valid)
    {
        priv->valid = valid;

        g_signal_emit (self,
                       signals[VALID_CHANGED],
                       0, /* detail */
                       valid,
                       NULL);
    }
}

static void
on_valid_changed (GReminderKeywordWidget *keyword G_GNUC_UNUSED,
                  gboolean                valid,
                  gpointer                user_data)
{
    GReminderKeywordsWidget *self = user_data;

    g_reminder_keywords_widget_set_valid (self, valid);
}

static GReminderKeywordWidget *
g_reminder_keywords_widget_add_keyword (GReminderKeywordsWidget *self)
{
    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private (self);

    g_reminder_keywords_widget_private_untrack_last (priv);

    _Keyword *k = _keyword_new (self);
    priv->keywords = g_slist_append (priv->keywords, k);

    priv->last = g_object_ref (k->keyword);
    priv->valid_id = g_signal_connect (priv->last,
                                       "valid-changed",
                                       G_CALLBACK (on_valid_changed),
                                       self);

    return k->keyword;
}

static void
g_reminder_keywords_widget_empty_list (GReminderKeywordsWidget *self)
{
    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private (self);

    if (priv->keywords)
    {
        g_slist_free_full (priv->keywords, _keyword_free);
        priv->keywords = NULL;
    }
    g_reminder_keywords_widget_set_valid (self, FALSE);
}

G_REMINDER_VISIBLE void
g_reminder_keywords_widget_reset (GReminderKeywordsWidget *self)
{
    g_return_if_fail (G_REMINDER_IS_KEYWORDS_WIDGET (self));

    g_reminder_keywords_widget_empty_list (self);
    gtk_widget_grab_focus (GTK_WIDGET (g_reminder_keywords_widget_add_keyword (self)));
}

G_REMINDER_VISIBLE void
g_reminder_keywords_widget_reset_with_data (GReminderKeywordsWidget *self,
                                            const GSList            *keywords)
{
    g_return_if_fail (G_REMINDER_IS_KEYWORDS_WIDGET (self));
    g_return_if_fail (keywords);

    g_reminder_keywords_widget_empty_list (self);

    for (; keywords; keywords = g_slist_next (keywords))
    {
        GReminderKeywordWidget *k = g_reminder_keywords_widget_add_keyword (self);
        g_reminder_keyword_widget_set_keyword (k, keywords->data);
        g_reminder_keyword_widget_toggle_active (k);
        g_reminder_keywords_widget_set_valid (self, TRUE);
    }

    g_reminder_keywords_widget_add_keyword (self);
}

static void
on_button_pressed (GReminderKeywordWidget *keyword G_GNUC_UNUSED,
                   gboolean                active,
                   gpointer                user_data)
{
    _Keyword *k = user_data;

    GReminderKeywordsWidget *self = k->keywords;
    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private (self);

    if (active)
        g_reminder_keywords_widget_add_keyword (self);
    else
        g_reminder_keywords_widget_private_remove_keyword (priv, k);
}

static void
g_reminder_keywords_widget_dispose (GObject *object)
{
    GReminderKeywordsWidget *self = G_REMINDER_KEYWORDS_WIDGET (object);
    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private (self);

    g_reminder_keywords_widget_empty_list (self);
    g_reminder_keywords_widget_private_untrack_last (priv);

    G_OBJECT_CLASS (g_reminder_keywords_widget_parent_class)->dispose (object);
}

static void
g_reminder_keywords_widget_class_init (GReminderKeywordsWidgetClass *klass)
{
    G_OBJECT_CLASS (klass)->dispose = g_reminder_keywords_widget_dispose;

    signals[VALID_CHANGED] = g_signal_new ("valid-changed",
                                           G_REMINDER_TYPE_KEYWORDS_WIDGET,
                                           G_SIGNAL_RUN_LAST,
                                           0, /* class offset */
                                           NULL, /* accumulator */
                                           NULL, /* accumulator data */
                                           g_cclosure_marshal_VOID__OBJECT,
                                           G_TYPE_NONE,
                                           1, /* number of params */
                                           G_TYPE_BOOLEAN);
}

static void
g_reminder_keywords_widget_init (GReminderKeywordsWidget *self)
{
    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private ((GReminderKeywordsWidget *) self);

    priv->keywords = NULL;
    priv->valid = FALSE;
    priv->last = NULL;

    g_reminder_keywords_widget_add_keyword (self);
}

G_REMINDER_VISIBLE GtkWidget *
g_reminder_keywords_widget_new (void)
{
    return gtk_widget_new (G_REMINDER_TYPE_KEYWORDS_WIDGET,
                           "orientation", GTK_ORIENTATION_VERTICAL,
                           "spacing",     10,
                           NULL);
}
