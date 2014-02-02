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

#ifndef __G_REMINDER_WINDOW_PRIVATE_H__
#define __G_REMINDER_WINDOW_PRIVATE_H__

#include "greminder-window.h"

G_BEGIN_DECLS

typedef struct _GReminderWindowPrivate GReminderWindowPrivate;

struct _GReminderWindow
{
    GtkApplicationWindow parent_instance;
};

struct _GReminderWindowClass
{
    GtkApplicationWindowClass parent_class;
};

G_END_DECLS

#endif /*__G_REMINDER_WINDOW_PRIVATE_H__*/
