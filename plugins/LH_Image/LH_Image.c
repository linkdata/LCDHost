/**
  \file     LH_Image.c
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009 Johan Lindh

  This file is part of LCDHost.

  LCDHost is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LCDHost is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with LCDHost.  If not, see <http://www.gnu.org/licenses/>.


  */

#include <QtCore/QtGlobal>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <assert.h>

#include "LH_Image.h"
#include "logo_blob.h"
#include "lh_plugin.h"

#define VERSION "0.0.1"

#ifndef Q_UNUSED
# if defined(Q_CC_INTEL) && !defined(Q_OS_WIN) || defined(Q_CC_RVCT)
template <typename T>
inline void qUnused(T &x) { (void)x; }
#   define Q_UNUSED(x) qUnused(x);
# else
#   define Q_UNUSED(x) (void)x;
# endif
#endif

LH_SIGNATURE();

#ifndef STRINGIZE
# define STRINGIZE_(x) #x
# define STRINGIZE(x) STRINGIZE_(x)
#endif

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>Image</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" "r" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Johan \"SirReal\" Lindh</author>"
  "<homepageurl><a href=\"http://www.linkdata.se/software/lcdhost\">Link Data Stockholm</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
    "Display static images."
  "</shortdesc>"
  "<longdesc>"
    "Show the contents of image files on screen. This plugin comes with "
    "source code illustrating how a plugin written in C might look."
  "</longdesc>"
"</lcdhostplugin>";

/**************************************************************************
** Object Image
**************************************************************************/

/**
  This structure is what we'll be using as an 'instance' of ourselves.
  */
typedef struct _lh_image_s
{
    char filename[256];
    lh_blob *blob;
    lh_callback_t cb;
    int cb_id;
    const lh_systemstate *state;
    lh_setup_item setup_filename;
    lh_setup_item *setup_array[2];
} lh_image;

/**
  image_new() allocates a new instance of this class and returns it.
  */
static void * image_new( const lh_class *cls )
{
    lh_image *img;
    Q_UNUSED(cls);

    img = calloc(1, sizeof(lh_image));
    if( img )
    {
        img->blob = NULL;
        img->state = 0;
        memset( &img->setup_filename, 0, sizeof(img->setup_filename) );
        img->setup_filename.name = "Filename";
        img->setup_filename.help = NULL;
        img->setup_filename.type = lh_type_string_filename;
        img->setup_filename.param.size = sizeof(img->filename);
        img->setup_filename.data.s = img->filename;
        img->setup_filename.flags = 0;
        img->setup_array[0] = & img->setup_filename;
        img->setup_array[1] = NULL;
        return img;
    }

    return NULL;
}

/**
  */

static const char *image_init(void*obj,lh_callback_t cb,int cb_id,const char*name,const lh_systemstate*state)
{
    lh_image *img = obj;
    Q_UNUSED(name);
    img->cb = cb;
    img->cb_id = cb_id;
    img->state = state;
    return 0;
}

/**
  Since we store the setup array inside our struct, and do
  the setup when creating it, just return the array.

  This function is called right after the instance is created,
  and the returned data is expected to be valid until the instance
  is destroyed.
  */
static lh_setup_item ** image_setup_data(void *obj)
{
    lh_image *img = obj;
    return img->setup_array;
}

/**
  React to changes in setup immediately. We don't need to check which
  item is changed since we only have one. To illustrate self-unloading,
  if the filename equals 'unload', we request unloading.
  */
static void image_setup_change(void*obj,lh_setup_item *i)
{
    lh_image *img = obj;
    if( img && img->cb )
    {
        if( !strcmp( i->data.s, "unload" ) )
        {
            strcpy( i->data.s, "was-unloaded" );
            img->cb( img->cb_id, img, lh_cb_setup_refresh, i );
            img->cb( img->cb_id, img, lh_cb_unload, "Filename was 'unload'." );
        }
        else
        {
            if( img->blob )
            {
                free( img->blob );
                img->blob = NULL;
            }
            img->cb( img->cb_id, img, lh_cb_render, NULL );
        }
    }
    return;
}

/**
  Not using input.
  */
static void image_input(void*obj,lh_setup_item *i,int f,int v)
{
    Q_UNUSED(obj);
    Q_UNUSED(i);
    Q_UNUSED(f);
    Q_UNUSED(v);
    return;
}

/**
  We don't use polling, so we always return zero.
  */
static int image_polling(void*obj)
{
    Q_UNUSED(obj);
    return 0;
}

/**
  If the image loading has failed, we'll retry every second
  */
static int image_notify(void*obj,int note,void *param)
{
    lh_image *img = obj;
    Q_UNUSED(param);
    if( img && ((!note) || (note&LH_NOTE_SECOND)) )
    {
        if( img->blob == 0 )
            img->cb( img->cb_id, img, lh_cb_render, 0 );
    }
    return LH_NOTE_SECOND;
}

/**
  This might be a good place to check if the image file
  has been updated.
  */
static void image_prerender(void*obj)
{
    Q_UNUSED(obj);
    return;
}

/**
  We're so primitive, we can't even tell what size picture
  we'll be returning. No worries. Negative size means
  'I don't know', and LCDHost will get the size from the
  image data.
  */
static int image_width(void*obj,int h)
{
    Q_UNUSED(obj);
    Q_UNUSED(h);
    return -1;
}

