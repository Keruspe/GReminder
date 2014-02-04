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

#include "greminder-row-private.h"

struct _GReminderRowPrivate
{
    GReminderItem *item;
};

G_DEFINE_TYPE_WITH_PRIVATE (GReminderRow, g_reminder_row, GTK_TYPE_LIST_BOX_ROW)

G_REMINDER_VISIBLE GReminderItem *
g_reminder_row_get_item (const GReminderRow *self)
{
    g_return_val_if_fail (G_REMINDER_IS_ROW (self), NULL);

    GReminderRowPrivate *priv = g_reminder_row_get_instance_private ((GReminderRow *) self);

    return priv->item;
}

static void
g_reminder_row_dispose (GObject *object)
{
    GReminderRowPrivate *priv = g_reminder_row_get_instance_private (G_REMINDER_ROW (object));

    g_clear_object (&priv->item);

    G_OBJECT_CLASS (g_reminder_row_parent_class)->dispose (object);
}

static void
g_reminder_row_class_init (GReminderRowClass *klass)
{
    G_OBJECT_CLASS (klass)->dispose = g_reminder_row_dispose;
}

static void
g_reminder_row_init (GReminderRow *self G_GNUC_UNUSED)
{
}

G_REMINDER_VISIBLE GtkWidget *
g_reminder_row_new (GReminderItem *item)
{
    g_return_val_if_fail (G_REMINDER_IS_ITEM (item), NULL);

    GtkWidget *self = gtk_widget_new (G_REMINDER_TYPE_ROW, NULL);
    GReminderRowPrivate *priv = g_reminder_row_get_instance_private (G_REMINDER_ROW (self));

    priv->item = g_object_ref (item);
    gtk_container_add (GTK_CONTAINER (self), gtk_label_new (g_reminder_item_get_contents (item)));

    return self;
}
