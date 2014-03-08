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

#include <leveldb/c.h>

#include <string.h>

#define G_REMINDER_CLEANUP_SLIST_FREE G_REMINDER_CLEANUP (g_reminder_slist_free_ptr)

#define G_REMINDER_CLEANUP_DB_ITER_DESTROY G_REMINDER_CLEANUP (g_reminder_db_iter_destroy)

static void
g_reminder_slist_free_ptr (GSList **l)
{
    g_slist_free_full (*l, g_free);
    *l = NULL;
}

static void
g_reminder_db_iter_destroy (leveldb_iterator_t **it)
{
    leveldb_iter_destroy (*it);
}

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

G_REMINDER_VISIBLE gboolean
g_reminder_db_save (const GReminderDb   *self,
                    const GReminderItem *item)
{
    g_return_val_if_fail (G_REMINDER_IS_DB (self), FALSE);
    g_return_val_if_fail (G_REMINDER_IS_ITEM (item), FALSE);

    GReminderDbPrivate *priv = g_reminder_db_get_instance_private ((GReminderDb *) self);
    
    const gchar *contents = g_reminder_item_get_contents (item);
    const gchar *checksum = g_reminder_item_get_checksum (item);

    if (!g_reminder_db_private_put (priv, checksum, strlen (checksum), contents))
        return FALSE;

    for (const GSList *k = g_reminder_item_get_keywords (item); k; k = g_slist_next (k))
    {
        if (!g_reminder_db_private_put_suffix (priv, k->data, checksum) ||
            !g_reminder_db_private_put_suffix (priv, checksum, k->data))
                return FALSE;
    }

    return TRUE;
}

static GSList *
g_reminder_db_private_find (GReminderDbPrivate *priv,
                            const gchar        *keyword)
{
    G_REMINDER_CLEANUP_DB_ITER_DESTROY leveldb_iterator_t *it = leveldb_create_iterator (priv->db, priv->roptions);
    GSList *items = NULL;
    size_t len;
    size_t klen = strlen (keyword); 

    leveldb_iter_seek (it, keyword, strlen (keyword));
    while (leveldb_iter_valid (it))
    {
        const gchar *key = leveldb_iter_key (it, &len);
        if (klen != len && key[klen])
            break;
        if (memcmp (keyword, key, klen))
            break;
        items = g_slist_prepend (items, sdup (leveldb_iter_value (it, &len), &len));
        leveldb_iter_next (it);
    }

    return items;
}