static int image_height(void*obj,int w)
{
    Q_UNUSED(obj);
    Q_UNUSED(w);
    return -1;
}

static int is_absolute( const char *filename )
{
    const char *p;
    if( !filename || !*filename ) return 0;
    p = filename;
    while( *p && isspace(*p) ) p ++;
    if( *p == '\\' || *p == '/' ) return 1;
    if( isalpha(*p) && *(p+1) == ':' ) return 1;
    return 0;
}

/**
    lh_binaryfile_to_blob()

    Reads a file and converts it into a lh_blob structure.

    \param filename
        Name of the file to load into a blob.
    \return
        A pointer to a malloc()'d lh_blob. The caller must
        free this when done with it.
*/
lh_blob *lh_binaryfile_to_blob( const char *filename )
{
    lh_blob *blob = NULL;
    long filesize = 0;
    FILE *f;

    if( filename == NULL ) return NULL;

    f = fopen( filename, "rb" );
    if( f != NULL )
    {
        if( fseek( f, 0, SEEK_END ) == 0 ) filesize = ftell( f );
        if( filesize > 0 )
        {
            fseek( f, 0, SEEK_SET );
            blob = malloc( sizeof(lh_blob) + filesize );
            if( blob )
            {
                blob->sign = 0xDEADBEEF;
                blob->len = filesize;
                if( fread( & blob->data, 1, filesize, f ) != (size_t) filesize )
                {
                    blob->sign = 0;
                    blob->len = 0;
                    free( blob );
                    blob = NULL;
                }
            }
        }
        fclose( f );
    }

    return blob;
}

/**
  We'll just use the provided lh_binaryfile_to_blob()
  function to allocate and fill a blob with the file
  data. Let LCDHost handle B/W color matching and
  sizing issues that may arise.
  */
static const lh_blob * image_render_blob(void*obj,int w,int h)
{
    char *fullname;
    size_t n;
    lh_image *img = obj;

    Q_UNUSED(w);
    Q_UNUSED(h);

    if( img->blob == NULL )
    {
        const char *dir_layout = img->state->dir_layout;
        if( !is_absolute(img->filename) && dir_layout )
        {
            n = strlen( dir_layout ) + strlen( img->filename ) + 1;
            fullname = malloc( n );
            if( fullname )
            {
                strcpy( fullname, dir_layout );
                strcat( fullname, img->filename );
                /* convert it from UTF-8 to local 8 bit */
                img->cb( img->cb_id, img, lh_cb_utf8_to_local8bit, fullname );
                img->blob = lh_binaryfile_to_blob( fullname );
                free( fullname );
            }
        }
        else
            img->blob = lh_binaryfile_to_blob( img->filename );
    }

    return img->blob ? img->blob : logo_blob;
}

/**
  Free resources.
  */
static void image_delete(void*obj)
{
    lh_image *img = obj;
    if( img->blob ) free( img->blob );
    memset( img, 0, sizeof(lh_image) );
    free( img );
    return;
}

/**************************************************************************
** LCDHost data
**************************************************************************/

static lh_class class_image =
{
    sizeof(lh_class),
    "Static",
    "StaticImage",
    "Image",
    -1,-1,
    {
        sizeof(lh_object_calltable),
        image_init,
        image_setup_data,
        0, /* not using setup_resize, the filename isn't dynamically allocated */
        image_setup_change,
        image_input,
        image_polling,
        image_notify,
        0, /* not using class list */
        0 /* not using term */
    },
    {
        sizeof(lh_instance_calltable),
        image_new,
        image_prerender,
        image_width,
        image_height,
        image_render_blob,
        0, /* not using render_qimage */
        image_delete
    }
};

/**************************************************************************
** Exported functions
**
** Note that you'll probably have to modify the EXPORT define to match
** what your compiler wants to see in order to declare these functions
** as exported by the shared library (DLL).
**************************************************************************/

#ifdef Q_DECL_EXPORT
# define EXPORT Q_DECL_EXPORT
#endif

#ifdef _MSC_VER
# define EXPORT __declspec(dllexport)
#endif

#ifndef EXPORT
# define EXPORT
#endif

static const char *plugin_init(void*ref,lh_callback_t cb,int cb_id,const char*name,const lh_systemstate*state)
{
    Q_UNUSED(ref);
    Q_UNUSED(cb);
    Q_UNUSED(cb_id);
    Q_UNUSED(name);
    Q_UNUSED(state);
    return 0;
}

static const lh_class *lh_image_classes[2] =
{
    & class_image,
    NULL
};

static const lh_class ** plugin_class_list(void*ref)
{
    Q_UNUSED(ref);
    return lh_image_classes;
}

EXPORT void *lh_create()
{
    return & class_image;
}

static lh_object_calltable lh_image_objtable =
{
    sizeof(lh_object_calltable),
    plugin_init, /* init */
    0, /* setup_data */
    0, /* setup_resize */
    0, /* setup_change */
    0, /* input */
    0, /* polling */
    0, /* notify */
    plugin_class_list, /* class list */
    0 /* term */
};

EXPORT const lh_object_calltable* lh_get_object_calltable( void *ref )
{
    Q_UNUSED(ref);
    return & lh_image_objtable;
}

EXPORT void lh_destroy( void *ref )
{
    Q_UNUSED(ref);
    return;
}

/* eof */
