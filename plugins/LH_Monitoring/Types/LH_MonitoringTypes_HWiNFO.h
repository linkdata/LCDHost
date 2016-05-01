/*
  Copyright (c) 2009-2016 Johan Lindh <johan@linkdata.se>

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

#ifndef _HWISENSSM_H_INCLUDED_
#define _HWISENSSM_H_INCLUDED_

#define HWiNFO_SENSORS_MAP_FILE_NAME     "HWiNFO_SENS_SM"

#define HWiNFO_MAX_SENSORS         64    // max # of sensor instances (extended in v3)

#define HWiNFO_MAX_TEMPERATURES    128   // max # of temp sensor outputs per sensor
#define HWiNFO_MAX_VOLTAGES        32    // max # of volt sensor outputs per sensor
#define HWiNFO_MAX_FANS            16    // max # of fan sensor outputs per sensor
#define HWiNFO_MAX_CURRENTS        16    // max # of current sensor outputs per sensor
#define HWiNFO_MAX_POWERS          16    // max # of power sensor outputs per sensor
#define HWiNFO_MAX_CLOCKS          16    // max # of clock sensor outputs per sensor (added in v2)
#define HWiNFO_MAX_USAGES          32    // max # of usage sensor outputs per sensor (added in v2)
#define HWiNFO_MAX_OTHER           32    // max # of other sensor outputs per sensor

#define HWiNFO_SENSORS_STRING_LEN  128

#pragma pack(1)

#ifdef Q_OS_WIN
# include <windows.h>
#endif

typedef struct _HWiNFO_SENSORS_READING_LIST {

  BOOL fValid;                              // If this reading has a valid value
  char szLabel[HWiNFO_SENSORS_STRING_LEN];  // e.g. "Chassis2 Fan"
  char szUnit[16];                          // e.g. "RPM"
  double Value;

} HWiNFO_SENSORS_READING_LIST, *PHWiNFO_SENSORS_READING_LIST;

typedef struct _HWiNFO_SENSORS_SENSOR_ENTRY {

  BOOL fValid;          // If this sensor entry is present
  char szSensorName[HWiNFO_SENSORS_STRING_LEN];

  HWiNFO_SENSORS_READING_LIST Temperatures[HWiNFO_MAX_TEMPERATURES];
  HWiNFO_SENSORS_READING_LIST Voltages[HWiNFO_MAX_VOLTAGES];
  HWiNFO_SENSORS_READING_LIST Fans[HWiNFO_MAX_FANS];
  HWiNFO_SENSORS_READING_LIST Currents[HWiNFO_MAX_CURRENTS];
  HWiNFO_SENSORS_READING_LIST Powers[HWiNFO_MAX_POWERS];
  HWiNFO_SENSORS_READING_LIST Clocks[HWiNFO_MAX_CLOCKS];  // added in v2
  HWiNFO_SENSORS_READING_LIST Usages[HWiNFO_MAX_USAGES];  // added in v2
  HWiNFO_SENSORS_READING_LIST Others[HWiNFO_MAX_OTHER];

} HWiNFO_SENSORS_SENSOR_ENTRY, *PHWiNFO_SENSORS_SENSOR_ENTRY;

typedef struct _HWiNFO_SENSORS_SHARED_MEM_HEADER {

    DWORD   dwSignature;      // "HWiS"
    DWORD   dwVersion;        // v3 is latest
    DWORD   dwRevision;       //

} HWiNFO_SENSORS_SHARED_MEM_HEADER, *PHWiNFO_SENSORS_SHARED_MEM_HEADER;

typedef struct _HWiNFO_SENSORS_SHARED_MEM {

  HWiNFO_SENSORS_SHARED_MEM_HEADER header;
  __time64_t  poll_time;                                        // last polling time
  HWiNFO_SENSORS_SENSOR_ENTRY Sensors[HWiNFO_MAX_SENSORS];

} HWiNFO_SENSORS_SHARED_MEM, *PHWiNFO_SENSORS_SHARED_MEM;

#pragma pack()

#endif
