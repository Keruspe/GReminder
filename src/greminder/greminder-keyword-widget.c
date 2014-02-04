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

#include "greminder-keyword-widget-private.h"

struct _GReminderKeywordWidgetPrivate
{
    GtkEntry  *entry;
    GtkButton *button;

    gboolean   active;

    gulong     button_pressed_id;
};

G_DEFINE_TYPE_WITH_PRIVATE (GReminderKeywordWidget, g_reminder_keyword_widget, GTK_TYPE_BOX)

enum
{
    BUTTON_PRESSED,

    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_REMINDER_VISIBLE const gchar *
g_reminder_keyword_widget_get_keyword (const GReminderKeywordWidget *self)
{
    g_return_val_if_fail (G_REMINDER_IS_KEYWORD_WIDGET (self), NULL);

    GReminderKeywordWidgetPrivate *priv = g_reminder_keyword_widget_get_instance_private ((GReminderKeywordWidget *) self);

    return gtk_entry_get_text (priv->entry);
}

G_REMINDER_VISIBLE void
g_reminder_keyword_widget_set_keyword (GReminderKeywordWidget *self,
                                       const gchar            *keyword)
{
    g_return_if_fail (G_REMINDER_IS_KEYWORD_WIDGET (self));

    GReminderKeywordWidgetPrivate *priv = g_reminder_keyword_widget_get_instance_private (self);

    gtk_entry_set_text (priv->entry, keyword);
}

static void
g_reminder_keyword_widget_private_toggle_active (GReminderKeywordWidgetPrivate *priv)
{
    priv->active = !priv->active;
    gtk_widget_set_sensitive (GTK_WIDGET (priv->entry), priv->active);
    gtk_editable_set_editable (GTK_EDITABLE (priv->entry), priv->active);
    gtk_button_set_label (priv->button, priv->active ? "+" : "-");
}

static void
on_button_pressed (GtkButton *button G_GNUC_UNUSED,
                   gpointer   user_data)
{
    GReminderKeywordWidget *self = user_data;
    GReminderKeywordWidgetPrivate *priv = g_reminder_keyword_widget_get_instance_private (self);

    if (!gtk_entry_get_text_length (priv->entry))
        return;

    g_reminder_keyword_widget_private_toggle_active (priv);

    g_signal_emit (self,
                   signals[BUTTON_PRESSED],
                   0, /* detail */
                   !priv->active,
                   NULL);
}

G_REMINDER_VISIBLE void
g_reminder_keyword_widget_toggle_active (GReminderKeywordWidget *self)
{
    g_return_if_fail (G_REMINDER_IS_KEYWORD_WIDGET (self));

    GReminderKeywordWidgetPrivate *priv = g_reminder_keyword_widget_get_instance_private (self);

    g_reminder_keyword_widget_private_toggle_active (priv);
}

static void
g_reminder_keyword_widget_dispose (GObject *object)
{
    GReminderKeywordWidgetPrivate *priv = g_reminder_keyword_widget_get_instance_private (G_REMINDER_KEYWORD_WIDGET (object));

    if (priv->button_pressed_id)
    {
        g_signal_handler_disconnect (priv->button, priv->button_pressed_id);
        priv->button_pressed_id = 0;
    }

    G_OBJECT_CLASS (g_reminder_keyword_widget_parent_class)->dispose (object);
}

static void
g_reminder_keyword_widget_class_init (GReminderKeywordWidgetClass *klass)
{
    G_OBJECT_CLASS (klass)->dispose = g_reminder_keyword_widget_dispose;

    signals[BUTTON_PRESSED] = g_signal_new ("button-pressed",
                                            G_REMINDER_TYPE_KEYWORD_WIDGET,
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
g_reminder_keyword_widget_init (GReminderKeywordWidget *self)
{
    static GdkRGBA white = { 255, 255, 255, 0.5 };

    GReminderKeywordWidgetPrivate *priv = g_reminder_keyword_widget_get_instance_private ((GReminderKeywordWidget *) self);

    GtkWidget *entry = gtk_entry_new ();
    priv->entry = GTK_ENTRY (entry);
    gtk_widget_override_color (entry, GTK_STATE_FLAG_INSENSITIVE, &white);
    GtkWidget *button = gtk_button_new_with_label ("+");
    priv->button = GTK_BUTTON (button);

    priv->active = TRUE;

    priv->button_pressed_id = g_signal_connect (G_OBJECT (button),
                                                "pressed",
                                                G_CALLBACK (on_button_pressed),
                                                self);

    GtkBox *box = GTK_BOX (self);
    gtk_box_pack_start (box, entry, TRUE, TRUE, 0);
    gtk_box_pack_start (box, button, FALSE, TRUE, 0);
}

G_REMINDER_VISIBLE GtkWidget *
g_reminder_keyword_widget_new (void)
{
    return gtk_widget_new (G_REMINDER_TYPE_KEYWORD_WIDGET,
                           "orientation", GTK_ORIENTATION_HORIZONTAL,
                           "spacing",     10,
                           NULL);
}
