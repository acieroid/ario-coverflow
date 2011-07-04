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

#ifdef GDK_WINDOWING_QUARTZ
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "ario-debug.h"
#include "ario-util.h"
#include "covers/ario-cover.h"
#include "covers/ario-cover-handler.h"
#include "preferences/ario-preferences.h"
#include "lib/ario-conf.h"
#include "lib/gtk-builder-helpers.h"
#include "plugins/ario-plugin.h"
#include "servers/ario-server.h"

#define LIST_SQUARE 1
#define N_COVERS 7

static void ario_coverflow_finalize (GObject *object);
static void ario_coverflow_set_property (GObject *object,
                                           guint prop_id,
                                           const GValue *value,
                                           GParamSpec *pspec);
static void ario_coverflow_get_property (GObject *object,
                                           guint prop_id,
                                           GValue *value,
                                           GParamSpec *pspec);

static void realize (GtkWidget *widget, gpointer data);
static gboolean expose_event (GtkWidget *widget,
                              GdkEventExpose *event,
                              gpointer data);
static gboolean configure_event (GtkWidget *widget,
                                 GdkEvent *event,
                                 gpointer data);

static void draw_square (void);
static void draw_albums (void);

static void allocate_textures (ArioCoverflow *coverflow);
static void load_texture (ArioServerAlbum *album);

static void gl_init_lights(void);
static void gl_init_textures(ArioCoverflow *coverflow);

struct ArioCoverflowPrivate
{
        gboolean connected;

        GtkUIManager *ui_manager;

        GtkWidget *error_label;
        GtkWidget *drawing_area;

        GList *album;
        GLuint textures[N_COVERS];

        gboolean gl_initialized;
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
}

static void
ario_coverflow_unselect (ArioSource *source)
{
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
        GdkGLConfig *glconfig = NULL;

        coverflow->priv = ARIO_COVERFLOW_GET_PRIVATE (coverflow);

        /* Create scrolled window */
        scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

        /* Initialize opengl or display an error */
        ARIO_LOG_DBG("Initializing OpenGL");
        coverflow->priv->gl_initialized = FALSE; /* not initialized by default */
        if (!gtk_gl_init_check(NULL, NULL))  {
                coverflow->priv->error_label = gtk_label_new ("Can't initialize OpenGL");
                gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolledwindow), 
                                                       coverflow->priv->error_label);
        }
        else {
                glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB |
                                                      GDK_GL_MODE_DEPTH |
                                                      GDK_GL_MODE_DOUBLE);
                if (glconfig == NULL) {
                        ARIO_LOG_DBG("Cannot find double-buffered visual");
                        glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB |
                                                              GDK_GL_MODE_DEPTH);
                        if (glconfig == NULL) {
                                coverflow->priv->error_label = gtk_label_new ("Can't find any OpenGL-capable visual");
                                gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolledwindow), 
                                                                       coverflow->priv->error_label);
                        }
                        else {
                                coverflow->priv->gl_initialized = TRUE;
                        }
                }
                else {
                        coverflow->priv->gl_initialized = TRUE;
                }
        }

        /* If we have initialized GL, we can create the drawing area */
        if (coverflow->priv->gl_initialized) {
                ARIO_LOG_DBG ("OpenGL initialized");
                coverflow->priv->drawing_area = gtk_drawing_area_new();

                gtk_widget_set_gl_capability (coverflow->priv->drawing_area,
                                              glconfig, NULL, TRUE,
                                              GDK_GL_RGBA_TYPE);
                gtk_widget_add_events (coverflow->priv->drawing_area,
                                       GDK_BUTTON_PRESS_MASK |
                                       GDK_VISIBILITY_NOTIFY_MASK);

                g_signal_connect_after (G_OBJECT (coverflow->priv->drawing_area),
                                        "realize", G_CALLBACK (realize), coverflow);
                g_signal_connect (G_OBJECT (coverflow->priv->drawing_area),
                                  "expose_event", G_CALLBACK (expose_event), 
                                  coverflow);
                g_signal_connect (G_OBJECT (coverflow->priv->drawing_area),
                                  "configure_event", G_CALLBACK (configure_event),
                                  coverflow);

                gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolledwindow),
                                                       coverflow->priv->drawing_area);

                /* Get the album list */
                coverflow->priv->album = ario_server_get_albums(NULL);
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

static void
realize (GtkWidget *widget, gpointer data)
{
        ArioCoverflow *coverflow = (ArioCoverflow *) data;

        GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
        GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

        if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
                return;

        glClearColor (0.1, 0.1, 0.1, 1.0);
        glClearDepth (1.0);
        gl_init_lights();
        gl_init_textures(coverflow);
        allocate_textures(coverflow);

        /* Display lists */
        glNewList (LIST_SQUARE, GL_COMPILE);
            draw_square ();
        glEndList ();

        gdk_gl_drawable_gl_end (gldrawable);
}