static GReminderItem *
g_reminder_db_private_get_item (GReminderDbPrivate *priv,
                                const gchar        *hash)
{
    G_REMINDER_CLEANUP_SLIST_FREE GSList *keywords = NULL;
    G_REMINDER_CLEANUP_DB_ITER_DESTROY leveldb_iterator_t *it = NULL;
    G_REMINDER_CLEANUP_FREE gchar *err = NULL;
    size_t hlen = strlen (hash);
    size_t len;
    G_REMINDER_CLEANUP_FREE gchar *contents = sdup (leveldb_get (priv->db, priv->roptions, hash, hlen, &len, &err), &len);
    if (err)
        return NULL;

    it = leveldb_create_iterator (priv->db, priv->roptions);

    leveldb_iter_seek (it, hash, strlen (hash));
    while (leveldb_iter_valid (it))
    {
        const gchar *key = leveldb_iter_key (it, &len);
        if (len != hlen && !key[hlen])
        {
            if (memcmp (hash, key, hlen))
                break;
            keywords = g_slist_prepend (keywords, sdup (leveldb_iter_value (it, &len), &len));
        }
        leveldb_iter_next (it);
    }

    return g_reminder_item_new (keywords, contents);
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

G_REMINDER_VISIBLE gboolean
g_reminder_db_delete (const GReminderDb   *self,
                      const GReminderItem *item)
{
    g_return_val_if_fail (G_REMINDER_IS_DB (self), FALSE);

    GReminderDbPrivate *priv = g_reminder_db_get_instance_private ((GReminderDb *) self);

    const gchar *checksum = g_reminder_item_get_checksum (item);

    if (!g_reminder_db_private_delete (priv, checksum, strlen (checksum)))
        return FALSE;

    for (const GSList *k = g_reminder_item_get_keywords (item); k; k = g_slist_next (k))
    {
        if (!g_reminder_db_private_delete_suffix (priv, k->data, checksum) ||
            !g_reminder_db_private_delete_suffix (priv, checksum, k->data))
                return FALSE;
    }

    return TRUE;
}

G_REMINDER_VISIBLE gboolean
g_reminder_db_delete_key (const GReminderDb *self,
                          const gchar       *key)
{
    g_return_val_if_fail (G_REMINDER_IS_DB (self), FALSE);
    g_return_val_if_fail (key, FALSE);

    GReminderDbPrivate *priv = g_reminder_db_get_instance_private ((GReminderDb *) self);

    return g_reminder_db_private_delete (priv, key, strlen (key));
}

G_REMINDER_VISIBLE gboolean
g_reminder_db_delete_with_suffix (const GReminderDb *self,
                                  const gchar       *key,
                                  const gchar       *suffix)
{
    g_return_val_if_fail (G_REMINDER_IS_DB (self), FALSE);
    g_return_val_if_fail (key, FALSE);
    g_return_val_if_fail (suffix, FALSE);

    GReminderDbPrivate *priv = g_reminder_db_get_instance_private ((GReminderDb *) self);

    return g_reminder_db_private_delete_suffix (priv, key, suffix);
}

G_REMINDER_VISIBLE GSList *
g_reminder_db_find (const GReminderDb *self,
                    const gchar       *keywords)
{
    g_return_val_if_fail (G_REMINDER_IS_DB (self), NULL);

    GReminderDbPrivate *priv = g_reminder_db_get_instance_private ((GReminderDb *) self);

    G_REMINDER_CLEANUP_STRFREEV gchar **ks = g_strsplit (keywords, " ", -1);
    G_REMINDER_CLEANUP_SLIST_FREE GSList *hashs = NULL;
    GSList *items = NULL;

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

    return items;
}

static GSList *
g_reminder_db_private_get_keywords (GReminderDbPrivate *priv)
{
    GSList *keywords = NULL;
    G_REMINDER_CLEANUP_DB_ITER_DESTROY leveldb_iterator_t *it = leveldb_create_iterator (priv->db, priv->roptions);
    G_REMINDER_CLEANUP_FREE gchar *last_hash = NULL;

    leveldb_iter_seek_to_first (it);

    while (leveldb_iter_valid (it))
    {
        size_t len;
        const gchar *key = leveldb_iter_key (it, &len);
        gboolean has_nul = FALSE;
        for (guint i = 0; i < len; ++i)
        {
            if (!key[i])
            {
                has_nul = TRUE;
                break;
            }
        }
        if (has_nul && !(last_hash && !g_strcmp0 (key, last_hash)))
        {
            gboolean found = FALSE;
            for (const GSList *k = keywords; k; k = g_slist_next (k))
            {
                if (!g_strcmp0 (k->data, key))
                {
                    found = TRUE;
                    break;
                }
            }
            if (!found)
                keywords = g_slist_insert_sorted (keywords, sdup (key, &len), (GCompareFunc) g_strcmp0);
        }
        else
        {
            g_free (last_hash);
            last_hash = sdup (key, &len);
        }
        leveldb_iter_next (it);
    }

    return keywords;
}

G_REMINDER_VISIBLE GtkListStore *
g_reminder_db_get_keywords (const GReminderDb *self)
{
    g_return_val_if_fail (G_REMINDER_IS_DB (self), NULL);

    GReminderDbPrivate *priv = g_reminder_db_get_instance_private ((GReminderDb *) self);
    GtkListStore *store = gtk_list_store_new (1, G_TYPE_STRING);
    GtkTreeIter iter;

    for (G_REMINDER_CLEANUP_SLIST_FREE GSList *k = g_reminder_db_private_get_keywords (priv); k; k = g_slist_next (k))
    {
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 0, k->data, -1);
    }

    return store;
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

static gchar *
g_reminder_db_get_dir_path (void)
{
    return g_build_filename (g_get_user_data_dir (), "greminder", NULL);
}

static GFile *
g_reminder_db_get_dir (void)
{
    G_REMINDER_CLEANUP_FREE gchar *db_dir_path = g_reminder_db_get_dir_path ();
    return g_file_new_for_path (db_dir_path);
}

static gchar *
g_reminder_db_get_full_path (void)
{
    G_REMINDER_CLEANUP_FREE gchar *db_dir_path = g_reminder_db_get_dir_path ();
    return g_build_filename (db_dir_path, "greminder.db", NULL);
}

static void
g_reminder_db_init (GReminderDb *self)
{
    GReminderDbPrivate *priv = g_reminder_db_get_instance_private ((GReminderDb *) self);

    G_REMINDER_CLEANUP_UNREF GFile *db_dir = g_reminder_db_get_dir ();

    if (!g_file_query_exists (db_dir, NULL))
    {
        G_REMINDER_CLEANUP_ERROR_FREE GError *error = NULL;
        g_file_make_directory_with_parents (db_dir, NULL, &error);
        if (error)
        {
            g_error ("Could not creade db directory: %s", error->message);
            priv->db = NULL;
            return;
        }
    }

    priv->options = leveldb_options_create ();
    leveldb_options_set_create_if_missing (priv->options, TRUE);

    priv->roptions = leveldb_readoptions_create ();
    priv->woptions = leveldb_writeoptions_create ();
    leveldb_writeoptions_set_sync (priv->woptions, TRUE);

    G_REMINDER_CLEANUP_FREE gchar *db_full_path = g_reminder_db_get_full_path ();
    G_REMINDER_CLEANUP_FREE gchar *err = NULL;
    priv->db = leveldb_open (priv->options, db_full_path, &err);
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
    return NULL;
}
