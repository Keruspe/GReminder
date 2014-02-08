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

#ifndef __G_REMINDER_WINDOW_H__
#define __G_REMINDER_WINDOW_H__

#include "greminder-db.h"

G_BEGIN_DECLS

#define G_REMINDER_TYPE_WINDOW            (g_reminder_window_get_type ())
#define G_REMINDER_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_REMINDER_TYPE_WINDOW, GReminderWindow))
#define G_REMINDER_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_REMINDER_TYPE_WINDOW))
#define G_REMINDER_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_REMINDER_TYPE_WINDOW, GReminderWindowClass))
#define G_REMINDER_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_REMINDER_TYPE_WINDOW))
#define G_REMINDER_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_REMINDER_TYPE_WINDOW, GReminderWindowClass))

typedef struct _GReminderWindow GReminderWindow;
typedef struct _GReminderWindowClass GReminderWindowClass;

G_REMINDER_VISIBLE
GType g_reminder_window_get_type (void);

void g_reminder_window_edit (GReminderWindow *self,
                             GReminderItem   *item);

GtkWidget *g_reminder_window_new (GtkApplication *app,
                                  GReminderDb    *db);

G_END_DECLS

#endif /*__G_REMINDER_WINDOW_H__*/
