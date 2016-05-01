/**
  \file     lh_plugin.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Link Data Stockholm nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
  */

/**
  These are the entry points for LCDHost plugins. Normal sequence is:

  1.  LCDHost examines the plugin file for build info XML and signature.
  2.  Either the user decides to load the plugin or it's set to auto load
  3.  LCDHost has the operating system load the shared library
  4.  lh_create() is called, requires non-NULL return
  5.  lh_get_object_calltable() is called, requires non-NULL return
  6.  obj_init() is called, returns NULL or error message
    * at this point, barring errors, the plugin is considered loaded
  7.  if defined in calltable, obj_class_list() is called to retrieve list of classes
    * when the user decides to unload the plugin or LCDHost shuts down:
  8.  if defined in calltable, obj_term() is called
  9.  lh_destroy() is called
  10. LCDHost has the operating system unload the shared library

  void *lh_create()
    Create the plugin object and return an opaque reference to it.
    LCDHost will provide this reference when calling other functions
    as the 'ref' parameter. Return NULL on error.

  const lh_object_calltable* lh_get_object_calltable( void *ref )
    Returns a standard object calltable. The function pointers in
    the structure may be NULL except for obj_init() which is required.

  void lh_destroy( void *ref )
    Free resources associated with 'ref'. The shared library is about to be
    removed from memory.

  */

#ifndef LH_PLUGIN_H
#define LH_PLUGIN_H

#include <stdlib.h>
#include "lh_systemstate.h"

#define LH_API_MAJOR 5
#define LH_API_MINOR 0
#define LH_DEVICE_MAXBUTTONS 32

/**
  Notify flags. One or more of these are set in the 'note' parameter to the
  notify() function, and used in the return value from that same function
  to set the notification mask. Return zero if you're not interested in notifications.
  */
#define LH_NOTE_NONE        0x00000000
#define LH_NOTE_SECOND      0x00000001 /* Once-a-second notification */
#define LH_NOTE_CPU         0x00000002 /* CPU load data updated */
#define LH_NOTE_MEM         0x00000004 /* memory load data updated */
#define LH_NOTE_NET         0x00000008 /* network load data updated */
#define LH_NOTE_DEVICE      0x00000010 /* output device has changed */

#define LH_ALIGN_LEFT       0
#define LH_ALIGN_CENTER     1
#define LH_ALIGN_RIGHT      2
#define LH_ALIGN_TOP        LH_ALIGN_LEFT
#define LH_ALIGN_BOTTOM     LH_ALIGN_RIGHT
#define LH_ALIGN_MAX        LH_ALIGN_RIGHT

#define LH_METHOD_ADJUST    0   /**< add size offset size source */
#define LH_METHOD_PERMILLE  1   /**< make size X 1/1000:s of source (e.g. 1000 = same as source) */
#define LH_METHOD_LEFT      2   /**< adjust size so our right edge is the same as source left/top */
#define LH_METHOD_RIGHT     3   /**< adjust size so our right edge is the same as source right/bottom */
#define LH_METHOD_ABSOLUTE  4   /**< size is set to whatever offset is set to */
#define LH_METHOD_TOP       LH_METHOD_LEFT
#define LH_METHOD_BOTTOM    LH_METHOD_RIGHT
#define LH_METHOD_MAX       LH_METHOD_ABSOLUTE

typedef double qreal;
typedef struct lh_class_t lh_class;

