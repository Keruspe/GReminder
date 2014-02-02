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

#ifndef __G_REMINDER_ITEM_H__
#define __G_REMINDER_ITEM_H__

#include "greminder-macros.h"

G_BEGIN_DECLS

#define G_REMINDER_TYPE_ITEM            (g_reminder_item_get_type ())
#define G_REMINDER_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_REMINDER_TYPE_ITEM, GReminderItem))
#define G_REMINDER_IS_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_REMINDER_TYPE_ITEM))
#define G_REMINDER_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_REMINDER_TYPE_ITEM, GReminderItemClass))
#define G_REMINDER_IS_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_REMINDER_TYPE_ITEM))
#define G_REMINDER_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_REMINDER_TYPE_ITEM, GReminderItemClass))

typedef struct _GReminderItem GReminderItem;
typedef struct _GReminderItemClass GReminderItemClass;

G_REMINDER_VISIBLE
GType g_reminder_item_get_type (void);

const GSList *g_reminder_item_get_keywords (const GReminderItem *self);
const gchar  *g_reminder_item_get_contents (const GReminderItem *self);
const gchar  *g_reminder_item_get_checksum (const GReminderItem *self);

void g_reminder_item_add_keyword  (GReminderItem *self,
                                   const gchar   *keyword);
void g_reminder_item_del_keyword  (GReminderItem *self,
                                   const gchar   *keyword);
void g_reminder_item_set_contents (GReminderItem *self,
                                   const gchar   *contents);

GReminderItem *g_reminder_item_new (const GSList *keywords,
                                    const gchar  *contents);

G_END_DECLS

#endif /*__G_REMINDER_ITEM_H__*/
