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

#include "greminder-db-private.h"

#include <string.h>

#include <leveldb/c.h>

struct _GReminderDbPrivate
{
    leveldb_t              *db;
    leveldb_options_t      *options;
    leveldb_readoptions_t  *roptions;
    leveldb_writeoptions_t *woptions;
};

G_DEFINE_TYPE_WITH_PRIVATE (GReminderDb, g_reminder_db, G_TYPE_OBJECT)

static gboolean
g_reminder_db_private_put (GReminderDbPrivate *priv,
                           const gchar        *key,
                           size_t              klen,
                           const gchar        *value)
{
    G_REMINDER_CLEANUP_FREE gchar *err = NULL;
    leveldb_put (priv->db, priv->woptions, key, klen, value, strlen (value), &err);
    return !err;
}

static gboolean
g_reminder_db_private_put_suffix (GReminderDbPrivate *priv,
                                  const gchar        *key,
                                  const gchar        *value)
{
    G_REMINDER_CLEANUP_FREE gchar *_key = g_strdup_printf ("%s%c%s", key, '\0', value);
    return g_reminder_db_private_put (priv, _key, strlen (key) + strlen (value) + 1, value);
}

static gchar *sdup (const gchar *in, size_t *s)
{
    gchar *out = g_new0 (char, *s + 1);
    memcpy (out, in, *s);
    return out;
}

G_REMINDER_VISIBLE void
g_reminder_db_save (GReminderDb   *self,
                    GReminderItem *item)
{
    g_return_if_fail (G_REMINDER_IS_DB (self));
    g_return_if_fail (G_REMINDER_IS_ITEM (item));

    GReminderDbPrivate *priv = g_reminder_db_get_instance_private (self);
    
    const gchar *contents = g_reminder_item_get_contents (item);
    const gchar *checksum = g_reminder_item_get_checksum (item);

    if (!g_reminder_db_private_put (priv, checksum, strlen (checksum), contents))
        return; /* TODO: GError */

    for (const GSList *k = g_reminder_item_get_keywords (item); k; k = g_slist_next (k))
    {
        if (!g_reminder_db_private_put_suffix (priv, k->data, checksum) ||
            !g_reminder_db_private_put_suffix (priv, checksum, k->data))
                return; /* TODO: GError */
    }
}

static GSList *
g_reminder_db_private_find (GReminderDbPrivate *priv,
                            const gchar        *keyword)
{
    GSList *items = NULL;
    leveldb_iterator_t *it = leveldb_create_iterator (priv->db, priv->roptions);
    size_t len;
    size_t klen = strlen (keyword); 

    leveldb_iter_seek (it, keyword, strlen (keyword));
    while (leveldb_iter_valid (it))
    {
        /* FIXME: foo must not match foobar */
        const gchar *key = leveldb_iter_key (it, &len);
        if (memcmp (keyword, key, klen))
            break;
        items = g_slist_prepend (items, sdup (leveldb_iter_value (it, &len), &len));
        leveldb_iter_next (it);
    }

    leveldb_iter_destroy (it); /* TODO: cleanup */
    return items;
}

static GReminderItem *
g_reminder_db_private_get_item (GReminderDbPrivate *priv,
                                const gchar        *hash)
{
    size_t len;
    size_t hlen = strlen (hash);
    G_REMINDER_CLEANUP_FREE gchar *err = NULL;
    G_REMINDER_CLEANUP_FREE gchar *contents = sdup (leveldb_get (priv->db, priv->roptions, hash, strlen (hash), &len, &err), &len);
    if (err) /* TODO: handle */
        return NULL;

    GSList *keywords = NULL;
    leveldb_iterator_t *it = leveldb_create_iterator (priv->db, priv->roptions);

    leveldb_iter_seek (it, hash, strlen (hash));
    while (leveldb_iter_valid (it))
    {
        /* FIXME: foo must not match foobar */
        const gchar *key = leveldb_iter_key (it, &len);
        if (len != hlen)
        {
            if (memcmp (hash, key, hlen))
                break;
            keywords = g_slist_prepend (keywords, sdup (leveldb_iter_value (it, &len), &len));
        }
        leveldb_iter_next (it);
    }

    leveldb_iter_destroy (it); /* TODO: cleanup */

    GReminderItem *item = g_reminder_item_new (keywords, contents);
    g_slist_free_full (keywords, g_free); /* TODO: cleanup */
    return item;
}