/**
  All plugins must embed an XML document containing build information.
  The document must be readable from the plugin shared library using
  standard binary file I/O. The document must contain the maintag
  <lcdhostplugin>. LCDHost will search the shared library for the
  string '<lcdhostplugin>' and then check to see that it's a correctly
  formed XML document. Encoding must be UTF-8. There must be only one
  copy of the string '<lcdhostplugin>' in the shared library.

  Sample C code to embed the document, copy, paste and modify to suit.
  Note that you could also embed the document using other methods.


  char __lcdhostplugin_xml[] =
  "<?xml version=\"1.0\"?>"
  "<lcdhostplugin>"
    "<id>NAME</id>"
    "<rev>" STRINGIZE(REVISION) "</rev>"
    "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
    "<ver>" "r" STRINGIZE(REVISION) "</ver>"
    "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
    "<author>Johan \"SirReal\" Lindh</author>"
    "<homepageurl><a href=\"http://www.linkdata.se/software/lcdhost\">Link Data Stockholm</a></homepageurl>"
    "<logourl></logourl>"
    "<shortdesc>"
    "ONE_LINE_DESCRIPTION"
    "</shortdesc>"
    "<longdesc>"
    "MULTI_LINE_DESCRIPTION"
    "</longdesc>"
  "</lcdhostplugin>";


  'versionurl' is the version information URL. It replaces $ID to <id>,
  $ARCH to the current architecture and $REV to <rev>.
  This URL should return a text/xml document like the following sample:

  <lhver arch="win32" url="http://lcdhost.googlecode.com/files/$ID_$ARCH_R$REV.zip">
   <f id="LH_Text" r="6" />
  </lhver>

  The 'lhver' element contains default attribute values for 'f' elements.
  The 'url' attribute expands the same parameters as the version url.

  A 'f' element may contain the following attributes:
  'id'        The plain filename, without system prefix or suffixes
  'arch'      The architecture (ex, 'win32', 'mac32' or 'lin64d')
  'r'         The revision number
  'url'       The download URL
  'api'       The API versions in the form 'major.minor'

  There may be any number of 'f' elements.

  The document will be cached, so if several plugins refer to the same URL,
  the cached copy will be used. The cache is cleared intermittently.
  */

#ifndef STRINGIZE
# define STRINGIZE_(x) #x
# define STRINGIZE(x) STRINGIZE_(x)
#endif

/**
  The lh_blob type is used to embed binary data, usually a JPG or PNG image file.
  */
typedef struct lh_blob_t
{
    unsigned len; /* sizeof(data) */
    unsigned sign; /* 0xDEADBEEF */
    unsigned char data[1]; /* actual data, length varies */
} lh_blob;

/**
  Used with lh_callback().
  Add new codes AT THE END, or existing plugins will break.
  */
typedef enum lh_callbackcode_t
{
    lh_cb_unload, /* ask that the plugin be unloaded, param: NULL or const char *message */

    lh_cb_setup_refresh, /* LCDHost will re-read your setup item, param: lh_setup_item* */
    lh_cb_setup_rebuild, /* use this if you add or remove setup items, LCDHost will call setup_data() again */
    lh_cb_log, /* log an UTF-8 encoded string in the error log */
    lh_cb_polling, /* ask for a call to the polling function, param: NULL */
    lh_cb_notify, /* ask for a call to the notify function, param: NULL */

    /* This request causes LCDHost to call your lh_class_list() function again. */
    lh_cb_class_refresh,

    /* These requests are meaningful only if you supply layout classes and have active instances */
    lh_cb_render, /* ask for a rendering sequence (prerender/width/height/render), param: NULL */
    lh_cb_sethidden, /* set visibility state for self and children, param int* */

    /* These requests are meaningful only for device drivers */
    lh_cb_arrive, /* a new device have been detected, param is lh_device pointer */
    lh_cb_leave, /* a device has left the system */
    lh_cb_input, /* a device input has changed, param is pointer to lh_device_input */

    lh_cb_load_layout, /* request loading of a new layout, param: UTF-8 encoded file name relative to data path */

    /* Support calls */
    lh_cb_utf8_to_local8bit, /* request UTF-8 to local 8-bit conversion, param: char *string */

    lh_cb_unused
} lh_callbackcode;

/**
  The plugin-to-LCDHost callback.
  */
typedef void (*lh_callback_t)( int cb_id, const void *obj, lh_callbackcode code, void *param );

