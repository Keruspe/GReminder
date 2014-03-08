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

#include "greminder-actions-private.h"

struct _GReminderActionsPrivate
{
    GtkButton *actions[G_REMINDER_ACTION_LAST];

    gulong     c_signals[G_REMINDER_ACTION_LAST];
};

G_DEFINE_TYPE_WITH_PRIVATE (GReminderActions, g_reminder_actions, GTK_TYPE_BOX)

G_REMINDER_VISIBLE void
g_reminder_actions_set_state (GReminderActions *self,
                              GReminderState    state)
{
    g_return_if_fail (G_REMINDER_IS_ACTIONS (self));

    GReminderActionsPrivate *priv = g_reminder_actions_get_instance_private (self);

    gboolean active[G_REMINDER_ACTION_LAST] = { FALSE };

    switch (state)
    {
    case G_REMINDER_STATE_EDITABLE:
        active[G_REMINDER_ACTION_DELETE] = TRUE;
        active[G_REMINDER_ACTION_EDIT] = TRUE;
        /* fallthrough */
    case G_REMINDER_STATE_VALID:
        active[G_REMINDER_ACTION_SAVE] = TRUE;
        /* fallthrough */
    case G_REMINDER_STATE_BLANK:
        active[G_REMINDER_ACTION_NEW] = TRUE;
    }

    for (GReminderAction a = G_REMINDER_ACTION_FIRST; a != G_REMINDER_ACTION_LAST; ++a)
        gtk_widget_set_sensitive (GTK_WIDGET (priv->actions[a]), active[a]);
}

#define ON_BUTTON_PRESSED_PROTO(name)           \
    static void                                 \
    on_##name (GtkButton *button G_GNUC_UNUSED, \
               gpointer   user_data)

#define ON_BUTTON_PRESSED(action, name)                                                 \
    ON_BUTTON_PRESSED_PROTO (name)                                                      \
    {                                                                                   \
        GReminderActions *self = user_data;                                             \
        g_reminder_actions_set_state (self, actions[G_REMINDER_ACTION_##action].state); \
        g_signal_emit (self,                                                            \
                       actions[G_REMINDER_ACTION_##action].signal,                      \
                       0, /* detail */                                                  \
                       NULL);                                                           \
    }

ON_BUTTON_PRESSED_PROTO (new);
ON_BUTTON_PRESSED_PROTO (delete);
ON_BUTTON_PRESSED_PROTO (edit);
ON_BUTTON_PRESSED_PROTO (save);

static struct {
    guint          signal;
    const gchar   *name;
    const gchar   *label;
    GCallback      callback;
    GReminderState state;
} actions[G_REMINDER_ACTION_LAST] = {
    [G_REMINDER_ACTION_NEW]    = { 0, "new",    "New",    G_CALLBACK (on_new),    G_REMINDER_STATE_BLANK    },
    [G_REMINDER_ACTION_DELETE] = { 0, "delete", "Delete", G_CALLBACK (on_delete), G_REMINDER_STATE_BLANK    },
    [G_REMINDER_ACTION_EDIT]   = { 0, "edit",   "Edit",   G_CALLBACK (on_edit),   G_REMINDER_STATE_EDITABLE },
    [G_REMINDER_ACTION_SAVE]   = { 0, "save", "  Save",   G_CALLBACK (on_save),   G_REMINDER_STATE_EDITABLE }
};

ON_BUTTON_PRESSED (NEW,    new)
ON_BUTTON_PRESSED (DELETE, real_delete)
ON_BUTTON_PRESSED (EDIT,   edit)
ON_BUTTON_PRESSED (SAVE,   save)

ON_BUTTON_PRESSED_PROTO (delete)
{
    GtkWidget *dialog = gtk_message_dialog_new (NULL,
                                                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_USE_HEADER_BAR,
                                                GTK_MESSAGE_WARNING,
                                                GTK_BUTTONS_OK_CANCEL,
                                                "Are you sure you want to delete this entry?");
    switch (gtk_dialog_run (GTK_DIALOG (dialog)))
    {
    case GTK_RESPONSE_OK:
        on_real_delete (NULL, user_data);
        break;
    case GTK_RESPONSE_CANCEL:
    case GTK_RESPONSE_NONE:
        /* Nothing to do */
        break;
    default:
        g_assert_not_reached ();
    }
    gtk_widget_destroy (dialog);
}

static void
g_reminder_actions_dispose (GObject *object)
{
    GReminderActionsPrivate *priv = g_reminder_actions_get_instance_private (G_REMINDER_ACTIONS (object));

    if (priv->c_signals[G_REMINDER_ACTION_FIRST])
    {
        for (GReminderAction a = G_REMINDER_ACTION_FIRST; a != G_REMINDER_ACTION_LAST; ++a)
        {
            g_signal_handler_disconnect (priv->actions[a], priv->c_signals[a]);
            priv->c_signals[a] = 0;
        }
    }

    G_OBJECT_CLASS (g_reminder_actions_parent_class)->dispose (object);
}

static void
g_reminder_actions_class_init (GReminderActionsClass *klass)
{
    G_OBJECT_CLASS (klass)->dispose = g_reminder_actions_dispose;

    for (GReminderAction a = G_REMINDER_ACTION_FIRST; a != G_REMINDER_ACTION_LAST; ++a)
    {
        actions[a].signal = g_signal_new (actions[a].name,
                                          G_REMINDER_TYPE_ACTIONS,
                                          G_SIGNAL_RUN_LAST,
                                          0, /* class offset */
                                          NULL, /* accumulator */
                                          NULL, /* accumulator data */
                                          g_cclosure_marshal_VOID__VOID,
                                          G_TYPE_NONE,
                                          0); /* number of params */
    }
}

static void
g_reminder_actions_init (GReminderActions *self)
{
    GReminderActionsPrivate *priv = g_reminder_actions_get_instance_private ((GReminderActions *) self);

    GtkBox *box = GTK_BOX (self);

    for (GReminderAction a = G_REMINDER_ACTION_FIRST; a != G_REMINDER_ACTION_LAST; ++a)
    {
        GtkWidget *button = gtk_button_new_with_label (actions[a].label);
        priv->actions[a] = GTK_BUTTON (button);
        priv->c_signals[a] = g_signal_connect (G_OBJECT (button),
                                               "pressed",
                                               actions[a].callback,
                                               self);
        gtk_box_pack_start (box, button, TRUE, TRUE, 0);
    }

    g_reminder_actions_set_state (self, G_REMINDER_STATE_BLANK);
}

G_REMINDER_VISIBLE GtkWidget *
g_reminder_actions_new (void)
{
    return gtk_widget_new (G_REMINDER_TYPE_ACTIONS,
                           "orientation", GTK_ORIENTATION_HORIZONTAL,
                           "spacing",     2,
                           "homogeneous", TRUE,
                           "margin-left",  12,
                           NULL);
}
