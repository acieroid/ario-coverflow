/*
 *  Copyright (C) 2011 Quentin Stievenart <quentin.stievenart@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "ario-coverflow.h"
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <string.h>
#include <config.h>
#include <glib/gi18n.h>

/* TODO: not include all of this */
#include "ario-debug.h"
#include "ario-util.h"
#include "covers/ario-cover.h"
#include "covers/ario-cover-handler.h"
#include "preferences/ario-preferences.h"
#include "lib/ario-conf.h"
#include "lib/gtk-builder-helpers.h"
#include "plugins/ario-plugin.h"

static void ario_coverflow_finalize (GObject *object);
static void ario_coverflow_set_property (GObject *object,
                                           guint prop_id,
                                           const GValue *value,
                                           GParamSpec *pspec);
static void ario_coverflow_get_property (GObject *object,
                                           guint prop_id,
                                           GValue *value,
                                           GParamSpec *pspec);

struct ArioCoverflowPrivate
{
        gboolean connected;

        GtkUIManager *ui_manager;

        GtkWidget *error_label;

        GSList *albums;

        /* TODO: maybe useless */
        gboolean selected;
};

/* Object properties */
enum
{
        PROP_0,
        PROP_UI_MANAGER
};

#define ARIO_COVERFLOW_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_ARIO_COVERFLOW, ArioCoverflowPrivate))
G_DEFINE_TYPE (ArioCoverflow, ario_coverflow, ARIO_TYPE_SOURCE)

static gchar *
ario_coverflow_get_id (ArioSource *source)
{
        return "coverflow";
}

static gchar *
ario_coverflow_get_name (ArioSource *source)
{
        return _("Coverflow");
}

static gchar *
ario_coverflow_get_icon (ArioSource *source)
{
        return GTK_STOCK_CDROM;
}

static void
ario_coverflow_select (ArioSource *source)
{
        ArioCoverflow *coverflow = ARIO_COVERFLOW (source);
        /* TODO */
        coverflow->priv->selected = TRUE;
}

static void
ario_coverflow_unselect (ArioSource *source)
{
        ArioCoverflow *coverflow = ARIO_COVERFLOW (source);

        /* Remember to be lazy until tab is selected again */
        coverflow->priv->selected = FALSE;
}

static void
ario_coverflow_class_init (ArioCoverflowClass *klass)
{
        ARIO_LOG_FUNCTION_START;
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        ArioSourceClass *source_class = ARIO_SOURCE_CLASS (klass);

        /* GObject virtual methods */
        object_class->finalize = ario_coverflow_finalize;
        object_class->set_property = ario_coverflow_set_property;
        object_class->get_property = ario_coverflow_get_property;

        /* ArioSource virtual methods */
        source_class->get_id = ario_coverflow_get_id;
        source_class->get_name = ario_coverflow_get_name;
        source_class->get_icon = ario_coverflow_get_icon;
        source_class->select = ario_coverflow_select;
        source_class->unselect = ario_coverflow_unselect;

        /* Object properties */
        g_object_class_install_property (object_class,
                                         PROP_UI_MANAGER,
                                         g_param_spec_object ("ui-manager",
                                                              "GtkUIManager",
                                                              "GtkUIManager object",
                                                              GTK_TYPE_UI_MANAGER,
                                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

        /* Private attributes */
        g_type_class_add_private (klass, sizeof (ArioCoverflowPrivate));
}


static void
ario_coverflow_init (ArioCoverflow *coverflow)
{
        ARIO_LOG_FUNCTION_START;
        GtkWidget *scrolledwindow;

        coverflow->priv = ARIO_COVERFLOW_GET_PRIVATE (coverflow);

        /* Create scrolled window */
        scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

        /* Initialize opengl or display an error */
        ARIO_LOG_DBG("Initializing OpenGL");
        if (!gtk_gl_init_check(NULL, NULL)) {
                coverflow->priv->error_label = gtk_label_new ("Can't initialize OpenGL");
                gtk_scrolled_window_add_with_viewport (scrolledwindow, 
                                                       coverflow->priv->error_label);
        }

        gtk_widget_show_all (scrolledwindow);

        /* Add scrolled window to coverflow */
        gtk_box_pack_start (GTK_BOX (coverflow), scrolledwindow, TRUE, TRUE, 0);
}

static void
ario_coverflow_finalize (GObject *object)
{
        ARIO_LOG_FUNCTION_START;
        ArioCoverflow *coverflow;

        g_return_if_fail (object != NULL);
        g_return_if_fail (IS_ARIO_COVERFLOW (object));

        coverflow = ARIO_COVERFLOW (object);

        g_return_if_fail (coverflow->priv != NULL);

        G_OBJECT_CLASS (ario_coverflow_parent_class)->finalize (object);
}

static void
ario_coverflow_set_property (GObject *object,
                               guint prop_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
        ARIO_LOG_FUNCTION_START;
        ArioCoverflow *coverflow = ARIO_COVERFLOW (object);

        switch (prop_id) {
        case PROP_UI_MANAGER:
                coverflow->priv->ui_manager = g_value_get_object (value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                break;
        }
}

static void
ario_coverflow_get_property (GObject *object,
                               guint prop_id,
                               GValue *value,
                               GParamSpec *pspec)
{
        ARIO_LOG_FUNCTION_START;
        ArioCoverflow *coverflow = ARIO_COVERFLOW (object);

        switch (prop_id) {
        case PROP_UI_MANAGER:
                g_value_set_object (value, coverflow->priv->ui_manager);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
                break;
        }
}

GtkWidget *
ario_coverflow_new (GtkUIManager *mgr)
{
        ARIO_LOG_FUNCTION_START;
        ArioCoverflow *coverflow;

        coverflow = g_object_new (TYPE_ARIO_COVERFLOW,
                                    "ui-manager", mgr,
                                    NULL);

        g_return_val_if_fail (coverflow->priv != NULL, NULL);

        coverflow->priv->connected = ario_server_is_connected ();

        return GTK_WIDGET (coverflow);
}