/**
    Definition of signature area
    The signature area is optional by highly recommended.
*/
typedef struct lh_signature_t
{
    char marker[16]; /* unique series to allow finding the sig */
    char sha1[20]; /* SHA1 of the plugin binary excluding the lh_signature structure */
    /* char sign[256]; */ /* 2048-bit RSA signature of the shared plugin's SHA-1 digest, PKCS1 padded */
    /* char url[128]; */ /* URL to the public key */
    int size; /* sizeof(lh_signature) */
} lh_signature;

#define LH_SIGNATURE_MARKER {'\x07', '\x62', '\x78', '\xf2', '\x72', '\xae', '\xb0', '\x61', '\xb2', '\xf6', '\xe5', '\x74', '\xf3', '\x22', '\x02', '\x5c' }

/* Declare a signature area - don't mess with the constants, */
/* they're there so that SignPlugin can find the right spot. */
#define LH_SIGNATURE() lh_signature _lh_plugin_signature = { LH_SIGNATURE_MARKER, {0}, sizeof(lh_signature) }

typedef enum lh_setup_type_t
{
    lh_type_none,
    lh_type_integer,
    lh_type_integer_boolean, /* checkbox */
    lh_type_integer_color, /* 32-bit AARRGGBB */
    lh_type_integer_slider,
    lh_type_integer_progress,
    lh_type_integer_list, /* using dropdown box, have user select one of the param.list strings */
    lh_type_fraction,
    lh_type_string, /* all strings are null-terminated, utf-8 encoded */
    lh_type_string_script, /* inline script */
    lh_type_string_filename, /* present the user with an file selection dialog */
    lh_type_string_font, /* simple font selection, see QFont::toString() for string format */
    lh_type_string_inputstate, /* ask user for a specific button state or half axis movement */
    lh_type_string_inputvalue, /* ask user for a button or full axis movement */
    lh_type_image_png, /* allows the display of a PNG image in the setup pane (data.s and param.size used) */
    lh_type_image_qimage, /* allows the display of a QImage in the setup pane static_cast<QImage*>(data.s) */
    lh_type_integer_listbox, /* using listbox, have user select one of the param.list strings */
    lh_type_string_button, /* a clickable button */
    lh_type_string_html, /* static HTML text, for best effect start the name with '~' to hide it */
    lh_type_last /* marks last used value */
} lh_setup_type;

#define LH_FLAG_READONLY    0x0001 /* Setup item is disabled in GUI */
#define LH_FLAG_HIDDEN      0x0002 /* Setup item is excluded from GUI */
#define LH_FLAG_FOCUS       0x0004 /* Setup item is focused when user double clicks instance */
#define LH_FLAG_AUTORENDER  0x0008 /* Request a render when changed */
#define LH_FLAG_FIRST       0x0010 /* Used with LH_QtSetupItem's - Same as calling setOrder(-1) */
#define LH_FLAG_LAST        0x0020 /* Used with LH_QtSetupItem's - Same as calling setOrder(1) */
#define LH_FLAG_NOSAVE      0x0040
#define LH_FLAG_BLANKTITLE  0x0080 /* Setup item title is not shown in GUI (blank space is shown) */
#define LH_FLAG_NOSOURCE    0x0100 /* Setup item must not be used as a data source */
#define LH_FLAG_NOSINK      0x0200 /* Setup item must not be used as a data sink */
#define LH_FLAG_HIDETITLE   0x0400 /* Setup item title is not shown in GUI (all space to value) */
#define LH_FLAG_HIDEVALUE   0x0800 /* Setup item value is not shown in GUI (all space to title) */
#define LH_FLAG_NOSAVE_LINK 0x1000
#define LH_FLAG_NOSAVE_DATA 0x2000
#define LH_FLAG_NEEDREFRESH 0x4000

typedef union lh_setup_param_t
{
    size_t size;    /**< size of buffer pointed to by 'data.s' */
    struct {
        int min;    /**< int, slider or progress minimum value */
        int max;    /**< int, slider or progress maximum value */
    } slider;
    struct {
        qreal min;  /**< qreal minimum value */
        qreal max;  /**< qreal maximum value */
    } range;
    const char *list; /**< tab-delimited list of strings */
} lh_setup_param;