static gboolean
expose_event (GtkWidget *widget,
              GdkEventExpose *event,
              gpointer data)
{
        GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
        GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

        if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
                return FALSE;

        /* Clear */
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        /* Draw */
        draw_albums();
    
        /* Swap buffers */
        if (gdk_gl_drawable_is_double_buffered (gldrawable))
                gdk_gl_drawable_swap_buffers (gldrawable);
        else
                glFlush ();

        gdk_gl_drawable_gl_end (gldrawable);
        return TRUE;
}

static gboolean
configure_event (GtkWidget *widget,
                 GdkEvent *event,
                 gpointer data)
{
        GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
        GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
        GtkAllocation allocation;

        if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
                return FALSE;

        gtk_widget_get_allocation (widget, &allocation);  
        glViewport(allocation.x, allocation.y, allocation.width, allocation.height);

        gdk_gl_drawable_gl_end (gldrawable);
        return TRUE;
}


static void
draw_square (void)
{
        int i;
        static GLfloat vertices[4][2] = {
            { -0.5, -0.5 },
            { -0.5,  0.5 },
            {  0.5,  0.5 },
            {  0.5, -0.5 },
        };
        static GLfloat texture[4][2] = {
            { 1, 0 },
            { 0, 0 },
            { 0, 1 },
            { 1, 1 },
        };
        static GLfloat normal[3] = { 0.0, 0.0, 1.0 };

        glBegin (GL_QUADS);
            glNormal3fv (normal);
            for (i = 0; i < 4; i++) {
                    glVertex2fv (vertices[i]);
                    glTexCoord2fv (texture[i]);
            }
        glEnd ();
}

static void
draw_albums (void)
{
        guchar *pixels;
        GdkPixbuf *pixbuf;
        GLsizei width, height;

        pixbuf = ario_cover_handler_get_large_cover ();
        if (pixbuf == NULL) {
                ARIO_LOG_DBG ("No cover");
                return;
        }

        pixels = gdk_pixbuf_get_pixels (pixbuf);
        width = gdk_pixbuf_get_width (pixbuf);
        height = gdk_pixbuf_get_height (pixbuf);

/*        glBindTexture (GL_TEXTURE_2D, texture1);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, 
                      GL_UNSIGNED_BYTE, (GLvoid *) pixels); 
        glCallList (LIST_SQUARE);
*/
}

static void
allocate_textures (ArioCoverflow *coverflow)
{
        int i, texture_left, texture_right;
        GList *left, *right;

        /* Current cover */
        glBindTexture (GL_TEXTURE_2D, coverflow->priv->textures[N_COVERS/2]);
        load_texture(coverflow->priv->album->data);

        /* Left side */
        texture_left = N_COVERS/2-1;
        texture_right = N_COVERS/2+1;
        right = left = coverflow->priv->album;
        for (i = 0; i < N_COVERS/2; i++) {
                if (g_list_previous (left)) {
                        left = g_list_previous (left);
                        glBindTexture (GL_TEXTURE_2D, coverflow->priv->textures[texture_left]);
                        load_texture (left->data);
                        texture_left--;
                }
                if (g_list_next (right)) {
                        right = g_list_next (right);
                        glBindTexture (GL_TEXTURE_2D, coverflow->priv->textures[texture_right]);
                        load_texture (right->data);
                        texture_right++;
                }
        }
}

static void
load_texture (ArioServerAlbum *album)
{
        GLsizei width, height;
        guchar *pixels;
        gchar *cover_path;
        GdkPixbuf *pixbuf;

        ARIO_LOG_DBG ("Loading texture for: %s - %s", album->artist, album->album);
        cover_path = ario_cover_make_cover_path (album->artist, album->album, NORMAL_COVER);
        pixbuf = gdk_pixbuf_new_from_file (cover_path, NULL);
        if (pixbuf == NULL) return; /* TODO: allocate a "blank" cover */

        pixels = gdk_pixbuf_get_pixels (pixbuf);
        width = gdk_pixbuf_get_width (pixbuf);
        height = gdk_pixbuf_get_height (pixbuf);

        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, 
                      GL_UNSIGNED_BYTE, (GLvoid *) pixels); 

        g_free (cover_path);
        g_free (pixels);
        /*g_object_unref (pixbuf);*/
}

static void
gl_init_lights(void)
{
        static GLfloat light_pos[] = { 0.0, 0.0, 3.0, 0.0 };
        static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
        static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };

        glLightfv (GL_LIGHT0, GL_POSITION, light_pos);
        glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);

        glEnable (GL_LIGHTING);
        glEnable (GL_LIGHT0);
        glEnable (GL_DEPTH_TEST);
}

static void
gl_init_textures(ArioCoverflow *coverflow)
{
        int i;
        glEnable (GL_TEXTURE_2D);

        glGenTextures(N_COVERS, coverflow->priv->textures);
        for (i = 0; i < N_COVERS; i++) {
                glBindTexture(GL_TEXTURE_2D, coverflow->priv->textures[i]);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
}
