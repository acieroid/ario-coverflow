/*
 *  Copyright (C) 2011 - Quentin Stievenart <quentin.stievenart@gmail.com>
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

#ifndef __ARIO_COVERFLOW_H
#define __ARIO_COVERFLOW_H

#include <gtk/gtkhbox.h>
#include <config.h>
#include "sources/ario-source.h"

G_BEGIN_DECLS

#define TYPE_ARIO_COVERFLOW         (ario_coverflow_get_type ())
#define ARIO_COVERFLOW(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_ARIO_COVERFLOW, ArioCoverflow))
#define ARIO_COVERFLOW_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), TYPE_ARIO_COVERFLOW, ArioCoverflowClass))
#define IS_ARIO_COVERFLOW(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_ARIO_COVERFLOW))
#define IS_ARIO_COVERFLOW_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_ARIO_COVERFLOW))
#define ARIO_COVERFLOW_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_ARIO_COVERFLOW, ArioCoverflowClass))

typedef struct ArioCoverflowPrivate ArioCoverflowPrivate;

typedef struct
{
        ArioSource parent;

        ArioCoverflowPrivate *priv;
} ArioCoverflow;

typedef struct
{
        ArioSourceClass parent;
} ArioCoverflowClass;

GType                   ario_coverflow_get_type   (void) G_GNUC_CONST;

GtkWidget*              ario_coverflow_new        (GtkUIManager *mgr);

G_END_DECLS

#endif /* __ARIO_COVERFLOW_H */