typedef union lh_setup_data_t
{
    char *s; /* note that only 's' can be initialized in a global or static, see C syntax rules */
    qreal f;
    int i;
} lh_setup_data;

/**
 You can have LCDHost automatically update a setup item with data from another item.
 The 'link' member, if not NULL, specifies how data linking is handled. The first
 character specifies if it's a data source or a data sink. The rest is a path
 specification separated with slashes. If the first character is an at sign '@'
 then it's a source, with the path defining it's position in the data source tree.
 If the first character is an equal sign '=', then it's a sink, and the path
 specifies which source it wants.

 Examples:
    link = "@/system/Mail count"; // ok, data source providing a mail count
    link = "=/system/Mail count"; // ok, data sink reading the above source
    link = "/system/cpu/count"; // error, missing command character
    link = "=system/cpu/count"; // error, missing initial path slash
*/

#define LH_LINK_SIZE 256

/**
    You can have LCDHost automatically update a setup item with data
    from another item. All setup items have a link path associated with
    them. If you don't specify a path explicitly, one will be generated
    using the id of the item and it's parents.

    Programatically, you can link any two items. If the data can be
    converted, it will be transferred when updated at the source.

    The UI will show a data linking button for a setup item if
    the LH_FLAG_NOSINK is not set. If clicked, the UI will list
    all setup items that do not have the LH_FLAG_NOSOURCE flag and
    have a compatible mimetype, meaning that the sink may be less
    specialized than the source (as in, you can link 'text/plain'
    to 'text') or identical.

    Both the link paths and mime type text are ASCIIZ. Link path
    separator is a forward slash.

    The maximum length is LH_LINK_SIZE, including NUL.

    \c mime
        Mime type - this is not enforced, currently just used
        to filter sources in user UI. If NULL, a default mime
        type will be selected matching the setup item type.
    \c publish
        When this setup item is changed, notify all items that
        are subscribing to the link path 'publish'.
    \c subscribe
        When the setup items that publish to the link path 'subscribe'
        are changed, send the (possibly converted) data to this item.
*/
typedef struct lh_setup_link_t
{
    const char *mime;
    const char *publish;
    char subscribe[LH_LINK_SIZE];
} lh_setup_link;

/**
 Setup items are the main information link between LCDHost and it's plugins.

 Start a setup item name with '^' to not display the name, leaving the name column blank.
 Obsoleted. Use LH_FLAG_BLANKTITLE.

 Start a setup item name with '~' to extend the setup item into the name column.
 Obsoleted. Use LH_FLAG_HIDETITLE.
*/
typedef struct lh_setup_item_t
{
    const char * name; /* name to identify this item uniquely, and display to the user (start with ~ to hide from display */
    const char * help; /* short HTML help text shows as tooltip, may be NULL */
    lh_setup_type type; /* type of data, see enum above */
    int flags; /* LH_FLAG_xxx */
    lh_setup_param param;
    lh_setup_data data;
    lh_setup_link link;
} lh_setup_item;

/**
    Common methods to all objects created in plugins, and also available to the plugin themselves.
    Creation is done using one of the following:
      lh_create() is called for a plugin
      a layout class is instantiated using obj_new() from the lh_instance_calltable
      a device reported using the lh_cb_arrive callback have been created by the plugin
    Initialization is always done with obj_init().
    Termination is always done with obj_term().
    Destruction is done with:
      lh_destroy() for a plugin
      an instance is deleted using obj_delete() from the lh_instance_calltable
      a device reported using the lh_leave callback will shortly be deleted by the plugin
*/
typedef struct lh_object_calltable_t
{
    int size; // sizeof(lh_object_calltable)
    const char * (*obj_init)(void*,lh_callback_t,int,const char*,const lh_systemstate*); /**< return error msg or NULL */
    lh_setup_item ** (*obj_setup_data)(void*); /**< return array of pointers to setup items, NULL terminated */
    void (*obj_setup_resize)(void*, lh_setup_item*, size_t); /**< item data storage is too small, please resize */
    void (*obj_setup_change)(void*, lh_setup_item*); /**< given item has been changed */
    void (*obj_setup_input)(void*, lh_setup_item*, int, int); /**< input item has changed, wanted flags in 'f', new state/value in 'v' */
    int (*obj_polling)(void*); /**< return ms to wait before next call, or zero to stop polling */
    int (*obj_notify)(void*,int,void*); /**< return wanted notification mask, see LH_NOTE_xxx */
    const lh_class **(*obj_class_list)(void*); /**< return current list of layout classes, or NULL */
    void (*obj_term)(void*); /**< terminate */
} lh_object_calltable;

