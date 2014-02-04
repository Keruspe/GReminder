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

#ifndef __G_REMINDER_KEYWORD_WIDGET_H__
#define __G_REMINDER_KEYWORD_WIDGET_H__

#include "greminder-macros.h"

G_BEGIN_DECLS

#define G_REMINDER_TYPE_KEYWORD_WIDGET            (g_reminder_keyword_widget_get_type ())
#define G_REMINDER_KEYWORD_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_REMINDER_TYPE_KEYWORD_WIDGET, GReminderKeywordWidget))
#define G_REMINDER_IS_KEYWORD_WIDGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_REMINDER_TYPE_KEYWORD_WIDGET))
#define G_REMINDER_KEYWORD_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), G_REMINDER_TYPE_KEYWORD_WIDGET, GReminderKeywordWidgetClass))
#define G_REMINDER_IS_KEYWORD_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_REMINDER_TYPE_KEYWORD_WIDGET))
#define G_REMINDER_KEYWORD_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), G_REMINDER_TYPE_KEYWORD_WIDGET, GReminderKeywordWidgetClass))

typedef struct _GReminderKeywordWidget GReminderKeywordWidget;
typedef struct _GReminderKeywordWidgetClass GReminderKeywordWidgetClass;

G_REMINDER_VISIBLE
GType g_reminder_keyword_widget_get_type (void);

const gchar *g_reminder_keyword_widget_get_keyword (const GReminderKeywordWidget *self);
void g_reminder_keyword_widget_set_keyword (GReminderKeywordWidget *self,
                                            const gchar            *keyword);

void g_reminder_keyword_widget_toggle_active (GReminderKeywordWidget *self);

GtkWidget *g_reminder_keyword_widget_new (void);

G_END_DECLS

#endif /*__G_REMINDER_KEYWORD_WIDGET_H__*/