static gboolean
g_reminder_db_private_delete (GReminderDbPrivate *priv,
                              const gchar        *key,
                              size_t              klen)
{
    G_REMINDER_CLEANUP_FREE gchar *err = NULL;
    leveldb_delete (priv->db, priv->woptions, key, klen, &err);
    return !err;
}

static gboolean
g_reminder_db_private_delete_suffix (GReminderDbPrivate *priv,
                                     const gchar        *key,
                                     const gchar        *suffix)
{
    G_REMINDER_CLEANUP_FREE gchar *_key = g_strdup_printf ("%s%c%s", key, '\0', suffix);
    return g_reminder_db_private_delete (priv, _key, strlen (key) + strlen (suffix) + 1);
}

G_REMINDER_VISIBLE void
g_reminder_db_delete (GReminderDb   *self,
                      GReminderItem *item)
{
    g_return_if_fail (G_REMINDER_IS_DB (self));

    GReminderDbPrivate *priv = g_reminder_db_get_instance_private (self);

    const gchar *checksum = g_reminder_item_get_checksum (item);

    if (!g_reminder_db_private_delete (priv, checksum, strlen (checksum)))
        return; /* TODO: GError */

    for (const GSList *k = g_reminder_item_get_keywords (item); k; k = g_slist_next (k))
    {
        if (!g_reminder_db_private_delete_suffix (priv, k->data, checksum) ||
            !g_reminder_db_private_delete_suffix (priv, checksum, k->data))
                return; /* TODO: GError */
    }
}

G_REMINDER_VISIBLE GSList *
g_reminder_db_find (GReminderDb *self,
                    const gchar *keywords)
{
    g_return_val_if_fail (G_REMINDER_IS_DB (self), NULL);

    GReminderDbPrivate *priv = g_reminder_db_get_instance_private (self);

    GSList *hashs = NULL, *items = NULL;
    gchar **ks = g_strsplit (keywords, " ", -1);

    for (gchar **k = ks; *k; ++k)
    {
        GSList *hs = g_reminder_db_private_find (priv, *k);
        if (!hashs)
        {
            hashs = hs;
            continue;
        }

        GSList *_hashs = NULL;
        for (; hs; hs = g_slist_next (hs))
        {
            for (GSList *__hashs = hashs; __hashs; __hashs = g_slist_next (__hashs))
            {
                if (!g_strcmp0 (__hashs->data, hs->data))
                    _hashs = g_slist_prepend (_hashs, hs->data);
            }
        }
        g_slist_free_full (hashs, g_free);
        hashs = _hashs;
    }

    for (GSList *_hashs = hashs; _hashs; _hashs = g_slist_next (_hashs))
    {
        GReminderItem *item = g_reminder_db_private_get_item (priv, _hashs->data);
        if (!item)
            continue;
        items = g_slist_prepend (items, item);
    }

    g_slist_free_full (hashs, g_free);
    g_strfreev (ks); /* TODO: cleanup */
    return items;
}

static void
g_reminder_db_finalize (GObject *object)
{
    GReminderDbPrivate *priv = g_reminder_db_get_instance_private (G_REMINDER_DB (object));

    leveldb_options_destroy (priv->options);
    leveldb_readoptions_destroy (priv->roptions);
    leveldb_writeoptions_destroy (priv->woptions);

    if (priv->db)
        leveldb_close (priv->db);

    G_OBJECT_CLASS (g_reminder_db_parent_class)->finalize (object);
}

static void
g_reminder_db_class_init (GReminderDbClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = g_reminder_db_finalize;
}

static void
g_reminder_db_init (GReminderDb *self)
{
    GReminderDbPrivate *priv = g_reminder_db_get_instance_private ((GReminderDb *) self);

    priv->options = leveldb_options_create ();
    leveldb_options_set_create_if_missing (priv->options, TRUE);

    priv->roptions = leveldb_readoptions_create ();
    priv->woptions = leveldb_writeoptions_create ();
    leveldb_writeoptions_set_sync (priv->woptions, TRUE);

    G_REMINDER_CLEANUP_FREE gchar *err = NULL;
    priv->db = leveldb_open (priv->options, "greminder.db", &err);
    if (err)
        priv->db = NULL;
}

G_REMINDER_VISIBLE GReminderDb *
g_reminder_db_new (void)
{
    GReminderDb *self = G_REMINDER_DB (g_object_new (G_REMINDER_TYPE_DB, NULL));
    GReminderDbPrivate *priv = g_reminder_db_get_instance_private (self);

    if (priv->db)
        return self;

    g_object_unref (self);
    return NULL; /* TODO: GError */
}