#define lh_object_calltable_NULL { sizeof(lh_object_calltable), 0,0,0,0,0,0,0,0,0 }


/**
  */
typedef struct lh_device_backlight_t
{
    int r, g, b; /* backlight color setting */
    char is_present; /* device has backlight */
    char is_readable; /* device backlight setting is readable */
    char is_writeable; /* device backlight setting is writable */
} lh_device_backlight;


enum lh_device_flag
{
    lh_df_button      = 0x0001,
    lh_df_value       = 0x0002,
    lh_df_down        = 0x0004, // button down or positive values
    lh_df_pos         = 0x0004,
    lh_df_up          = 0x0008, // button up or negative values
    lh_df_neg         = 0x0008,
    lh_df_keyboard    = 0x1000,
    lh_df_mouse       = 0x2000,
    lh_df_joystick    = 0x4000,
    lh_df_other       = 0x8000
};

/**
  Used when the state of a device input (button, slider, stick, whatever) changes.
  The devid must be set, and must be the same as the devid in the lh_device struct.
  The 'item' is used to uniquely identify the control for that device. It may not
  be zero. The 'control' string is the human readable name for the 'item'. If it's
  NULL, it will be substituted by the 'item' in base-16.
  */
typedef struct lh_device_input_t
{
    const char *devid; /* device id */
    const char *control; /* control name, human readable, unique for the device (may be NULL) */
    int item; /* control id, unique for the device (must not be zero) */
    int flags; /* describes kind of control, see lh_device_flag */
    int value; /* */
} lh_device_input;

/**
  This structure is what defines a driver device methods. It's embedded in
  the \c lh_device structure, which is passed as a parameter when a plugin
  reports a new device using the lh_cb_arrive callback value.

  Members in this structure may be set to NULL. Return values are either NULL,
  to indicate success, or a pointer to an UTF-8 encoded error string.
  The exception to this is the \c obj_buttons function, which returns the
  current button state.

  IMPORTANT! Since the plugin creates and destroys devices when it needs,
  you MUST let LCDHost know that a device is dying before it's lh_device pointer
  goes stale, otherwise you'll crash the driver. Similarly, don't advertise
  a device until it's ready to accept calls from LCDHost.

  \code
    myDevice = new myDevice();
    callback(cb_id, myDevice, lh_cb_arrive, myDevice->devtable );
    callback(cb_id, myDevice, lh_cb_leave, myDevice->devtable );
    delete myDevice;
  \endcode

  */
typedef struct lh_device_calltable_t
{
    int size; // sizeof(lh_device_calltable)
    // lh_object_calltable o;
    const char* (*obj_open)(void*); /**< device is selected by user for output */
    const char* (*obj_render_qimage)(void*,void*); /**< render a QImage onto the device */
    const char* (*obj_render_argb32)(void*,int,int,const void*); /**< render an ARB32 bitmap onto the device */
    const char* (*obj_render_mono)(void*,int,int,const void*); /**< render a monochrome one-byte-per-pixel map */
    const char* (*obj_get_backlight)(void*,lh_device_backlight*); /**< return current backlight state */
    const char* (*obj_set_backlight)(void*,lh_device_backlight*); /**< change current backlight state */
    const char* (*obj_close)(void*); /**< device is no longer selected for output */
} lh_device_calltable;

