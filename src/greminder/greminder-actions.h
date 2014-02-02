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

#ifndef __G_REMINDER_ACTIONS_H__
#define __G_REMINDER_ACTIONS_H__

#include "greminder-macros.h"

G_BEGIN_DECLS

#define G_REMINDER_TYPE_ACTIONS            (g_reminder_actions_get_type ())
#define G_REMINDER_ACTIONS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_REMINDER_TYPE_ACTIONS, GReminderActions))
#define G_REMINDER_IS_ACTIONS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_REMINDER_TYPE_ACTIONS))
#define G_REMINDER_ACTIONS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_REMINDER_TYPE_ACTIONS, GReminderActionsClass))
#define G_REMINDER_IS_ACTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_REMINDER_TYPE_ACTIONS))
#define G_REMINDER_ACTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_REMINDER_TYPE_ACTIONS, GReminderActionsClass))

typedef struct _GReminderActions GReminderActions;
typedef struct _GReminderActionsClass GReminderActionsClass;

typedef enum
{
    G_REMINDER_ACTION_FIRST,

    G_REMINDER_ACTION_NEW = G_REMINDER_ACTION_FIRST,
    G_REMINDER_ACTION_DELETE,
    G_REMINDER_ACTION_EDIT,
    G_REMINDER_ACTION_SAVE,

    G_REMINDER_ACTION_LAST
} GReminderAction;

G_REMINDER_VISIBLE
GType g_reminder_actions_get_type (void);

void g_reminder_actions_set_editable (GReminderActions *self,
                                      gboolean          editable);

GtkWidget *g_reminder_actions_new (void);

G_END_DECLS

#endif /*__G_REMINDER_ACTIONS_H__*/
