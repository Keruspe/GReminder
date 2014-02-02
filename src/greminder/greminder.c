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

#include "greminder-window.h"

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <stdlib.h>

static void
show_help (const gchar *caller)
{
    printf (_("Usage:\n"));
    /* Translators: help for gremidner version */
    printf ("  %s version: %s\n", caller, _("display the version"));
    /* Translators: help for greminder help */
    printf ("  %s help:    %s\n", caller, _("display this help"));
}

static void
show_version (void)
{
    printf ("%s\n", PACKAGE_STRING);
}

static gboolean
is_help (const gchar *option)
{
    return (!g_strcmp0 (option, "help") ||
            !g_strcmp0 (option, "-h") ||
            !g_strcmp0 (option, "--help"));
}

static gboolean
is_version (const gchar *option)
{
    return (!g_strcmp0 (option, "v") ||
            !g_strcmp0 (option, "version") ||
            !g_strcmp0 (option, "-v") ||
            !g_strcmp0 (option, "--version"));
}

gint
main (gint argc, gchar *argv[])
{
    G_REMINDER_INIT_GETTEXT ();

    if (argc > 1)
    {
        if (is_help (argv[1]))
        {
            show_help (argv[0]);
            return EXIT_SUCCESS;
        }
        else if (is_version (argv[1]))
        {
            show_version ();
            return EXIT_SUCCESS;
        }
    }

    gtk_init (&argc, &argv);
    g_object_set (gtk_settings_get_default (), "gtk-application-prefer-dark-theme", TRUE, NULL);

    GtkApplication *app = gtk_application_new ("org.gnome.GReminder", G_APPLICATION_FLAGS_NONE);
    GApplication *gapp = G_APPLICATION (app);
    G_REMINDER_CLEANUP_ERROR_FREE GError *error = NULL;

    G_APPLICATION_GET_CLASS (gapp)->activate = NULL;
    g_application_register (gapp, NULL, &error);

    if (error)
    {
        fprintf (stderr, "%s: %s\n", _("Failed to register the gtk application"), error->message);
        return EXIT_FAILURE;
    }

    G_REMINDER_CLEANUP_UNREF GReminderDb *db = g_reminder_db_new ();
    if (!db)
    {
        fprintf (stderr, "Failed to initialize database");
        return EXIT_FAILURE;
    }

    GtkWidget *win = g_reminder_window_new (app, db);
    gtk_widget_show_all (win);

    return g_application_run (gapp, argc, argv);
}
