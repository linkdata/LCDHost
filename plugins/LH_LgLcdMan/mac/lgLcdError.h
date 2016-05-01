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
 * Author:      Adarsh
 *
 * Copyright:   Copyright © 2010, Logitech 
 *              Written under contract by Robosoft Technologies Pvt. Ltd.
 *
 * Description: Applet API error codes.
 
 *************************************************************************************/

#pragma once

// Applet API error codes
enum {
    ERROR_FAILED                = -1,
    ERROR_SUCCESS               = 0,
    ERROR_ACCESS_DENIED         = 5L,
    ERROR_INVALID_PARAMETER     = 87L,
    ERROR_CALL_NOT_IMPLEMENTED  = 120L,
    ERROR_LOCK_FAILED           = 167L,
    ERROR_ALREADY_EXISTS        = 183L,
    ERROR_NO_MORE_ITEMS         = 259L,
    ERROR_DEVICE_NOT_CONNECTED  = 1167L,
    ERROR_INTERNAL_ERROR        = 1359L,
    ERROR_TIMEOUT               = 1460L,
    RPC_S_PROTOCOL_ERROR        = 1728L,
    RPC_X_WRONG_PIPE_VERSION    = 1832L
};
