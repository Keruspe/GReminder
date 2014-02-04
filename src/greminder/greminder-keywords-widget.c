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
    GSList *keywords;
};

G_DEFINE_TYPE_WITH_PRIVATE (GReminderKeywordsWidget, g_reminder_keywords_widget, GTK_TYPE_BOX)

typedef struct
{
    GReminderKeywordsWidget *keywords;

    GReminderKeywordWidget  *keyword;
    gulong                   signal;
} _Keyword;

G_REMINDER_VISIBLE GSList *
g_reminder_keywords_widget_get_keywords (GReminderKeywordsWidget *self)
{
    g_return_val_if_fail (G_REMINDER_IS_KEYWORDS_WIDGET (self), NULL);

    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private (self);

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

static GReminderKeywordWidget *
g_reminder_keywords_widget_add_keyword (GReminderKeywordsWidget *self)
{
    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private (self);

    _Keyword *k = _keyword_new (self);
    priv->keywords = g_slist_append (priv->keywords, k);
    return k->keyword;
}

static void
g_reminder_keywords_widget_private_empty_list (GReminderKeywordsWidgetPrivate *priv)
{
    if (priv->keywords)
    {
        g_slist_free_full (priv->keywords, _keyword_free);
        priv->keywords = NULL;
    }
}

G_REMINDER_VISIBLE void
g_reminder_keywords_widget_reset (GReminderKeywordsWidget *self)
{
    g_return_if_fail (G_REMINDER_IS_KEYWORDS_WIDGET (self));

    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private (self);

    g_reminder_keywords_widget_private_empty_list (priv);
    g_reminder_keywords_widget_add_keyword (self);
}

G_REMINDER_VISIBLE void
g_reminder_keywords_widget_reset_with_data (GReminderKeywordsWidget *self,
                                            const GSList            *keywords)
{
    g_return_if_fail (G_REMINDER_IS_KEYWORDS_WIDGET (self));
    g_return_if_fail (keywords);

    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private (self);

    g_reminder_keywords_widget_private_empty_list (priv);

    for (; keywords; keywords = g_slist_next (keywords))
    {
        GReminderKeywordWidget *k = g_reminder_keywords_widget_add_keyword (self);
        g_reminder_keyword_widget_set_keyword (k, keywords->data);
        g_reminder_keyword_widget_toggle_active (k);
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
    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private (G_REMINDER_KEYWORDS_WIDGET (object));

    if (priv->keywords)
        g_reminder_keywords_widget_private_empty_list (priv);

    G_OBJECT_CLASS (g_reminder_keywords_widget_parent_class)->dispose (object);
}

static void
g_reminder_keywords_widget_class_init (GReminderKeywordsWidgetClass *klass)
{
    G_OBJECT_CLASS (klass)->dispose = g_reminder_keywords_widget_dispose;
}

static void
g_reminder_keywords_widget_init (GReminderKeywordsWidget *self)
{
    GReminderKeywordsWidgetPrivate *priv = g_reminder_keywords_widget_get_instance_private ((GReminderKeywordsWidget *) self);

    priv->keywords = NULL;
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
