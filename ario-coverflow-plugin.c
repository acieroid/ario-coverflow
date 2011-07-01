/*
 * Copyright (C) 2011 Quentin Stievenart <quentin.stievenart@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "ario-coverflow-plugin.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <ario-debug.h>
#include <ario-shell.h>
#include <ario-source-manager.h>
#include "ario-coverflow.h"

#define ARIO_COVERFLOW_PLUGIN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), ARIO_TYPE_COVERFLOW_PLUGIN, ArioCoverflowPluginPrivate))

struct _ArioCoverflowPluginPrivate
{
        GtkWidget *source;
};

ARIO_PLUGIN_REGISTER_TYPE(ArioCoverflowPlugin, ario_coverflow_plugin)

static void
ario_coverflow_plugin_init (ArioCoverflowPlugin *plugin)
{
        plugin->priv = ARIO_COVERFLOW_PLUGIN_GET_PRIVATE (plugin);
}

static void
ario_coverflow_plugin_finalize (GObject *object)
{
        G_OBJECT_CLASS (ario_coverflow_plugin_parent_class)->finalize (object);
}

static void
impl_activate (ArioPlugin *plugin,
               ArioShell *shell)
{
        GtkUIManager *uimanager;
        ArioCoverflowPlugin *p = ARIO_COVERFLOW_PLUGIN (plugin);
        g_object_get (shell, "ui-manager", &uimanager, NULL);

        p->priv->source = ario_coverflow_new (uimanager);
        g_return_if_fail (IS_ARIO_COVERFLOW (p->priv->source));

        g_object_unref(uimanager);

        ario_source_manager_append (ARIO_SOURCE (p->priv->source));
        ario_source_manager_reorder ();
}

static void
impl_deactivate (ArioPlugin *plugin,
                 ArioShell *shell)
{
        ArioCoverflowPlugin *p = ARIO_COVERFLOW_PLUGIN (plugin);
        ario_source_manager_remove (ARIO_SOURCE (p->priv->source));
}

static void
ario_coverflow_plugin_class_init (ArioCoverflowPluginClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        ArioPluginClass *plugin_class = ARIO_PLUGIN_CLASS (klass);

        object_class->finalize = ario_coverflow_plugin_finalize;

        plugin_class->activate = impl_activate;
        plugin_class->deactivate = impl_deactivate;

        g_type_class_add_private (object_class, sizeof (ArioCoverflowPluginPrivate));
}