/**
  This structure gives basic information about a device.

  \c name is the text to present to the user as the
  devices name, for example 'Logitech G19'. This text should
  be localized to the users language, or be in English.

  In particular, note the 'obj' member, which will be passed
  as the first parameter to the functions in lh_object_calltable
  and lh_device_calltable.

  A pointer to an instance of this structure is supplied
  with the lh_cb_arrive and lh_cb_leave callbacks. That pointer
  must remain valid from lh_cb_arrive until lh_cb_leave returns.

  Two devices must not share the same lh_device instance.
  */
typedef struct lh_device_t
{
    int size; /* sizeof(lh_device) */
    const void *obj; /* driver internal object reference */
    const char *devid; /* ASCII, uniquely identifies the device with the driver across reboots */
    const char *name; /* UTF-8 encoded, should be localized */
    int width; /* width in pixels */
    int height; /* height in pixels */
    int depth; /* bit depth */
    int noauto; /* don't autoselect this device; manual selection only */
    const char *button[LH_DEVICE_MAXBUTTONS]; /* UTF-8 encoded softbutton names (may be NULL) */
    lh_blob *logo; /* device logo or image */
    lh_object_calltable objtable;
    lh_device_calltable table;
} lh_device;

/**
  This structure is what defines a layout class methods. It's embedded in the
  \c lh_class structure. Members in this structure may be set to NULL.
  */
typedef struct lh_instance_calltable_t
{
    int size; // sizeof(lh_instance_calltable)
    // lh_object_calltable o;
    void * (*obj_new)(const lh_class*); /**< return a new instance of the class */
    void (*obj_prerender)(void*); /**< called right before width/height/render_xxx as a notification */
    int (*obj_width)(void*,int); /**< return suggested width given a height (or -1 for default width) */
    int (*obj_height)(void*,int); /**< return suggested height given a width (or -1 for default height) */
    const lh_blob * (*obj_render_blob)(void*,int,int); /**< render object to any image format using width x height */
    void * (*obj_render_qimage)(void*,int,int); /**< render object to QImage using width x height */
    void (*obj_delete)(void*); /**< delete this instance of the class */
} lh_instance_calltable;

#define lh_instance_calltable_NULL { sizeof(lh_instance_calltable), /*lh_object_calltable_NULL,*/ 0,0,0,0,0,0,0 }

/**
  This structure gives basic information about the class.

  Both the path and name members are UTF-8 encoded and NULL terminated strings.

  \c path is where to place the class in the class list tree. It must be in
  english, and LCDHost may attempt to translate the path items into the users
  language. Example: '/System/Date and time'

  \c id is the internal identifier for this class. It should remain static on
  all platforms/languages. The name of the shared library plus this will be
  used to identify the class uniquely.

  \c name is the text to present to the user as the class' name, for example
  'Clock'. This text should be localized to the users language.

  The width/height members are used to estimate the size of an instance before
  actually creating it. Set to -1 if you have no idea.
  */
struct lh_class_t
{
    int size; /* sizeof(lh_class) */
    const char *path; /* UTF-8 encoded, must NOT be localized */
    const char *id;   /* UTF-8 encoded, must NOT be localized */
    const char *name; /* UTF-8 encoded, should be localized */
    int width;
    int height;
    lh_object_calltable objtable;
    lh_instance_calltable table;
};


#ifdef __cplusplus
class lh_plugin_calltable
{
public:
    void * (*lh_create) (lh_callback_t, void *); /* Required */
    const lh_object_calltable *(*lh_get_object_calltable)(void*); /* Required */
    void (*lh_destroy) (void*); /* Required */
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Utility functions in lh_plugin.c */
#if 0
lh_blob *lh_binaryfile_to_blob( const char *filename ); /* Caller must free() the blob */
void lh_blob_to_headerfile( lh_blob *blob, const char *filename, const char *varname );
int lh_cpuload( lh_cpudata *from, lh_cpudata *to ); /* Returns average load between those times as permille */
#endif

#ifdef __cplusplus
}
#endif


#endif /* LH_PLUGIN_H */
