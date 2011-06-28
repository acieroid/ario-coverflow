/*
 * Copyright (C) 2002-2005 - Paolo Maggi
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

#ifndef __ARIO_COVERFLOW_PLUGIN_H__
#define __ARIO_COVERFLOW_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <ario-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define ARIO_TYPE_COVERFLOW_PLUGIN                 (ario_coverflow_plugin_get_type ())
#define ARIO_COVERFLOW_PLUGIN(o)                   (G_TYPE_CHECK_INSTANCE_CAST ((o), ARIO_TYPE_COVERFLOW_PLUGIN, ArioCoverflowPlugin))
#define ARIO_COVERFLOW_PLUGIN_CLASS(k)             (G_TYPE_CHECK_CLASS_CAST((k), ARIO_TYPE_COVERFLOW_PLUGIN, ArioCoverflowPluginClass))
#define ARIO_IS_COVERFLOW_PLUGIN(o)                (G_TYPE_CHECK_INSTANCE_TYPE ((o), ARIO_TYPE_COVERFLOW_PLUGIN))
#define ARIO_IS_COVERFLOW_PLUGIN_CLASS(k)          (G_TYPE_CHECK_CLASS_TYPE ((k), ARIO_TYPE_COVERFLOW_PLUGIN))
#define ARIO_COVERFLOW_PLUGIN_GET_CLASS(o)         (G_TYPE_INSTANCE_GET_CLASS ((o), ARIO_TYPE_COVERFLOW_PLUGIN, ArioCoverflowPluginClass))

/* Private structure type */
typedef struct _ArioCoverflowPluginPrivate        ArioCoverflowPluginPrivate;

/*
 * Main object structure
 */
typedef struct _ArioCoverflowPlugin                ArioCoverflowPlugin;

struct _ArioCoverflowPlugin
{
        ArioPlugin parent_instance;

        /*< private >*/
        ArioCoverflowPluginPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _ArioCoverflowPluginClass        ArioCoverflowPluginClass;

struct _ArioCoverflowPluginClass
{
        ArioPluginClass parent_class;
};

/*
 * Public methods
 */
GType        ario_coverflow_plugin_get_type                (void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_ario_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __ARIO_COVERFLOW_PLUGIN_H__ */

