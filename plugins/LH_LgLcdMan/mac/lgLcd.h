//Copyright 2010 Logitech, Inc.

//************************************************************************
//  The Logitech LCD SDK, including all acompanying documentation,
//  is protected by intellectual property laws.  All use of the Logitech
//  LCD SDK is subject to the License Agreement found in the
//  "Logitech LCD SDK License Agreement" file and in the Reference Manual.  
//  All rights not expressly granted by Logitech are reserved.
//************************************************************************

/************************************************************************************
 
 * File name:   lgLcd.h
 *
 * Copyright:   Copyright © 2010, Logitech Inc.
 *
 * Description:  Description of Applet APIs funtion and structures...
 
 *************************************************************************************/
#ifndef _lgLcd_H_
#define _lgLcd_H_

#include <Carbon/Carbon.h>

#ifdef __cplusplus
extern "C" {
#endif


#define LGLCD_INVALID_CONNECTION            (-1)
#define LGLCD_INVALID_DEVICE                (-1)

typedef int lgLcdDeviceRef;
typedef int lgLcdConnectionRef;
typedef CFStringRef lgLcdStringRef;


#define MANUAL_METHOD                   0
#define AUTOSTART_METHOD                1
#define AUTOSTARTABLE_ALWAYS            2
#define NON_AUTOSTARTABLE_ALWAYS        3

#define LGLCD_HINT_MARKER       (0x80000000)

// Common Soft-Buttons available through the SDK
#define LGLCDBUTTON_LEFT                    (0x00000100)
#define LGLCDBUTTON_RIGHT                   (0x00000200)
#define LGLCDBUTTON_OK                      (0x00000400)
#define LGLCDBUTTON_CANCEL                  (0x00000800)
#define LGLCDBUTTON_UP                      (0x00001000)
#define LGLCDBUTTON_DOWN                    (0x00002000)
#define LGLCDBUTTON_MENU                    (0x00004000)

// Soft-Button masks. Kept for backwards compatibility
#define LGLCDBUTTON_BUTTON0                 (0x00000001)
#define LGLCDBUTTON_BUTTON1                 (0x00000002)
#define LGLCDBUTTON_BUTTON2                 (0x00000004)
#define LGLCDBUTTON_BUTTON3                 (0x00000008)
#define LGLCDBUTTON_BUTTON4                 (0x00000010)
#define LGLCDBUTTON_BUTTON5                 (0x00000020)
#define LGLCDBUTTON_BUTTON6                 (0x00000040)
#define LGLCDBUTTON_BUTTON7                 (0x00000080)


//************************************************************************
// lgLcdDeviceDesc
//************************************************************************
typedef struct
    {
        unsigned long Width;
        unsigned long Height;
        unsigned long Bpp;
        unsigned long NumSoftButtons;
    } lgLcdDeviceDesc;


//************************************************************************
// lgLcdDeviceDescEx
//************************************************************************
typedef struct
    {
        unsigned long deviceFamilyId;
        lgLcdStringRef  deviceDisplayName;
        unsigned long Width;            // # of pixels (horizontally) on the LCD
        unsigned long Height;           // # of pixels (lines) on the LCD
        unsigned long Bpp;              // # of bits per pixel (1,8,16,24,...)
        unsigned long NumSoftButtons;
        unsigned long Reserved1;
        unsigned long Reserved2;
    } lgLcdDeviceDescEx;


//************************************************************************
// lgLcdBitmap
//************************************************************************

#define LGLCD_BMP_FORMAT_160x43x1           (0x00000001)
#define LGLCD_BMP_FORMAT_QVGAx32         (0x00000003)
#define LGLCD_BMP_WIDTH                     (160)
#define LGLCD_BMP_HEIGHT                    (43)
#define LGLCD_BW_BMP_WIDTH                  (160)
#define LGLCD_BW_BMP_HEIGHT                 (43)
#define LGLCD_QVGA_BMP_WIDTH             (320)
#define LGLCD_QVGA_BMP_HEIGHT            (240)
#define LGLCD_QVGA_BMP_BPP                  (4)

typedef struct
    {
        unsigned long Format;
    } lgLcdBitmapHeader;

typedef struct
    {
        lgLcdBitmapHeader hdr;
        unsigned char pixels[LGLCD_BMP_WIDTH * LGLCD_BMP_HEIGHT];
    } lgLcdBitmap160x43x1;

typedef struct
    {
        lgLcdBitmapHeader hdr;  // Format = LGLCD_BMP_FORMAT_QVGAx32
        unsigned char pixels[LGLCD_QVGA_BMP_WIDTH * LGLCD_QVGA_BMP_HEIGHT * 4];
    } lgLcdBitmapQVGAx32;
//
// Generic bitmap for use by both color and BW applets
//
typedef union
    {
        lgLcdBitmapHeader hdr;          // provides easy access to the header
        lgLcdBitmap160x43x1 bmp_mono;   // B/W bitmap data
        lgLcdBitmapQVGAx32 bmp_qvga32;  // Color bitmap data
    } lgLcdBitmap;

// Priorities
#define LGLCD_PRIORITY_IDLE_NO_SHOW                 (0)
#define LGLCD_PRIORITY_BACKGROUND                   (64)    
#define LGLCD_PRIORITY_NORMAL                       (128)
#define LGLCD_PRIORITY_ALERT                        (255)
#define LGLCD_SYNC_UPDATE(priority)                 (0x80000000 | (priority))
#define LGLCD_SYNC_COMPLETE_WITHIN_FRAME(priority)  (0xC0000000 | (priority))
#define LGLCD_ASYNC_UPDATE(priority)                (priority)

// Foreground mode for client applications
#define LGLCD_LCD_FOREGROUND_APP_NO                 (0)
#define LGLCD_LCD_FOREGROUND_APP_YES                (1)

// Device family definitions
#define LGLCD_DEVICE_FAMILY_BW_160x43_GAMING           (0x00000001)
#define LGLCD_DEVICE_FAMILY_KEYBOARD_G15                (0x00000001)
#define LGLCD_DEVICE_FAMILY_BW_160x43_AUDIO             (0x00000002)
#define LGLCD_DEVICE_FAMILY_SPEAKERS_Z10                (0x00000002)
#define LGLCD_DEVICE_FAMILY_JACKBOX                     (0x00000004)
#define LGLCD_DEVICE_FAMILY_BW_160x43_BASIC            (0x00000008)
#define LGLCD_DEVICE_FAMILY_LCDEMULATOR_G15             (0x00000008)
#define LGLCD_DEVICE_FAMILY_RAINBOW                 (0x00000010)
#define LGLCD_DEVICE_FAMILY_QVGA_BASIC              (0x00000020)
#define LGLCD_DEVICE_FAMILY_QVGA_GAMING            (0x00000040)
#define LGLCD_DEVICE_FAMILY_GAMEBOARD_G13               (0x00000080)
#define LGLCD_DEVICE_FAMILY_OTHER                       (0x80000000)

// Capabilities of applets connecting to LCD Manager. 
#define LGLCD_APPLET_CAP_BASIC                          (0x00000000)
#define LGLCD_APPLET_CAP_BW                             (0x00000001)
#define LGLCD_APPLET_CAP_QVGA                           (0x00000002)

// Notifications sent by LCD Manager to applets connected to it. 
#define LGLCD_NOTIFICATION_DEVICE_ARRIVAL               (0x00000001)
#define LGLCD_NOTIFICATION_DEVICE_REMOVAL               (0x00000002)
#define LGLCD_NOTIFICATION_CLOSE_CONNECTION             (0x00000003)
#define LGLCD_NOTIFICATION_APPLET_DISABLED              (0x00000004)
#define LGLCD_NOTIFICATION_APPLET_ENABLED              (0x00000005)
#define LGLCD_NOTIFICATION_TERMINATE_APPLET          (0x00000006)

// Device types used in notifications
#define LGLCD_DEVICE_BW                         (0x00000001)
#define LGLCD_DEVICE_QVGA                           (0x00000002)

//************************************************************************
// Callbacks
//************************************************************************

// Callback used to notify client of soft button change
typedef unsigned long ( *lgLcdOnSoftButtonsCB)( 
                                               int device,
                                               unsigned long inButtons,
                                               const void* pContext);

// Callback used to allow client to pop up a "configuration panel"
typedef unsigned long (*lgLcdOnConfigureCB)( int inConnection,
                                            const void* pContext);

// Callback used to notify client of events, such as device arrival, ...
// Arrival, removal, applet enable/disable supported as of version 3.0.
typedef unsigned long ( *lgLcdOnNotificationCB)( int inConnection,
                                                const void* pContext,
                                                unsigned long inNotificationCode,
                                                unsigned long inNotifyParm1,
                                                unsigned long inNotifyParm2,
                                                unsigned long inNotifyParm3,
                                                unsigned long inNotifyParm4);



// lgLcdNotificationContext
//************************************************************************
typedef struct
    {
        // Set to NULL if not notifiable
        lgLcdOnNotificationCB   notificationCallback;
        void*                   notifyContext;
    } lgLcdNotificationContext;

//************************************************************************
// lgLcdConfigureContext
//************************************************************************
typedef struct
    {
        // Set to NULL if not configurable
        lgLcdOnConfigureCB  configCallback;
        void*               configContext;
    } lgLcdConfigureContext;

typedef struct
    {
        // "Friendly name" display in the listing
        lgLcdStringRef appFriendlyName;
        // isPersistent determines whether this connection persists in the list
        Boolean isPersistent;
        // isAutostartable determines whether the client can be started byLCDMon
        Boolean isAutostartable;
        lgLcdConfigureContext onConfigure;
        unsigned int dwAppletCapabilitiesSupported;
        // --> Connection handle
        lgLcdConnectionRef connection; // Reference to clientconnection on 
        //return
        unsigned long dwReserved1;
        lgLcdNotificationContext onNotify;
        lgLcdStringRef  iconName;// Additional field used for Mac, to display      
        //Applet icon in UI and color display.
    } lgLcdConnectContext;  

typedef lgLcdConnectContext lgLcdConnectContextEx;

//************************************************************************
// lgLcdOpenContext
//************************************************************************

typedef struct
    {
        // Set to NULL if no softbutton notifications are needed
        lgLcdOnSoftButtonsCB softbuttonsChangedCallback;
        void* softbuttonsChangedContext;
    } lgLcdSoftbuttonsChangedContext;

typedef struct
    {
        lgLcdConnectionRef connection;
        // Device index to open
        int index;
        lgLcdSoftbuttonsChangedContext onSoftbuttonsChanged;
        // --> Device handle
        lgLcdDeviceRef device;
    } lgLcdOpenContext;

    
typedef struct
    {
        lgLcdConnectionRef connection;
        // Device type to open (either LGLCD_DEVICE_BW or LGLCD_DEVICE_QVGA)
        int deviceType;
        lgLcdSoftbuttonsChangedContext onSoftbuttonsChanged;
        // --> Device handle
        lgLcdDeviceRef device;
    } lgLcdOpenByTypeContext;
    

//************************************************************************
// Prototypes
//************************************************************************

// Initialize the library by calling this function.
int lgLcdInit(void);

// Must be called to release the library and free all allocated structures.
int lgLcdDeInit(void);


int lgLcdConnect(lgLcdConnectContext *ctx);

//If the callbacks are needed..
int lgLcdConnectEx(lgLcdConnectContextEx *ctx); 

// Must be called to release the connection and free all allocated resources
int lgLcdDisconnect(lgLcdConnectionRef connection);


int lgLcdSetDeviceFamiliesToUse(lgLcdConnectionRef connection,
                                unsigned long dwDeviceFamiliesSupported);    // Or'd combination of LGLCD_DEVICE_FAMILY_... defines


int lgLcdEnumerate(lgLcdConnectionRef connection,  int index,
                   lgLcdDeviceDesc *description);

int lgLcdOpen( lgLcdOpenContext *ctx);

    
// Opens by type
int lgLcdOpenByType(lgLcdOpenByTypeContext *ctx);   

    
// Closes the LCD. Must be paired with lgLcdOpen.
int lgLcdClose( lgLcdDeviceRef device);

// Reads the state of the soft buttons for the device.
int lgLcdReadSoftButtons(lgLcdDeviceRef device, unsigned long *buttons);

// Provides a bitmap to be displayed on the LCD. The priority field
// further describes the way in which the bitmap is to be applied.
int lgLcdUpdateBitmap(lgLcdDeviceRef device,
                      const lgLcdBitmapHeader *bitmap,
                      unsigned long priority);

// Sets the calling application as the shown application on the LCD, and stops
// any type of rotation among other applications on the LCD.
int lgLcdSetAsLCDForegroundApp(lgLcdDeviceRef device,  int foregroundYesNoFlag);

#ifdef __cplusplus
}
#endif

#endif // _lgLcd_H_
