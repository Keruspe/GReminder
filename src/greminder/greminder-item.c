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

#include "greminder-item-private.h"

struct _GReminderItemPrivate
{
    GSList *keywords;
    gchar  *contents;
    gchar  *checksum;
};

G_DEFINE_TYPE_WITH_PRIVATE (GReminderItem, g_reminder_item, G_TYPE_OBJECT)

G_REMINDER_VISIBLE const GSList *
g_reminder_item_get_keywords (const GReminderItem *self)
{
    g_return_val_if_fail (G_REMINDER_IS_ITEM (self), NULL);

    GReminderItemPrivate *priv = g_reminder_item_get_instance_private ((GReminderItem *) self);

    return priv->keywords;
}

G_REMINDER_VISIBLE const gchar *
g_reminder_item_get_contents (const GReminderItem *self)
{
    g_return_val_if_fail (G_REMINDER_IS_ITEM (self), NULL);

    GReminderItemPrivate *priv = g_reminder_item_get_instance_private ((GReminderItem *) self);

    return priv->contents;
}

G_REMINDER_VISIBLE const gchar *
g_reminder_item_get_checksum (const GReminderItem *self)
{
    g_return_val_if_fail (G_REMINDER_IS_ITEM (self), NULL);

    GReminderItemPrivate *priv = g_reminder_item_get_instance_private ((GReminderItem *) self);

    return priv->checksum;
}

static void
g_reminder_item_private_add_keyword (GReminderItemPrivate *priv,
                                     const gchar          *keyword)
{
    priv->keywords = g_slist_append (priv->keywords, g_strdup (keyword));
}

G_REMINDER_VISIBLE void
g_reminder_item_add_keyword (GReminderItem *self,
                             const gchar   *keyword)
{
    g_return_if_fail (G_REMINDER_IS_ITEM (self));

    GReminderItemPrivate *priv = g_reminder_item_get_instance_private ((GReminderItem *) self);

    g_reminder_item_private_add_keyword (priv, keyword);
}

G_REMINDER_VISIBLE void
g_reminder_item_del_keyword (GReminderItem *self,
                             const gchar   *keyword)
{
    g_return_if_fail (G_REMINDER_IS_ITEM (self));

    GReminderItemPrivate *priv = g_reminder_item_get_instance_private ((GReminderItem *) self);

    for (GSList *k = priv->keywords; k; k = g_slist_next (k))
    {
        gchar *kk = k->data;

        if (!g_strcmp0 (kk, keyword))
        {
            g_free (kk);
            priv->keywords = g_slist_delete_link (priv->keywords, k);
            break;
        }
    }
}

static void
g_reminder_item_private_update_checksum (GReminderItemPrivate *priv)
{
    g_free (priv->checksum);
    priv->checksum = g_compute_checksum_for_string (G_CHECKSUM_SHA1, priv->contents, -1);
}

G_REMINDER_VISIBLE void
g_reminder_item_set_contents (GReminderItem *self,
                              const gchar   *contents)
{
    g_return_if_fail (G_REMINDER_IS_ITEM (self));

    GReminderItemPrivate *priv = g_reminder_item_get_instance_private ((GReminderItem *) self);

    g_free (priv->contents);
    priv->contents = g_strdup (contents);
    g_reminder_item_private_update_checksum (priv);
}

static void
g_reminder_item_finalize (GObject *object)
{
    GReminderItemPrivate *priv = g_reminder_item_get_instance_private (G_REMINDER_ITEM (object));

    g_slist_free_full (priv->keywords, g_free);
    g_free (priv->contents);
    g_free (priv->checksum);

    G_OBJECT_CLASS (g_reminder_item_parent_class)->finalize (object);
}

static void
g_reminder_item_class_init (GReminderItemClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = g_reminder_item_finalize;
}

static void
g_reminder_item_init (GReminderItem *self)
{
    GReminderItemPrivate *priv = g_reminder_item_get_instance_private ((GReminderItem *) self);
    
    priv->keywords = NULL;
    priv->checksum = NULL;
}

G_REMINDER_VISIBLE GReminderItem *
g_reminder_item_new (const GSList *keywords,
                     const gchar  *contents)
{
    g_return_val_if_fail (keywords, NULL);
    g_return_val_if_fail (contents, NULL);

    GReminderItem *self = G_REMINDER_ITEM (g_object_new (G_REMINDER_TYPE_ITEM, NULL));
    GReminderItemPrivate *priv = g_reminder_item_get_instance_private ((GReminderItem *) self);

    priv->contents = g_strdup (contents);
    g_reminder_item_private_update_checksum (priv);

    for (const GSList *k = keywords; k; k = g_slist_next (k))
        g_reminder_item_private_add_keyword (priv, k->data);

    return self;
}
