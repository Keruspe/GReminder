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

#ifndef __GREMINDER_MACROS_H__
#define __GREMINDER_MACROS_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define G_REMINDER_INIT_GETTEXT()                       \
    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);        \
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8"); \
    textdomain (GETTEXT_PACKAGE)

#define G_REMINDER_VISIBLE      __attribute__((visibility("default")))
#define G_REMINDER_CLEANUP(fun) __attribute__((cleanup(fun)))

#define G_REMINDER_CLEANUP_FREE       G_REMINDER_CLEANUP (g_reminder_free_ptr)
#define G_REMINDER_CLEANUP_ERROR_FREE G_REMINDER_CLEANUP (g_reminder_error_free_ptr)

#define G_REMINDER_CLEANUP_UNREF G_REMINDER_CLEANUP (g_reminder_unref_ptr)

#define G_REMINDER_TRIVIAL_CLEANUP_FUN_FULL(name, type, fun, param_type) \
    static inline void                                                   \
    g_reminder_##name##_ptr (param_type ptr)                             \
    {                                                                    \
        g_clear_pointer ((type *) ptr, fun);                             \
    }

#define G_REMINDER_TRIVIAL_CLEANUP_FUN(name, type, fun) \
    G_REMINDER_TRIVIAL_CLEANUP_FUN_FULL (name, type, fun, type *)

G_REMINDER_TRIVIAL_CLEANUP_FUN_FULL (free,       gpointer,  g_free,         gpointer)

G_REMINDER_TRIVIAL_CLEANUP_FUN      (error_free, GError *,  g_error_free)

G_REMINDER_TRIVIAL_CLEANUP_FUN_FULL (unref,      GObject *, g_object_unref, gpointer)

G_END_DECLS

#endif /*__GREMINDER_MACROS_H__*/
