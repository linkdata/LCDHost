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


#include <QtGlobal>
#include <QDebug>
#include <QDateTime>

#define NDIS_SUPPORT_NDIS6 1

#include <stdio.h>
/* #include <limits.h> */
#include <windows.h>
#include <regstr.h>

#define NDIS_STATISTICS_INFO_REVISION_1                             1
#define OID_GEN_STATISTICS                                          0x00020106
#define NDIS_OBJECT_TYPE_DEFAULT                                    0x80
#define IOCTL_NDIS_QUERY_GLOBAL_STATS                               0x00170002
#define OID_GEN_CO_BYTES_XMIT                                       0x00020201
#define OID_GEN_CO_BYTES_RCV                                        0x00020207

#define NDIS_STATISTICS_FLAGS_VALID_DIRECTED_FRAMES_RCV             0x00000001
#define NDIS_STATISTICS_FLAGS_VALID_MULTICAST_FRAMES_RCV            0x00000002
#define NDIS_STATISTICS_FLAGS_VALID_BROADCAST_FRAMES_RCV            0x00000004
#define NDIS_STATISTICS_FLAGS_VALID_BYTES_RCV                       0x00000008
#define NDIS_STATISTICS_FLAGS_VALID_RCV_DISCARDS                    0x00000010
#define NDIS_STATISTICS_FLAGS_VALID_RCV_ERROR                       0x00000020
#define NDIS_STATISTICS_FLAGS_VALID_DIRECTED_FRAMES_XMIT            0x00000040
#define NDIS_STATISTICS_FLAGS_VALID_MULTICAST_FRAMES_XMIT           0x00000080
#define NDIS_STATISTICS_FLAGS_VALID_BROADCAST_FRAMES_XMIT           0x00000100
#define NDIS_STATISTICS_FLAGS_VALID_BYTES_XMIT                      0x00000200
#define NDIS_STATISTICS_FLAGS_VALID_XMIT_ERROR                      0x00000400
#define NDIS_STATISTICS_FLAGS_VALID_XMIT_DISCARDS                   0x00008000
#define NDIS_STATISTICS_FLAGS_VALID_DIRECTED_BYTES_RCV              0x00010000
#define NDIS_STATISTICS_FLAGS_VALID_MULTICAST_BYTES_RCV             0x00020000
#define NDIS_STATISTICS_FLAGS_VALID_BROADCAST_BYTES_RCV             0x00040000
#define NDIS_STATISTICS_FLAGS_VALID_DIRECTED_BYTES_XMIT             0x00080000
#define NDIS_STATISTICS_FLAGS_VALID_MULTICAST_BYTES_XMIT            0x00100000
#define NDIS_STATISTICS_FLAGS_VALID_BROADCAST_BYTES_XMIT            0x00200000

typedef struct _NDIS_OBJECT_HEADER
{
    UCHAR   Type;
    UCHAR   Revision;
    USHORT  Size;
} NDIS_OBJECT_HEADER, *PNDIS_OBJECT_HEADER;

typedef struct _NDIS_STATISTICS_INFO
{
    NDIS_OBJECT_HEADER          Header;
    ULONG                       SupportedStatistics;
    ULONG64                     ifInDiscards;           // OID_GEN_RCV_ERROR + OID_GEN_RCV_NO_BUFFER = OID_GEN_RCV_DISCARDS
    ULONG64                     ifInErrors;             // OID_GEN_RCV_ERROR
    ULONG64                     ifHCInOctets;           // OID_GEN_BYTES_RCV = OID_GEN_DIRECTED_BYTES_RCV + OID_GEN_MULTICAST_BYTES_RCV + OID_GEN_BROADCAST_BYTES_RCV
    ULONG64                     ifHCInUcastPkts;        // OID_GEN_DIRECTED_FRAMES_RCV
    ULONG64                     ifHCInMulticastPkts;    // OID_GEN_MULTICAST_FRAMES_RCV
    ULONG64                     ifHCInBroadcastPkts;    // OID_GEN_BROADCAST_FRAMES_RCV
    ULONG64                     ifHCOutOctets;          // OID_GEN_BYTES_XMIT = OID_GEN_DIRECTED_BYTES_XMIT + OID_GEN_MULTICAST_BYTES_XMIT + OID_GEN_BROADCAST_BYTES_XMIT
    ULONG64                     ifHCOutUcastPkts;       // OID_GEN_DIRECTED_FRAMES_XMIT
    ULONG64                     ifHCOutMulticastPkts;   // OID_GEN_MULTICAST_FRAMES_XMIT
    ULONG64                     ifHCOutBroadcastPkts;   // OID_GEN_BROADCAST_FRAMES_XMIT
    ULONG64                     ifOutErrors;            // OID_GEN_XMIT_ERROR
    ULONG64                     ifOutDiscards;          // OID_GEN_XMIT_DISCARDS
    ULONG64                     ifHCInUcastOctets;      // OID_GEN_DIRECTED_BYTES_RCV
    ULONG64                     ifHCInMulticastOctets;  // OID_GEN_MULTICAST_BYTES_RCV
    ULONG64                     ifHCInBroadcastOctets;  // OID_GEN_BROADCAST_BYTES_RCV
    ULONG64                     ifHCOutUcastOctets;     // OID_GEN_DIRECTED_BYTES_XMIT
    ULONG64                     ifHCOutMulticastOctets; // OID_GEN_MULTICAST_BYTES_XMIT
    ULONG64                     ifHCOutBroadcastOctets; // OID_GEN_BROADCAST_BYTES_XMIT
}NDIS_STATISTICS_INFO, *PNDIS_STATISTICS_INFO;

#include "NetworkEngine_NDIS.h"

class NDIS_Adapter
{
public:
    NDIS_Adapter(HANDLE h) : hDev(h), mode(0), working(false) {}
    ~NDIS_Adapter() { if( hDev != INVALID_HANDLE_VALUE ) CloseHandle(hDev); }

    HANDLE      hDev;
    QString     description;
    QString     servicename;
    int		mode; /* -1=disabled, 0=NDIS6, 1=NIDS5-64, 2=NDIS5-32 */
    bool        working; /* true if reporting stats */
};

NetworkEngine_NDIS::NetworkEngine_NDIS()
{
    HKEY	hKeyClassNet;
    HKEY	hKeyAdapter;
    WCHAR       wszName[ 16 ];
    DWORD	cbName;
    DWORD	dwIndex;
    DWORD	dwType, dwDataLen;
    WCHAR	wszBuf[ _MAX_PATH ];
    HANDLE	hMAC;
    QString     ndis_regkey("Software\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards");
    QString     filename;
    QString     servicename;
    QString     description;
    const char *modetext;

    /* enumerate all adapters */
    if( RegOpenKeyExW( HKEY_LOCAL_MACHINE, (const WCHAR*) ndis_regkey.utf16(), 0, KEY_READ, &hKeyClassNet) == ERROR_SUCCESS )
    {
        for( dwIndex = 0; cbName = sizeof(wszName), RegEnumKeyExW( hKeyClassNet, dwIndex, wszName, &cbName, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS; dwIndex++ )
        {
            if( RegOpenKeyExW( hKeyClassNet, wszName, 0, KEY_READ, &hKeyAdapter ) == ERROR_SUCCESS )
            {
                bool skip_it = true;

                servicename.clear();
                wszBuf[0] = 0;
                dwDataLen = sizeof( wszBuf );
                if( RegQueryValueExW( hKeyAdapter, L"ServiceName", NULL, &dwType,
                                     (PUCHAR)wszBuf, &dwDataLen ) == ERROR_SUCCESS && dwType == REG_SZ )
                {
                    /* find the service name in our list */
                    servicename = QString::fromUtf16( (const ushort*)wszBuf );
                    skip_it = false;
                    foreach( NDIS_Adapter *adapter, list_ )
                    {
                        if( adapter->servicename == servicename )
                        {
                            if( adapter->hDev != INVALID_HANDLE_VALUE ) skip_it = true;
                            break;
                        }
                    }
                }

                if( skip_it )
                {
                    /* ServiceName not found, or it's already in our list and has a valid handle */
                    RegCloseKey( hKeyAdapter );
                    continue;
                }

                /* Get the description */
                description.clear();
                wszBuf[0] = 0;
                dwDataLen = sizeof( wszBuf );
                if( RegQueryValueExW( hKeyAdapter, L"Description", NULL, &dwType, (PUCHAR)wszBuf, &dwDataLen ) == ERROR_SUCCESS )
                {
                    description = QString::fromUtf16( (const ushort*)wszBuf );
                }

                /* not open, or not previously seen, find a free spot */
                filename = "\\\\.\\";
                filename.append( servicename );
                hMAC = CreateFileW( (const WCHAR*) filename.utf16(), 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, INVALID_HANDLE_VALUE );
                if( hMAC != INVALID_HANDLE_VALUE )
                {
                    NDIS_Adapter *adapter = new NDIS_Adapter(hMAC);
                    adapter->description = description;
                    adapter->servicename = servicename;
                    list_.append( adapter );
                    sample( NULL, description );
                    switch( adapter->mode )
                    {
                    case 0: modetext = "NDIS 6"; break;
                    case 1: modetext = "NDIS 5.1 64-bit"; break;
                    case 2: modetext = "NDIS 5.1 32-bit"; break;
                    default: modetext = "NO"; break;
                    }
                    if( adapter->working ) qDebug() << "NDIS: Adapter" << adapter->description << "reporting" << modetext << "statistics";
                    else qDebug() << "NDIS: Adapter" << adapter->description << "isn't responding";
                }
                RegCloseKey( hKeyAdapter );
            }
        }
        RegCloseKey( hKeyClassNet );
    }
}

NetworkEngine_NDIS::~NetworkEngine_NDIS()
{
    while( !list_.isEmpty() )
        delete list_.takeFirst();
    return;
}

bool NetworkEngine_NDIS::working() const
{
    int count = 0;
    foreach( NDIS_Adapter *adapter, list_ )
        if( adapter->hDev != INVALID_HANDLE_VALUE && adapter->working ) count ++;
    return count ? true : false;
}

QStringList NetworkEngine_NDIS::list() const
{
    QStringList list;
    foreach( NDIS_Adapter *adapter, list_ )
        if( adapter->hDev != INVALID_HANDLE_VALUE && adapter->working ) list.append( adapter->description );
    return list;
}

void NetworkEngine_NDIS::sample( lh_netdata *data, QString which )
{
    ULONG OidCode;
    NDIS_STATISTICS_INFO stats;
    DWORD dwDataLen;
    ULONG64 long_in, long_out;
    ULONG32 short_in, short_out;

    if( data )
    {
        data->when = QDateTime::currentDateTime().toMSecsSinceEpoch();
        data->device = 0;
        data->in = 0;
        data->out = 0;
    }

    foreach( NDIS_Adapter *adapter, list_ )
    {
        if( which.isEmpty() || adapter->description == which )
        {
            if( adapter->hDev == INVALID_HANDLE_VALUE )
            {
                QString filename;
                filename = "\\\\.\\";
                filename.append( adapter->servicename );
                adapter->hDev = CreateFileW( (const WCHAR*) filename.utf16(), 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, INVALID_HANDLE_VALUE );
            }

            if( adapter->hDev == INVALID_HANDLE_VALUE ) break;

            if( data && !which.isEmpty() ) data->device = (qint64) adapter->hDev;

            if( adapter->mode == 0 )
            {
                /* NDIS6: OID_GEN_STATISTICS */
                OidCode = OID_GEN_STATISTICS;
                memset( &stats, 0, sizeof(stats) );
                stats.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
                stats.Header.Revision = NDIS_STATISTICS_INFO_REVISION_1;
                stats.Header.Size = sizeof(NDIS_STATISTICS_INFO);
                if( DeviceIoControl( adapter->hDev, IOCTL_NDIS_QUERY_GLOBAL_STATS, &OidCode, sizeof(OidCode), &stats, sizeof(stats), &dwDataLen, NULL ) )
                {
                    adapter->working = true;
                    if( data )
                    {
                        data->in += stats.ifHCInOctets;
                        data->out += stats.ifHCOutOctets;
                    }
                }
                else
                {
                    adapter->mode = 1;
                }
            }

            if( adapter->mode == 1 )
            {
                /* NDIS5: OID_GEN_CO_BYTES_XMIT/RCV, 64 bit */
                if( (OidCode = OID_GEN_CO_BYTES_XMIT),
                    DeviceIoControl( adapter->hDev, IOCTL_NDIS_QUERY_GLOBAL_STATS, &OidCode, sizeof(OidCode), &long_out, sizeof(long_out), &dwDataLen, NULL ) &&
                    (OidCode = OID_GEN_CO_BYTES_RCV),
                    DeviceIoControl( adapter->hDev, IOCTL_NDIS_QUERY_GLOBAL_STATS, &OidCode, sizeof(OidCode), &long_in, sizeof(long_in), &dwDataLen, NULL )
                    )
                {
                    adapter->working = true;
                    if( data )
                    {
                        data->in += long_in;
                        data->out += long_out;
                    }
                }
                else
                {
                    adapter->mode = 2;
                }
            }

            if( adapter->mode == 2 )
            {
                /* NDIS5: OID_GEN_CO_BYTES_XMIT/RCV, 32 bit */
                if( (OidCode = OID_GEN_CO_BYTES_XMIT),
                    DeviceIoControl( adapter->hDev, IOCTL_NDIS_QUERY_GLOBAL_STATS, &OidCode, sizeof(OidCode), &short_out, sizeof(short_out), &dwDataLen, NULL ) &&
                    (OidCode = OID_GEN_CO_BYTES_RCV),
                    DeviceIoControl( adapter->hDev, IOCTL_NDIS_QUERY_GLOBAL_STATS, &OidCode, sizeof(OidCode), &short_in, sizeof(short_in), &dwDataLen, NULL )
                    )
                    {
                    adapter->working = true;
                    if( data )
                    {
                        data->in += short_in;
                        data->out += short_out;
                    }
                }
                else
                {
                    adapter->mode = -1;
                }
            }

            if( adapter->mode == -1 && adapter->working )
            {
                /* device is now disabled. */
                /* if it was once working, reinitialize it */
                if( adapter->hDev != INVALID_HANDLE_VALUE )
                {
                    qDebug() << "NDIS device" << adapter->description << "has stopped reporting stats, trying to reopen it";
                    CloseHandle(adapter->hDev);
                    adapter->hDev = INVALID_HANDLE_VALUE;
                    adapter->mode = 0;
                }
            }
        }
    }

    return;
}

